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
	xaml = NULL;
	text = NULL;
}

void
TextSelection::ApplyPropertyValue (DependencyProperty *formatting, Value *value)
{
	RichTextBox *rtb = anchor.GetRichTextBox ();
	if (rtb == NULL) {
		g_warning ("this shouldn't happen...");
		return;
	}

	rtb->ApplyFormattingToSelection (this, formatting, value);
}

Value *
TextSelection::GetPropertyValue (DependencyProperty *formatting) const
{
	Value *value = start.GetParent()->GetValue (formatting);
	Value *current_value;

	DocumentWalker walker (start.GetParentNode(), DocumentWalker::Forward);

	walker.Step ();// skip the ::Leave for the start node

	while (true) {
		IDocumentNode *node;
		DocumentWalker::StepType step = walker.Step (&node);
		switch (step) {
		case DocumentWalker::Enter:
			current_value = node->AsDependencyObject()->GetValue (formatting);
			if (!Value::AreEqual (value, current_value))
				return NULL;

			if (node == end.GetParentNode())
				return value;

		case DocumentWalker::Leave:
			break;
		case DocumentWalker::Done:
			// this shouldn't happen, but do we care?
			g_warning ("TextSelecion::GetPropertyValue document walker found the end of the document before finding the selection end.");
			break;
			return NULL;
		}
	}
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

	g_free (text);
	text = NULL;

	g_free (xaml);
	xaml = NULL;
}


void
TextSelection::Insert (TextElement *element)
{
	if (anchor.GetParent() == NULL || moving.GetParent() == NULL)
		// if either are null we're going nowhere fast...
		return;

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

		if (parents_children == NULL)
			return; /* #rtx24 */

		DependencyObject *new_el;
		if (element->Is (el->GetObjectType()) /* a more precise check perhaps?  instead of subclass? */) {
			// we don't need to split the node.  we just
			// need to reach in and reparent children
			// after @loc into element.
			new_el = node ? node->Split (loc, element) : NULL;
		}
		else {
			new_el = node ? node->Split (loc) : NULL;
		}

		int new_el_loc = parents_children->IndexOf (el) + 1;

		if (new_el)
			parents_children->Insert (new_el_loc, new_el);

		el = el_parent;
		loc = new_el_loc;
		if (new_el && new_el != element)
			new_el->unref ();

		if (new_el == element) {
			// we've already inserted the element as part of the split.
			return;
		}
	}

	printf ("NIEX TextSelection::Insert\n");
}

bool
TextSelection::SelectWithError (const TextPointer *anchorPosition, const TextPointer *movingPosition, MoonError *error)
{
	// FIXME: we need to verify that both TextPointers come from the same RTB, and if they don't, raise ArgExc.
	
	// once we've verified, just set start and end

	anchor = *anchorPosition;
	moving = *movingPosition;

	if (anchor.CompareTo_np (moving) < 0) {
		start = anchor;
		end = moving;
	}
	else {
		start = moving;
		end = anchor;
	}

	g_free (text);
	text = NULL;

	g_free (xaml);
	xaml = NULL;

	return true;
}

bool
TextSelection::Select (const TextPointer *anchorPosition, const TextPointer *movingPosition)
{
	MoonError err;
	
	return SelectWithError (anchorPosition, movingPosition, &err);
}

bool
TextSelection::Select (const TextPointer& anchorPosition, const TextPointer& movingPosition)
{
	return Select (&anchorPosition, &movingPosition);
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

			if (strlen (text) < (size_t) anchor.ResolveLocation ()){
				// #339RT enters here
				g_free (new_text);
				new_text = g_strdup ("BUGBUGBUG");
			} else {
				strncpy (new_text, run_text, anchor.ResolveLocation());
				strcpy (new_text + anchor.ResolveLocation(), text);
				strncpy (new_text + anchor.ResolveLocation() + strlen(text), run_text + anchor.ResolveLocation(), strlen (run_text) - anchor.ResolveLocation());
			}

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

			if (anchor.CompareTo_np (moving) < 0) {
				start = anchor;
				end = moving;
			}
			else {
				start = moving;
				end = anchor;
			}
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

			if (anchor.CompareTo_np (moving) < 0) {
				start = anchor;
				end = moving;
			}
			else {
				start = moving;
				end = anchor;
			}
		}
	}

	g_free (this->text);
	this->text = NULL;
}

char*
TextSelection::GetText ()
{
	GString *gstr;
	TextPointer tp;


	if (text)
		goto done;

	if (anchor.GetParent() == NULL ||
	    moving.GetParent() == NULL) {
		text = g_strdup ("");
		goto done;
	}

	if (anchor.GetParent() == moving.GetParent() &&
	    anchor.GetLocation () == moving.GetLocation()) {
		text = g_strdup ("");
		goto done;
	}

	gstr = g_string_new ("");
	tp = anchor;

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

	text = g_string_free (gstr, FALSE);

 done:
	return g_strdup (text);
}

	
void
TextSelection::SetXamlWithError (const char *xaml, MoonError *error)
{
	printf ("setting xaml to %s\n", xaml);
}

char*
TextSelection::GetXaml ()
{
	const char *header = "<Section xml:space=\"preserve\" HasTrailingParagraphBreakOnPaste=\"False\" xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\">";
	const char *trailer = "</Section>";

	GString *str;
	ArrayList ancestors;
	DependencyObject *el;

	if (xaml)
		goto done;

	if (!anchor.GetParent() || !moving.GetParent()) {
		xaml = g_strdup ("");
		goto done;
	}

	if (anchor.GetParent() == moving.GetParent() && anchor.GetLocation () == moving.GetLocation()) {
		xaml = g_strdup ("");
		goto done;
	}

	str = g_string_new (header);

	// first we serialize the xaml start elements for all
	// TextElements that contain the selection start (but not the
	// most deeply nested element itself)

	el = anchor.GetParent();
	if (el && !el->Is (Type::RICHTEXTBOX)) {
		// skip anchor.GetParent() here.
		el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
		while (el) {
			if (el->Is (Type::RICHTEXTBOX))
				break;
			ancestors.Add (el);
			el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
			if (!el)
				break;
		}
	}

	for (int i = ancestors.GetCount() - 1; i >= 0; i --) {
		TextElement *te = (TextElement*)ancestors[i];
		IDocumentNode *node = IDocumentNode::CastToIDocumentNode (te);
		node->SerializeXamlStartElement (str);
	}




	// now we output the start element (and deal with the case where start_element == end_element)
	el = anchor.GetParent ();
	if (el->Is (Type::RUN)) {
		if (el == moving.GetParent()) {
			// if both textpointers are in the same
			// element, we need to use start+length form
			((Run*)el)->SerializeXaml (str, anchor.ResolveLocation (), moving.ResolveLocation () - anchor.ResolveLocation());
		}
		else {
			// since the moving textpointer is outside
			// this run, we just use the start form.
			((Run*)el)->SerializeXaml (str, anchor.ResolveLocation ());
		}
	}
	else {
		((TextElement*)el)->SerializeXaml (str);
	}

	if (anchor.GetParent() != moving.GetParent()) {
		// now walk the document from start element to end element, outputting everything manually along the way.
		DocumentWalker walker (anchor.GetParentNode(), DocumentWalker::Forward);
		IDocumentNode *node;
		DocumentWalker::StepType stepType;

		stepType = walker.Step (); // step out of the start element

		while (stepType != DocumentWalker::Done) {
			stepType = walker.Step (&node);

			if (node == moving.GetParentNode())
				break;
			if (stepType == DocumentWalker::Enter)
				node->SerializeXamlStartElement(str);
			else
				node->SerializeXamlEndElement(str);
		}

		// now we output the end element
		el = moving.GetParent ();
		if (el->Is (Type::RUN)) {
			((Run*)el)->SerializeXaml (str, 0, moving.ResolveLocation ());
		}
		else {
			((TextElement*)el)->SerializeXaml (str);
		}
	}

	// now serialize the xaml end elements for all TextElements
	// that contain the selection end (but not the most deeply
	// nested element itself)

	if (anchor.GetParent() == moving.GetParent ()) {
		// this case is trivial, we just output the same list
		// of end elements that we outputted the start
		// elements before
		for (int i = ancestors.GetCount() - 1; i >= 0; i --) {
			TextElement *te = (TextElement*)ancestors[i];
			IDocumentNode *node = IDocumentNode::CastToIDocumentNode (te);
			node->SerializeXamlEndElement (str);
		}
	}
	else {
		ancestors.SetCount (0);

		DependencyObject *el = moving.GetParent();
		if (el && !el->Is (Type::RICHTEXTBOX)) {
			// skip moving.GetParent() here.
			el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
			while (el) {
				if (el->Is (Type::RICHTEXTBOX))
					break;
				ancestors.Add (el);
				el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
				if (!el)
					break;
			}
		}

		for (int i = ancestors.GetCount() - 1; i >= 0; i --) {
			TextElement *te = (TextElement*)ancestors[i];
			IDocumentNode *node = IDocumentNode::CastToIDocumentNode (te);
			node->SerializeXamlEndElement (str);
		}
	}

	g_string_append (str, trailer);

	xaml = g_string_free (str, FALSE);

 done:
	return g_strdup (xaml);
}

TextPointer*
TextSelection::GetStart () const
{
	return new TextPointer (start);
}

TextPointer*
TextSelection::GetEnd () const
{
	return new TextPointer (end);
}

TextPointer
TextSelection::GetStart_np () const
{
	return start;
}

TextPointer
TextSelection::GetEnd_np () const
{
	return end;
}

TextPointer*
TextSelection::GetAnchor () const
{
	return new TextPointer (anchor);
}

TextPointer*
TextSelection::GetMoving () const
{
	return new TextPointer (moving);
}

TextPointer
TextSelection::GetAnchor_np () const
{
	return anchor;
}

TextPointer
TextSelection::GetMoving_np () const
{
	return moving;
}

bool
TextSelection::IsEmpty () const
{
	return anchor.Equal (moving);
}

};
