/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * uielement.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <stdlib.h>
#include <math.h>

#include "trigger.h"
#include "uielement.h"
#include "collection.h"
#include "canvas.h"
#include "brush.h"
#include "transform.h"
#include "runtime.h"
#include "geometry.h"
#include "shape.h"
#include "dirty.h"
#include "eventargs.h"
#include "timemanager.h"
#include "media.h"
#include "resources.h"
#include "popup.h"
#include "window.h"
#include "provider.h"
#include "effect.h"
#include "projection.h"
#include "factory.h"
#include "bitmapcache.h"

namespace Moonlight {

//#define DEBUG_INVALIDATE 0

UIElement::UIElement ()
	: DependencyObject (Type::UIELEMENT), visual_parent (this, VisualParentWeakRef), subtree_object (this, SubtreeObjectWeakRef)
{
	Init ();
}


UIElement::UIElement (Type::Kind object_type)
	: DependencyObject (object_type), visual_parent (this, VisualParentWeakRef), subtree_object (this, SubtreeObjectWeakRef)
{
	Init ();
}

void
UIElement::Init ()
{
	providers.inherited = new InheritedPropertyValueProvider (this, PropertyPrecedence_Inherited);

	loaded = false;
	visual_level = 0;
	subtree_object = NULL;
	opacityMask = NULL;
	
	flags = UIElement::RENDER_VISIBLE | UIElement::HIT_TEST_VISIBLE;

	hidden_desire = Size (-INFINITY, -INFINITY);
	bounds = Rect (0,0,0,0);
	global_bounds = Rect (0,0,0,0);
	surface_bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);
	cairo_matrix_init_identity (&layout_xform);
	cairo_matrix_init_identity (&local_xform);
	cairo_matrix_init_identity (&render_xform);
	cairo_matrix_init_identity (&cache_xform);
	Matrix3D::Identity (local_projection);
	Matrix3D::Identity (absolute_projection);
	Matrix3D::Identity (render_projection);
	effect_padding = Thickness (0);
	bitmap_cache = NULL;
	bitmap_cache_size = 0;

	dirty_flags = DirtyMeasure;
	PropagateFlagUp (DIRTY_MEASURE_HINT);
	up_dirty_node = down_dirty_node = NULL;
	force_invalidate_of_new_bounds = false;
	dirty_region = new Region ();

	desired_size = Size (0, 0);
	render_size = Size (0, 0);
	
	ComputeLocalTransform ();
	ComputeLocalProjection ();
	ComputeTotalRenderVisibility ();
	ComputeTotalHitTestVisibility ();
}

UIElement::~UIElement()
{
	InvalidateBitmapCache ();
	delete dirty_region;
}

void
UIElement::SetIsLoaded (bool value)
{
	if (IsLoaded () != value) {
		loaded = value;
		OnIsLoadedChanged (loaded);
	}
}

void
UIElement::OnIsLoadedChanged (bool loaded)
{
	// If we unload, emit the Unloaded event before the children
	if (!loaded) {
		ClearForeachGeneration (UIElement::LoadedEvent);
		Emit (UIElement::UnloadedEvent);
	}

	// Then propagate the state change to the children
	VisualTreeWalker walker (this);
	while (UIElement *element = walker.Step ())
		element->SetIsLoaded (loaded);

	// Finally if we're loading, emit the Loaded event after the children
	if (loaded && HasHandlers (UIElement::LoadedEvent)) {
		GetDeployment ()->AddAllLoadedHandlers (this, true);
		GetDeployment ()->EmitLoadedAsync ();
	}
}


bool
UIElement::IsSubtreeLoaded (UIElement *element)
{
	while (element && !element->IsLoaded ())
		element = element->GetVisualParent ();
	return element;
}


bool
UIElement::CoerceCursor (DependencyObject *obj, DependencyProperty *p, Value *value, Value **coerced, MoonError *error)
{
	if (Value::IsNull (value))
		*coerced = new Value (CursorTypeDefault, Type::CURSORTYPE);
	else
		*coerced = new Value (*value);
	return true;
}

void
UIElement::Dispose()
{
	if (IsAttached ()) {
		Surface *surface = GetDeployment ()->GetSurface ();
		if (surface)
			surface->RemoveDirtyElement (this);
	}

	Value *v = GetValueNoAutoCreate (TriggersProperty);
	if (v && !v->IsNull) {
		TriggerCollection *triggers = v->AsTriggerCollection ();
	
		if (triggers != NULL) {
			int triggers_count = triggers->GetCount ();
			for (int i = 0; i < triggers_count; i++)
				triggers->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
		}
	}
	
	subtree_object = NULL;

	DependencyObject::Dispose();
}

void
UIElement::OnIsAttachedChanged (bool value)
{
	if (subtree_object)
		subtree_object->SetIsAttached (value);
	
	DependencyObject::OnIsAttachedChanged (value);

	if (!value) {
		CacheInvalidateHint ();

		/* we're losing our surface, delete ourselves from the dirty list if we're on it */
		Surface *surface = GetDeployment ()->GetSurface ();
		if (surface) {
			surface->RemoveDirtyElement (this);
			if (surface->GetFocusedElement () == this)
				surface->FocusElement (NULL);
		}
	}
}

Rect
UIElement::IntersectBoundsWithClipPath (Rect unclipped, bool transform)
{
	Geometry *clip = GetClip ();
	Geometry *layout_clip = transform ? NULL : LayoutInformation::GetLayoutClip (this);
	Rect box;

	if (!clip && !layout_clip)
		return unclipped;

	if (clip)
		box = clip->GetBounds ();
	else
		box = layout_clip->GetBounds ();

	if (layout_clip)
		box = box.Intersection (layout_clip->GetBounds());

	if (!GetRenderVisible())
		box = Rect (0,0,0,0);

	if (transform)
		box = box.Transform (&absolute_xform);

	return box.Intersection (unclipped);
}

void
UIElement::RenderClipPath (cairo_t *cr, bool path_only)
{
	cairo_new_path (cr);

	Geometry *geometry = GetClip();
	if (!geometry)
		return;

	geometry->Draw (cr);
	if (!path_only)
		cairo_clip (cr);
}

void
UIElement::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::UIELEMENT) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	  
	if (args->GetId () == UIElement::OpacityProperty) {
		InvalidateVisibility ();
	} else if (args->GetId () == UIElement::VisibilityProperty) {
		// note: invalid enum values are only validated in 1.1 (managed code),
		// the default value for VisibilityProperty is VisibilityCollapsed
		// (see bug #340799 for more details)
		if (args->GetNewValue()->AsVisibility () == VisibilityVisible)
			flags |= UIElement::RENDER_VISIBLE;
		else
			flags &= ~UIElement::RENDER_VISIBLE;

		InvalidateVisibility ();
		InvalidateMeasure ();
		if (GetVisualParent ())
			GetVisualParent ()->InvalidateMeasure ();
		if (GetDeployment ()->GetSurface ()->GetFocusedElement () == this) {
			GetDeployment ()->GetSurface ()->FocusElement (NULL);
		}
	} else if (args->GetId () == UIElement::IsHitTestVisibleProperty) {
		if (args->GetNewValue()->AsBool())
			flags |= UIElement::HIT_TEST_VISIBLE;
		else
			flags &= ~UIElement::HIT_TEST_VISIBLE;

		UpdateTotalHitTestVisibility();
	} else if (args->GetId () == UIElement::ClipProperty) {
		InvalidateClip ();
	} else if (args->GetId () == UIElement::OpacityMaskProperty) {
		opacityMask = args->GetNewValue() ? args->GetNewValue()->AsBrush() : NULL;
		InvalidateMask ();
	} else if (args->GetId () == UIElement::RenderTransformProperty 
		   || args->GetId () == UIElement::RenderTransformOriginProperty) {
		UpdateTransform ();
	}
	else if (args->GetId () == UIElement::TriggersProperty) {
		if (args->GetOldValue()) {
			// remove the old trigger targets
			TriggerCollection *triggers = args->GetOldValue()->AsTriggerCollection();
			int triggers_count = triggers->GetCount ();
			for (int i = 0; i < triggers_count; i++)
				triggers->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
		}

		if (args->GetNewValue()) {
			// set the new ones
			TriggerCollection *triggers = args->GetNewValue()->AsTriggerCollection();
			int triggers_count = triggers->GetCount ();
			for (int i = 0; i < triggers_count; i++)
				triggers->GetValueAt (i)->AsEventTrigger ()->SetTarget (this);
		}
	} else if (args->GetId () == UIElement::UseLayoutRoundingProperty) {
		InvalidateMeasure ();
		InvalidateArrange ();
	} else if (args->GetId () == UIElement::EffectProperty) {
		bool old_effect = args->GetOldValue () != NULL;
		bool new_effect = args->GetNewValue () != NULL;

		InvalidateEffect ();

		/* need to update transform when whether we render to
		   an intermediate buffer or not change */
		if (old_effect != new_effect && IsAttached ())
			GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyTransform);
	} else if (args->GetId () == UIElement::ProjectionProperty) {
		UpdateProjection ();
	} else if (args->GetId () == UIElement::CacheModeProperty) {
		bool old_mode = args->GetOldValue () != NULL;
		bool new_mode = args->GetNewValue () != NULL;

		InvalidateCacheMode ();

		/* need to update transform when whether we render to
		   an intermediate buffer or not change */
		if (old_mode != new_mode && IsAttached ())
			GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyTransform);
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

void
UIElement::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (PropertyHasValueNoAutoCreate (UIElement::TriggersProperty, col)) {
		switch (args->GetChangedAction()) {
		case CollectionChangedActionReplace:
			args->GetOldItem()->AsEventTrigger ()->RemoveTarget (this);
			// fall thru to Add
		case CollectionChangedActionAdd:
			args->GetNewItem()->AsEventTrigger ()->SetTarget (this);
			break;
		case CollectionChangedActionRemove:
			args->GetOldItem()->AsEventTrigger ()->RemoveTarget (this);
			break;
		case CollectionChangedActionClearing: {
			int triggers_count = col->GetCount ();
			for (int i = 0; i < triggers_count; i++)
				col->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
			break;
		}
		case CollectionChangedActionCleared:
			// nothing needed here.
			break;
		}
	}
	else {
		DependencyObject::OnCollectionChanged (col, args);
	}
}

#if 1
int
UIElement::DumpHierarchy (UIElement *obj)
{
	if (obj == NULL)
		return 0;
	
	int n = DumpHierarchy (obj->GetVisualParent ());
	for (int i = 0; i < n; i++)
		putchar (' ');
	printf ("%s (%p)\n", obj->GetTypeName(), obj);
	return n + 4;
}
#endif

void
UIElement::UpdateBounds (bool force_redraw)
{
	//InvalidateMeasure ();
	//InvalidateArrange ();

	if (IsAttached ())
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyBounds);

	force_invalidate_of_new_bounds |= force_redraw;
}

void
UIElement::UpdateTotalRenderVisibility ()
{
	if (IsAttached ())
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyRenderVisibility);
}

void
UIElement::UpdateTotalHitTestVisibility ()
{
	if (IsAttached ())
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyHitTestVisibility);
}

bool
UIElement::GetActualTotalRenderVisibility ()
{
	bool visible = (flags & UIElement::RENDER_VISIBLE) != 0;
	bool parent_visible = true;

	total_opacity = GetOpacity ();

	if (GetVisualParent ()) {
		GetVisualParent ()->ComputeTotalRenderVisibility ();
		parent_visible = visible && GetVisualParent ()->GetRenderVisible ();
		total_opacity *= GetVisualParent ()->total_opacity;
	}

	visible = visible && parent_visible;

	return visible;
}

void
UIElement::ComputeTotalRenderVisibility ()
{
	if (GetActualTotalRenderVisibility ())
		flags |= UIElement::TOTAL_RENDER_VISIBLE;
	else
		flags &= ~UIElement::TOTAL_RENDER_VISIBLE;
}

bool
UIElement::GetActualTotalHitTestVisibility ()
{
	bool visible = (flags & UIElement::HIT_TEST_VISIBLE) != 0;

	if (visible && GetVisualParent ()) {
		GetVisualParent ()->ComputeTotalHitTestVisibility ();
		visible = visible && GetVisualParent ()->GetHitTestVisible ();
	}

	return visible;
}

void
UIElement::ComputeTotalHitTestVisibility ()
{
	if (GetActualTotalHitTestVisibility ())
		flags |= UIElement::TOTAL_HIT_TEST_VISIBLE;
	else
		flags &= ~UIElement::TOTAL_HIT_TEST_VISIBLE;
}

void
UIElement::UpdateTransform ()
{
	if (IsAttached ()) {
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyLocalTransform);
	}
}

void
UIElement::ComputeLocalTransform ()
{
	Transform *transform = GetRenderTransform ();
	Point transform_origin = GetTransformOrigin ();
	cairo_matrix_t render;
	
	cairo_matrix_init_identity (&render);
	cairo_matrix_init_identity (&local_xform);

	if (transform == NULL)
		return;

	transform->GetTransform (&render);
	cairo_matrix_translate (&local_xform, transform_origin.x, transform_origin.y);
	cairo_matrix_multiply (&local_xform, &render, &local_xform);
	cairo_matrix_translate (&local_xform, -transform_origin.x, -transform_origin.y);
}

void
UIElement::UpdateProjection ()
{
	if (IsAttached ()) {
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyLocalProjection);
	}
}

void
UIElement::ComputeLocalProjection ()
{
	Projection *projection = GetRenderProjection ();
	double width, height;

	if (projection == NULL) {
		Canvas::SetZ (this, NAN);
		return;
	}

	GetSizeForBrush (NULL, &width, &height);
	projection->SetObjectSize (width, height);
	Canvas::SetZ (this, projection->DistanceFromXYPlane ());
}

void
UIElement::TransformBounds (cairo_matrix_t *old, cairo_matrix_t *current)
{
	Rect updated;

	cairo_matrix_t tween = *old;
	cairo_matrix_invert (&tween);
	cairo_matrix_multiply (&tween, &tween, current);

	Point p0 (0,0);
	Point p1 (1,0);
	Point p2 (1,1);
	Point p3 (0,1);
	
	p0 = p0 - p0.Transform (&tween);
	p1 = p1 - p1.Transform (&tween);
	p2 = p2 - p2.Transform (&tween);
	p3 = p3 - p3.Transform (&tween);

	if (p0 == p1 && p1 == p2 && p2 == p3) {
		//printf ("shifting position\n");
		ShiftPosition (bounds.GetTopLeft ().Transform (&tween));
		ComputeGlobalBounds ();
		ComputeSurfaceBounds ();
		return;
	}

	UpdateBounds ();
}

void
UIElement::ComputeTransform ()
{
	Projection *projection = GetRenderProjection ();
	CacheMode *cacheMode = GetRenderCacheMode ();
	cairo_matrix_t old = absolute_xform;
	cairo_matrix_t old_cache = cache_xform;
	double m[16], old_projection[16];
	Matrix3D::Init (old_projection, local_projection);
	cairo_matrix_init_identity (&absolute_xform);
	cairo_matrix_init_identity (&render_xform);
	cairo_matrix_init_identity (&cache_xform);
	Matrix3D::Identity (absolute_projection);
	Matrix3D::Identity (local_projection);
	flags &= ~UIElement::RENDER_PROJECTION;

	if (GetVisualParent () != NULL) {
		absolute_xform = GetVisualParent ()->absolute_xform;
		Matrix3D::Init (absolute_projection,
				GetVisualParent ()->absolute_projection);
	} else if (GetParent () != NULL && GetParent ()->Is (Type::POPUP)) {  
		// FIXME we shouldn't be examing a subclass type here but we'll do this
		// for now to get popups in something approaching the right place while
		// we figure out a cleaner way to handle it long term.
		Popup *popup = (Popup *)GetParent ();
		UIElement *el = (UIElement *) popup;

		// determine whether we inherit an affine or a perspective
		// transformation from our parent.
		while (el) {
			flags |= (el->flags & UIElement::RENDER_PROJECTION);
			el = (UIElement *) el->GetVisualParent ();
		}

		if (flags & UIElement::RENDER_PROJECTION) {
			Matrix3D::Init (local_projection,
					popup->absolute_projection);
			Matrix3D::Translate (m,
					     popup->GetHorizontalOffset (),
					     popup->GetVerticalOffset (),
					     0.0);
			Matrix3D::Multiply (local_projection, m,
					    local_projection);
		}
		else {
			// use absolute_projection as rendering to
			// intermediate buffer might have reset
			// absolute_xform
#define M(row, col) (popup->absolute_projection)[col * 4 + row]
			render_xform.xx = M (0, 0);
			render_xform.yx = M (1, 0);
			render_xform.xy = M (0, 1);
			render_xform.yy = M (1, 1);
			render_xform.x0 = M (0, 3);
			render_xform.y0 = M (1, 3);
#undef M

			cairo_matrix_translate (&render_xform,
						popup->GetHorizontalOffset (),
						popup->GetVerticalOffset ());
		}
	}

	cairo_matrix_multiply (&render_xform, &layout_xform, &render_xform);
	cairo_matrix_multiply (&render_xform, &local_xform, &render_xform);

	Matrix3D::Affine (m,
			  render_xform.xx, render_xform.xy,
			  render_xform.yx, render_xform.yy,
			  render_xform.x0, render_xform.y0);
	Matrix3D::Multiply (local_projection, m, local_projection);

	// reset vector transformations when rendering to intermediate buffer
	if (RenderToIntermediate ()) {
		cairo_matrix_init_identity (&render_xform);
		cairo_matrix_init_identity (&absolute_xform);
	}
	else {
		cairo_matrix_multiply (&absolute_xform, &render_xform,
				       &absolute_xform);
	}

	if (projection) {
		projection->GetTransform (m);
		Matrix3D::Multiply (local_projection, m, local_projection);
		flags |= UIElement::RENDER_PROJECTION;
	}

	Matrix3D::Multiply (absolute_projection, local_projection,
			    absolute_projection);

	if (memcmp (old_projection, local_projection, sizeof (double) * 16)) {
		if (GetVisualParent ())
			GetVisualParent ()->Invalidate (GetSubtreeBounds ());
		else if (GetDeployment ()->GetSurface ()->IsTopLevel (this))
			InvalidateSubtreePaint ();

		if (IsAttached ())
			GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyNewBounds);
	}

	if (cacheMode) {
		cairo_matrix_t inverse;

		// ignore bitmap cache scale when effect property is set
		if (!GetRenderEffect ())
			cacheMode->GetTransform (&cache_xform);
			
		if (memcmp (&old_cache, &cache_xform, sizeof (cairo_matrix_t)))
			InvalidateBitmapCache ();
		
		inverse = cache_xform;
		cairo_matrix_invert (&inverse);

		Matrix3D::Affine (m,
				  inverse.xx, inverse.xy,
				  inverse.yx, inverse.yy,
				  inverse.x0, inverse.y0);
		Matrix3D::Multiply (render_projection, m, local_projection);
	}
	else {
		Matrix3D::Init (render_projection, local_projection);
	}

	if (moonlight_flags & RUNTIME_INIT_USE_UPDATE_POSITION
	    && !(dirty_flags & DirtyBounds))
		TransformBounds (&old, &absolute_xform);
	else {
		UpdateBounds ();
	}

	ComputeComposite ();
}

void
UIElement::ComputeBounds ()
{
	g_warning ("UIElement:ComputeBounds has been called. The derived class %s should have overridden it.",
		   GetTypeName ());
}

void
UIElement::ComputeGlobalBounds ()
{
	global_bounds = IntersectBoundsWithClipPath (extents.GrowBy (effect_padding), false).Transform (local_projection);
}

void
UIElement::ComputeSurfaceBounds ()
{
	surface_bounds = IntersectBoundsWithClipPath (extents.GrowBy (effect_padding), false).Transform (absolute_projection);
}

void
UIElement::ShiftPosition (Point p)
{
	bounds.x = p.x;
	bounds.y = p.y;
}

void
UIElement::ComputeComposite ()
{
	flags &= ~COMPOSITE_MASK;

	if (GetRenderCacheMode ())
		flags |= (COMPOSITE_CACHE | COMPOSITE_TRANSFORM);

	if (opacityMask)
		flags |= COMPOSITE_OPACITY_MASK;

	if (IS_TRANSLUCENT (GetOpacity ()))
		flags |= COMPOSITE_OPACITY;

	if (GetRenderEffect ())
		flags |= (COMPOSITE_EFFECT | COMPOSITE_TRANSFORM);

	if (GetClip ())
		flags |= COMPOSITE_CLIP;

	if (flags & RENDER_PROJECTION)
		flags |= COMPOSITE_TRANSFORM;

	// optimization: apply local opacity at the transform stage when
	// no effect needs to be rendered
	if (flags & COMPOSITE_TRANSFORM) {
		if ((flags & COMPOSITE_EFFECT) == 0)
			flags &= ~COMPOSITE_OPACITY;
	}
}

void
UIElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && prop->GetId () == UIElement::RenderTransformProperty) {
		UpdateTransform ();
	}
	else if (prop && prop->GetId () == UIElement::ClipProperty) {
		InvalidateClip ();
	}
	else if (prop && prop->GetId () == UIElement::OpacityMaskProperty) {
		InvalidateMask ();
	}
	else if (prop && prop->GetId () == UIElement::EffectProperty) {
		InvalidateEffect ();
	}
	else if (prop && prop->GetId () == UIElement::ProjectionProperty) {
		UpdateProjection ();
	}
	else if (prop && prop->GetId () == UIElement::CacheModeProperty) {
		InvalidateCacheMode ();
	}

	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
}

void 
UIElement::CacheInvalidateHint () 
{
}

void
UIElement::SetVisualParent (UIElement *visual_parent)
{
	this->visual_parent = visual_parent;
	SetIsAttached (visual_parent && visual_parent->IsAttached());
}

void
UIElement::SetSubtreeObject (DependencyObject *value)
{
	subtree_object = value;
}

void
UIElement::ElementRemoved (UIElement *item)
{
	// Invalidate ourself in the size of the item's subtree
	Invalidate (item->GetSubtreeBounds());
	item->SetVisualParent (NULL);
	item->SetIsLoaded (false);
	item->SetIsAttached (false);
	item->SetMentor (NULL);

	Rect emptySlot (0,0,0,0);
	LayoutInformation::SetLayoutSlot (item, &emptySlot);
	item->ClearValue (LayoutInformation::LayoutClipProperty);

	InvalidateMeasure ();

	providers.inherited->ClearInheritedPropertiesOnRemovingFromTree (item);
}

void
UIElement::ElementAdded (UIElement *item)
{
	item->SetVisualParent (this);
	item->UpdateTotalRenderVisibility ();
	item->UpdateTotalHitTestVisibility ();
	//item->UpdateBounds (true);
	item->Invalidate ();
	
	providers.inherited->PropagateInheritedPropertiesOnAddingToTree (item);
	item->SetIsAttached (IsAttached ());
	item->SetIsLoaded (IsLoaded ());
	DependencyObject *o = this;
	while (o && !o->Is (Type::FRAMEWORKELEMENT))
		o = o->GetMentor ();
	item->SetMentor (o);

	UpdateBounds (true);
	
	InvalidateMeasure ();
	ClearValue (LayoutInformation::LayoutClipProperty);
	ClearValue (LayoutInformation::PreviousConstraintProperty);
	item->SetRenderSize (Size (0,0));
	item->UpdateTransform ();
	item->UpdateProjection ();
	item->InvalidateMeasure ();
	item->InvalidateArrange ();
	if (item->HasFlag (DIRTY_SIZE_HINT) || item->ReadLocalValue (LayoutInformation::LastRenderSizeProperty))
		item->PropagateFlagUp (DIRTY_SIZE_HINT);
}

void
UIElement::InvalidateMeasure ()
{
	dirty_flags |= DirtyMeasure;
	PropagateFlagUp (DIRTY_MEASURE_HINT);
	GetTimeManager()->NeedRedraw ();
}

void
UIElement::InvalidateArrange ()
{
	dirty_flags |= DirtyArrange;
	PropagateFlagUp (DIRTY_ARRANGE_HINT);
	GetTimeManager()->NeedRedraw ();
}

void
UIElement::DoMeasureWithError (MoonError *error)
{
	Size *last = LayoutInformation::GetPreviousConstraint (this);
	UIElement *parent = GetVisualParent ();
	Size infinite (INFINITY, INFINITY);

	if (!IsAttached () && !last && !parent && IsLayoutContainer ()) {
		last = &infinite;
	}
	
      	if (last) {
		Size previous_desired = GetDesiredSize ();

		// This will be a noop on non layout elements
		MeasureWithError (*last, error);
		
		if (previous_desired == GetDesiredSize ())
		    return;
	}
	
	// a canvas doesn't care about the child size changing like this
	if (parent)
		parent->InvalidateMeasure ();

	dirty_flags &= ~DirtyMeasure;
}

void
UIElement::DoArrangeWithError (MoonError *error)
{
	Value *lastVal = ReadLocalValue (LayoutInformation::LayoutSlotProperty);
	Rect *last = Value::IsNull (lastVal) ? NULL : lastVal->AsRect ();
	Size previous_render = Size ();
	UIElement *parent = GetVisualParent ();
	Rect viewport;

	if (!parent) {
		Size desired = Size ();
		Size available = Size ();
		Surface *surface = GetDeployment ()->GetSurface ();

		if (IsLayoutContainer ()) {
			desired = GetDesiredSize ();
			if (IsAttached () && surface->IsTopLevel (this) && !GetParent ()) {
				Size *measure = LayoutInformation::GetPreviousConstraint (this);
				if (measure)
					desired = desired.Max (*LayoutInformation::GetPreviousConstraint (this));
				else 
					desired = Size (surface->GetWindow ()->GetWidth (), surface->GetWindow ()->GetHeight ());
			}
		} else {
			FrameworkElement *fe = (FrameworkElement*)this;
			desired = Size (fe->GetActualWidth (), fe->GetActualHeight ());
		}
		
		viewport = Rect (Canvas::GetLeft (this),
				 Canvas::GetTop (this), 
				 desired.width, desired.height);

		last = &viewport;
	}

	if (last) {
		ArrangeWithError (*last, error);
	} else {
		if (parent)
			parent->InvalidateArrange ();
	}
}

bool 
UIElement::InsideClip (cairo_t *cr, double x, double y)
{
	Geometry* clip;
	bool inside = true;
	double nx = x;
	double ny = y;

	clip = GetClip ();
	if (!clip)
		return true;

	TransformPoint (&nx, &ny);

	if (!clip->GetBounds ().PointInside (nx, ny))
		return false;

	cairo_save (cr);
	cairo_new_path (cr);

	clip->Draw (cr);
	inside = cairo_in_fill (cr, nx, ny);

	cairo_restore (cr);

	return inside;
}

bool
UIElement::InsideObject (cairo_t *cr, double x, double y)
{
	return InsideClip (cr, x, y);
}

int
UIElement::AddHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor, bool invoke_data_dtor_on_destroy, bool handledEventsToo)
{
	int rv = DependencyObject::AddHandler (event_id, handler, data, data_dtor, invoke_data_dtor_on_destroy, handledEventsToo);
	if (IsLoaded () && event_id == UIElement::LoadedEvent) {
		GetDeployment ()->AddAllLoadedHandlers (this, true);// (this, FindHandlerToken (UIElement::LoadedEvent, handler, data));
	}
	return rv;
}

void
UIElement::PropagateFlagUp (UIElementFlags flag)
{
	SetFlag (flag);
	UIElement *e = this;
	while ((e = e->GetVisualParent ()) && !e->HasFlag (flag)) {
		e->SetFlag (flag);
	}
}

int
UIElement::RemoveHandler (int event_id, EventHandler handler, gpointer data)
{
	int token = FindHandlerToken (event_id, handler, data);

	if (token != -1)
		RemoveHandler (event_id, token);

	return token;
}

void
UIElement::RemoveHandler (int event_id, int token)
{
	if (event_id == UIElement::LoadedEvent)
		Deployment::GetCurrent()->RemoveLoadedHandler (this, token);
	DependencyObject::RemoveHandler (event_id, token);
}


bool
UIElement::Focus (bool recurse)
{
	return false;
}

//
// Queues the invalidate for the current region, performs any 
// updates to the RenderTransform (optional) and queues a 
// new redraw with the new bounding box
//
void
UIElement::FullInvalidate (bool rendertransform)
{
	Invalidate ();
	if (rendertransform) {
		UpdateTransform ();
		UpdateProjection ();
	}
	UpdateBounds (true /* force an invalidate here, even if the bounds don't change */);
}

void
UIElement::Invalidate (Rect r)
{
	if (!GetRenderVisible() || IS_INVISIBLE(total_opacity))
		return;

#ifdef DEBUG_INVALIDATE
	printf ("Requesting invalidate for object %p %s (%s) at %f %f - %f %f\n", 
		this, GetName(), GetTypeName(),
		r.x, r.y, 
		r.w, r.h);
#endif


	if (IsAttached ()) {
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyInvalidate);

		InvalidateBitmapCache ();

		if (RenderToIntermediate ())
			dirty_region->Union (GetSubtreeBounds ());
		else
			dirty_region->Union (r);

		GetTimeManager()->NeedRedraw ();

		Emit (InvalidatedEvent);
	}
}

void
UIElement::Invalidate (Region *region)
{
	if (!GetRenderVisible () || IS_INVISIBLE (total_opacity))
		return;

	if (IsAttached ()) {
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyInvalidate);

		InvalidateBitmapCache ();

		if (RenderToIntermediate ())
			dirty_region->Union (GetSubtreeBounds ());
		else
			dirty_region->Union (region);

		GetTimeManager()->NeedRedraw ();

		Emit (InvalidatedEvent);
	}
}

void
UIElement::Invalidate ()
{
	Invalidate (GetBounds ());
}

/*
void
UIElement::InvalidatePaint ()
{
	Invalidate ();
}
*/

void
UIElement::InvalidateParent (Rect r)
{
	if (GetVisualParent ())
		GetVisualParent ()->Invalidate (r);
	else if (IsAttached ())
		GetDeployment ()->GetSurface ()->Invalidate (r);
}

void
UIElement::InvalidateSubtreePaint ()
{
	Invalidate (GetSubtreeBounds ());
}

void
UIElement::InvalidateClip ()
{
	InvalidateParent (GetSubtreeBounds ());
	UpdateBounds (true);
	ComputeComposite ();
}

void
UIElement::InvalidateMask ()
{
	InvalidateParent (GetSubtreeBounds ());
	ComputeComposite ();
}

void
UIElement::InvalidateVisibility ()
{
	UpdateTotalRenderVisibility ();
	InvalidateParent (GetSubtreeBounds ());
	ComputeComposite ();
}

void
UIElement::InvalidateEffect ()
{
	Effect    *effect = GetRenderEffect ();
	Thickness old_effect_padding = effect_padding;

	if (effect)
		effect_padding = effect->Padding ();
	else
		effect_padding = Thickness (0);

	InvalidateParent (GetSubtreeBounds ());

	if (old_effect_padding != effect_padding)
		UpdateBounds ();

	ComputeComposite ();
}

void
UIElement::InvalidateBitmapCache ()
{
	if (bitmap_cache) {
		if (!GetDeployment ()->IsShuttingDown ())
			GetDeployment ()->GetSurface ()->RemoveGPUSurface (bitmap_cache_size);
		bitmap_cache->unref ();
		bitmap_cache = NULL;
		bitmap_cache_size = 0;
	}
}

void
UIElement::InvalidateCacheMode ()
{
	UpdateTransform ();

	InvalidateBitmapCache ();
	InvalidateSubtreePaint ();

	ComputeComposite ();
}

/*
void
UIElement::InvalidateIntrisicSize ()
{
	InvalidateMeasure ();
	InvalidateArrange ();
	UpdateBounds (true);
}
*/

void
UIElement::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	uielement_list->Prepend (new UIElementNode (this));
}

void
UIElement::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

void
UIElement::FindElementsInHostCoordinates_p (Point p, HitTestCollection *uielement_list)
{
	List *list = new List ();
	cairo_t *ctx = measuring_context_create ();
	
	FindElementsInHostCoordinates (ctx, p, list);
	
	UIElementNode *node = (UIElementNode *) list->First ();
	while (node) {
		uielement_list->Add (Value (node->uielement));
		node = (UIElementNode *) node->next;
	}
	
	delete list;
	measuring_context_destroy (ctx);
}


void
UIElement::FindElementsInHostCoordinates (cairo_t *cr, Point P, List *uielement_list)
{
	uielement_list->Prepend (new UIElementNode (this));
}


void
UIElement::FindElementsInHostCoordinates_r (Rect r, HitTestCollection *uielement_list)
{
	List *list = new List ();
	cairo_t *ctx = measuring_context_create ();
	
	FindElementsInHostCoordinates (ctx, r, list);
	
	UIElementNode *node = (UIElementNode *) list->First ();
	while (node) {
		uielement_list->Add (Value (node->uielement));
		node = (UIElementNode *) node->next;
	}
	
	delete list;
	measuring_context_destroy (ctx);
}

void
UIElement::FindElementsInHostCoordinates (cairo_t *cr, Rect r, List *uielement_list)
{
	uielement_list->Prepend (new UIElementNode (this));
}

bool
UIElement::EmitKeyDown (MoonKeyEvent *event)
{
	if (HasHandlers (KeyDownEvent))
		return Emit (KeyDownEvent, new KeyEventArgs (event));
	else
		return false;
}

bool
UIElement::EmitKeyUp (MoonKeyEvent *event)
{
	if (HasHandlers (KeyUpEvent))
		return Emit (KeyUpEvent, new KeyEventArgs (event));
	else
		return false;
}

bool
UIElement::EmitGotFocus ()
{
	if (HasHandlers (GotFocusEvent))
		return Emit (GotFocusEvent, new RoutedEventArgs (this));
	else
		return false;
}

bool
UIElement::EmitLostFocus ()
{
	if (HasHandlers (LostFocusEvent))
		return Emit (LostFocusEvent, new RoutedEventArgs (this));
	else
		return false;
}

bool
UIElement::EmitLostMouseCapture ()
{
	if (HasHandlers (LostMouseCaptureEvent)) {
		MouseEventArgs *e = MoonUnmanagedFactory::CreateMouseEventArgs ();
		e->SetSource (this);
		return Emit (LostMouseCaptureEvent, e);
	}
	else {
		return false;
	}
}

bool
UIElement::CaptureMouse ()
{
	if (!IsAttached ())
		return false;

	return GetDeployment ()->GetSurface ()->SetMouseCapture (this);
}

void
UIElement::ReleaseMouseCapture ()
{
	if (!IsAttached ())
		return;

	GetDeployment ()->GetSurface ()->ReleaseMouseCapture (this);
}

void
UIElement::DoRender (Context *ctx, Region *parent_region)
{
	Region *region;

	if (ctx->IsImmutable ())
		return;

	if (RenderToIntermediate ()) {
		region = new Region (GetSubtreeExtents ().Transform (&cache_xform).RoundOut ());
	}
	else {
		region = new Region (GetSubtreeExtents ().Transform (&render_xform).Transform (ctx).RoundOut ());
		region->Intersect (parent_region);
	}

	if (!GetRenderVisible() || IS_INVISIBLE (total_opacity) || region->IsEmpty ()) {
		delete region;
		return;
	}

#if OCCLUSION_CULLING_STATS
	GetDeployment ()->GetSurface ()->uielements_rendered_with_painters ++;
#endif

	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);

	PreRender (ctx, region, false);

	if (ctx->IsMutable ()) {
		cairo_t *cr = ctx->Push (Context::Cairo ());

		Render (cr, region);
		ctx->Pop ();
	}

	PostRender (ctx, region, false);

	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);

	delete region;
}

bool
UIElement::UseOcclusionCulling ()
{
	// for now the only things that drop us out of the occlusion
	// culling pass for a subtree are projections and effects.
	if (GetRenderEffect ()) return FALSE;
	if (GetRenderProjection ()) return FALSE;
	if (GetRenderCacheMode ()) return FALSE;
	if (flags & UIElement::RENDER_PROJECTION) return FALSE;

	return TRUE;
}

bool
UIElement::RenderToIntermediate ()
{
	if (GetRenderEffect ()) return TRUE;
	if (GetRenderProjection ()) return TRUE;
	if (GetRenderCacheMode ()) return TRUE;
	if (flags & UIElement::RENDER_PROJECTION) return TRUE;

	return FALSE;
}

void
UIElement::FrontToBack (Region *surface_region, List *render_list)
{
	if (surface_region->RectIn (GetSubtreeBounds().RoundOut()) == CAIRO_REGION_OVERLAP_OUT)
		return;

	double local_opacity = GetOpacity ();

	if (!GetRenderVisible ()
	    || IS_INVISIBLE (local_opacity))
		return;

	if (!UseOcclusionCulling ()) {
		Region *self_region;

		if (RenderToIntermediate ()) {
			self_region = new Region (GetSubtreeExtents ().Transform (&cache_xform).RoundOut ());
		}
		else {
			self_region = new Region (surface_region);
			self_region->Intersect (GetLocalBounds ().RoundOut ());
		}

		// we need to include our children in this one, since
		// we'll be rendering them in the PostRender method.
		if (!self_region->IsEmpty())
			render_list->Prepend (new RenderNode (this, self_region, true,
							      UIElement::CallPreRender, UIElement::CallPostRender));
		// don't remove the region from surface_region because
		// there are likely holes in it
		return;
	}

	Region *region;
	bool delete_region;
	bool can_subtract_self;
	
	if (!GetClip ()
	    && !GetOpacityMask ()
	    && !IS_TRANSLUCENT (GetOpacity ())) {
		region = surface_region;
		delete_region = false;
		can_subtract_self = true;
	}
	else {
		region = new Region (surface_region);
		delete_region = true;
		can_subtract_self = false;
	}

	RenderNode *cleanup_node = new RenderNode (this, NULL, false, NULL, UIElement::CallPostRender);
	
	render_list->Prepend (cleanup_node);

	Region *self_region = new Region (region);

	VisualTreeWalker walker (this, ZReverse, false);
	while (UIElement *child = walker.Step ())
		child->FrontToBack (region, render_list);

	if (!GetOpacityMask () && !IS_TRANSLUCENT (local_opacity)) {
		delete self_region;
		if (GetRenderBounds().IsEmpty ()) {  // empty bounds mean that this element draws nothing itself
			self_region = new Region ();
		}
		else {
			self_region = new Region (region);
			self_region->Intersect (GetRenderBounds().RoundOut ()); // note the RoundOut
		}
	} else {
		self_region->Intersect (GetLocalBounds().RoundOut ()); // note the RoundOut
	}

	if (render_list->First() == cleanup_node) {
		// our children (if we had any) didn't intersect
		// the region.  so there's no need for the cleanup
		// node at all.
		render_list->Remove (render_list->First());

		if (self_region->IsEmpty()) {
			/* we don't intersect the surface region either, so just bail */
			delete self_region;
			if (delete_region)
				delete region;
			return;
		}
		else {
			// we intersect the surface region, so add a
			// single node that does all the work
			render_list->Prepend (new RenderNode (this, self_region, true, UIElement::CallPreRender, UIElement::CallPostRender));
		}
	}
	else {
		// our children intersected the region, so prepend the
		// prerender/render call here.
		render_list->Prepend (new RenderNode (this, self_region, !self_region->IsEmpty(), UIElement::CallPreRender, NULL));
	}

	if (!self_region->IsEmpty()) {
		if (((absolute_xform.yx == 0 && absolute_xform.xy == 0) /* no skew/rotation */
		     || (absolute_xform.xx == 0 && absolute_xform.yy == 0)) /* allow 90 degree rotations */
		    && can_subtract_self)
			region->Subtract (GetCoverageBounds ());
	}

	if (delete_region)
		delete region;
}

void
UIElement::PreRender (Context *ctx, Region *region, bool skip_children)
{
	if (flags & COMPOSITE_TRANSFORM) {
		Rect r = GetSubtreeExtents ().Transform (&cache_xform).GrowBy (effect_padding);

		ctx->Push (Context::Group (r));
		ctx->Push (Context::AbsoluteTransform (cache_xform));
	}
	else {
		ctx->Push (Context::Transform (render_xform));
	}

	if (flags & COMPOSITE_CLIP) {
		Rect              r = GetSubtreeExtents ().Transform (ctx).GrowBy (effect_padding);
		Geometry          *clip = GetClip ();
		cairo_t           *cr = ctx->Push (Context::Cairo (Rect ()));
		cairo_path_t      *path;
		cairo_rectangle_t rect;

		clip->Draw (cr);
		cairo_identity_matrix (cr);
		path = cairo_copy_path (cr);
		ctx->Pop ();

		// we need this check because ::PreRender can (and
		// will) be called for elements with empty regions.
		// 
		// NOTE: we cannot safely restrict to the region
		// when doing occlusion culling since our region
		// may have been culled in that case
		if (!region->IsEmpty () && !skip_children)
			r = r.Intersection (region->GetExtents ());

		if (cairo_path_is_rectangle (path, &rect)) {
			Rect box = Rect (rect.x,
					 rect.y,
					 rect.width,
					 rect.height);

			ctx->Push (Context::Clip (box.Intersection (r)));
		}
		else {
			Rect bounds = clip->GetBounds ().Transform (ctx).Intersection (r);

			if (bounds.IsEmpty ())
				ctx->Push (Context::Clip ());
			else
				ctx->Push (Context::Group (bounds));
		}

		cairo_path_destroy (path);
	}

	if (flags & COMPOSITE_EFFECT) {
		Rect r = GetSubtreeExtents ().Transform (ctx).GrowBy (effect_padding);

		ctx->Push (Context::Group (r));
	}

	if (flags & (COMPOSITE_OPACITY | COMPOSITE_OPACITY_MASK)) {
		Rect r = GetSubtreeExtents ().Transform (ctx);

		// we need this check because ::PreRender can (and
		// will) be called for elements with empty regions.
		//
		// The region passed in here is the redraw region
		// intersected with the render bounds of a given
		// element.  For Panels with no width/height specified
		// in the xaml, this region will be empty. (check
		// panel.cpp::FrontToBack - we insert the ::PreRender
		// calling node if either the panel background or any
		// of the children intersect the redraw region.)  We
		// can't clip to the empty region, obviously, as it
		// will keep all descendents from drawing to the
		// screen.
		// 
		// NOTE: we cannot safely restrict to the region
		// when doing occlusion culling since our region
		// may have been culled
		if (!region->IsEmpty () && !skip_children)
			r = r.Intersection (region->GetExtents ());

		if (flags & COMPOSITE_OPACITY)
			ctx->Push (Context::Group (r));

		if (flags & COMPOSITE_OPACITY_MASK)
			ctx->Push (Context::Group (r));
	}

	if (flags & COMPOSITE_CACHE) {
		Rect r = GetSubtreeExtents ().Transform (&cache_xform);

		if (!bitmap_cache) {
			Surface *surface = GetDeployment ()->GetSurface ();

			ctx->Push (Context::Group (r));
			ctx->Push (Context::AbsoluteTransform (cache_xform));

			VisualTreeWalker walker (this, ZForward, false);
			Render (ctx->Push (Context::Cairo ()), region);
			ctx->Pop ();
			while (UIElement *child = walker.Step ())
				child->DoRender (ctx, region);

			ctx->Pop ();
			ctx->Pop (&bitmap_cache);

			bitmap_cache_size = r.RoundOut ().Area () * 4;
			surface->AddGPUSurface (bitmap_cache_size);
		}
 
		ctx->Push (Context::Group (r), bitmap_cache);

		// push empty clip to prevent sub-tree rendering
		ctx->Push (Context::Clip ());
	}

	// subtree bounds clipping
	ctx->Push (Context::Clip (GetSubtreeExtents ().Transform (ctx)));
}

void
UIElement::PostRender (Context *ctx, Region *region, bool skip_children)
{
	if (!skip_children) {
		VisualTreeWalker walker (this, ZForward, false);
		while (UIElement *child = walker.Step ())
			child->DoRender (ctx, region);
	}

	// subtree bounds clipping
	ctx->Pop ();

	if (flags & COMPOSITE_CACHE) {
		MoonSurface *src;
		Rect        r;

		// pop empty clip
		ctx->Pop ();

		r = ctx->Pop (&src);
		ctx->Push (Context::AbsoluteTransform ());

		if (!r.IsEmpty ()) {
			ctx->Blend (src, 1.0, r.x, r.y);

			src->unref ();
		}

		ctx->Pop ();
	}

	if (flags & COMPOSITE_OPACITY_MASK) {
		MoonSurface *surface;
		Rect        r = ctx->Pop (&surface);
 
		ctx->Push (Context::Clip (r));

		if (!r.IsEmpty ()) {
			cairo_surface_t *src = surface->Cairo ();
			cairo_t         *cr = ctx->Push (Context::Cairo ());
			cairo_matrix_t  ctm;
			Point           p = GetOriginPoint ();
			Rect            area = Rect (p.x, p.y, 0.0, 0.0);
			cairo_pattern_t *mask = NULL;

			cairo_get_matrix (cr, &ctm);
			cairo_identity_matrix (cr);
			GetSizeForBrush (cr, &(area.width), &(area.height));
			opacityMask->SetupBrush (cr, area);
			mask = cairo_get_source (cr);
			cairo_pattern_reference (mask);
			cairo_set_source_surface (cr, src, r.x, r.y);
			cairo_set_matrix (cr, &ctm);
			cairo_mask (cr, mask);
			cairo_pattern_destroy (mask);
			cairo_surface_destroy (src);

			ctx->Pop ();

			surface->unref ();
		}

		ctx->Pop ();
	}

	if (flags & COMPOSITE_OPACITY) {
		MoonSurface *src;
		Rect        r = ctx->Pop (&src);

		ctx->Push (Context::AbsoluteTransform ());

		if (!r.IsEmpty ()) {
			ctx->Blend (src, GetOpacity (), r.x, r.y);

			src->unref ();
		}

		ctx->Pop ();
	}

	if (flags & COMPOSITE_EFFECT) {
		Effect      *effect;
		MoonSurface *src;
		Rect        r = ctx->Pop (&src);

		if (!r.IsEmpty () && (effect = GetRenderEffect ())) {
			effect->Render (ctx, src, r.x, r.y);

			src->unref ();
		}
	}

	if (flags & COMPOSITE_CLIP) {
		MoonSurface *surface;
		Rect        r = ctx->Pop (&surface);

		if (!r.IsEmpty ()) {
			cairo_surface_t *src = surface->Cairo ();
			cairo_t         *cr = ctx->Push (Context::Cairo ());

			RenderClipPath (cr);

			cairo_identity_matrix (cr);
			cairo_set_source_surface (cr, src, r.x, r.y);
			cairo_paint (cr);
			cairo_surface_destroy (src);

			ctx->Pop ();

			surface->unref ();
		}
	}

	if (flags & COMPOSITE_TRANSFORM) {
		MoonSurface *src;
		Rect        r;

		ctx->Pop ();
		r = ctx->Pop (&src);

		if (!r.IsEmpty ()) {
			ctx->Project (src,
				      render_projection,
				      GetOpacity (),
				      r.x, r.y);

			src->unref ();
		}
	}
	else {
		ctx->Pop ();
	}

	if (moonlight_flags & RUNTIME_INIT_SHOW_CLIPPING) {
		cairo_t *cr = ctx->Push (Context::Cairo ());

		cairo_save (cr);
		cairo_new_path (cr);
		cairo_transform (cr, &render_xform);
		cairo_set_line_width (cr, 1);
		
		Geometry *geometry = GetClip ();
		if (geometry) {
			geometry->Draw (cr);
			cairo_set_source_rgba (cr, 0.0, 1.0, 1.0, 1.0);
			cairo_stroke (cr);
		}
		
		geometry = LayoutInformation::GetCompositeClip ((FrameworkElement *)this);
		if (geometry) {
			geometry->Draw (cr);
			cairo_set_source_rgba (cr, 0.0, 0.0, 1.0, 1.0);
			cairo_stroke (cr);
			geometry->unref ();
		}

		cairo_restore (cr);

		ctx->Pop ();
	}
	
	if (moonlight_flags & RUNTIME_INIT_SHOW_BOUNDING_BOXES) {
		cairo_t *cr = ctx->Push (Context::Cairo ());

		cairo_save (cr);
		cairo_new_path (cr);
		//RenderClipPath (cr);
		cairo_identity_matrix (cr);
		cairo_set_source_rgba (cr, 1.0, 0.5, 0.2, 1.0);
		cairo_set_line_width (cr, 1);
		cairo_rectangle (cr, bounds.x + .5, bounds.y + .5, bounds.width - .5, bounds.height - .5);
		cairo_stroke (cr);
		cairo_restore (cr);

		ctx->Pop ();
	}
}

void
UIElement::Paint (Context *ctx,  Rect bounds, cairo_matrix_t *xform)
{
	Region region (bounds.RoundOut ());

#if OCCLUSION_CULLING_STATS
	GetDeployment ()->GetSurface ()->uielements_rendered_with_occlusion_culling = 0;
	GetDeployment ()->GetSurface ()->uielements_rendered_with_painters = 0;
#endif

	cairo_matrix_t inverse = layout_xform;

	// reverse the effect of the layout_xform
	cairo_matrix_invert (&inverse);

	if (xform)
		cairo_matrix_multiply (&inverse, &inverse, xform);

	ctx->Push (Context::Transform (inverse));
	DoRender (ctx, &region);
	ctx->Pop ();

#if OCCLUSION_CULLING_STATS
	printf ("%d UIElements rendered front-to-back for: %s(%p)\n", GetDeployment ()->GetSurface ()->uielements_rendered_with_occlusion_culling, GetName (), this);
	printf ("%d UIElements rendered back-to-front for: %s(%p)\n", GetDeployment ()->GetSurface ()->uielements_rendered_with_painters, GetName (), this);
#endif
}

void
UIElement::CallPreRender (Context *ctx, UIElement *element, Region *region, bool skip_children)
{
	element->PreRender (ctx, region, skip_children);
}

void
UIElement::CallPostRender (Context *ctx, UIElement *element, Region *region, bool skip_children)
{
	element->PostRender (ctx, region, skip_children);
}

void
UIElement::Render (cairo_t *cr, Region *region, bool path_only)
{
	/* do nothing by default */
}

void
UIElement::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	g_warning ("UIElement:GetSizeForBrush has been called. The derived class %s should have overridden it.",
		   GetTypeName ());
	*height = *width = 0.0;
}

TimeManager *
UIElement::GetTimeManager ()
{
	Deployment *deployment = GetDeployment();
	Surface *surface;

	if (!(deployment = GetDeployment()))
		return NULL;
	if (!(surface = deployment->GetSurface()))
		return NULL;
	return surface->GetTimeManager ();
}

GeneralTransform *
UIElement::GetTransformToUIElementWithError (UIElement *to_element, MoonError *error)
{
	/* walk from this up to the root.  if we hit null before we hit the toplevel, it's an error */
	UIElement *visual = this;
	bool ok = false;

	if (visual && IsAttached ()) {
		while (visual) {
			if (GetDeployment ()->GetSurface()->IsTopLevel (visual))
				ok = true;
			visual = visual->GetVisualParent ();
		}
	}

	if (!ok || (to_element && !to_element->IsAttached ())) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001,
				   "visual");
		return NULL;
	}

	if (to_element && !GetDeployment ()->GetSurface()->IsTopLevel (to_element)) {
		/* if @to_element is specified we also need to make sure there's a path to the root from it */
		ok = false;
		visual = to_element->GetVisualParent ();
		if (visual && to_element->IsAttached ()) {
			while (visual) {
				if (GetDeployment ()->GetSurface()->IsTopLevel (visual))
					ok = true;
				visual = visual->GetVisualParent ();
			}
		}

		if (!ok) {
			MoonError::FillIn (error, MoonError::ARGUMENT, 1001,
					   "visual");
			return NULL;
		}
	}
	
	double result[16];
	// A = From, B = To, M = what we want
	// A = M * B
	// => M = A * inv (B)
	if (to_element) {
		double inverse[16];

		Matrix3D::Inverse (inverse, to_element->absolute_projection);
		Matrix3D::Multiply (result, absolute_projection, inverse);
	}
	else {
		Matrix3D::Init (result, absolute_projection);
	}

	if (Matrix3D::Is2DAffine(result)) {
		cairo_matrix_t _matrix;
		
#define M(row, col) result[col * 4 + row]
		_matrix.xx = M (0, 0);
		_matrix.yx = M (1, 0);
		_matrix.xy = M (0, 1);
		_matrix.yy = M (1, 1);
		_matrix.x0 = M (0, 3);
		_matrix.y0 = M (1, 3);
#undef M

		Matrix *matrix = new Matrix (&_matrix);

		MatrixTransform *transform = MoonUnmanagedFactory::CreateMatrixTransform ();
		transform->SetValue (MatrixTransform::MatrixProperty, matrix);
		matrix->unref ();

		return transform;
	}

	InternalTransform *internal = MoonUnmanagedFactory::CreateInternalTransform ();
	internal->SetTransform (result);

	return internal;
}

void
UIElement::TransformPoint (double *x, double *y)
{
	double inverse[16];

	if (Matrix3D::Inverse (inverse, absolute_projection)) {
		double p[4];

		//
		// use value for Z that gives us zero when
		// multiplied by inverse
		//
#define M(row, col) inverse[col * 4 + row]
		p[0] = *x;
		p[1] = *y;
		p[2] = -(M (2, 0) * p[0] +
			 M (2, 1) * p[1] +
			 M (2, 3) * 1.0) / M (2, 2);
		p[3] = 1.0;
#undef M

		Matrix3D::TransformPoint (p, inverse, p);

		*x = p[0] / p[3];
		*y = p[1] / p[3];
	}
}

#define EFFECT_FLAGS RUNTIME_INIT_INTERMEDIATE_SURFACES

Effect *
UIElement::GetRenderEffect ()
{
	return (moonlight_flags & EFFECT_FLAGS) == (EFFECT_FLAGS) ? GetEffect () : NULL;
}

#define PROJECTION_FLAGS RUNTIME_INIT_INTERMEDIATE_SURFACES

Projection *
UIElement::GetRenderProjection ()
{
	return (moonlight_flags & PROJECTION_FLAGS) == (PROJECTION_FLAGS) ? GetProjection () : NULL;
}

#define CACHE_MODE_FLAGS RUNTIME_INIT_INTERMEDIATE_SURFACES

CacheMode *
UIElement::GetRenderCacheMode ()
{
	return (moonlight_flags & CACHE_MODE_FLAGS) == (CACHE_MODE_FLAGS) ? GetCacheMode () : NULL;
}

};
