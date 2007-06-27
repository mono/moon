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

void
UIElement::UpdateBounds ()
{
	double cx1 = x1;
	double cy1 = y1;
	double cx2 = x2;
	double cy2 = y2;
	
	GetBounds ();
	
	//
	// If we changed, notify the parent to recompute its bounds
	//
	if (x1 != cx1 || y1 != cy1 || y2 != cy2 || x2 != cx2){
		if (parent != NULL)
			parent->UpdateBounds();
	}
}

void
UIElement::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	printf ("get_xform_for called on a non-container, you must implement this in your container\n");
	exit (1);
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
		Invalidate ();
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
UIElement::DumpHierarchy (UIElement *obj)
{
	if (obj == NULL)
		return 0;
	
	int n = DumpHierarchy (obj->parent);
	for (int i = 0; i < n; i++)
		putchar (' ');
	printf ("%s (%p)\n", dependency_object_get_name (obj), obj);
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
	  	Invalidate ();
	} else if (prop == UIElement::VisibilityProperty) {
		// XXX we need a FullInvalidate if this is changing
		// from or to Collapsed, but as there's no way to tell that...
		FullInvalidate (true);
	} else if (Type::Find (subprop->type)->IsSubclassOf (Type::BRUSH)) {
		Invalidate ();
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
	Invalidate ();
	if (rendertransform)
		UpdateTransform ();
	UpdateBounds ();
	Invalidate ();
}

void
UIElement::Invalidate ()
{
	Surface *s = GetSurface ();
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
				    (int) x1, (int)y1, 
				    (int)(x2-x1+2), (int)(y2-y1+2));
}


bool
UIElement::InsideObject (Surface *s, double x, double y)
{
	printf ("UIElement derivatives should implement inside object\n");
}

bool
UIElement::HandleMotion (Surface *s, int state, double x, double y)
{
	if (InsideObject (s, x, y)){
		s->cb_motion (this, state, x, y);
		return true;
	} 
	return false;
}

bool
UIElement::HandleButton (Surface *s, callback_mouse_event cb, int state, double x, double y)
{
	if (InsideObject (s, x, y)){
		cb (this, state, x, y);
		return true;
	}
	return false;
}

void
UIElement::Enter (Surface *s, int state, double x, double y)
{
	s->cb_enter (this, state, x, y);
}

void
UIElement::Leave (Surface *s)
{
	s->cb_mouse_leave (this);
}

void
UIElement::GetBounds ()
{
	g_warning ("UIElement:getbounds has been called. The derived class %s should have overridden it.",
		   dependency_object_get_name (this));
}

void
UIElement::DoRender (cairo_t *cr, int x, int y, int width, int height)
{
	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);
	Render (cr, x, y, width, height);
	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);
}

void
UIElement::Render (cairo_t *cr, int x, int y, int width, int height)
{
	g_warning ("UIElement:render has been called. The derived class %s should have overridden it.",
		   dependency_object_get_name (this));
}

void
UIElement::GetSizeForBrush (cairo_t *cr, double *width, double *height)
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
	UIElement::CursorProperty = DependencyObject::Register (Type::UIELEMENT, "Cursor", Type::INT32);
	UIElement::IsHitTestVisibleProperty = DependencyObject::Register (Type::UIELEMENT, "IsHitTestVisible", Type::BOOL);
	UIElement::VisibilityProperty = DependencyObject::Register (Type::UIELEMENT, "Visibility", new Value ((gint32)VisibilityVisible));
	UIElement::ResourcesProperty = DependencyObject::Register (Type::UIELEMENT, "Resources", Type::RESOURCE_COLLECTION);
	UIElement::ZIndexProperty = DependencyObject::Register (Type::UIELEMENT, "ZIndex", new Value ((gint32)0));;
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

