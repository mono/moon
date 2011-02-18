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

#include "textpointer.h"
#include "dependencyobject.h"
#include "textelement.h"
#include "utils.h"
#include "type.h"
#include "richtextbox.h"

namespace Moonlight {

//
// DocumentWalker
//

DocumentWalker::DocumentWalker (IDocumentNode *node, DocumentWalker::Direction direction)
{
	this->node = node;
	this->child_index = 0;
	this->direction = direction;
}

DocumentWalker::~DocumentWalker ()
{
}

DocumentWalker::StepType
DocumentWalker::Step (IDocumentNode **node_return)
{
	DependencyObjectCollection *children = node->GetDocumentChildren();
	if (direction == DocumentWalker::Forward) {
		if (children && children->GetCount() > child_index) {
			// 1) if node is a container, step into the child
			node = IDocumentNode::CastToIDocumentNode (children->GetValueAt (child_index)->AsTextElement());
			child_index = 0;
			if (node_return)
				*node_return = node;
			return Enter;
		}
		else {
			// 2) if it's not a container, then we walk up a level, step out of this node
			IDocumentNode *parent_node = node->GetParentDocumentNode();
			if (parent_node == NULL)
				return Done;

			DependencyObjectCollection *parent_children= parent_node->GetDocumentChildren();
			int i = parent_children->IndexOf (node->AsDependencyObject());

			if (node_return)
				*node_return = node;

			child_index = i + 1;
			node = parent_node;
			return Leave;
		}
	}
	else {
		if (children && children->GetCount() > child_index && child_index >= 0) {
			// 1) if node is a container, step into the child
			node = IDocumentNode::CastToIDocumentNode (children->GetValueAt (child_index)->AsTextElement());
			children = node->GetDocumentChildren ();
			child_index = children ? children->GetCount() - 1 : 0;
			if (node_return)
				*node_return = node;
			return Enter;
		}
		else {
			// 2) if it's not a container, then we walk up a level, step out of this node
			IDocumentNode *parent_node = node->GetParentDocumentNode();
			if (parent_node == NULL)
				return Done;

			DependencyObjectCollection *parent_children= parent_node->GetDocumentChildren();
			int i = parent_children->IndexOf (node->AsDependencyObject());

			if (node_return)
				*node_return = node;

			child_index = i - 1;
			node = parent_node;
			return Leave;
		}
	}
}

IDocumentNode *
DocumentWalker::GetNode ()
{
	return node;
}


//
// TextPointer
//

static int
compare_locations (guint32 loc1, guint32 loc2)
{
	if (loc1 == CONTENT_END)
		return loc2 == CONTENT_END ? 0 : 1;
	else if (loc2 == CONTENT_END)
		return -1;
	else {
		gint32 compare_location = loc1 - loc2;
		if (compare_location == 0)
			return 0;
		else
			return compare_location < 0 ? -1 : 1;
	}
}

static bool
verify_textpointer_in_document (const TextPointer *pointer, MoonError *error)
{
	RichTextBox *rtb = pointer->GetRichTextBox ();

	if (rtb)
		return true;

	MoonError::FillIn (error, MoonError::NOT_SUPPORTED_EXCEPTION, "Can't use a TextPointer which is outside of a RichTextBox's document");
	return false;
}


int
TextPointer::CompareToWithError (const TextPointer *pointer, MoonError *error) const
{
	if (!verify_textpointer_in_document (this, error) ||
	    !verify_textpointer_in_document (pointer, error))
		return -1;

	if (this->GetParent() == pointer->GetParent()) {
		DependencyObjectCollection *children = this->GetParentNode()->GetDocumentChildren();
		if (children && children->GetCount() > 0)
			return compare_locations (this->GetLocation(), pointer->GetLocation());
		else
			return this->ResolveLocation() - pointer->ResolveLocation();
	}
	else {
		GPtrArray *this_array = g_ptr_array_new();
		GPtrArray *pointer_array = g_ptr_array_new();

		DependencyObject *el = GetParent();
		while (el) {
			g_ptr_array_insert (this_array, 0, el);
			if (el->Is (Type::RICHTEXTBOX))
				break;
			el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
			if (!el)
				break;
		}

		el = pointer->GetParent();
		while (el) {
			g_ptr_array_insert (pointer_array, 0, el);
			if (el->Is (Type::RICHTEXTBOX))
				break;
			el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
			if (!el)
				break;
		}

		guint32 count_to_compare = MIN (this_array->len, pointer_array->len);

		for (guint32 i = 0; i < count_to_compare; i ++) {
			DependencyObject *this_el = (DependencyObject*)g_ptr_array_index (this_array, i);
			DependencyObject *pointer_el = (DependencyObject*)g_ptr_array_index (pointer_array, i);

			if (this_el == pointer_el)
				continue;

			if (i == 0) {
				// this shouldn't happen... there shouldn't be a difference between two paths on the first element, since that should always be a RTB
				int rv = this_array->len < pointer_array->len ? -1 : 1;
				g_ptr_array_free (this_array, TRUE);
				g_ptr_array_free (pointer_array, TRUE);
				return rv;
			}

			/* at this point this_el and pointer_el are
			   different.  check index i-1's idea of their
			   positions */
			DependencyObject *common_parent = (DependencyObject*)g_ptr_array_index (this_array, i-1);
			IDocumentNode *common_parent_node = IDocumentNode::CastToIDocumentNode (common_parent);
			int this_index = common_parent_node->GetDocumentChildren()->IndexOf (Value (this_el));
			int pointer_index = common_parent_node->GetDocumentChildren()->IndexOf (Value (pointer_el));
			g_ptr_array_free (this_array, TRUE);
			g_ptr_array_free (pointer_array, TRUE);
			return this_index < pointer_index ? -1 : 1;
		}

		// if we make it here, it means we've run through
		// "count_to_compare" items that were identical, and
		// one of the paths is longer (so represents a child
		// of items[count_to_compare].

		// so we need to figure out which array has more
		// elements, then compare that against the other
		// TextPointer's location

		if (count_to_compare < this_array->len) {
			// @this's parent is a child of pointer_array[count_to_compare-1]
			DependencyObject *parent = (DependencyObject*)g_ptr_array_index(pointer_array, count_to_compare - 1);
			IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
			guint32 child_index = parent_node->GetDocumentChildren()->IndexOf (Value ((DependencyObject*)g_ptr_array_index(this_array, count_to_compare)));

			return pointer->GetLocation() > child_index ? -1 : 1;
		}
		else if (count_to_compare < pointer_array->len) {
			// @pointer's parent is a child of this_array[count_to_compare-1]
			DependencyObject *parent = (DependencyObject*)g_ptr_array_index(this_array, count_to_compare - 1);
			IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
			guint32 child_index = parent_node->GetDocumentChildren()->IndexOf (Value ((DependencyObject*)g_ptr_array_index(pointer_array, count_to_compare)));

			return child_index >= this->GetLocation () ? -1 : 1;
		}
	}
	return -1;
}

int
TextPointer::CompareTo_np (const TextPointer& pointer) const
{
	MoonError error;
	return CompareToWithError (&pointer, &error);
}

int
TextPointer::CompareTo (const TextPointer *pointer)
{
	MoonError error;
	return CompareToWithError (pointer, &error);
}

bool
TextPointer::Equal (const TextPointer& pointer) const
{
	return parent == pointer.parent && location == pointer.location;
}

Rect
TextPointer::GetCharacterRect (LogicalDirection dir) const
{
	RichTextBox *rtb = GetRichTextBox ();
	if (!rtb) {
		g_warning ("a TextPointer outside of a RichTextBox?  say it ain't so...");
		return Rect::ManagedEmpty;
	}

	return rtb->GetCharacterRect (this, dir);
}

TextPointer *
TextPointer::GetNextInsertionPosition (LogicalDirection dir) const
{
	// FIXME: implement this
	printf ("NIEX: TextPointer::GetNextInsertionPosition\n");
	return NULL;
}

TextPointer
TextPointer::GetNextInsertionPosition_np (LogicalDirection dir) const
{
	// FIXME: implement this
	printf ("NIEX: TextPointer::GetNextInsertionPosition_np\n");
	return *this;
}

class PositionAtOffsetIterator {
public:
	PositionAtOffsetIterator (TextElement *parent, guint32 location, LogicalDirection logical_direction);
	TextPointer *GetTextPointer (int offset, LogicalDirection dir);

private:
	DependencyObject *GetElementParent ();

	bool Step (int *offset);

	DependencyObject *element;
	guint32 location;
	LogicalDirection logical_direction;
};


PositionAtOffsetIterator::PositionAtOffsetIterator (TextElement *element, guint32 location, LogicalDirection logical_direction)
{
	this->element = element;
	this->location = location;
	this->logical_direction = logical_direction;
}

TextPointer*
PositionAtOffsetIterator::GetTextPointer (int offset, LogicalDirection dir)
{
	while (Step (&offset)) ;

	if (element == NULL)
		return NULL;

	return new TextPointer (element, location, dir);
}

DependencyObject*
PositionAtOffsetIterator::GetElementParent()
{
	if (element->Is(Type::RICHTEXTBOX))
		return NULL;
	else {
		// element is a TextElement
		DependencyObject *p = element->GetParent(); // this first GetParent returns the parent collection
		if (!p) return NULL;
		return p->GetParent(); // and this the actual parent
	}
}

bool
PositionAtOffsetIterator::Step (int *offset)
{
	if (element == NULL)
		return false;

	if (*offset == 0) {
		// we're done stepping.  element/location should be valid.
		return false;
	}

	if (*offset < 0) {
		// we're stepping backward

		if (location == CONTENT_START) {
			// we're stepping backward but we're currently
			// at element's ContentStart.  we need to step
			// up into our parent
			DependencyObject *parent = GetElementParent();

			if (!parent) {
				// we can't back up anymore.  result is NULL.
				element = NULL;
				return true;
			}

			IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
			(*offset) ++;	
			location = parent_node->GetDocumentChildren()->IndexOf (Value (element));
			element = parent;
			return true;
		}

		if (element->Is (Type::RUN)) {
			const char *text = ((Run*)element)->GetText();
			guint32 textlen = strlen (text);

			if (location == CONTENT_END)
				location = textlen;

			if (location + *offset < CONTENT_START) {
				// bump up to the parent after adding the location to our offset
				*offset += location;
				location = CONTENT_START;
				return true;
			}
			else {
				// the operation can be satisfied completely inside this run
				location += *offset;
				*offset = 0;
				return false;
			}
		}
		else {
			IDocumentNode *node = IDocumentNode::CastToIDocumentNode (element);
			DependencyObjectCollection *doc_children = node->GetDocumentChildren();
			int children_count = doc_children ? doc_children->GetCount() : 0;
			if (children_count) {
				if (location == CONTENT_END)
					location = children_count;

				element = node->GetDocumentChildren()->GetValueAt(location-1)->AsTextElement();
				(*offset) ++;
				location = CONTENT_END;
				return true;
			}
			else {
				// not a Run, but also doesn't have children
				DependencyObject *parent = GetElementParent();

				if (!parent) {
					// we can't back up anymore.  result is NULL.
					element = NULL;
					return true;
				}

				IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
				(*offset) ++;
				location = parent_node->GetDocumentChildren()->IndexOf (Value (element));
				element = parent;
				return true;
			}
		}
	}
	else /* if offset > 0 */ {
		if (location == CONTENT_END) {
			// we're stepping forward but we're currently
			// at element's ContentEnd.  we need to step
			// up into our parent
			DependencyObject *parent = GetElementParent();

			if (!parent) {
				// we can't step forward anymore.  result is NULL.
				element = NULL;
				return true;
			}

			IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
			(*offset) --;
			location = parent_node->GetDocumentChildren()->IndexOf (Value (element));
			if ((int)location == parent_node->GetDocumentChildren()->GetCount() - 1)
				location = CONTENT_END;
			else
				location ++;

			element = parent;
			return true;
		}

		if (element->Is (Type::RUN)) {
			const char *text = ((Run*)element)->GetText();

			if (text == NULL) {
				(*offset) --;
				location = CONTENT_END;
				return true;
			}

			guint32 textlen = strlen (text);

			if (location + *offset >= textlen) {
				// bump up to the parent after subtracting the remaining length of the run from offset
				*offset = location + *offset - textlen;
				location = CONTENT_END;
				return true;
			}
			else {
				// the operation can be satisfied completely inside this run
				location += *offset;
				*offset = 0;
				return false;
			}
		}
		else {
			IDocumentNode *node = IDocumentNode::CastToIDocumentNode (element);
			DependencyObjectCollection *doc_children = node->GetDocumentChildren();
			int children_count = doc_children ? doc_children->GetCount() : 0;

			if (children_count > 0) {
				if ((int)location > children_count - 1) {
					DependencyObject *parent = GetElementParent();

					if (!parent) {
						// we can't step forward anymore.  result is NULL.
						element = NULL;
						return true;
					}

					IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
					(*offset) --;
					location = parent_node->GetDocumentChildren()->IndexOf (Value (element)) + 1;
					if ((int)location == children_count - 1)
						location = CONTENT_END;
					element = parent;
					return true;
				}
				else {
					element = node->GetDocumentChildren()->GetValueAt(location)->AsTextElement();
					(*offset) --;
					location = CONTENT_START;
					return true;
				}
			}
			else {
				// not a Run, but also doesn't have children
				DependencyObject *parent = GetElementParent();

				if (!parent) {
					// we can't step forward anymore.  result is NULL.
					element = NULL;
					return true;
				}

				IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
				(*offset) --;
				location = parent_node->GetDocumentChildren()->IndexOf (Value (element)) + 1;
				if ((int)location == children_count - 1)
					location = CONTENT_END;
				element = parent;
				return true;
			}
		}
	}
}


TextPointer *
TextPointer::GetPositionAtOffset (int offset, LogicalDirection dir) const
{
	PositionAtOffsetIterator iter ((TextElement*)GetParent(), GetLocation(), GetLogicalDirection());

	return iter.GetTextPointer (offset, dir);
}

TextPointer
TextPointer::GetPositionAtOffset_np (int offset, LogicalDirection dir) const
{
	TextPointer *tp = GetPositionAtOffset (offset, dir);
	if (tp == NULL)
		return *this;
	TextPointer rv = *tp;
	delete tp;
	return rv;
}

int
TextPointer::ResolveLocation () const
{
	if (location != CONTENT_END)
		return location;

	// FIXME double check this case
	if (parent == NULL)
		return 0;

	if (parent->Is (Type::RUN)) {
		const char *text = ((Run*)parent)->GetText();
		if (text == NULL)
			return 0;

		return strlen (text);
	}
	else {
		IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
		return parent_node->GetDocumentChildren()->GetCount();
	}
		
}

bool
TextPointer::GetIsAtInsertionPosition () const
{
	// FIXME we need a bunch more tests to see if this is correct, but it seems to work for the tests we have..
	if (parent != NULL)
		return parent->Is (Type::RUN);

	return false;
}

TextPointer
TextPointer::GetPositionInsideRun (int offset) const
{
	DependencyObject *parent = GetParent ();
	if (parent == NULL)
		return *this;

	if (offset > 0) {
		if (parent->Is(Type::RUN)) {
			int location = ResolveLocation ();
			const char *text = ((Run*)parent)->GetText();
			if ((guint)location < strlen (text))
				return GetPositionAtOffset_np (1, LogicalDirectionForward);
		}

		// we're at the end of the run (or not in a run at all).  we need to walk the document until we hit another run.
		DocumentWalker walker (IDocumentNode::CastToIDocumentNode (parent), DocumentWalker::Forward);
		walker.Step ();

		while (true) {
			IDocumentNode *node;
			DocumentWalker::StepType type = walker.Step (&node);

			switch (type) {
			case DocumentWalker::Done:
				// there is no position beyond this
				return *this;
			case DocumentWalker::Enter:
				if (node->AsDependencyObject()->Is(Type::RUN))
					return ((Run*)node->AsDependencyObject())->GetContentStart_np();
				break;
			case DocumentWalker::Leave:
				// do nothing here
				break;
			}
		}
	}
	else {
		if (parent->Is(Type::RUN)) {
			int location = ResolveLocation ();
			if (location > 0)
				return GetPositionAtOffset_np (-1, LogicalDirectionForward);
		}

		// we're at the start of the run (or not in a run at all).  we need to walk the document until we hit another run.
		DocumentWalker walker (IDocumentNode::CastToIDocumentNode (parent), DocumentWalker::Backward);
		walker.Step ();

		while (true) {
			IDocumentNode *node;
			DocumentWalker::StepType type = walker.Step (&node);

			switch (type) {
			case DocumentWalker::Done:
				// there is no position before this
				return *this;
			case DocumentWalker::Enter:
				if (node->AsDependencyObject()->Is(Type::RUN))
					return ((Run*)node->AsDependencyObject())->GetContentEnd_np();
				break;
			case DocumentWalker::Leave:
				// do nothing here
				break;
			}
		}
	}

	return *this;
}

RichTextBox *
TextPointer::GetRichTextBox () const
{
	DependencyObject *el = GetParent();
	while (el) {
		if (el->Is (Type::RICHTEXTBOX))
			return (RichTextBox*)el;
		el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
		if (!el)
			break;
	}

	return NULL;
}

#undef CONTENT_START
#undef CONTENT_END

};
