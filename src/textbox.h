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
/* @IncludeInKinds */
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
	TextBoxModelChangedBrush,
	TextBoxModelChangedFont
};


/* @SilverlightVersion="2" */
/* @Namespace=None */
/* @IncludeInKinds */
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

class TextBoxDynamicPropertyValueProvider;

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
/* @IncludeInKinds */
class TextBox : public Control, public ITextSource {
	friend class TextBoxDynamicPropertyValueProvider;
	
	TextFontDescription *font;
	TextSelection selection;
	TextBuffer *buffer;
	int maxlen;
	int cursor;
	
	int selection_changed:1;
	int text_changed:1;
	int emit:1;
	
	static void key_down (EventObject *sender, EventArgs *args, void *closure);
	static void key_up (EventObject *sender, EventArgs *args, void *closure);
	
	void OnKeyDown (KeyEventArgs *args);
	void OnKeyUp (KeyEventArgs *args);
	
	int CursorBackSpace (GdkModifierType modifiers);
	int CursorDelete (GdkModifierType modifiers);
	int CursorPageDown (GdkModifierType modifiers);
	int CursorPageUp (GdkModifierType modifiers);
	int CursorHome (GdkModifierType modifiers);
	int CursorEnd (GdkModifierType modifiers);
	int CursorRight (GdkModifierType modifiers);
	int CursorLeft (GdkModifierType modifiers);
	int CursorDown (GdkModifierType modifiers);
	int CursorUp (GdkModifierType modifiers);
	
	void ClearSelection ();
	
	void EmitSelectionChanged ();
	void EmitTextChanged ();
	
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
	void SelectAll ();
	/* @GenerateCBinding,GeneratePInvoke */
	void Select (int start, int length);
	
	// Methods needed by TextBoxView
	TextSelection *GetSelection () { return &selection; }
	TextBuffer *GetBuffer () { return buffer; }
	int GetCursor () { return cursor; }
	
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
	
	// Internal Events
	const static int ModelChangedEvent;
};


/* @SilverlightVersion="2" */
/* @Namespace=Microsoft.Internal */
/* @IncludeInKinds */
class TextBoxView : public FrameworkElement {
	glong blink_timeout;
	TextLayout *layout;
	TextBox *textbox;
	Rect cursor;
	
	int cursor_visible:1;
	int focused:1;
	int dirty:1;
	
	static void focus_out (EventObject *sender, EventArgs *args, gpointer closure);
	static void focus_in (EventObject *sender, EventArgs *args, gpointer closure);
	void OnFocusOut (EventArgs *args);
	void OnFocusIn (EventArgs *args);
	
	static void selection_changed (EventObject *sender, EventArgs *args, gpointer closure);
	static void model_changed (EventObject *sender, EventArgs *args, gpointer closure);
	static void text_changed (EventObject *sender, EventArgs *args, gpointer closure);
	
	void OnModelChanged (TextBoxModelChangedEventArgs *args);
	void OnSelectionChanged (RoutedEventArgs *args);
	void OnTextChanged (TextChangedEventArgs *args);
	
	static gboolean blink (void *user_data);
	void ConnectBlinkTimeout (guint multiplier);
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
	virtual Size ArrangeOverride (Size size);
	
	//
	// Property Accessors
	//
	TextBox *GetTextBox () { return textbox; }
	void SetTextBox (TextBox *textbox);
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
/* @IncludeInKinds */
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
