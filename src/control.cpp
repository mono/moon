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
	applied_template = NULL;
	template_root = NULL;
	bindings = NULL;
}

Control::~Control ()
{
	if (applied_template)
		applied_template->unref();

	if (bindings)
		delete bindings;
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
	
	if (template_root) {
		cairo_matrix_t offset;
		
		GetTransformFor (template_root, &offset);
		//extents = extents.Union (item->GetExtents ().Transform (&offset));
		extents = bounds_with_children.Union (template_root->GetSubtreeBounds ());
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
	if (template_root)
		return template_root->InsideObject (cr, x, y);
	
	return false;
}

void
Control::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	if (InsideObject (cr, p.x, p.y)) {
		uielement_list->Prepend (new UIElementNode (this));
		template_root->HitTest (cr, p, uielement_list);
	}
}

void
Control::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

void
Control::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == Control::TemplateProperty) {
		if (IsLoaded())
			ApplyTemplate ();
	}

	FrameworkElement::OnPropertyChanged (args);
}

void
Control::OnLoaded ()
{
	// XXX we need some ordering work here
	ApplyTemplate ();

	FrameworkElement::OnLoaded ();
}

bool
Control::ApplyTemplate ()
{
	if (applied_template == GetTemplate ())
		return false;

	if (applied_template) {
		applied_template->unref();
		applied_template = NULL;
		
		delete bindings;
		bindings = NULL;
	}
	
	ElementRemoved (template_root);

	if (!GetTemplate())
		return false;

	applied_template = GetTemplate ();
	applied_template->ref();

	bindings = new List ();

	FrameworkElement *el = applied_template->Apply(this, bindings);
	if (!el)
		return false;

	ElementAdded (el);

	return true;
}

void
Control::ElementAdded (UIElement *item)
{
	if (item == template_root)
		return;

	ElementRemoved (template_root);

	template_root = item;

	if (template_root) {
		template_root->ref ();
		FrameworkElement::ElementAdded (template_root);
	}
}

void
Control::ElementRemoved (UIElement *item)
{
	if (template_root && item == template_root) {
		template_root->unref ();
		template_root = NULL;
	}

	if (item)
		FrameworkElement::ElementRemoved (item);
}

DependencyObject *
Control::GetTemplateChild (char *name)
{
	if (template_root)
		return template_root->FindName (name);
	
	return NULL;
}

Size
Control::MeasureOverride (Size availableSize)
{
	Size contents = Size (0.0, 0.0);
	Thickness border = *GetBorderThickness () + *GetPadding ();

	if (template_root) {
		template_root->Measure (availableSize.GrowBy (-border));
		contents = template_root->GetDesiredSize ();
	}

	return contents.GrowBy (border);
}
