/*
 * list.cpp: a non-sucky linked list implementation
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "list.h"


List::Node::Node ()
{
	next = 0;
	prev = 0;
}


List::List ()
{
	length = 0;
	head = 0;
	tail = 0;
}


List::~List ()
{
	Clear (true);
}


List::Node *
List::First ()
{
	return head;
}


List::Node *
List::Last ()
{
	return tail;
}


bool
List::IsEmpty ()
{
	return !head;
}


int
List::Length ()
{
	return length;
}


void
List::Clear (bool freeNodes)
{
	if (freeNodes) {
		List::Node *n, *nn;
		
		n = head;
		while (n) {
			nn = n->next;
			delete n;
			n = nn;
		}
	}
	
	length = 0;
	head = 0;
	tail = 0;
}


List::Node *
List::Append (List::Node *node)
{
	node->prev = tail;
	node->next = 0;
	
	if (tail)
		tail->next = node;
	else
		head = node;
	
	tail = node;
	
	length++;
	
	return node;
}


List::Node *
List::Prepend (List::Node *node)
{
	node->next = head;
	node->prev = 0;
	
	if (head)
		head->prev = node;
	else
		tail = node;
	
	head = node;
	
	length++;
	
	return node;
}


List::Node *
List::Insert (List::Node *node, int index)
{
	List::Node *n = head;
	int i = 0;
	
	if (head) {
		while (n->next && i < index) {
			n = n->next;
			i++;
		}
		
		if (i == index) {
			// Inserting @node before @n
			node->prev = n->prev;
			node->next = n;
			
			if (n->prev)
				n->prev->next = node;
			else
				head = node;
			
			n->prev = node;
		} else {
			// Inserting @node after @n (means @n was the tail)
			tail = n->next = node;
			node->prev = n;
			node->next = 0;
		}
	} else {
		// @node will be the only node in the list
		head = tail = node;
		node->next = 0;
		node->prev = 0;
	}
	
	length++;
	
	return node;
}

List::Node *
List::InsertBefore (List::Node *node, List::Node *before)
{
	if (before == NULL)
		return Append (node);
	
	node->next = before;
	node->prev = before->prev;
	
	if (before->prev != NULL)
		before->prev->next = node;
	else
		head = node;
	
	before->prev = node;
	
	length++;
	
	return node;
}


List::Node *
List::Replace (List::Node *node, int index)
{
	List::Node *n;
	
	if (!(n = Index (index)))
		return 0;
	
	node->next = n->next;
	node->prev = n->prev;
	
	if (n->prev)
		n->prev->next = node;
	else
		head = node;
	
	if (n->next)
		n->next->prev = node;
	else
		tail = node;
	
	n->next = 0;
	n->prev = 0;
	
	return n;
}

List::Node *
List::Find (NodeAction find, void *data)
{
	List::Node *n = head;
	
	if (!find)
		return 0;
	
	while (n) {
		if (find (n, data))
			return n;
		
		n = n->next;
	}
	
	return 0;
}


void
List::Remove (NodeAction find, void *data)
{
	List::Node *n;
	
	if ((n = Find (find, data))) {
		Unlink (n);
		delete n;
	}
}

void
List::Remove (List::Node *node)
{
	Unlink (node);
	delete node;
}

void
List::RemoveAt (int index)
{
	List::Node *node;
	if (!(node = Index (index)))
		return;
	
	Unlink (node);
	delete node;
}

void
List::Unlink (List::Node *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		head = node->next;
	
	if (node->next)
		node->next->prev = node->prev;
	else
		tail = node->prev;
	
	node->prev = 0;
	node->next = 0;
	
	length--;
}


List::Node *
List::Index (int index)
{
	List::Node *n = head;
	int i = 0;
	
	if (index < 0)
		return 0;
	
	while (n && i < index) {
		n = n->next;
		i++;
	}
	
	if (i == index)
		return n;
	
	return 0;
}

int
List::IndexOf (List::Node *node)
{
	List::Node *n = head;
	int i = 0;
	
	while (n && n != node) {
		n = n->next;
		i++;
	}
	
	return n == node ? i : -1;
}


int
List::IndexOf (NodeAction find, void *data)
{
	List::Node *n = head;
	int i = 0;
	
	if (!find)
		return -1;
	
	while (n) {
		if (find (n, data))
			return i;
		
		n = n->next;
		i++;
	}
	
	return -1;
}

void
List::ForEach (NodeAction action, void *data)
{
	List::Node *node = head;
	bool move = true;

	if (!action)
		return;

	while (node && move) {
		if (!action (node, data))
			move = false;
		else
			node = node->next;
	}
}


Queue::Queue ()
{
	pthread_mutex_init (&lock, NULL);
	list = new List ();
}

Queue::~Queue ()
{
	pthread_mutex_destroy (&lock);
	delete list;
}

bool
Queue::IsEmpty ()
{
	bool empty;
	
	Lock ();
	empty = list->IsEmpty ();
	Unlock ();
	
	return empty;
}

int
Queue::Length ()
{
	int length;
	
	Lock ();
	length = list->Length ();
	Unlock ();
	
	return length;
}

void
Queue::Clear (bool freeNodes)
{
	Lock ();
	list->Clear (freeNodes);
	Unlock ();
}

void
Queue::Push (List::Node *node)
{
	Lock ();
	list->Append (node);
	Unlock ();
}

List::Node *
Queue::Pop ()
{
	List::Node *node;
	
	Lock ();
	if ((node = list->First ()))
		list->Unlink (node);
	Unlock ();
	
	return node;
}

void
Queue::Lock ()
{
	pthread_mutex_lock (&lock);
}

void
Queue::Unlock ()
{
	pthread_mutex_unlock (&lock);
}

List *
Queue::LinkedList ()
{
	return list;
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

static bool
IntNodeFinder (List::Node *node, void *data)
{
	int val = *((int *) data);
	
	return ((IntNode *) node)->id == val;
}


int main (int argc, char **argv)
{
	List::Node *node;
	List *list;
	
	list = new List ();
	
	node = list->Append (new IntNode (2));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Append (new IntNode (4));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Append (new IntNode (5));
	printf ("appended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Insert (new IntNode (3), 1);
	printf ("inserted node with id = %d at index = %d\n",
		((IntNode *) node)->id, list->IndexOf (node));
	node = list->Prepend (new IntNode (1));
	printf ("prepended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Prepend (new IntNode (0));
	printf ("prepended node with id = %d\n", ((IntNode *) node)->id);
	node = list->Insert (new IntNode (6), 6);
	printf ("inserted node with id = %d at index = %d\n",
		((IntNode *) node)->id, list->IndexOf (node));
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->next)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->prev)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nunlinking the last item in the list\n");
	list->Unlink (list->Last ());
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->next)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->prev)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nunlinking the first item in the list\n");
	list->Unlink (list->First ());
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->next)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->prev)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nreplacing 4 with 8\n");
	int id = 4;
	int index = list->IndexOf (IntNodeFinder, &id);
	if ((node = list->Replace (new IntNode (8), index)))
		delete node;
	else
		printf ("unsuccessful\n");
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->next)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->prev)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nremoving 5\n");
	id = 5;
	list->Remove (IntNodeFinder, &id);
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->next)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->prev)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nremoving 1\n");
	id = 1;
	list->Remove (IntNodeFinder, &id);
	
	printf ("\nlist contains (in order):\n");
	for (node = list->First (); node != NULL; node = node->next)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	printf ("\nlist contains (in reverse order):\n");
	for (node = list->Last (); node != NULL; node = node->prev)
		printf ("node id = %d\n", ((IntNode *) node)->id);
	
	list->Clear (true);
	delete list;
	
	return 0;
}

#endif /* TEST_PROGRAM */
