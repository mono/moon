/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-gallium.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#define __MOON_GALLIUM__

#include "surface-gallium.h"

#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"

namespace Moonlight {

GalliumSurface::Transfer::Transfer (pipe_context  *context,
				    pipe_resource *texture)
{
	resource = NULL;
	pipe     = context;

	transfer = pipe_get_transfer (pipe,
				      texture,
				      0,
				      0,
				      0,
				      PIPE_TRANSFER_READ_WRITE,
				      0, 0,
				      texture->width0,
				      texture->height0);

	pipe_resource_reference (&resource, texture);
}

GalliumSurface::Transfer::~Transfer ()
{
	pipe->transfer_destroy (pipe, transfer);
	pipe_resource_reference (&resource, NULL);
}

void *
GalliumSurface::Transfer::Map ()
{
	return pipe->transfer_map (pipe, transfer);
}

void
GalliumSurface::Transfer::Unmap ()
{
	return pipe->transfer_unmap (pipe, transfer);
}

GalliumSurface::GalliumSurface (pipe_context *context,
				int          width,
				int          height)
{
	struct pipe_screen   *screen = context->screen;
	struct pipe_resource templat;

	pipe   = context;
	mapped = NULL;

	memset (&templat, 0, sizeof (templat));
	templat.target = PIPE_TEXTURE_2D;
	templat.format = PIPE_FORMAT_B8G8R8A8_UNORM;
	templat.width0 = width;
	templat.height0 = height;
	templat.depth0 = 1;
	templat.last_level = 0;
	templat.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET |
		PIPE_BIND_TRANSFER_WRITE | PIPE_BIND_TRANSFER_READ;

	texture = screen->resource_create (screen, &templat);
}


GalliumSurface::~GalliumSurface ()
{
	if (mapped)
		cairo_surface_destroy (mapped);

	pipe_resource_reference (&texture, NULL);
}

void
GalliumSurface::CairoDestroy (void *data)
{
	Transfer *transfer = (Transfer *) data;

	transfer->Unmap ();
	delete transfer;
}

cairo_surface_t *
GalliumSurface::Cairo ()
{
	static cairo_user_data_key_t unused;
	Transfer                     *transfer;
	unsigned char                *data;

	if (!pipe) {
		g_warning ("GalliumSurface::Cairo called with invalid "
			   "surface instance.");
		return NULL;
	}

	if (mapped)
		return cairo_surface_reference (mapped);

	transfer = new Transfer (pipe, texture);
	data     = (unsigned char *) transfer->Map ();

	mapped = cairo_image_surface_create_for_data (data,
						      CAIRO_FORMAT_ARGB32,
						      texture->width0,
						      texture->height0,
						      texture->width0 * 4);

	cairo_surface_set_user_data (mapped,
				     &unused,
				     (void *) transfer,
				     CairoDestroy);

	return cairo_surface_reference (mapped);
}

void
GalliumSurface::Sync ()
{
	if (mapped) {
		cairo_surface_destroy (mapped);
		mapped = NULL;
	}
}

struct pipe_resource *
GalliumSurface::Texture ()
{
	Sync ();

	return texture;
}

};
