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
#include "richtextlayout.h"
#include "textelement.h"
#include "textpointer.h"
#include "textselection.h"
#include "list.h"

namespace Moonlight {

/* @Namespace=None */
class ContentChangedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~ContentChangedEventArgs () {}
	
 public:
	/* @GeneratePInvoke */
	ContentChangedEventArgs () { SetObjectType (Type::CONTENTCHANGEDEVENTARGS); }
};

enum RichTextBoxModelChangeType {
	RichTextBoxModelChangedNothing,
	RichTextBoxModelChangedIsReadOnly,
	RichTextBoxModelChangedTextAlignment,
	RichTextBoxModelChangedTextWrapping,
	RichTextBoxModelChangedSelection,
	RichTextBoxModelChangedContent,
	RichTextBoxModelChangedBrush
};

class RichTextBoxView;

/* @Namespace=System.Windows.Controls */
/* @ContentProperty=Blocks */
class RichTextBox : public Control, public IDocumentNode, public ITextAttributes {
	friend class RichTextBoxDynamicPropertyValueProvider;
	friend class RichTextBoxView;
	friend class TextSelection;
	friend class TextPointer;

private:
	void ApplyFormattingToSelection (TextSelection *selection, DependencyProperty *prop, Value *value);

protected:
	DependencyObject *contentElement;
	
	TextSelection *selection;
	GPtrArray *text_pointers;

	Stack *undo;
	Stack *redo;
	double cursor_offset;
	RichTextBoxView *view;
	MoonIMContext *im_ctx;
	
	short accepts_return:1;
	short need_im_reset:1;
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
	TextPointer CursorDown (const TextPointer& cursor, bool page);
	TextPointer CursorUp (const TextPointer& cursor, bool page);
	TextPointer CursorLineBegin (const TextPointer& cursor);
	TextPointer CursorLineEnd (const TextPointer& cursor, bool include = false);
	TextPointer CursorNextWord (const TextPointer& cursor);
	TextPointer CursorPrevWord (const TextPointer& cursor);
	
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
	bool HasSelectedText () { return false; /* FIXME */ }
	//TextBuffer *GetBuffer () { return buffer; }
	bool IsFocused () { return focused; }
	
	//
	// Protected Events
	//
	const static int ModelChangedEvent;
	
	virtual ~RichTextBox ();

	/* @GeneratePInvoke */
	RichTextBox ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int AcceptsReturnProperty;
	/* @PropertyType=double,ReadOnly,GenerateAccessors,ManagedFieldAccess=Private */
	const static int BaselineOffsetProperty;
	/* @PropertyType=BlockCollection,AutoCreateValue,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Private */
	const static int BlocksProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int CaretBrushProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsReadOnlyProperty;
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
	
	//
	// Overrides
	//
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnIsAttachedChanged (bool value);
	virtual void OnApplyTemplate ();

	// IDocumentNode interface
	virtual DependencyObjectCollection* GetDocumentChildren ();
	virtual void AddTextPointer (TextPointer *pointer);
	virtual void RemoveTextPointer (TextPointer *pointer);

	virtual char* Serialize ();
	virtual void SerializeProperties (bool force, GString *str);

	//
	// ITextAttributes Interface Methods
	//
	virtual TextFontDescription *FontDescription () { return NULL; }
	virtual FlowDirection Direction () { return FlowDirectionLeftToRight; }
	virtual TextDecorations Decorations () { return TextDecorationsNone; }
	virtual Brush *Background (bool selected) { return NULL; }
	virtual Brush *Foreground (bool selected) { return NULL; }

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

	// TextPointer things

	/* @GeneratePInvoke */
	TextPointer* GetPositionFromPoint (Point point);

	/* @GeneratePInvoke */
	TextPointer* GetContentStart ();

	/* @GeneratePInvoke */
	TextPointer* GetContentEnd ();
	
	//
	// Selection Operations
	//
	/* @GeneratePInvoke */
	void SelectAll ();

	/* @GeneratePInvoke */
	TextSelection *GetSelection ();
	void SetSelection (TextSelection *selection);

	//
	// Property Accessors
	//
	void SetAcceptsReturn (bool accept);
	bool GetAcceptsReturn ();
	
	void SetBaselineOffset (double offset);
	double GetBaselineOffset ();

	void SetBlocks (BlockCollection *blocks);
	BlockCollection *GetBlocks ();
	
	void SetCaretBrush (Brush *caret);
	Brush *GetCaretBrush ();
	
	void SetHorizontalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetHorizontalScrollBarVisibility ();
	
	void SetIsReadOnly (bool readonly);
	bool GetIsReadOnly ();
	
	void SetTextAlignment (TextAlignment alignment);
	TextAlignment GetTextAlignment ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
	
	void SetVerticalScrollBarVisibility (ScrollBarVisibility visibility);
	ScrollBarVisibility GetVerticalScrollBarVisibility ();
	
	void SetXaml (const char *xaml);
	const char *GetXaml ();

	Rect GetCharacterRect (TextPointer *tp, LogicalDirection direction);

	RichTextBoxView *GetView () { return view; }

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
	RichTextLayout *layout;
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
	RichTextBoxView ();
	
	virtual ~RichTextBoxView ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
 	/* @PropertyType=UIElementCollection,AutoCreator=RichTextBoxView::CreateChildren,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int ChildrenProperty;

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
	RichTextLayoutLine *GetLineFromY (double y, int *index = NULL);
	RichTextLayoutLine *GetLineFromIndex (int index);
	
	TextPointer GetLocationFromXY (double x, double y);
	Rect GetCharacterRect (TextPointer *tp, LogicalDirection direction);
	Rect GetCursor () { return cursor; }
	
	void OnLostFocus ();
	void OnGotFocus ();

	void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);

	double GetBaselineOffset ();

	void OnModelChanged (RichTextBoxModelChangeType change, PropertyChangedEventArgs *args);

	// ITextLayoutContainer interface
	void DocumentPropertyChanged (TextElement *onElement, PropertyChangedEventArgs *args);
	void DocumentCollectionChanged (TextElement *onElement, Collection *col, CollectionChangedEventArgs *args);

	//
	// Property Accessors
	//
	RichTextBox *GetTextBox () { return textbox; }
	void SetTextBox (RichTextBox *textbox);
	
	bool GetEnableCursor () { return enable_cursor ? true : false; }
	void SetEnableCursor (bool enable);

	void SetChildren (UIElementCollection *children);
	UIElementCollection *GetChildren ();

	// Autocreator for the Children property
	static Value *CreateChildren (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj);
};

};
#endif /* __RICHTEXTBOX_H__ */
