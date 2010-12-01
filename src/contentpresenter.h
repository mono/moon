/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentpresenter.h:
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __CONTENT_PRESENTER_H__
#define __CONTENT_PRESENTER_H__

#include <glib.h>
#include "frameworkelement.h"
#include "validators.h"

namespace Moonlight {

/* @ContentProperty="Content" */
/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class ContentPresenter : public FrameworkElement {
 protected:
	/* @GeneratePInvoke */
	ContentPresenter ();
	
	virtual ~ContentPresenter ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
	void ClearRoot ();
	
 public:
 	/* @PropertyType=object,Validator=ContentControlContentValidator */
	const static int ContentProperty;
 	/* @PropertyType=DataTemplate,GenerateAccessors */
	const static int ContentTemplateProperty;
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	//
	// Property Accessors
	//
	void SetContentTemplate (DataTemplate *t);
	DataTemplate *GetContentTemplate ();
	
	//
	// Events
	//
	/* @GenerateManagedEvent=false */
	const static int ContentPresenterClearRootEvent;
};

};

#endif /* __CONTENT_PRESENTER_H__ */
