/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * richtextbox.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __RICHTEXTBOX_H__
#define __RICHTEXTBOX_H__

#include <glib.h>
#include <cairo.h>

#include "textelement.h"
#include "control.h"

/* @Namespace=System.Windows.Controls */
/* @ContentProperty=Blocks */
class RichTextArea : public Control {
 protected:
	virtual ~RichTextArea () {}
	
 public:
	/* @PropertyType=BlockCollection,AutoCreateValue,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int BlocksProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsReadOnlyProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,GenerateAccessors */
	const static int TextAlignmentProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,GenerateAccessors */
	const static int TextWrappingProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityAuto,ManagedFieldAccess=Private */
	const static int HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityAuto,ManagedFieldAccess=Private */
	const static int VerticalScrollBarVisibilityProperty;
	
	/* @GeneratePInvoke,GenerateCBinding */
	RichTextArea ();
	
	//
	// Property Accessors
	//
	void SetIsReadOnly (bool readonly);
	bool GetIsReadOnly ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
	
	/* @DelegateType=RoutedEventHandler */
	const static int SelectionChangedEvent;
};

#endif /* __RICHTEXTBOX_H__ */
