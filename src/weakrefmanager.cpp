/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * weakrefmanager.cpp: 
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <stdio.h>

#include "deployment.h"
#include "dependencyobject.h"
#include "eventargs.h"

namespace Moonlight {

class WeakRefNode : public List::Node
{
public:
	WeakRefNode (EventObject **obj, const char *name, int token)
	{
		this->obj = obj;
		this->o = *obj;
		this->token = token;
		this->name = name;
	}

	EventObject *o;
	EventObject **obj;
	int token;
	const char *name;
};

class WeakRefClosure {
public:
	WeakRefClosure (WeakRefManager *mgr, EventObject **obj, const char *name)
	{
		this->mgr = mgr;
		this->obj = obj;
		this->o = *obj;
		this->name = name;
	}

	~WeakRefClosure ()
	{
		this->mgr = NULL;
		this->obj = NULL;
		this->o = NULL;
		this->name = NULL;
	}

	static void delete_closure (WeakRefClosure *closure)
	{
		delete closure;
	}

	WeakRefManager *mgr;
	EventObject **obj;
	EventObject *o;
	const char *name;
};

WeakRefManager::WeakRefManager (EventObject *forObj)
{
	this->forObj = forObj;
	weakrefs = new List ();
#if DEBUG_WEAKREF
	if (forObj->GetObjectType() == Type::DEPLOYMENT)
		printf ("weakrefmanager %p forobj= %p/Deployment\n", this, forObj);
	else
		printf ("weakrefmanager %p forobj= %p/%s\n", this, forObj, forObj->GetTypeName());
#endif
}

WeakRefManager::~WeakRefManager ()
{
#if DEBUG_WEAKREF
	printf ("~weakrefmanager %p forobj= %p/%s\n", this, forObj, forObj->GetTypeName());
#endif

	Deployment *depl = Deployment::GetCurrent();

	// depl can be null if we're being destroyed from ~Deployment
	if (depl && !depl->IsShuttingDown ()) {
		// iterate over the list unregistering handlers
		while (weakrefs->First()) {
			WeakRefNode *n = (WeakRefNode*)weakrefs->First();

			n->o->RemoveHandler (EventObject::DestroyedEvent, n->token);

			weakrefs->Remove (n);
		}
	}
	delete weakrefs;

	weakrefs = NULL;
	forObj = NULL;
}

void
WeakRefManager::Add (EventObject **obj, const char *name)
{
	if (!obj || !*obj)
		return;

	EventObject *o = *obj;

	for (List::Node *n = weakrefs->First(); n; n = n->next) {
		WeakRefNode *wrn = (WeakRefNode*)n;
		if (wrn->obj == obj)
			// we already have a weakref for this
			// particular pointer.  we don't need another.
			return;
	}

	int token = o->AddHandler (EventObject::DestroyedEvent,
				   WeakRefManager::clear_weak_ref,
				   new WeakRefClosure (this, obj, name),
				   (GDestroyNotify)WeakRefClosure::delete_closure);

#if DEBUG_WEAKREF
	printf ("WeakRefManager %p adding ref %p, o = %p, name = %s, token = %d\n", this, obj, o, name, token);
#endif

	weakrefs->Append (new WeakRefNode (obj, name, token));
}

void
WeakRefManager::Clear (EventObject **obj, const char *name)
{
	if (!obj || !*obj)
		return;

	ClearWeakRef (obj, name, false, true);
}

void
WeakRefManager::ClearWeakRef (EventObject **obj, const char *name, bool nullRef, bool removeHandler)
{
	EventObject *o = *obj;

#if DEBUG_WEAKREF
	printf ("Clearing weakref %p, o = %p/%s\n", obj, o, o->GetTypeName());
#endif

	// null out the reference
	if (nullRef) {
#if DEBUG_WEAKREF
		printf ("nulling out %p, o = %p\n", obj, o);
#endif
		*obj = NULL;
	}

	// now remove the WeakRefNode from our list
	List::Node *n = weakrefs->First();
	if (!n) {
#if DEBUG_WEAKREF
		printf ("ClearWeakRef called, but no weakrefs registered.\n");
#endif
		return;
	}

#if SANITY
	bool first = true;
#endif
	while (n) {
		WeakRefNode *wrn = (WeakRefNode*)n;
		List::Node *next = n->next;

		if (wrn->obj == obj) {
#if SANITY
			if (!first)
				abort ();

			first = true;
#endif
			if (removeHandler && !Deployment::GetCurrent()->IsShuttingDown())
				o->RemoveHandler (EventObject::DestroyedEvent, wrn->token);

			// remove the list node
			weakrefs->Remove (n);
		}

		n = next;
	}
}

void
WeakRefManager::clear_weak_ref (EventObject *sender, EventArgs *callData, gpointer closure)
{
	WeakRefClosure *wrclosure = (WeakRefClosure*)closure;
	WeakRefManager *mgr = wrclosure->mgr;

#if DEBUG_WEAKREF
	printf ("weakrefmanager::clear_weak_ref (mgr = %p,  obj = %p, name = %s)\n", mgr, wrclosure->obj, wrclosure->name);
#endif

	EventObject **obj = wrclosure->obj;

	if (!obj || !*obj) {
#if DEBUG_WEAKREF
		printf (" + nuh uh!\n");
#endif
		return;
	}

	mgr->ClearWeakRef (obj, wrclosure->name, true, false);
}


};
