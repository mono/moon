/*
 * weakrefmanager.h: 
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_WEAKREFMANAGER_H__
#define __MOON_WEAKREFMANAGER_H__

#include "list.h"

namespace Moonlight {

class EventObject;
class EventArgs;

class WeakRefManager {
public:
	WeakRefManager (EventObject *forObj);
	~WeakRefManager ();

	void Add (EventObject **obj, const char *name);
	void Clear (EventObject **obj, const char *name);

private:
	EventObject *forObj;

	List *weakrefs;

	static void clear_weak_ref (EventObject *sender, EventArgs *callData, gpointer closure);
	void ClearWeakRef (EventObject **obj, const char *name, bool nullRef, bool removeHandler);
};

};

#endif // __MOON_WEAKREFMANAGER_H__
