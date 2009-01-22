/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentpresenter.cpp:
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "contentpresenter.h"


void
ContentPresenter::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType () != Type::CONTENTPRESENTER) {
		Control::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == ContentPresenter::ContentTemplateProperty) {
		if (IsLoaded ())
			ApplyTemplate ();
	} else if (args->property == ContentPresenter::ContentProperty) {
		if (IsLoaded ())
			ApplyTemplate ();
	}
	
	NotifyListenersOfPropertyChange (args);
}
