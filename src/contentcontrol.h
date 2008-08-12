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

G_BEGIN_DECLS

#include "control.h"

//
// ContentControl Class
//
/* @ContentProperty="Content" */
/* @SilverlightVersion="2" */
/* @Namespace=None  */ // This class is from MS sources, so the DP is registered there.
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
	
	/* @GenerateCBinding */
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

void content_control_set_content (ContentControl *content_control, DependencyObject *content);
DependencyObject *content_control_get_content (ContentControl *content_control);

//void content_control_set_content_template (ContentControl *content_control, DataTemplate *t);
//DataTemplate *content_control_get_content_template (ContentControl *content_control);

void content_control_set_is_enabled (ContentControl *content_control, bool value);
bool content_control_get_is_enabled (ContentControl *content_control);

void content_control_set_text_alignment (ContentControl *content_control, TextAlignment alignment);
TextAlignment content_control_get_text_alignment (ContentControl *content_control);

void content_control_set_text_decorations (ContentControl *content_control, TextDecorations decorations);
TextDecorations content_control_get_text_decorations (ContentControl *content_control);

void content_control_set_text_wrapping (ContentControl *content_control, TextWrapping wrapping);
TextWrapping content_control_get_text_wrapping (ContentControl *content_control);

G_END_DECLS

#endif /* __MOON_CONTENT_CONTROL_H__ */
