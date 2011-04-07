/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-cairo.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <string.h>

#include "context-cairo.h"

namespace Moonlight {

CairoContext::CairoContext (CairoSurface *surface) : Context (surface)
{
}

void
CairoContext::Push (Group extents)
{
	Rect            r = extents.r.RoundOut ();
        MoonSurface     *surface = new CairoSurface (r.width, r.height);
        Target          *target = new Target (surface, extents.r);
        cairo_matrix_t  matrix;

	Top ()->GetMatrix (&matrix);

	Stack::Push (new Context::Node (target, &matrix, &extents.r));

	target->unref ();
	surface->unref ();
}

void
CairoContext::Blit (unsigned char *data,
		    int           stride)
{
	cairo_surface_t *dst = cairo_get_target (Top ()->Cairo ());

	for (int i = 0; i < cairo_image_surface_get_height (dst); i++)
		memcpy (cairo_image_surface_get_data (dst) +
			cairo_image_surface_get_stride (dst) * i,
			data + stride * i,
			MIN (cairo_image_surface_get_stride (dst), stride));
}

void
CairoContext::Flush ()
{
	cairo_surface_flush (cairo_get_target (Top ()->Cairo ()));
}

};
