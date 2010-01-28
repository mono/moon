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
#include "provider.h"

//#define DEBUG_INVALIDATE 0
#define MIN_FRONT_TO_BACK_COUNT 25

UIElement::UIElement ()
{
	SetObjectType (Type::UIELEMENT);

	visual_level = 0;
	visual_parent = NULL;
	subtree_object = NULL;
	opacityMask = NULL;
	
	flags = UIElement::RENDER_VISIBLE | UIElement::HIT_TEST_VISIBLE;

	hidden_desire = Size (-INFINITY, -INFINITY);
	bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);
	cairo_matrix_init_identity (&layout_xform);
	cairo_matrix_init_identity (&local_xform);

	emitting_loaded = false;
	dirty_flags = DirtyMeasure;
	up_dirty_node = down_dirty_node = NULL;
	force_invalidate_of_new_bounds = false;
	dirty_region = new Region ();

	desired_size = Size (0, 0);
	render_size = Size (0, 0);
	
	ComputeLocalTransform ();
	ComputeTotalRenderVisibility ();
	ComputeTotalHitTestVisibility ();
}

UIElement::~UIElement()
{
	delete dirty_region;
}

bool
UIElement::IsSubtreeLoaded (UIElement *element)
{
	while (element && !element->IsLoaded ())
		element = element->GetVisualParent ();
	return element;
}

void
UIElement::Dispose()
{
	TriggerCollection *triggers = GetTriggers ();
	
	if (triggers != NULL) {
		for (int i = 0; i < triggers->GetCount (); i++)
			triggers->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
	}
	
	if (!IsDisposed ()) {
		VisualTreeWalker walker (this);
		while (UIElement *child = walker.Step ())
			child->SetVisualParent (NULL);
	}
	
	if (subtree_object) {
		subtree_object->unref ();
		subtree_object = NULL;
	}

	DependencyObject::Dispose();
}

void
UIElement::ClearWalkedForLoaded ()
{
	UIElement *parent = GetVisualParent ();
	if (parent)
		parent->ClearWalkedForLoaded ();
	flags &= ~UIElement::WALKED_FOR_LOADED;
}

void
UIElement::SetSurface (Surface *s)
{
	if (GetSurface() == s)
		return;

	if (s == NULL && GetSurface()) {
		/* we're losing our surface, delete ourselves from the dirty list if we're on it */
		GetSurface()->RemoveDirtyElement (this);
	}

	if (subtree_object != NULL && subtree_object->Is(Type::UIELEMENT))
		subtree_object->SetSurface (s);

	DependencyObject::SetSurface (s);
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
	cairo_set_matrix (cr, &absolute_xform);

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
		if (args->GetNewValue()->AsInt32() == VisibilityVisible)
			flags |= UIElement::RENDER_VISIBLE;
		else
			flags &= ~UIElement::RENDER_VISIBLE;

		InvalidateVisibility ();
		InvalidateMeasure ();
		if (GetVisualParent ())
			GetVisualParent ()->InvalidateMeasure ();
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
			for (int i = 0; i < triggers->GetCount (); i++)
				triggers->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
		}

		if (args->GetNewValue()) {
			// set the new ones
			TriggerCollection *triggers = args->GetNewValue()->AsTriggerCollection();
			for (int i = 0; i < triggers->GetCount (); i++)
				triggers->GetValueAt (i)->AsEventTrigger ()->SetTarget (this);
		}
	} else if (args->GetId () == UIElement::UseLayoutRoundingProperty) {
		InvalidateMeasure ();
		InvalidateArrange ();
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
UIElement::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col == GetTriggers ()) {
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
		case CollectionChangedActionClearing:
			for (int i = 0; i < col->GetCount (); i++)
				col->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
			break;
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

	Surface *surface = GetSurface ();
	if (surface)
		surface->AddDirtyElement (this, DirtyBounds);

	force_invalidate_of_new_bounds |= force_redraw;
}

void
UIElement::UpdateTotalRenderVisibility ()
{
	Surface *surface = GetSurface ();
	if (surface)
		surface->AddDirtyElement (this, DirtyRenderVisibility);
}

void
UIElement::UpdateTotalHitTestVisibility ()
{
	VisualTreeWalker walker (this);
	while (UIElement *child = walker.Step ())
		child->UpdateTotalHitTestVisibility ();

	if (GetSurface())
		GetSurface ()->AddDirtyElement (this, DirtyHitTestVisibility);
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
	if (GetSurface()) {
		GetSurface()->AddDirtyElement (this, DirtyLocalTransform);
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
		return;
	}

	UpdateBounds ();
}

void
UIElement::ComputeTransform ()
{
	cairo_matrix_t old = absolute_xform;
	cairo_matrix_init_identity (&absolute_xform);

	if (GetVisualParent () != NULL) {
		absolute_xform = GetVisualParent ()->absolute_xform;
	} else if (GetParent () != NULL && GetParent ()->Is (Type::POPUP)) {  
		// FIXME we shouldn't be examing a subclass type here but we'll do this
		// for now to get popups in something approaching the right place while
		// we figure out a cleaner way to handle it long term.
		Popup *popup = (Popup *)GetParent ();
		absolute_xform = popup->absolute_xform;
		cairo_matrix_translate (&absolute_xform, popup->GetHorizontalOffset (), popup->GetVerticalOffset ());
	}

	cairo_matrix_multiply (&absolute_xform, &layout_xform, &absolute_xform);
	cairo_matrix_multiply (&absolute_xform, &local_xform, &absolute_xform);
	
	if (moonlight_flags & RUNTIME_INIT_USE_UPDATE_POSITION)
		TransformBounds (&old, &absolute_xform);
	else {
		UpdateBounds ();
	}
}

void
UIElement::ComputeBounds ()
{
	g_warning ("UIElement:ComputeBounds has been called. The derived class %s should have overridden it.",
		   GetTypeName ());
}

void
UIElement::ShiftPosition (Point p)
{
	bounds.x = p.x;
	bounds.y = p.y;
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
	
	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
}

void 
UIElement::CacheInvalidateHint () 
{
	VisualTreeWalker walker (this);
	while (UIElement *child = walker.Step ())
		child->CacheInvalidateHint ();
}

void
UIElement::SetVisualParent (UIElement *visual_parent)
{
	this->visual_parent = visual_parent;

	if (visual_parent && visual_parent->GetSurface () != GetSurface())
		SetSurface (visual_parent->GetSurface());
}

void
UIElement::SetSubtreeObject (DependencyObject *value)
{
	if (subtree_object == value)
	  return;

	if (subtree_object)
	  subtree_object->unref ();

	subtree_object = value;

	if (subtree_object)
	  subtree_object->ref ();
}

void
UIElement::ElementRemoved (UIElement *item)
{
	// Invalidate ourself in the size of the item's subtree
	Invalidate (item->GetSubtreeBounds());

	if (GetSurface ())
		GetSurface()->RemoveDirtyElement (item);
	item->SetVisualParent (NULL);
	item->CacheInvalidateHint ();
	item->ClearLoaded ();

	Rect emptySlot (0,0,0,0);
	LayoutInformation::SetLayoutSlot (item, &emptySlot);
	item->ClearValue (LayoutInformation::LayoutClipProperty);

	InvalidateMeasure ();
}

void
UIElement::ElementAdded (UIElement *item)
{
	ClearWalkedForLoaded ();
		
	item->SetVisualLevel (GetVisualLevel() + 1);
	item->SetVisualParent (this);
	item->UpdateTotalRenderVisibility ();
	item->UpdateTotalHitTestVisibility ();
	//item->UpdateBounds (true);
	item->Invalidate ();

	if (0 != (flags & (UIElement::IS_LOADED | UIElement::PENDING_LOADED))) {
		InheritedPropertyValueProvider::PropagateInheritedPropertiesOnAddingToTree (item);

		bool post = false;

		item->WalkTreeForLoadedHandlers (&post, true, false);

		if (post)
			Deployment::GetCurrent()->PostLoaded ();
	}

	UpdateBounds (true);
	
	InvalidateMeasure ();
	ClearValue (LayoutInformation::LayoutClipProperty);
	ClearValue (LayoutInformation::PreviousConstraintProperty);
	item->SetRenderSize (Size (0,0));
	item->UpdateTransform ();
	item->InvalidateMeasure ();
	item->InvalidateArrange ();
	if (item->ReadLocalValue (LayoutInformation::LastRenderSizeProperty))
		PropagateFlagUp (DIRTY_SIZE_HINT);
}

void
UIElement::InvalidateMeasure ()
{
	dirty_flags |= DirtyMeasure;
	PropagateFlagUp (DIRTY_MEASURE_HINT);
}

void
UIElement::InvalidateArrange ()
{
	dirty_flags |= DirtyArrange;
	PropagateFlagUp (DIRTY_ARRANGE_HINT);
}

void
UIElement::DoMeasure ()
{
	Size *last = LayoutInformation::GetPreviousConstraint (this);
	UIElement *parent = GetVisualParent ();
	Size infinite (INFINITY, INFINITY);

	if (!GetSurface () && !last && !parent && IsLayoutContainer ()) {
		last = &infinite;
	}
	
      	if (last) {
		Size previous_desired = GetDesiredSize ();

		// This will be a noop on non layout elements
		Measure (*last);
		
		if (previous_desired == GetDesiredSize ())
		    return;
	}
	
	// a canvas doesn't care about the child size changing like this
	if (parent)
		parent->InvalidateMeasure ();

	dirty_flags &= ~DirtyMeasure;
}

void
UIElement::DoArrange ()
{
	Rect *last = LayoutInformation::GetLayoutSlot (this);
	Size previous_render = Size ();
	UIElement *parent = GetVisualParent ();
	Rect viewport;

	if (!parent) {
		Size desired = Size ();
		Size available = Size ();
		Surface *surface = GetSurface ();

		if (IsLayoutContainer ()) {
			desired = GetDesiredSize ();
			if (surface && surface->IsTopLevel (this) && !GetParent ()) {
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
		Arrange (*last);
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
UIElement::AddHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor)
{
	int rv = DependencyObject::AddHandler (event_id, handler, data, data_dtor);
	if (event_id == UIElement::LoadedEvent) {
		ClearWalkedForLoaded ();
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

#if WALK_METRICS
int walk_count = 0;
#endif

void
UIElement::WalkTreeForLoadedHandlers (bool *post, bool only_unemitted, bool force_walk_up)
{
	List *walk_list = new List();
	List *subtree_list = new List ();

	bool post_loaded = false;
	Deployment *deployment = GetDeployment ();
	Application *application = deployment->GetCurrentApplication ();

	DeepTreeWalker *walker = new DeepTreeWalker (this);

	// we need to make sure to apply the default style to all
	// controls in the subtree
	while (UIElement *element = (UIElement*)walker->Step ()) {
#if WALK_METRICS
		walk_count ++;
#endif

		if (element->HasBeenWalkedForLoaded ()) {
			walker->SkipBranch ();
			continue;
		}

		if (element->Is(Type::CONTROL)) {
			Control *control = (Control*)element;
			if (!control->default_style_applied) {
				ManagedTypeInfo *key = control->GetDefaultStyleKey ();
				if (key) {
					if (application == NULL)
						g_warning ("attempting to use a null application when applying default style when emitting Loaded event.");
					else
						application->ApplyDefaultStyle (control, key);
				}
			}

			if (!control->GetTemplateRoot () /* we only need to worry about this if the template hasn't been expanded */
			    && control->GetTemplate())
				post_loaded = true; //XXX do we need this? control->ReadLocalValue (Control::TemplateProperty) == NULL;
		}

 		element->flags |= UIElement::PENDING_LOADED;
		element->OnLoaded ();
		if (element->HasHandlers (UIElement::LoadedEvent)) {
			post_loaded = true;
			subtree_list->Prepend (new UIElementNode (element));
		}
		element->SetWalkedForLoaded ();
	}

	if (force_walk_up || !post_loaded || HasHandlers (UIElement::LoadedEvent)) {
		// we need to walk back up to the root to collect all loaded events
		UIElement *parent = this;
		while (parent->GetVisualParent())
			parent = parent->GetVisualParent();
		delete walker;
		walker = new DeepTreeWalker (parent, Logical/*Reverse*/);

		while (UIElement *element = (UIElement*)walker->Step ()) {
#if WALK_METRICS
			walk_count ++;
#endif

			if (element == this) {
				// we already walked this, so add our subtree list here.
				walk_list->Prepend (subtree_list);
				subtree_list->Clear (false);
				walker->SkipBranch ();
			}
			else if (element->HasBeenWalkedForLoaded ()) {
				walker->SkipBranch ();
			}
			else {
				walk_list->Prepend (new UIElementNode (element));
				element->SetWalkedForLoaded ();
			}
		}

		// if we didn't add the subtree's loaded handlers
		// (because somewhere up above the subtree we skipped
		// the branch) add it here.
		if (walk_list->IsEmpty ()) {
			walk_list->Prepend (subtree_list);
			subtree_list->Clear (false);
		}
	}
	else {
		// otherwise we only copy the events from the subtree
		walk_list->Prepend (subtree_list);
		subtree_list->Clear (false);
	}

	while (UIElementNode *ui = (UIElementNode*)walk_list->First ()) {
		// remove it from the walk list
		walk_list->Unlink (ui);

		deployment->AddAllLoadedHandlers (ui->uielement, only_unemitted);

		delete ui;
	}

	if (post)
		*post = post_loaded;

	delete walker;
	delete walk_list;
	delete subtree_list;
}


void
UIElement::OnLoaded ()
{
	flags |= UIElement::IS_LOADED;
	flags &= ~UIElement::PENDING_LOADED;
}

void
UIElement::ClearLoaded ()
{
	UIElement *e = NULL;
	Surface *s = Deployment::GetCurrent ()->GetSurface ();
	if (s->GetFocusedElement () == this)
		s->FocusElement (NULL);
		
	ClearForeachGeneration (UIElement::LoadedEvent);
	ClearWalkedForLoaded ();

	if (!IsLoaded ())
		return;
	
	flags &= ~UIElement::IS_LOADED;

	VisualTreeWalker walker (this);
	while ((e = walker.Step ()))
		e->ClearLoaded ();
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
	if (rendertransform)
		UpdateTransform ();
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


	if (GetSurface ()) {
		GetSurface()->AddDirtyElement (this, DirtyInvalidate);

		dirty_region->Union (r);

		GetSurface()->GetTimeManager()->NeedRedraw ();

		Emit (InvalidatedEvent);
	}
}

void
UIElement::Invalidate (Region *region)
{
	if (!GetRenderVisible () || IS_INVISIBLE (total_opacity))
		return;

	if (GetSurface ()) {
		GetSurface()->AddDirtyElement (this, DirtyInvalidate);

		dirty_region->Union (region);

		GetSurface()->GetTimeManager()->NeedRedraw ();

		Emit (InvalidatedEvent);
	}
}

void
UIElement::Invalidate ()
{
	Invalidate (bounds);
}

/*
void
UIElement::InvalidatePaint ()
{
	Invalidate ();
}
*/

void
UIElement::InvalidateSubtreePaint ()
{
	Invalidate (GetSubtreeBounds ());
}

void
UIElement::InvalidateClip ()
{
	InvalidateSubtreePaint ();
	UpdateBounds (true);
}

void
UIElement::InvalidateMask ()
{
	InvalidateSubtreePaint ();
}

void
UIElement::InvalidateVisibility ()
{
	UpdateTotalRenderVisibility ();
	InvalidateSubtreePaint ();
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
		uielement_list->Add (new Value (node->uielement));
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
		uielement_list->Add (new Value (node->uielement));
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
UIElement::EmitKeyDown (GdkEventKey *event)
{
	return Emit (KeyDownEvent, new KeyEventArgs (event));
}

bool
UIElement::EmitKeyUp (GdkEventKey *event)
{
	return Emit (KeyUpEvent, new KeyEventArgs (event));
}

bool
UIElement::EmitGotFocus ()
{
	return Emit (GotFocusEvent, new RoutedEventArgs (this));
}

bool
UIElement::EmitLostFocus ()
{
	return Emit (LostFocusEvent, new RoutedEventArgs (this));
}

bool
UIElement::EmitLostMouseCapture ()
{
	MouseEventArgs *e = new MouseEventArgs ();
	e->SetSource (this);
	return Emit (LostMouseCaptureEvent, e);
}

bool
UIElement::CaptureMouse ()
{
	Surface *s = GetSurface ();
	if (s == NULL)
		return false;

	return s->SetMouseCapture (this);
}

void
UIElement::ReleaseMouseCapture ()
{
	Surface *s = GetSurface ();
	if (s == NULL)
		return;

	s->ReleaseMouseCapture (this);
}

void
UIElement::DoRender (cairo_t *cr, Region *parent_region)
{
	Region *region = new Region (GetSubtreeBounds ());
	region->Intersect (parent_region);

	if (!GetRenderVisible() || IS_INVISIBLE (total_opacity) || region->IsEmpty ()) {
		delete region;
		return;
	}

#if FRONT_TO_BACK_STATS
	GetSurface()->uielements_rendered_back_to_front ++;
#endif

	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);

	PreRender (cr, region, false);

	Render (cr, region);

	PostRender (cr, region, false);

	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);

	delete region;
}

bool
UIElement::UseBackToFront ()
{
	return VisualTreeWalker (this).GetCount () < MIN_FRONT_TO_BACK_COUNT;
}

void
UIElement::FrontToBack (Region *surface_region, List *render_list)
{
	double local_opacity = GetOpacity ();

	if (surface_region->RectIn (GetSubtreeBounds().RoundOut()) == GDK_OVERLAP_RECTANGLE_OUT)
		return;

	if (!GetRenderVisible ()
	    || IS_INVISIBLE (local_opacity))
		return;

	if (!UseBackToFront ()) {
		Region *self_region = new Region (surface_region);
		self_region->Intersect (GetSubtreeBounds().RoundOut());

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

	VisualTreeWalker walker (this, ZReverse);
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
		self_region->Intersect (GetSubtreeBounds().RoundOut ()); // note the RoundOut
	}

	if (self_region->IsEmpty() && render_list->First() == cleanup_node) {
		/* we don't intersect the surface region, and none of
		   our children did either, remove the cleanup node */
		render_list->Remove (render_list->First());
		delete self_region;
		if (delete_region)
			delete region;
		return;
	}

	render_list->Prepend (new RenderNode (this, self_region, !self_region->IsEmpty(), UIElement::CallPreRender, NULL));

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
UIElement::PreRender (cairo_t *cr, Region *region, bool skip_children)
{
	double local_opacity = GetOpacity ();

	cairo_save (cr);

	cairo_set_matrix (cr, &absolute_xform);
	RenderClipPath (cr);

	if (opacityMask || IS_TRANSLUCENT (local_opacity)) {
		Rect r = GetSubtreeBounds ().RoundOut();
		cairo_identity_matrix (cr);

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
		if (!region->IsEmpty()) {
			region->Draw (cr);
			cairo_clip (cr);
		}
		r.Draw (cr);
		cairo_clip (cr);
	}
	cairo_set_matrix (cr, &absolute_xform);

	/*
	if (ClipToExtents ()) {
		extents.Draw (cr); 
		cairo_clip (cr);
	}
	*/

	if (IS_TRANSLUCENT (local_opacity))
		cairo_push_group (cr);

	if (opacityMask != NULL)
		cairo_push_group (cr);
}

void
UIElement::PostRender (cairo_t *cr, Region *region, bool front_to_back)
{
	// if we didn't render front to back, then render the children here
	if (!front_to_back) {
		VisualTreeWalker walker (this, ZForward);
		while (UIElement *child = walker.Step ())
			child->DoRender (cr, region);
	}

	double local_opacity = GetOpacity ();

	if (opacityMask != NULL) {
		cairo_pattern_t *data = cairo_pop_group (cr);
		if (cairo_pattern_status (data) == CAIRO_STATUS_SUCCESS) {
			cairo_pattern_t *mask = NULL;
			Point p = GetOriginPoint ();
			Rect area = Rect (p.x, p.y, 0.0, 0.0);
			GetSizeForBrush (cr, &(area.width), &(area.height));
			opacityMask->SetupBrush (cr, area);
			mask = cairo_get_source (cr);
			cairo_pattern_reference (mask);
			cairo_set_source (cr, data);
			cairo_mask (cr, mask);
			cairo_pattern_destroy (mask);
		}
		cairo_pattern_destroy (data);
	}

	if (IS_TRANSLUCENT (local_opacity)) {
		cairo_pattern_t *data = cairo_pop_group (cr);
		if (cairo_pattern_status (data) == CAIRO_STATUS_SUCCESS) {
			cairo_set_source (cr, data);
			cairo_paint_with_alpha (cr, local_opacity);
		}
		cairo_pattern_destroy (data);
	}

	cairo_restore (cr);
	
	if (moonlight_flags & RUNTIME_INIT_SHOW_CLIPPING) {
		cairo_save (cr);
		cairo_new_path (cr);
		cairo_set_matrix (cr, &absolute_xform);
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
		}

		cairo_restore (cr);
	}
	
	if (moonlight_flags & RUNTIME_INIT_SHOW_BOUNDING_BOXES) {
		cairo_save (cr);
		cairo_new_path (cr);
		//RenderClipPath (cr);
		cairo_identity_matrix (cr);
		cairo_set_source_rgba (cr, 1.0, 0.5, 0.2, 1.0);
		cairo_set_line_width (cr, 1);
		cairo_rectangle (cr, bounds.x + .5, bounds.y + .5, bounds.width - .5, bounds.height - .5);
		cairo_stroke (cr);
		cairo_restore (cr);
	}
}

void
UIElement::Paint (cairo_t *ctx,  Region *region, cairo_matrix_t *xform)
{
	// FIXME xform is ignored for now
	if (xform)
		g_warning ("passing a transform to UIElement::Paint is not yet supported");

#if FRONT_TO_BACK_STATS
	uielements_rendered_front_to_back = 0;
	uielements_rendered_back_to_front = 0;
#endif

	bool did_front_to_back = false;
	List *render_list = new List ();

	if (moonlight_flags & RUNTIME_INIT_RENDER_FRONT_TO_BACK) {
		Region *copy = new Region (region);
		FrontToBack (copy, render_list);
		
		if (!render_list->IsEmpty ()) {
			while (RenderNode *node = (RenderNode*)render_list->First()) {
#if FRONT_TO_BACK_STATS
				uielements_rendered_front_to_back ++;
#endif
				node->Render (ctx);

				render_list->Remove (node);
			}

			did_front_to_back = true;
		}

		delete render_list;
		delete copy;
	}

	if (!did_front_to_back) {
		DoRender (ctx, region);
	}

#if FRONT_TO_BACK_STATS
	printf ("UIElements rendered front-to-back for: %s(%p)\n", uielements_rendered_front_to_back, GetName (), this);
	printf ("UIElements rendered back-to-front for: %s(%p)\n", uielements_rendered_back_to_front, GetName (), this);
#endif
}

void
UIElement::CallPreRender (cairo_t *cr, UIElement *element, Region *region, bool front_to_back)
{
	element->PreRender (cr, region, front_to_back);
}

void
UIElement::CallPostRender (cairo_t *cr, UIElement *element, Region *region, bool front_to_back)
{
	element->PostRender (cr, region, front_to_back);
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
	Surface *surface = GetSurface ();
	Deployment *deployment;
	
	if (surface == NULL) {
		deployment = GetDeployment ();
		if (deployment != NULL)
			surface = deployment->GetSurface ();
	}
		
	return surface ? surface->GetTimeManager() : NULL;
}

GeneralTransform *
UIElement::GetTransformToUIElementWithError (UIElement *to_element, MoonError *error)
{
	/* walk from this up to the root.  if we hit null before we hit the toplevel, it's an error */
	UIElement *visual = this;
	bool ok = false;

	if (visual && GetSurface()) {
		while (visual) {
			if (GetSurface()->IsTopLevel (visual))
				ok = true;
			visual = visual->GetVisualParent ();
		}
	}

	if (!ok || (to_element && !to_element->GetSurface ())) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001,
				   "visual");
		return NULL;
	}

	if (to_element && !to_element->GetSurface()->IsTopLevel (to_element)) {
		/* if @to_element is specified we also need to make sure there's a path to the root from it */
		ok = false;
		visual = to_element->GetVisualParent ();
		if (visual && to_element->GetSurface()) {
			while (visual) {
				if (to_element->GetSurface()->IsTopLevel (visual))
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
	
	cairo_matrix_t result;
	// A = From, B = To, M = what we want
	// A = M * B
	// => M = A * inv (B)
	if (to_element) {
		cairo_matrix_t inverse = to_element->absolute_xform;
		cairo_matrix_invert (&inverse);
		cairo_matrix_multiply (&result, &absolute_xform, &inverse);
	}
	else {
		result = absolute_xform;
	}

	Matrix *matrix  = new Matrix (&result);

	MatrixTransform *transform = new MatrixTransform ();
	transform->SetValue (MatrixTransform::MatrixProperty, matrix);
	matrix->unref ();

	return transform;
}

void
UIElement::TransformPoint (double *x, double *y)
{
	cairo_matrix_t inverse = absolute_xform;
	cairo_matrix_invert (&inverse);
	
	cairo_matrix_transform_point (&inverse, x, y);
}

