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

	double padding_width = padding->left + padding->right;
	double padding_height = padding->top + padding->bottom;
	double border_width = border_thickness->left + border_thickness->right;
	double border_height = border_thickness->top + border_thickness->bottom;

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
	Size desired_size = GetDesiredSize ();

	if (desired_size.width <= finalSize.width
	    && desired_size.height <= finalSize.height) {

		/* nothing to do here, the final size is large enough
		   to keep our desired size */
	}

	else {
		SetDesiredSize (finalSize);
		desired_size = finalSize;
	}
		
	UIElement *child = GetChild ();
	if (child) {
		Thickness *margins = GetMargin ();
		Thickness *border = GetBorderThickness ();
		Thickness *padding = GetPadding ();

		Rect childRect;

		childRect.x = padding->left + border->left + margins->left;
		childRect.y = padding->top + border->top + margins->top;

		childRect.width = desired_size.width - padding->left - padding->right - border->left - border->right - margins->left - margins->right;
		childRect.height = desired_size.height - padding->top - padding->bottom - border->top - border->bottom - margins->top - margins->bottom;

		child->Arrange (childRect);
	}

	return desired_size;
}
