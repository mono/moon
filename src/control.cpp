/*
 * control.cpp:
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#define Visual _XVisual
#include <cairo-xlib.h>
#undef Visual
#include "rect.h"
#include "runtime.h"
#include "control.h"
#include "canvas.h"

void 
Control::Render (cairo_t *cr, Region *region)
{
	if (real_object)
		real_object->DoRender (cr, region);
}

void
Control::FrontToBack (Region *surface_region, List *render_list)
{
	if (real_object)
		return real_object->FrontToBack (surface_region, render_list);
}

void 
Control::ComputeBounds ()
{
	if (real_object) {
		bounds = real_object->GetBounds ();
		bounds_with_children = real_object->GetSubtreeBounds ();
	}
	else {
		bounds = Rect (0, 0, 0, 0);
		bounds_with_children = Rect (0, 0, 0, 0);
	}

	double x1, x2, y1, y2;
	
	x1 = y1 = 0.0;
	x2 = framework_element_get_width (this);
	y2 = framework_element_get_height (this);

	if (x2 != 0.0 && y2 != 0.0) {

		Rect fw_rect = Rect (x1,y1,x2,y2);
		
		if (real_object)
			bounds = bounds.Union (fw_rect);
		else
			bounds = fw_rect;
	       
	}
	
	bounds = IntersectBoundsWithClipPath (bounds, false).Transform (&absolute_xform);
	bounds_with_children = IntersectBoundsWithClipPath (bounds_with_children.Union (bounds), false);
							    
}

void
Control::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == Canvas::TopProperty || subobj_args->property == Canvas::LeftProperty) {
		real_object->UpdateTransform ();
	}

	UIElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Control::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	// Compute left/top if its attached to the item
	Value *val_top = item->GetValue (Canvas::TopProperty);
	double top = val_top == NULL ? 0.0 : val_top->AsDouble();

	Value *val_left = item->GetValue (Canvas::LeftProperty);
	double left = val_left == NULL ? 0.0 : val_left->AsDouble();
		
	cairo_matrix_init_translate (result, left, top);
}

bool 
Control::InsideObject (cairo_t *cr, double x, double y)
{
	if (real_object)
		return real_object->InsideObject (cr, x, y);
	else
		return false;
}

void
Control::HitTest (cairo_t *cr, double x, double y, List *uielement_list)
{
	if (InsideObject (cr, x, y)) {
		uielement_list->Prepend (new UIElementNode (this));
		real_object->HitTest (cr, x, y, uielement_list);
	}
}

void
Control::OnLoaded ()
{
	if (real_object)
		real_object->OnLoaded ();

	FrameworkElement::OnLoaded ();
}

Control::~Control ()
{
	if (real_object)
		base_unref (real_object);
}

UIElement*
Control::InitializeFromXaml (const char *xaml,
			     Type::Kind *element_type, XamlLoader *loader)
{
	// No callback, figure out how this will work in the plugin to satisfy deps
	UIElement *element = (UIElement*)xaml_create_from_str (loader, xaml, false, element_type);
	if (element == NULL)
		return NULL;

	if (real_object){
		real_object->SetVisualParent (NULL);
		base_unref (real_object);
	}

	real_object = (FrameworkElement *) element;
	real_object->SetVisualParent (this);

	real_object->AddPropertyChangeListener (this);
	real_object->UpdateTotalRenderVisibility ();
	real_object->UpdateTransform ();
	UpdateBounds ();

	if (loader)
		SetSurface (loader->GetSurface ());

	return element;
}

UIElement* 
control_initialize_from_xaml (Control *control, const char *xaml,
			      Type::Kind *element_type)
{
	return control->InitializeFromXaml (xaml, element_type, NULL);
}

UIElement* 
control_initialize_from_xaml_callbacks (Control *control, const char *xaml,
			      Type::Kind *element_type, XamlLoader *loader)
{
	return control->InitializeFromXaml (xaml, element_type, loader);
}

Control *
control_new (void)
{
	return new Control ();
}
