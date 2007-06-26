/*
 * list.h: a non-sucky linked list implementation
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __LIST_H__
#define __LIST_H__


class List {
public:
	class Node {
	public:
		// these should be treated as private
		Node *next;
		Node *prev;
		
		// public
		Node ();
		
		Node *Next ();
		Node *Prev ();
		
		Node *Unlink ();
	};
	
	typedef int (* NodeComparer) (Node *n0, Node *n1);
	typedef bool (* NodeFinder) (Node *node, void *data);
	
protected:
	Node *head;
	Node *tail;
	Node *tailpred;
public:
	// constructors
	List ();
	
	// properties
	Node *First ();
	Node *Last ();
	bool IsEmpty ();
	int Length ();
	
	// methods
	void Clear (bool freeNodes);
	
	Node *Append (Node *node);
	Node *Prepend (Node *node);
	Node *Insert (Node *node, int index);
	Node *InsertSorted (Node *node, NodeComparer cmp);
	Node *InsertSorted (Node *node, NodeComparer cmp, bool stable);
	
	Node *Replace (Node *node, int index);
	
	Node *Find (NodeFinder find, void *data);
	void Remove (NodeFinder find, void *data);
	
	
	Node *Index (int index);
	
	int IndexOf (Node *node);
	int IndexOf (NodeFinder find, void *data);
};


class Stack : public List {
public:
	// constructors
	Stack () { }
	
	// methods
	void Push (Node *node);
	Node *Pop ();
};


#endif /* __LIST_H__ */
