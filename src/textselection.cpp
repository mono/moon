/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textpointer.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include "textselection.h"
#include "richtextbox.h"
#include "factory.h"

namespace Moonlight {

//
// TextSelection
//

TextSelection::TextSelection ()
{
}

void
TextSelection::ApplyPropertyValue (DependencyProperty *formatting, Value *value)
{
	DependencyObject *el = anchor.GetParent();
	while (el) {
		if (el->Is (Type::RICHTEXTBOX))
			break;
		el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
		if (!el)
			break;
	}
	if (el == NULL) {
		g_warning ("this shouldn't happen...");
		return;
	}
	
	RichTextBox *rtb = (RichTextBox*)el;

	rtb->ApplyFormattingToSelection (this, formatting, value);
}

Value *
TextSelection::GetPropertyValue (DependencyProperty *formatting)
{
	printf ("TextSelection::GetPropertyValue\n");
	// FIXME: implement this
	return NULL;
}

void
TextSelection::ClearSelection ()
{
	// clear out the selection
	if (anchor.GetParent() == moving.GetParent()) {
		bool remove_element = false;

		if (anchor.GetLocation () != moving.GetLocation()) {

			if (anchor.GetParent()->Is (Type::RUN)) {
				Run *run = (Run*)anchor.GetParent();
				char *run_text = g_strdup (run->GetText());

				if (run_text) {
					int length_to_remove = moving.ResolveLocation() - anchor.ResolveLocation();
					
					memmove (run_text + anchor.ResolveLocation(), run_text + moving.ResolveLocation (), strlen (run_text + moving.ResolveLocation()));
					*(run_text + strlen(run_text) - length_to_remove) = 0;

					run->SetText (run_text);

					// we need to remove the run if we've cleared all the text in it
					remove_element = !*run_text;
				} 
				else {
					remove_element = true;
				}
				g_free (run_text);
			}
			else {
				IDocumentNode *node = anchor.GetParentNode();

				int length_to_remove = moving.ResolveLocation() - anchor.ResolveLocation();

				for (int i = 0; i < length_to_remove; i ++)
					node->GetDocumentChildren()->RemoveAt (anchor.ResolveLocation());

				// we need to remove the element if we've removed all the children
				remove_element = node->GetDocumentChildren()->GetCount() == 0;
			}
		}

		DependencyObject *el = anchor.GetParent();
		while (remove_element) {
			if (el->Is (Type::RICHTEXTBOX))
				break;

			DependencyObject* parent = el->GetParent() ? el->GetParent()->GetParent() : NULL;
			if (!parent) {
				g_warning ("shouldn't happen");
				return;
			}

			IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
			parent_node->GetDocumentChildren()->Remove (Value(el));

			el = parent;
			remove_element = parent_node->GetDocumentChildren()->GetCount() == 0;
		}
	}
	else {
		// harder case, anchor/moving are in different elements
		printf ("NIEX hard case TextSelection::ClearSelection\n");
	}
}


void
TextSelection::Insert (TextElement *element)
{
	if (anchor.GetParent() == NULL || moving.GetParent() == NULL)
		// if either are null we're going nowhere fast...
		return;

	// refactor out the "clear out selection" from SetText
	ClearSelection ();

	// at this point both anchor and moving are the same location

	// depending on what the anchor's parent is,
	// and what @element is, we might need to
	// split the tree higher up
	DependencyObject *el = anchor.GetParent ();
	int loc = anchor.ResolveLocation();

	while (el) {
		DependencyObject *el_parent = el->GetParent();
		if (el_parent)
			el_parent = el_parent->GetParent();

		IDocumentNode *node = IDocumentNode::CastToIDocumentNode (el);

		DependencyObjectCollection *children = node->GetDocumentChildren ();
		if (children && element->Is (children->GetElementType())) {
			// we can insert the element here, so let's do it and be done with things
			children->Insert (loc, element);
			return;
		}

		if (!el_parent) {
			g_warning ("new element cannot fit inside element, and we can't split it");
			return;
		}
				
		// we need to split the current element at
		// @loc, add the new element to el's parent
		// after el, and walk back up to el's parent
		// with new_el's index as @loc.
		IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (el_parent);
		DependencyObjectCollection *parents_children = parent_node ? parent_node->GetDocumentChildren () : NULL;
		DependencyObject *new_el = node ? node->Split (loc) : NULL;

		if (!new_el) {
			g_warning ("split failed");
			return;
		}

		int new_el_loc = parents_children->IndexOf (el) + 1;
		parents_children->Insert (new_el_loc, new_el);
		el = el_parent;
		loc = new_el_loc;
		new_el->unref ();
	}


	printf ("TextSelection::Insert\n");
}

bool
TextSelection::SelectWithError (TextPointer *anchorPosition, TextPointer *movingPosition, MoonError *error)
{
	// FIXME: we need to verify that both TextPointers come from the same RTB, and if they don't, raise ArgExc.
	
	// once we've verified, just set start and end

	// FIXME: is @movingPosition always End?  do we need to compare and swap them?

	this->anchor = *anchorPosition;
	this->moving = *movingPosition;

	return true;
}

bool
TextSelection::Select (TextPointer *anchorPosition, TextPointer *movingPosition)
{
	MoonError err;
	
	return SelectWithError (anchorPosition, movingPosition, &err);
}

void
TextSelection::SetText (const char *text)
{
#define CONTENT_START (0)
#define CONTENT_END ((guint32)-1)

	if (anchor.GetParent() == NULL || moving.GetParent() == NULL)
		// if either are null we're going nowhere fast...
		return;

	ClearSelection ();

	// at this point the selection is empty and anchor == moving
	if (text && *text) {
		if (anchor.GetParent()->Is (Type::RUN)) {
			const char *run_text = ((Run*)anchor.GetParent())->GetText();
			if (run_text == NULL)
				run_text = "\0";

			char *new_text = (char*)g_malloc0 (strlen (run_text) + strlen (text) + 1);
				
			strncpy (new_text, run_text, anchor.ResolveLocation());
			strcpy (new_text + anchor.ResolveLocation(), text);
			strncpy (new_text + anchor.ResolveLocation() + strlen(text), run_text + anchor.ResolveLocation(), strlen (run_text) - anchor.ResolveLocation());

			((Run*)anchor.GetParent())->SetText (new_text);

			if (moving.GetLocation() > strlen (new_text)) {
				anchor = TextPointer (anchor.GetParent(), CONTENT_END, anchor.GetLogicalDirection());
				moving = TextPointer (anchor.GetParent(), CONTENT_END, anchor.GetLogicalDirection());
			}
			else {
				TextPointer new_anchor (anchor.GetParent(), anchor.ResolveLocation () + strlen (text), anchor.GetLogicalDirection());
				moving = TextPointer (anchor.GetParent(), anchor.ResolveLocation () + strlen (text), anchor.GetLogicalDirection());
				anchor = new_anchor;
			}

			g_free (new_text);
		}
		else {
			IDocumentNode *node = anchor.GetParentNode();
			DependencyObjectCollection *children = node->GetDocumentChildren();

			if (!children) {
				// this can happen when anchor is in an InlineUIContainer.
				printf ("NIEX TextSelection.SetText for anchor == InlineUIContainer.\n");
				return;
			}

			Run *r = MoonUnmanagedFactory::CreateRun ();
			r->SetText (text);

			if (children->Is(Type::BLOCK_COLLECTION)) {
				// the node takes Blocks as children, so we need to create a Paragraph first.
				Paragraph *p = MoonUnmanagedFactory::CreateParagraph();
				children->Insert (anchor.GetLocation(), Value (p));
				children = p->GetInlines();
				children->Add (Value (r));
				p->unref ();
			}
			else {
				children->Insert (anchor.GetLocation(), Value (r));
			}

			anchor = TextPointer (r, CONTENT_END, anchor.GetLogicalDirection());
			moving = TextPointer (r, CONTENT_END, anchor.GetLogicalDirection());
			r->unref ();
		}
	}
}

char*
TextSelection::GetText ()
{
	if (anchor.GetParent() == moving.GetParent() &&
	    anchor.GetLocation () == moving.GetLocation())
	    return g_strdup ("");

	GString *gstr = g_string_new ("");
	TextPointer tp = anchor;

	while (tp.CompareTo_np (moving) < 0) {
		DependencyObject *parent = tp.GetParent ();

		if (parent && parent->Is (Type::RUN)) {
			Run *run = (Run*)parent;
			if (parent == moving.GetParent()) {
				// tp and moving are in the same element, so we append the substring and set tp = moving.
				g_string_append_len (gstr, run->GetText() + tp.ResolveLocation(), moving.ResolveLocation() - tp.ResolveLocation());
				tp = moving;
			}
			else {
				g_string_append (gstr, run->GetText());
				tp = run->GetContentEnd_np();
				tp = tp.GetPositionAtOffset_np (1, tp.GetLogicalDirection());
			}
		}
		else {
			TextPointer new_tp = tp.GetPositionAtOffset_np (1, tp.GetLogicalDirection());
			if (new_tp.CompareTo_np (tp) == 0)
				break;
			tp = new_tp;
		}
	}

	printf ("returning %s from TextSelection::GetText\n", gstr->str);

	return g_string_free (gstr, FALSE);
}

	
void
TextSelection::SetXaml (const char *xaml)
{
	printf ("setting xaml to %s\n", xaml);
}

char*
TextSelection::GetXaml ()
{
	if (anchor.GetLocation () == moving.GetLocation())
		return g_strdup ("");

	// I'm guessing this should only include ancestors (up to the
	// root) that are required to serialize the actual selection,
	// but for now let's just serialize the entire contents of the
	// RTB.

	DependencyObject *el = anchor.GetParent();
	while (el) {
		if (el->Is (Type::RICHTEXTBOX))
			break;
		el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
		if (!el)
			break;
	}
	if (el == NULL) {
		g_warning ("this shouldn't happen...");
		return NULL;
	}

	// el should be the RichTextBox now.
	GString *str = g_string_new ("");
	IDocumentNode *node = IDocumentNode::CastToIDocumentNode (el);
	node->SerializeXaml(str);
	return g_string_free (str, FALSE);
}

TextPointer*
TextSelection::GetStart ()
{
	return new TextPointer (anchor);
}

TextPointer*
TextSelection::GetEnd ()
{
	return new TextPointer (moving);
}

bool
TextSelection::IsEmpty ()
{
	return anchor.CompareTo_np (moving) == 0;
}

};
