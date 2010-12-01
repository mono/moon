/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentpresenter.cpp:
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
#include "contentpresenter.h"
#include "managedtypeinfo.h"

namespace Moonlight {

ContentPresenter::ContentPresenter ()
{
	SetObjectType (Type::CONTENTPRESENTER);
}

ContentPresenter::~ContentPresenter ()
{
}

void
ContentPresenter::ClearRoot ()
{
	if (HasHandlers (ContentPresenter::ContentPresenterClearRootEvent))
		Emit (ContentPresenter::ContentPresenterClearRootEvent, NULL);
}

void
ContentPresenter::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::CONTENTPRESENTER) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == ContentPresenter::ContentProperty) {
		Deployment *deployment = GetDeployment ();
		Value *old_value = args->GetOldValue ();
		Value *new_value = args->GetNewValue ();
		
		if (new_value && !new_value->Is (deployment, Type::UIELEMENT))
			SetValue (FrameworkElement::DataContextProperty, new_value);
		else
			ClearValue (FrameworkElement::DataContextProperty);
		
		if ((new_value && new_value->Is (deployment, Type::UIELEMENT)) ||
		    (old_value && old_value->Is (deployment, Type::UIELEMENT)))
			ClearRoot ();
		
		InvalidateMeasure ();
	} else if (args->GetId () == ContentPresenter::ContentTemplateProperty) {
		ClearRoot ();
		InvalidateMeasure ();
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

};
