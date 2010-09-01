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

namespace Moonlight {

MoonSurface::MoonSurface ()
{
	refcount = 1;
}

MoonSurface *
MoonSurface::ref ()
{
	g_atomic_int_exchange_and_add (&refcount, 1);

	return this;
}

void
MoonSurface::unref ()
{
	int v;

	v = g_atomic_int_exchange_and_add (&refcount, -1);
	if (v == 0)
		delete this;
}

MoonSurface *
MoonSurface::Similar (int width, int height)
{
	g_warning ("MoonSurface::Similar has been called. The derived class should have overridden it.");

	return NULL;
}

cairo_surface_t *
MoonSurface::Cairo ()
{
	g_warning ("MoonSurface::Cairo has been called. The derived class should have overridden it.");

	return NULL;
}

};
