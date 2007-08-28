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
#include "runtime.h"
#include "control.h"
#include "canvas.h"

void 
Control::Render (cairo_t *cr, int x, int y, int width, int height)
{
	if (real_object)
		real_object->DoRender (cr, x, y, width, height);
}

void 
Control::ComputeBounds ()
{
	if (real_object) {
		bounds = real_object->GetBounds ();
	}
	else {
		bounds = Rect (0, 0, 0, 0);
	}

	double x1, x2, y1, y2;
	
	x1 = y1 = 0.0;
	x2 = framework_element_get_width (this);
	y2 = framework_element_get_height (this);

	if (x2 != 0.0 && y2 != 0.0) {

		Rect fw_rect = bounding_rect_for_transformed_rect (&absolute_xform,
								   Rect (x1,y1,x2,y2));

		if (real_object)
			bounds = bounds.Union (fw_rect);
		else
			bounds = fw_rect;
	}
}

void
Control::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (subprop == Canvas::TopProperty || subprop == Canvas::LeftProperty) {
		real_object->UpdateTransform ();
	}
}

void
Control::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	// XXX this is copied from the canvas implementation due to some
	// funky, funky behavior we're seeing.

	*result = absolute_xform;

	// Compute left/top if its attached to the item
	Value *val_top = item->GetValue (Canvas::TopProperty);
	double top = val_top == NULL ? 0.0 : val_top->AsDouble();

	Value *val_left = item->GetValue (Canvas::LeftProperty);
	double left = val_left == NULL ? 0.0 : val_left->AsDouble();
		
	cairo_matrix_translate (result, left, top);
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
Control::HandleMotion (cairo_t *cr, int state, double x, double y, MouseCursor *cursor)
{
	if (real_object)
		real_object->HandleMotion (cr, state, x, y, cursor);
	FrameworkElement::HandleMotion (cr, state, x, y, NULL);
}

void
Control::HandleButtonPress (cairo_t *cr, int state, double x, double y)
{
	if (real_object)
		real_object->HandleButtonPress (cr, state, x, y);
	FrameworkElement::HandleButtonPress (cr, state, x, y);
}

void
Control::HandleButtonRelease (cairo_t *cr, int state, double x, double y)
{
	if (real_object)
		real_object->HandleButtonRelease (cr, state, x, y);
	FrameworkElement::HandleButtonRelease (cr, state, x, y);
}

void 
Control::Enter (cairo_t *cr, int state, double x, double y)
{
	if (real_object){
		FrameworkElement::Enter (cr, state, x, y);
		real_object->Enter (cr, state, x, y);
	}
}

void 
Control::Leave ()
{
	if (real_object){
		real_object->Leave ();
		FrameworkElement::Leave ();
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
			     Type::Kind *element_type)
{
	// No callback, figure out how this will work in the plugin to satisfy deps
	UIElement *element = (UIElement*)xaml_create_from_str (xaml, false, element_type);
	if (element == NULL)
		return NULL;

	if (real_object){
		real_object->parent = NULL;
		base_unref (real_object);
	}

	real_object = (FrameworkElement *) element;
	real_object->parent = this;

	// sink the ref, we own this
	base_ref (real_object);

	real_object->Attach (NULL,this);
	real_object->UpdateTotalOpacity ();
	real_object->UpdateTransform ();
	UpdateBounds ();

	return element;
}

UIElement* 
control_initialize_from_xaml (Control *control, const char *xaml,
			      Type::Kind *element_type)
{
	return control->InitializeFromXaml (xaml, element_type);
}

Control *
control_new (void)
{
	return new Control ();
}
