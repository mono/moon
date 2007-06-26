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
#include "canvas.h"
#include "collection.h"
#include "brush.h"
#include "transform.h"
#include "runtime.h"

/**
 * item_getbounds:
 * @item: the item to update the bounds of
 *
 * Does this by requesting bounds update to all of its parents. 
 */
void
item_update_bounds (UIElement *item)
{
	item->updatebounds();
}

void
UIElement::updatebounds ()
{
	double cx1 = x1;
	double cy1 = y1;
	double cx2 = x2;
	double cy2 = y2;
	
	getbounds ();
	
	//
	// If we changed, notify the parent to recompute its bounds
	//
	if (x1 != cx1 || y1 != cy1 || y2 != cy2 || x2 != cx2){
		if (parent != NULL)
			parent->updatebounds();
	}
}

void
UIElement::get_xform_for (UIElement *item, cairo_matrix_t *result)
{
	printf ("get_xform_for called on a non-container, you must implement this in your container\n");
	exit (1);
}

void 
item_invalidate (UIElement *item)
{
	Surface *s = item_get_surface (item);
	double res [6];
	
	if (s == NULL)
		return;

#ifdef DEBUG_INVALIDATE
	printf ("Requesting invalidate for object %p (%s) at %d %d - %d %d\n", 
		item, Type::Find(item->GetObjectType())->name,
		(int) item->x1, (int)item->y1, 
		(int)(item->x2-item->x1+1), (int)(item->y2-item->y1+1));
#endif
	// 
	// Note: this is buggy: why do we need to queue the redraw on the toplevel
	// widget (s->data) and does not work with the drawing area?
	//
	gtk_widget_queue_draw_area ((GtkWidget *)s->drawing_area, 
				    (int) item->x1, (int)item->y1, 
				    (int)(item->x2-item->x1+2), (int)(item->y2-item->y1+2));
}

void 
item_set_transform_origin (UIElement *item, Point p)
{
	item->SetValue (UIElement::RenderTransformOriginProperty, p);
}

void
item_get_render_affine (UIElement *item, cairo_matrix_t *result)
{
	Value* v = item->GetValue (UIElement::RenderTransformProperty);
	if (v == NULL)
		cairo_matrix_init_identity (result);
	else {
		Transform *t = v->AsTransform();
		t->GetTransform (result);
	}
}

UIElement::UIElement () : opacityMask(NULL), parent(NULL), flags (0), x1 (0), y1(0), x2(0), y2(0)
{
	cairo_matrix_init_identity (&absolute_xform);

	this->SetValue (UIElement::TriggersProperty, Value (new TriggerCollection ()));
	this->SetValue (UIElement::ResourcesProperty, Value (new ResourceCollection ()));
}

UIElement::~UIElement ()
{
	if (opacityMask != NULL) {
		opacityMask->Detach (NULL, this);
		opacityMask->unref ();
	}
}

//
// Intercept any changes to the triggers property and mirror that into our
// own variable
//
void
UIElement::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == UIElement::OpacityProperty ||
	    prop == UIElement::ZIndexProperty) {
		item_invalidate (this);
	} else if (prop == UIElement::VisibilityProperty) {
		// XXX we need a FullInvalidate if this is changing
		// from or to Collapsed, but as there's no way to tell that...
		FullInvalidate (true);
	} else if (prop == UIElement::ClipProperty) {
		FullInvalidate (false);
	} else if (prop == UIElement::OpacityMaskProperty) {
		if (opacityMask != NULL) {
			opacityMask->Detach (NULL, this);
			opacityMask->unref ();
		}
		
		if ((opacityMask = uielement_get_opacity_mask (this)) != NULL) {
			opacityMask->Attach (NULL, this);
			opacityMask->ref ();
		}
		
		FullInvalidate (false);
	} else if (prop == RenderTransformProperty || prop == RenderTransformOriginProperty) {
		FullInvalidate (true);
	} else if (prop == TriggersProperty){
		Value *v = GetValue (prop);
		TriggerCollection *newcol = v ?  v->AsTriggerCollection() : NULL;

		if (newcol) {
			if  (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
				
			newcol->closure = this;
		}
	} else if (prop == ResourcesProperty) {
		Value *v = GetValue (prop);
		ResourceCollection *newcol = v ? v->AsResourceCollection() : NULL;

		if (newcol) {
			if  (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
				
			newcol->closure = this;
		}
	} else {
		Visual::OnPropertyChanged (prop);
	}
}

#if true
int
UIElement::dump_hierarchy (UIElement *obj)
{
	if (obj == NULL)
		return 0;
	
	int n = dump_hierarchy (obj->parent);
	for (int i = 0; i < n; i++)
		putchar (' ');
	printf ("%s (%p)\n", dependency_object_get_name (obj), obj);
	return n + 4;
}
#endif

void
UIElement::update_xform ()
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
	item_get_render_affine (this, &user_transform);

	if (parent != NULL)
		parent->get_xform_for (this, &absolute_xform);
	else
		cairo_matrix_init_identity (&absolute_xform);
	
	Point p = getxformorigin ();
	cairo_matrix_translate (&absolute_xform, p.x, p.y);
	cairo_matrix_multiply (&absolute_xform, &user_transform, &absolute_xform);
	cairo_matrix_translate (&absolute_xform, -p.x, -p.y);
	//printf ("      Final position for %s x=%g y=%g\n", dependency_object_get_name (this), absolute_xform.x0, absolute_xform.y0);
}

void
UIElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == UIElement::RenderTransformProperty ||
	    prop == UIElement::RenderTransformOriginProperty) {
		FullInvalidate (true);
	} else if (prop == UIElement::ClipProperty ||
		   prop == UIElement::OpacityMaskProperty) {

		// Maybe this could also be just item_invalidate?
		FullInvalidate (false);
	} else if (prop == UIElement::OpacityProperty) {
	  	item_invalidate (this);
	} else if (prop == UIElement::VisibilityProperty) {
		// XXX we need a FullInvalidate if this is changing
		// from or to Collapsed, but as there's no way to tell that...
		FullInvalidate (true);
	} else if (Type::Find (subprop->type)->IsSubclassOf (Type::BRUSH)) {
		item_invalidate (this);
	}
	
	Visual::OnSubPropertyChanged (prop, subprop);
}

void
UIElement::OnLoaded ()
{
	if (!(flags & UIElement::IS_LOADED)) {
		flags |= UIElement::IS_LOADED;
		events->Emit ("Loaded");
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
	item_invalidate (this);
	if (rendertransform)
		update_xform ();
	item_update_bounds (this);
	item_invalidate (this);
}

void
item_set_render_transform (UIElement *item, Transform *transform)
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

bool
UIElement::inside_object (Surface *s, double x, double y)
{
	printf ("UIElement derivatives should implement inside object\n");
}

bool
UIElement::handle_motion (Surface *s, int state, double x, double y)
{
	if (inside_object (s, x, y)){
		s->cb_motion (this, state, x, y);
		return true;
	} 
	return false;
}

bool
UIElement::handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y)
{
	if (inside_object (s, x, y)){
		cb (this, state, x, y);
		return true;
	}
	return false;
}

void
UIElement::enter (Surface *s, int state, double x, double y)
{
	s->cb_enter (this, state, x, y);
}

void
UIElement::leave (Surface *s)
{
	s->cb_mouse_leave (this);
}

void
UIElement::getbounds ()
{
	g_warning ("UIElement:getbounds has been called. The derived class %s should have overridden it.",
		   dependency_object_get_name (this));
}

void
UIElement::dorender (cairo_t *cr, int x, int y, int width, int height)
{
	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);
	render (cr, x, y, width, height);
	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);
}

void
UIElement::render (cairo_t *cr, int x, int y, int width, int height)
{
	g_warning ("UIElement:render has been called. The derived class %s should have overridden it.",
		   dependency_object_get_name (this));
}

void
UIElement::get_size_for_brush (cairo_t *cr, double *width, double *height)
{
	double x1, y1, x2, y2;
	
	cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	
	*height = fabs (y2 - y1);
	*width = fabs (x2 - x1);
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

Surface *
item_get_surface (UIElement *item)
{
	if (Type::Find (item->GetObjectType())->IsSubclassOf (Type::CANVAS)) {
		Canvas *canvas = (Canvas *) item;
		if (canvas->surface)
			return canvas->surface;
	}

	if (item->parent != NULL)
		return item_get_surface (item->parent);

	return NULL;
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
	UIElement::CursorProperty = DependencyObject::Register (Type::UIELEMENT, "Cursor", Type::INT32);
	UIElement::IsHitTestVisibleProperty = DependencyObject::Register (Type::UIELEMENT, "IsHitTestVisible", Type::BOOL);
	UIElement::VisibilityProperty = DependencyObject::Register (Type::UIELEMENT, "Visibility", new Value ((gint32)VisibilityVisible));
	UIElement::ResourcesProperty = DependencyObject::Register (Type::UIELEMENT, "Resources", Type::RESOURCE_COLLECTION);
	UIElement::ZIndexProperty = DependencyObject::Register (Type::UIELEMENT, "ZIndex", new Value ((gint32)0));;
}

