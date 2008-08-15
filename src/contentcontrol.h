/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentcontrol.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
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
 	/* @PropertyType=bool,DefaultValue=true */
	static DependencyProperty *IsEnabledProperty;
 	/* @PropertyType=gint32,DefaultValue=TextAlignmentLeft */
	static DependencyProperty *TextAlignmentProperty;
 	/* @PropertyType=gint32,DefaultValue=TextDecorationsNone */
	static DependencyProperty *TextDecorationsProperty;
 	/* @PropertyType=gint32,DefaultValue=TextWrappingNoWrap */
	static DependencyProperty *TextWrappingProperty;
	
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
	
	void SetIsEnabled (bool value);
	bool GetIsEnabled ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextDecorations (TextDecorations decorations);
	TextDecorations GetTextDecorations ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
};

#endif /* __MOON_CONTENT_CONTROL_H__ */
