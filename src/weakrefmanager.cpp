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

	Clear ();
}

void
WeakRefBase::Clear ()
{
	if (field == NULL)
		return;

	/* clear old field value */
	if (field != NULL && field->AsEventObject ()) {
		field->AsEventObject ()->RemoveHandler (EventObject::DestroyedEvent, clear_weak_ref, this);
		if (storeInManaged && obj && obj->clearManagedRef && !obj->GetDeployment ()->IsShuttingDown ()) {
			/* We have to check if we're shutting down, since clearManagedRef is a managed callback */
			// Don't strengthen the Value* because we're going to delete it next.
			obj->clearManagedRef (obj, field, id);
		}
	}
	delete field;
	field = NULL;
}

void
WeakRefBase::Set (const EventObject *ptr)
{
	if (field == NULL && ptr == NULL)
		return;

	if (field != NULL)
		if (field->AsEventObject () == ptr)
			return;

#ifdef DEBUG_WEAKREF
	printf ("WeakRefBase::Set () %p changing field '%s::%p' from %p = %i = %s to %p = %i = %s\n",
		this, obj ? obj->GetTypeName () : NULL, id,
		field, GET_OBJ_ID (((EventObject *) field)), field ? ((EventObject *) field)->GetTypeName () : NULL,
		ptr, GET_OBJ_ID (((EventObject *) ptr)), ptr ? ((EventObject *) ptr)->GetTypeName () : NULL);
#endif

	/* clear old field value */
	Clear ();
	field = ptr ? new Value ((EventObject *) ptr) : NULL;

	/* set new field value */
	if (field != NULL) {
		field->AsEventObject ()->unref ();
		field->SetNeedUnref (false);

		if (storeInManaged && obj && obj->addManagedRef && !obj->GetDeployment ()->IsShuttingDown ()) {
			/* We have to check if we're shutting down, since addManagedRef is a managed callback */
			obj->addManagedRef (obj, field, id);
		}
		field->AsEventObject ()->AddHandler (EventObject::DestroyedEvent, clear_weak_ref, this);
	}
}

};