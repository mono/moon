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

UIElement::UIElement ()
{
	opacityMask = NULL;
	flags = UIElement::RENDER_VISIBLE | UIElement::HIT_TEST_VISIBLE;

	bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);

	dirty_flags = 0;
	up_dirty_node = down_dirty_node = NULL;
	force_invalidate_of_new_bounds = false;
	dirty_region = new Region ();

#if SL_2_0
	desired_size = Size (0, 0);
#endif
	
	// XXX bad bad bad.  no virtual method calls in ctors
	SetValue (UIElement::TriggersProperty, Value::CreateUnref (new TriggerCollection ()));
	SetValue (UIElement::ResourcesProperty, Value::CreateUnref (new ResourceDictionary ()));

	ComputeLocalTransform ();
	ComputeTotalRenderVisibility ();
	ComputeTotalHitTestVisibility ();
}

UIElement::~UIElement ()
{
	delete dirty_region;
}

void
UIElement::SetSurface (Surface *s)
{
	if (s == NULL && GetSurface()) {
		/* we're losing our surface, delete ourselves from the dirty list if we're on it */
		GetSurface()->RemoveDirtyElement (this);
	}
	DependencyObject::SetSurface (s);
}

Rect
UIElement::IntersectBoundsWithClipPath (Rect unclipped, bool transform)
{
	Value *value = GetValue (UIElement::ClipProperty);
	if (!value)
		return unclipped;

	Geometry *geometry = value->AsGeometry ();
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

	Value *value = GetValue (UIElement::ClipProperty);
	if (!value)
		return;

	Geometry *geometry = value->AsGeometry ();
	geometry->Draw (NULL, cr);
	cairo_clip (cr);
}

void
UIElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::UIELEMENT) {
		Visual::OnPropertyChanged (args);
		return;
	}
	  
	if (args->property == UIElement::OpacityProperty) {
		UpdateTotalRenderVisibility ();
		Invalidate ();
	}
	else if (args->property == UIElement::VisibilityProperty) {
		// note: invalid enum values are only validated in 1.1 (managed code),
		// the default value for VisibilityProperty is VisibilityCollapsed
		// (see bug #340799 for more details)
		if (args->new_value->AsInt32() == VisibilityVisible)
			flags |= UIElement::RENDER_VISIBLE;
		else
			flags &= ~UIElement::RENDER_VISIBLE;
		UpdateTotalRenderVisibility();
	}
	else if (args->property == UIElement::IsHitTestVisibleProperty) {
		if (args->new_value->AsBool())
			flags |= UIElement::HIT_TEST_VISIBLE;
		else
			flags &= ~UIElement::HIT_TEST_VISIBLE;
		UpdateTotalHitTestVisibility();
	}
	else if (args->property == UIElement::ClipProperty) {
		Invalidate(GetSubtreeBounds());
		// force invalidation even if the bounding rectangle
		// changes (since the clip can be concave)
		UpdateBounds (true);
	}
	else if (args->property == UIElement::OpacityMaskProperty) {
		opacityMask = args->new_value ? args->new_value->AsBrush() : NULL;
		Invalidate ();
	}
	else if (args->property == UIElement::RenderTransformProperty || args->property == UIElement::RenderTransformOriginProperty) {
		UpdateTransform ();
	}

	NotifyListenersOfPropertyChange (args);
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
	uielement_get_render_affine (this, &local_transform);
	transform_origin = GetTransformOrigin ();

	if (GetVisualParent () != NULL)
		GetVisualParent ()->GetTransformFor (this, &parent_transform);
	else
		cairo_matrix_init_identity (&parent_transform);
}

void
UIElement::ComputeTransform ()
{
	//printf ("Compute transform for %s\n", GetName ());
	if (GetVisualParent () != NULL)
		absolute_xform = GetVisualParent ()->absolute_xform;
	else 
		GetTransformFor (this, &absolute_xform);

	cairo_matrix_multiply (&absolute_xform, &parent_transform, &absolute_xform);
	cairo_matrix_translate (&absolute_xform, transform_origin.x, transform_origin.y);
	cairo_matrix_multiply (&absolute_xform, &local_transform, &absolute_xform);
	cairo_matrix_translate (&absolute_xform, -transform_origin.x, -transform_origin.y);
	//printf ("      Final position for %s x=%g y=%g\n", GetTypeName(), absolute_xform.x0, absolute_xform.y0);

	// a change in transform requires a change in our bounds, more than likely
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
	
	Visual::OnSubPropertyChanged (prop, obj, subobj_args);
}

bool 
UIElement::InsideClip (cairo_t *cr, double x, double y)
{
	Value* clip_geometry = GetValue (UIElement::ClipProperty);
	Geometry* clip;
	bool ret = false;
	double nx = x;
	double ny = y;
	
	if (clip_geometry == NULL) {
		return true;
	}

	clip = clip_geometry->AsGeometry ();
	
	if (clip == NULL) {
		return true;
	}
	
	cairo_save (cr);

	clip->Draw (NULL, cr);

	uielement_transform_point (this, &nx, &ny);

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
	flags |= UIElement::IS_LOADED;

	Emit (LoadedEvent, NULL, true);
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
UIElement::HitTest (cairo_t *cr, double x, double y, List *uielement_list)
{
	uielement_list->Prepend (new UIElementNode (this));
}

bool
UIElement::EmitMouseMove (GdkEvent *event)
{
	return Emit (MouseMoveEvent, new MouseEventArgs (event));
}

bool
UIElement::EmitMouseLeftButtonDown (GdkEvent *event)
{
	return Emit (MouseLeftButtonDownEvent, new MouseEventArgs (event));
}

bool
UIElement::EmitMouseLeftButtonUp (GdkEvent *event)
{
	return Emit (MouseLeftButtonUpEvent, new MouseEventArgs (event));
}

bool
UIElement::EmitKeyDown (int state, Key key, int platform_key_code)
{
	return Emit (KeyDownEvent, new KeyboardEventArgs (state, platform_key_code, key));
}

bool
UIElement::EmitKeyUp (int state, Key key, int platform_key_code)
{
	return Emit (KeyUpEvent, new KeyboardEventArgs (state, platform_key_code, key));
}

bool
UIElement::EmitMouseEnter (GdkEvent *event)
{
	return Emit (MouseEnterEvent, new MouseEventArgs (event));
}

bool
UIElement::EmitMouseLeave ()
{
	// LAMESPEC: msdn2 says this event is raised with null args in JS,
	// but the JS is clearly passed an EventArgs instance.
	return Emit (MouseLeaveEvent, new EventArgs ());
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
UIElement::DoRender (cairo_t *cr, Region *region)
{
	if (!GetRenderVisible() || IS_INVISIBLE (total_opacity) || region->RectIn (GetSubtreeBounds().RoundOut()) == GDK_OVERLAP_RECTANGLE_OUT)
		return;

#if FRONT_TO_BACK_STATS
	GetSurface()->uielements_rendered_back_to_front ++;
#endif

	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);

	PreRender (cr, region, false);

	Render (cr, region);

	PostRender (cr, region, false);

	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);
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
	if (!self_region->IsEmpty()) {
		render_list->Prepend (new RenderNode (this, self_region, true, CallPreRender, CallPostRender));

		bool subtract = ((absolute_xform.yx == 0 && absolute_xform.xy == 0) /* no skew */
				 && !IS_TRANSLUCENT (local_opacity)
				 && (GetValue (UIElement::ClipProperty) == NULL)
				 && (GetOpacityMask () == NULL)); // XXX we can easily deal with opaque solid color brushes.

		// element type specific checks
		if (subtract) {
			if (Is (Type::MEDIAELEMENT)) {
				MediaElement *media = (MediaElement *) this;
				MediaPlayer *mplayer = media->GetMediaPlayer ();
				Stretch stretch = media->GetStretch ();
				
				subtract = (!media->IsClosed () && mplayer
					    && mplayer->HasRenderedFrame ()
					    && ((mplayer->GetVideoWidth () == media->GetBounds ().w
						 && mplayer->GetVideoHeight () == media->GetBounds ().h)
						|| (stretch == StretchFill || stretch == StretchUniformToFill)));
				
				//Rect r = me->GetBounds();
				//printf ("r.bounds = %g %g %g %g\n", r.x, r.y, r.w, r.h);
			}
			else if (Is (Type::IMAGE)) {
				Image *image = (Image *) this;
				Stretch stretch = image->GetStretch ();
				
				subtract = (image->surface && !image->surface->has_alpha
					    && ((image->GetImageWidth () == image->GetBounds ().w
						 && image->GetImageHeight () == image->GetBounds ().h)
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
	} else {
		delete self_region;
	}
}

void
UIElement::PreRender (cairo_t *cr, Region *region, bool front_to_back)
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
		cairo_rectangle (cr, r.x, r.y, r.w, r.h);
		cairo_clip (cr);
		cairo_set_matrix (cr, &absolute_xform);
	}

	if (IS_TRANSLUCENT (local_opacity))
		cairo_push_group (cr);

	if (opacityMask != NULL)
		cairo_push_group (cr);
}

void
UIElement::PostRender (cairo_t *cr, Region *region, bool front_to_back)
{
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
		
		Value *value = GetValue (UIElement::ClipProperty);
		if (value) {
			Geometry *geometry = value->AsGeometry ();
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
		cairo_rectangle (cr, bounds.x + .5, bounds.y + .5, bounds.w - .5, bounds.h - .5);
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
	Render (cr, (int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h);
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

void
UIElement::SetOpacityMask (Brush *mask)
{
	SetValue (UIElement::OpacityMaskProperty, Value (mask));
}

Brush *
UIElement::GetOpacityMask ()
{
	Value *value = GetValue (UIElement::OpacityMaskProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
UIElement::SetOpacity (double opacity)
{
	SetValue (UIElement::OpacityProperty, Value (opacity));
}

double
UIElement::GetOpacity ()
{
	return GetValue (UIElement::OpacityProperty)->AsDouble ();
}


DependencyProperty *UIElement::ClipProperty;
DependencyProperty *UIElement::CursorProperty;
DependencyProperty *UIElement::IsHitTestVisibleProperty;
DependencyProperty *UIElement::OpacityMaskProperty;
DependencyProperty *UIElement::OpacityProperty;
DependencyProperty *UIElement::RenderTransformOriginProperty;
DependencyProperty *UIElement::RenderTransformProperty;
DependencyProperty *UIElement::ResourcesProperty;
DependencyProperty *UIElement::TagProperty;
DependencyProperty *UIElement::TriggersProperty;
DependencyProperty *UIElement::VisibilityProperty;
DependencyProperty *UIElement::ZIndexProperty;

void
uielement_init (void)
{
	UIElement::ClipProperty = DependencyObject::Register (Type::UIELEMENT, "Clip", Type::GEOMETRY);
	UIElement::CursorProperty = DependencyObject::Register (Type::UIELEMENT, "Cursor", new Value ((gint32)MouseCursorDefault));
	UIElement::IsHitTestVisibleProperty = DependencyObject::Register (Type::UIELEMENT, "IsHitTestVisible", new Value (true));
	UIElement::OpacityMaskProperty = DependencyObject::Register (Type::UIELEMENT, "OpacityMask", Type::BRUSH);
	UIElement::OpacityProperty = DependencyObject::Register (Type::UIELEMENT, "Opacity", new Value(1.0));
	UIElement::RenderTransformOriginProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransformOrigin", new Value (Point (0,0)), Type::POINT);
	UIElement::RenderTransformProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransform", Type::TRANSFORM);
	UIElement::ResourcesProperty = DependencyObject::Register (Type::UIELEMENT, "Resources", Type::RESOURCE_DICTIONARY);
	UIElement::TagProperty = DependencyObject::Register (Type::UIELEMENT, "Tag", Type::STRING);
	UIElement::TriggersProperty = DependencyObject::Register (Type::UIELEMENT, "Triggers", Type::TRIGGER_COLLECTION);
	UIElement::VisibilityProperty = DependencyObject::Register (Type::UIELEMENT, "Visibility", new Value ((gint32)VisibilityVisible));
	UIElement::ZIndexProperty = DependencyObject::Register (Type::UIELEMENT, "ZIndex", new Value ((gint32)0));;
}

UIElement *
uielement_new (void)
{
	return new UIElement ();
}

Surface *
uielement_get_surface (UIElement *item)
{
	return item->GetSurface ();
}

void 
uielement_invalidate (UIElement *item)
{
	item->Invalidate ();
}

void 
uielement_set_transform_origin (UIElement *item, Point p)
{
	item->SetValue (UIElement::RenderTransformOriginProperty, p);
}

void
uielement_get_render_affine (UIElement *item, cairo_matrix_t *result)
{
	Value* v = item->GetValue (UIElement::RenderTransformProperty);
	if (v == NULL)
		cairo_matrix_init_identity (result);
	else {
		Transform *t = v->AsTransform();
		t->GetTransform (result);
	}
}

/**
 * uielement_getbounds:
 * @item: the item to update the bounds of
 *
 * Does this by requesting bounds update to all of its parents. 
 */
void
uielement_update_bounds (UIElement *item)
{
	item->UpdateBounds();
}

void
uielement_set_render_transform (UIElement *item, Transform *transform)
{
	item->SetValue (UIElement::RenderTransformProperty, Value(transform));
}

void
uielement_set_opacity_mask (UIElement *item, Brush *mask)
{
	item->SetOpacityMask (mask);
}

Brush *
uielement_get_opacity_mask (UIElement *item)
{
	return item->GetOpacityMask ();
}

void
uielement_set_opacity (UIElement *item, double opacity)
{
	item->SetOpacity (opacity);
}

double
uielement_get_opacity (UIElement *item)
{
	return item->GetOpacity ();
}

//
// Maps the x, y coordinate to the space of the given item
//
void
uielement_transform_point (UIElement *item, double *x, double *y)
{
	cairo_matrix_t inverse = item->absolute_xform;
	cairo_matrix_invert (&inverse);

	cairo_matrix_transform_point (&inverse, x, y);
}

UIElement *
uielement_get_parent (UIElement *item)
{
	return item->GetVisualParent ();
}

#if SL_2_0
Size
uielement_get_desired_size (UIElement *item)
{
	return item->desired_size; 
}
#endif

bool
uielement_capture_mouse (UIElement *item)
{
	return item->CaptureMouse ();
}

void
uielement_release_mouse_capture (UIElement *item)
{
	item->ReleaseMouseCapture ();
}
