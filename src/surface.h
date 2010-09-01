/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_H__
#define __MOON_SURFACE_H__

#include <cairo.h>
#include <glib.h>

namespace Moonlight {

class MoonSurface {
public:
	MoonSurface ();

	MoonSurface *ref ();
	void        unref ();

	virtual MoonSurface *Similar (int width, int height);
	virtual cairo_surface_t *Cairo ();

private:
	gint32 refcount;
};

};

#endif /* __MOON_SURFACE_H__ */
