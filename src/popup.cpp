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
		Emit (Popup::IsOpenChangedEvent);
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
	NotifyListenersOfPropertyChange (args);
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
		
	visible = true;

	if (child)
		Deployment::GetCurrent ()->GetSurface ()->AttachLayer (child);
}
