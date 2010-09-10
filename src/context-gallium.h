/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-gallium.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_GALLIUM_H__
#define __MOON_CONTEXT_GALLIUM_H__

#include "context.h"
#include "surface-gallium.h"

#ifdef __MOON_GALLIUM__

#include "pipe/p_context.h"
#include "cso_cache/cso_context.h"

namespace Moonlight {

class GalliumContext : public Context {
public:
	GalliumContext (pipe_screen *screen);
	virtual ~GalliumContext ();

	void Push (Group extents);

// private:
	void Clear (GalliumSurface *dst);

	pipe_context *pipe;

	cso_context *cso;

	void *vs;
	void *fs;

	pipe_resource *default_texture;
	pipe_sampler_view *fragment_sampler_views[PIPE_MAX_SAMPLERS];
	pipe_sampler_view *vertex_sampler_views[PIPE_MAX_VERTEX_SAMPLERS];

	pipe_vertex_element velems[2];
};

};

#endif /* __MOON_GALLIUM__ */

#endif /* __MOON_CONTEXT_GALLIUM_H__ */
