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
	real_object = NULL;
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
Control::FrontToBack (Region *surface_region, List *render_list)
{
	double local_opacity = GetOpacity ();

	if (surface_region->RectIn (bounds_with_children.RoundOut()) == GDK_OVERLAP_RECTANGLE_OUT)
		return;

	if (!GetRenderVisible ()
	    || IS_INVISIBLE (local_opacity))
		return;

	if (!UseBackToFront ()) {
		Region *self_region = new Region (surface_region);
		self_region->Intersect (bounds_with_children.RoundOut());

		// we need to include our children in this one, since
		// we'll be rendering them in the PostRender method.
		if (!self_region->IsEmpty())
			render_list->Prepend (new RenderNode (this, self_region, true,
							      UIElement::CallPreRender, UIElement::CallPostRender));
		// don't remove the region from surface_region because
		// there are likely holes in it
		return;
	}

	Region *region;
	bool delete_region;
	bool can_subtract_self;
	
	if (!GetClip ()
	    && !GetOpacityMask ()
	    && !IS_TRANSLUCENT (GetOpacity ())) {
		region = surface_region;
		delete_region = false;
		can_subtract_self = true;
	}
	else {
		region = new Region (surface_region);
		delete_region = true;
		can_subtract_self = false;
	}

	RenderNode *panel_cleanup_node = new RenderNode (this, NULL, false, NULL, UIElement::CallPostRender);
	
	render_list->Prepend (panel_cleanup_node);

	Region *self_region = new Region (region);

	ContentWalker walker = ContentWalker (this, ZReverse);
	while (DependencyObject *content = walker.Step ()) {
		if (content->Is (Type::UIELEMENT))
			((UIElement *)content)->FrontToBack (region, render_list);
	}

	if (!GetOpacityMask () && !IS_TRANSLUCENT (local_opacity)) {
		delete self_region;
		if (!GetBackground ()) {
			self_region = new Region ();
		}
		else {
			self_region = new Region (region);
			self_region->Intersect (GetRenderBounds().RoundOut ()); // note the RoundOut
		}
	} else {
		self_region->Intersect (GetSubtreeBounds().RoundOut ()); // note the RoundOut
	}

	if (self_region->IsEmpty() && render_list->First() == panel_cleanup_node) {
		/* we don't intersect the surface region, and none of
		   our children did either, remove the cleanup node */
		render_list->Remove (render_list->First());
		delete self_region;
		if (delete_region)
			delete region;
		return;
	}

	render_list->Prepend (new RenderNode (this, self_region, !self_region->IsEmpty(), UIElement::CallPreRender, NULL));

	if (!self_region->IsEmpty()) {
		bool subtract = ((absolute_xform.yx == 0 && absolute_xform.xy == 0) /* no skew/rotation */
				 && can_subtract_self);

		if (subtract) {
			Brush *background = GetBackground ();
			
			if (background)
				subtract = background->IsOpaque ();
			else
				subtract = false;
		}

 		if (subtract)
			region->Subtract (bounds);
	}

	if (delete_region)
		delete region;
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
	if (real_object)
		return real_object->InsideObject (cr, x, y);
	else
		return false;
}

void
Control::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	if (InsideObject (cr, p.x, p.y)) {
		uielement_list->Prepend (new UIElementNode (this));
		real_object->HitTest (cr, p, uielement_list);
	}
}

void
Control::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

