/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textbox.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXTBOX_H__
#define __TEXTBOX_H__

#include <glib.h>
#include <cairo.h>

#include <moon-path.h>
#include <eventargs.h>
#include <thickness.h>
#include <control.h>
#include <layout.h>
#include <brush.h>
#include <font.h>


/* @SilverlightVersion="2" */
/* @ContentProperty="Text" */
/* @Namespace=System.Windows.Documents */
class TextBox : public Control {
 protected:
	virtual ~TextBox () { }
	
 public:
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	static DependencyProperty *AcceptsReturnProperty;
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	static DependencyProperty *IsReadOnlyProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors */
	static DependencyProperty *MaxLengthProperty;
	/* @PropertyType=Brush,DefaultValue=0,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionBackgroundProperty;
	/* @PropertyType=Brush,DefaultValue=0,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionForegroundProperty;
	/* @PropertyType=string,Version=2.0,GenerateAccessors */
	static DependencyProperty *TextProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,Version=2.0,GenerateAccessors */
	static DependencyProperty *TextAlignmentProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,Version=2.0,GenerateAccessors */
	static DependencyProperty *TextWrappingProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TextBox () { }
	
	virtual Type::Kind GetObjectType () { return Type::TEXTBOX; }
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetTransformOrigin ();
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	virtual Value *GetValue (DependencyProperty *property);
	
	//
	// Property Accessors
	//
	void SetAcceptsReturn (bool accept);
	bool GetAcceptsReturn ();
	
	void SetIsReadOnly (bool readOnly);
	bool GetIsReadOnly ();
	
	void SetMaxLength (int length);
	int GetMaxLength ();
	
	void SetSelectionBackground (Brush *background);
	Brush *GetSelectionBackground ();
	
	void SetSelectionForeground (Brush *foreground);
	Brush *GetSelectionForeground ();
	
	void SetText (const char *text);
	const char *GetText ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
};

#endif /* __TEXTBOX_H__ */
