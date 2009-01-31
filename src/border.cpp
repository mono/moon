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
#include "geometry.h"
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
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;
		
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
	Thickness border = *GetPadding () + *GetBorderThickness ();

	Size specified = Size (GetWidth (), GetHeight ());

	finalSize = finalSize.Max (specified);
	finalSize = finalSize.Min (specified);

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;

		Size desired = child->GetDesiredSize ();
		Rect childRect (0,0,finalSize.width,finalSize.height);

		childRect = childRect.GrowBy (-border);

		if (GetHorizontalAlignment () != HorizontalAlignmentStretch && isnan (GetWidth ()))
			childRect.width = MIN (desired.width, childRect.width);

		if (GetVerticalAlignment () != VerticalAlignmentStretch && isnan (GetHeight ()))
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

	Geometry *clip = LayoutInformation::GetLayoutClip (this);
	if (clip) {
		clip->Draw (cr);
		cairo_clip (cr);
	}	

	if (background) {
		CornerRadius *round = GetCornerRadius ();
		background->SetupBrush (cr, extents);

		cairo_new_path (cr);
		
		if (round) {
			double top_adj = MAX (round->topLeft + round->topRight - extents.width, 0) / 2;
			double bottom_adj = MAX (round->bottomLeft + round->bottomRight - extents.width, 0) / 2;
			double left_adj = MAX (round->topLeft + round->bottomLeft - extents.height, 0) / 2;
			double right_adj = MAX (round->topRight + round->bottomRight - extents.height, 0) / 2;

			double tlt = round->topLeft - top_adj;
			cairo_move_to (cr, extents.x + tlt, extents.y);

			double trt = round->topRight - top_adj;
			double trr = round->topRight - right_adj;
			cairo_line_to (cr, extents.x + extents.width - trt, extents.y);
			cairo_curve_to (cr, 
					extents.x + extents.width - trt +  trt * ARC_TO_BEZIER, extents.y,
					extents.x + extents.width, extents.y + trr * ARC_TO_BEZIER,
					extents.x + extents.width, extents.y + trr);

			double brr = round->bottomRight - right_adj;
			double brb = round->bottomRight - bottom_adj;
			cairo_line_to (cr, extents.x + extents.width, extents.y + extents.height - brr);
			cairo_curve_to (cr,
					extents.x + extents.width, extents.x + extents.height - brr + brr * ARC_TO_BEZIER, 
					extents.x + extents.width + brb * ARC_TO_BEZIER - brb,  extents.y + extents.height,
					extents.x + extents.width - brb, extents.y + extents.height);

			double blb = round->bottomLeft - bottom_adj;
			double bll = round->bottomLeft - left_adj;
			cairo_line_to (cr, extents.x + blb, extents.y + extents.height);
			cairo_curve_to (cr,
					extents.x + blb - blb * ARC_TO_BEZIER, extents.y + extents.height,
					extents.x, extents.y + extents.height - bll * ARC_TO_BEZIER,
					extents.x, extents.y + extents.height - bll);

			double tll = round->topLeft - left_adj;
			cairo_line_to (cr, extents.x, extents.y + tll);
			cairo_curve_to (cr,
					extents.x, extents.y + tll - tll * ARC_TO_BEZIER,
					extents.x + tlt - tlt * ARC_TO_BEZIER, extents.y,
					extents.x + tlt, extents.y);
		}				

		else
			extents.Draw (cr);

		background->Fill (cr);
	}
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
	if (GetBackground ())
		return FrameworkElement::InsideObject (cr, x, y);

	return false;
}

void
Border::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

