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
	Value *field;
	const char *name;
	WeakRefBase ()
	{
		this->obj = NULL;
		this->name = NULL;
		this->field = NULL;
	}
	WeakRefBase (EventObject *obj, const char *name)
	{
		this->obj = obj;
		this->name = name;
		this->field = NULL;
	}
	~WeakRefBase ()
	{
		Set (NULL);
	}
	static void clear_weak_ref (EventObject *sender, EventArgs *callData, gpointer closure);
	void ClearWeakRef ();
	void Set (const EventObject *ptr);
	void Clear ();

public:
	EventObject *GetFieldValue () { return field ? field->AsEventObject () : NULL; }
};

template<typename EO>
class WeakRef : public WeakRefBase {
public:
	WeakRef ()
	{
	}
	WeakRef (EventObject *obj, const char *name)
		: WeakRefBase (obj, name)
	{
	}

	operator EO* () const { return (EO *) (field ? field->AsEventObject () : NULL); }
	void operator=(const EO *ptr) { Set (ptr); }

	EO* operator->() const { return (EO *) (field ? field->AsEventObject () : NULL); }

private:
	// Disallow the following operations
	WeakRef operator=(const WeakRef wr); // assignment operator
	EO& operator*(); // dereference operator
	WeakRef (const WeakRef& wr); // copy ctor
};

};

#endif // __MOON_WEAKREFMANAGER_H__
