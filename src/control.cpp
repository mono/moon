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
	enabled_local = true;
	enabled_parent = true;
	template_root = NULL;
}

Control::~Control ()
{
}

void
Control::Dispose ()
{
	if (applied_template) {
		applied_template->unref();
		applied_template = NULL;
	}

	if (template_root) {
		template_root->unref ();
		template_root = NULL;
	}

	FrameworkElement::Dispose ();
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
Control::OnLoaded ()
{
	UpdateEnabled ();
	FrameworkElement::OnLoaded ();
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
	} else if (args->GetId () == Control::IsEnabledProperty) {
		if (!args->GetNewValue ()->AsBool ())
			ReleaseMouseCapture ();
	}
	NotifyListenersOfPropertyChange (args, error);
}

void
Control::SetVisualParent (UIElement *visual_parent)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	FrameworkElement::SetVisualParent (visual_parent);
	
	if (!visual_parent) {
		enabled_parent = true;
	} else {
		UIElement *parent = GetVisualParent ();
		while (parent) {
			if (!types->IsSubclassOf (parent->GetObjectType (), Type::CONTROL)) {
				parent = parent->GetVisualParent ();
			}
			else {
				this->enabled_parent = ((Control *)parent)->GetIsEnabled ();
				break;
			}
		}
	}
	SetValue (Control::IsEnabledProperty, Value (enabled_local));
}

bool
Control::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
	
	if (property->GetId () == Control::IsEnabledProperty) {
		this->enabled_local = value->AsBool ();

		Value v (enabled_local && (enabled_parent));
		
		// If we don't propagate the changes down the tree here, the EnabledChanged events
		// from the subtree are raised in the wrong order
		bool b = FrameworkElement::SetValueWithErrorImpl (property, &v, error);
		if (b)
			UpdateEnabled ();
		return  b;
	}
	return FrameworkElement::SetValueWithErrorImpl (property, value, error);
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

	NameScope::GetNameScope (el)->Lock ();
	ElementAdded (el);

	MoonError e;
	template_root->SetParent (this, &e);
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
Control::Focus (bool recurse)
{
	Surface *surface = GetSurface ();
	if (!surface)
		return false;
		
	 /* according to msdn, these three things must be true for an element to be focusable:
	 *
	 * 1. the element must be visible
	 * 2. the element must have IsTabStop = true
	 * 3. the element must be part of the plugin's visual tree, and must have had its Loaded event fired.
	 */
	 
	 /*
	 * If the current control is not focusable, we walk the visual tree and stop as soon
	 * as we find the first focusable child. That then becomes focused
	 */
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	DeepTreeWalker walker (this);
	while (UIElement *e = walker.Step ()) {
		if (!types->IsSubclassOf (e->GetObjectType (), Type::CONTROL))
			continue;
		
		Control *c = (Control *)e;
		if (!c->GetIsEnabled ()) {
			if (!recurse)
				return false;

			walker.SkipBranch ();
			continue;
		}
		
		if (c->IsLoaded () && c->GetRenderVisible () && c->GetIsTabStop ())
			return surface->FocusElement (c);
		
		if (!recurse)
			return false;
	}
	return false;	
}

void
Control::UpdateEnabled ()
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	DeepTreeWalker walker = DeepTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child == this || !types->IsSubclassOf (child->GetObjectType (), Type::CONTROL))
			continue;

		Control *control = (Control *)child;
		control->enabled_parent = (enabled_local && enabled_parent);
		control->SetValue (Control::IsEnabledProperty, Value (control->enabled_local));
		walker.SkipBranch ();
	}
}
