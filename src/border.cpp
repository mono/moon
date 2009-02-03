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

	Size arranged = finalSize;
	
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
		arranged = child->GetRenderSize ();

		if (GetHorizontalAlignment () == HorizontalAlignmentStretch || !isnan (GetWidth ()))
			arranged.width = MAX (arranged.width, finalSize.width);
		    
		if (GetVerticalAlignment () == VerticalAlignmentStretch || !isnan (GetHeight()))
			arranged.height = MAX (arranged.height, finalSize.height);
	}

	return arranged;
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

	CornerRadius *round = GetCornerRadius ();
	CornerRadius adjusted = CornerRadius (0);
	Thickness thickness = *GetBorderThickness ();
	Rect paint_border = extents;
	Rect paint_background = paint_border.GrowBy (-thickness);

	if (round) {
		adjusted = *round;
		adjusted.topLeft = MAX (round->topLeft - MAX (thickness.left, thickness.top), 0);
		adjusted.topRight = MAX (round->topRight - MAX (thickness.right, thickness.top), 0);
		adjusted.bottomRight = MAX (round->bottomRight - MAX (thickness.right, thickness.bottom), 0);
		adjusted.bottomLeft = MAX (round->bottomLeft - MAX (thickness.left, thickness.bottom), 0);
	}

	/* 
	 * NOTE filling this way can leave alpha artifacts between the border fill and bg fill
	 * but some simple inspection of the ms results make me think that is what happens there
	 * too.
	 */
	if (border_brush && paint_border != paint_background) {
		border_brush->SetupBrush (cr, paint_border);

		cairo_new_path (cr);

		cairo_fill_rule_t old = cairo_get_fill_rule (cr);
		cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

		paint_border.Draw (cr, round);
		paint_background.Draw (cr, round ? &adjusted : NULL);
		border_brush->Fill (cr);

		cairo_set_fill_rule (cr, old);
	}

	if (background) {
		background->SetupBrush (cr, paint_background);

		cairo_new_path (cr);
		paint_background.Draw (cr, round ? &adjusted : NULL);

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

