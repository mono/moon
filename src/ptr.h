/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MONO_PTR_H__
#define __MONO_PTR_H__
// debug
#define ds(x)

#include "debug.h"

// to prevent unwanted assignments
class PtrBase {
private:
	void operator==(const PtrBase &b) const;
	void operator!=(const PtrBase &b) const;
	operator Value();
};

/*************************************************************
 DOPtr takes ownership of a refcounted object, so it won't
 touch the initial refcount, it will only unref when destroyed
**************************************************************/
template<class T>
class DOPtr : private PtrBase {
public:
	DOPtr(T* ptr = 0) : value(ptr), initted(false) {
		init();
	}

	~DOPtr() {
		ds (printf("~DOPtr %p %p %d\n", this, value, initted));
		if (value && initted)
			value->unref();
	}
	operator bool() const { return(value != 0); }

	DOPtr* operator=(T* ptr) {
		if (value == ptr)
			return this;
		T *old = value;
		value = ptr;
		if (old && initted) old->unref();
		initted = false;
		init ();
		return this;
	}
	T* get() const { return value; }
	T* operator->() const { return value; }
	T& operator*() const { return *value; }

	operator T*() { return value; }

	template <class U>
	operator U*() { return static_cast<U*> (value); }

private:
	T *value;
	bool initted;

	void init() {
		ds (printf("init %p %p\n", this, value));
		if (!value)
			return;
		initted = true;
	}
};

#endif
