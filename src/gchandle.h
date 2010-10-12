/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gchandle.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_GCHANDLE_H__
#define __MOON_GCHANDLE_H__

#include <glib.h>

namespace Moonlight {

struct GCHandle {
private:
	void *ptr;

public:
	GCHandle () { ptr = NULL; }
	GCHandle (int handle) { ptr = GINT_TO_POINTER (handle); }
	GCHandle (void *handle) { ptr = handle; }
	//GCHandle (const GCHandle &e) { ptr = e.ptr; }
	//GCHandle& operator= (const GCHandle& e) { ptr = e.ptr; return *this; }

	void *ToIntPtr () const { return ptr; }
	int ToInt () const { return GPOINTER_TO_INT (ptr); }
	bool IsAllocated () const { return ptr != NULL; }
	bool IsWeak () const { return (ToInt () & 7) - 1 == 0; }
	bool IsNormal () const { return (ToInt () & 7) - 1 == 2; }
	void Clear () { ptr = NULL; }

	static GCHandle Zero;
};

};

#endif