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
FrameworkElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::FRAMEWORKELEMENT) {
		UIElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == FrameworkElement::WidthProperty ||
	    args->property == FrameworkElement::HeightProperty) {
		Point p = GetRenderTransformOrigin ();

		/* normally we'd only update the bounds of this
		   element on a width/height change, but if the render
		   transform is someplace other than (0,0), the
		   transform needs to be updated as well. */
		FullInvalidate (p.x != 0.0 || p.y != 0.0);
	}

	NotifyListenersOfPropertyChange (args);
}

void
FrameworkElement::ComputeBounds ()
{
	double x1, x2, y1, y2;
	
	x1 = y1 = 0.0;
	x2 = GetValue (FrameworkElement::WidthProperty)->AsDouble ();
	y2 = GetValue (FrameworkElement::HeightProperty)->AsDouble ();
	
	if (x2 != 0.0 && y2 != 0.0)
		bounds = IntersectBoundsWithClipPath (Rect (x1,y1,x2,y2), false).Transform (&absolute_xform);
}

bool
FrameworkElement::InsideObject (cairo_t *cr, double x, double y)
{
	double height = GetValue (FrameworkElement::HeightProperty)->AsDouble ();
	double width = GetValue (FrameworkElement::WidthProperty)->AsDouble ();
	double nx = x, ny = y;

	uielement_transform_point (this, &nx, &ny);
	if (nx < 0 || ny < 0 || nx > width || ny > height)
		return false;

	return UIElement::InsideObject (cr, x, y);
}

void
FrameworkElement::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*width = GetValue (FrameworkElement::WidthProperty)->AsDouble ();
	*height = GetValue (FrameworkElement::HeightProperty)->AsDouble ();
}

FrameworkElement *
framework_element_new (void)
{
	return new FrameworkElement ();
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

DependencyProperty *FrameworkElement::HeightProperty;
DependencyProperty *FrameworkElement::WidthProperty;

void
framework_element_init (void)
{
	FrameworkElement::HeightProperty = DependencyObject::Register (Type::FRAMEWORKELEMENT, "Height", new Value (0.0));
	FrameworkElement::WidthProperty = DependencyObject::Register (Type::FRAMEWORKELEMENT, "Width", new Value (0.0));
}
