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
		finalSize = finalSize.Max (child->GetRenderSize ().GrowBy (border));
	}

	return finalSize;
}

void 
Border::Render (cairo_t *cr, Region *region)
{
	Brush *background = GetBackground ();
	Brush *border_brush = GetBorderBrush ();

	cairo_set_matrix (cr, &absolute_xform);

	Geometry *clip = LayoutInformation::GetLayoutClip (this);
	if (clip) {
		cairo_save (cr);
		clip->Draw (cr);
		cairo_clip (cr);
	}	

	Rect paint = extents;
	CornerRadius *round = GetCornerRadius ();
	if (border_brush) {
		border_brush->SetupBrush (cr, paint);

		cairo_new_path (cr);
		paint.Draw (cr, round);
		
		border_brush->Fill (cr);
	}

	if (background) {
		Thickness bthick = *GetBorderThickness ();
		CornerRadius adjusted;
		
		paint = paint.GrowBy (-bthick);
		
		if (round) {
			adjusted = *round;
			adjusted.topLeft = MAX (round->topLeft - MAX (bthick.left, bthick.top), 0);
			adjusted.topRight = MAX (round->topRight - MAX (bthick.right, bthick.top), 0);
			adjusted.bottomRight = MAX (round->bottomRight - MAX (bthick.right, bthick.bottom), 0);
			adjusted.bottomLeft = MAX (round->bottomLeft - MAX (bthick.left, bthick.bottom), 0);
		}

		background->SetupBrush (cr, paint);

		cairo_new_path (cr);
		/* XXX FIXME this is not quite the right rounding */
		paint.Draw (cr, round ? &adjusted : NULL);

		background->Fill (cr);
	}
	
	if (clip)
		cairo_restore (cr);
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

