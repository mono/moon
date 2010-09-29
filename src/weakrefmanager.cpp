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
	printf ("WeakRefBase::ClearWeakRef () %p clearing field '%s::%s' whose value is %p = %i = %s\n",
		this, obj ? obj->GetTypeName () : NULL, name, field, GET_OBJ_ID (field), field ? field->GetTypeName () : NULL);
#endif

	Clear ();
}

void
WeakRefBase::Clear ()
{
	if (field == NULL)
		return;

	/* clear old field value */
	if (field != NULL) {
		field->RemoveHandler (EventObject::DestroyedEvent, clear_weak_ref, this);
		if (obj && obj->clearStrongRef && !obj->GetDeployment ()->IsShuttingDown ()) {
			/* We have to check if we're shutting down, since clearStrongRef is a managed callback */
			obj->clearStrongRef (obj, field, name);
		}
	}
	field = NULL;
}

void
WeakRefBase::Set (const EventObject *ptr)
{
	if (ptr == field)
		return;

#ifdef DEBUG_WEAKREF
	printf ("WeakRefBase::Set () %p changing field '%s::%s' from %p = %i = %s to %p = %i = %s\n",
		this, obj ? obj->GetTypeName () : NULL, name,
		field, GET_OBJ_ID (((EventObject *) field)), field ? ((EventObject *) field)->GetTypeName () : NULL,
		ptr, GET_OBJ_ID (((EventObject *) ptr)), ptr ? ((EventObject *) ptr)->GetTypeName () : NULL);
#endif

	/* clear old field value */
	Clear ();
	field = (EventObject *) ptr;
	/* set new field value */
	if (field != NULL) {
		if (obj && obj->addStrongRef && !obj->GetDeployment ()->IsShuttingDown ()) {
			/* We have to check if we're shutting down, since addStrongRef is a managed callback */
			obj->addStrongRef (obj, field, name);
		}
		field->AddHandler (EventObject::DestroyedEvent, clear_weak_ref, this);
	}
}

};