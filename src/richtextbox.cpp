/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * richtextbox.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <cairo.h>

#include "managedtypeinfo.h"
#include "contentcontrol.h"
#include "richtextbox.h"
#include "timemanager.h"
#include "deployment.h"
#include "runtime.h"
#include "border.h"
#include "window.h"
#include "panel.h"


//
// RichTextBoxUndoActions
//

enum RichTextBoxUndoActionType {
	RichTextBoxUndoActionTypeInsert,
	RichTextBoxUndoActionTypeDelete,
	RichTextBoxUndoActionTypeReplace,
};

class RichTextBoxUndoAction : public List::Node {
 public:
	RichTextBoxUndoActionType type;
	// FIXME: what else do we need?
};

class RichTextBoxUndoStack {
	int max_count;
	List *list;
	
 public:
	RichTextBoxUndoStack (int max_count);
	~RichTextBoxUndoStack ();
	
	bool IsEmpty ();
	void Clear ();
	
	void Push (RichTextBoxUndoAction *action);
	RichTextBoxUndoAction *Peek ();
	RichTextBoxUndoAction *Pop ();
};


//
// RichTextBoxUndoStack
//

RichTextBoxUndoStack::RichTextBoxUndoStack (int max_count)
{
	this->max_count = max_count;
	this->list = new List ();
}

RichTextBoxUndoStack::~RichTextBoxUndoStack ()
{
	delete list;
}

bool
RichTextBoxUndoStack::IsEmpty ()
{
	return list->IsEmpty ();
}

void
RichTextBoxUndoStack::Clear ()
{
	list->Clear (true);
}

void
RichTextBoxUndoStack::Push (RichTextBoxUndoAction *action)
{
	if (list->Length () == max_count) {
		List::Node *node = list->Last ();
		list->Unlink (node);
		delete node;
	}
	
	list->Prepend (action);
}

RichTextBoxUndoAction *
RichTextBoxUndoStack::Pop ()
{
	List::Node *node = list->First ();
	
	if (node)
		list->Unlink (node);
	
	return (RichTextBoxUndoAction *) node;
}

RichTextBoxUndoAction *
RichTextBoxUndoStack::Peek ()
{
	return (RichTextBoxUndoAction *) list->First ();
}


//
// TextPointer
//

int
TextPointer::CompareTo (TextPointer *pointer)
{
	// FIXME: implement this
	return 0;
}

Rect
TextPointer::GetCharacterRect (LogicalDirection dir)
{
	// FIXME: implement this
	return Rect ();
}

TextPointer *
TextPointer::GetNextInsertionPoint (LogicalDirection dir)
{
	// FIXME: implement this
	return NULL;
}

TextPointer *
TextPointer::GetPositionAtOffset (int offset, LogicalDirection dir)
{
	// FIXME: implement this
	return NULL;
}


//
// TextSelection
//

void
TextSelection::ApplyPropertyValue (DependencyProperty *formatting, Value *value)
{
	// FIXME: implement this
}

Value *
TextSelection::GetPropertyValue (DependencyProperty *formatting)
{
	// FIXME: implement this
	return NULL;
}

void
TextSelection::Insert (TextElement *element)
{
	// FIXME: implement this
}

bool
TextSelection::SelectWithError (TextPointer *anchor, TextPointer *cursor, MoonError *error)
{
	// FIXME: implement this
	return false;
}

bool
TextSelection::Select (TextPointer *anchor, TextPointer *cursor)
{
	MoonError err;
	
	return SelectWithError (anchor, cursor, &err);
}


//
// RichTextBox
//

// emit state, also doubles as available event mask
#define NOTHING_CHANGED         (0)
#define SELECTION_CHANGED       (1 << 0)
#define CONTENT_CHANGED         (1 << 1)

#define CONTROL_MASK MoonModifier_Control
#define SHIFT_MASK   MoonModifier_Shift
#define ALT_MASK     MoonModifier_Mod1

static MoonWindow *
GetNormalWindow (RichTextArea *textbox)
{
	if (!textbox->IsAttached ())
		return NULL;
	
	return textbox->GetDeployment ()->GetSurface ()->GetNormalWindow ();
}

static MoonClipboard *
GetClipboard (RichTextArea *textbox, MoonClipboardType clipboardType)
{
	MoonWindow *window = GetNormalWindow (textbox);
	
	if (!window)
		return NULL;
	
	return window->GetClipboard (clipboardType);
}

RichTextArea::RichTextArea ()
{
	SetObjectType (Type::RICHTEXTAREA);
	
	ManagedTypeInfo *type_info = g_new (ManagedTypeInfo, 1);
	type_info->Initialize (GetObjectType (), "System.Windows.Controls.RichTextArea");
	SetDefaultStyleKey (type_info);
	ManagedTypeInfo::Free (type_info);
	
	AddHandler (UIElement::MouseLeftButtonMultiClickEvent, RichTextArea::mouse_left_button_multi_click, this);
	
	contentElement = NULL;
	
	MoonWindowingSystem *ws = runtime_get_windowing_system ();
	im_ctx = ws->CreateIMContext ();
	im_ctx->SetUsePreedit (false);
	
	im_ctx->SetRetrieveSurroundingCallback ((MoonCallback) RichTextArea::retrieve_surrounding, this);
	im_ctx->SetDeleteSurroundingCallback ((MoonCallback) RichTextArea::delete_surrounding, this);
	im_ctx->SetCommitCallback ((MoonCallback) RichTextArea::commit, this);
	
	undo = new RichTextBoxUndoStack (10);
	redo = new RichTextBoxUndoStack (10);
	
	events_mask = CONTENT_CHANGED | SELECTION_CHANGED;
	emit = NOTHING_CHANGED;
	
	selection_anchor = 0;
	selection_cursor = 0;
	cursor_offset = 0.0;
	batch = 0;
	
	accepts_return = false;
	need_im_reset = false;
	is_read_only = false;
	have_offset = false;
	selecting = false;
	setvalue = true;
	captured = false;
	focused = false;
	view = NULL;
}

RichTextArea::~RichTextArea ()
{
	RemoveHandler (UIElement::MouseLeftButtonMultiClickEvent, RichTextArea::mouse_left_button_multi_click, this);
	
	ResetIMContext ();
	delete im_ctx;
	
	//delete buffer;
	delete undo;
	delete redo;
}

void
RichTextArea::SetIsAttached (bool value)
{
	Control::SetIsAttached (value);
	
	Surface *surface = GetDeployment ()->GetSurface ();
	
	if (value) {
		surface->AddHandler (Surface::WindowAvailableEvent, RichTextArea::attach_im_client_window, this);
		surface->AddHandler (Surface::WindowUnavailableEvent, RichTextArea::detach_im_client_window, this);
	} else if (surface) {
		surface->RemoveHandler (Surface::WindowAvailableEvent, RichTextArea::attach_im_client_window, this);
		surface->RemoveHandler (Surface::WindowUnavailableEvent, RichTextArea::detach_im_client_window, this);
		
		DetachIMClientWindow (NULL, NULL);
	}
}

void
RichTextArea::attach_im_client_window (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextArea *) closure)->AttachIMClientWindow (sender, args);
}

void
RichTextArea::AttachIMClientWindow (EventObject *sender, EventArgs *calldata)
{
	im_ctx->SetClientWindow (GetDeployment ()->GetSurface ()->GetWindow());
}

void
RichTextArea::detach_im_client_window (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextArea *) closure)->DetachIMClientWindow (sender, args);
}

void
RichTextArea::DetachIMClientWindow (EventObject *sender, EventArgs *calldata)
{
	im_ctx->SetClientWindow (NULL);
}

double
RichTextArea::GetCursorOffset ()
{
	if (!have_offset && view) {
		cursor_offset = view->GetCursor ().x;
		have_offset = true;
	}
	
	return cursor_offset;
}

int
RichTextArea::CursorDown (int cursor, bool page)
{
	return -1;
}

int
RichTextArea::CursorUp (int cursor, bool page)
{
	return -1;
}

int
RichTextArea::CursorNextWord (int cursor)
{
	return -1;
}

int
RichTextArea::CursorPrevWord (int cursor)
{
	return -1;
}

int
RichTextArea::CursorLineBegin (int cursor)
{
	return -1;
}

int
RichTextArea::CursorLineEnd (int cursor, bool include)
{
	return -1;
}

bool
RichTextArea::KeyPressBackSpace (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressDelete (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressPageDown (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressPageUp (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressDown (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressUp (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressHome (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressEnd (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressRight (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressLeft (MoonModifier modifiers)
{
	return false;
}

bool
RichTextArea::KeyPressUnichar (gunichar c)
{
	return false;
}

void
RichTextArea::BatchPush ()
{
	batch++;
}

void
RichTextArea::BatchPop ()
{
	if (batch == 0) {
		g_warning ("RichTextArea batch underflow");
		return;
	}
	
	batch--;
}

void
RichTextArea::EmitSelectionChanged ()
{
	EmitAsync (RichTextArea::SelectionChangedEvent, new RoutedEventArgs ());
}

void
RichTextArea::EmitContentChanged ()
{
	EmitAsync (RichTextArea::ContentChangedEvent, new ContentChangedEventArgs ());
}

void
RichTextArea::SyncSelection ()
{
	// FIXME: sync the TextSelection
}

void
RichTextArea::SyncContent ()
{
	// FIXME: generate the proper xaml
	char *xaml = NULL;
	
	setvalue = false;
	SetValue (RichTextArea::XamlProperty, Value (xaml, true));
	setvalue = true;
}

void
RichTextArea::SyncAndEmit (bool sync_text)
{
	if (batch != 0 || emit == NOTHING_CHANGED)
		return;
	
	if (sync_text && (emit & CONTENT_CHANGED))
		SyncContent ();
	
	if (emit & SELECTION_CHANGED)
		SyncSelection ();
	
	if (IsLoaded ()) {
		// eliminate events that we can't emit
		emit &= events_mask;
		
		if (emit & CONTENT_CHANGED)
			EmitContentChanged ();
		
		if (emit & SELECTION_CHANGED)
			EmitSelectionChanged ();
	}
	
	emit = NOTHING_CHANGED;
}

void
RichTextArea::Paste (MoonClipboard *clipboard, const char *str)
{
	// FIXME: implement this
}

void
RichTextArea::paste (MoonClipboard *clipboard, const char *text, gpointer closure)
{
	((RichTextArea *) closure)->Paste (clipboard, text);
}

void
RichTextArea::OnKeyDown (KeyEventArgs *args)
{
	MoonKeyEvent *event = args->GetEvent ();
	MoonModifier modifiers = (MoonModifier) event->GetModifiers ();
	Key key = event->GetSilverlightKey ();
	MoonClipboard *clipboard;
	bool handled = false;
	
	if (event->IsModifier ())
		return;
	
	// set 'emit' to NOTHING_CHANGED so that we can figure out
	// what has chanegd after applying the changes that this
	// keypress will cause.
	emit = NOTHING_CHANGED;
	BatchPush ();
	
	switch (key) {
	case KeyBACKSPACE:
		if (is_read_only)
			break;
		
		handled = KeyPressBackSpace (modifiers);
		break;
	case KeyDELETE:
		if (is_read_only)
			break;
		
		if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == SHIFT_MASK) {
			// Shift+Delete => Cut
			if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
				if (selection_cursor != selection_anchor) {
					// copy selection to the clipboard and then cut
					// FIXME: clipboard->SetText (GetSelectedText (), -1);
				}
			}
			
			// FIXME: SetSelectedText ("");
			handled = true;
		} else {
			handled = KeyPressDelete (modifiers);
		}
		break;
	case KeyINSERT:
		if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == SHIFT_MASK) {
			// Shift+Insert => Paste
			if (is_read_only)
				break;
			
			if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
				// paste clipboard contents to the buffer
				clipboard->AsyncGetText (RichTextArea::paste, this);
			}
			
			handled = true;
		} else if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == CONTROL_MASK) {
			// Control+Insert => Copy
			if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
				if (selection_cursor != selection_anchor) {
					// copy selection to the clipboard
					// FIXME: clipboard->SetText (GetSelectedText (), -1);
				}
			}
			
			handled = true;
		}
		break;
	case KeyPAGEDOWN:
		handled = KeyPressPageDown (modifiers);
		break;
	case KeyPAGEUP:
		handled = KeyPressPageUp (modifiers);
		break;
	case KeyHOME:
		handled = KeyPressHome (modifiers);
		break;
	case KeyEND:
		handled = KeyPressEnd (modifiers);
		break;
	case KeyRIGHT:
		handled = KeyPressRight (modifiers);
		break;
	case KeyLEFT:
		handled = KeyPressLeft (modifiers);
		break;
	case KeyDOWN:
		handled = KeyPressDown (modifiers);
		break;
	case KeyUP:
		handled = KeyPressUp (modifiers);
		break;
	default:
		if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == CONTROL_MASK) {
			switch (key) {
			case KeyA:
				// Ctrl+A => Select All
				handled = true;
				SelectAll ();
				break;
			case KeyC:
				// Ctrl+C => Copy
				if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
					if (selection_cursor != selection_anchor) {
						// copy selection to the clipboard
						// FIXME: clipboard->SetText (GetSelectedText (), -1);
					}
				}
				
				handled = true;
				break;
			case KeyX:
				// Ctrl+X => Cut
				if (is_read_only)
					break;
				
				if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
					if (selection_cursor != selection_anchor) {
						// copy selection to the clipboard and then cut
						// FIXME: clipboard->SetText (GetSelectedText(), -1);
					}
				}
				
				// FIXME: SetSelectedText ("");
				handled = true;
				break;
			case KeyV:
				// Ctrl+V => Paste
				if (is_read_only)
					break;
				
				if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
					// paste clipboard contents to the buffer
					clipboard->AsyncGetText (RichTextArea::paste, this);
				}
				
				handled = true;
				break;
			case KeyY:
				// Ctrl+Y => Redo
				if (!is_read_only) {
					handled = true;
					Redo ();
				}
				break;
			case KeyZ:
				// Ctrl+Z => Undo
				if (!is_read_only) {
					handled = true;
					Undo ();
				}
				break;
			default:
				// unhandled Control commands
				break;
			}
		}
		break;
	}
	
	if (handled) {
		args->SetHandled (handled);
		ResetIMContext ();
	}
	
	BatchPop ();
	
	SyncAndEmit ();
}

void
RichTextArea::PostOnKeyDown (KeyEventArgs *args)
{
	MoonKeyEvent *event = args->GetEvent ();
	int key = event->GetSilverlightKey ();
	gunichar c;
	
	// Note: we don't set Handled=true because anything we handle here, we
	// want to bubble up.
	if (!is_read_only && im_ctx->FilterKeyPress (event)) {
		need_im_reset = true;
		return;
	}
	
	if (is_read_only || event->IsModifier ())
		return;
	
	// set 'emit' to NOTHING_CHANGED so that we can figure out
	// what has changed after applying the changes that this
	// keypress will cause.
	emit = NOTHING_CHANGED;
	BatchPush ();
	
	switch (key) {
	case KeyENTER:
		KeyPressUnichar ('\r');
		break;
	default:
		if ((event->GetModifiers () & (CONTROL_MASK | ALT_MASK)) == 0) {
			// normal character input
			if ((c = event->GetUnicode ()))
				KeyPressUnichar (c);
		}
		break;
	}
	
	BatchPop ();
	
	SyncAndEmit ();
}

void
RichTextArea::OnKeyUp (KeyEventArgs *args)
{
	if (!is_read_only) {
		if (im_ctx->FilterKeyPress (args->GetEvent()))
			need_im_reset = true;
	}
}

bool
RichTextArea::DeleteSurrounding (int offset, int n_chars)
{
	// FIXME: implement this
	return true;
}

gboolean
RichTextArea::delete_surrounding (MoonIMContext *context, int offset, int n_chars, gpointer user_data)
{
	return ((RichTextArea *) user_data)->DeleteSurrounding (offset, n_chars);
}

bool
RichTextArea::RetrieveSurrounding ()
{
	// FIXME: implement this
	//im_ctx->SetSurroundingText (text, -1, cursor - text);
	
	return true;
}

gboolean
RichTextArea::retrieve_surrounding (MoonIMContext *context, gpointer user_data)
{
	return ((RichTextArea *) user_data)->RetrieveSurrounding ();
}

void
RichTextArea::Commit (const char *str)
{
	// FIXME: implement this
}

void
RichTextArea::commit (MoonIMContext *context, const char *str, gpointer user_data)
{
	((RichTextArea *) user_data)->Commit (str);
}

void
RichTextArea::ResetIMContext ()
{
	if (need_im_reset) {
		im_ctx->Reset ();
		need_im_reset = false;
	}
}

void
RichTextArea::OnMouseLeftButtonDown (MouseButtonEventArgs *args)
{
	double x, y;
	int cursor;
	
	args->SetHandled (true);
	Focus ();
	
	if (view) {
		args->GetPosition (view, &x, &y);
		
		cursor = view->GetCursorFromXY (x, y);
		
		ResetIMContext ();
		
		// Single-Click: cursor placement
		captured = CaptureMouse ();
		selecting = true;
		
		BatchPush ();
		emit = NOTHING_CHANGED;
		//SetSelectionStart (cursor);
		//SetSelectionLength (0);
		BatchPop ();
		
		SyncAndEmit ();
	}
}

void
RichTextArea::OnMouseLeftButtonMultiClick (MouseButtonEventArgs *args)
{
	int cursor, start, end;
	double x, y;
	
	args->SetHandled (true);
	
	if (view) {
		args->GetPosition (view, &x, &y);
		
		cursor = view->GetCursorFromXY (x, y);
		
		ResetIMContext ();
		
		if (((MoonButtonEvent*)args->GetEvent())->GetNumberOfClicks () == 3) {
			// Note: Silverlight doesn't implement this, but to
			// be consistent with other TextEntry-type
			// widgets in Gtk+, we will.
			//
			// Triple-Click: select the line
			if (captured)
				ReleaseMouseCapture ();
			start = CursorLineBegin (cursor);
			end = CursorLineEnd (cursor, true);
			selecting = false;
			captured = false;
		} else {
			// Double-Click: select the word
			if (captured)
				ReleaseMouseCapture ();
			start = CursorPrevWord (cursor);
			end = CursorNextWord (cursor);
			selecting = false;
			captured = false;
		}
		
		BatchPush ();
		emit = NOTHING_CHANGED;
		//SetSelectionStart (start);
		//SetSelectionLength (end - start);
		BatchPop ();
		
		SyncAndEmit ();
	}
}

void
RichTextArea::mouse_left_button_multi_click (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextArea *) closure)->OnMouseLeftButtonMultiClick ((MouseButtonEventArgs *) args);
}

void
RichTextArea::OnMouseLeftButtonUp (MouseButtonEventArgs *args)
{
	if (captured)
		ReleaseMouseCapture ();
	
	args->SetHandled (true);
	selecting = false;
	captured = false;
}

void
RichTextArea::OnMouseMove (MouseEventArgs *args)
{
	int anchor = selection_anchor;
	int cursor = selection_cursor;
	MoonClipboard *clipboard;
	double x, y;
	
	if (selecting) {
		args->GetPosition (view, &x, &y);
		args->SetHandled (true);
		
		cursor = view->GetCursorFromXY (x, y);
		
		BatchPush ();
		emit = NOTHING_CHANGED;
		//SetSelectionStart (MIN (anchor, cursor));
		//SetSelectionLength (abs (cursor - anchor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		BatchPop ();
		
		SyncAndEmit ();
		
		if ((clipboard = GetClipboard (this, MoonClipboard_Primary))) {
			// copy the selection to the primary clipboard
			// FIXME: clipboard->SetText (GetSelectedText (), -1);
		}
	}
}

void
RichTextArea::OnLostFocus (RoutedEventArgs *args)
{
	BatchPush ();
	emit = NOTHING_CHANGED;
	//SetSelectionStart (selection_cursor);
	//SetSelectionLength (0);
	BatchPop ();
	
	SyncAndEmit ();
	
	focused = false;
	
	if (view)
		view->OnLostFocus ();
	
	if (!is_read_only) {
		im_ctx->FocusOut ();
		need_im_reset = true;
	}
}

void
RichTextArea::OnGotFocus (RoutedEventArgs *args)
{
	focused = true;
	
	if (view)
		view->OnGotFocus ();
	
	if (!is_read_only) {
		im_ctx->FocusIn ();
		need_im_reset = true;
	}
}

void
RichTextArea::EmitCursorPositionChanged (double height, double x, double y)
{
	//Emit (RichTextArea::CursorPositionChangedEvent, new CursorPositionChangedEventArgs (height, x, y));
}

void
RichTextArea::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	RichTextBoxModelChangeType changed = RichTextBoxModelChangedNothing;
	DependencyProperty *prop;
	
	if (args->GetProperty ()->GetOwnerType () != Type::RICHTEXTAREA) {
		Control::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == RichTextArea::AcceptsReturnProperty) {
		// update accepts_return state
		accepts_return = args->GetNewValue ()->AsBool ();
	} else if (args->GetId () == RichTextArea::BaselineOffsetProperty) {
		// propagate this to our view
		changed = RichTextBoxModelChangedBaselineOffset;
	} else if (args->GetId () == RichTextArea::CaretBrushProperty) {
		// Note: if we wanted to be perfect, we could invalidate the
		// blinking cursor rect if it is active... but is it that
		// really important? I don't think so...
	} else if (args->GetId () == RichTextArea::IsReadOnlyProperty) {
		// update is_read_only state
		is_read_only = args->GetNewValue ()->AsBool ();
		
		if (focused) {
			if (is_read_only) {
				ResetIMContext ();
				im_ctx->FocusOut ();
			} else {
				im_ctx->FocusIn ();
			}
		}
		
		if (view)
			view->SetEnableCursor (!is_read_only);
	} else if (args->GetId () == RichTextArea::SelectionProperty) {
		// FIXME: probably need to do stuff here...
		changed = RichTextBoxModelChangedSelection;
	} else if (args->GetId () == RichTextArea::TextAlignmentProperty) {
		changed = RichTextBoxModelChangedTextAlignment;
	} else if (args->GetId () == RichTextArea::TextWrappingProperty) {
		if (contentElement) {
			if ((prop = contentElement->GetDependencyProperty ("HorizontalScrollBarVisibility"))) {
				// If TextWrapping is set to Wrap, disable the horizontal scroll bars
				if (args->GetNewValue ()->AsTextWrapping () == TextWrappingWrap)
					contentElement->SetValue (prop, Value (ScrollBarVisibilityDisabled, Type::SCROLLBARVISIBILITY));
				else
					contentElement->SetValue (prop, GetValue (RichTextArea::HorizontalScrollBarVisibilityProperty));
			}
		}
		
		changed = RichTextBoxModelChangedTextWrapping;
	} else if (args->GetId () == RichTextArea::HorizontalScrollBarVisibilityProperty) {
		// XXX more crap because these aren't templatebound.
		if (contentElement) {
			if ((prop = contentElement->GetDependencyProperty ("HorizontalScrollBarVisibility"))) {
				// If TextWrapping is set to Wrap, disable the horizontal scroll bars
				if (GetTextWrapping () == TextWrappingWrap)
					contentElement->SetValue (prop, Value (ScrollBarVisibilityDisabled, Type::SCROLLBARVISIBILITY));
				else
					contentElement->SetValue (prop, args->GetNewValue ());
			}
		}
	} else if (args->GetId () == RichTextArea::VerticalScrollBarVisibilityProperty) {
		// XXX more crap because these aren't templatebound.
		if (contentElement) {
			if ((prop = contentElement->GetDependencyProperty ("VerticalScrollBarVisibility")))
				contentElement->SetValue (prop, args->GetNewValue ());
		}
	} else if (args->GetId () == RichTextArea::XamlProperty) {
		// FIXME: need to sync the XAML to Blocks
	}
	
	if (changed != RichTextBoxModelChangedNothing && HasHandlers (ModelChangedEvent))
		Emit (ModelChangedEvent, new RichTextBoxModelChangedEventArgs (changed, args));
	
	NotifyListenersOfPropertyChange (args, error);
}

void
RichTextArea::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop->GetOwnerType () != Type::RICHTEXTAREA)
		Control::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
RichTextArea::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	// FIXME: need to sync the Blocks to XAML
	Control::OnCollectionChanged (col, args);
}

void
RichTextArea::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (!PropertyHasValueNoAutoCreate (RichTextArea::BlocksProperty, col)) {
		Control::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	// FIXME: if we could pass the obj to the View via the Args, the View
	// could probably optimize the invalidated region to minimize redraw
	if (HasHandlers (ModelChangedEvent))
		Emit (ModelChangedEvent, new RichTextBoxModelChangedEventArgs (RichTextBoxModelChangedContent, args));
}

void
RichTextArea::OnApplyTemplate ()
{
	DependencyProperty *prop;
	
	contentElement = GetTemplateChild ("ContentElement");
	
	if (contentElement == NULL) {
		g_warning ("RichTextArea::OnApplyTemplate: no ContentElement found");
		Control::OnApplyTemplate ();
		return;
	}
	
	// XXX LAME these should be template bindings in the textbox template.
	if ((prop = contentElement->GetDependencyProperty ("VerticalScrollBarVisibility")))
		contentElement->SetValue (prop, GetValue (RichTextArea::VerticalScrollBarVisibilityProperty));
	
	if ((prop = contentElement->GetDependencyProperty ("HorizontalScrollBarVisibility"))) {
		// If TextWrapping is set to Wrap, disable the horizontal scroll bars
		if (GetTextWrapping () == TextWrappingWrap)
			contentElement->SetValue (prop, Value (ScrollBarVisibilityDisabled, Type::SCROLLBARVISIBILITY));
		else
			contentElement->SetValue (prop, GetValue (RichTextArea::HorizontalScrollBarVisibilityProperty));
	}
	
	// Create our view control
	view = new RichTextBoxView ();
	
	view->SetEnableCursor (!is_read_only);
	view->SetTextBox (this);
	
	// Insert our RichTextBoxView
	if (contentElement->Is (Type::CONTENTCONTROL)) {
		ContentControl *control = (ContentControl *) contentElement;
		
		control->SetValue (ContentControl::ContentProperty, Value (view));
	} else if (contentElement->Is (Type::BORDER)) {
		Border *border = (Border *) contentElement;
		
		border->SetValue (Border::ChildProperty, Value (view));
	} else if (contentElement->Is (Type::PANEL)) {
		DependencyObjectCollection *children = ((Panel *) contentElement)->GetChildren ();
		
		children->Add (view);
	} else {
		g_warning ("RichTextBox::OnApplyTemplate: don't know how to handle a ContentElement of type %s",
			   contentElement->GetType ()->GetName ());
		view->unref ();
		view = NULL;
	}
	
	Control::OnApplyTemplate ();
}

void
RichTextArea::SelectAll ()
{
	// FIXME: implement this
}

bool
RichTextArea::CanUndo ()
{
	return !undo->IsEmpty ();
}

bool
RichTextArea::CanRedo ()
{
	return !redo->IsEmpty ();
}

void
RichTextArea::Undo ()
{
	//RichTextBoxUndoActionReplace *replace;
	//RichTextBoxUndoActionInsert *insert;
	//RichTextBoxUndoActionDelete *dele;
	RichTextBoxUndoAction *action;
	int anchor = 0, cursor = 0;
	
	if (undo->IsEmpty ())
		return;
	
	action = undo->Pop ();
	redo->Push (action);
	
	switch (action->type) {
	case RichTextBoxUndoActionTypeInsert:
		//insert = (TextBoxUndoActionInsert *) action;
		
		//buffer->Cut (insert->start, insert->length);
		//anchor = action->selection_anchor;
		//cursor = action->selection_cursor;
		break;
	case RichTextBoxUndoActionTypeDelete:
		//dele = (TextBoxUndoActionDelete *) action;
		
		//buffer->Insert (dele->start, dele->text, dele->length);
		//anchor = action->selection_anchor;
		//cursor = action->selection_cursor;
		break;
	case RichTextBoxUndoActionTypeReplace:
		//replace = (TextBoxUndoActionReplace *) action;
		
		//buffer->Cut (replace->start, replace->inlen);
		//buffer->Insert (replace->start, replace->deleted, replace->length);
		//anchor = action->selection_anchor;
		//cursor = action->selection_cursor;
		break;
	}
	
	BatchPush ();
	//SetSelectionStart (MIN (anchor, cursor));
	//SetSelectionLength (abs (cursor - anchor));
	emit = CONTENT_CHANGED | SELECTION_CHANGED;
	selection_anchor = anchor;
	selection_cursor = cursor;
	BatchPop ();
	
	SyncAndEmit ();
}

void
RichTextArea::Redo ()
{
	//RichTextBoxUndoActionReplace *replace;
	//RichTextBoxUndoActionInsert *insert;
	//RichTextBoxUndoActionDelete *dele;
	RichTextBoxUndoAction *action;
	int anchor = 0, cursor = 0;
	
	if (redo->IsEmpty ())
		return;
	
	action = redo->Pop ();
	undo->Push (action);
	
	switch (action->type) {
	case RichTextBoxUndoActionTypeInsert:
		//insert = (TextBoxUndoActionInsert *) action;
		
		//buffer->Insert (insert->start, insert->buffer->text, insert->buffer->len);
		//anchor = cursor = insert->start + insert->buffer->len;
		break;
	case RichTextBoxUndoActionTypeDelete:
		//dele = (TextBoxUndoActionDelete *) action;
		
		//buffer->Cut (dele->start, dele->length);
		//anchor = cursor = dele->start;
		break;
	case RichTextBoxUndoActionTypeReplace:
		//replace = (TextBoxUndoActionReplace *) action;
		
		//buffer->Cut (replace->start, replace->length);
		//buffer->Insert (replace->start, replace->inserted, replace->inlen);
		//anchor = cursor = replace->start + replace->inlen;
		break;
	}
	
	BatchPush ();
	//SetSelectionStart (MIN (anchor, cursor));
	//SetSelectionLength (abs (cursor - anchor));
	emit = CONTENT_CHANGED | SELECTION_CHANGED;
	selection_anchor = anchor;
	selection_cursor = cursor;
	BatchPop ();
	
	SyncAndEmit ();
}


//
// RichTextBoxView
//

#define CURSOR_BLINK_ON_MULTIPLIER    2
#define CURSOR_BLINK_OFF_MULTIPLIER   1
#define CURSOR_BLINK_DELAY_MULTIPLIER 3
#define CURSOR_BLINK_DIVIDER          3

RichTextBoxView::RichTextBoxView ()
{
	SetObjectType (Type::RICHTEXTBOXVIEW);
	
	AddHandler (UIElement::MouseLeftButtonDownEvent, RichTextBoxView::mouse_left_button_down, this);
	AddHandler (UIElement::MouseLeftButtonUpEvent, RichTextBoxView::mouse_left_button_up, this);
	
	SetCursor (CursorTypeIBeam);
	
	cursor = Rect (0, 0, 0, 0);
	layout = new TextLayout ();
	selection_changed = false;
	had_selected_text = false;
	cursor_visible = false;
	enable_cursor = true;
	blink_timeout = 0;
	textbox = NULL;
	dirty = false;
}

RichTextBoxView::~RichTextBoxView ()
{
	RemoveHandler (UIElement::MouseLeftButtonDownEvent, RichTextBoxView::mouse_left_button_down, this);
	RemoveHandler (UIElement::MouseLeftButtonUpEvent, RichTextBoxView::mouse_left_button_up, this);
	
	if (textbox) {
		textbox->RemoveHandler (RichTextArea::ModelChangedEvent, RichTextBoxView::model_changed, this);
		textbox->view = NULL;
	}
	
	DisconnectBlinkTimeout ();
	
	delete layout;
}

TextLayoutLine *
RichTextBoxView::GetLineFromY (double y, int *index)
{
	return layout->GetLineFromY (Point (), y, index);
}

TextLayoutLine *
RichTextBoxView::GetLineFromIndex (int index)
{
	return layout->GetLineFromIndex (index);
}

int
RichTextBoxView::GetCursorFromXY (double x, double y)
{
	return layout->GetCursorFromXY (Point (), x, y);
}

gboolean
RichTextBoxView::blink (void *user_data)
{
	return ((RichTextBoxView *) user_data)->Blink ();
}

static guint
GetCursorBlinkTimeout (RichTextBoxView *view)
{
	MoonWindow *window;
	
	if (!view->IsAttached ())
		return CURSOR_BLINK_TIMEOUT_DEFAULT;
	
	if (!(window = view->GetDeployment ()->GetSurface ()->GetWindow ()))
		return CURSOR_BLINK_TIMEOUT_DEFAULT;
	
	return runtime_get_windowing_system ()->GetCursorBlinkTimeout (window);
}

void
RichTextBoxView::ConnectBlinkTimeout (guint multiplier)
{
	guint timeout = GetCursorBlinkTimeout (this) * multiplier / CURSOR_BLINK_DIVIDER;
	TimeManager *manager;
	
	if (!IsAttached () || !(manager = GetDeployment ()->GetSurface ()->GetTimeManager ()))
		return;
	
	blink_timeout = manager->AddTimeout (MOON_PRIORITY_DEFAULT, timeout, RichTextBoxView::blink, this);
}

void
RichTextBoxView::DisconnectBlinkTimeout ()
{
	TimeManager *manager;
	
	if (blink_timeout != 0) {
		if (!IsAttached () || !(manager = GetDeployment ()->GetSurface ()->GetTimeManager ()))
			return;
		
		manager->RemoveTimeout (blink_timeout);
		blink_timeout = 0;
	}
}

bool
RichTextBoxView::Blink ()
{
	guint multiplier;
	
	SetCurrentDeployment (true);
	
	if (cursor_visible) {
		multiplier = CURSOR_BLINK_OFF_MULTIPLIER;
		HideCursor ();
	} else {
		multiplier = CURSOR_BLINK_ON_MULTIPLIER;
		ShowCursor ();
	}
	
	ConnectBlinkTimeout (multiplier);
	
	return false;
}

void
RichTextBoxView::DelayCursorBlink ()
{
	DisconnectBlinkTimeout ();
	ConnectBlinkTimeout (CURSOR_BLINK_DELAY_MULTIPLIER);
	UpdateCursor (true);
	ShowCursor ();
}

void
RichTextBoxView::BeginCursorBlink ()
{
	if (blink_timeout == 0) {
		ConnectBlinkTimeout (CURSOR_BLINK_ON_MULTIPLIER);
		UpdateCursor (true);
		ShowCursor ();
	}
}

void
RichTextBoxView::EndCursorBlink ()
{
	DisconnectBlinkTimeout ();
	
	if (cursor_visible)
		HideCursor ();
}

void
RichTextBoxView::ResetCursorBlink (bool delay)
{
	if (textbox->IsFocused () && !textbox->HasSelectedText ()) {
		if (enable_cursor) {
			// cursor is blinkable... proceed with blinkage
			if (delay)
				DelayCursorBlink ();
			else
				BeginCursorBlink ();
		} else {
			// cursor not blinkable, but we still need to keep track of it
			UpdateCursor (false);
		}
	} else {
		// cursor not blinkable... stop all blinkage
		EndCursorBlink ();
	}
}

void
RichTextBoxView::InvalidateCursor ()
{
	Invalidate (cursor.Transform (&absolute_xform));
}

void
RichTextBoxView::ShowCursor ()
{
	cursor_visible = true;
	InvalidateCursor ();
}

void
RichTextBoxView::HideCursor ()
{
	cursor_visible = false;
	InvalidateCursor ();
}

void
RichTextBoxView::UpdateCursor (bool invalidate)
{
	int cur = textbox->GetCursor ();
	Rect current = cursor;
	Rect rect;
	
	// invalidate current cursor rect
	if (invalidate && cursor_visible)
		InvalidateCursor ();
	
	// calculate the new cursor rect
	cursor = layout->GetCursor (Point (), cur);
	
	// transform the cursor rect into absolute coordinates for the IM context
	rect = cursor.Transform (&absolute_xform);
	
	textbox->im_ctx->SetCursorLocation (rect);
	
	if (cursor != current)
		textbox->EmitCursorPositionChanged (cursor.height, cursor.x, cursor.y);
	
	// invalidate the new cursor rect
	if (invalidate && cursor_visible)
		InvalidateCursor ();
}

void
RichTextBoxView::UpdateText ()
{
	const char *text = NULL; // FIXME: textbox->GetDisplayText ();
	
	layout->SetText (text ? text : "", -1);
}

void
RichTextBoxView::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = GetActualHeight ();
	*width = GetActualWidth ();
}

Size
RichTextBoxView::ComputeActualSize ()
{
	if (ReadLocalValue (LayoutInformation::LayoutSlotProperty))
		return FrameworkElement::ComputeActualSize ();

	Layout (Size (INFINITY, INFINITY));

	Size actual (0,0);
	layout->GetActualExtents (&actual.width, &actual.height);
       
	return actual;
}

Size
RichTextBoxView::MeasureOverride (Size availableSize)
{
	Size desired = Size ();
	
	Layout (availableSize);
	
	layout->GetActualExtents (&desired.width, &desired.height);
	
	/* FIXME using a magic number for minumum width here */
	if (isinf (availableSize.width))
		desired.width = MAX (desired.width, 11);

	return desired.Min (availableSize);
}

Size
RichTextBoxView::ArrangeOverride (Size finalSize)
{
	Size arranged = Size ();
	
	Layout (finalSize);
	
	layout->GetActualExtents (&arranged.width, &arranged.height);

	arranged = arranged.Max (finalSize);

	return arranged;
}

void
RichTextBoxView::Layout (Size constraint)
{
	layout->SetMaxWidth (constraint.width);
	
	layout->Layout ();
	dirty = false;
}

void
RichTextBoxView::Paint (cairo_t *cr)
{
	layout->Render (cr, GetOriginPoint (), Point ());
	
	if (cursor_visible) {
		cairo_antialias_t alias = cairo_get_antialias (cr);
		Brush *caret = textbox->GetCaretBrush ();
		double h = round (cursor.height);
		double x = cursor.x;
		double y = cursor.y;
		
		// disable antialiasing
		cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
		
		// snap 'x' to the half-pixel grid (to try and get a sharp 1-pixel-wide line)
		cairo_user_to_device (cr, &x, &y);
		x = trunc (x) + 0.5; y = trunc (y);
		cairo_device_to_user (cr, &x, &y);
		
		// set the cursor color
		bool unref = false;
		if (!caret) {
			caret = new SolidColorBrush ("Black");
			unref = true;
		}
		caret->SetupBrush (cr, cursor);
		
		// draw the cursor
		cairo_set_line_width (cr, 1.0);
		cairo_move_to (cr, x, y);
		cairo_line_to (cr, x, y + h);
		
		// stroke the caret
		caret->Stroke (cr);
		
		if (unref)
			caret->unref ();

		// restore antialiasing
		cairo_set_antialias (cr, alias);
	}
}

void
RichTextBoxView::Render (cairo_t *cr, Region *region, bool path_only)
{
	Size renderSize = GetRenderSize ();
	
	UpdateCursor (false);
	
	if (selection_changed) {
		//layout->Select (textbox->GetSelectionStart (), textbox->GetSelectionLength ());
		selection_changed = false;
	}
	
	cairo_save (cr);
	ApplyTransform (cr);
	
	if (!path_only)
		RenderLayoutClip (cr);
	
	layout->SetAvailableWidth (renderSize.width);
	Paint (cr);
	cairo_restore (cr);
}

void
RichTextBoxView::OnModelChanged (RichTextBoxModelChangedEventArgs *args)
{
	switch (args->changed) {
	case RichTextBoxModelChangedTextAlignment:
		// text alignment changed, update our layout
		if (layout->SetTextAlignment (args->property->GetNewValue()->AsTextAlignment ()))
			dirty = true;
		break;
	case RichTextBoxModelChangedTextWrapping:
		// text wrapping changed, update our layout
		if (layout->SetTextWrapping (args->property->GetNewValue()->AsTextWrapping ()))
			dirty = true;
		break;
	case RichTextBoxModelChangedSelection:
		if (had_selected_text || textbox->HasSelectedText ()) {
			// the selection has changed, update the layout's selection
			had_selected_text = textbox->HasSelectedText ();
			selection_changed = true;
			ResetCursorBlink (false);
		} else {
			// cursor position changed
			ResetCursorBlink (true);
			return;
		}
		break;
	case RichTextBoxModelChangedBrush:
		// a brush has changed, no layout updates needed, we just need to re-render
		break;
	case RichTextBoxModelChangedContent:
		// the text has changed, need to recalculate layout/bounds
		UpdateText ();
		dirty = true;
		break;
	default:
		// nothing changed??
		return;
	}
	
	if (dirty) {
		InvalidateMeasure ();
		UpdateBounds (true);
	}
	
	Invalidate ();
}

void
RichTextBoxView::model_changed (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextBoxView *) closure)->OnModelChanged ((RichTextBoxModelChangedEventArgs *) args);
}

void
RichTextBoxView::OnLostFocus ()
{
	EndCursorBlink ();
}

void
RichTextBoxView::OnGotFocus ()
{
	ResetCursorBlink (false);
}

void
RichTextBoxView::OnMouseLeftButtonDown (MouseButtonEventArgs *args)
{
	// proxy to our parent TextBox control
	textbox->OnMouseLeftButtonDown (args);
}

void
RichTextBoxView::mouse_left_button_down (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextBoxView *) closure)->OnMouseLeftButtonDown ((MouseButtonEventArgs *) args);
}

void
RichTextBoxView::OnMouseLeftButtonUp (MouseButtonEventArgs *args)
{
	// proxy to our parent TextBox control
	textbox->OnMouseLeftButtonUp (args);
}

void
RichTextBoxView::mouse_left_button_up (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextBoxView *) closure)->OnMouseLeftButtonUp ((MouseButtonEventArgs *) args);
}

void
RichTextBoxView::SetTextBox (RichTextArea *textbox)
{
	TextLayoutAttributes *attrs;
	
	if (this->textbox == textbox)
		return;
	
	if (this->textbox) {
		// remove the event handlers from the old textbox
		this->textbox->RemoveHandler (RichTextArea::ModelChangedEvent, RichTextBoxView::model_changed, this);
	}
	
	this->textbox = textbox;
	
	if (textbox) {
		textbox->AddHandler (RichTextArea::ModelChangedEvent, RichTextBoxView::model_changed, this);
		
		// sync our state with the textbox
		layout->SetTextAttributes (new List ());
		attrs = new TextLayoutAttributes ((ITextAttributes *) textbox, 0);
		layout->GetTextAttributes ()->Append (attrs);
		
		layout->SetTextAlignment (textbox->GetTextAlignment ());
		layout->SetTextWrapping (textbox->GetTextWrapping ());
		had_selected_text = textbox->HasSelectedText ();
		selection_changed = true;
		UpdateText ();
	} else {
		layout->SetTextAttributes (NULL);
		layout->SetText (NULL, -1);
	}
	
	UpdateBounds (true);
	InvalidateMeasure ();
	Invalidate ();
	dirty = true;
}

void
RichTextBoxView::SetEnableCursor (bool enable)
{
	if ((enable_cursor && enable) || (!enable_cursor && !enable))
		return;
	
	enable_cursor = enable;
	
	if (enable)
		ResetCursorBlink (false);
	else
		EndCursorBlink ();
}
