/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * popup.cpp
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "popup.h"
#include "runtime.h"
#include "deployment.h"
#include "canvas.h"

namespace Moonlight {

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
Popup::OnIsAttachedChanged (bool attached)
{
	FrameworkElement::OnIsAttachedChanged (attached);
	if (!attached && GetIsOpen ())
		SetIsOpen (false);
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

			providers.inherited->ClearInheritedPropertiesOnRemovingFromTree (el);

			el->SetLogicalParent (NULL, error);

			if (error->number)
				return;
		}
		if (args->GetNewValue () && !args->GetNewValue ()->GetIsNull ()) {
			FrameworkElement *el = args->GetNewValue ()->AsFrameworkElement ();
			el->SetLogicalParent (this, error);
			if (error->number) 
				return;
			
			providers.inherited->PropagateInheritedPropertiesOnAddingToTree (el);

			if (GetIsOpen ())
				Show (el);
		}
	} else if (args->GetId () == Popup::HorizontalOffsetProperty
		   || args->GetId () == Popup::VerticalOffsetProperty) {
		UIElement * child = GetChild ();
		if (child)
			child->InvalidateMeasure ();
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
}

void
Popup::Show (UIElement *child)
{
	if (visible || !child)
		return;

	visible = true;
	Deployment::GetCurrent ()->GetSurface ()->AttachLayer (child);
}

};
