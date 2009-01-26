/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * border.cpp:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "runtime.h"
#include "brush.h"
#include "border.h"
#include "thickness.h"

Border::Border()
{
	SetObjectType (Type::BORDER);
}

Size
Border::MeasureOverride (Size availableSize)
{
	Size desired = Size (0,0);
	Size specified = Size (GetWidth (), GetHeight ());

	availableSize = availableSize.Max (specified);
	availableSize = availableSize.Min (specified);

	Thickness border = *GetPadding () + *GetBorderThickness ();

	// Get the desired size of our child, and include any margins we set
	if (UIElement *child = GetChild ()) {
		child->Measure (availableSize.GrowBy (-border));
		desired = child->GetDesiredSize ();
	}

	desired = desired.GrowBy (border);

	if (GetHorizontalAlignment () == HorizontalAlignmentStretch && !isinf (availableSize.width))
		desired.width = availableSize.width;

	if (GetVerticalAlignment () == VerticalAlignmentStretch && !isinf (availableSize.height))
		desired.height = availableSize.height;

	desired = desired.Max (specified);
	desired = desired.Min (specified);

	return desired;
}

Size
Border::ArrangeOverride (Size finalSize)
{
	Thickness border = *GetPadding () + *GetBorderThickness ();

	Size specified = Size (GetWidth (), GetHeight ());

	finalSize = finalSize.Max (specified);
	finalSize = finalSize.Min (specified);

	if (UIElement *child = GetChild ()) {
		Size desired = child->GetDesiredSize ();
		Rect childRect (0,0,finalSize.width,finalSize.height);

		childRect = childRect.GrowBy (-border);

		if (GetHorizontalAlignment () != HorizontalAlignmentStretch)
			childRect.width = MIN (desired.width, childRect.width);

		if (GetVerticalAlignment () != VerticalAlignmentStretch)
			childRect.height = MIN (desired.height, childRect.height);
		
		child->Arrange (childRect);
		finalSize = finalSize.Max (child->GetRenderSize ());
	}

	return finalSize;
}

void 
Border::Render (cairo_t *cr, Region *region)
{
	Brush *background = GetBackground ();

	cairo_set_matrix (cr, &absolute_xform);
	
	if (background) {
		background->SetupBrush (cr, extents);

		cairo_new_path (cr);
		extents.Draw (cr);
		background->Fill (cr);
	}
}

void
Border::ComputeBounds ()
{
	Size specified = Size (GetActualWidth (), GetActualHeight ());
	specified = specified.Max (GetWidth (), GetHeight ());
	extents = Rect (0, 0, specified.width, specified.height);
	bounds_with_children = bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);

	UIElement *child = GetChild ();

	if (child)
		bounds_with_children = bounds_with_children.Union (child->GetSubtreeBounds ());
}

void
Border::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::BORDER) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == Border::ChildProperty){
		if (args->old_value) {
			ElementRemoved (args->old_value->AsUIElement ());
		}
		if (args->new_value) {
			ElementAdded (args->new_value->AsUIElement ());
		}

		SetSubtreeObject (args->new_value ? args->new_value->AsUIElement() : NULL);

		UpdateBounds ();
		InvalidateMeasure ();
	}
	else if (args->property == Border::PaddingProperty
		 || args->property == Border::BorderThicknessProperty) {
		InvalidateMeasure ();
	}
	NotifyListenersOfPropertyChange (args);
}

bool 
Border::InsideObject (cairo_t *cr, double x, double y)
{
	UIElement *child = GetChild ();

	if (child)
		return child->InsideObject (cr, x, y);
	else
		return false;
}

void
Border::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	UIElement *child = GetChild ();

	if (InsideObject (cr, p.x, p.y)) {
		uielement_list->Prepend (new UIElementNode (this));
		child->HitTest (cr, p, uielement_list);
	}
}

void
Border::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

