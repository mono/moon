/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentcontrol.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "application.h"
#include "contentcontrol.h"
#include "managedtypeinfo.h"

namespace Moonlight {

ContentControl::ContentControl ()
{
	SetContentSetsParent (true);
	SetObjectType (Type::CONTENTCONTROL);

	ManagedTypeInfo type_info (GetObjectType ());
	SetDefaultStyleKey (&type_info);
}

ContentControl::~ContentControl ()
{
}

UIElement *
ContentControl::GetDefaultTemplate ()
{
	Value *content = GetValue (ContentControl::ContentProperty);
	if (!content || content->GetIsNull ())
		return NULL;

	if (content->Is (GetDeployment (), Type::UIELEMENT))
		return content->AsUIElement ();
	
	return Control::GetDefaultTemplate ();
}

void
ContentControl::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::CONTENTCONTROL) {
		Control::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == ContentControl::ContentProperty) {
		if (args->GetOldValue() && args->GetOldValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
			if (GetContentSetsParent ()) {
				args->GetOldValue()->AsFrameworkElement()->SetLogicalParent (NULL, error);
				if (error->number)
					return;
			}
		}
		if (args->GetNewValue() && args->GetNewValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
			if (GetContentSetsParent ()) {
				args->GetNewValue()->AsFrameworkElement()->SetLogicalParent (this, error);
				if (error->number)
					return;
			}
		}

		if (HasHandlers (ContentControl::ContentControlChangedEvent))
			Emit (ContentControl::ContentControlChangedEvent, new ContentControlChangedEventArgs (args->GetOldValue(), args->GetNewValue()));
		InvalidateMeasure ();
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

ContentControlChangedEventArgs::ContentControlChangedEventArgs (Value *old_content, Value *new_content)
{
	SetObjectType (Type::CONTENTCONTROLCHANGEDEVENTARGS);

	this->old_content = old_content;
	this->new_content = new_content;
}

Value *
ContentControlChangedEventArgs::GetOldContent ()
{
	return old_content;
}

Value*
ContentControlChangedEventArgs::GetNewContent ()
{
	return new_content;
}

};
