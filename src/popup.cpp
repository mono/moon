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
	visible = false;
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
			Show ();
		else
			Hide ();
	} else if (args->GetId () == Popup::ChildProperty) {
		if (args->GetOldValue () && !args->GetOldValue ()->GetIsNull ()) {
			FrameworkElement *el = args->GetOldValue ()->AsFrameworkElement ();
			if (el->GetLogicalParent () == this)
				el->SetLogicalParent (NULL, error);
			if (error->number)
				return;
		}
		if (args->GetNewValue () && !args->GetNewValue ()->GetIsNull ()) {
			FrameworkElement *el = args->GetNewValue ()->AsFrameworkElement ();
			args->GetNewValue ()->AsFrameworkElement ()->SetLogicalParent (this, error);
			if (error->number)
				return;
		}	
	}
	NotifyListenersOfPropertyChange (args);
}

void
Popup::Hide ()
{
	if (!visible)
		return;

	visible = false;
	UIElement *child = GetChild ();
	if (child) {
		Deployment::GetCurrent ()->GetSurface ()->DetachLayer (child);
	}
}

void
Popup::Show ()
{
	if (visible)
		return;
		
	visible = true;
	UIElement *child = GetChild ();
	if (child) {
		Deployment::GetCurrent ()->GetSurface ()->AttachLayer (child);
	}
}
