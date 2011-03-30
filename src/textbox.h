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

#include <cairo.h>

#include "inputmethod.h"
#include "fontsource.h"
#include "moon-path.h"
#include "eventargs.h"
#include "thickness.h"
#include "control.h"
#include "textlayout.h"
#include "brush.h"
#include "fonts.h"
#include "size.h"

namespace Moonlight {

/* @Namespace=None */
class TextChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~TextChangedEventArgs () { }
	
 public:
	/* @GeneratePInvoke */
	TextChangedEventArgs () { SetObjectType (Type::TEXTCHANGEDEVENTARGS); }
};

/* @Namespace=None */
class CursorPositionChangedEventArgs : public EventArgs {
	double height, x, y;
	
 protected:
	virtual ~CursorPositionChangedEventArgs () { }
	
 public:
	/* @SkipFactories */
	CursorPositionChangedEventArgs ()
	{
		SetObjectType (Type::CURSORPOSITIONCHANGEDEVENTARGS);
		this->height = 0.0;
		this->x = 0.0;
		this->y = 0.0;
	}
	
	/* @SkipFactories */
	CursorPositionChangedEventArgs (double height, double x, double y)
	{
		SetObjectType (Type::CURSORPOSITIONCHANGEDEVENTARGS);
		this->height = height;
		this->x = x;
		this->y = y;
	}
	
	/* @GeneratePInvoke */
	double GetCursorHeight () { return height; }
	
	/* @GeneratePInvoke */
	double GetCursorX () { return x; }
	
	/* @GeneratePInvoke */
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
class TextBoxBaseDynamicPropertyValueProvider;

/* @Namespace=None,ManagedEvents=Manual */
class TextBoxBase : public Control, public ITextAttributes {
 protected:
	friend class TextBoxView;
	friend class TextBoxBaseDynamicPropertyValueProvider;
	
	DependencyObject *contentElement;
	
	FontResource *font_resource;
	TextFontDescription *font;
	GPtrArray *downloaders;
	
	TextBoxUndoStack *undo;
	TextBoxUndoStack *redo;
	int selection_anchor;
	int selection_cursor;
	double cursor_offset;
	MoonIMContext *im_ctx;
	TextBuffer *buffer;
	WeakRef <TextBoxView> view;
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
	
	// MoonIMContext events
	EVENTHANDLER (TextBoxBase, AttachIMClientWindow, EventObject, EventArgs);
	EVENTHANDLER (TextBoxBase, DetachIMClientWindow, EventObject, EventArgs);

	static gboolean delete_surrounding (MoonIMContext *context, int offset, int n_chars, gpointer user_data);
	static gboolean retrieve_surrounding (MoonIMContext *context, gpointer user_data);
	static void commit (MoonIMContext *context, const char *str, gpointer user_data);
	bool DeleteSurrounding (int offset, int n_chars);
	void Commit (const char *str);
	bool RetrieveSurrounding ();
	
	// Clipboard callbacks
	static void paste (MoonClipboard *clipboard, const char *text, gpointer closure);
	void Paste (MoonClipboard *clipboard, const char *text);
	
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
	
	bool KeyPressBackSpace (MoonModifier modifiers);
	bool KeyPressDelete (MoonModifier modifiers);
	bool KeyPressPageDown (MoonModifier modifiers);
	bool KeyPressPageUp (MoonModifier modifiers);
	bool KeyPressHome (MoonModifier modifiers);
	bool KeyPressEnd (MoonModifier modifiers);
	bool KeyPressRight (MoonModifier modifiers);
	bool KeyPressLeft (MoonModifier modifiers);
	bool KeyPressDown (MoonModifier modifiers);
	bool KeyPressUp (MoonModifier modifiers);
	
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
	void Initialize (Type::Kind type);
	virtual ~TextBoxBase ();
	
 public:
	/* @SkipFactories */
	TextBoxBase ()
		: view (this, ViewWeakRef) { }
	
	//
	// Overrides
	//
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnIsAttachedChanged (bool value);
	virtual void OnApplyTemplate ();
	
	//
	// ITextAttributes Interface Methods
	//
	virtual TextDecorations Decorations () { return TextDecorationsNone; }
	virtual TextFontDescription *FontDescription () { return font; }
	virtual FlowDirection Direction () { return FlowDirectionLeftToRight; }
	
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
	
	/* @GeneratePInvoke */
	void OnMouseLeftButtonDown (MouseButtonEventArgs *args);
	/* @GeneratePInvoke */
	void OnMouseLeftButtonUp (MouseButtonEventArgs *args);
	/* @GeneratePInvoke */
	void OnMouseMove (MouseEventArgs *args);
	
	/* @GeneratePInvoke */
	void PostOnKeyDown (KeyEventArgs *args);
	/* @GeneratePInvoke */
	void OnKeyDown (KeyEventArgs *args);
	/* @GeneratePInvoke */
	void OnKeyUp (KeyEventArgs *args);

	/* @GeneratePInvoke */
	void OnGotFocus (RoutedEventArgs *args);
	/* @GeneratePInvoke */
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
	/* @GeneratePInvoke */
	bool SelectWithError (int start, int length, MoonError *error);
	
	/* @GeneratePInvoke */
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

	const static void *ViewWeakRef;
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
	
	/* @GeneratePInvoke */
	TextBox ();
	
	virtual ~TextBox () { }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int AcceptsReturnProperty;
	/* @PropertyType=double,ReadOnly,GenerateAccessors,ManagedFieldAccess=Private */
	const static int BaselineOffsetProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int CaretBrushProperty;
	/* @PropertyType=FontSource,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int FontSourceProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=InputScope,GenerateAccessors,GenerateManagedAccessors=false,Validator=NullOrInDesignMode,Browsable=Never */
	const static int InputScopeProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsReadOnlyProperty;
	/* @PropertyType=gint32,DefaultValue=0,GenerateAccessors,Validator=PositiveIntValidator */
	const static int MaxLengthProperty;
	/* @PropertyType=string,DefaultValue=\"\",AlwaysChange,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int SelectedTextProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int SelectionBackgroundProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int SelectionForegroundProperty;
	/* @PropertyType=gint32,DefaultValue=0,AlwaysChange,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionLengthProperty;
	/* @PropertyType=gint32,DefaultValue=0,AlwaysChange,ManagedFieldAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionStartProperty;
	/* @PropertyType=string,GenerateAccessors,GenerateManagedAccessors=false */
	const static int TextProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,GenerateAccessors */
	const static int TextAlignmentProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,GenerateAccessors */
	const static int TextWrappingProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityHidden,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int VerticalScrollBarVisibilityProperty;
	/* @PropertyType=object,GenerateAccessors,GenerateManagedAccessors=false,Validator=NullOrInDesignMode,Browsable=Never */
	const static int WatermarkProperty;
	
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

	void SetBaselineOffset (double offset);
	double GetBaselineOffset ();
	
	void SetCaretBrush (Brush *caret);
	virtual Brush *GetCaretBrush ();
	
	void SetFontSource (FontSource *source);
	FontSource *GetFontSource ();
	
	void SetHorizontalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetHorizontalScrollBarVisibility ();
	
	void SetInputScope (InputScope *scope);
	InputScope *GetInputScope ();
	
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
	
	void SetWatermark (Value *value);
	Value *GetWatermark ();
	
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

	void SetBaselineOffset (double offset);
	
	virtual void SetSelectedText (const char *text);
	virtual const char *GetSelectedText ();
	
	virtual void SetSelectionStart (int start);
	virtual int GetSelectionStart ();
	
	virtual void SetSelectionLength (int length);
	virtual int GetSelectionLength ();
	
	/* @GeneratePInvoke */
	PasswordBox ();
	
	virtual ~PasswordBox ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=double,ReadOnly,GenerateAccessors,ManagedFieldAccess=Private */
	const static int BaselineOffsetProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int CaretBrushProperty;
	/* @PropertyType=FontSource,ManagedFieldAccess=Internal,GenerateAccessors */
	const static int FontSourceProperty;
	/* @PropertyType=gint32,DefaultValue=0,GenerateAccessors,Validator=PositiveIntValidator */
	const static int MaxLengthProperty;
	/* @PropertyType=char,DefaultValue=(gunichar) 9679\, Type::CHAR,GenerateAccessors */
	const static int PasswordCharProperty;
	/* @PropertyType=string,DefaultValue=\"\",AlwaysChange,GenerateAccessors,Validator=NonNullValidator */
	const static int PasswordProperty;
	/* @PropertyType=string,DefaultValue=\"\",ManagedAccess=Internal,GenerateAccessors */
	const static int SelectedTextProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int SelectionBackgroundProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int SelectionForegroundProperty;
	/* @PropertyType=gint32,DefaultValue=0,ManagedAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionLengthProperty;
	/* @PropertyType=gint32,DefaultValue=0,ManagedAccess=Internal,GenerateAccessors,Validator=PositiveIntValidator */
	const static int SelectionStartProperty;
	
	//
	// Overrides
	//
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	virtual const char *GetDisplayText ();
	
	//
	// Property Accesors
	//
	double GetBaselineOffset ();

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
	static bool blink (void *user_data);
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
	/* @GeneratePInvoke */
	TextBoxView ();

	virtual ~TextBoxView ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual Size ComputeActualSize ();
	virtual Size MeasureOverrideWithError (Size availableSize, MoonError *error);
	virtual Size ArrangeOverrideWithError (Size finalSize, MoonError *error);
	
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

	double GetBaselineOffset ();
	
	//
	// Property Accessors
	//
	TextBoxBase *GetTextBox () { return textbox; }
	void SetTextBox (TextBoxBase *textbox);
	
	bool GetEnableCursor () { return enable_cursor ? true : false; }
	void SetEnableCursor (bool enable);
};

};
#endif /* __TEXTBOX_H__ */
