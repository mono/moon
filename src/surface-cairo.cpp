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

CairoSurface::CairoSurface (int width,
			    int height)
{
	size[0] = width;
	size[1] = height;
	stride  = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
	data    = (unsigned char *) g_malloc0 (height * stride);
}

CairoSurface::~CairoSurface ()
{
	g_free (data);
}

cairo_surface_t *
CairoSurface::Cairo ()
{
	return cairo_image_surface_create_for_data (data,
						    CAIRO_FORMAT_ARGB32,
						    size[0],
						    size[1],
						    stride);
}

};
