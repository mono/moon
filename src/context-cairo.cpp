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
CairoContext::Clear (Color *color)
{
	cairo_t *cr = Context::Push (Cairo ());

	cairo_save (cr);
	cairo_set_operator (cr , CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
	cairo_paint (cr);
	cairo_restore (cr);

	Pop ();
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
CairoContext::Blend (Color *color)
{
	cairo_t *cr = Context::Push (Cairo ());

	cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
	cairo_paint (cr);

	Pop ();
}

void
CairoContext::Blend (MoonSurface *src,
		     double      alpha,
		     double      x,
		     double      y)
{
	cairo_surface_t *surface = src->Cairo ();
	cairo_t         *cr = Context::Push (Cairo ());

	cairo_set_source_surface (cr, surface, x, y);
	cairo_paint_with_alpha (cr, alpha);
	cairo_surface_destroy (surface);

	Pop ();
}

void
CairoContext::Flush ()
{
	cairo_surface_flush (cairo_get_target (Top ()->Cairo ()));
}

};
