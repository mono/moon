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
	Size size;

	if (measure_cb)
		size = (*measure_cb)(availableSize);
	else
		size = MeasureOverride (availableSize);

	SetDesiredSize (size);
}

Size
FrameworkElement::MeasureOverride (Size availableSize)
{
#if SL_2_0
	double width = GetWidth ();
	double height = GetHeight ();

	// if our width is not set, or is smaller than our configured MinWidth,
	// bump it up to the minimum.
	if (width == /*XXX NAN*/ 0 || width < GetMinWidth ())
		width = GetMinWidth ();

	// and do the same with height
	if (height == /*XXX NAN*/ 0 || height < GetMinHeight ())
		height = GetMinHeight ();

	DependencyObject *content = GetContent ();
	if (content) {
		if (content->Is (Type::UIELEMENT)) {
			// Get the desired size of our content, and include any margins we set
			UIElement *el = (UIElement*)content;
			Thickness *margins = GetMargin ();

			el->Measure (availableSize);

			Size child_size = el->GetDesiredSize ();

			// if the child's size + margins is > our idea
			// of what our size should be, use the
			// child+margins instead.
			if (child_size.width + margins->left + margins->right > width)
				width = child_size.width + margins->left + margins->right;
			if (child_size.height + margins->top + margins->bottom > height)
				height = child_size.height + margins->top + margins->bottom;
		}
		else if (content->Is (Type::COLLECTION)) {
			g_warning ("non-panel has a collection for its ContentProperty.  unsupported");
		}
		else {
			g_warning ("unsupport content of FrameworkElement (%s)", content->GetTypeName());
		}
	}

	// make sure we don't go over our configured max size
	if (width > GetMaxWidth ())
		width = GetMaxWidth ();
	if (height > GetMaxHeight ())
		height = GetMaxHeight ();

	// now choose whichever is smaller, our chosen size or the availableSize.
	return Size (availableSize.width > width ? width : availableSize.width,
		     availableSize.height > height ? height : availableSize.height);
#else
	return availableSize;
#endif
}


// not sure about the disconnect between these two methods..  I would
// imagine both should take Rects and ArrangeOverride would return a
// rectangle as well..
void
FrameworkElement::Arrange (Rect finalRect)
{
	Size size;
	Size finalSize (finalRect.w, finalRect.h);

	if (arrange_cb)
		size = (*arrange_cb)(finalSize);
	else
		size = ArrangeOverride (finalSize);

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
