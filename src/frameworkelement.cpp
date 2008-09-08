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
	extents = Rect (0.0, 0.0, GetWidth (), GetHeight ());
	bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
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

	// XXX this is hack to get around the 0.0 w/h defaults we still
	// have in place due to 1.0
	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
	Size specified = Size (vw ? GetWidth () : NAN, vh ? GetHeight () : NAN);
	
#if SL_2_0
	Thickness margin = *GetMargin ();
	Size size = availableSize.GrowBy (-margin);

	size = size.Min (specified);
	size = size.Max (specified);

	size = size.Min (GetMaxWidth (), GetMaxHeight ());
	size = size.Max (GetMinWidth (), GetMinHeight ());
#else
	Size size = availableSize;

	size = size.Min (specified);
	size = size.Max (specified);
#endif

	if (measure_cb)
		size = (*measure_cb)(size);
	else
		size = MeasureOverride (size);

	// XXX ugly hack to fake some sort of exception case
	if (isnan (size.width) || isnan (size.height)) {
		SetDesiredSize (Size (0,0));
		return;
	}

	// postcondition the results
	size = size.Min (specified);
	size = size.Max (specified);

#if SL_2_0
	size = size.Min (GetMaxWidth (), GetMaxHeight ());
	size = size.Max (GetMinWidth (), GetMinHeight ());

	size = size.GrowBy (margin);
#endif
	size = size.Min (availableSize);

	SetDesiredSize (size);
}

Size
FrameworkElement::MeasureOverride (Size availableSize)
{
	if (!GetVisualParent ())
		return Size (NAN, NAN);

	return Size (0,0);
}


// not sure about the disconnect between these two methods..  I would
// imagine both should take Rects and ArrangeOverride would return a
// rectangle as well..
void
FrameworkElement::Arrange (Rect finalRect)
{
	Size finalSize = GetDesiredSize ();
	Size size = GetDesiredSize ();

#if SL_2_0
	Thickness margin = *GetMargin ();
	finalRect = finalRect.GrowBy (-margin);
#endif
	// XXX this is hack to get around the 0.0 w/h defaults we still
	// have in place due to 1.0
	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
	Size specified = Size (vw ? GetWidth () : NAN, vh ? GetHeight () : NAN);

	size = size.Min (finalRect.width, finalRect.height);
	
	if (arrange_cb)
		size = (*arrange_cb)(size);
	else
		size = ArrangeOverride (size);


	// postcondition the results
	size = size.Min (specified);
	size = size.Max (specified);

#if SL_2_0
	size = size.Min (GetMaxWidth (), GetMaxHeight ());
	size = size.Max (GetMinWidth (), GetMinHeight ());

	size = size.GrowBy (margin);
#endif
	SetRenderSize (size);

	size = size.Min (finalRect.width, finalRect.height);
	SetDesiredSize (size);
	
	// XXX what do we do with finalRect.x and y?

	g_warning ("more here in FrameworkElement::Arrange.  move the bounds or something?  set properties?  who knows!?");
				     
}

Size
FrameworkElement::ArrangeOverride (Size finalSize)
{
	return finalSize;
}

void
FrameworkElement::RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb)
{
	this->measure_cb = measure_cb;
	this->arrange_cb = arrange_cb;
}
