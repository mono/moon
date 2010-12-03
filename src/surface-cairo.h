/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-cairo.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_CAIRO_H__
#define __MOON_SURFACE_CAIRO_H__

#include "surface.h"

namespace Moonlight {

class CairoSurface : public MoonSurface {
public:
	CairoSurface (cairo_surface_t *data,
		      int             width,
		      int             height);
	virtual ~CairoSurface ();

	void Reshape (int width, int height);
	cairo_surface_t *Cairo ();

private:
	int             size[2];
	cairo_surface_t *surface;
};

};

#endif /* __MOON_SURFACE_CAIRO_H__ */
