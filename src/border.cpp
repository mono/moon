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

	desired = desired.Max (specified);
	desired = desired.Min (specified);

	return desired;
}

Size
Border::ArrangeOverride (Size finalSize)
{
	Size desired = Size (0,0);
	Thickness border = *GetPadding () + *GetBorderThickness ();

	Size specified = Size (GetWidth (), GetHeight ());

	finalSize = finalSize.Max (specified);
	finalSize = finalSize.Min (specified);

	if (UIElement *child = GetChild ()) {
		Rect childRect = Rect (0.0, 0.0, finalSize.width, finalSize.height);

		child->Arrange (childRect.GrowBy (-border));
		desired = desired.Max (child->GetRenderSize ());
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
	Rect *r = LayoutInformation::GetLayoutSlot (this);
	Rect slot = r ? *r : Rect ();
	
	Size specified = Size (GetWidth (), GetHeight ());
	HorizontalAlignment horiz = GetHorizontalAlignment ();
	VerticalAlignment vert = GetVerticalAlignment ();
	cairo_matrix_t layout_xform;
	cairo_matrix_init_identity (&layout_xform);

	if (!isnan (specified.width))
		horiz = HorizontalAlignmentCenter;
	if (!isnan (specified.height))
		vert = VerticalAlignmentCenter;
	
	switch (horiz) {
	case HorizontalAlignmentCenter:
		cairo_matrix_translate (&layout_xform, (slot.width  - GetActualWidth ()) * .5, 0);
		break;
	case HorizontalAlignmentRight:
		cairo_matrix_translate (&layout_xform, slot.width - GetActualWidth (), 0);
		break;
	default:
		break;
	}

	switch (vert) {
	case VerticalAlignmentCenter:
		cairo_matrix_translate (&layout_xform, 0, (slot.height  - GetActualHeight ()) * .5);
		break;
	case VerticalAlignmentBottom:
		cairo_matrix_translate (&layout_xform, 0, slot.height - GetActualHeight ());
		break;
	default:
		break;
	}
	
	extents = Rect (0, 0, GetActualWidth (), GetActualHeight ());
	extents = extents.Transform (&layout_xform);

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

