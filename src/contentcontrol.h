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


namespace Moonlight {

/* @Namespace=None */
class ContentControlChangedEventArgs : public EventArgs {
	Value *old_content;
	Value *new_content;
	
 protected:
	virtual ~ContentControlChangedEventArgs () {}
	
 public:
	ContentControlChangedEventArgs (Value *old_content, Value *new_content);
	
	/* @GeneratePInvoke */
	Value *GetOldContent ();
	
	/* @GeneratePInvoke */
	Value *GetNewContent ();
};


/* @ContentProperty="Content" */
/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class ContentControl : public Control {
 protected:
	/* @GeneratePInvoke */
	ContentControl ();
	
	virtual ~ContentControl ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
 	/* @PropertyType=object,Validator=ContentControlContentValidator */
	const static int ContentProperty;
 	/* @PropertyType=DataTemplate,GenerateAccessors */
	const static int ContentTemplateProperty;
	
	virtual UIElement *GetDefaultTemplate ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	//
	// Property Accessors
	//
	void SetContentTemplate (DataTemplate *t);
	DataTemplate *GetContentTemplate ();
	
	/* @GeneratePInvoke */
	void SetContentSetsParent (bool value) { content_sets_parent = value; }
	/* @GeneratePInvoke */
	bool GetContentSetsParent () { return content_sets_parent; }
	
	//
	// Events
	//
	/* @GenerateManagedEvent=false */
	const static int ContentControlChangedEvent;

  private:
	bool content_sets_parent;
};

};

#endif /* __CONTENT_CONTROL_H__ */
