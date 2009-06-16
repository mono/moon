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

void
Popup::emit_opened (EventObject *sender)
{
	((Popup *)sender)->Emit (Popup::OpenedEvent);
}

void
Popup::emit_closed (EventObject *sender)
{
	((Popup *)sender)->Emit (Popup::ClosedEvent);
}
	
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
	GetDeployment ()->RemoveHandler (Deployment::ShuttingDownEvent, ShuttingDownCallback, this);
	FrameworkElement::Dispose ();
}

void
Popup::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::POPUP) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Popup::IsOpenProperty) {
		if (args->GetNewValue () && args->GetNewValue ()->AsBool ())
			Show (GetChild ());
		else
			Hide (GetChild ());
	} else if (args->GetId () == Popup::ChildProperty) {
		if (args->GetOldValue () && !args->GetOldValue ()->GetIsNull ()) {
			FrameworkElement *el = args->GetOldValue ()->AsFrameworkElement ();
			if (el->GetLogicalParent () == this) {
				Hide (el);
				el->SetLogicalParent (NULL, error);
			}
			if (error->number)
				return;
		}
		if (args->GetNewValue () && !args->GetNewValue ()->GetIsNull ()) {
			FrameworkElement *el = args->GetNewValue ()->AsFrameworkElement ();
			args->GetNewValue ()->AsFrameworkElement ()->SetLogicalParent (this, error);
			if (error->number) 
				return;
			
			if (GetIsOpen ())
				Show (el);
		}	
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
	if (!visible)
		return;

	// we intentionally don't track whether we've added a tick
	// call (to make sure we only add it once) for this event
	// because multiple IsOpen changes cause multiple async events
	// in SL.
	AddTickCall (Popup::emit_closed);
	visible = false;

	if (child)
		Deployment::GetCurrent ()->GetSurface ()->DetachLayer (child);
}

void
Popup::SetSurface (Surface *s)
{
	 if (!shutting_down && !s && GetIsOpen ())
	 	SetIsOpen (false);

	FrameworkElement::SetSurface (s);
}

void
Popup::Show (UIElement *child)
{
	if (visible)
		return;

	// we intentionally don't track whether we've added a tick
	// call (to make sure we only add it once) for this event
	// because multiple IsOpen changes cause multiple async events
	// in SL.
	AddTickCall (Popup::emit_opened);
	visible = true;

	if (child)
		Deployment::GetCurrent ()->GetSurface ()->AttachLayer (child);
}
