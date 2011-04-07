/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "surface.h"

#include <mono/io-layer/atomic.h>

namespace Moonlight {

MoonSurface::MoonSurface ()
{
	refcount = 1;
}

MoonSurface::~MoonSurface ()
{
}

MoonSurface *
MoonSurface::ref ()
{
	InterlockedExchangeAdd (&refcount, 1);

	return this;
}

void
MoonSurface::unref ()
{
	int v;

	v = InterlockedExchangeAdd (&refcount, -1) - 1;
	if (v == 0)
		delete this;
}

cairo_surface_t *
MoonSurface::Cairo ()
{
	g_warning ("MoonSurface::Cairo has been called. The derived class should have overridden it.");

	return NULL;
}

};
