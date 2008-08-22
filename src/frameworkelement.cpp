/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * frameworkelement.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>

#include <math.h>

#include "frameworkelement.h"
#include "trigger.h"
#include "thickness.h"
#include "collection.h"

FrameworkElement::FrameworkElement ()
{
	measure_cb = NULL;
	arrange_cb = NULL;
}

void
FrameworkElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::FRAMEWORKELEMENT) {
		UIElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == FrameworkElement::WidthProperty ||
	    args->property == FrameworkElement::HeightProperty) {
		Point *p = GetRenderTransformOrigin ();

		/* normally we'd only update the bounds of this
		   element on a width/height change, but if the render
		   transform is someplace other than (0,0), the
		   transform needs to be updated as well. */
		FullInvalidate (p->x != 0.0 || p->y != 0.0);
	}

	NotifyListenersOfPropertyChange (args);
}

void
FrameworkElement::ComputeBounds ()
{
	double x1, x2, y1, y2;
	
	x1 = y1 = 0.0;
	y2 = GetHeight ();
	x2 = GetWidth ();
	
	if (x2 != 0.0 && y2 != 0.0)
		bounds = IntersectBoundsWithClipPath (Rect (x1,y1,x2,y2), false).Transform (&absolute_xform);
}

bool
FrameworkElement::InsideObject (cairo_t *cr, double x, double y)
{
	double height = GetHeight ();
	double width = GetWidth ();
	double nx = x, ny = y;
	
	TransformPoint (&nx, &ny);
	if (nx < 0 || ny < 0 || nx > width || ny > height)
		return false;
	
	return UIElement::InsideObject (cr, x, y);
}

void
FrameworkElement::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = GetHeight ();
	*width = GetWidth ();
}

void
FrameworkElement::Measure (Size availableSize)
{
	SetDesiredSize (MeasureOverride (availableSize));
}

Size
FrameworkElement::MeasureOverride (Size availableSize)
{
	if (measure_cb)
		return (*measure_cb)(availableSize);

	return availableSize;
}


// not sure about the disconnect between these two methods..  I would
// imagine both should take Rects and ArrangeOverride would return a
// rectangle as well..
void
FrameworkElement::Arrange (Rect finalRect)
{
	Size finalSize (finalRect.w, finalRect.h);
	Size r = ArrangeOverride (finalSize);

	g_warning ("more here in FrameworkElement::Arrange.  move the bounds or something?  set properties?  who knows!?");
}

Size
FrameworkElement::ArrangeOverride (Size finalSize)
{
	if (arrange_cb)
		return (*arrange_cb)(finalSize);

	return finalSize;
}

void
FrameworkElement::RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb)
{
	this->measure_cb = measure_cb;
	this->arrange_cb = arrange_cb;
}
