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

#include "control.h"
#include "text.h"
#include "grid.h"

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class ContentPresenter : public Control {
 protected:
	virtual ~ContentPresenter () { }
	
 public:
 	/* @PropertyType=object */
	static DependencyProperty *ContentProperty;
 	/* @PropertyType=DataTemplate,GenerateAccessors */
	static DependencyProperty *ContentTemplateProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ContentPresenter ();
	
	virtual Type::Kind GetObjectType () { return Type::CONTENTPRESENTER; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnLoaded ();
	
	//
	// Property Accessors
	//
	void SetContentTemplate (DataTemplate *t);
	DataTemplate *GetContentTemplate ();
};

#endif /* __CONTENT_PRESENTER_H__ */
