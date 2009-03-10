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
	TextBoxModelChangedTextAlignment,
	TextBoxModelChangedTextWrapping,
	TextBoxModelChangedSelection,
	TextBoxModelChangedBrush,
	TextBoxModelChangedFont,
	TextBoxModelChangedText
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
class TextBoxUndoStack;

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class TextBox : public Control, public ITextAttributes {
	friend class TextBoxView;
	
	DependencyObject *contentElement;
	
	TextFontDescription *font;
	TextBoxUndoStack *undo;
	TextBoxUndoStack *redo;
	int selection_anchor;
	int selection_cursor;
	double cursor_offset;
	TextBuffer *buffer;
	TextBoxView *view;
	
	int have_offset:1;
	int inkeypress:1;
	int selecting:1;
	int setvalue:1;
	int captured:1;
	int focused:1;
	int emit:2;
	
	// focus in/out events
	static void focus_out (EventObject *sender, EventArgs *args, gpointer closure);
	static void focus_in (EventObject *sender, EventArgs *args, gpointer closure);
	void OnFocusOut (EventArgs *args);
	void OnFocusIn (EventArgs *args);
	
	// mouse events
	static void mouse_left_button_down (EventObject *sender, EventArgs *args, gpointer closure);
	static void mouse_left_button_up (EventObject *sender, EventArgs *args, gpointer closure);
	static void mouse_enter (EventObject *sender, EventArgs *args, gpointer closure);
	static void mouse_leave (EventObject *sender, EventArgs *args, gpointer closure);
	static void mouse_move (EventObject *sender, EventArgs *args, gpointer closure);
	void OnMouseLeftButtonDown (MouseEventArgs *args);
	void OnMouseLeftButtonUp (MouseEventArgs *args);
	void OnMouseEnter (MouseEventArgs *args);
	void OnMouseLeave (EventArgs *args);
	void OnMouseMove (MouseEventArgs *args);
	
	// keypress events
	static void key_down (EventObject *sender, EventArgs *args, void *closure);
	static void key_up (EventObject *sender, EventArgs *args, void *closure);
	void OnKeyDown (KeyEventArgs *args);
	void OnKeyUp (KeyEventArgs *args);
	
	static void paste (GtkClipboard *clipboard, const char *text, gpointer closure);
	void Paste (GtkClipboard *clipboard, const char *text);
	
 protected:
	void Initialize (Type::Kind type, const char *type_name);
	
	virtual int CursorDown (int cursor, bool page);
	virtual int CursorUp (int cursor, bool page);
	virtual int CursorLineBegin (int cursor);
	virtual int CursorLineEnd (int cursor, bool include = false);
	virtual int CursorNextWord (int cursor);
	virtual int CursorPrevWord (int cursor);
	
	void KeyPressUnichar (gunichar c);
	
	void KeyPressBackSpace (GdkModifierType modifiers);
	void KeyPressDelete (GdkModifierType modifiers);
	void KeyPressPageDown (GdkModifierType modifiers);
	void KeyPressPageUp (GdkModifierType modifiers);
	void KeyPressHome (GdkModifierType modifiers);
	void KeyPressEnd (GdkModifierType modifiers);
	void KeyPressRight (GdkModifierType modifiers);
	void KeyPressLeft (GdkModifierType modifiers);
	void KeyPressDown (GdkModifierType modifiers);
	void KeyPressUp (GdkModifierType modifiers);
	
	double GetCursorOffset ();
	
	void SyncSelectedText ();
	void SyncText ();
	
	void ClearSelection (int start);
	
	bool CanUndo ();
	bool CanRedo ();
	void Undo ();
	void Redo ();
	
	void EmitSelectionChanged ();
	void EmitTextChanged ();
	
	void SyncAndEmit ();
	
	//
	// Protected Property Accessors
	//
	bool HasSelectedText () { return selection_cursor != selection_anchor; }
	TextBuffer *GetBuffer () { return buffer; }
	int GetCursor () { return selection_cursor; }
	bool IsFocused () { return focused; }
	
	void SetSelectionStart (int start);
	void SetSelectionLength (int length);
	
	//
	// Protected Events
	//
	const static int ModelChangedEvent;
	
	virtual ~TextBox ();
	
 public:
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	const static int AcceptsReturnProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	const static int IsReadOnlyProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors,Validator=PositiveIntValidator */
	const static int MaxLengthProperty;
	/* @PropertyType=string,DefaultValue=\"\",Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int SelectedTextProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	const static int SelectionBackgroundProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	const static int SelectionForegroundProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionLengthProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionStartProperty;
	/* @PropertyType=string,Version=2.0,GenerateAccessors */
	const static int TextProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,Version=2.0,GenerateAccessors */
	const static int TextAlignmentProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,Version=2.0,GenerateAccessors */
	const static int TextWrappingProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int VerticalScrollBarVisibilityProperty;
	
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
	void Select (int start, int length);
	/* @GenerateCBinding,GeneratePInvoke */
	void SelectAll ();
	
	//
	// ITextAttributes Interface Methods
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
	
	int selection_changed:1;
	int had_selected_text:1;
	int cursor_visible:1;
	int dirty:1;
	
	// TextBox events
	static void model_changed (EventObject *sender, EventArgs *args, gpointer closure);
	void OnModelChanged (TextBoxModelChangedEventArgs *args);
	
	// cursor blink
	static gboolean blink (void *user_data);
	void ConnectBlinkTimeout (guint multiplier);
	void DisconnectBlinkTimeout ();
	void ResetCursorBlink (bool delay);
	void DelayCursorBlink ();
	void BeginCursorBlink ();
	void EndCursorBlink ();
	void ShowCursor ();
	void HideCursor ();
	bool Blink ();
	
	void UpdateCursor (bool invalidate);
	
	void Layout (cairo_t *cr, Size constraint);
	void Paint (cairo_t *cr);
	
 protected:
	virtual ~TextBoxView ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TextBoxView ();
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);
	
	//
	// Methods
	//
	int GetLineCount () { return layout->GetLineCount (); }
	TextLayoutLine *GetLineFromY (double y, int *index = NULL);
	TextLayoutLine *GetLineFromIndex (int index);
	
	int GetCursorFromXY (double x, double y);
	Rect GetCursor () { return cursor; }
	
	void OnFocusOut ();
	void OnFocusIn ();
	
	//
	// Property Accessors
	//
	TextBox *GetTextBox () { return textbox; }
	void SetTextBox (TextBox *textbox);
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class PasswordBox : public TextBox {
 protected:
	virtual int CursorDown (int cursor, bool page);
	virtual int CursorUp (int cursor, bool page);
	virtual int CursorLineBegin (int cursor);
	virtual int CursorLineEnd (int cursor, bool include = false);
	virtual int CursorNextWord (int cursor);
	virtual int CursorPrevWord (int cursor);
	
 public:
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	const static int MaxLengthProperty;
	/* @PropertyType=string,DefaultValue=\"\",Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PasswordValidator */
	const static int PasswordProperty;
	/* @PropertyType=char,DefaultValue=9679,Version=2.0,GenerateAccessors */
	const static int PasswordCharProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	const static int SelectionBackgroundProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	const static int SelectionForegroundProperty;
	
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
