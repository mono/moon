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
	if (width == /*XXX NAN*/ 0 || width < GetMinWidth ())
		width = GetMinWidth ();

	// and do the same with height
	if (height == /*XXX NAN*/ 0 || height < GetMinHeight ())
		height = GetMinHeight ();

	Thickness *margins = GetMargin ();
	Thickness *border_thickness = GetBorderThickness ();
	Thickness *padding = GetPadding ();
	Brush *border_brush = GetBorderBrush ();

	double padding_width = padding ? padding->left + padding->right : 0;
	double padding_height = padding ? padding->top + padding->bottom: 0;
	double border_width = border_brush && border_thickness ? border_thickness->left + border_thickness->right : 0;
	double border_height = border_brush && border_thickness ? border_thickness->top + border_thickness->right : 0;

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
