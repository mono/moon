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
#include "control.h"

void 
Control::UpdateTransform ()
{
	if (real_object){
		real_object->UpdateTransform ();
		absolute_xform = real_object->absolute_xform;
	}
}

void 
Control::Render (cairo_t *cr, int x, int y, int width, int height)
{
	if (real_object)
		real_object->DoRender (cr, x, y, width, height);
}

void 
Control::GetBounds ()
{
	if (real_object){
		real_object->GetBounds ();
		x1 = real_object->x1;
		y1 = real_object->y1;
		x2 = real_object->x2;
		y2 = real_object->y2;
		//printf ("CONTROL-CANVAS: Bounds obtained: %g %g %g %g\n", x1, y1, x2, y2);
	} else {
		x1 = y1 = x2 = y2 = 0;
	}
}

void 
Control::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	if (parent != NULL){
		parent->GetTransformFor (this, result);
	} else {
		cairo_matrix_init_identity (result);
	}
}

Point 
Control::GetTransformOrigin ()
{
	if (real_object)
		return real_object->GetTransformOrigin ();
	else
		return Point (0, 0);
}

bool 
Control::InsideObject (Surface *s, double x, double y)
{
	if (real_object)
		return real_object->InsideObject (s, x, y);
	else
		return false;
}

bool
Control::HandleMotion (Surface *s, int state, double x, double y)
{
	if (real_object)
		return real_object->HandleMotion (s, state, x, y);
	return false;
}

bool
Control::HandleButton (Surface *s, callback_mouse_event cb, int state, double x, double y)
{
	if (real_object)
		return real_object->HandleButton (s, cb, state, x, y);
	return false;
}

void 
Control::Enter (Surface *s, int state, double x, double y)
{
	if (real_object)
		real_object->Enter (s, state, x, y);
}

void 
Control::Leave (Surface *s)
{
	if (real_object)
		real_object->Leave (s);
}

void 
Control::OnPropertyChanged (DependencyProperty *prop)
{
	if (real_object)
		real_object->OnPropertyChanged (prop);
}

void 
Control::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (real_object)
		real_object->OnSubPropertyChanged (prop, subprop);
}

bool 
Control::OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child) 
{
	if (real_object)
		real_object->OnChildPropertyChanged (prop, child);
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
control_initialize_from_xaml (Control *control, const char *xaml,
			      xaml_create_custom_element_callback *cecb,
			      xaml_set_custom_attribute_callback *sca,
			      xaml_hookup_event_callback *hue,
			      Type::Kind *element_type)
{
	// No callback, figure out how this will work in the plugin to satisfy deps

	UIElement *element = xaml_create_from_str (xaml, false, cecb, sca, hue, element_type);
	if (element == NULL)
		return NULL;

	if (control->real_object){
		control->real_object->parent = NULL;
		base_unref (control->real_object);
	}

	// 
	// It does not matter, they are the same
	//
	control->real_object = (FrameworkElement *) element;
	control->real_object->parent = control;

	// sink the ref, we own this
	base_ref (control->real_object);

	return element;
}

Control *
control_new (void)
{
	return new Control ();
}
