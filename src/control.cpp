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
#include "namescope.h"

Control::Control ()
{
	applied_template = NULL;
	template_root = NULL;
	bindings = NULL;
	SetForeground (new SolidColorBrush ("black"));
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
		background->SetupBrush (cr, extents);

		cairo_new_path (cr);
		extents.Draw (cr);
		background->Fill (cr);
	}
}

void
Control::ComputeBounds ()
{
	extents = Rect (0, 0, GetActualWidth (), GetActualHeight ());
	bounds_with_children = bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
	
	if (template_root) {
		bounds_with_children = bounds_with_children.Union (template_root->GetSubtreeBounds ());
	}
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
	if (args->property->GetOwnerType() != Type::CONTROL) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == Control::TemplateProperty) {
		if (IsLoaded())
			ApplyTemplate ();
		InvalidateMeasure ();
	}
	else if (args->property == Control::PaddingProperty
		 || args->property == Control::BorderThicknessProperty) {
		InvalidateMeasure ();
	}
	NotifyListenersOfPropertyChange (args);
}

void
Control::OnLoaded ()
{
	FrameworkElement::OnLoaded ();

	// XXX we need some ordering work here
	ApplyTemplate ();
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

	OnApplyTemplate ();

	return true;
}

void
Control::OnApplyTemplate ()
{
	Emit (TemplateAppliedEvent);
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
Control::GetTemplateChild (const char *name)
{
	if (template_root)
		return template_root->FindName (name);
	
	return NULL;
}

Size
Control::MeasureOverride (Size availableSize)
{
	Size desired = Size (0,0);
	Size specified = Size (GetWidth (), GetHeight ());

	availableSize = availableSize.Max (specified);
	availableSize = availableSize.Min (specified);

	Thickness border = *GetPadding () + *GetBorderThickness ();

	// Get the desired size of our child, and include any margins we set
	if (UIElement *child = template_root) {
		Size childAvailable = availableSize.GrowBy (-border);
		g_warning ("control ca (%f, %f)", childAvailable.width, childAvailable.height);
		child->Measure (availableSize.GrowBy (-border));
		desired = child->GetDesiredSize ();
		childAvailable = desired;
		g_warning ("control cd (%f, %f)", childAvailable.width, childAvailable.height);

	}

	desired = desired.GrowBy (border);

	desired = desired.Max (specified);
	desired = desired.Min (specified);

	return desired;
}

Size
Control::ArrangeOverride (Size finalSize)
{
	Size desired = Size (0,0);
	Thickness border = *GetPadding () + *GetBorderThickness ();

	Size specified = Size (GetWidth (), GetHeight ());

	finalSize = finalSize.Max (specified);
	finalSize = finalSize.Min (specified);

	if (UIElement *child = template_root) {
		Rect childRect = Rect (0.0, 0.0, finalSize.width, finalSize.height).GrowBy (-border);
		child->Arrange (childRect.GrowBy (-border));
		desired = desired.Max (child->GetRenderSize ());
	}

	desired = desired.GrowBy (border);

	desired = desired.Max (specified);
	desired = desired.Min (specified);

	return desired;
}
