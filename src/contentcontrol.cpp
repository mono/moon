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
	
	// basically when either Content or ContentTemplate changes
	// you need to reapply the template, get the new root, and
	// then call ElementRemoved on the old visual tree root and
	// ElementAdded on the new root.
	
	if (args->property == ContentControl::ContentTemplateProperty) {
		
	} else if (args->property == ContentControl::ContentProperty) {
		
	}
	
	NotifyListenersOfPropertyChange (args);
}
