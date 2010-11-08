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

#define CONTENT_START (0)
#define CONTENT_END ((guint32)-1)

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

int
TextPointer::CompareTo (TextPointer *pointer)
{
	if (this->GetParent() == pointer->GetParent()) {
		return compare_locations (this->GetLocation(), pointer->GetLocation());
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

			return pointer->GetLocation() >= child_index ? -1 : 1;
		}
		else if (count_to_compare < pointer_array->len) {
			// @pointer's parent is a child of this_array[count_to_compare-1]
			DependencyObject *parent = (DependencyObject*)g_ptr_array_index(this_array, count_to_compare - 1);
			IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
			guint32 child_index = parent_node->GetDocumentChildren()->IndexOf (Value ((DependencyObject*)g_ptr_array_index(pointer_array, count_to_compare)));

			return child_index >= this->GetLocation () ? -1 : 1;
		}
	}
}

int
TextPointer::CompareTo_np (TextPointer pointer)
{
	return CompareTo (&pointer);
}

Rect
TextPointer::GetCharacterRect (LogicalDirection dir)
{
	DependencyObject *el = GetParent();
	while (el) {
		if (el->Is (Type::RICHTEXTBOX))
			break;
		el = el->GetParent() ? el->GetParent()->GetParent() : NULL;
		if (!el)
			break;
	}
	if (!el) {
		g_warning ("a TextPointer outside of a RichTextBox?  say it ain't so...");
		return Rect ();
	}

	return ((RichTextBox*)el)->GetCharacterRect (this, dir);
}

TextPointer *
TextPointer::GetNextInsertionPosition (LogicalDirection dir)
{
	// FIXME: implement this
	return NULL;
}

TextPointer
TextPointer::GetNextInsertionPosition_np (LogicalDirection dir)
{
	// FIXME: implement this
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
	if (element == NULL) {
		g_warning ("element is null, shouldn't happen. bailing out.");
		return false;
	}

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
			if (location == parent_node->GetDocumentChildren()->GetCount() - 1)
				location = CONTENT_END;
			else
				location ++;

			element = parent;
			return true;
		}

		if (element->Is (Type::RUN)) {
			const char *text = ((Run*)element)->GetText();

			if (text == NULL) {
				*offset --;
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

			if (children_count) {
				if (location > children_count - 1) {
					DependencyObject *parent = GetElementParent();

					if (!parent) {
						// we can't step forward anymore.  result is NULL.
						element = NULL;
						return true;
					}

					IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
					(*offset) --;
					location = parent_node->GetDocumentChildren()->IndexOf (Value (element));
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
				location = parent_node->GetDocumentChildren()->IndexOf (Value (element));
				element = parent;
				return true;
			}
		}
	}
#undef CONTENT_START
#undef CONTENT_END
}


TextPointer *
TextPointer::GetPositionAtOffset (int offset, LogicalDirection dir)
{
	PositionAtOffsetIterator iter ((TextElement*)GetParent(), GetLocation(), GetLogicalDirection());

	return iter.GetTextPointer (offset, dir);
}

TextPointer
TextPointer::GetPositionAtOffset_np (int offset, LogicalDirection dir)
{
	TextPointer *tp = GetPositionAtOffset (offset, dir);
	if (tp == NULL)
		return *this;
	TextPointer rv = *tp;
	delete tp;
	return rv;
}

int
TextPointer::ResolveLocation ()
{
	if (location != (guint32)-1)
		return location;

	if (parent->Is (Type::RUN)) {
		return strlen (((Run*)parent)->GetText());
	}
	else {
		IDocumentNode *parent_node = IDocumentNode::CastToIDocumentNode (parent);
		return parent_node->GetDocumentChildren()->GetCount();
	}
		
}

bool
TextPointer::GetIsAtInsertionPosition ()
{
	if (parent->Is (Type::PARAGRAPH)) {
		return location != 0;
	}
	else {
		// FIXME we need a bunch more tests to see if this is correct, but it seems to work for the tests we have..
		return parent->Is (Type::RUN);
	}
}

int
TextPointer::Comparer (gconstpointer tpp1, gconstpointer tpp2)
{
	TextPointer *tp1 = *(TextPointer **)tpp1;
	TextPointer *tp2 = *(TextPointer **)tpp2;

	int location_diff = tp1->GetLocation() - tp2->GetLocation ();

	if (location_diff == 0) {
		if (tp1->GetLogicalDirection() == tp2->GetLogicalDirection())
			return 0;
		else if (tp1->GetLogicalDirection () == LogicalDirectionBackward)
			return -1;
		else
			return 1;
	}
	else if (location_diff < 0) {
		return -1;
	}
	else {
		return 1;
	}
}


};
