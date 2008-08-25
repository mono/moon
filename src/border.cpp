/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * border.cpp:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "border.h"
#include "thickness.h"

Border::Border()
{
}

Size
Border::MeasureOverride (Size availableSize)
{

	double width = GetWidth ();
	double height = GetHeight ();

	// if our width is not set, or is smaller than our configured MinWidth,
	// bump it up to the minimum.
	if (isnan (width) || width < GetMinWidth ())
		width = GetMinWidth ();

	// and do the same with height
	if (isnan (height) || height < GetMinHeight ())
		height = GetMinHeight ();

	Thickness *margins = GetMargin ();
	Thickness *border_thickness = GetBorderThickness ();
	Thickness *padding = GetPadding ();

	double padding_width = padding ? padding->left + padding->right : 0;
	double padding_height = padding ? padding->top + padding->bottom: 0;
	double border_width = border_thickness ? border_thickness->left + border_thickness->right : 0;
	double border_height = border_thickness ? border_thickness->top + border_thickness->bottom : 0;

	Size child_size = Size (0,0);

	// Get the desired size of our child, and include any margins we set
	UIElement *child = GetChild ();
	if (child) {
		double margin_width = margins->left + margins->right;
		double margin_height = margins->top + margins->bottom;

		child->Measure (availableSize);

		child_size = child->GetDesiredSize ();

		child_size.width += margin_width;
		child_size.height += margin_height;
	}

	// if the child's size + margins is > our idea
	// of what our size should be, use the
	// child+margins instead.
	if (child_size.width + padding_width + border_width > width)
		width = child_size.width + padding_width + border_width;
	if (child_size.height + padding_height + border_height > height)
		height = child_size.height + padding_height + border_height;

	// make sure we don't go over our configured max size
	if (width > GetMaxWidth ())
		width = GetMaxWidth ();
	if (height > GetMaxHeight ())
		height = GetMaxHeight ();

	// now choose whichever is smaller, our chosen size or the availableSize.
	return Size (availableSize.width > width ? width : availableSize.width,
		     availableSize.height > height ? height : availableSize.height);
}

Size
Border::ArrangeOverride (Size finalSize)
{
	// XXX what do we do with finalRect.x and y?

	Size desired_size = GetDesiredSize ();

	g_warning ("in Border::ArrangeOverride (%g, %g)", finalSize.width, finalSize.height);
	if (desired_size.width <= finalSize.width
	    && desired_size.height <= finalSize.height) {
		// the parent gave us a final rectangle large enough
		// for what we need.  keep our desired size, and pass
		// off the right arg to our child.

		g_warning ("border has desired size of (%g, %g)", desired_size.width, desired_size.height);
		UIElement *child = GetChild ();
		if (child) {
			Thickness *margins = GetMargin ();
			Thickness *border_thickness = GetBorderThickness ();
			Thickness *padding = GetPadding ();

			double padding_left = padding ? padding->left : 0;
			double padding_top = padding ? padding->top: 0;
			double padding_right = padding ? padding->right : 0;
			double padding_bottom = padding ? padding->bottom: 0;
			double border_left = border_thickness ? border_thickness->left : 0;
			double border_top = border_thickness ? border_thickness->top : 0;
			double border_right = border_thickness ? border_thickness->right : 0;
			double border_bottom = border_thickness ? border_thickness->bottom : 0;

			Rect childRect;

			childRect.x = padding_left + border_left + margins->left;
			childRect.y = padding_top + border_top + margins->top;

			childRect.w = desired_size.width - padding_left - padding_right - border_left - border_right - margins->left - margins->right;
			childRect.h = desired_size.height - padding_top - padding_bottom - border_top - border_bottom - margins->top - margins->bottom;

			child->Arrange (childRect);
		}

		return desired_size;
	}
	else {
		g_warning ("border has desired size of (%g, %g", desired_size.width, desired_size.height);
		g_warning ("unhandled case");
	}

	return finalSize;
}
