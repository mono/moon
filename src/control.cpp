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
#include "validators.h"
#include "style.h"

namespace Moonlight {

Control::Control ()
	: template_root (this, TemplateRootWeakRef)
{
	SetObjectType (Type::CONTROL);

	providers.isenabled = new InheritedIsEnabledValueProvider (this, PropertyPrecedence_IsEnabled);
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
Control::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::CONTROL) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Control::TemplateProperty) {
		if (GetSubtreeObject ())
			ElementRemoved ((UIElement *) GetSubtreeObject ());
		InvalidateMeasure ();
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
		PropertyChangedEventArgs *pargs = new PropertyChangedEventArgs (args->GetProperty(),
										args->GetId (),
										args->GetOldValue(),
										args->GetNewValue(),
										true);
		EmitAsync (IsEnabledChangedEvent, pargs);
	} else if (args->GetId () == Control::HorizontalContentAlignmentProperty
		   || args->GetId () == Control::VerticalContentAlignmentProperty) {
		InvalidateArrange ();
	}
	NotifyListenersOfPropertyChange (args, error);
}

void
Control::OnIsAttachedChanged (bool attached)
{
	FrameworkElement::OnIsAttachedChanged (attached);
	providers.isenabled->SetDataSource (GetLogicalParent ());
}


void
Control::SetVisualParent (UIElement *visual_parent)
{
	if (GetVisualParent () != visual_parent) {
		FrameworkElement::SetVisualParent (visual_parent);
		providers.isenabled->SetDataSource (GetLogicalParent ());
 	}
}

void
Control::OnLogicalParentChanged (DependencyObject *old_parent, DependencyObject *new_parent)
{
	FrameworkElement::OnLogicalParentChanged  (old_parent, new_parent);
	providers.isenabled->SetDataSource (new_parent);
}

void
Control::Dispose ()
{
	template_root = NULL;
	FrameworkElement::Dispose ();
}

bool
Control::DoApplyTemplateWithError (MoonError *error)
{
	ControlTemplate *t = GetTemplate ();
	if (!t)
		return FrameworkElement::DoApplyTemplateWithError (error);

	// If the template expands to an element which is *not* a UIElement
	// we don't apply the template.
	DependencyObject *root = t->GetVisualTreeWithError (this, error);
	if (root && !root->Is (Type::UIELEMENT)) {
		g_warning ("Control::DoApplyTemplate () Template root was not a UIElement");
		root->unref ();
		root = NULL;
	}

	if (!root)
		return FrameworkElement::DoApplyTemplateWithError (error);

	// No need to ref template_root here as ElementAdded refs it
	// and it is cleared when ElementRemoved is called.
	if (template_root != root && template_root != NULL) {
		template_root->RemoveParent (this, NULL);
		template_root->SetMentor (NULL);
		template_root = NULL;
	}

	template_root = (UIElement *) root;

	ElementAdded (template_root);

	if (IsLoaded ())
		GetDeployment ()->EmitLoadedAsync ();

	root->unref ();
	
	return true;
}


void
Control::UpdateIsEnabledSource (Control *control)
{
	providers.isenabled->SetDataSource (control);
}

void
Control::ElementAdded (UIElement *item)
{
	MoonError e;
	item->AddParent (this, &e);
	SetSubtreeObject (item);
	FrameworkElement::ElementAdded (item);
}

void
Control::ElementRemoved (UIElement *item)
{
	MoonError e;
	if (template_root != NULL) {
		template_root->RemoveParent (this, &e);
		template_root->SetMentor (NULL);
		template_root = NULL;
	}
	item->RemoveParent (this, &e);
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
	if (!IsAttached ())
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
	Types *types = GetDeployment ()->GetTypes ();
	Surface *surface = GetDeployment ()->GetSurface ();
	DeepTreeWalker walker (this);
	while (UIElement *e = walker.Step ()) {
		if (e->GetVisibility () != VisibilityVisible) {
			walker.SkipBranch ();
			continue;
		}

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

};
