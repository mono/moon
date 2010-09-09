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

CairoSurface::CairoSurface (cairo_surface_t *data)
{
	surface = cairo_surface_reference (data);
}

CairoSurface::~CairoSurface ()
{
	cairo_surface_destroy (surface);
}

cairo_surface_t *
CairoSurface::Cairo ()
{
	return cairo_surface_reference (surface);
}

};
