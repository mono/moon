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

#define SHOW_BOUNDING_BOXES 0

void
UIElement::UpdateBounds (bool force_redraw_of_new_bounds)
{
	Rect obounds = bounds;
	
	ComputeBounds ();
	
	//
	// If we changed, notify the parent to recompute its bounds
	//
	if (bounds != obounds) {
		// Invalidate the old bounds
		Invalidate (obounds);

		// And the new rect
		Invalidate (bounds);

		if (parent != NULL)
			parent->UpdateBounds();
	}
	else if (force_redraw_of_new_bounds) {
		Invalidate (bounds);
	}
}

void
UIElement::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	printf ("get_xform_for called on a non-container, you must implement this in your container\n");
	exit (1);
}

UIElement::UIElement () : opacityMask(NULL), parent(NULL), flags (UIElement::RENDER_VISIBLE | UIElement::LAYOUT_VISIBLE | UIElement::HIT_TEST_VISIBLE)
{
	LoadedEvent = RegisterEvent ("Loaded");
	MotionEvent = RegisterEvent ("MouseMove");
	ButtonPressEvent = RegisterEvent ("MouseLeftButtonDown");
	ButtonReleaseEvent = RegisterEvent ("MouseLeftButtonUp");
	KeyDownEvent = RegisterEvent ("KeyDown");
	KeyUpEvent = RegisterEvent ("KeyUp");
	EnterEvent = RegisterEvent ("MouseEnter");
	LeaveEvent = RegisterEvent ("MouseLeave");

	bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);

	this->SetValue (UIElement::TriggersProperty, Value (new TriggerCollection ()));
	this->SetValue (UIElement::ResourcesProperty, Value (new ResourceDictionary ()));
}

UIElement::~UIElement ()
{
	if (opacityMask != NULL) {
		opacityMask->Detach (NULL, this);
		opacityMask->unref ();
	}
}

void
UIElement::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::UIELEMENT) {
		Visual::OnPropertyChanged (prop);
		return;
	}
	  
	if (prop == UIElement::OpacityProperty ||
	    prop == UIElement::ZIndexProperty) {
		Invalidate ();
	}
	else if (prop == UIElement::VisibilityProperty) {
		switch (GetValue (prop)->AsInt32()) {
		case VisibilityVisible:
			flags |= UIElement::RENDER_VISIBLE | UIElement::LAYOUT_VISIBLE;
			Invalidate ();
			break;
		case VisibilityHidden:
			flags &= ~UIElement::RENDER_VISIBLE;
			flags |= UIElement::LAYOUT_VISIBLE;
			Invalidate ();
			break;
		case VisibilityCollapsed:
			flags &= ~(UIElement::RENDER_VISIBLE | UIElement::LAYOUT_VISIBLE);
			FullInvalidate (true);
			break;
		default:
			g_assert_not_reached ();
			break;
		}
	}
	else if (prop == UIElement::IsHitTestVisibleProperty) {
		if (GetValue (prop)->AsBool())
			flags |= UIElement::HIT_TEST_VISIBLE;
		else
			flags &= ~UIElement::HIT_TEST_VISIBLE;
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
	else if (prop == RenderTransformProperty || prop == RenderTransformOriginProperty) {
		UpdateTransform ();
	}
	else if (prop == TriggersProperty) {
		Value *v = GetValue (prop);
		TriggerCollection *newcol = v ?  v->AsTriggerCollection() : NULL;

		if (newcol) {
			if  (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
				
			newcol->closure = this;
		}
	}
	else if (prop == ResourcesProperty) {
		Value *v = GetValue (prop);
		ResourceDictionary *newcol = v ? v->AsResourceDictionary() : NULL;

		if (newcol) {
			if  (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
				
			newcol->closure = this;
		}
	}

	NotifyAttacheesOfPropertyChange (prop);
}

#if true
int
UIElement::DumpHierarchy (UIElement *obj)
{
	if (obj == NULL)
		return 0;
	
	int n = DumpHierarchy (obj->parent);
	for (int i = 0; i < n; i++)
		putchar (' ');
	printf ("%s (%p)\n", obj->GetTypeName(), obj);
	return n + 4;
}
#endif

void
UIElement::UpdateTransform ()
{
	cairo_matrix_t user_transform;
	
	//
	// What is more important, the space used by 6 doubles,
	// or the time spent calling the parent (that will call
	// DependencyObject->GetProperty to get the positions?
	//
	// Currently we go for thiner, but if we decide to go
	// for reduced computation, we should introduce the 
	// base transform in UIElement that will be updated by the
	// container on demand
	//
	uielement_get_render_affine (this, &user_transform);

	if (parent != NULL)
		parent->GetTransformFor (this, &absolute_xform);
	else
		cairo_matrix_init_identity (&absolute_xform);
	
	Point p = GetTransformOrigin ();
	cairo_matrix_translate (&absolute_xform, p.x, p.y);
	cairo_matrix_multiply (&absolute_xform, &user_transform, &absolute_xform);
	cairo_matrix_translate (&absolute_xform, -p.x, -p.y);
	//printf ("      Final position for %s x=%g y=%g\n", GetTypeName(), absolute_xform.x0, absolute_xform.y0);

	// a change in transform requires a change in our bounds, more than likely
	UpdateBounds ();
}

void
UIElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == UIElement::RenderTransformProperty) {
		UpdateTransform ();
	}
	else if (prop == UIElement::ClipProperty ||
		 prop == UIElement::OpacityMaskProperty) {

		Invalidate ();
	}
	
	Visual::OnSubPropertyChanged (prop, subprop);
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


// XXX this should really intersect with the bounding rect.
void
UIElement::Invalidate (Rect r)
{
	Surface *s = GetSurface ();
	
	if (s == NULL)
		return;

#ifdef DEBUG_INVALIDATE
	printf ("Requesting invalidate for object %p (%s) at %d %d - %d %d\n", 
		this, Type::Find(GetObjectType())->name,
		(int) r.x, (int)r.y, 
		(int)(r.w+2), (int)(r.h+2));
#endif

	gtk_widget_queue_draw_area (s->GetDrawingArea (),
				    (int) r.x, (int)r.y, 
				    (int)(r.w+2), (int)(r.h+2));
}

void
UIElement::Invalidate ()
{
	// invalidate the full bounds of this UIElement.
	Invalidate (bounds);
}

void
UIElement::HandleMotion (cairo_t *cr, int state, double x, double y, MouseCursor *cursor)
{
	if (cursor && *cursor == MouseCursorDefault)
		*cursor = (MouseCursor)GetValue (UIElement::CursorProperty)->AsInt32();

	MouseEventArgs e;
	e.state = state;
	e.x = x;
	e.y = y;

	Emit (MotionEvent, &e);
}

void
UIElement::HandleButtonPress (cairo_t *cr, int state, double x, double y)
{
	MouseEventArgs e;
	e.state = state;
	e.x = x;
	e.y = y;

	Emit (ButtonPressEvent, &e);
}

void
UIElement::HandleButtonRelease (cairo_t *cr, int state, double x, double y)
{
	MouseEventArgs e;
	e.state = state;
	e.x = x;
	e.y = y;

	Emit (ButtonReleaseEvent, &e);
}

void
UIElement::Enter (cairo_t *cr, int state, double x, double y)
{
	MouseEventArgs e;
	e.state = state;
	e.x = x;
	e.y = y;

	Emit (EnterEvent, &e);
}

void
UIElement::Leave ()
{
	Emit (LeaveEvent);
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

	if (s->GetMouseCapture() != this)
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
UIElement::DoRender (cairo_t *cr, int x, int y, int width, int height)
{

	cairo_pattern_t *mask = NULL;
	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);
	if (opacityMask != NULL) {
		cairo_save (cr);
		opacityMask->SetupBrush (cr, this);
		mask = cairo_get_source (cr);
		cairo_pattern_reference (mask);
		cairo_push_group (cr);
	}
	
	Render (cr, x, y, width, height);

	if (opacityMask != NULL) {
		cairo_pop_group_to_source (cr);
		cairo_rectangle (cr, bounds.x + 1, bounds.y + 1, bounds.w - 2, bounds.h - 2);
		cairo_clip (cr);
		cairo_mask (cr, mask);
		cairo_pattern_destroy (mask);
		cairo_restore (cr);
	}

	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);

#if SHOW_BOUNDING_BOXES
	cairo_restore (cr);
	cairo_save (cr);
	cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 1.0);
	cairo_set_line_width (cr, 2);
	cairo_rectangle (cr, bounds.x + 1, bounds.y + 1, bounds.w - 2, bounds.h - 2);
	cairo_stroke (cr);
	cairo_new_path (cr);
#endif
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

double
UIElement::GetTotalOpacity ()
{
	double opacity = uielement_get_opacity (this);
	// this is recursive to parents
	UIElement *uielement = this->parent;
	while (uielement) {
		double parent_opacity = uielement_get_opacity (uielement);
		if (parent_opacity < 1.0)
			opacity *= parent_opacity;
		// FIXME: we should be calling FrameworkElement::Parent
		uielement = uielement->parent;
	}

	return opacity;
}

Surface*
UIElement::GetSurface ()
{
	return parent == NULL ? NULL : parent->GetSurface();
}

DependencyProperty* UIElement::RenderTransformProperty;
DependencyProperty* UIElement::OpacityProperty;
DependencyProperty* UIElement::ClipProperty;
DependencyProperty* UIElement::OpacityMaskProperty;
DependencyProperty* UIElement::TriggersProperty;
DependencyProperty* UIElement::RenderTransformOriginProperty;
DependencyProperty* UIElement::CursorProperty;
DependencyProperty* UIElement::IsHitTestVisibleProperty;
DependencyProperty* UIElement::VisibilityProperty;
DependencyProperty* UIElement::ResourcesProperty;
DependencyProperty* UIElement::ZIndexProperty;

void
uielement_init (void)
{
	UIElement::RenderTransformProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransform", Type::TRANSFORM);
	UIElement::OpacityProperty = DependencyObject::Register (Type::UIELEMENT, "Opacity", new Value(1.0));
	UIElement::ClipProperty = DependencyObject::Register (Type::UIELEMENT, "Clip", Type::GEOMETRY);
	UIElement::OpacityMaskProperty = DependencyObject::Register (Type::UIELEMENT, "OpacityMask", Type::BRUSH);
	UIElement::TriggersProperty = DependencyObject::Register (Type::UIELEMENT, "Triggers", Type::TRIGGER_COLLECTION);
	UIElement::RenderTransformOriginProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransformOrigin", Type::POINT);
	UIElement::CursorProperty = DependencyObject::Register (Type::UIELEMENT, "Cursor", new Value ((gint32)MouseCursorDefault));
	UIElement::IsHitTestVisibleProperty = DependencyObject::Register (Type::UIELEMENT, "IsHitTestVisible", new Value (true));
	UIElement::VisibilityProperty = DependencyObject::Register (Type::UIELEMENT, "Visibility", new Value ((gint32)VisibilityVisible));
	UIElement::ResourcesProperty = DependencyObject::Register (Type::UIELEMENT, "Resources", Type::RESOURCE_DICTIONARY);
	UIElement::ZIndexProperty = DependencyObject::Register (Type::UIELEMENT, "ZIndex", new Value ((gint32)0));;
}

UIElement*
uielement_new ()
{
	return new UIElement ();
}

Surface*
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
	return item->parent;
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
