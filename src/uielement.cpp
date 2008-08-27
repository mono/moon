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
#include "clock.h"
#include "media.h"

//#define DEBUG_INVALIDATE 0
#define MIN_FRONT_TO_BACK_COUNT 25

UIElement::UIElement ()
{
	visual_parent = NULL;
	opacityMask = NULL;
	
	flags = UIElement::RENDER_VISIBLE | UIElement::HIT_TEST_VISIBLE;

	bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);

	emitting_loaded = false;
	dirty_flags = 0;
	up_dirty_node = down_dirty_node = NULL;
	force_invalidate_of_new_bounds = false;
	dirty_region = new Region ();

#if SL_2_0
	desired_size = Size (0, 0);
#endif
	
	ComputeLocalTransform ();
	ComputeTotalRenderVisibility ();
	ComputeTotalHitTestVisibility ();

	// XXX bad bad bad.  no virtual method calls in ctors
	SetValue (UIElement::TriggersProperty, Value::CreateUnref (new TriggerCollection ()));
	SetValue (UIElement::ResourcesProperty, Value::CreateUnref (new ResourceDictionary ()));
}

UIElement::~UIElement()
{
	delete dirty_region;
}

void
UIElement::Dispose()
{
	TriggerCollection *triggers = GetTriggers ();
	
	for (int i = 0; i < triggers->GetCount (); i++)
		triggers->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
	
	ContentWalker walker = ContentWalker (this);
	while (DependencyObject *content = walker.Step ()) {
		if (content->Is (Type::UIELEMENT))
			((UIElement *)content)->SetVisualParent (NULL);
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

	DependencyObject::SetSurface (s);
}

Rect
UIElement::IntersectBoundsWithClipPath (Rect unclipped, bool transform)
{
	Geometry *geometry = GetClip ();
	if (!geometry)
		return unclipped;

	Rect box = geometry->ComputeBounds (NULL, false);

	if (!GetRenderVisible())
		box = Rect (0,0,0,0);

	if (transform)
		box = box.Transform (&absolute_xform);

	return box.Intersection (unclipped);
}

void
UIElement::RenderClipPath (cairo_t *cr)
{
	cairo_new_path (cr);
	cairo_set_matrix (cr, &absolute_xform);

	Geometry *geometry = GetClip();
	if (!geometry)
		return;

	geometry->Draw (NULL, cr);
	cairo_clip (cr);
}

void
UIElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::UIELEMENT) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	  
	if (args->property == UIElement::OpacityProperty) {
		UpdateTotalRenderVisibility ();
		Invalidate (GetSubtreeBounds ());
	} else if (args->property == UIElement::VisibilityProperty) {
		// note: invalid enum values are only validated in 1.1 (managed code),
		// the default value for VisibilityProperty is VisibilityCollapsed
		// (see bug #340799 for more details)
		if (args->new_value->AsInt32() == VisibilityVisible)
			flags |= UIElement::RENDER_VISIBLE;
		else
			flags &= ~UIElement::RENDER_VISIBLE;
		UpdateTotalRenderVisibility();
		Invalidate (GetSubtreeBounds ());
	} else if (args->property == UIElement::IsHitTestVisibleProperty) {
		if (args->new_value->AsBool())
			flags |= UIElement::HIT_TEST_VISIBLE;
		else
			flags &= ~UIElement::HIT_TEST_VISIBLE;
		UpdateTotalHitTestVisibility();
	} else if (args->property == UIElement::ClipProperty) {
		Invalidate(GetSubtreeBounds());
		// force invalidation even if the bounding rectangle
		// changes (since the clip can be concave)
		UpdateBounds (true);
	} else if (args->property == UIElement::OpacityMaskProperty) {
		opacityMask = args->new_value ? args->new_value->AsBrush() : NULL;
		Invalidate (GetSubtreeBounds ());
	} else if (args->property == UIElement::RenderTransformProperty || args->property == UIElement::RenderTransformOriginProperty) {
		UpdateTransform ();
	}
	else if (args->property == UIElement::TriggersProperty) {
		if (args->old_value) {
			// remove the old trigger targets
			TriggerCollection *triggers = args->old_value->AsTriggerCollection();
			for (int i = 0; i < triggers->GetCount (); i++)
				triggers->GetValueAt (i)->AsEventTrigger ()->RemoveTarget (this);
		}

		if (args->new_value) {
			// set the new ones
			TriggerCollection *triggers = args->new_value->AsTriggerCollection();
			for (int i = 0; i < triggers->GetCount (); i++)
				triggers->GetValueAt (i)->AsEventTrigger ()->SetTarget (this);
		}
	}

	NotifyListenersOfPropertyChange (args);
}

void
UIElement::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col == GetTriggers ()) {
		switch (args->action) {
		case CollectionChangedActionReplace:
			args->old_value->AsEventTrigger ()->RemoveTarget (this);
			// fall thru to Add
		case CollectionChangedActionAdd:
			args->new_value->AsEventTrigger ()->SetTarget (this);
			break;
		case CollectionChangedActionRemove:
			args->old_value->AsEventTrigger ()->RemoveTarget (this);
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

#if true
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
	if (GetSurface ())
		GetSurface ()->AddDirtyElement (this, DirtyBounds);

	force_invalidate_of_new_bounds |= force_redraw;
}

void
UIElement::UpdateTotalRenderVisibility ()
{
	if (GetSurface())
		GetSurface()->AddDirtyElement (this, DirtyRenderVisibility);
}

void
UIElement::UpdateTotalHitTestVisibility ()
{
	ContentWalker walker = ContentWalker (this);
	while (DependencyObject *content = walker.Step ()) {
		if (content->Is (Type::UIELEMENT))
			((UIElement *)content)->UpdateTotalHitTestVisibility ();
	}

	if (GetSurface())
		GetSurface ()->AddDirtyElement (this, DirtyHitTestVisibility);
}

void
UIElement::UpdatePosition ()
{
	if (this->dirty_flags & (DirtyLocalTransform | DirtyTransform))
		return;

	if (GetSurface())
		GetSurface()->AddDirtyElement (this, DirtyPosition);
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
		this->dirty_flags &= ~DirtyPosition;
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
UIElement::ComputeTransform ()
{
	if (GetVisualParent () != NULL) {
		cairo_matrix_t parent_transform;
		GetVisualParent ()->GetTransformFor (this, &parent_transform);
		absolute_xform = GetVisualParent ()->absolute_xform;
		cairo_matrix_multiply (&absolute_xform, &parent_transform, &absolute_xform);
	} else 
		GetTransformFor (this, &absolute_xform);

	cairo_matrix_multiply (&absolute_xform, &local_xform, &absolute_xform);
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
UIElement::ComputePosition ()
{
	Point p (bounds.x, bounds.y);

	cairo_matrix_t inverse = absolute_xform;
	cairo_matrix_invert (&inverse);

	p = p.Transform (&inverse);

	ComputeLocalTransform();
	ComputeTransform();

	p = p.Transform (&absolute_xform);

// 	printf ("old position is %g %g, new position is %g %g\n",
// 		bounds.x, bounds.y, p.x, p.y);

	ShiftPosition (p);
}

void
UIElement::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	g_warning ("GetTransformFor called on a non-container, you must implement this in your container\n");
}

void
UIElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == UIElement::RenderTransformProperty) {
		UpdateTransform ();
	}
	else if (prop == UIElement::ClipProperty) {
		Invalidate(GetSubtreeBounds());
		// force invalidation even if the bounding rectangle
		// changes (since the clip can be concave)
		UpdateBounds (true);
	}
	else if (prop == UIElement::OpacityMaskProperty) {
	        Invalidate ();
	}
	
	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
}

void 
UIElement::CacheInvalidateHint () 
{
	ContentWalker walker = ContentWalker (this);
	while (DependencyObject *content = walker.Step ()) {
		if (content && content->Is (Type::UIELEMENT)) {
			((UIElement *)content)->CacheInvalidateHint ();
		}
	}
}

void
UIElement::ContentRemoved (DependencyObject *obj)
{
	DependencyObject:: ContentRemoved (obj);

	if (obj->Is (Type::UIELEMENT)) {
		UIElement *item = (UIElement *) obj;
		
		Invalidate (item->GetSubtreeBounds());
		
		item->CacheInvalidateHint ();
		item->SetVisualParent (NULL);
		item->ClearLoaded ();
	}
}

void
UIElement::ContentAdded (DependencyObject *obj)
{
	DependencyObject::ContentAdded (obj);

	if (obj->Is (Type::UIELEMENT)) {
		UIElement *item = (UIElement *) obj;
		
		item->SetVisualParent (this);
		item->UpdateTransform ();
		item->UpdateTotalRenderVisibility ();
		item->UpdateTotalHitTestVisibility ();
		item->Invalidate ();

		if (IsLoaded ())
			item->OnLoaded ();
		
		UpdateBounds (true);
	}
}

void
UIElement::InvalidateMeasure ()
{
	g_warning ("UIElement::InvalidateMeasure not implemented - it should add a dirty flag (DirtyMeasure)");
}

void
UIElement::InvalidateArrange ()
{
	g_warning ("UIElement::InvalidateArrange not implemented - it should add a dirty flag (DirtyArrange)");
}

void
UIElement::UpdateLayout ()
{
	g_warning ("UIElement::UpdateLayout not implemented - it should process the layout dirty flags synchronously");
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
	
	cairo_save (cr);

	clip->Draw (NULL, cr);

	TransformPoint (&nx, &ny);

	if (cairo_in_stroke (cr, nx, ny) || (clip->IsFilled () && cairo_in_fill (cr, nx, ny)))
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

void
UIElement::OnLoaded ()
{
	if (emitting_loaded)
		return;

	emitting_loaded = true;

	ContentWalker walker = ContentWalker (this);
	while (DependencyObject *content = walker.Step ()) {
		if (content && content->Is (Type::UIELEMENT))
			((UIElement *)content)->OnLoaded ();
	}
		   
	flags |= UIElement::IS_LOADED;

	Emit (LoadedEvent, NULL, true);

 	emitting_loaded = false;

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

void
UIElement::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	uielement_list->Prepend (new UIElementNode (this));
}

void
UIElement::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
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

	s->SetMouseCapture (NULL);
}

void
UIElement::DoRender (cairo_t *cr, Region *parent_region)
{
	Region *region = new Region (GetSubtreeBounds ());
	region->Intersect (parent_region);

	if (!GetRenderVisible() || IS_INVISIBLE (total_opacity) || region->IsEmpty ()) 
		return;

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
	return ContentWalker (this).GetCount () < MIN_FRONT_TO_BACK_COUNT;
}

void
UIElement::FrontToBack (Region *surface_region, List *render_list)
{
	double local_opacity = GetOpacity ();

	if (!GetRenderVisible ()
	    || IS_INVISIBLE (total_opacity)) {
		return;
	}

	Region* self_region = new Region (surface_region);
	self_region->Intersect (bounds.RoundOut());  // note the RoundOut here.
	if (self_region->IsEmpty()) {
		delete self_region;
		return;
	}
		
	render_list->Prepend (new RenderNode (this, self_region, true, CallPreRender, CallPostRender));

	bool subtract = ((absolute_xform.yx == 0 && absolute_xform.xy == 0) /* no skew */
			 && !IS_TRANSLUCENT (local_opacity)
			 && (GetClip () == NULL)
			 && (GetOpacityMask () == NULL)); // XXX we can easily deal with opaque solid color brushes.

	// element type specific checks
	if (subtract) {
		if (Is (Type::MEDIAELEMENT)) {
			MediaElement *media = (MediaElement *) this;
			MediaPlayer *mplayer = media->GetMediaPlayer ();
			Stretch stretch = media->GetStretch ();

			subtract = (!media->IsClosed () && mplayer
				    && mplayer->HasRenderedFrame ()
				    && ((mplayer->GetVideoWidth () == media->GetBounds ().width
					 && mplayer->GetVideoHeight () == media->GetBounds ().height)
					|| (stretch == StretchFill || stretch == StretchUniformToFill)));

			//Rect r = me->GetBounds();
				//printf ("r.bounds = %g %g %g %g\n", r.x, r.y, r.width, r.height);
		}
		else if (Is (Type::IMAGE)) {
			Image *image = (Image *) this;
			Stretch stretch = image->GetStretch ();

			subtract = (image->surface && !image->surface->has_alpha
				    && ((image->GetImageWidth () == image->GetBounds ().width
					 && image->GetImageHeight () == image->GetBounds ().height)
					|| (stretch == StretchFill || stretch == StretchUniformToFill)));
		}
		else if (Is (Type::RECTANGLE)) {
			// if we're going to subtract anything we'll
			// do it in here, so set this to false so that
			// it doesn't happen later.
			subtract = false;

			Rectangle *rectangle = (Rectangle *) this;
			Brush *fill = rectangle->GetFill ();

			if (fill != NULL && fill->IsOpaque()) {
				/* make it a little easier - only consider the rectangle inside the corner radii.
				   we're also a little more conservative than we need to be, regarding stroke
				   thickness. */
				double xr = (rectangle->GetRadiusX () + rectangle->GetStrokeThickness () / 2);
				double yr = (rectangle->GetRadiusY () + rectangle->GetStrokeThickness () / 2);

				Rect r = bounds.GrowBy (-xr, -yr).RoundOut();

				Region *inner_rect_region = new Region (self_region);
				inner_rect_region->Intersect (r);
				if (!inner_rect_region->IsEmpty())
					surface_region->Subtract (inner_rect_region);

				delete inner_rect_region;
			}
		}
		// XXX more stuff here for non-panel subclasses...
		else {
			subtract = false;
		}
	}

	if (subtract)
		surface_region->Subtract (bounds);
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
		ContentWalker walker = ContentWalker (this, ZForward);
		while (DependencyObject *content = walker.Step ()) {
			if (!content->Is (Type::UIELEMENT))
				continue;
			
			// DoRender does all the proper region and visibility checking
			((UIElement *)content)->DoRender (cr, region);
		}
	}

	double local_opacity = GetOpacity ();

	if (opacityMask != NULL) {
		cairo_pattern_t *mask;
		cairo_pattern_t *data = cairo_pop_group (cr);
		opacityMask->SetupBrush (cr, this);
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
			geometry->Draw (NULL, cr);
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
UIElement::Render (cairo_t *cr, Region *region)
{
        Rect rect = region->ClipBox ();
	Render (cr, (int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
}

void
UIElement::Render (cairo_t *cr, int x, int y, int width, int height)
{
	g_warning ("UIElement:Render has been called. The derived class %s should have overridden it.",
		   GetTypeName ());
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
	return surface ? surface->GetTimeManager() : NULL;
}

#if SL_2_0
GeneralTransform *
UIElement::GetTransformToUIElement (UIElement *to_element)
{
	cairo_matrix_t result;
	cairo_matrix_t inverse = absolute_xform;
	cairo_matrix_invert (&inverse);
	cairo_matrix_multiply (&result, &inverse, &to_element->absolute_xform);

	Matrix *matrix  = new Matrix (&result);

	MatrixTransform *transform = new MatrixTransform ();
	transform->SetValue (MatrixTransform::MatrixProperty, matrix);
	matrix->unref ();

	return transform;
}
#endif



void
UIElement::TransformPoint (double *x, double *y)
{
	cairo_matrix_t inverse = absolute_xform;
	cairo_matrix_invert (&inverse);
	
	cairo_matrix_transform_point (&inverse, x, y);
}
