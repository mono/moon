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


enum TextBoxModelChangeType {
	TextBoxModelChangedNothing,
	TextBoxModelChangedCursorPosition,
	TextBoxModelChangedSelection,
	TextBoxModelChangedLayout,
	TextBoxModelChangedBrush
};

/* @SilverlightVersion="2" */
/* @Namespace=None */
class TextBoxModelChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~TextBoxModelChangedEventArgs () { }
	
 public:
	TextBoxModelChangeType changed;
	
	TextBoxModelChangedEventArgs (TextBoxModelChangeType changed) { this->changed = changed; }
	
	virtual Type::Kind GetObjectType () { return Type::TEXTBOXMODELCHANGEDEVENTARGS; }
};



class TextBuffer;

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class TextBox : public Control {
	TextFontDescription *font;
	TextSelection selection;
	TextLayoutHints *hints;
	TextBuffer *buffer;
	int maxlen;
	int caret;
	
	static void key_down (EventObject *sender, EventArgs *args, void *closure);
	static void key_up (EventObject *sender, EventArgs *args, void *closure);
	
	void OnKeyDown (KeyEventArgs *args);
	void OnKeyUp (KeyEventArgs *args);
	
	//
	// Private Property Accessors
	//
	void SetSelectionStart (int start);
	void SetSelectionLength (int length);
	
 protected:
 	virtual ~TextBox ();
	
 public:
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	static DependencyProperty *AcceptsReturnProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	static DependencyProperty *IsReadOnlyProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors,Validator=PositiveIntValidator */
	static DependencyProperty *MaxLengthProperty;
	/* @PropertyType=string,DefaultValue=\"\",Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *SelectedTextProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionBackgroundProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionForegroundProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	static DependencyProperty *SelectionLengthProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	static DependencyProperty *SelectionStartProperty;
	/* @PropertyType=string,DefaultValue=\"\",Version=2.0,GenerateAccessors */
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
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual Size ArrangeOverride (Size size);
	virtual void OnApplyTemplate ();
	
	//
	// Methods
	//
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SelectAll ();
	/* @GenerateCBinding,GeneratePInvoke */
	void Select (int start, int length);
	
	// Methods needed by TextBoxView
	TextFontDescription *GetFontDescription () { return font; }
	TextLayoutHints *GetLayoutHints () { return hints; }
	TextSelection *GetSelection () { return &selection; }
	TextBuffer *GetBuffer () { return buffer; }
	
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
	
	// Internal Events
	const static int ModelChangedEvent;
};


/* @SilverlightVersion="2" */
/* @Namespace=Microsoft.Internal */
class TextBoxView : public FrameworkElement {
	TextLayout *layout;
	bool dirty;
	
	glong blink_timeout;
	bool cursor_visible;
	Rect cursor;
	
	static void focus_out (EventObject *sender, EventArgs *args, gpointer closure);
	static void focus_in (EventObject *sender, EventArgs *args, gpointer closure);
	void OnFocusOut (EventArgs *args);
	void OnFocusIn (EventArgs *args);
	
	static void model_changed (EventObject *sender, EventArgs *args, gpointer closure);
	void OnModelChanged (TextBoxModelChangedEventArgs *args);
	
	static gboolean blink (void *user_data);
	void ConnectBlinkTimeout (guint multiplier);
	void DelayCursorBlink ();
	void BeginCursorBlink ();
	void EndCursorBlink ();
	void ShowCursor ();
	void HideCursor ();
	bool Blink ();
	
	void Layout (cairo_t *cr);
	void Paint (cairo_t *cr);
	
 protected:
	virtual ~TextBoxView ();
	
 public:
	/* @PropertyType=TextBox,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	static DependencyProperty *TextBoxProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TextBoxView ();
	
	virtual Type::Kind GetObjectType () { return Type::TEXTBOXVIEW; }
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual Size ArrangeOverride (Size size);
	
	//
	// Property Accessors
	//
	void SetTextBox (TextBox *textbox);
	TextBox *GetTextBox ();
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class PasswordBox : public TextBox {
 public:
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	static DependencyProperty *MaxLengthProperty;
	/* @PropertyType=string,DefaultValue=\"\",Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PasswordValidator */
	static DependencyProperty *PasswordProperty;
	/* @PropertyType=char,DefaultValue=9679,Version=2.0,GenerateAccessors */
	static DependencyProperty *PasswordCharProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionBackgroundProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	static DependencyProperty *SelectionForegroundProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PasswordBox () { }
	
	virtual Type::Kind GetObjectType () { return Type::PASSWORDBOX; }
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	//
	// Property Accesors
	//
	void SetMaxLength (int length);
	int GetMaxLength ();
	
	void SetPassword (const char *password);
	const char *GetPassword ();
	
	void SetPasswordChar (int passwordChar);
	int GetPasswordChar ();
	
	void SetSelectionBackground (Brush *brush);
	Brush *GetSelectionBackground ();
	
	void SetSelectionForeground (Brush *brush);
	Brush *GetSelectionForeground ();
	
	//
	// Events
	//
	const static int PasswordChangedEvent;
};

#endif /* __TEXTBOX_H__ */
