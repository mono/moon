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

class MOON_API CairoSurface : public MoonSurface {
public:
	CairoSurface (int width,
		      int height);
	virtual ~CairoSurface ();

	cairo_surface_t *Cairo ();
	unsigned char *GetData () {
		return data;
	}

private:
	int           size[2];
	int           stride;
	unsigned char *data;
};

};

#endif /* __MOON_SURFACE_CAIRO_H__ */
