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
	if (args->GetId () == Popup::IsOpenProperty) {
		Emit (Popup::IsOpenChangedEvent);
		if (args->GetNewValue () && args->GetNewValue ()->AsBool ())
			Show ();
		else
			Hide ();
	}
	DependencyObject::OnPropertyChanged (args, error);
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
