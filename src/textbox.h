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

#include "moon-path.h"
#include "eventargs.h"
#include "thickness.h"
#include "control.h"
#include "layout.h"
#include "brush.h"
#include "size.h"
#include "font.h"

G_BEGIN_DECLS

void textbox_shutdown (void);

G_END_DECLS


/* @SilverlightVersion="2" */
/* @Namespace=None */
class SelectionChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~SelectionChangedEventArgs () { }
	
 public:
	GPtrArray *removed;
	GPtrArray *added;
	
	/* @GenerateCBinding,GeneratePInvoke */
	SelectionChangedEventArgs () { removed = NULL; added = NULL; }
	SelectionChangedEventArgs (GPtrArray *removedItems, GPtrArray *addedItems)
	{
		removed = removedItems;
		added = addedItems;
	}
	
	virtual Type::Kind GetObjectType () { return Type::SELECTIONCHANGEDEVENTARGS; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetAddedItems (GPtrArray *addedItems) { added = addedItems; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	GPtrArray *GetAddedItems () { return added; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetRemovedItems (GPtrArray *removedItems) { removed = removedItems; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	GPtrArray *GetRemovedItems () { return removed; }
};


/* @SilverlightVersion="2" */
/* @Namespace=None */
class TextChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~TextChangedEventArgs () { }
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TextChangedEventArgs () { }
	
	virtual Type::Kind GetObjectType () { return Type::TEXTCHANGEDEVENTARGS; }
};


class TextBuffer;

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class TextBox : public Control {
	TextFontDescription *font;
	TextLayoutHints *hints;
	TextLayout *layout;
	TextBuffer *buffer;
	int maxlen;
	int caret;
	
	TextSelection selection;
	
	void Layout (cairo_t *cr);
	void Paint (cairo_t *cr);
	
	//
	// Private Property Accessors
	//
	void SetSelectionStart (int start);
	void SetSelectionLength (int length);
	
 protected:
 	bool dirty;
 	
	virtual ~TextBox ();
	
 public:
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	static DependencyProperty *AcceptsReturnProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	static DependencyProperty *IsReadOnlyProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors */
	static DependencyProperty *MaxLengthProperty;
	/* @PropertyType=string,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *SelectedTextProperty;
	/* @PropertyType=Brush,DefaultValue=0,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionBackgroundProperty;
	/* @PropertyType=Brush,DefaultValue=0,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionForegroundProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *SelectionLengthProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *SelectionStartProperty;
	/* @PropertyType=string,Version=2.0,GenerateAccessors */
	static DependencyProperty *TextProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,Version=2.0,GenerateAccessors */
	static DependencyProperty *TextAlignmentProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,Version=2.0,GenerateAccessors */
	static DependencyProperty *TextWrappingProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *VerticalScrollBarVisibilityProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TextBox ();
	
	virtual Type::Kind GetObjectType () { return Type::TEXTBOX; }
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual Value *GetValue (DependencyProperty *property);
	virtual Size ArrangeOverride (Size size);
	
	//
	// Methods
	//
	/* @GenerateCBinding,GeneratePInvoke */
	void SelectAll ();
	/* @GenerateCBinding,GeneratePInvoke */
	void Select (int start, int length);
	
	//
	// Property Accessors
	//
	void SetAcceptsReturn (bool accept);
	bool GetAcceptsReturn ();
	
	void SetHorizontalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetHorizontalScrollBarVisibility ();
	
	void SetIsReadOnly (bool readOnly);
	bool GetIsReadOnly ();
	
	void SetMaxLength (int length);
	int GetMaxLength ();
	
	void SetSelectionBackground (Brush *background);
	Brush *GetSelectionBackground ();
	
	void SetSelectionForeground (Brush *foreground);
	Brush *GetSelectionForeground ();
	
	void SetSelectedText (const char *text);
	const char *GetSelectedText ();
	
	int GetSelectionStart ();
	int GetSelectionLength ();
	
	void SetText (const char *text);
	const char *GetText ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
	
	void SetVerticalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetVerticalScrollBarVisibility ();
	
	//
	// Events
	//
	const static int SelectionChangedEvent;
	const static int TextChangedEvent;
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class PasswordBox : public TextBox {
 public:
 	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors */
 	static DependencyProperty *MaxLengthProperty;
 	/* @PropertyType=string,ManagedFieldAccess=Internal,Version=2.0,GenerateAccessors */
 	static DependencyProperty *PasswordProperty;
 	/* @PropertyType=string,Version=2.0,GenerateAccessors */
 	static DependencyProperty *PasswordCharProperty;
 	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
 	static DependencyProperty *SelectionBackgroundProperty;
 	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
 	static DependencyProperty *SelectionForegroundProperty;
 	
 	/* @SilverlightVersion="2" */
	const static int PasswordChangedEvent;

 	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
 	/* @GenerateCBinding,GeneratePInvoke */
	PasswordBox () { }
	
 	virtual Type::Kind GetObjectType () { return Type::PASSWORDBOX; }
 	
 	int GetMaxLength ();
	void SetMaxLength (int length);
	
 	const char *GetPassword ();
	void SetPassword (const char* password);
	
	const char *GetPasswordChar ();
	void SetPasswordChar (const char *passwordChar);
	
 	Brush *GetSelectionBackground ();
	void SetSelectionBackground (Brush *brush);
	
 	Brush *GetSelectionForeground ();
	void SetSelectionForeground (Brush *brush);
 private:
 	
};

#endif /* __TEXTBOX_H__ */
