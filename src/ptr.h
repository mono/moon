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
	explicit RefPtr(T* ptr = 0);
	RefPtr(const RefPtr& rhs);
	~RefPtr() { if (value) value->unref(); }
	operator bool() const { return(value != 0); }
	RefPtr& operator=(const RefPtr& rhs) {
		if (value == rhs.value)
			return *this;
		T *old = value;
		value = rhs.value;
		if (value) value->ref();
		if (old) old->unref();
		return *this;
	}
	T* operator->() const { return value; }
	T& operator*() const { return *value; }
private:
	T *value;
};

template<class T>
RefPtr<T>::RefPtr(T* ptr) : value(ptr) {
	if (value) value->ref();
}

template<class T>
RefPtr<T>::RefPtr(const RefPtr& rhs) : value(rhs.value) {
	if (value) value->ref();
}


/*************************************************************
 OwnerPtr takes ownership of a refcounted object, so it won't
 touch the initial refcount, it will only unref when destroyed
**************************************************************/
template<class T>
class OwnerPtr : private PtrBase {
public:
	OwnerPtr(T* ptr = 0);
	OwnerPtr(const OwnerPtr& rhs);
	~OwnerPtr() { if (value) value->unref(); }
	operator bool() const { return(value != 0); }
	OwnerPtr& operator=(const OwnerPtr& rhs) {
		if (value == rhs.value)
			return *this;
		T *old = value;
		value = rhs.value;
		if (old) old->unref();
		return *this;
	}
	T* operator->() const { return value; }
	T& operator*() const { return *value; }

	operator T*() { return value; }

private:
	T *value;
};

template<class T>
OwnerPtr<T>::OwnerPtr(T* ptr) : value(ptr) {
}

template<class T>
OwnerPtr<T>::OwnerPtr(const OwnerPtr& rhs) : value(rhs.value) {
}

#endif
