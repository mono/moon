/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentcontrol.h:
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CONTENT_CONTROL_H__
#define __MOON_CONTENT_CONTROL_H__

#include <glib.h>
#include "control.h"

//
// ContentControl Class
//
/* @ContentProperty="Content" */
/* @SilverlightVersion="2" */
/* @Namespace=None  */
/* @ManagedDependencyProperties=Manual */  // This class is from MS sources, so the DP is registered there.
class ContentControl : public Control {
 protected:
	virtual ~ContentControl () {}
	
 public:
 	/* @PropertyType=DependencyObject */
	static DependencyProperty *ContentProperty;
	
 	/* @PropertyType=DependencyObject */
	static DependencyProperty *ContentTemplateProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ContentControl () {}
	
	virtual Type::Kind GetObjectType () { return Type::CONTENTCONTROL; }
	
	//
	// Property Accessors
	//
	void SetContent (DependencyObject *content);
	DependencyObject *GetContent ();
	
	//void SetContentTemplate (DataTemplate *t);
	//DataTemplate *GetContentTemplate ();
};

#endif /* __MOON_CONTENT_CONTROL_H__ */
