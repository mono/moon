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
#include "util/u_sampler.h"

#include <stdio.h>

namespace Moonlight {

GalliumPipe::GalliumPipe (pipe_context *context)
{
	pipe     = context;
	refcount = 1;
}
	
GalliumPipe::~GalliumPipe ()
{
	pipe->destroy (pipe);
}

GalliumPipe *
GalliumPipe::ref ()
{
	g_atomic_int_inc (&refcount);

	return this;
}

void
GalliumPipe::unref ()
{
	int v = g_atomic_int_exchange_and_add (&refcount, -1) - 1;

	if (v == 0)
		delete this;
}

pipe_context *
GalliumPipe::Pipe ()
{
	return pipe;
}
	
GalliumSurface::Transfer::Transfer (GalliumPipe   *pipe,
				    pipe_resource *texture)
{
	resource = NULL;
	gpipe    = pipe->ref ();

	transfer = pipe_get_transfer (gpipe->Pipe (),
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
	struct pipe_context *pipe = gpipe->Pipe ();

	pipe->transfer_destroy (pipe, transfer);
	pipe_resource_reference (&resource, NULL);

	gpipe->unref ();
}

void *
GalliumSurface::Transfer::Map ()
{
	struct pipe_context *pipe = gpipe->Pipe ();

	return pipe->transfer_map (pipe, transfer);
}

void
GalliumSurface::Transfer::Unmap ()
{
	struct pipe_context *pipe = gpipe->Pipe ();

	return pipe->transfer_unmap (pipe, transfer);
}

GalliumSurface::GalliumSurface (pipe_resource *texture)
{
	gpipe        = NULL;
	mapped       = NULL;
	resource     = NULL;
	sampler_view = NULL;

	pipe_resource_reference (&resource, texture);
}

GalliumSurface::GalliumSurface (GalliumPipe *pipe,
				int         width,
				int         height)
{
	struct pipe_resource pt;
	struct pipe_screen   *screen = pipe->Pipe ()->screen;

	gpipe        = pipe->ref ();
	mapped       = NULL;
	sampler_view = NULL;

	memset (&pt, 0, sizeof (pt));
	pt.target = PIPE_TEXTURE_2D;
	pt.format = PIPE_FORMAT_B8G8R8A8_UNORM;
	pt.width0 = width;
	pt.height0 = height;
	pt.depth0 = 1;
	pt.last_level = 0;
	pt.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET |
		PIPE_BIND_TRANSFER_WRITE | PIPE_BIND_TRANSFER_READ;

	g_assert (screen->is_format_supported (screen,
					       pt.format,
					       pt.target,
					       0,
					       pt.bind,
					       0));

	resource = screen->resource_create (screen, &pt);

	g_assert (resource);
}

GalliumSurface::~GalliumSurface ()
{
	if (mapped)
		cairo_surface_destroy (mapped);

	pipe_sampler_view_reference (&sampler_view, NULL);
	pipe_resource_reference (&resource, NULL);

	if (gpipe)
		gpipe->unref ();
}

void
GalliumSurface::CairoDestroy (void *data)
{
	Transfer *transfer = (Transfer *) data;

	transfer->Unmap ();
	delete transfer;
}

cairo_surface_t *
GalliumSurface::Cairo (GalliumPipe *pipe)
{
	static cairo_user_data_key_t key;
	Transfer                     *transfer;
	unsigned char                *data;

	if (mapped)
		return cairo_surface_reference (mapped);

	transfer = new Transfer (pipe, resource);
	data     = (unsigned char *) transfer->Map ();

	mapped = cairo_image_surface_create_for_data (data,
						      CAIRO_FORMAT_ARGB32,
						      resource->width0,
						      resource->height0,
						      resource->width0 * 4);

	cairo_surface_set_user_data (mapped,
				     &key,
				     (void *) transfer,
				     CairoDestroy);

	return cairo_surface_reference (mapped);
}

cairo_surface_t *
GalliumSurface::Cairo ()
{
	if (!gpipe) {
		g_warning ("GalliumSurface::Cairo called with invalid "
			   "surface instance.");

		return cairo_image_surface_create (CAIRO_FORMAT_INVALID, 0, 0);
	}

	return Cairo (gpipe);
}

void
GalliumSurface::Sync ()
{
	if (mapped) {
		cairo_surface_destroy (mapped);
		mapped = NULL;
	}
}

struct pipe_sampler_view *
GalliumSurface::SamplerView ()
{
	struct pipe_sampler_view templ;
	struct pipe_context      *pipe = gpipe->Pipe ();

	Sync ();

	if (sampler_view)
		return sampler_view;

	u_sampler_view_default_template (&templ, resource, resource->format);
	sampler_view = pipe->create_sampler_view (pipe, resource, &templ);

	return sampler_view;
}

struct pipe_resource *
GalliumSurface::Texture ()
{
	Sync ();

	return resource;
}

};
