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
//#define DEBUG_WEAKREF

namespace Moonlight {

/*
 * WeakRefBase
 */

void
WeakRefBase::clear_weak_ref (EventObject *sender, EventArgs *callData, gpointer closure)
{
	((WeakRefBase *) closure)->ClearWeakRef ();
}

void
WeakRefBase::ClearWeakRef ()
{
#ifdef DEBUG_WEAKREF
	printf ("WeakRefBase::ClearWeakRef () %p clearing field '%s::%p' whose value is %p = %i = %s\n",
		this, obj ? obj->GetTypeName () : NULL, id, field, GET_OBJ_ID (field), field ? field->GetTypeName () : NULL);
#endif

	Set (NULL);
}

void
WeakRefBase::Set (EventObject *ptr)
{
	if (field == ptr)
		return;

#ifdef DEBUG_WEAKREF
	printf ("WeakRefBase::Set () %p changing field '%s::%p' from %p = %i = %s to %p = %i = %s\n",
		this, obj ? obj->GetTypeName () : NULL, id,
		field, GET_OBJ_ID (field), field ? field->GetTypeName () : NULL,
		ptr, GET_OBJ_ID (ptr), ptr ? ptr->GetTypeName () : NULL);
#endif

	if (field)
		field->RemoveHandler (EventObject::DestroyedEvent, clear_weak_ref, this);
	field = ptr;
	if (field)
		field->AddHandler (EventObject::DestroyedEvent, clear_weak_ref, this);

	if (storeInManaged && obj && obj->setManagedRef && !obj->GetDeployment ()->IsShuttingDown ()) {
		/* We have to check if we're shutting down, since setManagedRef is a managed callback */
		obj->setManagedRef (obj, field ? field->GetManagedHandle () : GCHandle::Zero, id);
	}
}

};