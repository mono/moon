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
#include "application.h"
#include "geometry.h"

Control::Control ()
{
	SetObjectType (Type::CONTROL);

	applied_template = NULL;
	template_root = NULL;
}

Control::~Control ()
{
	if (applied_template)
		applied_template->unref();
}

void
Control::FindElementsInHostCoordinates (cairo_t *cr, Point p, List *uielement_list)
{
	if (GetIsEnabled ())
		FrameworkElement::FindElementsInHostCoordinates (cr, p, uielement_list);
}

void
Control::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	if (GetIsEnabled ())
		FrameworkElement::HitTest (cr, p, uielement_list);
}

bool
Control::InsideObject (cairo_t *cr, double x, double y)
{
	/* 
	 * Controls don't get hit themselves the rendered elements 
	 * do and it bubbles up
	 */
	return false;
}

void
Control::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::CONTROL) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Control::TemplateProperty) {
		if (IsLoaded())
			ApplyTemplate ();
		InvalidateMeasure ();
	}
	else if (args->GetId () == Control::PaddingProperty
		 || args->GetId () == Control::BorderThicknessProperty) {
		InvalidateMeasure ();
	}
	NotifyListenersOfPropertyChange (args, error);
}

bool
Control::ApplyTemplate ()
{
	if (applied_template == GetTemplate ())
		return false;

	ClearTemplate ();

	if (!GetTemplate())
		return false;

	//printf ("APPLYING TEMPLATE TO %s\n", GetTypeName());

	applied_template = GetTemplate ();
	applied_template->ref();

	FrameworkElement *el = applied_template->Apply(this);
	if (!el)
		return false;

	ElementAdded (el);

	OnApplyTemplate ();

	return true;
}

void
Control::ClearTemplate ()
{
	if (applied_template) {
		applied_template->unref();
		applied_template = NULL;
	}
	
	ElementRemoved (template_root);
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

	SetSubtreeObject (template_root);
}

void
Control::ElementRemoved (UIElement *item)
{
	if (template_root && item == template_root) {
		template_root->unref ();
		template_root = NULL;
		SetSubtreeObject (NULL);
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

bool
Control::Focus ()
{
	Surface *surface;
	
	/* according to msdn, these three things must be true for an element to be focusable:
	 *
	 * 1. the element must be visible
	 * 2. the element must have IsTabStop = true
	 * 3. the element must be part of the plugin's visual tree, and must have had its Loaded event fired.
	 */
	
	if (!IsLoaded () || !GetRenderVisible () || !GetIsTabStop ())
		return false;
	
	if (!(surface = GetSurface ()))
		return false;
	
	return surface->FocusElement (this);
}
