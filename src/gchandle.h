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
	guint32 handle;

public:
	GCHandle () { handle = 0; }
	GCHandle (guint32 handle) { this->handle = handle; }
	GCHandle (void *handle) { this->handle = GPOINTER_TO_UINT (handle); }
	//GCHandle (const GCHandle &e) { handle = e handle; }
	//GCHandle& operator= (const GCHandle& e) { handle = e.handle; return *this; }

	void *ToIntPtr () const { return GUINT_TO_POINTER (handle); }
	guint32 ToInt () const { return handle; }
	bool IsAllocated () const { return handle != 0; }
	bool IsWeak () const { return (handle & 7) - 1 == 0; }
	bool IsNormal () const { return (handle & 7) - 1 == 2; }
	void Clear () { handle = 0; }

	static GCHandle Zero;
};

};

#endif
