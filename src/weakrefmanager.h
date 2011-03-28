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

namespace Moonlight {

class EventObject;

class WeakRefBase {
protected:
	EventObject *obj;
	EventObject *field;
	const void *id;

	WeakRefBase (EventObject *obj = NULL, const void *id = NULL)
	{
		this->obj = obj;
		this->id = id;
		this->field = NULL;
	}
	~WeakRefBase ()
	{
		Set (NULL);
	}
	static void clear_weak_ref (EventObject *sender, EventArgs *callData, gpointer closure);
	void ClearWeakRef ();
	void Set (EventObject *ptr);

public:
	EventObject *GetFieldValue () { return field; }
};

template<typename EO>
class WeakRef : public WeakRefBase {
public:

	WeakRef (EventObject *obj = NULL, const void *id = NULL)
		: WeakRefBase (obj, id)
	{
	}

	operator EO* () const { return (EO *) (field); }
	void operator=(EO *ptr) { Set (ptr); }

	EO* operator->() const { return (EO *) (field); }
	EO* GetFieldValue () const { return (EO *) field; }

private:
	// Disallow the following operations
	WeakRef operator=(const WeakRef wr); // assignment operator
	EO& operator*(); // dereference operator
	WeakRef (const WeakRef& wr); // copy ctor
};

};

#endif // __MOON_WEAKREFMANAGER_H__
