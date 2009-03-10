/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentcontrol.cpp:
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "contentcontrol.h"


ContentControl::ContentControl ()
{
	ManagedTypeInfo *type_info = new ManagedTypeInfo ("System.Windows", "System.Windows.Controls.ContentControl");
	
	SetObjectType (Type::CONTENTCONTROL);
	SetDefaultStyleKey (type_info);
}

ContentControl::~ContentControl ()
{
}

void
ContentControl::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->GetProperty ()->GetOwnerType () != Type::CONTENTCONTROL) {
		Control::OnPropertyChanged (args);
		return;
	}
	
	if (args->GetId () == ContentControl::ContentProperty) {
		Emit (ContentControl::ContentChangedEvent, new ContentChangedEventArgs (args->old_value, args->new_value));
	}
	
	NotifyListenersOfPropertyChange (args);
}

ContentChangedEventArgs::ContentChangedEventArgs (Value *old_content, Value *new_content)
{
	SetObjectType (Type::CONTENTCHANGEDEVENTARGS);

	this->old_content = old_content;
	this->new_content = new_content;
}

ContentChangedEventArgs::~ContentChangedEventArgs ()
{
}

Value*
ContentChangedEventArgs::GetOldContent ()
{
	return old_content;
}

Value*
ContentChangedEventArgs::GetNewContent ()
{
	return new_content;
}
