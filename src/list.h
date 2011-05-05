/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * list.h: a non-sucky linked list implementation
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __LIST_H__
#define __LIST_H__

#include "pal.h"

namespace Moonlight {

class List {
public:
	class Node {
	public:
		Node *next;
		Node *prev;
		
		// public
		Node ();
		virtual ~Node () { }
	};

	template<typename T>
	class GenericNode : public Node {
	protected:
		T element;
	public:
		GenericNode () {}
		GenericNode (T element) : element (element) {}
		T GetElement () { return element; }
	};
	
	typedef bool (* NodeAction) (Node *node, void *data);

protected:
	int length;
	Node *head;
	Node *tail;
	
public:
	// constructors
	List ();
	virtual ~List ();
	
	// properties
	Node *First () { return head; }
	Node *Last ();
	bool IsEmpty ();
	int Length ();
	
	// methods
	void Clear (bool freeNodes);
	
	Node *Append (Node *node);
	Node *Prepend (Node *node);
	Node *Prepend (List *list);
	Node *Insert (Node *node, int index);
	Node *InsertAfter (Node *node, Node *after);
	Node *InsertBefore (Node *node, Node *before);
	
	Node *Replace (Node *node, int index);
	
	Node *Find (NodeAction find, void *data);
	void Remove (NodeAction find, void *data);
	void Remove (Node *node);
	void RemoveAt (int index);
	void Unlink (Node *node);
	
	Node *Index (int index);
	
	int IndexOf (Node *node);
	int IndexOf (NodeAction find, void *data);
	
	void ForEach (NodeAction action, void *data);
};

class Queue {
protected:
	MoonMutex lock;
	List *list;
	
public:
	Queue ();
	~Queue ();
	
	// convenience properties
	bool IsEmpty ();
	int Length ();
	
	// convenience methods
	void Clear (bool freeNodes);
	
	void Push (List::Node *node);
	List::Node *Pop ();
	
	void Lock ();
	void Unlock ();
	
	// accessing the internal linked list directly requires manual Locking/Unlocking.
	List *LinkedList ();

	// copies the queue and empties the original
	void  MoveTo (Queue &queue);
};

class Stack {
protected:
	List *list;
	int max_depth;

public:
	Stack (int max_depth = -1);
	~Stack ();

	bool IsEmpty ();

	void Clear (bool freeNodes);

	void Push (List::Node *node);
	List::Node *Pop ();

	List::Node *Top ();
};


class ArrayList {
private:
	void **array;
	int size; // size of array
	int count; // # of items in the array
	
public:
	ArrayList ();
	~ArrayList ();
	
	int GetCount () { return count; }
	void SetCount (int value);
	
	int GetCapacity ();
	void SetCapacity (int value);
	
	void EnsureCapacity (int capacity);
	int Add (void *item);
	void RemoveAt (int index);
	void *& operator [] (int index) { return array [index]; }
};

};
#endif /* __LIST_H__ */
