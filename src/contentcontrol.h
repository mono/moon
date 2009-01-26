/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentcontrol.h:
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __CONTENT_CONTROL_H__
#define __CONTENT_CONTROL_H__

#include <glib.h>
#include "control.h"


/* @SilverlightVersion="2" */
/* @Namespace=None */
/* @IncludeInKinds */	
class ContentChangedEventArgs : public EventArgs {
	Value *old_content;
	Value *new_content;
	
 protected:
	virtual ~ContentChangedEventArgs ();
	
 public:
	ContentChangedEventArgs (Value *old_content, Value *new_content);
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetOldContent ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetNewContent ();
};


/* @ContentProperty="Content" */
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
/* @IncludeInKinds */	
class ContentControl : public Control {
 protected:
	virtual ~ContentControl ();
	
 public:
 	/* @PropertyType=object */
	static DependencyProperty *ContentProperty;
 	/* @PropertyType=DataTemplate,GenerateAccessors */
	static DependencyProperty *ContentTemplateProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ContentControl ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	//
	// Property Accessors
	//
	void SetContentTemplate (DataTemplate *t);
	DataTemplate *GetContentTemplate ();
	
	//
	// Events
	//
	const static int ContentChangedEvent;
};

#endif /* __CONTENT_CONTROL_H__ */
