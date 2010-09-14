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
	GalliumContext (GalliumSurface *surface);
	virtual ~GalliumContext ();

	void Push (Group extents);

	void Project (MoonSurface  *src,
		      const double *matrix,
		      double       alpha,
		      double       x,
		      double       y);

// private:
	void SetFramebuffer ();
	void SetScissor ();
	void SetRasterizer ();
	void SetViewport ();
	void SetConstantBuffer (const void *data, int bytes);

	pipe_resource *SetupVertexData (pipe_resource      *texture,
					pipe_sampler_state *sampler,
					const double       *matrix,
					double             x,
					double             y);

	void TransformMatrix (double *out, const double *matrix);
	
	pipe_context *pipe;

	cso_context *cso;

	void *vs;
	void *fs;

	pipe_resource *default_texture;

	pipe_vertex_element velems[2];

	float vertices[4][2][4];

	pipe_resource *constant_buffer;
	int is_softpipe;

	pipe_blend_state blend_over;

	pipe_sampler_state project_sampler;
	void *project_fs;
	void *project_alpha_fs;
};

};

#endif /* __MOON_GALLIUM__ */

#endif /* __MOON_CONTEXT_GALLIUM_H__ */
