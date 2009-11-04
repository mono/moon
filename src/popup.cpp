/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * popup.cpp
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "popup.h"
#include "runtime.h"
#include "deployment.h"
	
Popup::Popup ()
{
	SetObjectType (Type::POPUP);
	shutting_down = false;
	visible = false;
	GetDeployment ()->AddHandler (Deployment::ShuttingDownEvent, ShuttingDownCallback, this);
}

void
Popup::Dispose ()
{
	if (!shutting_down && GetIsOpen ())
		Hide (GetChild ());
	GetDeployment ()->RemoveHandler (Deployment::ShuttingDownEvent, ShuttingDownCallback, this);
	FrameworkElement::Dispose ();
}

void
Popup::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	if (visible)
		FrameworkElement::HitTest (cr, p, uielement_list);
}

void
Popup::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::POPUP) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Popup::IsOpenProperty) {
		// we intentionally don't track whether we've added a tick
		// call (to make sure we only add it once) for this event
		// because multiple IsOpen changes cause multiple async events
		// in SL.
		if (args->GetNewValue () && args->GetNewValue ()->AsBool ()) {
			Show (GetChild ());
			
			EmitAsync (Popup::OpenedEvent);
		}
		else {
			Hide (GetChild ());
			
			EmitAsync (Popup::ClosedEvent);
		}
	} else if (args->GetId () == Popup::ChildProperty) {
		if (args->GetOldValue () && !args->GetOldValue ()->GetIsNull ()) {
			FrameworkElement *el = args->GetOldValue ()->AsFrameworkElement ();
			if (GetIsOpen ())
				Hide (el);

			el->SetLogicalParent (NULL, error);
			if (error->number)
				return;
		}
		if (args->GetNewValue () && !args->GetNewValue ()->GetIsNull ()) {
			FrameworkElement *el = args->GetNewValue ()->AsFrameworkElement ();
			el->SetLogicalParent (this, error);
			if (error->number) 
				return;
			
			if (GetIsOpen ())
				Show (el);
		}
	} else if (args->GetId () == Popup::HorizontalOffsetProperty
		   || args->GetId () == Popup::VerticalOffsetProperty) {
		UIElement * child = GetChild ();
		if (child)
			child->UpdateTransform ();
	}
	NotifyListenersOfPropertyChange (args, error);
}

void
Popup::ShuttingDownHandler (Deployment *sender, EventArgs *args)
{
	shutting_down = true;
}

void
Popup::Hide (UIElement *child)
{
	if (!visible || !child)
		return;

	visible = false;
	Deployment::GetCurrent ()->GetSurface ()->DetachLayer (child);
	// When the popup is closed, we signal the child that its parent
	// is enabled as it no longer has any parent
	PropagateIsEnabledState (child, true);
}

void
Popup::SetSurface (Surface *s)
{
	FrameworkElement::SetSurface (s);
}

void
Popup::Show (UIElement *child)
{
	if (visible || !child)
		return;

	visible = true;
	Deployment::GetCurrent ()->GetSurface ()->AttachLayer (child);
	PropagateIsEnabledState (child, Control::GetParentEnabledState (this));
}

void
Popup::PropagateIsEnabledState (UIElement *child, bool enabled_parent)
{
	DeepTreeWalker walker (child);
	while (UIElement *e = walker.Step ()) {
		if (!e->Is (Type::CONTROL))
			continue;
		
		Control *control = (Control *) e;
		control->enabled_parent = enabled_parent;
		control->UpdateEnabled ();
		walker.SkipBranch ();
	}
}