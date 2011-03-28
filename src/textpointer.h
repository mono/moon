/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textpointer.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXTPOINTER_H__
#define __TEXTPOINTER_H__

#include <glib.h>
#include "enums.h"
#include "rect.h"
#include "error.h"
#include "weakrefmanager.h"

#define CONTENT_START (0)
#define CONTENT_END ((guint32)-1)

namespace Moonlight {

class DependencyObject;
class DependencyObjectCollection;
class TextPointer;
class RichTextBox;
class TextElement;

class IDocumentNode {
public:
	// splits this node at loc, with locations 0-loc remaining in @this, and (loc+1)... moving to the return value
	// if @into is provided, the elements from (loc+1)... are reparented into it and it is returned.
	virtual DependencyObject *Split (int loc, TextElement *into = NULL) = 0;

	virtual IDocumentNode *GetParentDocumentNode () = 0;
	virtual DependencyObjectCollection *GetDocumentChildren () = 0;

	virtual void SerializeText (GString *str) = 0;
	virtual void SerializeXaml (GString *str) = 0;
	virtual void SerializeXamlProperties (bool force, GString *str) = 0;

	virtual void SerializeXamlStartElement (GString* str) = 0;
	virtual void SerializeXamlEndElement (GString* str) = 0;

	virtual DependencyObject *AsDependencyObject() = 0;
	static IDocumentNode* CastToIDocumentNode (DependencyObject *obj);
};

class DocumentWalker {
public:
	enum Direction {
		Forward,
		Backward
	};

	DocumentWalker (IDocumentNode *node, Direction direction);
	~DocumentWalker ();

	enum StepType {
		Enter,
		Leave,
		Done
	};

	StepType Step (IDocumentNode **node_return = NULL);
	IDocumentNode *GetNode ();

private:
	IDocumentNode *node;
	int child_index;
	Direction direction;
};


class TextPointer {
public:
	TextPointer ()
		: parent (), location (0), direction (LogicalDirectionBackward)
	{}

	TextPointer (const TextPointer& pointer)
		: parent (), location (pointer.location), direction (pointer.direction)
	{
		parent = pointer.parent.GetFieldValue ();
	}

	TextPointer (DependencyObject *parent, gint32 location, LogicalDirection direction)
		: parent (), location (location), direction (direction)
	{
		this->parent = parent;
	}

	~TextPointer () {}

	/* @GeneratePInvoke */
	static void Free (TextPointer *pointer) { delete pointer; }

	/* @GeneratePInvoke */
	int CompareToWithError (const TextPointer *pointer, MoonError *error) const;
	/* @GeneratePInvoke */
	Rect GetCharacterRect (LogicalDirection dir) const;
	/* @GeneratePInvoke */
	TextPointer *GetNextInsertionPosition (LogicalDirection dir) const;
	/* @GeneratePInvoke */
	TextPointer *GetPositionAtOffset (int offset, LogicalDirection dir) const;

	TextPointer GetNextInsertionPosition_np (LogicalDirection dir) const;
	TextPointer GetPositionAtOffset_np (int offset, LogicalDirection dir) const;
	int CompareTo_np (const TextPointer& pointer) const;
	int CompareTo (const TextPointer *pointer);

	bool Equal (const TextPointer& pointer) const;

	/* @GeneratePInvoke */
	bool GetIsAtInsertionPosition () const;
	/* @GeneratePInvoke */
	LogicalDirection GetLogicalDirection () const { return direction; }
	/* @GeneratePInvoke */
	guint32 GetLocation () const { return location; }
	/* @GeneratePInvoke */
	DependencyObject *GetParent () const { return parent; }

	IDocumentNode *GetParentNode () const { return IDocumentNode::CastToIDocumentNode (parent); }

	TextPointer GetPositionInsideRun (int offset) const;

	int ResolveLocation () const;

	RichTextBox *GetRichTextBox () const;

	TextPointer& operator=(const TextPointer& wr) // assignment operator
	{
		if (this != &wr) {
			parent = wr.parent.GetFieldValue ();
			location = wr.location;
			direction = wr.direction;
		}
		return *this;
	}

private:
	WeakRef<DependencyObject> parent;
	guint32 location;
	LogicalDirection direction;
};

};

#endif /* __TEXTPOINTER_H__ */
