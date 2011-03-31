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

#include <math.h>

#include "geometry.h"
#include "runtime.h"
#include "brush.h"
#include "border.h"
#include "thickness.h"

namespace Moonlight {

Border::Border()
{
	SetObjectType (Type::BORDER);
}

Size
Border::MeasureOverrideWithError (Size availableSize, MoonError *error)
{
	Size desired = Size (0,0);

	Thickness border = *GetPadding () + *GetBorderThickness ();

	// Get the desired size of our child, and include any margins we set
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		child->MeasureWithError (availableSize.GrowBy (-border), error);
		desired = child->GetDesiredSize ();
	}

	desired = desired.GrowBy (border);

	desired = desired.Min (availableSize);

	return desired;
}

Size
Border::ArrangeOverrideWithError (Size finalSize, MoonError *error)
{
	Thickness border = *GetPadding () + *GetBorderThickness ();
	Size arranged = finalSize;
	
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		Rect childRect (0,0,finalSize.width,finalSize.height);

		childRect = childRect.GrowBy (-border);


		child->ArrangeWithError (childRect, error);

		arranged = Size (childRect.width, childRect.height).GrowBy (border);

		arranged = arranged.Max (finalSize);
	}

	return finalSize;
}

void 
Border::Render (Context *ctx, Region *region)
{
	cairo_t *cr = ctx->Push (Context::Cairo ());
	Render (cr, region);
	ctx->Pop ();
}

void 
Border::Render (cairo_t *cr, Region *region, bool path_only)
{
	Brush *background = GetBackground ();
	Brush *border_brush = GetBorderBrush ();

	cairo_save (cr);
	if (!path_only)
		RenderLayoutClip (cr);

	CornerRadius r = *GetCornerRadius ();
	CornerRadius *round =  &r;
	Thickness thickness = *GetBorderThickness ();
	Rect paint_border = extents;
	Rect paint_background = paint_border.GrowBy (-thickness);

	CornerRadius inner_adjusted = *round;
	inner_adjusted.topLeft = MAX (round->topLeft - MAX (thickness.left, thickness.top) * .5, 0);
	inner_adjusted.topRight = MAX (round->topRight - MAX (thickness.right, thickness.top) * .5, 0);
	inner_adjusted.bottomRight = MAX (round->bottomRight - MAX (thickness.right, thickness.bottom) * .5, 0);
	inner_adjusted.bottomLeft = MAX (round->bottomLeft - MAX (thickness.left, thickness.bottom) * .5, 0);

	CornerRadius outer_adjusted = *round;
	outer_adjusted.topLeft = outer_adjusted.topLeft ? MAX (round->topLeft + MAX (thickness.left, thickness.top) * .5, 0) : 0;
	outer_adjusted.topRight = outer_adjusted.topRight ? MAX (round->topRight + MAX (thickness.right, thickness.top) * .5, 0) : 0;
	outer_adjusted.bottomRight = outer_adjusted.bottomRight ? MAX (round->bottomRight + MAX (thickness.right, thickness.bottom) * .5, 0) : 0;
	outer_adjusted.bottomLeft = outer_adjusted.bottomLeft ? MAX (round->bottomLeft + MAX (thickness.left, thickness.bottom) * .5, 0) : 0;

	/* 
	 * NOTE filling this way can leave alpha artifacts between the border fill and bg fill
	 * but some simple inspection of the ms results make me think that is what happens there
	 * too.
	 */
	cairo_new_path (cr);
	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

	if (border_brush && !paint_border.IsEmpty ()) {
		border_brush->SetupBrush (cr, paint_border);

		paint_border.Draw (cr, &outer_adjusted);
		paint_background.Draw (cr, &inner_adjusted);

		if (!path_only)
			border_brush->Fill (cr);
	}

	if (background && !paint_background.IsEmpty ()) {
		background->SetupBrush (cr, paint_background);

		paint_background.Draw (cr, &inner_adjusted);

		if (!path_only)
			background->Fill (cr);
	}
	
	cairo_restore (cr);
}

void
Border::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::BORDER) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Border::ChildProperty){
		if (args->GetOldValue() && args->GetOldValue()->AsUIElement()) {
			ElementRemoved (args->GetOldValue()->AsUIElement ());
			SetSubtreeObject (NULL);
			if (args->GetOldValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
				args->GetOldValue()->AsFrameworkElement()->SetLogicalParent (NULL, error);
				if (error->number)
					return;
			}
				
		}
		if (args->GetNewValue() && args->GetNewValue()->AsUIElement()) {
			SetSubtreeObject (args->GetNewValue()->AsUIElement());
			ElementAdded (args->GetNewValue()->AsUIElement ());
			if (args->GetNewValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
				FrameworkElement *fwe = args->GetNewValue()->AsFrameworkElement ();
				if (fwe->GetLogicalParent() && fwe->GetLogicalParent() != this) {
					MoonError::FillIn (error, MoonError::ARGUMENT, "Content is already a child of another element");
					return;
				}

				args->GetNewValue()->AsFrameworkElement()->SetLogicalParent (this, error);
				if (error->number)
					return;
			}
		}

		UpdateBounds ();
		InvalidateMeasure ();
	}
	else if (args->GetId () == Border::PaddingProperty
		 || args->GetId () == Border::BorderThicknessProperty) {
		InvalidateMeasure ();
	} else if (args->GetId () == Border::BackgroundProperty) {
		Invalidate ();
	} else if (args->GetId () == Border::BorderBrushProperty) {
		Invalidate ();
	}
	NotifyListenersOfPropertyChange (args, error);
}

void
Border::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && (prop->GetId () == Border::BackgroundProperty || prop->GetId () == Border::BorderBrushProperty)) {
		Invalidate ();
	}
	else
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

bool
Border::InsideObject (cairo_t *cr, double x, double y)
{
	if (!FrameworkElement::InsideObject (cr, x, y))
		return false;

	cairo_save (cr);
	cairo_new_path (cr);
	cairo_set_matrix (cr, &absolute_xform);

	TransformPoint (&x, &y);

	Render (cr, NULL, true);
	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	bool inside = cairo_in_fill (cr, x, y);
	cairo_restore (cr);

	return inside;
}

};
