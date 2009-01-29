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


/* @SilverlightVersion="2" */
/* @Namespace=None */
class TextChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~TextChangedEventArgs () { }
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TextChangedEventArgs () { SetObjectType (Type::TEXTCHANGEDEVENTARGS); }
};


enum TextBoxModelChangeType {
	TextBoxModelChangedNothing,
	TextBoxModelChangedCursorPosition,
	TextBoxModelChangedTextAlignment,
	TextBoxModelChangedTextWrapping,
	TextBoxModelChangedReadOnly,
	TextBoxModelChangedBrush,
	TextBoxModelChangedFont
};


/* @SilverlightVersion="2" */
/* @Namespace=None */
class TextBoxModelChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~TextBoxModelChangedEventArgs () { }
	
 public:
	PropertyChangedEventArgs *property;
	TextBoxModelChangeType changed;
	
	TextBoxModelChangedEventArgs (TextBoxModelChangeType changed, PropertyChangedEventArgs *property = NULL)
	{
		SetObjectType (Type::TEXTBOXMODELCHANGEDEVENTARGS);
		this->property = property;
		this->changed = changed;
	}
};


class TextBuffer;


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class TextBox : public Control, public ITextSource {
	friend class TextBoxView;
	
	TextFontDescription *font;
	TextSelection selection;
	TextBuffer *buffer;
	int maxlen;
	int cursor;
	
	int setvalue:1;
	int frozen:3;
	
 protected:
	int KeyPressUnichar (gunichar c);
	
	int KeyPressBackSpace (GdkModifierType modifiers);
	int KeyPressDelete (GdkModifierType modifiers);
	int KeyPressPageDown (GdkModifierType modifiers);
	int KeyPressPageUp (GdkModifierType modifiers);
	int KeyPressHome (GdkModifierType modifiers);
	int KeyPressEnd (GdkModifierType modifiers);
	int KeyPressRight (GdkModifierType modifiers);
	int KeyPressLeft (GdkModifierType modifiers);
	int KeyPressDown (GdkModifierType modifiers);
	int KeyPressUp (GdkModifierType modifiers);
	
	void PreKeyPress ();
	void PostKeyPress (int changed);
	
	void ClearSelection ();
	
	void EmitSelectionChanged (bool sync = true);
	void EmitTextChanged (bool sync = true);
	
	//
	// Protected Property Accessors
	//
	TextSelection *GetSelection () { return &selection; }
	TextBuffer *GetBuffer () { return buffer; }
	int GetCursor () { return cursor; }
	
	void SetSelectionStart (int start);
	void SetSelectionLength (int length);
	
	//
	// Protected Events
	//
	const static int ModelChangedEvent;
	
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
	
	//
	// Overrides
	//
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnApplyTemplate ();
	
	//
	// Methods
	//
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool SelectAll ();
	/* @GenerateCBinding,GeneratePInvoke */
	void Select (int start, int length);
	
	//
	// ITextSource Interface Methods
	//
	virtual TextDecorations Decorations () { return TextDecorationsNone; }
	virtual TextFontDescription *FontDescription () { return font; }
	
	virtual Brush *Background (bool selected)
	{
		if (selected)
			return GetSelectionBackground ();
		else
			return NULL;
	}
	
	virtual Brush *Foreground (bool selected)
	{
		if (selected)
			return GetSelectionForeground ();
		else
			return GetForeground ();
	}
	
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
/* @Namespace=Microsoft.Internal */
class TextBoxView : public FrameworkElement {
	glong blink_timeout;
	TextLayout *layout;
	TextBox *textbox;
	Rect cursor;
	
	int cursor_visible:1;
	int focused:1;
	int dirty:1;
	
	// focus in/out events
	static void focus_out (EventObject *sender, EventArgs *args, gpointer closure);
	static void focus_in (EventObject *sender, EventArgs *args, gpointer closure);
	void OnFocusOut (EventArgs *args);
	void OnFocusIn (EventArgs *args);
	
	// keypress events
	static void key_down (EventObject *sender, EventArgs *args, void *closure);
	static void key_up (EventObject *sender, EventArgs *args, void *closure);
	void OnKeyDown (KeyEventArgs *args);
	void OnKeyUp (KeyEventArgs *args);
	
	// TextBox events
	static void selection_changed (EventObject *sender, EventArgs *args, gpointer closure);
	static void model_changed (EventObject *sender, EventArgs *args, gpointer closure);
	static void text_changed (EventObject *sender, EventArgs *args, gpointer closure);
	void OnModelChanged (TextBoxModelChangedEventArgs *args);
	void OnSelectionChanged (RoutedEventArgs *args);
	void OnTextChanged (TextChangedEventArgs *args);
	
	// cursor blink
	static gboolean blink (void *user_data);
	void ConnectBlinkTimeout (guint multiplier);
	void DisconnectBlinkTimeout ();
	void DelayCursorBlink ();
	void BeginCursorBlink ();
	void EndCursorBlink ();
	void ShowCursor ();
	void HideCursor ();
	bool Blink ();
	
	void UpdateCursor (bool invalidate);
	
	void Layout (cairo_t *cr);
	void Paint (cairo_t *cr);
	
 protected:
	virtual ~TextBoxView ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TextBoxView ();
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, Region *region);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	
	//
	// Property Accessors
	//
	TextBox *GetTextBox () { return textbox; }
	void SetTextBox (TextBox *textbox);
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
	PasswordBox ();
	
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
