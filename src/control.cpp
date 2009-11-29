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
#include "tabnavigationwalker.h"
#include "deployment.h"

Control::Control ()
{
	SetObjectType (Type::CONTROL);

	enabled_local = true;
	enabled_parent = true;
	template_root = NULL;
}

Control::~Control ()
{
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
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	UIElement *e = GetVisualParent ();
	while (e && !types->IsSubclassOf (e->GetObjectType (), Type::CONTROL))
		e = e->GetVisualParent ();
	if (e)
		((Control *)e)->UpdateEnabled ();

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
		if (GetSubtreeObject ())
			ElementRemoved ((UIElement *) GetSubtreeObject ());
	}
	else if (args->GetId () == Control::PaddingProperty
		 || args->GetId () == Control::BorderThicknessProperty) {
		InvalidateMeasure ();
	} else if (args->GetId () == Control::IsEnabledProperty) {
		if (!args->GetNewValue ()->AsBool ()) {
			Surface *surface = Deployment::GetCurrent ()->GetSurface ();
			if (surface && surface->GetFocusedElement () == this) {
				// Ensure this element loses focus, then try to focus the next suitable element
				surface->FocusElement (NULL);
				TabNavigationWalker::Focus (this, true);
			}
			ReleaseMouseCapture ();
		}
		args->ref (); // to counter the unref in Emit
		Emit (IsEnabledChangedEvent, args);
	} else if (args->GetId () == Control::HorizontalContentAlignmentProperty
		   || args->GetId () == Control::VerticalContentAlignmentProperty) {
		InvalidateArrange ();
	}
	NotifyListenersOfPropertyChange (args, error);
}

void
Control::SetVisualParent (UIElement *visual_parent)
{
	FrameworkElement::SetVisualParent (visual_parent);
	if (!UIElement::IsSubtreeLoaded (this))
		return;

	this->enabled_parent = GetParentEnabledState (this);
	SetValue (Control::IsEnabledProperty, Value (enabled_local));
}

bool
Control::GetParentEnabledState (UIElement *element)
{
	do {
		element = element->GetVisualParent ();
	} while (element && !element->Is (Type::CONTROL));
	
	return element ? ((Control *) element)->GetIsEnabled () : true;
}

bool
Control::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
	if (property->GetId () == Control::IsEnabledProperty) {
		this->enabled_local = value->AsBool ();
		if ((enabled_local && enabled_parent) == GetIsEnabled ())
			return true;

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
Control::DoApplyTemplate ()
{
	ControlTemplate *t = GetTemplate ();
	if (!t)
		return FrameworkElement::DoApplyTemplate ();

	// If the template expands to an element which is *not* a UIElement
	// we don't apply the template.
	DependencyObject *root = t->GetVisualTree (this);
	if (root && !root->Is (Type::UIELEMENT)) {
		g_warning ("Control::DoApplyTemplate () Template root was not a UIElement");
		root->unref ();
		root = NULL;
	}

	if (!root)
		return FrameworkElement::DoApplyTemplate ();

	// No need to ref template_root here as ElementAdded refs it
	// and it is cleared when ElementRemoved is called.
	template_root = (UIElement *)root;
	ElementAdded (template_root);

	if (GetSurface()) {
		bool post = false;

		((UIElement*)root)->WalkTreeForLoadedHandlers (&post, true, true);

		if (post)
			Deployment::GetCurrent()->PostLoaded ();
	}
	
	return true;
}

void
Control::ElementAdded (UIElement *item)
{
	MoonError e;
	item->SetParent (this, &e);
	SetSubtreeObject (item);
	FrameworkElement::ElementAdded (item);
}

void
Control::ElementRemoved (UIElement *item)
{
	template_root = NULL;
	MoonError e;
	item->SetParent (NULL, &e);
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

		// A control is focusable if it is attached to a visual tree whose root
		// element has been loaded
		bool loaded = false;
		for (UIElement *check = this; !loaded && check != NULL; check = check->GetVisualParent ())
			loaded |= check->IsLoaded ();

		if (loaded && c->GetRenderVisible () && c->GetIsTabStop ())
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
