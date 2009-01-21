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


void
ContentControl::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType () != Type::CONTENTCONTROL) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == ContentControl::ContentTemplateProperty) {
		if (IsLoaded ())
			ApplyTemplate ();
	} else if (args->property == ContentControl::ContentProperty) {
		if (IsLoaded ())
			ApplyTemplate ();
		
		Emit (ContentControl::ContentChangedEvent, new ContentChangedEventArgs (args->old_value, args->new_value));
	}
	
	NotifyListenersOfPropertyChange (args);
}
