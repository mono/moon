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

extern "C" void dump_rtb_text (RichTextBox *rtb)
{
	GString *str = g_string_new("");

	rtb->SerializeXaml (str);

	printf ("%s\n", str->str);

	g_string_free (str, TRUE);
}

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

	enum RichTextBoxActionType {
		APPLY_FORMATTING,
		SET_SELECTION_TEXT,
		BACKSPACE
	};

	RichTextBoxActionType GetType () { return type; }

protected:
	RichTextBoxAction (RichTextBox *rtb, RichTextBoxActionType type) : rtb(rtb), type(type) { }

	RichTextBox *rtb;
	RichTextBoxActionType type;
};

class RichTextBoxActionBackspace : public RichTextBoxAction {
public:
	RichTextBoxActionBackspace (RichTextBox* rtb, TextSelection *selection)
		: RichTextBoxAction (rtb, RichTextBoxAction::BACKSPACE)
	{
		pointer = selection->GetStart();
		new_pointer = NULL;

		old_run_text = NULL;
	}

	virtual ~RichTextBoxActionBackspace ()
	{
		delete pointer;
		delete new_pointer;

		g_free (old_run_text);
	}

	virtual void Do ();
	virtual void Undo ();

private:
	TextPointer *pointer;
	TextPointer *new_pointer;

	enum BackspaceType {
		ENTIRELY_IN_RUN,
		SIBLING_RUNS
	};


	BackspaceType backspace_type;

	char *old_run_text;

	void RemoveCharacterFromRun (Run *run, int loc);
};

static Run*
FindPreviousRun (Run *r)
{
	DocumentWalker walker (IDocumentNode::CastToIDocumentNode (r), DocumentWalker::Backward);

	while (true) {
		IDocumentNode *node;
		DocumentWalker::StepType step = walker.Step (&node);

		switch (step) {
		case DocumentWalker::Enter:
			if (node->AsDependencyObject ()->Is(Type::RUN))
				return ((Run*)node->AsDependencyObject());
			break;
		case DocumentWalker::Leave:
			break;
		case DocumentWalker::Done:
			return NULL;
		}
	}
}

void
RichTextBoxActionBackspace::RemoveCharacterFromRun (Run *run, int loc)
{
	old_run_text = g_strdup (run->GetText());

	char *new_text = g_strdup (old_run_text);

	char *p = g_utf8_prev_char (new_text + loc);
	int length_of_char = new_text + loc - p;

	memmove (p, new_text + loc, strlen (new_text + loc));

	new_text[strlen (new_text) - length_of_char] = 0;

	run->SetText (new_text);
}

void
RichTextBoxActionBackspace::Do ()
{
	TextSelection* selection = rtb->GetSelection();

	if (pointer->GetParent() == NULL)
		return;

	if (pointer->GetParent()->Is (Type::RUN)) {
		Run *run = (Run*)pointer->GetParent();
		int loc = pointer->ResolveLocation();

		if (loc > 0) {
			// we can handle it entirely within this run.
			// easiest case.

			RemoveCharacterFromRun (run, loc);

			new_pointer = pointer->GetPositionAtOffset (-1, LogicalDirectionForward);
			selection->Select (new_pointer, new_pointer);
		}
		else {
			Run *prev_run = FindPreviousRun (run);

			// if there's no previous run, we do nothing,
			// since we're already at the beginning of the
			// document.
			if (!prev_run)
				return;

			// we need to delete the first character in
			// the previous run, pulling this run along
			// with us as we go.  this has a couple of
			// implications:

			// if @run and @run-before-this-run are in the
			// same parent container, we just transfer the
			// pointer to the new run and delete the last
			// character there.

			// if @run and @run-before-this-run are in
			// separate parent containers, we need to
			// remove all the intervening parents (FIXME:
			// and save them in this action?) and transfer
			// @run and all its siblings into the other
			// container.

			if (prev_run->GetParent()->GetParent() == run->GetParent()->GetParent()) {
				// the runs share a parent, so we just
				// move the selection to the
				// previous_run and delete a character
				// from there.

				TextPointer content_end = prev_run->GetContentEnd_np();
				
				RemoveCharacterFromRun (prev_run, content_end.ResolveLocation ());

				new_pointer = content_end.GetPositionAtOffset (-1, LogicalDirectionForward);
				selection->Select (new_pointer, new_pointer);
			}
			else {
				printf ("NIEX: RichTextBoxActionBackspace::Do with run/prev-run not in the same container\n");
			}
		}
	}
	else {
		printf ("NIEX: RichTextBoxActionBackspace::Do with textpointer in a %s element\n", pointer->GetParent()->GetTypeName());
	}
	
}

void
RichTextBoxActionBackspace::Undo ()
{
	TextSelection* selection = rtb->GetSelection();

	switch (backspace_type) {
	case RichTextBoxActionBackspace::ENTIRELY_IN_RUN:
		((Run*)pointer->GetParent())->SetText (old_run_text);
		selection->Select (pointer, pointer);
		break;
	case RichTextBoxActionBackspace::SIBLING_RUNS:
		break;
	}
}

class RichTextBoxActionApplyFormatting : public RichTextBoxAction {
public:
	RichTextBoxActionApplyFormatting (RichTextBox* rtb, TextSelection* selection, DependencyProperty* prop, Value* v)
		: RichTextBoxAction (rtb, RichTextBoxAction::APPLY_FORMATTING)
	{
		start = selection->GetStart_np();
		end = selection->GetEnd_np();
		
		this->prop = prop;
		this->v = *v;
	}

	virtual ~RichTextBoxActionApplyFormatting ()
	{
	}

	virtual void Do ();

	virtual void Undo ()
	{
		printf ("NIEX: RichTextBoxActionApplyFormatting::Undo\n");
	}

private:
	DependencyProperty* prop;
	Value v;

	TextPointer start;
	TextPointer end;
};

void
RichTextBoxActionApplyFormatting::Do ()
{
	TextSelection* selection = rtb->GetSelection();

	TextPointer s = start;
	TextPointer e = end;

	if (!s.GetParent()->Is (Type::RUN)) {
		// walk the document forward from s.GetParent() until we hit a run
		DocumentWalker walker (s.GetParentNode (), DocumentWalker::Forward);

		while (true) {
			IDocumentNode *node;
			DependencyObject *obj;
			DocumentWalker::StepType step = walker.Step(&node);

			switch (step) {
			case DocumentWalker::Enter:
				obj = node->AsDependencyObject();
				if (obj->Is (Type::RUN)) {
					s = ((TextElement*)obj)->GetContentStart_np();
				}
				break;
			case DocumentWalker::Leave:
				// nothing to do on leave
				break;
			case DocumentWalker::Done:
				// we didn't find a run, bail early
				g_warning ("Didn't find a Run to apply formatting changes to, bailing");
				return;
			}
		}
	}

	if (!e.GetParent()->Is (Type::RUN)) {
		// walk the document backward from e.GetParent() until we hit a run
		DocumentWalker walker (e.GetParentNode (), DocumentWalker::Backward);

		while (true) {
			IDocumentNode *node;
			DependencyObject *obj;
			DocumentWalker::StepType step = walker.Step(&node);

			switch (step) {
			case DocumentWalker::Enter:
				obj = node->AsDependencyObject();
				if (obj->Is (Type::RUN)) {
					e = ((TextElement*)obj)->GetContentEnd_np();
				}
				break;
			case DocumentWalker::Leave:
				// nothing to do on leave
				break;
			case DocumentWalker::Done:
				// we didn't find a run, bail early
				g_warning ("Didn't find a Run to apply formatting changes to, bailing");
				return;
			}
		}

	}

	if (s.Equal (e)) // selection is empty for whatever reason
		return;

	if (s.GetParent() == e.GetParent()) {
		// easy, the selection starts and ends in the same element.

		if (s.GetLocation() == CONTENT_START && e.GetLocation () == CONTENT_END) {
			// it's the entire run, so just change
			// the formatting on it.
			s.GetParent ()->SetValue (prop, &v);
		}
		else {
			int loc1 = s.ResolveLocation ();
			int loc2 = e.ResolveLocation ();

			if (loc1 == loc2)
				return;
			else if (loc1 > loc2) {
				// swap them so loc1 < loc2
				int tmp = loc2;
				loc2 = loc1;
				loc1 = tmp;
			}

			DependencyObjectCollection *parents_children = s.GetParentNode()->GetParentDocumentNode()->GetDocumentChildren();
			int index_in_parent = parents_children->IndexOf (s.GetParent());

			// we split at loc2 first since
			// splitting at loc1 would modify what
			// loc2 is
			Run *trailing_run = (Run*)s.GetParentNode()->Split (loc2);
			Run *formatted_run = (Run*)s.GetParentNode()->Split (loc1);

			if (trailing_run)
				parents_children->Insert (index_in_parent+1, trailing_run);
			if (formatted_run) {
				formatted_run->SetValue (prop, &v);
				parents_children->Insert (index_in_parent+1, formatted_run);

				selection->Select (formatted_run->GetContentStart_np(), formatted_run->GetContentEnd_np());
			}
		}
	}
	else {
		Run *run;

		// split the start run and format the section that needs it
		run = (Run*)s.GetParentNode()->Split (s.ResolveLocation ());
		if (run) {
			DependencyObjectCollection *parents_children = e.GetParentNode()->GetParentDocumentNode()->GetDocumentChildren();
			int index_in_parent = parents_children->IndexOf (s.GetParent());
			run->SetValue (prop, &v);
			parents_children->Insert (index_in_parent+1, run);
			s = run->GetContentStart_np();
		}

		// now walk the document until we reach the end parent, apply the formatting to all runs along the way
		DocumentWalker walker (s.GetParentNode(), DocumentWalker::Forward);
		while (true) {
			IDocumentNode *node;
			DocumentWalker::StepType step = walker.Step(&node);

			switch (step) {
			case DocumentWalker::Enter:
				if (e.GetParentNode() == node) {
					goto split_end_node;
				}
				break;
			case DocumentWalker::Leave:
				// nothing to do on leave
				break;
			case DocumentWalker::Done:
				// we didn't run into the end parent?  uhhhh
				g_warning ("badness, we didn't see the end element while iterating the document");
				return;
			}
		}

		// split the end run and format the section that needs it
	split_end_node:
		run = (Run*)e.GetParentNode()->Split (e.ResolveLocation ());
		// in this case the returned run is the text we *don't* want to format
		e.GetParent()->SetValue (prop, &v);
		e = ((Run*)e.GetParent())->GetContentEnd_np();
		if (run) {
			DependencyObjectCollection *parents_children = e.GetParentNode()->GetParentDocumentNode()->GetDocumentChildren();
			int index_in_parent = parents_children->IndexOf (e.GetParent());
			parents_children->Insert (index_in_parent+1, run);
		}

		selection->Select (&s, &e);
	}
}

class RichTextBoxActionSetSelectionText : public RichTextBoxAction {
public:
	RichTextBoxActionSetSelectionText (RichTextBox* rtb, TextSelection* selection, const char* text, bool insert_paragraph = false)
		: RichTextBoxAction (rtb, RichTextBoxAction::SET_SELECTION_TEXT)
	{
		this->insert_paragraph = insert_paragraph;

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

	virtual void Do ();

	virtual void Undo ();

	void AppendText (const char *text_);
private:
	char* text;
	char* previous_text;

	TextPointer* start;
	TextPointer* end;
	bool insert_paragraph; // if true we insert a new paragraph at the beginning of ::Do (before inserting the text)
};

void
RichTextBoxActionSetSelectionText::Do ()
{
	TextSelection *selection = rtb->GetSelection();
	selection->Select (start, end);
	previous_text = selection->GetText ();

	if (insert_paragraph)
		selection->Insert (MoonUnmanagedFactory::CreateParagraph());

	selection->SetText (text);
}

void
RichTextBoxActionSetSelectionText::Undo ()
{
	TextSelection *selection = rtb->GetSelection();
	TextPointer *new_end = selection->GetEnd();

	// we need to remove the currently inserted text, replacing it with the old text
	selection->Select (start, new_end);
	selection->SetText (previous_text);
	selection->Select (start, end);
}

void
RichTextBoxActionSetSelectionText::AppendText (const char *text_)
{
	// this should be valid.  if we're able to append text
	// to this action then the selection should be empty.
	TextSelection *selection = rtb->GetSelection();

	selection->SetText (text_);

	char *new_text = g_strconcat (text, text_, NULL);
	g_free (text);
	text = new_text;
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

	flattened = new ArrayList ();

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
	double y = view->GetCursor ().y;
	double x = GetCursorOffset ();
	RichTextLayoutLine *line;
	int index, n;
	
	if (!(line = view->GetLineFromY (y, &index)))
		return cursor;
	
	if (page) {
		// calculate the number of lines to skip over

		// FIXME: this only worked for TextBox where lines
		// were not different heights.  we can't know how many
		// lines we have to skip by doing this math.
		n = GetActualHeight () / line->size.height;
	} else {
		n = 1;
	}
	
	if (index + n >= view->GetLineCount ()) {
		// go to the end of the last line
		line = view->GetLineFromIndex (view->GetLineCount () - 1);
		
		have_offset = false;
		
		return line->end;
	}
	
	line = view->GetLineFromIndex (index + n);
	
	return line->GetLocationFromX (Point (), x);
}

TextPointer
RichTextBox::CursorUp (const TextPointer& cursor, bool page)
{
	double y = view->GetCursor ().y;
	double x = GetCursorOffset ();
	RichTextLayoutLine *line;
	int index, n;
	
	if (!(line = view->GetLineFromY (y, &index)))
		return cursor;
	
	if (page) {
		// calculate the number of lines to skip over

		// FIXME: this only worked for TextBox where lines
		// were not different heights.  we can't know how many
		// lines we have to skip by doing this math.
		n = GetActualHeight () / line->size.height;
	} else {
		n = 1;
	}
	
	if (index - n < 0) {
		// go to the end of the last line
		line = view->GetLineFromIndex (0);
		
		have_offset = false;
		
		return line->start;
	}
	
	line = view->GetLineFromIndex (index - n);
	
	return line->GetLocationFromX (Point (), x);
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
	double y = view->GetCursor ().y;
	RichTextLayoutLine *line;
	int index;

	if (!(line = view->GetLineFromY (y, &index)))
		return cursor;

	return line->start;
}

TextPointer
RichTextBox::CursorLineEnd (const TextPointer& cursor, bool include)
{
	double y = view->GetCursor ().y;
	RichTextLayoutLine *line;
	int index;

	if (!(line = view->GetLineFromY (y, &index)))
		return cursor;

	return line->start;
}

bool
RichTextBox::KeyPressBackSpace (MoonModifier modifiers)
{
	if ((modifiers & (ALT_MASK | SHIFT_MASK)) != 0)
		return false;

	RichTextBoxAction *action;

	if (selection->IsEmpty ()) {
		// BackSpace w/o active selection: delete just the previous character (and other magic)
		action = new RichTextBoxActionBackspace(this, selection);
	}
	else {
		// BackSpace w/ active selection: delete the selected text
		action = new RichTextBoxActionSetSelectionText(this, selection, "");
	}

	action->Do ();
	undo->Push (action);
	return true;

#if false
	else if ((modifiers & CONTROL_MASK) != 0) {
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
	bool handled = false;

	if ((modifiers & (ALT_MASK | SHIFT_MASK)) != 0)
		return false;

	if (!selection->IsEmpty ()) {
		// Delete w/ active selection: delete the selected text
		RichTextBoxActionSetSelectionText *action = new RichTextBoxActionSetSelectionText(this, selection, "");
		action->Do ();
		undo->Push (action);
		return true;
	}
	
#if 0
	if ((modifiers & CONTROL_MASK) != 0) {
		// Ctrl+Delete: delete the word starting at the cursor
		length = CursorNextWord (cursor) - cursor;
		start = cursor;
	} else if (cursor < buffer->len) {
		// Delete: delete the char after the cursor position
		if (buffer->text[cursor] == '\r' && buffer->text[cursor + 1] == '\n')
			length = 2;
		else
			length = 1;
		
		start = cursor;
	}
	
	if (length > 0) {
		action = new TextBoxUndoActionDelete (selection_anchor, selection_cursor, buffer, start, length);
		undo->Push (action);
		redo->Clear ();
		
		buffer->Cut (start, length);
		emit |= TEXT_CHANGED;
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
	
#endif
	return handled;
}

bool
RichTextBox::KeyPressPageDown (MoonModifier modifiers)
{
	return false;
}

bool
RichTextBox::KeyPressPageUp (MoonModifier modifiers)
{
	TextPointer anchor = selection->GetAnchor_np();
	TextPointer cursor;
	bool handled = false;
	bool have;
	
	if ((modifiers & (CONTROL_MASK | ALT_MASK)) != 0) {
		return false;
	}
	
	// move the cursor down by one line from its current position
	cursor = CursorUp (cursor, false);
	have = have_offset;
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		selection->Select (&cursor, &cursor);
	}

	emit |= SELECTION_CHANGED;
	have_offset = have;
	handled = true;

	return handled;
}

bool
RichTextBox::KeyPressDown (MoonModifier modifiers)
{
	TextPointer anchor = selection->GetAnchor_np();
	TextPointer cursor;
	bool handled = false;
	bool have;
	
	if ((modifiers & (CONTROL_MASK | ALT_MASK)) != 0) {
		return false;
	}
	
	// move the cursor down by one line from its current position
	cursor = CursorDown (cursor, false);
	have = have_offset;
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		selection->Select (&cursor, &cursor);
	}

	emit |= SELECTION_CHANGED;
	have_offset = have;
	handled = true;
	
	return handled;
}

bool
RichTextBox::KeyPressUp (MoonModifier modifiers)
{
	TextPointer anchor = selection->GetAnchor_np();
	TextPointer cursor;
	bool handled = false;
	bool have;
	
	if ((modifiers & (CONTROL_MASK | ALT_MASK)) != 0) {
		return false;
	}
	
	// move the cursor down by one line from its current position
	cursor = CursorUp (cursor, false);
	have = have_offset;
	
	if ((modifiers & SHIFT_MASK) == 0) {
		// clobber the selection
		selection->Select (&cursor, &cursor);
	}

	emit |= SELECTION_CHANGED;
	have_offset = have;
	handled = true;

	return handled;
}

bool
RichTextBox::KeyPressHome (MoonModifier modifiers)
{
	bool handled = false;
	
	if ((modifiers & ALT_MASK) != 0)
		return false;

	TextPointer anchor = selection->GetAnchor_np ();
	TextPointer moving = selection->GetMoving_np ();
	TextPointer new_moving;

	if ((modifiers & CONTROL_MASK) != 0) {
		// move the moving pointer to the beginning of the buffer
		new_moving = TextPointer (this, CONTENT_START, LogicalDirectionForward);

	} else {
		// move the moving pointer to the beginning of the line
		new_moving = CursorLineBegin (moving);
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		if (!anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		// clobber the selection
		anchor = new_moving;
		moving = new_moving;
	}
	else {
		if (anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		moving = new_moving;
	}

	selection->Select (&anchor, &moving);

	
	return handled;
}

bool
RichTextBox::KeyPressEnd (MoonModifier modifiers)
{
	bool handled = false;
	
	if ((modifiers & ALT_MASK) != 0)
		return false;

	TextPointer anchor = selection->GetAnchor_np ();
	TextPointer moving = selection->GetMoving_np ();
	TextPointer new_moving;

	if ((modifiers & CONTROL_MASK) != 0) {
		// move the moving pointer to the end of the buffer
		new_moving = TextPointer (this, CONTENT_END, LogicalDirectionForward);
	} else {
		// move the moving pointer to the end of the line
		new_moving = CursorLineEnd (moving);
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		if (!anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		// clobber the selection
		anchor = new_moving;
		moving = new_moving;
	}
	else {
		if (anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		moving = new_moving;
	}

	selection->Select (&anchor, &moving);
	
	return handled;
}

bool
RichTextBox::KeyPressRight (MoonModifier modifiers)
{
	bool handled = false;
	
	if ((modifiers & ALT_MASK) != 0)
		return false;

	TextPointer anchor = selection->GetAnchor_np ();
	TextPointer moving = selection->GetMoving_np ();
	TextPointer new_moving;

	if ((modifiers & CONTROL_MASK) != 0) {
		// move the cursor to the beginning of the next word
		new_moving = CursorNextWord (moving);
	} else {
		new_moving = moving.GetPositionInsideRun (1);
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		if (!anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		// clobber the selection
		anchor = new_moving;
		moving = new_moving;
	}
	else {
		if (anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		moving = new_moving;
	}

	selection->Select (&anchor, &moving);
	
	return handled;
}

bool
RichTextBox::KeyPressLeft (MoonModifier modifiers)
{
	bool handled = false;
	
	if ((modifiers & ALT_MASK) != 0)
		return false;

	TextPointer anchor = selection->GetAnchor_np ();
	TextPointer moving = selection->GetMoving_np ();
	TextPointer new_moving;

	if ((modifiers & CONTROL_MASK) != 0) {
		// move the cursor to the beginning of the previous word
		new_moving = CursorPrevWord (moving);
	} else {
		new_moving = moving.GetPositionInsideRun (-1);
	}
	
	if ((modifiers & SHIFT_MASK) == 0) {
		if (!anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		// clobber the selection
		anchor = new_moving;
		moving = new_moving;
	}
	else {
		if (anchor.Equal (moving))
			emit |= SELECTION_CHANGED;

		moving = new_moving;
	}

	selection->Select (&anchor, &moving);
	
	return handled;
}

bool
RichTextBox::KeyPressUnichar (gunichar c)
{
	if (c != '\r') { 
		printf ("NIEX: KeyPressUnichar %x\n", c);
		return false;
	}

	if (!accepts_return)
		return false;

	RichTextBoxActionSetSelectionText *action = new RichTextBoxActionSetSelectionText(this, selection, "", true);
	action->Do ();
	undo->Push (action);

	return true;
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
	if (view)
		view->OnModelChanged (RichTextBoxModelChangedSelection, NULL);
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
RichTextBox::SyncAndEmit ()
{
	if (batch != 0 || emit == NOTHING_CHANGED)
		return;
	
	if (emit & SELECTION_CHANGED)
		SyncSelection ();
	
	if (IsLoaded ()) {
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

	BatchPush ();

	action->Do ();

	BatchPop ();
	SyncAndEmit ();

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
	RichTextBoxActionSetSelectionText *set_selection;

	TextSelection *selection = GetSelection();
	if (selection->GetStart()->GetParent() == NULL)
		SelectAll();

	BatchPush ();

	if (selection->IsEmpty ()) {
		// we only do this if the selection is empty (we're
		// blinking a cursor) because if we're going to clear
		// a selection we need to start a new action for that.
		RichTextBoxAction *action = (RichTextBoxAction*)undo->Top ();
		if (action && action->GetType () == RichTextBoxAction::SET_SELECTION_TEXT) {
			((RichTextBoxActionSetSelectionText*)action)->AppendText (str);
			goto done;
		}
	}

	set_selection = new RichTextBoxActionSetSelectionText(this, selection, str);

	set_selection->Do ();
	undo->Push (set_selection);

 done:
	BatchPop ();
	SyncAndEmit ();
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

	BatchPush ();
	action->Do ();
	BatchPop ();
	SyncAndEmit ();

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
		emit = SELECTION_CHANGED;
		GetSelection()->Select (&cursor, &cursor);
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

		TextPointer anchor = selection->GetAnchor_np();
		TextPointer moving = view->GetLocationFromXY (x, y);

		selection->Select (&anchor, &moving);
		emit = SELECTION_CHANGED;

		BatchPop ();

		SyncAndEmit ();
		
		if ((clipboard = GetClipboard (this, MoonClipboard_Primary))) {
			// copy the selection to the primary clipboard
			char *selection_text = selection->GetText ();
			clipboard->SetText (selection_text);
			g_free (selection_text);
		}

		// FIXME: this sucks.  we should only invalidate the selection area (the union between old and new areas, that is..)
		Invalidate (); 
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
		Value *objv = NULL;

		if (xaml) {
			printf ("setting xaml to %s\n", xaml);

			SL4XamlLoader *loader = new SL4XamlLoader (GetDeployment()->GetSurface()); // XXX we're leaking this
			objv = loader->CreateFromStringWithError (xaml, true, &element_type, 0, error, this);
			if (element_type != Type::SECTION || error->code)
				return;
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
RichTextBox::Split (int loc, TextElement *into)
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
	return new TextPointer (this, CONTENT_START, LogicalDirectionBackward);
}

TextPointer*
RichTextBox::GetContentEnd ()
{
	return new TextPointer (this, CONTENT_END, LogicalDirectionForward);
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
	if (undo->IsEmpty ())
		return;
	
	BatchPush ();

	RichTextBoxAction *action = (RichTextBoxAction*)undo->Pop ();
	redo->Push (action);
	action->Undo ();
	
	//emit = CONTENT_CHANGED | SELECTION_CHANGED;

	BatchPop ();
	
	SyncAndEmit ();
}

void
RichTextBox::Redo ()
{
	if (redo->IsEmpty ())
		return;
	
	BatchPush ();

	RichTextBoxAction *action = (RichTextBoxAction*)redo->Pop ();
	undo->Push (action);

	action->Do ();
	
	//emit = CONTENT_CHANGED | SELECTION_CHANGED;

	BatchPop ();
	
	SyncAndEmit ();
}

Rect
RichTextBox::GetCharacterRect (const TextPointer *tp, LogicalDirection direction)
{
	return view ? view->GetCharacterRect (tp, direction) : Rect::ManagedEmpty;
}

void
RichTextBox::DocumentPropertyChanged (TextElement *onElement, PropertyChangedEventArgs *args)
{
	BatchPush ();
	emit = CONTENT_CHANGED;

	if (view)
		view->DocumentChanged (onElement);

	BatchPop ();
	SyncAndEmit ();
}

void
RichTextBox::DocumentCollectionChanged (TextElement *onElement, Collection *col, CollectionChangedEventArgs *args)
{
	BatchPush ();
	emit = CONTENT_CHANGED;

	if (view)
		view->DocumentChanged (onElement);

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
RichTextBoxView::GetCharacterRect (const TextPointer *tp, LogicalDirection direction)
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
	if (textbox->IsFocused () && textbox->GetSelection()->IsEmpty()) {
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
	TextPointer cur = textbox->GetSelection()->GetAnchor_np();

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
RichTextBoxView::PostRender (Context *ctx, Region *region, bool skip_children)
{
	// render our chidren if we need to
	if (!skip_children) {
		VisualTreeWalker walker = VisualTreeWalker (this, ZForward, false);
		while (UIElement *child = walker.Step ())
			child->DoRender (ctx, region);
	}

	if (ctx->IsMutable ()) {
		cairo_t *cr = ctx->Push (Context::Cairo ());

		Size renderSize = GetRenderSize ();
	
		UpdateCursor (false);
	
		layout->Select (textbox->GetSelection());
	
		cairo_save (cr);

		RenderLayoutClip (cr);
	
		layout->SetAvailableWidth (renderSize.width);
		Paint (cr);
		cairo_restore (cr);

		ctx->Pop ();
	}

	// Chain up, but skip children since we've already rendered them here.
	UIElement::PostRender (ctx, region, true);
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
		if (had_selected_text || !textbox->GetSelection()->IsEmpty()) {
			// the selection has changed, update the layout's selection
			had_selected_text = !textbox->GetSelection()->IsEmpty();
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

		had_selected_text = !textbox->GetSelection()->IsEmpty();
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
RichTextBoxView::DocumentChanged (TextElement *onElement)
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
