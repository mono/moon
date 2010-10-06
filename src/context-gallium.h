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

#define MAX_CONVOLVE_SIZE 32

#ifdef __MOON_GALLIUM__

#include "pipe/p_context.h"
#include "cso_cache/cso_context.h"
#include "cso_cache/cso_hash.h"

namespace Moonlight {

class GalliumContext : public Context {
public:
	class Surface : public Context::Surface {
	public:
		Surface (MoonSurface  *moon,
			 Rect         extents,
			 GalliumPipe  *pipe);
		virtual ~Surface ();

		cairo_surface_t *Cairo ();

	private:
		GalliumPipe *gpipe;
	};

	GalliumContext (GalliumSurface *surface);
	virtual ~GalliumContext ();

	void Push (Group extents);
	void Push (Group extents, MoonSurface *surface);

	void Clear (Color *color);

	void Project (MoonSurface  *src,
		      const double *matrix,
		      double       alpha,
		      double       x,
		      double       y);

	void Blur (MoonSurface *src,
		   double      radius,
		   double      x,
		   double      y);

	void DropShadow (MoonSurface *src,
			 double      dx,
			 double      dy,
			 double      radius,
			 Color       *color,
			 double      x,
			 double      y);

	void ShaderEffect (MoonSurface *src,
			   PixelShader *shader,
			   Brush       **sampler,
			   int         *sampler_mode,
			   int         n_sampler,
			   Color       *constant,
			   int         n_constant,
			   int         *ddxUvDdyUvPtr,
			   double      x,
			   double      y);

	void Flush ();

private:
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

	void InitMatrix (double *out);
	void TransformMatrix (double *out, const double *matrix);
	void *GetProjectShader (double alpha);
	void *GetConvolveShader (unsigned size);
	void *GetDropShadowShader (unsigned size);
	void *GetEffectShader (PixelShader *ps);

	pipe_context *pipe;
	GalliumPipe  *gpipe;

	cso_context *cso;

	void *default_vs;
	void *default_fs;

	pipe_vertex_element velems[2];

	float vertices[4][2][4];

	pipe_resource *constant_buffer;
	int is_softpipe;

	pipe_sampler_state default_sampler;

	pipe_blend_state blend_over;
	pipe_blend_state blend_src;

	pipe_sampler_state project_sampler;
	void *project_fs[2];

	void *convolve_fs[MAX_CONVOLVE_SIZE + 1];
	pipe_sampler_state convolve_sampler;

	void *dropshadow_fs[MAX_CONVOLVE_SIZE + 1];

	pipe_sampler_state effect_sampler;
	cso_hash *effect_fs;
};

};

#endif /* __MOON_GALLIUM__ */

#endif /* __MOON_CONTEXT_GALLIUM_H__ */
