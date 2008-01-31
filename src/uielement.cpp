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

#include <gtk/gtk.h>

#include "uielement.h"
#include "collection.h"
#include "brush.h"
#include "transform.h"
#include "runtime.h"
#include "geometry.h"
#include "shape.h"
#include "dirty.h"
#include "eventargs.h"

//#define DEBUG_INVALIDATE 0

extern guint32 moonlight_flags;


int UIElement::LoadedEvent = -1;
int UIElement::MouseMoveEvent = -1;
int UIElement::MouseLeftButtonDownEvent = -1;
int UIElement::MouseLeftButtonUpEvent = -1;
int UIElement::KeyDownEvent = -1;
int UIElement::KeyUpEvent = -1;
int UIElement::MouseEnterEvent = -1;
int UIElement::MouseLeaveEvent = -1;
int UIElement::InvalidatedEvent = -1;
int UIElement::GotFocusEvent = -1;
int UIElement::LostFocusEvent = -1;

void
UIElement::UpdateBounds (bool force_redraw_of_new_bounds)
{
	add_dirty_element (this, DirtyBounds);
	force_invalidate_of_new_bounds = force_redraw_of_new_bounds;
}

void
UIElement::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	g_warning ("GetTransformFor called on a non-container, you must implement this in your container\n");
}

UIElement::UIElement () : opacityMask(NULL), flags (UIElement::RENDER_VISIBLE | UIElement::HIT_TEST_VISIBLE)
{
	bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);

	dirty_flags = 0;
	up_dirty_node = down_dirty_node = NULL;
	dirty_region = new Region ();

	// XXX bad bad bad.  no virtual method calls in ctors
	this->SetValue (UIElement::TriggersProperty, Value::CreateUnref (new TriggerCollection ()));
	this->SetValue (UIElement::ResourcesProperty, Value::CreateUnref (new ResourceDictionary ()));

	ComputeLocalTransform ();
	ComputeTotalRenderVisibility ();
	ComputeTotalHitTestVisibility ();
}

UIElement::~UIElement ()
{
	if (opacityMask != NULL) {
		opacityMask->Detach (NULL, this);
		opacityMask->unref ();
	}
	
	delete dirty_region;

	remove_dirty_element (this);
}

Rect
UIElement::IntersectBoundsWithClipPath (Rect unclipped, bool transform)
{
	Value *value = GetValue (UIElement::ClipProperty);
	if (!value)
		return unclipped;

	Geometry *geometry = value->AsGeometry ();
	Rect box = geometry->ComputeBounds (NULL);

	if (!GetRenderVisible())
		box = Rect (0,0,0,0);

	if (transform)
		box = bounding_rect_for_transformed_rect (&absolute_xform,
							  box);

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
UIElement::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::UIELEMENT) {
		Visual::OnPropertyChanged (prop);
		return;
	}
	  
	if (prop == UIElement::OpacityProperty) {
		UpdateTotalRenderVisibility ();
	}
	else if (prop == UIElement::VisibilityProperty) {
		// note: invalid enum values are only validated in 1.1 (managed code),
		// the default value for VisibilityProperty is VisibilityCollapsed
		// (see bug #340799 for more details)
		if (GetValue (prop)->AsInt32() == VisibilityVisible)
			flags |= UIElement::RENDER_VISIBLE;
		else
			flags &= ~UIElement::RENDER_VISIBLE;
		UpdateTotalRenderVisibility();
	}
	else if (prop == UIElement::IsHitTestVisibleProperty) {
		if (GetValue (prop)->AsBool())
			flags |= UIElement::HIT_TEST_VISIBLE;
		else
			flags &= ~UIElement::HIT_TEST_VISIBLE;
		UpdateTotalHitTestVisibility();
	}
	else if (prop == UIElement::ClipProperty) {
		Invalidate ();
	}
	else if (prop == UIElement::OpacityMaskProperty) {
		if (opacityMask != NULL) {
			opacityMask->Detach (NULL, this);
			opacityMask->unref ();
		}
		
		if ((opacityMask = uielement_get_opacity_mask (this)) != NULL) {
			opacityMask->Attach (NULL, this);
			opacityMask->ref ();
		}
		
		Invalidate ();
	}
	else if (prop == UIElement::RenderTransformProperty || prop == UIElement::RenderTransformOriginProperty) {
		UpdateTransform ();
	}

	NotifyAttachersOfPropertyChange (prop);
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
UIElement::UpdateTotalRenderVisibility ()
{
	add_dirty_element (this, DirtyRenderVisibility);
}

void
UIElement::UpdateTotalHitTestVisibility ()
{
	add_dirty_element (this, DirtyHitTestVisibility);
}

void
UIElement::ComputeTotalRenderVisibility ()
{
	bool visible = (flags & UIElement::RENDER_VISIBLE) != 0;
	bool parent_visible = true;

	total_opacity = GetValue (OpacityProperty)->AsDouble();

	if ((visible || !IS_INVISIBLE (total_opacity)) && GetVisualParent ()) {
		GetVisualParent ()->ComputeTotalRenderVisibility ();
		parent_visible = visible && GetVisualParent ()->GetRenderVisible ();
		total_opacity *= GetVisualParent ()->total_opacity;
	}

	visible = visible && parent_visible;

	if (visible)
		flags |= UIElement::TOTAL_RENDER_VISIBLE;
	else
		flags &= ~UIElement::TOTAL_RENDER_VISIBLE;
}

void
UIElement::ComputeTotalHitTestVisibility ()
{
	bool visible = (flags & UIElement::HIT_TEST_VISIBLE) != 0;

	if (visible && GetVisualParent ()) {
		GetVisualParent ()->ComputeTotalHitTestVisibility ();
		visible = visible && GetVisualParent ()->GetHitTestVisible ();
	}

	if (visible)
		flags |= UIElement::TOTAL_HIT_TEST_VISIBLE;
	else
		flags &= ~UIElement::TOTAL_HIT_TEST_VISIBLE;
}

void
UIElement::UpdateTransform ()
{
	add_dirty_element (this, DirtyLocalTransform);
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
	if (GetVisualParent () != NULL)
		absolute_xform = GetVisualParent ()->absolute_xform;
	else
		cairo_matrix_init_identity (&absolute_xform);

	cairo_matrix_multiply (&absolute_xform, &parent_transform, &absolute_xform);
	cairo_matrix_translate (&absolute_xform, transform_origin.x, transform_origin.y);
	cairo_matrix_multiply (&absolute_xform, &local_transform, &absolute_xform);
	cairo_matrix_translate (&absolute_xform, -transform_origin.x, -transform_origin.y);
	//printf ("      Final position for %s x=%g y=%g\n", GetTypeName(), absolute_xform.x0, absolute_xform.y0);

	// a change in transform requires a change in our bounds, more than likely
}

void
UIElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop)
{
	if (prop == UIElement::RenderTransformProperty) {
		UpdateTransform ();
	}
	else if (prop == UIElement::ClipProperty ||
		 prop == UIElement::OpacityMaskProperty) {

		Invalidate ();
	}
	
	Visual::OnSubPropertyChanged (prop, obj, subprop);
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
	if (!(flags & UIElement::IS_LOADED)) {
		flags |= UIElement::IS_LOADED;
		Emit (LoadedEvent);
	}
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


	add_dirty_element (this, DirtyInvalidate);

	dirty_region->Union (r);

	Emit (InvalidatedEvent);
}

void
UIElement::Invalidate (Region *region)
{
	if (!GetRenderVisible () || IS_INVISIBLE (total_opacity))
		return;

	add_dirty_element (this, DirtyInvalidate);

	dirty_region->Union (region);

	Emit (InvalidatedEvent);
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

void
UIElement::EmitMouseMove (GdkEvent *event)
{
	MouseEventArgs *e = new MouseEventArgs(event);
	Emit (MouseMoveEvent, e);
	e->unref ();
}

void
UIElement::EmitMouseLeftButtonDown (GdkEvent *event)
{
	MouseEventArgs *e = new MouseEventArgs (event);
	Emit (MouseLeftButtonDownEvent, e);
	e->unref ();
}

void
UIElement::EmitMouseLeftButtonUp (GdkEvent *event)
{
	MouseEventArgs *e = new MouseEventArgs (event);
	Emit (MouseLeftButtonUpEvent, e);
	e->unref ();
}

void
UIElement::EmitKeyDown (int state, Key key, int platform_key_code)
{
	KeyboardEventArgs e;
	e.state = state;
	e.key = key;
	e.platformcode = platform_key_code;

	Emit (KeyDownEvent, &e);
}

void
UIElement::EmitKeyUp (int state, Key key, int platform_key_code)
{
	KeyboardEventArgs e;
	e.state = state;
	e.key = key;
	e.platformcode = platform_key_code;

	Emit (KeyUpEvent, &e);
}

void
UIElement::EmitMouseEnter (GdkEvent *event)
{
	MouseEventArgs *e = new MouseEventArgs (event);
	Emit (MouseEnterEvent, e);
	e->unref ();
}

void
UIElement::EmitMouseLeave ()
{
	Emit (MouseLeaveEvent);
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
UIElement::ComputeBounds ()
{
	g_warning ("UIElement:ComputeBounds has been called. The derived class %s should have overridden it.",
		   GetTypeName ());
}

void
UIElement::DoRender (cairo_t *cr, Region *region)
{
	if (!GetRenderVisible() || IS_INVISIBLE (total_opacity) || region->RectIn (GetSubtreeBounds().RoundOut()) == GDK_OVERLAP_RECTANGLE_OUT)
		return;

#if FRONT_TO_BACK_STATS
	GetSurface()->uielements_rendered_back_to_front ++;
#endif

	PreRender (cr, region, false);

	Render (cr, region);

	PostRender (cr, region, false);
}

void
UIElement::FrontToBack (Region *surface_region, List *render_list)
{
	double local_opacity = GetValue (OpacityProperty)->AsDouble();

	if (!GetRenderVisible ()
	    || IS_INVISIBLE (total_opacity)) {
		return;
	}

	Region* self_region = new Region (surface_region);
	self_region->Intersect (bounds.RoundOut());  // note the RoundOut here.
	if (!self_region->IsEmpty()) {
		render_list->Prepend (new RenderNode (this, self_region, true, CallPreRender, CallPostRender));

		bool subtract = (GetValue (UIElement::ClipProperty) == NULL
				 && (absolute_xform.yx == 0 && absolute_xform.xy == 0) /* no skew */
				 && opacityMask == NULL
				 && !IS_TRANSLUCENT (local_opacity));

		// element type specific checks
		if (subtract) {
			if (Is (Type::MEDIAELEMENT)) {
				MediaElement *me = (MediaElement*)this;
				subtract = (strcmp (media_element_get_current_state (me), "Buffering")
					    && me->mplayer
					    && me->mplayer->rendered_frame
					    && ((me->mplayer->width == me->GetBounds().w
						 && me->mplayer->height == me->GetBounds().h)
						||
						(media_base_get_stretch (me) == StretchFill
						 || media_base_get_stretch (me) == StretchUniformToFill)));
			}
			else if (Is (Type::IMAGE)) {
				Image *image = (Image*)this;
				subtract = (image->surface
					    && !image->surface->has_alpha
					    && ((image->GetWidth() == image->GetBounds().w
						 && image->GetHeight() == image->GetBounds().h)
						||
						(media_base_get_stretch (image) == StretchFill
						 || media_base_get_stretch (image) == StretchUniformToFill)));
			}
#if notyet
			else if (Is (Type::RECTANGLE)) {
				// if we're going to subtract anything we'll
				// do it in here, so set this to false so that
				// it doesn't happen later.
				subtract = false;


				Rectangle *rectangle = (Rectangle*)this;
				Brush *fill = shape_get_fill (rectangle);

				if (fill != NULL && fill->IsOpaque()) {
					/* make it a little easier - only consider the rectangle inside the corner radii.
					   we're also a little more conservative than we need to be, regarding stroke
					   thickness. */
					double xr = (rectangle->GetValue(Rectangle::RadiusXProperty)->AsDouble() +
						     rectangle->GetValue(Rectangle::StrokeThicknessProperty)->AsDouble());
					double yr = (rectangle->GetValue(Rectangle::RadiusYProperty)->AsDouble() +
						     rectangle->GetValue(Rectangle::StrokeThicknessProperty)->AsDouble());

					Rect r = bounds.GrowBy (-xr, -yr).RoundIn();

					Region *inner_rect_region = new Region (self_region);
					inner_rect_region->Intersect (r);
					if (!inner_rect_region->IsEmpty()) {
						surface_region->Subtract (inner_rect_region);
					}
					delete inner_rect_region;
				}
			}
#endif
			// XXX more stuff here for non-panel subclasses...
			else {
				subtract = false;
			}
		}

		if (subtract)
			surface_region->Subtract (bounds);

	}
}

void
UIElement::PreRender (cairo_t *cr, Region *region, bool front_to_back)
{
	double local_opacity = GetValue (OpacityProperty)->AsDouble();

	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);
	cairo_save (cr);

	cairo_set_matrix (cr, &absolute_xform);
	RenderClipPath (cr);

	if (opacityMask || IS_TRANSLUCENT (local_opacity)) {
		Rect r = GetSubtreeBounds ().RoundOut();
		cairo_save (cr);
		cairo_identity_matrix (cr);
		region->Draw (cr);
		cairo_clip (cr);
		cairo_rectangle (cr, r.x, r.y, r.w, r.h);
		cairo_clip (cr);
		cairo_restore (cr);
	}

	if (IS_TRANSLUCENT (local_opacity))
		cairo_push_group (cr);

	if (opacityMask != NULL)
		cairo_push_group (cr);
}

void
UIElement::PostRender (cairo_t *cr, Region *region, bool front_to_back)
{
	double local_opacity = GetValue (OpacityProperty)->AsDouble();

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
	
	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);

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
	*height = 
	  *width = 0.0;
}

DependencyProperty* UIElement::ClipProperty;
DependencyProperty* UIElement::CursorProperty;
DependencyProperty* UIElement::IsHitTestVisibleProperty;
DependencyProperty* UIElement::OpacityMaskProperty;
DependencyProperty* UIElement::OpacityProperty;
DependencyProperty* UIElement::RenderTransformOriginProperty;
DependencyProperty* UIElement::RenderTransformProperty;
DependencyProperty* UIElement::ResourcesProperty;
DependencyProperty* UIElement::TagProperty;
DependencyProperty* UIElement::TriggersProperty;
DependencyProperty* UIElement::VisibilityProperty;
DependencyProperty* UIElement::ZIndexProperty;

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

	/* lookup events */
	Type* t = Type::Find (Type::UIELEMENT);
	UIElement::LoadedEvent = t->LookupEvent ("Loaded");
	UIElement::MouseMoveEvent = t->LookupEvent ("MouseMove");
	UIElement::MouseLeftButtonDownEvent = t->LookupEvent ("MouseLeftButtonDown");
	UIElement::MouseLeftButtonUpEvent = t->LookupEvent ("MouseLeftButtonUp");
	UIElement::KeyDownEvent = t->LookupEvent ("KeyDown");
	UIElement::KeyUpEvent = t->LookupEvent ("KeyUp");
	UIElement::MouseEnterEvent = t->LookupEvent ("MouseEnter");
	UIElement::MouseLeaveEvent = t->LookupEvent ("MouseLeave");
	UIElement::InvalidatedEvent = t->LookupEvent("Invalidated");
	UIElement::GotFocusEvent = t->LookupEvent("GotFocus");
	UIElement::LostFocusEvent = t->LookupEvent("LostFocus");
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

double
uielement_get_opacity (UIElement *item)
{
	return item->GetValue (UIElement::OpacityProperty)->AsDouble();
}

void
uielement_set_opacity (UIElement *item, double opacity)
{
	item->SetValue (UIElement::OpacityProperty, Value (opacity));
}

Brush *
uielement_get_opacity_mask (UIElement *item)
{
	Value *value = item->GetValue (UIElement::OpacityMaskProperty);
	
	return value ? (Brush *) value->AsBrush () : NULL;
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
