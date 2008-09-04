/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * control.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "rect.h"
#include "runtime.h"
#include "control.h"
#include "canvas.h"

Control::Control ()
{
}

Control::~Control ()
{
}

void 
Control::Render (cairo_t *cr, Region *region)
{
	Brush *background = GetBackground ();

	cairo_set_matrix (cr, &absolute_xform);
	
	if (background) {
		background->SetupBrush (cr, this);

		cairo_new_path (cr);
		extents.Draw (cr);
		cairo_fill (cr);
	}
}

void
Control::ComputeBounds ()
{
	double width = GetWidth ();
	double height = GetHeight ();
	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
	
	Thickness border = *GetBorderThickness ();
	Thickness padding = *GetPadding ();

	extents = Rect ();
	bounds_with_children = Rect ();
	ContentWalker walker = ContentWalker (this);
	while (UIElement *item = (UIElement *)walker.Step ()) {
		cairo_matrix_t offset;
		
		GetTransformFor (item, &offset);
		//extents = extents.Union (item->GetExtents ().Transform (&offset));
		extents = bounds_with_children.Union (item->GetSubtreeBounds ());
	}
	extents.height += border.bottom + padding.bottom;
	extents.width += border.right + padding.right;

	// if width or height == NAN  Auto layout
	if (vh && vw && (width == width) && (height == height))
		extents = Rect (0.0, 0.0, width, height);
	else 
		g_warning ("AutoWidth (%f, %f, %f, %f)", extents.x, extents.y, extents.width, extents.height);

	bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
	bounds_with_children = bounds.Union (bounds);
}

void
Control::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	cairo_matrix_init_identity (result);
	
	Thickness border = *GetBorderThickness ();
	Thickness padding = *GetPadding ();
	
	cairo_matrix_translate (result, padding.left, padding.top);
	cairo_matrix_translate (result, border.left, border.top);
}

bool 
Control::InsideObject (cairo_t *cr, double x, double y)
{
	ContentWalker walker = ContentWalker (this);
	UIElement *content = (UIElement *)walker.Step ();
	
	if (content)
		return content->InsideObject (cr, x, y);
	
	return false;
}

void
Control::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	ContentWalker walker = ContentWalker (this);
	UIElement *content = (UIElement *)walker.Step ();

	if (InsideObject (cr, p.x, p.y)) {
		uielement_list->Prepend (new UIElementNode (this));
		content->HitTest (cr, p, uielement_list);
	}
}

void
Control::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

