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

/* @Namespace=None */
class ContentChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~ContentChangedEventArgs () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	ContentChangedEventArgs () { SetObjectType (Type::CONTENTCHANGEDEVENTARGS); }
};

/* @Namespace=None */
class TextPointer : public DependencyObject {
 protected:
	virtual ~TextPointer () {}
	
 public:
	/* @GeneratePInvoke,GenerateCBinding */
	TextPointer ();
};

/* @Namespace=None */
class TextSelection : public DependencyObject {
 protected:
	virtual ~TextSelection () {}
	
 public:
	/* @GeneratePInvoke,GenerateCBinding */
	TextSelection ();
};

/* @Namespace=System.Windows.Controls */
/* @ContentProperty=Blocks */
class RichTextArea : public Control {
	void SetBaselineOffset (double offset);
	void SetSelection (TextSelection *selection);
	
 protected:
	virtual ~RichTextArea () {}
	
 public:
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	const static int AcceptsReturnProperty;
	/* @PropertyType=double,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int BaselineOffsetProperty;
	/* @PropertyType=BlockCollection,AutoCreateValue,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int BlocksProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	const static int CaretBrushProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsReadOnlyProperty;
	/* @PropertyType=TextSelection,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int SelectionProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,GenerateAccessors */
	const static int TextAlignmentProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,GenerateAccessors */
	const static int TextWrappingProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityAuto,ManagedFieldAccess=Private */
	const static int HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityAuto,ManagedFieldAccess=Private */
	const static int VerticalScrollBarVisibilityProperty;
	/* @PropertyType=string,GenerateAccessors */
	const static int XamlProperty;
	
	/* @GeneratePInvoke,GenerateCBinding */
	RichTextArea ();
	
	//
	// Property Accessors
	//
	void SetAcceptsReturn (bool accept);
	bool GetAcceptsReturn ();
	
	double GetBaselineOffset ();
	
	void SetBlocks (BlockCollection *blocks);
	BlockCollection *GetBlocks ();
	
	void SetCaretBrush (Brush *caret);
	Brush *GetCaretBrush ();
	
	void SetIsReadOnly (bool readonly);
	bool GetIsReadOnly ();
	
	TextSelection *GetSelection ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
	
	void SetXaml (const char *xaml);
	const char *GetXaml ();
	
	/* @DelegateType=ContentChangedEventHandler */
	const static int ContentChangedEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int SelectionChangedEvent;
};

#endif /* __RICHTEXTBOX_H__ */
