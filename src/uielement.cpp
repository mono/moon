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
#include "dirty.h"

#define SHOW_BOUNDING_BOXES 0

void
UIElement::UpdateBounds (bool force_redraw_of_new_bounds)
{
	add_dirty_element (this, DirtyBounds);
	if (force_redraw_of_new_bounds)
		add_dirty_element (this, DirtyInvalidate);
}

void
UIElement::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	printf ("GetTransformFor called on a non-container, you must implement this in your container\n");
	exit (1);
}

UIElement::UIElement () : opacityMask(NULL), parent(NULL), flags (UIElement::RENDER_VISIBLE | UIElement::HIT_TEST_VISIBLE)
{
	LoadedEvent = RegisterEvent ("Loaded");
	MotionEvent = RegisterEvent ("MouseMove");
	ButtonPressEvent = RegisterEvent ("MouseLeftButtonDown");
	ButtonReleaseEvent = RegisterEvent ("MouseLeftButtonUp");
	KeyDownEvent = RegisterEvent ("KeyDown");
	KeyUpEvent = RegisterEvent ("KeyUp");
	EnterEvent = RegisterEvent ("MouseEnter");
	LeaveEvent = RegisterEvent ("MouseLeave");
	InvalidatedEvent = RegisterEvent("Invalidated");

	bounds = Rect (0,0,0,0);
	cairo_matrix_init_identity (&absolute_xform);

	dirty_flags = 0;
	dirty_rect = Rect (0,0,0,0);

	this->SetValue (UIElement::TriggersProperty, Value (new TriggerCollection ()));
	this->SetValue (UIElement::ResourcesProperty, Value (new ResourceDictionary ()));

	ComputeLocalTransform ();
	ComputeTotalOpacity ();
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
	  
	if (prop == UIElement::OpacityProperty) {
		UpdateTotalOpacity ();
	}
	else if (prop == UIElement::ZIndexProperty) {
		Invalidate ();
	}
	else if (prop == UIElement::VisibilityProperty) {
		switch (GetValue (prop)->AsInt32()) {
		case VisibilityVisible:
			flags |= UIElement::RENDER_VISIBLE;
			Invalidate ();
			break;
		case VisibilityCollapsed:
			FullInvalidate (true);
			flags &= ~UIElement::RENDER_VISIBLE;
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
UIElement::UpdateTotalOpacity ()
{
	add_dirty_element (this, DirtyOpacity);
}

void
UIElement::ComputeTotalOpacity ()
{
	double local_opacity = GetValue (OpacityProperty)->AsDouble();
	total_opacity = local_opacity * (parent ? parent->GetTotalOpacity () : 1.0);
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

	if (parent != NULL)
		parent->GetTransformFor (this, &parent_transform);
	else
		cairo_matrix_init_identity (&parent_transform);
}

void
UIElement::ComputeTransform ()
{
	if (parent != NULL)
		absolute_xform = parent->absolute_xform;
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

void
UIElement::Invalidate (Rect r)
{
	if (!GetVisible())
		return;

#ifdef DEBUG_INVALIDATE
	printf ("Requesting invalidate for object %p (%s) at %d %d - %d %d\n", 
		this, Type::Find(GetObjectType())->name,
		(int) r.x, (int)r.y, 
		(int)(r.w+2), (int)(r.h+2));
#endif

	add_dirty_element (this, DirtyInvalidate);
	if (dirty_rect.IsEmpty())
		dirty_rect = r;
	else
		dirty_rect = dirty_rect.Union (r);

	Emit (InvalidatedEvent);
}

void
UIElement::Invalidate ()
{
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
UIElement::HandleKeyDown (cairo_t *cr, int state, Key key, int platform_key_code)
{
	KeyboardEventArgs e;
	e.state = state;
	e.key = key;
	e.platformcode = platform_key_code;

	Emit (KeyDownEvent, &e);
}

void
UIElement::HandleKeyUp (cairo_t *cr, int state, Key key, int platform_key_code)
{
	KeyboardEventArgs e;
	e.state = state;
	e.key = key;
	e.platformcode = platform_key_code;

	Emit (KeyUpEvent, &e);
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
	if (total_opacity == 0.0)
		return;

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
	UIElement::RenderTransformOriginProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransformOrigin", Type::POINT);
	UIElement::RenderTransformProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransform", Type::TRANSFORM);
	UIElement::ResourcesProperty = DependencyObject::Register (Type::UIELEMENT, "Resources", Type::RESOURCE_DICTIONARY);
	UIElement::TagProperty = DependencyObject::Register (Type::UIELEMENT, "Tag", Type::STRING);
	UIElement::TriggersProperty = DependencyObject::Register (Type::UIELEMENT, "Triggers", Type::TRIGGER_COLLECTION);
	UIElement::VisibilityProperty = DependencyObject::Register (Type::UIELEMENT, "Visibility", new Value ((gint32)VisibilityVisible));
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
