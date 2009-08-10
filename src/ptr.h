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

// to prevent unwanted assignments
class PtrBase {
private:
	void operator==(const PtrBase &b) const;
	void operator!=(const PtrBase &b) const;
};

/*************************************************************
 Keeps the object alive by reffing when created/assigned and
 unreffing when destroyed/reassigned.
**************************************************************/
template<class T>
class RefPtr : private PtrBase {
public:
	RefPtr(T* ptr = 0) : value(ptr), initted(false) {
		init();
	}
	RefPtr(const RefPtr& rhs) : value(rhs.get()), initted(false) {
		init();
	}
	template<class U>
	RefPtr(const RefPtr<U>& rhs) : value(rhs.get()), initted(false) {
		init();
	}
	~RefPtr() {
		ds(printf("~RefPtr %p %p %d\n", this, value, initted));
		if (value && initted)
			value->unref();
	}
	operator bool() const { return(value != 0); }

	RefPtr& operator=(const RefPtr& rhs) {
		if (value == rhs.value)
			return *this;
		T *old = value;
		value = rhs.value;
		init ();
		if (old) old->unref();
		return *this;
	}

	template<class U>
	RefPtr& operator=(const RefPtr<U>& rhs) {
		if (value == rhs.value)
			return *this;
		T *old = value;
		value = rhs.value;
		init ();
		if (old) old->unref();
		return *this;
	}

	T* get() const { return value; }
	T* operator->() const { return value; }
	T& operator*() const { return *value; }

	operator T*() { return value; }

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


/*************************************************************
 OwnerPtr takes ownership of a refcounted object, so it won't
 touch the initial refcount, it will only unref when destroyed
**************************************************************/
template<class T>
class OwnerPtr : private PtrBase {
public:
	OwnerPtr(T* ptr = 0) : value(ptr), initted(false) {
		init();
	}
	OwnerPtr(const OwnerPtr& rhs) : value(rhs.get()), initted(false) {
		init();
	}
	template<class U>
	OwnerPtr(const OwnerPtr<U>& rhs) : value(rhs.get()), initted(false) {
		init();
	}
	~OwnerPtr() {
		ds (printf("~OwnerPtr %p %p %d\n", this, value, initted));
		if (value && initted)
			value->unref();
	}
	operator bool() const { return(value != 0); }

	OwnerPtr* operator=(T* ptr) {
		if (value == ptr)
			return this;
		T *old = value;
		value = ptr;
		init ();
		if (old) old->unref();
		return this;
	}
	T* get() const { return value; }
	T* operator->() const { return value; }
	T& operator*() const { return *value; }

	operator T*() { return value; }

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
