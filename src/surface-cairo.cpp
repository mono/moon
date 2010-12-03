/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-cairo.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "surface-cairo.h"

namespace Moonlight {

CairoSurface::CairoSurface (cairo_surface_t *data,
			    int             width,
			    int             height)
{
	surface = cairo_surface_reference (data);
	size[0] = width;
	size[1] = height;
}

CairoSurface::~CairoSurface ()
{
	cairo_surface_destroy (surface);
}

void
CairoSurface::Reshape (int width, int height)
{
	size[0] = width;
	size[1] = height;
}

cairo_surface_t *
CairoSurface::Cairo ()
{
	return cairo_surface_create_for_rectangle (surface,
						   0,
						   0,
						   size[0],
						   size[1]);
}

};
