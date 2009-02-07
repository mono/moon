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
	ManagedTypeInfo *key = GetDefaultStyleKey ();
	if (key && !GetStyle())
		Application::GetCurrent()->ApplyDefaultStyle (this, key);

	FrameworkElement::OnLoaded ();

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

	printf ("APPLYING TEMPLATE TO %s\n", GetTypeName());

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
