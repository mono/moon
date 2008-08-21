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
	if (real_object)
		real_object->DoRender (cr, region);
}

void
Control::FrontToBack (Region *surface_region, List *render_list)
{
	if (real_object)
		return real_object->FrontToBack (surface_region, render_list);
}

void 
Control::ComputeBounds ()
{
	if (real_object) {
		bounds = real_object->GetBounds ();
		bounds_with_children = real_object->GetSubtreeBounds ();
	} else {
		bounds = Rect (0, 0, 0, 0);
		bounds_with_children = Rect (0, 0, 0, 0);
	}
	
	double x1, x2, y1, y2;
	
	x1 = y1 = 0.0;
	x2 = GetWidth ();
	y2 = GetHeight ();
	
	if (x2 != 0.0 && y2 != 0.0) {
		Rect fw_rect = Rect (x1,y1,x2,y2);
		
		if (real_object)
			bounds = bounds.Union (fw_rect);
		else
			bounds = fw_rect;
	}
	
	bounds = IntersectBoundsWithClipPath (bounds, false).Transform (&absolute_xform);
	bounds_with_children = IntersectBoundsWithClipPath (bounds_with_children.Union (bounds), false);
}

void
Control::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == Canvas::TopProperty || subobj_args->property == Canvas::LeftProperty)
		real_object->UpdateTransform ();
	
	UIElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Control::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	double left = Canvas::GetLeft (item);
	double top = Canvas::GetTop (item);
	
	cairo_matrix_init_translate (result, left, top);
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

void
Control::SetContent (UIElement *element, Surface *surface)
{
	if (real_object){
		real_object->SetVisualParent (NULL);
		real_object->SetSurface (NULL);
		real_object->unref ();
	}

	real_object = (FrameworkElement *) element;
	real_object->SetVisualParent (this);

	//
	// It is not clear that we even need to do this, as attaching
	// will do this internally, but am keeping this to not break
	// things.   Also the @surface parameter could go away if we
	// determine its not needed
	//
	if (surface != NULL)
		SetSurface (surface);
	
	real_object->AddPropertyChangeListener (this);
	real_object->UpdateTotalRenderVisibility ();
	real_object->UpdateTransform ();
	UpdateBounds ();

}

UIElement *
Control::InitializeFromXaml (const char *xaml, Type::Kind *element_type, XamlLoader *loader)
{
	// No callback, figure out how this will work in the plugin to satisfy deps
	UIElement *element = (UIElement*)xaml_create_from_str (loader, xaml, false, element_type);
	if (element == NULL)
		return NULL;

	Surface *surface = loader ? loader->GetSurface () : NULL;

	SetContent (element, surface);
	
	return element;
}

void
Control::SetBorderThickness (Thickness *thickness)
{
	SetValue (Control::BorderThicknessProperty, Value (*thickness));
}

Thickness *
Control::GetBorderThickness ()
{
	return GetValue (Control::BorderThicknessProperty)->AsThickness ();
}

void
Control::SetPadding (Thickness *padding)
{
	SetValue (Control::PaddingProperty, Value (*padding));
}

Thickness *
Control::GetPadding ()
{
	return GetValue (Control::PaddingProperty)->AsThickness ();
}
