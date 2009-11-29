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


/* @Namespace=None */
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
/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class ContentControl : public Control {
 protected:
	virtual ~ContentControl ();
	
 public:
 	/* @PropertyType=object,Validator=ContentControlContentValidator */
	const static int ContentProperty;
 	/* @PropertyType=DataTemplate,GenerateAccessors */
	const static int ContentTemplateProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ContentControl ();
	
	virtual UIElement *GetDefaultTemplate ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	//
	// Property Accessors
	//
	void SetContentTemplate (DataTemplate *t);
	DataTemplate *GetContentTemplate ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetContentSetsParent (bool value) { content_sets_parent = value; }
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetContentSetsParent () { return content_sets_parent; }
	
	//
	// Events
	//
	/* @GenerateManagedEvent=false */
	const static int ContentChangedEvent;

  private:
	bool content_sets_parent;
};

#endif /* __CONTENT_CONTROL_H__ */
