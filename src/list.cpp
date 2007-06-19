/*
 * list.cpp: a non-sucky linked list implementation
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "list.h"


List::Node::Node ()
{
	next = this;
	prev = this;
}


List::Node *
List::Node::Next ()
{
	return next->next ? next : 0;
}


List::Node *
List::Node::Prev ()
{
	return prev->prev ? prev : 0;
}


List::Node *
List::Node::Unlink ()
{
	next->prev = prev;
	prev->next = next;
	
	return this;
}


List::List ()
{
	head = (List::Node *) &tail;
	tail = 0;
	tailpred = (List::Node *) &head;
}


List::Node *
List::First ()
{
	return head->next ? head : 0;
}


List::Node *
List::Last ()
{
	return head->next ? tailpred : 0;
}


bool
List::IsEmpty ()
{
	return head == (List::Node *) &tail;
}


int
List::Length ()
{
	List::Node *node;
	int n = 0;
	
	node = head;
	while (node->next) {
		node = node->next;
		n++;
	}
	
	return n;
}


void
List::Clear (bool freeNodes)
{
	if (freeNodes) {
		List::Node *n, *nn;
		
		n = head;
		while (n->next) {
			nn = n->next;
			delete n;
			n = nn;
		}
	}
	
	head = (List::Node *) &tail;
	tail = 0;
	tailpred = (List::Node *) &head;
}


List::Node *
List::Append (List::Node *node)
{
	node->next = (List::Node *) &tail;
	node->prev = tailpred;
	tailpred->next = node;
	tailpred = node;
	
	return node;
}


List::Node *
List::Prepend (List::Node *node)
{
	node->next = head;
	node->prev = (List::Node *) &head;
	head->prev = node;
	head = node;
	
	return node;
}


List::Node *
List::Insert (List::Node *node, int index)
{
	List::Node *n = head;
	int i = 0;
	
	while (n->next && i < index) {
		n = n->next;
		i++;
	}
	
	node->prev = n->prev;
	node->next = n;
	n->prev->next = node;
	n->prev = node;
	
	return node;
}


List::Node *
List::InsertSorted (List::Node *node, NodeComparer cmp)
{
	List::Node *n = head;
	
	if (!cmp)
		return Append (node);
	
	while (n->next && cmp (n, node) < 0)
		n = n->next;
	
	node->prev = n->prev;
	node->next = n;
	n->prev->next = node;
	n->prev = node;
	
	return node;
}


List::Node *
List::InsertSorted (List::Node *node, NodeComparer cmp, bool stable)
{
	List::Node *n = head;
	
	if (!cmp)
		return Append (node);
	
	if (!stable)
		return InsertSorted (node, cmp);
	
	while (n->next && cmp (n, node) <= 0)
		n = n->next;
	
	node->prev = n->prev;
	node->next = n;
	n->prev->next = node;
	n->prev = node;
	
	return node;
}


List::Node *
List::Find (NodeFinder find, void *data)
{
	List::Node *n = head;
	
	if (!find)
		return 0;
	
	while (n->next) {
		if (find (n, data))
			return n;
		
		n = n->next;
	}
	
	return 0;
}


void
List::Remove (NodeFinder find, void *data)
{
	List::Node *n;
	
	if ((n = Find (find, data))) {
		n->Unlink ();
		delete n;
	}
}


int
List::IndexOf (List::Node *node)
{
	List::Node *n = head;
	int i = 0;
	
	while (n->next && n != node) {
		n = n->next;
		i++;
	}
	
	return n == node ? i : -1;
}


int
List::IndexOf (NodeFinder find, void *data)
{
	List::Node *n = head;
	int i = 0;
	
	if (!find)
		return -1;
	
	while (n->next) {
		if (find (n, data))
			return i;
		
		n = n->next;
		i++;
	}
	
	return -1;
}




void
Stack::Push (List::Node *node)
{
	Prepend (node);
}


List::Node *
Stack::Pop ()
{
	List::Node *n = head;
	
	if (!n->next)
		return 0;
	
	n->next->prev = n->prev;
	head = n->next;
	
	return n;
}


//#define TEST_PROGRAM
#ifdef TEST_PROGRAM

#include <stdio.h>

class IntNode : public List::Node {
public:
	int id;
	
	IntNode (int i) { id = i; }
};


static int
IntNodeCompare (List::Node *n0, List::Node *n1)
{
	IntNode *in0 = (IntNode *) n0;
	IntNode *in1 = (IntNode *) n1;
	
	return in0->id - in1->id;
}


int main (int argc, char **argv)
{
	List::Node *node;
	List *list;
	
	list = new List ();
	
	node = list->Append (new IntNode (1));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Append (new IntNode (4));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Append (new IntNode (5));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Insert (new IntNode (2), 1);
	printf ("inserted node with id = %d at index = %d\n",
		((IntNode *) node)->id, list->IndexOf (node));
	node = list->Prepend (new IntNode (0));
	printf ("prepended node with id = %d\n", ((IntNode *) node)->id);
	node = list->InsertSorted (new IntNode (3), IntNodeCompare);
	printf ("insert sorted node with id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->Next ())
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->Prev ())
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	list->Clear (true);
	delete list;
	
	return 0;
}

#endif /* TEST_PROGRAM */
