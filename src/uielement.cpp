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

	bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);

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
	InvalidateArrange ();

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

	if (GetVisualParent () != NULL)
		absolute_xform = GetVisualParent ()->absolute_xform;

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
	item->CacheInvalidateHint ();
	item->SetVisualParent (NULL);
	item->ClearLoaded ();
	InvalidateMeasure ();
}

void
UIElement::ElementAdded (UIElement *item)
{
	item->SetVisualLevel (GetVisualLevel() + 1);
	item->SetVisualParent (this);
	item->UpdateTotalRenderVisibility ();
	item->UpdateTotalHitTestVisibility ();
	//item->UpdateBounds (true);
	item->Invalidate ();
	
	if (0 != (flags & (UIElement::IS_LOADED | UIElement::PENDING_LOADED))) {
		bool delay = false;
		List *load_list = item->WalkTreeForLoaded (&delay);

		// if we're loaded, key the delay state off of the
		// WalkTreeForLoaded call.
		//
		// if we're actually pending loaded, forcibly delay
		// the new element's Loaded firing as well.
		//
		if (0 != (flags & UIElement::PENDING_LOADED))
			delay = true;

		if (delay) {
			PostSubtreeLoad (load_list);
			// PostSubtreeLoad will take care of deleting the list for us.
		}
		else {
			EmitSubtreeLoad (load_list);
			delete load_list;
		}
	}

	UpdateBounds (true);
	InvalidateMeasure ();
	item->UpdateTransform ();
	item->InvalidateMeasure ();
	item->InvalidateArrange ();
}

void
UIElement::InvalidateMeasure ()
{
	dirty_flags |= DirtyMeasure;

	Surface *surface;
	if ((surface = GetSurface ()))
		surface->needs_measure = true;
}

void
UIElement::InvalidateArrange ()
{
	dirty_flags |= DirtyArrange;

	Surface *surface;
	if ((surface = GetSurface ()))
		surface->needs_arrange = true;
}

void
UIElement::DoMeasure ()
{
	Size *last = LayoutInformation::GetLastMeasure (this);
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
	if (parent && (!parent->Is (Type::CANVAS) || (IsLayoutContainer () && !last)))
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
			if (surface && this == surface->GetToplevel ()) {
				Size *measure = LayoutInformation::GetLastMeasure (this);
				if (measure)
					desired = desired.Max (*LayoutInformation::GetLastMeasure (this));
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
		previous_render = GetRenderSize ();
		Arrange (*last);

		if (previous_render == GetRenderSize ())
			return;
	}
	
	if (parent && (!parent->Is (Type::CANVAS) || (IsLayoutContainer () || !last))) 
		parent->InvalidateArrange ();
	
	if (!last)
		return;

	LayoutInformation::SetLastRenderSize (this, &previous_render);
}

bool 
UIElement::InsideClip (cairo_t *cr, double x, double y)
{
	Geometry* clip;
	bool ret = false;
	double nx = x;
	double ny = y;
	
	clip = GetClip ();
	
	if (clip == NULL) {
		return true;
	}

	TransformPoint (&nx, &ny);
	if (!clip->GetBounds ().PointInside (nx, ny))
		return false;
	
	cairo_save (cr);
	clip->Draw (cr);

	if (cairo_in_fill (cr, nx, ny))
		ret = true;
	
	cairo_new_path (cr);
	cairo_restore (cr);

	return ret;
}

bool
UIElement::InsideObject (cairo_t *cr, double x, double y)
{
	return InsideClip (cr, x, y);
}

class LoadedState : public EventObject {
public:
	LoadedState (List *list) { this->list = list; }
	~LoadedState () { delete list; }
  
	List* GetUIElementList () { return list; }

private:
	List* list;
};

void
UIElement::emit_delayed_loaded (EventObject *data)
{
	LoadedState *state = (LoadedState *)data;

	EmitSubtreeLoad (state->GetUIElementList ());
}

void
UIElement::EmitSubtreeLoad (List *l)
{
	while (UIElementNode* node = (UIElementNode*)l->First()) {
		l->Unlink (node);

		node->uielement->OnLoaded ();

		delete node;
	}
}

void
UIElement::PostSubtreeLoad (List *load_list)
{
	LoadedState *state = new LoadedState (load_list);

	GetDeployment()->GetSurface()->GetTimeManager()->AddTickCall (emit_delayed_loaded, state);

	state->unref ();
}

List*
UIElement::WalkTreeForLoaded (bool *delay)
{
	List *walk_list = new List();
	List *load_list = new List();

	if (delay)
		*delay = false;

	walk_list->Append (new UIElementNode (this));

	while (UIElementNode *next = (UIElementNode*)walk_list->First ()) {
		// remove it from the walk list
		walk_list->Unlink (next);

		if (next->uielement->Is(Type::CONTROL)) {
			Control *control = (Control*)next->uielement;
			if (!control->GetStyle() && !control->default_style_applied) {
				ManagedTypeInfo *key = control->GetDefaultStyleKey ();
				if (key) {
					if (Application::GetCurrent () == NULL)
						g_warning ("attempting to use a null application when applying default style when emitting Loaded event.");
					else
						Application::GetCurrent()->ApplyDefaultStyle (control, key);
				}
			}

			if (control->GetStyle() || control->default_style_applied)
				if (delay)
					*delay = true;
			  
		}

		// add it to the front of the load list, and mark it as PENDING_LOADED
		next->uielement->flags |= UIElement::PENDING_LOADED;
		load_list->Prepend (next);

		// and add its children to the front of the walk list
		VisualTreeWalker walker (next->uielement);
		while (UIElement *child = walker.Step ())
			walk_list->Prepend (new UIElementNode (child));
	}

	delete walk_list;

	return load_list;
}


void
UIElement::OnLoaded ()
{
	if (emitting_loaded || IsLoaded())
		return;

	emitting_loaded = true;

	flags |= UIElement::IS_LOADED;

	flags &= ~UIElement::PENDING_LOADED;

	Emit (LoadedEvent, NULL, true);

 	emitting_loaded = false;
}

void
UIElement::ClearLoaded ()
{
	if (!IsLoaded ())
		return;
	
	flags &= ~UIElement::IS_LOADED;
	
	Emit (UnloadedEvent, NULL, true);
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
	return Emit (GotFocusEvent, new EventArgs ());
}

bool
UIElement::EmitLostFocus ()
{
	return Emit (LostFocusEvent, new EventArgs ());
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
		cairo_pattern_t *mask;
		cairo_pattern_t *data = cairo_pop_group (cr);
		Point p = GetOriginPoint ();
		Rect area = Rect (p.x, p.y, 0.0, 0.0);
		GetSizeForBrush (cr, &(area.width), &(area.height));
		opacityMask->SetupBrush (cr, area);
		mask = cairo_get_source (cr);
		cairo_pattern_reference (mask);
		cairo_set_source (cr, data);
		cairo_mask (cr, mask);
		cairo_pattern_destroy (mask);
		cairo_pattern_destroy (data);
	}

	if (IS_TRANSLUCENT (local_opacity)) {
		cairo_pop_group_to_source (cr);
		cairo_paint_with_alpha (cr, local_opacity);
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
	Region *copy = new Region (region);

	if (moonlight_flags & RUNTIME_INIT_RENDER_FRONT_TO_BACK) {
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

	if (!ok) {
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

