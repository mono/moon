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
	printf ("TextSelection::ApplyPropertyValue (%s)\n", formatting->GetName());
	// FIXME: implement this
}

Value *
TextSelection::GetPropertyValue (DependencyProperty *formatting)
{
	printf ("TextSelection::GetPropertyValue\n");
	// FIXME: implement this
	return NULL;
}

void
TextSelection::Insert (TextElement *element)
{
	printf ("TextSelection::Insert\n");
	// FIXME: implement this
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

	if (anchor.GetParent() == moving.GetParent()) {
		// clear out the selection
		if (anchor.GetLocation () != moving.GetLocation()) {

			if (anchor.GetParent()->Is (Type::RUN)) {
				Run *run = (Run*)anchor.GetParent();
				char *run_text = g_strdup (run->GetText());

				int length_to_remove = moving.ResolveLocation() - anchor.ResolveLocation();

				memmove (run_text + anchor.ResolveLocation(), run_text + length_to_remove, length_to_remove);
				*(run_text + anchor.ResolveLocation() + length_to_remove) = 0;

				run->SetText (run_text);

				g_free (run_text);
			}
			else {
				IDocumentNode *node = anchor.GetParentNode();

				int length_to_remove = moving.ResolveLocation() - anchor.ResolveLocation();

				for (int i = 0; i < length_to_remove; i ++)
					node->GetDocumentChildren()->RemoveAt (anchor.ResolveLocation());
			}
		}

		if (text && *text) {
			if (anchor.GetParent()->Is (Type::RUN)) {
				const char *run_text = ((Run*)anchor.GetParent())->GetText();
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

				Run *r = MoonUnmanagedFactory::CreateRun ();
				r->SetText (text);

				if (children->Is(Type::BLOCK_COLLECTION)) {
					// the node takes Blocks as children, so we need to create a Paragraph first.
					Paragraph *p = MoonUnmanagedFactory::CreateParagraph();
					children->Insert (anchor.GetLocation(), Value (p));
					children = p->GetInlines();
					children->Add (Value (r));
				}
				else {
					children->Insert (anchor.GetLocation(), Value (r));
				}

				anchor = TextPointer (r, CONTENT_END, anchor.GetLogicalDirection());
				moving = TextPointer (r, CONTENT_END, anchor.GetLogicalDirection());
			}
		}
	}
	else {
		// harder case, anchor/moving are in different elements
		printf ("NIEX hard case TextSelection::SetText\n");
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
		if (tp.GetParent()->Is (Type::RUN)) {
			Run *run = (Run*)tp.GetParent();
			if (tp.GetParent() == moving.GetParent()) {
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
			tp = tp.GetPositionAtOffset_np (1, tp.GetLogicalDirection());
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
	IDocumentNode *node = IDocumentNode::CastToIDocumentNode (el);
	return node->Serialize();
}

TextPointer*
TextSelection::GetStart ()
{
	return &anchor;
}

TextPointer*
TextSelection::GetEnd ()
{
	return &moving;
}

bool
TextSelection::IsEmpty ()
{
	return anchor.CompareTo_np (moving) == 0;
}

};
