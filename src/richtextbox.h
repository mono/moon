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

#include "inputmethod.h"
#include "textelement.h"
#include "control.h"
#include "layout.h"

namespace Moonlight {

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
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsAtInsertionPositionProperty;
	/* @PropertyType=LogicalDirection,DefaultValue=LogicalDirectionForward,GenerateAccessors */
	const static int LogicalDirectionProperty;
	/* @PropertyType=DependencyObject,GenerateAccessors */
	const static int ParentProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TextPointer () { SetObjectType (Type::TEXTPOINTER); }
	
	//
	// Methods
	//
	/* @GeneratePInvoke,GenerateCBinding */
	int CompareTo (TextPointer *pointer);
	/* @GeneratePInvoke,GenerateCBinding */
	Rect GetCharacterRect (LogicalDirection dir);
	/* @GeneratePInvoke,GenerateCBinding */
	TextPointer *GetNextInsertionPosition (LogicalDirection dir);
	/* @GeneratePInvoke,GenerateCBinding */
	TextPointer *GetPositionAtOffset (int offset, LogicalDirection dir);
	
	//
	// Property Accessors
	//
	void SetIsAtInsertionPosition (bool value);
	bool GetIsAtInsertionPosition ();
	
	void SetLogicalDirection (LogicalDirection dir);
	LogicalDirection GetLogicalDirection ();
	
	void SetParent (DependencyObject *parent);
	DependencyObject *GetParent ();
};

/* @Namespace=None */
class TextSelection : public DependencyObject {
 protected:
	virtual ~TextSelection () {}
	
 public:
	/* @PropertyType=TextPointer,GenerateAccessors */
	const static int EndProperty;
	/* @PropertyType=TextPointer,GenerateAccessors */
	const static int StartProperty;
	/* @PropertyType=string,GenerateAccessors */
	const static int TextProperty;
	/* @PropertyType=string,GenerateAccessors */
	const static int XamlProperty;
	
	/* @GeneratePInvoke,GenerateCBinding */
	TextSelection ()
	{
		SetObjectType (Type::TEXTSELECTION);
		// FIXME: we should not be doing this... should they be autocreated?
		SetEnd (new TextPointer()); SetStart (new TextPointer());
	}
	
	//
	// Methods
	//
	/* @GeneratePInvoke,GenerateCBinding */
	void ApplyPropertyValue (DependencyProperty *formatting, Value *value);
	/* @GeneratePInvoke,GenerateCBinding */
	Value *GetPropertyValue (DependencyProperty *formatting);
	/* @GeneratePInvoke,GenerateCBinding */
	void Insert (TextElement *element);
	/* @GeneratePInvoke,GenerateCBinding */
	bool SelectWithError (TextPointer *anchor, TextPointer *cursor, MoonError *error);
	bool Select (TextPointer *anchor, TextPointer *cursor);
	
	//
	// Property Accessors
	//
	void SetStart (TextPointer *start);
	TextPointer *GetStart ();
	
	void SetEnd (TextPointer *end);
	TextPointer *GetEnd ();
	
	void SetText (const char *text);
	const char *GetText ();
	
	void SetXaml (const char *xaml);
	const char *GetXaml ();
};


enum RichTextBoxModelChangeType {
	RichTextBoxModelChangedNothing,
	RichTextBoxModelChangedBaselineOffset,
	RichTextBoxModelChangedTextAlignment,
	RichTextBoxModelChangedTextWrapping,
	RichTextBoxModelChangedSelection,
	RichTextBoxModelChangedContent,
	RichTextBoxModelChangedBrush
};

/* @Namespace=None */
class RichTextBoxModelChangedEventArgs : public EventArgs {
 protected:
	virtual ~RichTextBoxModelChangedEventArgs () { }
	
 public:
	PropertyChangedEventArgs *property;
	RichTextBoxModelChangeType changed;
	
	RichTextBoxModelChangedEventArgs (RichTextBoxModelChangeType changed, PropertyChangedEventArgs *property = NULL)
	{
		SetObjectType (Type::RICHTEXTBOXMODELCHANGEDEVENTARGS);
		this->property = property;
		this->changed = changed;
	}
};

class RichTextBoxUndoStack;
class RichTextBoxView;

/* @Namespace=System.Windows.Controls */
/* @ContentProperty=Blocks */
class RichTextBox : public Control {
	friend class RichTextBoxDynamicPropertyValueProvider;
	friend class RichTextBoxView;
	friend class TextSelection;
	friend class TextPointer;

 protected:
	DependencyObject *contentElement;
	
	Section *rootSection;

	RichTextBoxUndoStack *undo;
	RichTextBoxUndoStack *redo;
	int selection_anchor;
	int selection_cursor;
	double cursor_offset;
	RichTextBoxView *view;
	MoonIMContext *im_ctx;
	
	short accepts_return:1;
	short need_im_reset:1;
	short is_read_only:1;
	short have_offset:1;
	short selecting:1;
	short setvalue:1;
	short captured:1;
	short focused:1;
	
	short events_mask:2;
	short emit:2;
	
	short batch;
	
	// internal mouse events
	static void mouse_left_button_multi_click (EventObject *sender, EventArgs *args, gpointer closure);
	void OnMouseLeftButtonMultiClick (MouseButtonEventArgs *args);
	
	// MoonIMContext events
	static void attach_im_client_window (EventObject *sender, EventArgs *args, gpointer closure);
	static void detach_im_client_window (EventObject *sender, EventArgs *args, gpointer closure);
	void AttachIMClientWindow (EventObject *sender, EventArgs *calldata);
	void DetachIMClientWindow (EventObject *sender, EventArgs *calldata);
	
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
	int CursorDown (int cursor, bool page);
	int CursorUp (int cursor, bool page);
	int CursorLineBegin (int cursor);
	int CursorLineEnd (int cursor, bool include = false);
	int CursorNextWord (int cursor);
	int CursorPrevWord (int cursor);
	
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
	
	void EmitSelectionChanged ();
	void EmitContentChanged ();
	
	void SyncSelection ();
	void SyncContent ();
	
	void BatchPush ();
	void BatchPop ();
	
	void SyncAndEmit (bool sync_text = true);
	
	//
	// Protected Property Accessors
	//
	bool HasSelectedText () { return selection_cursor != selection_anchor; }
	//TextBuffer *GetBuffer () { return buffer; }
	int GetCursor () { return selection_cursor; }
	bool IsFocused () { return focused; }
	
	void SetBaselineOffset (double offset);
	void SetSelection (TextSelection *selection);
	
	//
	// Protected Events
	//
	const static int ModelChangedEvent;
	
	virtual ~RichTextBox ();
	
 public:
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int AcceptsReturnProperty;
	/* @PropertyType=double,DefaultValue=NAN,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int BaselineOffsetProperty;
	/* @PropertyType=BlockCollection,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int BlocksProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int CaretBrushProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsReadOnlyProperty;
	/* @PropertyType=TextSelection,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int SelectionProperty;
	/* @PropertyType=TextAlignment,DefaultValue=TextAlignmentLeft,GenerateAccessors */
	const static int TextAlignmentProperty;
	/* @PropertyType=TextWrapping,DefaultValue=TextWrappingNoWrap,GenerateAccessors */
	const static int TextWrappingProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityAuto,GenerateAccessors,ManagedFieldAccess=Private */
	const static int HorizontalScrollBarVisibilityProperty;
	/* @PropertyType=ScrollBarVisibility,DefaultValue=ScrollBarVisibilityAuto,GenerateAccessors,ManagedFieldAccess=Private */
	const static int VerticalScrollBarVisibilityProperty;
	/* @PropertyType=string,GenerateAccessors */
	const static int XamlProperty;
	
	/* @GeneratePInvoke,GenerateCBinding */
	RichTextBox ();
	
	//
	// Overrides
	//
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnIsAttachedChanged (bool value);
	virtual void OnApplyTemplate ();
	
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

	// TextPointer things

	/* @GenerateCBinding,GeneratePInvoke */
	TextPointer* GetPositionFromPoint (Point point);

	/* @GenerateCBinding,GeneratePInvoke */
	TextPointer* GetContentStart ();

	/* @GenerateCBinding,GeneratePInvoke */
	TextPointer* GetContentEnd ();
	
	//
	// Selection Operations
	//
	/* @GenerateCBinding,GeneratePInvoke */
	void SelectAll ();
	
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
	
	void SetHorizontalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetHorizontalScrollBarVisibility ();
	
	void SetIsReadOnly (bool readonly);
	bool GetIsReadOnly ();
	
	TextSelection *GetSelection ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
	
	void SetVerticalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetVerticalScrollBarVisibility ();
	
	void SetXaml (const char *xaml);
	const char *GetXaml ();
	
	//
	// Events
	//
	/* @DelegateType=ContentChangedEventHandler */
	const static int ContentChangedEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int SelectionChangedEvent;
	const static int CursorPositionChangedEvent;
};


/* @Namespace=Microsoft.Internal */
class RichTextBoxView : public FrameworkElement {
	RichTextBox *textbox;
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
	
	// RichTextBox events
	static void model_changed (EventObject *sender, EventArgs *args, gpointer closure);
	void OnModelChanged (RichTextBoxModelChangedEventArgs *args);
	
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
	virtual ~RichTextBoxView ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	RichTextBoxView ();
	
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
	
	//
	// Property Accessors
	//
	RichTextBox *GetTextBox () { return textbox; }
	void SetTextBox (RichTextBox *textbox);
	
	bool GetEnableCursor () { return enable_cursor ? true : false; }
	void SetEnableCursor (bool enable);
};

};
#endif /* __RICHTEXTBOX_H__ */
