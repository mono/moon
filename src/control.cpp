/*
 * runtime.cpp: Core surface and canvas definitions.
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

void 
Control::update_xform ()
{
	if (real_object){
		real_object->update_xform ();
		absolute_xform = real_object->absolute_xform;
	}
}

void 
Control::render (Surface *surface, int x, int y, int width, int height)
{
	if (real_object)
		real_object->dorender (surface, x, y, width, height);
}

void 
Control::getbounds ()
{
	if (real_object){
		real_object->getbounds ();
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
Control::get_xform_for (UIElement *item, cairo_matrix_t *result)
{
	if (parent != NULL){
		parent->get_xform_for (this, result);
	} else {
		cairo_matrix_init_identity (result);
	}
}

Point 
Control::getxformorigin ()
{
	if (real_object)
		return real_object->getxformorigin ();
	else
		return Point (0, 0);
}

bool 
Control::inside_object (Surface *s, double x, double y)
{
	if (real_object)
		return real_object->inside_object (s, x, y);
	else
		return false;
}

bool
Control::handle_motion (Surface *s, int state, double x, double y)
{
	if (real_object)
		return real_object->handle_motion (s, state, x, y);
	return false;
}

bool
Control::handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y)
{
	if (real_object)
		return real_object->handle_button (s, cb, state, x, y);
	return false;
}

void 
Control::enter (Surface *s, int state, double x, double y)
{
	if (real_object)
		real_object->enter (s, state, x, y);
}

void 
Control::leave (Surface *s)
{
	if (real_object)
		real_object->leave (s);
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

Control::~Control ()
{
	if (real_object)
		base_unref (real_object);
}

UIElement* 
control_initialize_from_xaml (Control *control, const char *xaml, Type::Kind *element_type)
{
	// No callback, figure out how this will work in the plugin to satisfy deps

	UIElement *element = xaml_create_from_str (xaml, false, NULL, NULL, NULL, element_type);
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
