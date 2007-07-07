/*
 * frameworkelement.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>

#include <gtk/gtk.h>

#include "frameworkelement.h"

FrameworkElement::FrameworkElement ()
{
}

void
FrameworkElement::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::FRAMEWORKELEMENT) {
		UIElement::OnPropertyChanged (prop);
		return;
	}

	if (prop == FrameworkElement::WidthProperty ||
	    prop == FrameworkElement::HeightProperty) {
		FullInvalidate (false);
	}

	NotifyAttacheesOfPropertyChange (prop);
}

bool
FrameworkElement::InsideObject (cairo_t *cr, double x, double y)
{
	double nx = x, ny = y;

	uielement_transform_point (this, &nx, &ny);
	if (nx < 0 || ny < 0 || nx > framework_element_get_width (this) || ny > framework_element_get_height (this))
		return false;

	return true;
}

void
FrameworkElement::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	double x1, x2, y1, y2;
		
	x1 = y1 = 0.0;
	x2 = framework_element_get_width (this);
	y2 = framework_element_get_height (this);

	cairo_matrix_transform_point (&absolute_xform, &x1, &y1);
	cairo_matrix_transform_point (&absolute_xform, &x2, &y2);

	*width = x2 - x1;
	*height = y2 - y1;
}

double
framework_element_get_height (FrameworkElement *framework_element)
{
	return framework_element->GetValue (FrameworkElement::HeightProperty)->AsDouble();
}

void
framework_element_set_height (FrameworkElement *framework_element, double height)
{
	framework_element->SetValue (FrameworkElement::HeightProperty, Value (height));
}

double
framework_element_get_width (FrameworkElement *framework_element)
{
	return framework_element->GetValue (FrameworkElement::WidthProperty)->AsDouble();
}

void
framework_element_set_width (FrameworkElement *framework_element, double width)
{
	framework_element->SetValue (FrameworkElement::WidthProperty, Value (width));
}

DependencyProperty* FrameworkElement::HeightProperty;
DependencyProperty* FrameworkElement::WidthProperty;

void
framework_element_init (void)
{
	FrameworkElement::HeightProperty = DependencyObject::Register (Type::FRAMEWORKELEMENT, "Height", new Value (0.0));
	FrameworkElement::WidthProperty = DependencyObject::Register (Type::FRAMEWORKELEMENT, "Width", new Value (0.0));
}

