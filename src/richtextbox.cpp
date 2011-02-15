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
#include "factory.h"

namespace Moonlight {

class RichTextBoxProvider : public FrameworkElementProvider {
	Value *xaml_value;
	Value *baseline_offset_value;

 public:
	RichTextBoxProvider (DependencyObject *obj, PropertyPrecedence precedence, int flags = 0)
		: FrameworkElementProvider (obj, precedence, flags)
	{
		xaml_value = NULL;
		baseline_offset_value = NULL;
	}

	virtual ~RichTextBoxProvider ()
	{
		delete xaml_value;
		delete baseline_offset_value;
	}

	virtual Value *GetPropertyValue (DependencyProperty *property)
	{
		if (property->GetId () == RichTextBox::XamlProperty) {
			delete xaml_value;
			GString *str = g_string_new ("");
			((RichTextBox*)obj)->SerializeXaml(str);
			xaml_value = new Value (g_string_free (str, FALSE), true);
			return xaml_value;
		}
		else if (property->GetId () == RichTextBox::BaselineOffsetProperty) {
			delete baseline_offset_value;
			RichTextBoxView *view = ((RichTextBox*)obj)->GetView();
			baseline_offset_value = new Value (view ? view->GetBaselineOffset() : 0);
			return baseline_offset_value;
		}

		return FrameworkElementProvider::GetPropertyValue (property);
	}
};

//
// RichTextBoxActions
//

class RichTextBoxAction : public List::Node {
public:
	virtual ~RichTextBoxAction () {}
	virtual void Do () = 0;
	virtual void Undo () = 0;
};

class RichTextBoxActionApplyFormatting : public RichTextBoxAction {
public:
	RichTextBoxActionApplyFormatting (RichTextBox* rtb, TextSelection* selection, DependencyProperty* prop, Value* v)
	{
		this->rtb = rtb;

		start = selection->GetStart();
		end = selection->GetEnd();
		
		this->prop = prop;
		this->v = *v;
	}

	virtual ~RichTextBoxActionApplyFormatting ()
	{
	}

	virtual void Do ()
	{
		TextSelection* selection = rtb->GetSelection();
		selection->Select (start, end);

		printf ("NIEX: RichTextBoxActionApplyFormatting::Do\n");
	}

	virtual void Undo ()
	{
		printf ("NIEX: RichTextBoxActionApplyFormatting::Undo\n");
	}

private:
	DependencyProperty* prop;
	Value v;

	TextPointer* start;
	TextPointer* end;
	RichTextBox* rtb;
};

class RichTextBoxActionSetSelectionText : public RichTextBoxAction {
public:
	RichTextBoxActionSetSelectionText (RichTextBox* rtb, TextSelection* selection, const char* text)
	{
		this->rtb = rtb;

		start = selection->GetStart();
		end = selection->GetEnd();

		this->text = g_strdup (text);
		this->previous_text = NULL;
	}

	virtual ~RichTextBoxActionSetSelectionText ()
	{
		delete start;
		delete end;

		g_free (text);
		g_free (previous_text);
	}

	virtual void Do ()
	{
		TextSelection *selection = rtb->GetSelection();
		selection->Select (start, end);
		previous_text = selection->GetText ();
		selection->SetText (text);
	}

	virtual void Undo ()
	{
		printf ("NIEX: RichTextBoxActionApplyFormatting::Undo\n");
	}

private:
	char* text;
	char* previous_text;

	TextPointer* start;
	TextPointer* end;
	RichTextBox* rtb;
};



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
GetNormalWindow (RichTextBox *textbox)
{
	if (!textbox->IsAttached ())
		return NULL;
	
	return textbox->GetDeployment ()->GetSurface ()->GetNormalWindow ();
}

static MoonClipboard *
GetClipboard (RichTextBox *textbox, MoonClipboardType clipboardType)
{
	MoonWindow *window = GetNormalWindow (textbox);
	
	if (!window)
		return NULL;
	
	return window->GetClipboard (clipboardType);
}

RichTextBox::RichTextBox ()
	: view (this, ViewWeakRef)
{
	SetObjectType (Type::RICHTEXTBOX);
	
	ManagedTypeInfo type_info (GetObjectType (), "System.Windows.Controls.RichTextBox");
	SetDefaultStyleKey (&type_info);
	
	AddHandler (UIElement::MouseLeftButtonMultiClickEvent, RichTextBox::mouse_left_button_multi_click, this);

	contentElement = NULL;
	
	MoonWindowingSystem *ws = Runtime::GetWindowingSystem ();
	im_ctx = ws->CreateIMContext ();
	im_ctx->SetUsePreedit (false);
	
	im_ctx->SetRetrieveSurroundingCallback ((MoonCallback) RichTextBox::retrieve_surrounding, this);
	im_ctx->SetDeleteSurroundingCallback ((MoonCallback) RichTextBox::delete_surrounding, this);
	im_ctx->SetCommitCallback ((MoonCallback) RichTextBox::commit, this);
	
	undo = new Stack (10);
	redo = new Stack (10);
	
	events_mask = CONTENT_CHANGED | SELECTION_CHANGED;
	emit = NOTHING_CHANGED;

	cursor_offset = 0.0;
	batch = 0;
	
	accepts_return = false;
	need_im_reset = false;
	have_offset = false;
	selecting = false;
	captured = false;
	focused = false;

	selection = NULL;

	delete providers.dynamicvalue;
	providers.dynamicvalue = new RichTextBoxProvider (this, PropertyPrecedence_DynamicValue);
}

RichTextBox::~RichTextBox ()
{
	ResetIMContext ();
	delete im_ctx;
	
	//delete buffer;
	delete undo;
	delete redo;
}

void
RichTextBox::OnIsAttachedChanged (bool value)
{
	Control::OnIsAttachedChanged (value);

	// Manually propagate the state change to the blocks property because it's a dynamically
	// provided value and not a local/autocreated value.
	GetBlocks ()->SetIsAttached (value);
	Surface *surface = GetDeployment ()->GetSurface ();
	
	if (value) {
		surface->AddHandler (Surface::WindowAvailableEvent, RichTextBox::attach_im_client_window, this);
		surface->AddHandler (Surface::WindowUnavailableEvent, RichTextBox::detach_im_client_window, this);
	} else if (surface) {
		surface->RemoveHandler (Surface::WindowAvailableEvent, RichTextBox::attach_im_client_window, this);
		surface->RemoveHandler (Surface::WindowUnavailableEvent, RichTextBox::detach_im_client_window, this);
		
		DetachIMClientWindow (NULL, NULL);
	}
}

void
RichTextBox::attach_im_client_window (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextBox *) closure)->AttachIMClientWindow (sender, args);
}

void
RichTextBox::AttachIMClientWindow (EventObject *sender, EventArgs *calldata)
{
	im_ctx->SetClientWindow (GetDeployment ()->GetSurface ()->GetWindow());
}

void
RichTextBox::detach_im_client_window (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextBox *) closure)->DetachIMClientWindow (sender, args);
}

void
RichTextBox::DetachIMClientWindow (EventObject *sender, EventArgs *calldata)
{
	im_ctx->SetClientWindow (NULL);
}

double
RichTextBox::GetCursorOffset ()
{
	if (!have_offset && view) {
		cursor_offset = view->GetCursor ().x;
		have_offset = true;
	}
	
	return cursor_offset;
}

TextPointer
RichTextBox::CursorDown (const TextPointer& cursor, bool page)
{
	// FIXME
	return cursor;
}

TextPointer
RichTextBox::CursorUp (const TextPointer& cursor, bool page)
{
	// FIXME
	return cursor;
}

TextPointer
RichTextBox::CursorNextWord (const TextPointer& cursor)
{
	// FIXME
	return cursor;
}

TextPointer
RichTextBox::CursorPrevWord (const TextPointer& cursor)
{
	// FIXME
	return cursor;
}

TextPointer
RichTextBox::CursorLineBegin (const TextPointer& cursor)
{
	// FIXME
	return cursor;
}

TextPointer
RichTextBox::CursorLineEnd (const TextPointer& cursor, bool include)
{
	// FIXME
	return cursor;
}

bool
RichTextBox::KeyPressBackSpace (MoonModifier modifiers)
{
#if 1
	return false;
#else
	TextSelection *selection = GetSelection ();

	TextBoxAction *action;
	int start = 0, length = 0;
	bool handled = false;

	if ((modifiers & (ALT_MASK | SHIFT_MASK)) != 0)
		return false;

	if (!selection->IsEmpty ()) {
		// BackSpace w/ active selection: delete the selected text
		length = abs (cursor - anchor);
		start = MIN (anchor, cursor);
	} else if ((modifiers & CONTROL_MASK) != 0) {
		// Ctrl+BackSpace: delete the word ending at the cursor
		start = CursorPrevWord (cursor);
		length = cursor - start;
	} else if (cursor > 0) {
		// BackSpace: delete the char before the cursor position
		if (cursor >= 2 && buffer->text[cursor - 1] == '\n' && buffer->text[cursor - 2] == '\r') {
			start = cursor - 2;
			length = 2;
		} else {
			start = cursor - 1;
			length = 1;
		}
	}

	if (length > 0) {
		action = new TextBoxActionDelete (selection_anchor, selection_cursor, buffer, start, length);
		undo->Push (action);
		redo->Clear ();
		
		buffer->Cut (start, length);
		emit |= TEXT_CHANGED;
		anchor = start;
		cursor = start;
		handled = true;
	}
	
	// check to see if selection has changed
	if (selection_anchor != anchor || selection_cursor != cursor) {
		SetSelectionStart (MIN (anchor, cursor));
		SetSelectionLength (abs (cursor - anchor));
		selection_anchor = anchor;
		selection_cursor = cursor;
		emit |= SELECTION_CHANGED;
		handled = true;
	}
	
	return handled;
#endif
}

bool
RichTextBox::KeyPressDelete (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressPageDown (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressPageUp (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressDown (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressUp (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressHome (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressEnd (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressRight (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressLeft (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressUnichar (gunichar c)
{
	return false;
}

void
RichTextBox::BatchPush ()
{
	batch++;
}

void
RichTextBox::BatchPop ()
{
	if (batch == 0) {
		g_warning ("RichTextBox batch underflow");
		return;
	}
	
	batch--;
}

void
RichTextBox::EmitSelectionChanged ()
{
	EmitAsync (RichTextBox::SelectionChangedEvent, MoonUnmanagedFactory::CreateRoutedEventArgs ());
}

void
RichTextBox::EmitContentChanged ()
{
	EmitAsync (RichTextBox::ContentChangedEvent, new ContentChangedEventArgs ());
}

void
RichTextBox::SyncSelection ()
{
	// FIXME: sync the TextSelection
}

void
RichTextBox::SyncContent ()
{
	// FIXME: generate the proper xaml
	char *xaml = NULL;
	
	SetValue (RichTextBox::XamlProperty, Value (xaml, true));
}

void
RichTextBox::SyncAndEmit (bool sync_text)
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
RichTextBox::Paste (MoonClipboard *clipboard, const char *str)
{
	// FIXME: ::Commit has some validation of the text selection.
	// we should probably as well.

	ResetIMContext ();

	RichTextBoxActionSetSelectionText *action = new RichTextBoxActionSetSelectionText (this, selection, str);

	action->Do ();

	undo->Push (action);
}

void
RichTextBox::paste (MoonClipboard *clipboard, const char *text, gpointer closure)
{
	((RichTextBox *) closure)->Paste (clipboard, text);
}

void
RichTextBox::OnKeyDown (KeyEventArgs *args)
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
		if (GetIsReadOnly())
			break;
		
		handled = KeyPressBackSpace (modifiers);
		break;
	case KeyDELETE:
		if (GetIsReadOnly())
			break;
		
		if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == SHIFT_MASK) {
			// Shift+Delete => Cut
			if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
				// FIXME: what about cutting xaml?  do
				// we need to store both xaml and text
				// fragments in the clipboard and
				// determine which to use on paste?
				if (!selection->IsEmpty ()) {
					char *selection_text = selection->GetText ();
					clipboard->SetText (selection_text);
					g_free (selection_text);
				}
			}

			// clear the selection
			RichTextBoxActionSetSelectionText *action = new RichTextBoxActionSetSelectionText(this, selection, "");
			action->Do ();
			undo->Push (action);

			handled = true;
		} else {
			handled = KeyPressDelete (modifiers);
		}
		break;
	case KeyINSERT:
		if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == SHIFT_MASK) {
			// Shift+Insert => Paste
			if (GetIsReadOnly())
				break;
			
			if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
				// paste clipboard contents to the buffer
				clipboard->AsyncGetText (RichTextBox::paste, this);
			}
			
			handled = true;
		} else if ((modifiers & (CONTROL_MASK | ALT_MASK | SHIFT_MASK)) == CONTROL_MASK) {
			// Control+Insert => Copy
			if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
				// FIXME: what about cutting xaml?  do
				// we need to store both xaml and text
				// fragments in the clipboard and
				// determine which to use on paste?
				if (!selection->IsEmpty ()) {
					char *selection_text = selection->GetText ();
					clipboard->SetText (selection_text);
					g_free (selection_text);
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
					// FIXME: what about cutting xaml?  do
					// we need to store both xaml and text
					// fragments in the clipboard and
					// determine which to use on paste?
					if (!selection->IsEmpty ()) {
						char *selection_text = selection->GetText ();
						clipboard->SetText (selection_text);
						g_free (selection_text);
					}
				}				
				handled = true;
				break;
			case KeyX: {
				// Ctrl+X => Cut
				if (GetIsReadOnly())
					break;
				
				if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
					// FIXME: what about cutting xaml?  do
					// we need to store both xaml and text
					// fragments in the clipboard and
					// determine which to use on paste?
					if (!selection->IsEmpty ()) {
						char *selection_text = selection->GetText ();
						clipboard->SetText (selection_text);
						g_free (selection_text);
					}
				}

				// clear the selection
				RichTextBoxActionSetSelectionText *action = new RichTextBoxActionSetSelectionText(this, selection, "");
				action->Do ();
				undo->Push (action);

				handled = true;
				break;
			}
			case KeyV:
				// Ctrl+V => Paste
				if (GetIsReadOnly())
					break;
				
				if ((clipboard = GetClipboard (this, MoonClipboard_Clipboard))) {
					// paste clipboard contents to the buffer
					clipboard->AsyncGetText (RichTextBox::paste, this);
				}
				
				handled = true;
				break;
			case KeyY:
				// Ctrl+Y => Redo
				if (!GetIsReadOnly()) {
					handled = true;
					Redo ();
				}
				break;
			case KeyZ:
				// Ctrl+Z => Undo
				if (!GetIsReadOnly()) {
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
RichTextBox::PostOnKeyDown (KeyEventArgs *args)
{
	MoonKeyEvent *event = args->GetEvent ();
	int key = event->GetSilverlightKey ();
	gunichar c;
	
	// Note: we don't set Handled=true because anything we handle here, we
	// want to bubble up.
	if (!GetIsReadOnly() && im_ctx->FilterKeyPress (event)) {
		need_im_reset = true;
		return;
	}
	
	if (GetIsReadOnly() || event->IsModifier ())
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
RichTextBox::OnKeyUp (KeyEventArgs *args)
{
	if (!GetIsReadOnly()) {
		if (im_ctx->FilterKeyPress (args->GetEvent()))
			need_im_reset = true;
	}
}

bool
RichTextBox::DeleteSurrounding (int offset, int n_chars)
{
	// FIXME: implement this
	return true;
}

gboolean
RichTextBox::delete_surrounding (MoonIMContext *context, int offset, int n_chars, gpointer user_data)
{
	return ((RichTextBox *) user_data)->DeleteSurrounding (offset, n_chars);
}

bool
RichTextBox::RetrieveSurrounding ()
{
	// FIXME: implement this
	//im_ctx->SetSurroundingText (text, -1, cursor - text);
	
	return true;
}

gboolean
RichTextBox::retrieve_surrounding (MoonIMContext *context, gpointer user_data)
{
	return ((RichTextBox *) user_data)->RetrieveSurrounding ();
}

void
RichTextBox::Commit (const char *str)
{
	TextSelection *selection = GetSelection();
	if (selection->GetStart()->GetParent() == NULL)
		SelectAll();

	RichTextBoxActionSetSelectionText *action = new RichTextBoxActionSetSelectionText(this, selection, str);

	action->Do ();

	undo->Push (action);
}

void
RichTextBox::commit (MoonIMContext *context, const char *str, gpointer user_data)
{
	((RichTextBox *) user_data)->Commit (str);
}

void
RichTextBox::ApplyFormattingToSelection (TextSelection *selection, DependencyProperty *prop, Value *value)
{
	RichTextBoxActionApplyFormatting *action = new RichTextBoxActionApplyFormatting (this, selection, prop, value);

	action->Do ();

	undo->Push (action);
}

void
RichTextBox::ResetIMContext ()
{
	if (need_im_reset) {
		im_ctx->Reset ();
		need_im_reset = false;
	}
}

void
RichTextBox::OnMouseLeftButtonDown (MouseButtonEventArgs *args)
{
	double x, y;
	TextPointer cursor;
	
	args->SetHandled (true);
	Focus ();
	
	if (view) {
		args->GetPosition (view, &x, &y);
		
		cursor = view->GetLocationFromXY (x, y);
		
		ResetIMContext ();
		
		// Single-Click: cursor placement
		captured = CaptureMouse ();
		selecting = true;
		
		BatchPush ();
		emit = NOTHING_CHANGED;
		printf ("> mouse left button down\n");
		GetSelection()->Select (&cursor, &cursor);
		printf ("< mouse left button down\n");
		BatchPop ();
		
		SyncAndEmit ();
	}
}

void
RichTextBox::OnMouseLeftButtonMultiClick (MouseButtonEventArgs *args)
{
	TextPointer cursor, start, end;
	double x, y;
	
	args->SetHandled (true);
	
	if (view) {
		args->GetPosition (view, &x, &y);
		
		cursor = view->GetLocationFromXY (x, y);
		
		ResetIMContext ();
		
		switch (((MoonButtonEvent*)args->GetEvent())->GetNumberOfClicks ()) {
		case 2:
			// Double-Click: select the word
			if (captured)
				ReleaseMouseCapture ();
			start = CursorPrevWord (cursor);
			end = CursorNextWord (cursor);
			selecting = false;
			captured = false;
			break;
		case 3:
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
			break;
		}
		
		BatchPush ();
		emit = NOTHING_CHANGED;
		GetSelection()->Select (&start, &end);
		BatchPop ();
		
		SyncAndEmit ();
	}
}

void
RichTextBox::mouse_left_button_multi_click (EventObject *sender, EventArgs *args, gpointer closure)
{
	((RichTextBox *) closure)->OnMouseLeftButtonMultiClick ((MouseButtonEventArgs *) args);
}

void
RichTextBox::OnMouseLeftButtonUp (MouseButtonEventArgs *args)
{
	if (captured)
		ReleaseMouseCapture ();
	
	args->SetHandled (true);
	selecting = false;
	captured = false;
}

void
RichTextBox::OnMouseMove (MouseEventArgs *args)
{
	if (selecting) {
		MoonClipboard *clipboard;
		double x, y;

		args->GetPosition (view, &x, &y);
		args->SetHandled (true);


		BatchPush ();
		emit = NOTHING_CHANGED;

		TextPointer *start = selection->GetStart();
		TextPointer cursor = view->GetLocationFromXY (x, y);

		selection->Select (start, &cursor);

		delete start;

		BatchPop ();

		SyncAndEmit ();

		
		if ((clipboard = GetClipboard (this, MoonClipboard_Primary))) {
			// copy the selection to the primary clipboard
			char *selection_text = selection->GetText ();
			clipboard->SetText (selection_text);
			g_free (selection_text);
		}
	}
}

void
RichTextBox::OnLostFocus (RoutedEventArgs *args)
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
	
	if (!GetIsReadOnly()) {
		im_ctx->FocusOut ();
		need_im_reset = true;
	}
}

void
RichTextBox::OnGotFocus (RoutedEventArgs *args)
{
	focused = true;
	
	if (view)
		view->OnGotFocus ();
	
	if (!GetIsReadOnly()) {
		im_ctx->FocusIn ();
		need_im_reset = true;
	}
}

void
RichTextBox::EmitCursorPositionChanged (double height, double x, double y)
{
	//Emit (RichTextBox::CursorPositionChangedEvent, new CursorPositionChangedEventArgs (height, x, y));
}

void
RichTextBox::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	RichTextBoxModelChangeType changed = RichTextBoxModelChangedNothing;
	DependencyProperty *prop;
	
	if (args->GetProperty ()->GetOwnerType () != Type::RICHTEXTBOX) {
		Control::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == RichTextBox::AcceptsReturnProperty) {
		// update accepts_return state
		accepts_return = args->GetNewValue ()->AsBool ();
	} else if (args->GetId () == RichTextBox::CaretBrushProperty) {
		// Note: if we wanted to be perfect, we could invalidate the
		// blinking cursor rect if it is active... but is it that
		// really important? I don't think so...
	} else if (args->GetId () == RichTextBox::IsReadOnlyProperty) {
		if (focused) {
			if (GetIsReadOnly()) {
				ResetIMContext ();
				im_ctx->FocusOut ();
			} else {
				im_ctx->FocusIn ();
			}
		}
		
		if (view)
			view->SetEnableCursor (!GetIsReadOnly());
	} else if (args->GetId () == RichTextBox::TextAlignmentProperty) {
		changed = RichTextBoxModelChangedTextAlignment;
	} else if (args->GetId () == RichTextBox::TextWrappingProperty) {
		if (contentElement) {
			if ((prop = contentElement->GetDependencyProperty ("HorizontalScrollBarVisibility"))) {
				// If TextWrapping is set to Wrap, disable the horizontal scroll bars
				if (args->GetNewValue ()->AsTextWrapping () == TextWrappingWrap)
					contentElement->SetValue (prop, Value (ScrollBarVisibilityDisabled, Type::SCROLLBARVISIBILITY));
				else
					contentElement->SetValue (prop, GetValue (RichTextBox::HorizontalScrollBarVisibilityProperty));
			}
		}
		
		changed = RichTextBoxModelChangedTextWrapping;
	} else if (args->GetId () == RichTextBox::HorizontalScrollBarVisibilityProperty) {
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
	} else if (args->GetId () == RichTextBox::VerticalScrollBarVisibilityProperty) {
		// XXX more crap because these aren't templatebound.
		if (contentElement) {
			if ((prop = contentElement->GetDependencyProperty ("VerticalScrollBarVisibility")))
				contentElement->SetValue (prop, args->GetNewValue ());
		}
	} else if (args->GetId () == RichTextBox::XamlProperty) {
		// FIXME: need to sync the XAML to Blocks
		char *xaml = args->GetNewValue()->AsString();
		Type::Kind element_type;
		MoonError error;
		Value *objv = NULL;

		if (xaml) {
			printf ("setting xaml to %s\n", xaml);

			SL4XamlLoader *loader = new SL4XamlLoader (GetDeployment()->GetSurface()); // XXX we're leaking this
			objv = loader->CreateFromStringWithError (xaml, true, &element_type, 0, &error, this);
			if (element_type != Type::SECTION) {
				g_warning ("awww, crap");
				return;
			}
		}

		// FIXME: we need to figure out if this is done before
		// the xaml loader (i.e. if you try to set RTB.Xaml to
		// something invalid, does it first clear out the
		// existing blocks?  right now our code won't)
		GetBlocks()->Clear();

		if (objv) {
			Section *s = objv->AsSection();
			BlockCollection *sblocks = s->GetBlocks();
			int count = sblocks->GetCount();
			for (int i = 0; i < count; i ++) {
				Block *b = sblocks->GetValueAt (0)->AsBlock();
				sblocks->RemoveAt (0);
				GetBlocks()->Add (Value(b));
			}
		}
	}
	
	if (changed != RichTextBoxModelChangedNothing && view)
		view->OnModelChanged (changed, args);
	
	NotifyListenersOfPropertyChange (args, error);
}

void
RichTextBox::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop->GetOwnerType () != Type::RICHTEXTBOX)
		Control::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
RichTextBox::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	// FIXME: need to sync the Blocks to XAML
	Control::OnCollectionChanged (col, args);
}

void
RichTextBox::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (!PropertyHasValueNoAutoCreate (RichTextBox::BlocksProperty, col)) {
		Control::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	// FIXME: if we could pass the obj to the View via the Args, the View
	// could probably optimize the invalidated region to minimize redraw
	if (view)
		view->OnModelChanged (RichTextBoxModelChangedContent, args);
}

void
RichTextBox::OnApplyTemplate ()
{
	DependencyProperty *prop;
	
	contentElement = GetTemplateChild ("ContentElement");
	
	if (contentElement == NULL) {
		g_warning ("RichTextBox::OnApplyTemplate: no ContentElement found");
		Control::OnApplyTemplate ();
		return;
	}
	
	// XXX LAME these should be template bindings in the textbox template.
	if ((prop = contentElement->GetDependencyProperty ("VerticalScrollBarVisibility")))
		contentElement->SetValue (prop, GetValue (RichTextBox::VerticalScrollBarVisibilityProperty));
	
	if ((prop = contentElement->GetDependencyProperty ("HorizontalScrollBarVisibility"))) {
		// If TextWrapping is set to Wrap, disable the horizontal scroll bars
		if (GetTextWrapping () == TextWrappingWrap)
			contentElement->SetValue (prop, Value (ScrollBarVisibilityDisabled, Type::SCROLLBARVISIBILITY));
		else
			contentElement->SetValue (prop, GetValue (RichTextBox::HorizontalScrollBarVisibilityProperty));
	}
	
	if (view) {
		view->SetTextBox (NULL);
	}

	// Create our view control
	view = MoonUnmanagedFactory::CreateRichTextBoxView ();
	view->unref ();
	
	view->SetEnableCursor (!GetIsReadOnly());
	view->SetTextBox (this);
	
	// Insert our RichTextBoxView
	if (contentElement->Is (Type::CONTENTPRESENTER)) {
		ContentPresenter *presenter = (ContentPresenter *) contentElement;
		
		presenter->SetValue (ContentPresenter::ContentProperty, Value (view));
	} else if (contentElement->Is (Type::CONTENTCONTROL)) {
		ContentControl *control = (ContentControl *) contentElement;
		
		control->SetValue (ContentControl::ContentProperty, Value (view));
	} else if (contentElement->Is (Type::BORDER)) {
		Border *border = (Border *) contentElement;
		
		border->SetValue (Border::ChildProperty, Value (view));
	} else if (contentElement->Is (Type::PANEL)) {
		DependencyObjectCollection *children = ((Panel *) contentElement)->GetChildren ();
		
		children->Add ((RichTextBoxView *) view);
	} else {
		g_warning ("RichTextBox::OnApplyTemplate: don't know how to handle a ContentElement of type %s",
			   contentElement->GetType ()->GetName ());
		view->SetTextBox (NULL);
		view = NULL;
	}
	
	Control::OnApplyTemplate ();
}

DependencyObject* 
RichTextBox::Split (int loc)
{
	return NULL;
}

IDocumentNode*
RichTextBox::GetParentDocumentNode ()
{
	return NULL;
}

DependencyObjectCollection*
RichTextBox::GetDocumentChildren ()
{
	return GetBlocks();
}

void
RichTextBox::SerializeXaml (GString *str)
{
	int c = GetBlocks()->GetCount();

	if (c == 0)
		return;

	const char *header = "<Section xml:space=\"preserve\" HasTrailingParagraphBreakOnPaste=\"False\" xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\">";
	const char *trailer = "</Section>";

	g_string_append (str, header);
	for (int i = 0; i < c; i ++) {
		Block *b = GetBlocks()->GetValueAt(i)->AsBlock();
		b->SerializeXaml(str);
	}
	g_string_append (str, trailer);
}

void
RichTextBox::SerializeXamlProperties (bool force, GString *str)
{
}


TextPointer*
RichTextBox::GetPositionFromPoint (Point point)
{
	return new TextPointer (view->GetLocationFromXY (point.x, point.y));
}

TextPointer*
RichTextBox::GetContentStart ()
{
	return new TextPointer (this, 0, LogicalDirectionBackward);
}

TextPointer*
RichTextBox::GetContentEnd ()
{
	return new TextPointer (this, -1, LogicalDirectionForward);
}

static TextElement*
FindRun (TextElement *el, bool from_start)
{
	if (el->Is(Type::RUN))
		return el;

	IDocumentNode *node = IDocumentNode::CastToIDocumentNode (el);
	DependencyObjectCollection *col = node->GetDocumentChildren ();
	int count = col ? col->GetCount () : 0;
	if (!col || !count)
		return el;

	for (int i = 0; i < count; i ++) {
		TextElement *child_el = FindRun (col->GetValueAt(from_start ? i : count - 1 - i)->AsTextElement(), from_start);
		if (child_el->Is (Type::RUN))
			return child_el;
	}

	// there wasn't a run child, we return our first child
	return col->GetValueAt(from_start ? 0 : count - 1)->AsTextElement();
}

void
RichTextBox::SelectAll ()
{
	TextPointer *start, *end;

	BlockCollection *col = GetBlocks();
	if (col->GetCount() == 0) {
		start = GetContentStart ();
		end = GetContentEnd ();
	}
	else {
		Paragraph *start_p = col->GetValueAt(0)->AsParagraph ();
		Paragraph *end_p = col->GetValueAt(col->GetCount() - 1)->AsParagraph ();

		TextElement *start_e = FindRun (start_p, true);
		TextElement *end_e = FindRun (end_p, false);

		start = start_e->GetContentStart();
		end = end_e->GetContentEnd();
	}

	GetSelection()->Select (start, end);

	delete start;
	delete end;
}

TextSelection *
RichTextBox::GetSelection ()
{
	if (!selection) {
		TextPointer *start = GetContentStart();
		selection = new TextSelection ();
		selection->Select (start, start);
		delete start;
	}
		
	return selection;
}
	
bool
RichTextBox::CanUndo ()
{
	return !undo->IsEmpty ();
}

bool
RichTextBox::CanRedo ()
{
	return !redo->IsEmpty ();
}

void
RichTextBox::Undo ()
{
	//RichTextBoxActionReplace *replace;
	//RichTextBoxActionInsert *insert;
	//RichTextBoxActionDelete *dele;
	RichTextBoxAction *action;

	if (undo->IsEmpty ())
		return;
	
	action = (RichTextBoxAction*)undo->Pop ();
	redo->Push (action);
	action->Undo ();
	
	BatchPush ();
	//SetSelectionStart (MIN (anchor, cursor));
	//SetSelectionLength (abs (cursor - anchor));
	emit = CONTENT_CHANGED | SELECTION_CHANGED;
#if notyet
	selection_anchor = anchor;
	selection_cursor = cursor;
#endif
	BatchPop ();
	
	SyncAndEmit ();
}

void
RichTextBox::Redo ()
{
	//RichTextBoxActionReplace *replace;
	//RichTextBoxActionInsert *insert;
	//RichTextBoxActionDelete *dele;
	RichTextBoxAction *action;

	if (redo->IsEmpty ())
		return;
	
	action = (RichTextBoxAction*)redo->Pop ();
	undo->Push (action);

	action->Do ();
	
	BatchPush ();
	//SetSelectionStart (MIN (anchor, cursor));
	//SetSelectionLength (abs (cursor - anchor));
	emit = CONTENT_CHANGED | SELECTION_CHANGED;
#if notyet
	selection_anchor = anchor;
	selection_cursor = cursor;
#endif
	BatchPop ();
	
	SyncAndEmit ();
}

Rect
RichTextBox::GetCharacterRect (TextPointer *tp, LogicalDirection direction)
{
	return view ? view->GetCharacterRect (tp, direction) : Rect::ManagedEmpty;
}


//
// RichTextBoxView
//

#define CURSOR_BLINK_ON_MULTIPLIER    2
#define CURSOR_BLINK_OFF_MULTIPLIER   1
#define CURSOR_BLINK_DELAY_MULTIPLIER 3
#define CURSOR_BLINK_DIVIDER          3

RichTextBoxView::RichTextBoxView ()
	: FrameworkElement (Type::RICHTEXTBOXVIEW)
{
	AddHandler (UIElement::MouseLeftButtonDownEvent, RichTextBoxView::mouse_left_button_down, this);
	AddHandler (UIElement::MouseLeftButtonUpEvent, RichTextBoxView::mouse_left_button_up, this);

	SetCursor (CursorTypeIBeam);
	
	cursor = Rect (0, 0, -1, -1);
	layout = new RichTextLayout ();
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
	if (!GetDeployment ()->IsShuttingDown ())
		DisconnectBlinkTimeout ();
	
	delete layout;
}

RichTextLayoutLine *
RichTextBoxView::GetLineFromY (double y, int *index)
{
	return layout->GetLineFromY (Point (), y, index);
}

RichTextLayoutLine *
RichTextBoxView::GetLineFromIndex (int index)
{
	return layout->GetLineFromIndex (index);
}

TextPointer
RichTextBoxView::GetLocationFromXY (double x, double y)
{
	return layout->GetLocationFromXY (Point (), x, y);
}


Rect
RichTextBoxView::GetCharacterRect (TextPointer *tp, LogicalDirection direction)
{
	return layout->GetCharacterRect (tp, direction);
}

bool
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
	
	return Runtime::GetWindowingSystem ()->GetCursorBlinkTimeout (window);
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
	TextPointer* cur = textbox->GetSelection()->GetStart();

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
       	if (layout->SetBlocks (textbox->GetBlocks())) {
		layout->ResetState ();
		GetChildren()->Clear(); // FIXME: pretty sure we need to ElementRemoved for all the children here.
	}
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
RichTextBoxView::MeasureOverrideWithError (Size availableSize, MoonError *error)
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
RichTextBoxView::ArrangeOverrideWithError (Size finalSize, MoonError *error)
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
	if (layout->SetMaxWidth (constraint.width)) {
		layout->ResetState();
		GetChildren()->Clear(); // FIXME: pretty sure we need to ElementRemoved for all the children here.
	}
	
	layout->Layout (this);
	dirty = false;
}

double
RichTextBoxView::GetBaselineOffset ()
{
	MoonError error;
	GeneralTransform *from_view_to_rtb = GetTransformToUIElementWithError (textbox, &error);

	Point p = from_view_to_rtb->Transform (Point (0,0));

	from_view_to_rtb->unref ();

	return layout->GetBaselineOffset () + p.y;
}

void
RichTextBoxView::Paint (cairo_t *cr)
{
	cairo_save (cr);

	if (GetFlowDirection () == FlowDirectionRightToLeft) {
		Rect bbox = layout->GetRenderExtents ();
		cairo_translate (cr, bbox.width, 0.0);
		cairo_scale (cr, -1.0, 1.0);
	}
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
	cairo_restore (cr);
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
	
	if (!path_only)
		RenderLayoutClip (cr);
	
	layout->SetAvailableWidth (renderSize.width);
	Paint (cr);
	cairo_restore (cr);
}

void
RichTextBoxView::OnModelChanged (RichTextBoxModelChangeType change, PropertyChangedEventArgs *args)
{
	switch (change) {
	case RichTextBoxModelChangedTextAlignment:
		// text alignment changed, update our layout
		if (layout->SetTextAlignment (args->GetNewValue()->AsTextAlignment ()))
			dirty = true;
		break;
	case RichTextBoxModelChangedTextWrapping:
		// text wrapping changed, update our layout
		if (layout->SetTextWrapping (args->GetNewValue()->AsTextWrapping ()))
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
	case RichTextBoxModelChangedIsReadOnly: {
		UIElementCollection *col = GetChildren();
		int count = col->GetCount();
		for (int i = 0; i < count; i ++) {
			UIElement *el = col->GetValueAt (i)->AsUIElement();
			if (el->Is (Type::CONTROL)) {
				((Control*)el)->SetIsEnabled (!args->GetNewValue()->AsBool());
			}
		}
		break;
	}
	default:
		// nothing changed??
		return;
	}
	
	if (dirty) {
		layout->ResetState ();
		GetChildren()->Clear(); // FIXME: pretty sure we need to ElementRemoved for all the children here.
		InvalidateMeasure ();
		UpdateBounds (true);
	}
	
	Invalidate ();
}

void
RichTextBoxView::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (PropertyHasValueNoAutoCreate (RichTextBoxView::ChildrenProperty, col)) {
		switch (args->GetChangedAction()) {
		case CollectionChangedActionReplace:
			ElementRemoved (args->GetOldItem()->AsUIElement ());
			// now fall thru to Add
		case CollectionChangedActionAdd:
			ElementAdded (args->GetNewItem()->AsUIElement ());
			break;
		case CollectionChangedActionRemove:
			ElementRemoved (args->GetOldItem()->AsUIElement ());
			break;
		case CollectionChangedActionClearing: {
			int children_count = col->GetCount ();
			for (int i = 0; i < children_count; i++) {
				UIElement *ui = col->GetValueAt (i)->AsUIElement ();
				ElementRemoved (ui);
			}
			break;
		}
		case CollectionChangedActionCleared:
			// nothing needed here.
			break;
		}
	}
	else {
		FrameworkElement::OnCollectionChanged (col, args);
	}
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
RichTextBoxView::SetTextBox (RichTextBox *textbox)
{
	if (this->textbox == textbox)
		return;
	
	this->textbox = textbox;
	
	if (textbox) {
		layout->SetTextAlignment (textbox->GetTextAlignment ());
		layout->SetTextWrapping (textbox->GetTextWrapping ());

		had_selected_text = textbox->HasSelectedText ();
		selection_changed = true;
		UpdateText ();
	} else {
		layout->SetBlocks (NULL);
	}
	
	UpdateBounds (true);
	InvalidateMeasure ();
	Invalidate ();
	dirty = true;
}

void
RichTextBoxView::DocumentPropertyChanged (TextElement *onElement, PropertyChangedEventArgs *args)
{
	// for now just relayout the entire RTB.  in the future, bump
	// up to onElement's parent Paragraph and just relayout that
	// paragraph.

	layout->ResetState();
	GetChildren()->Clear(); // FIXME: pretty sure we need to ElementRemoved for all the children here.
	InvalidateMeasure ();
	UpdateBounds (true);
}

void
RichTextBoxView::DocumentCollectionChanged (TextElement *onElement, Collection *col, CollectionChangedEventArgs *args)
{
	// for now just relayout the entire RTB.  in the future, bump
	// up to onElement's parent Paragraph and just relayout that
	// paragraph.

	layout->ResetState();
	GetChildren()->Clear(); // FIXME: pretty sure we need to ElementRemoved for all the children here.
	InvalidateMeasure ();
	UpdateBounds (true);
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

Value *
RichTextBoxView::CreateChildren (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj)
{
	UIElementCollection *col = new UIElementCollection (false);
	col->EnsureManagedPeer ();
	if (forObj)
		((RichTextBoxView*)forObj)->SetSubtreeObject (col);
	return Value::CreateUnrefPtr (col);
}

};
