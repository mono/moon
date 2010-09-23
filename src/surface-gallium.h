/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-gallium.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_GALLIUM_H__
#define __MOON_SURFACE_GALLIUM_H__

#include "surface.h"

#ifdef __MOON_GALLIUM__

#include "pipe/p_state.h"

namespace Moonlight {

class GalliumPipe {
public:
	GalliumPipe (pipe_context *context);
	~GalliumPipe ();

	GalliumPipe *ref ();
	void        unref ();

	pipe_context *Pipe ();

private:
	pipe_context *pipe;
	gint32       refcount;
};

class GalliumSurface : public MoonSurface {
public:
	class Transfer {
	public:
		Transfer (GalliumPipe   *context,
			  pipe_resource *texture);
		virtual ~Transfer ();

		void *Map ();
		void Unmap ();

	private:
		GalliumPipe   *gpipe;
		pipe_resource *resource;
		pipe_transfer *transfer;
	};

	GalliumSurface (pipe_resource *texture);
	GalliumSurface (GalliumPipe *pipe,
			int         width,
			int         height);
	virtual ~GalliumSurface ();

	cairo_surface_t *Cairo (GalliumPipe *pipe);
	cairo_surface_t *Cairo ();

	pipe_resource *Texture ();
	pipe_sampler_view *SamplerView ();

private:
	static void CairoDestroy (void *data);

	void Sync ();

	GalliumPipe       *gpipe;
	pipe_resource     *resource;
	pipe_sampler_view *sampler_view;
	cairo_surface_t   *mapped;
};

};

#endif /* __MOON_GALLIUM__ */

#endif /* __MOON_SURFACE_GALLIUM_H__ */

