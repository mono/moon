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

ContentControl::ContentControl ()
{
	ManagedTypeInfo *type_info = g_new (ManagedTypeInfo, 1);
	type_info->Initialize ("System.Windows", "System.Windows.Controls.ContentControl");
	
	SetContentSetsParent (true);
	SetObjectType (Type::CONTENTCONTROL);
	SetDefaultStyleKey (type_info);
	ManagedTypeInfo::Free (type_info);
	
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
	bool clearTemplate = false;
	if (args->GetProperty ()->GetOwnerType () != Type::CONTENTCONTROL) {
		Control::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == ContentControl::ContentProperty) {
		if (args->GetOldValue() && args->GetOldValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
			clearTemplate = true;
			if (GetContentSetsParent ()) {
				args->GetOldValue()->AsFrameworkElement()->SetLogicalParent (NULL, error);
				if (error->number)
					return;
			}
		}
		if (args->GetNewValue() && args->GetNewValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
			clearTemplate = true;
			if (GetContentSetsParent ()) {
				args->GetNewValue()->AsFrameworkElement()->SetLogicalParent (this, error);
				if (error->number)
					return;
			}
		}
		// I'm not sure why this check is here, but it breaks some Silverlight Toolkit 2.0 (July edition)
		//tests (databinding a SolidColourBrush from FrameworkElement.Background to ContentControl.Content) and
		// nothing regresses when I comment this out.
		//if (!GetContentSetsParent () && args->GetNewValue () && args->GetNewValue()->Is (GetDeployment (), Type::DEPENDENCY_OBJECT) && !args->GetNewValue ()->Is (GetDeployment (), Type::FRAMEWORKELEMENT)) {
		//	MoonError::FillIn (error, MoonError::ARGUMENT, "");
		//	return;
		//}
		
		if (clearTemplate && GetSubtreeObject ())
			ElementRemoved ((UIElement *) GetSubtreeObject ());

		Emit (ContentControl::ContentChangedEvent, new ContentChangedEventArgs (args->GetOldValue(), args->GetNewValue()));
		InvalidateMeasure ();
	}
	
	NotifyListenersOfPropertyChange (args, error);
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
