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

#include <gtk/gtk.h>
#include <cairo.h>

#include "fontsource.h"
#include "moon-path.h"
#include "eventargs.h"
#include "thickness.h"
#include "control.h"
#include "layout.h"
#include "brush.h"
#include "fonts.h"
#include "size.h"


/* @Namespace=System.Windows.Input */
class InputMethod : public DependencyObject {
 protected:
	virtual ~InputMethod () {}
	
 public:
	/* @PropertyType=bool,Attached,DefaultValue=true,Validator=IsInputMethodEnabledValidator */
	const static int IsInputMethodEnabledProperty;
	
 	/* @ManagedAccess=Internal,GeneratePInvoke,GenerateCBinding */
	InputMethod () { SetObjectType (Type::INPUTMETHOD); }
};

/* @Namespace=None */
class TextChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~TextChangedEventArgs () { }
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TextChangedEventArgs () { SetObjectType (Type::TEXTCHANGEDEVENTARGS); }
};


/* @Namespace=None */
class CursorPositionChangedEventArgs : public EventArgs {
	double height, x, y;
	
 protected:
	virtual ~CursorPositionChangedEventArgs () { }
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	CursorPositionChangedEventArgs ()
	{
		SetObjectType (Type::CURSORPOSITIONCHANGEDEVENTARGS);
		this->height = 0.0;
		this->x = 0.0;
		this->y = 0.0;
	}
	
	CursorPositionChangedEventArgs (double height, double x, double y)
	{
		SetObjectType (Type::CURSORPOSITIONCHANGEDEVENTARGS);
		this->height = height;
		this->x = x;
		this->y = y;
	}
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetCursorHeight () { return height; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetCursorX () { return x; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetCursorY () { return y; }
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


/* @Namespace=None */
class TextBoxModelChangedEventArgs : public EventArgs {
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

/* @Namespace=None,ManagedEvents=Manual */
class MOON_API TextBoxBase : public Control, public ITextAttributes {
 protected:
	friend class TextBoxView;
	
	DependencyObject *contentElement;
	
	TextFontDescription *font;
	GPtrArray *downloaders;
	char *font_source;
	
	TextBoxUndoStack *undo;
	TextBoxUndoStack *redo;
	int selection_anchor;
	int selection_cursor;
	double cursor_offset;
	GtkIMContext *im_ctx;
	TextBuffer *buffer;
	TextBoxView *view;
	int max_length;
	
	short accepts_return:1;
	short need_im_reset:1;
	short is_read_only:1;
	short have_offset:1;
	short multiline:1;
	short selecting:1;
	short setvalue:1;
	short captured:1;
	short focused:1;
	short secret:1;
	
	short events_mask:2;
	short emit:2;
	
	short batch;
	
	// internal mouse events
	static void mouse_left_button_multi_click (EventObject *sender, EventArgs *args, gpointer closure);
	void OnMouseLeftButtonMultiClick (MouseButtonEventArgs *args);
	
	// GtkIMContext events
	static gboolean delete_surrounding (GtkIMContext *context, int offset, int n_chars, gpointer user_data);
	static gboolean retrieve_surrounding (GtkIMContext *context, gpointer user_data);
	static void commit (GtkIMContext *context, const char *str, gpointer user_data);
	bool DeleteSurrounding (int offset, int n_chars);
	void Commit (const char *str);
	bool RetrieveSurrounding ();
	
	// GtkClipboard callbacks
	static void paste (GtkClipboard *clipboard, const char *text, gpointer closure);
	void Paste (GtkClipboard *clipboard, const char *text);
	
	//
	// Cursor Navigation
	//
	double GetCursorOffset ();
	virtual int CursorDown (int cursor, bool page);
	virtual int CursorUp (int cursor, bool page);
	virtual int CursorLineBegin (int cursor);
	virtual int CursorLineEnd (int cursor, bool include = false);
	virtual int CursorNextWord (int cursor);
	virtual int CursorPrevWord (int cursor);
	
	//
	// Keyboard Input
	//
	bool KeyPressUnichar (gunichar c);
	
	bool KeyPressBackSpace (GdkModifierType modifiers);
	bool KeyPressDelete (GdkModifierType modifiers);
	bool KeyPressPageDown (GdkModifierType modifiers);
	bool KeyPressPageUp (GdkModifierType modifiers);
	bool KeyPressHome (GdkModifierType modifiers);
	bool KeyPressEnd (GdkModifierType modifiers);
	bool KeyPressRight (GdkModifierType modifiers);
	bool KeyPressLeft (GdkModifierType modifiers);
	bool KeyPressDown (GdkModifierType modifiers);
	bool KeyPressUp (GdkModifierType modifiers);
	
	void ResetIMContext ();
	
	void EmitCursorPositionChanged (double height, double x, double y);
	
	virtual void EmitSelectionChanged () { }
	virtual void EmitTextChanged () = 0;
	
	virtual void SyncSelectedText () = 0;
	virtual void SyncText () = 0;
	
	void BatchPush ();
	void BatchPop ();
	
	void SyncAndEmit (bool sync_text = true);
	
	void AddFontResource (const char *resource);
	void AddFontSource (Downloader *downloader);
	
	void CleanupDownloaders ();
	void DownloaderComplete (Downloader *downloader);
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	//
	// Protected Property Accessors
	//
	bool HasSelectedText () { return selection_cursor != selection_anchor; }
	TextBuffer *GetBuffer () { return buffer; }
	int GetCursor () { return selection_cursor; }
	bool IsFocused () { return focused; }
	
	virtual const char *GetActualText () = 0;
	
	virtual void SetSelectedText (const char *text) = 0;
	virtual const char *GetSelectedText () = 0;
	
	virtual void SetSelectionStart (int start) = 0;
	virtual int GetSelectionStart () = 0;
	
	virtual void SetSelectionLength (int length) = 0;
	virtual int GetSelectionLength () = 0;
	
	void ClearSelection (int start);
	
	//
	// Protected Events
	//
	const static int ModelChangedEvent;
	
	//
	// Initialization/Destruction
	//
	void Initialize (Type::Kind type, const char *type_name);
	virtual ~TextBoxBase ();
	
 public:
	TextBoxBase () { }
	
	//
	// Overrides
	//
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void SetSurface (Surface *surface);
	virtual void OnApplyTemplate ();
	
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
	
	/* @GenerateCBinding,GeneratePInvoke */
	void OnMouseLeftButtonDown (MouseButtonEventArgs *args);
	/* @GenerateCBinding,GeneratePInvoke */
	void OnMouseLeftButtonUp (MouseButtonEventArgs *args);
	/* @GenerateCBinding,GeneratePInvoke */
	void OnMouseMove (MouseEventArgs *args);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void PostOnKeyDown (KeyEventArgs *args);
	/* @GenerateCBinding,GeneratePInvoke */
	void OnKeyDown (KeyEventArgs *args);
	/* @GenerateCBinding,GeneratePInvoke */
	void OnKeyUp (KeyEventArgs *args);

	/* @GenerateCBinding,GeneratePInvoke */
	void OnGotFocus (RoutedEventArgs *args);
	/* @GenerateCBinding,GeneratePInvoke */
	void OnLostFocus (RoutedEventArgs *args);
	
	//
	// Undo/Redo Operations
	//
	bool CanUndo ();
	bool CanRedo ();
	void Undo ();
	void Redo ();
	
	//
	// Selection Operations
	//
	/* @GenerateCBinding,GeneratePInvoke */
	bool SelectWithError (int start, int length, MoonError *error);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SelectAll ();
	
	virtual Brush *GetSelectionBackground () = 0;
	virtual Brush *GetSelectionForeground () = 0;
	virtual Brush *GetCaretBrush () = 0;
	
	virtual TextAlignment GetTextAlignment () { return TextAlignmentLeft; }
	virtual TextWrapping GetTextWrapping () { return TextWrappingNoWrap; }
	
	virtual const char *GetDisplayText () = 0;
	
	//
	// Events
	//
	const static int CursorPositionChangedEvent;
};


class TextBoxDynamicPropertyValueProvider;

/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class TextBox : public TextBoxBase {
	friend class TextBoxDynamicPropertyValueProvider;
	
 protected:
	virtual const char *GetActualText () { return GetText (); }
	
	virtual void EmitSelectionChanged ();
	virtual void EmitTextChanged ();
	
	virtual void SyncSelectedText ();
	virtual void SyncText ();
	
	virtual void SetSelectionStart (int start);
	virtual void SetSelectionLength (int length);
	
	virtual ~TextBox () { }
	
 public:
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	const static int AcceptsReturnProperty;
	/* @PropertyType=Brush,DefaultValue=new SolidColorBrush("black"),Version=2.0,GenerateAccessors */
	const static int CaretBrushProperty;
	/* @PropertyType=FontSource,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int FontSourceProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=bool,DefaultValue=false,Version=2.0,GenerateAccessors */
	const static int IsReadOnlyProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors,Validator=PositiveIntValidator */
	const static int MaxLengthProperty;
	/* @PropertyType=string,DefaultValue=\"\",AlwaysChange,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int SelectedTextProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	const static int SelectionBackgroundProperty;
	/* @PropertyType=Brush,Version=2.0,GenerateAccessors */
	const static int SelectionForegroundProperty;
	/* @PropertyType=gint32,DefaultValue=0,AlwaysChange,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionLengthProperty;
	/* @PropertyType=gint32,DefaultValue=0,AlwaysChange,Version=2.0,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionStartProperty;
	/* @PropertyType=string,Version=2.0,GenerateAccessors,GenerateManagedAccessors=false */
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
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnApplyTemplate ();
	
	virtual const char *GetDisplayText () { return GetText (); }
	
	//
	// Property Accessors
	//
	void SetAcceptsReturn (bool accept);
	bool GetAcceptsReturn ();
	
	void SetCaretBrush (Brush *caret);
	virtual Brush *GetCaretBrush ();
	
	void SetFontSource (FontSource *source);
	FontSource *GetFontSource ();
	
	void SetHorizontalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetHorizontalScrollBarVisibility ();
	
	void SetIsReadOnly (bool readOnly);
	bool GetIsReadOnly ();
	
	void SetMaxLength (int length);
	int GetMaxLength ();
	
	void SetSelectionBackground (Brush *background);
	virtual Brush *GetSelectionBackground ();
	
	void SetSelectionForeground (Brush *foreground);
	virtual Brush *GetSelectionForeground ();
	
	virtual void SetSelectedText (const char *text);
	virtual const char *GetSelectedText ();
	
	virtual int GetSelectionStart ();
	virtual int GetSelectionLength ();
	
	void SetText (const char *text);
	const char *GetText ();
	
	void SetTextAlignment (TextAlignment alignment);
	virtual TextAlignment GetTextAlignment ();
	
	void SetTextWrapping (TextWrapping wrapping);
	virtual TextWrapping GetTextWrapping ();
	
	void SetVerticalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetVerticalScrollBarVisibility ();
	
	//
	// Events
	//
	/* @DelegateType=RoutedEventHandler */
	const static int SelectionChangedEvent;
	/* @DelegateType=TextChangedEventHandler */
	const static int TextChangedEvent;
};


class PasswordBoxDynamicPropertyValueProvider;

/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class PasswordBox : public TextBoxBase {
	friend class PasswordBoxDynamicPropertyValueProvider;
	
	GString *display;
	
 protected:
	virtual int CursorDown (int cursor, bool page);
	virtual int CursorUp (int cursor, bool page);
	virtual int CursorLineBegin (int cursor);
	virtual int CursorLineEnd (int cursor, bool include = false);
	virtual int CursorNextWord (int cursor);
	virtual int CursorPrevWord (int cursor);
	
	virtual void EmitTextChanged ();
	
	virtual void SyncSelectedText ();
	virtual void SyncText ();
	void SyncDisplayText ();
	
	//
	// Protected Property Accessors
	//
	virtual const char *GetActualText () { return GetPassword (); }
	
	virtual void SetSelectedText (const char *text);
	virtual const char *GetSelectedText ();
	
	virtual void SetSelectionStart (int start);
	virtual int GetSelectionStart ();
	
	virtual void SetSelectionLength (int length);
	virtual int GetSelectionLength ();
	
	virtual ~PasswordBox ();
	
 public:
	/* @PropertyType=Brush,DefaultValue=new SolidColorBrush("black"),Version=2.0,GenerateAccessors */
	const static int CaretBrushProperty;
	/* @PropertyType=FontSource,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int FontSourceProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors,Validator=PositiveIntValidator */
	const static int MaxLengthProperty;
	/* @PropertyType=char,DefaultValue=(gunichar) 9679\, Type::CHAR,Version=2.0,GenerateAccessors */
	const static int PasswordCharProperty;
	/* @PropertyType=string,DefaultValue=\"\",AlwaysChange,Version=2.0,GenerateAccessors,Validator=NonNullValidator */
	const static int PasswordProperty;
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
	
	/* @GenerateCBinding,GeneratePInvoke */
	PasswordBox ();
	
	//
	// Overrides
	//
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	virtual const char *GetDisplayText ();
	
	//
	// Property Accesors
	//
	void SetCaretBrush (Brush *caret);
	virtual Brush *GetCaretBrush ();
	
	void SetFontSource (FontSource *source);
	FontSource *GetFontSource ();
	
	void SetMaxLength (int length);
	int GetMaxLength ();
	
	void SetPassword (const char *password);
	const char *GetPassword ();
	
	void SetPasswordChar (gunichar c);
	gunichar GetPasswordChar ();
	
	void SetSelectionBackground (Brush *background);
	virtual Brush *GetSelectionBackground ();
	
	void SetSelectionForeground (Brush *foreground);
	virtual Brush *GetSelectionForeground ();
	
	//
	// Events
	//
	/* @DelegateType=RoutedEventHandler */
	const static int PasswordChangedEvent;
};


/* @Namespace=Microsoft.Internal */
class TextBoxView : public FrameworkElement {
	TextBoxBase *textbox;
	glong blink_timeout;
	TextLayout *layout;
	Rect cursor;
	
	int selection_changed:1;
	int had_selected_text:1;
	int cursor_visible:1;
	int enable_cursor:1;
	int dirty:1;
	
	// mouse events
	static void mouse_left_button_down (EventObject *sender, EventArgs *args, gpointer closure);
	static void mouse_left_button_up (EventObject *sender, EventArgs *args, gpointer closure);
	void OnMouseLeftButtonDown (MouseButtonEventArgs *args);
	void OnMouseLeftButtonUp (MouseButtonEventArgs *args);
	
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
	void InvalidateCursor ();
	void UpdateText ();
	
	void Layout (Size constraint);
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
	virtual Size ComputeActualSize ();
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
	
	void OnLostFocus ();
	void OnGotFocus ();
	
	//
	// Property Accessors
	//
	TextBoxBase *GetTextBox () { return textbox; }
	void SetTextBox (TextBoxBase *textbox);
	
	bool GetEnableCursor () { return enable_cursor ? true : false; }
	void SetEnableCursor (bool enable);
};

#endif /* __TEXTBOX_H__ */
