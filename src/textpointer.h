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

namespace Moonlight {

class DependencyObject;
class DependencyObjectCollection;
class TextPointer;

class IDocumentNode {
public:
	virtual DependencyObjectCollection *GetDocumentChildren () = 0;

	virtual void AddTextPointer (TextPointer *pointer) = 0;
	virtual void RemoveTextPointer (TextPointer *pointer) = 0;

	virtual char* Serialize () = 0;

	static IDocumentNode* CastToIDocumentNode (DependencyObject *obj);
};



class TextPointer {
public:
	TextPointer ()
		: parent (NULL), location (0), direction (LogicalDirectionBackward)
	{ }

	TextPointer (const TextPointer& pointer)
		: parent (pointer.parent), location (pointer.location), direction (pointer.direction)
	{ }

	TextPointer (DependencyObject *parent, gint32 location, LogicalDirection direction)
		: parent (parent), location (location), direction (direction)
	{ }

	~TextPointer () {}

	static int Comparer (gconstpointer tpp1, gconstpointer tpp2);

	/* @GeneratePInvoke */
	static void Free (TextPointer *pointer) { delete pointer; }

	/* @GeneratePInvoke */
	int CompareTo (TextPointer *pointer);
	/* @GeneratePInvoke */
	Rect GetCharacterRect (LogicalDirection dir);
	/* @GeneratePInvoke */
	TextPointer *GetNextInsertionPosition (LogicalDirection dir);
	/* @GeneratePInvoke */
	TextPointer *GetPositionAtOffset (int offset, LogicalDirection dir);

	TextPointer GetNextInsertionPosition_np (LogicalDirection dir);
	TextPointer GetPositionAtOffset_np (int offset, LogicalDirection dir);
	int CompareTo_np (TextPointer pointer);

	/* @GeneratePInvoke */
	bool GetIsAtInsertionPosition ();
	/* @GeneratePInvoke */
	LogicalDirection GetLogicalDirection () { return direction; }
	/* @GeneratePInvoke */
	guint32 GetLocation () { return location; }
	/* @GeneratePInvoke */
	DependencyObject *GetParent () { return parent; }

	IDocumentNode *GetParentNode () { return IDocumentNode::CastToIDocumentNode (parent); }

	int ResolveLocation ();

private:
	DependencyObject *parent; // weakref?
	guint32 location;
	LogicalDirection direction;
};

};

#endif /* __TEXTPOINTER_H__ */
