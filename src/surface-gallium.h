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

class GalliumSurface : public MoonSurface {
public:
	class Transfer {
	public:
		Transfer (pipe_context  *context,
			  pipe_resource *texture);
		virtual ~Transfer ();

		void *Map ();
		void Unmap ();

	private:
		pipe_context  *pipe;
		pipe_resource *resource;
		pipe_transfer *transfer;
	};

	GalliumSurface (pipe_context *context,
			int          width,
			int          height);
	virtual ~GalliumSurface ();

	cairo_surface_t *Cairo ();

	pipe_sampler_view *SamplerView ();
	pipe_resource *Texture ();

private:
	static void CairoDestroy (void *data);

	void Sync ();

	pipe_context      *pipe;
	pipe_sampler_view *sampler_view;
	cairo_surface_t   *mapped;
};

};

#endif /* __MOON_GALLIUM__ */

#endif /* __MOON_SURFACE_GALLIUM_H__ */

