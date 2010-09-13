/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-gallium.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#define __MOON_GALLIUM__

#include "context-gallium.h"

#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"
#include "util/u_box.h"
#include "util/u_sampler.h"
#include "util/u_simple_shaders.h"
#include "pipe/p_screen.h"
#include "tgsi/tgsi_ureg.h"

namespace Moonlight {

GalliumContext::GalliumContext (GalliumSurface *surface)
{
	AbsoluteTransform  transform = AbsoluteTransform ();
	Surface            *cs = new Surface (surface, Rect ());
	struct pipe_screen *screen = surface->Screen ();

	pipe = screen->context_create (screen, NULL);
	cso  = cso_create_context (pipe);

	Stack::Push (new Context::Node (cs, &transform.m, NULL));
	cs->unref ();

	/* disable blending/masking */
	{
		struct pipe_blend_state blend;

		memset (&blend, 0, sizeof (blend));
		blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
		blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
		blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_ZERO;
		blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;
		blend.rt[0].colormask = PIPE_MASK_RGBA;

		cso_set_blend (cso, &blend);
	}

	/* no-op depth/stencil/alpha */
	{
		struct pipe_depth_stencil_alpha_state depthstencil;

		memset (&depthstencil, 0, sizeof (depthstencil));

		cso_set_depth_stencil_alpha (cso, &depthstencil);
	}

	/* rasterizer */
	{
		struct pipe_rasterizer_state rasterizer;

		memset (&rasterizer, 0, sizeof (rasterizer));
		rasterizer.cull_face = PIPE_FACE_NONE;
		rasterizer.gl_rasterization_rules = 1;

		cso_set_rasterizer (cso, &rasterizer);
	}

	/* clip */
	{
		struct pipe_clip_state clip;

		memset (&clip, 0, sizeof (clip));

		pipe->set_clip_state (pipe, &clip);
	}

	/* identity viewport */
	{
		struct pipe_viewport_state viewport;

		viewport.scale[0] = 1.0;
		viewport.scale[1] = 1.0;
		viewport.scale[2] = 1.0;
		viewport.scale[3] = 1.0;
		viewport.translate[0] = 0.0;
		viewport.translate[1] = 0.0;
		viewport.translate[2] = 0.0;
		viewport.translate[3] = 0.0;

		cso_set_viewport (cso, &viewport);
	}

	/* samplers */
	{
		struct pipe_sampler_state sampler;
		unsigned                  i;

		memset (&sampler, 0, sizeof (sampler));
		sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
		sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
		sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
		sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
		sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
		sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
		sampler.normalized_coords = 1;

		for (i = 0; i < PIPE_MAX_SAMPLERS; i++)
			cso_single_sampler (cso, i, &sampler);
		cso_single_sampler_done (cso);
	}

	/* default textures */
	{
		struct pipe_resource     templat;
		struct pipe_sampler_view view_templat;
		struct pipe_sampler_view *view;
		struct pipe_sampler_view *fragment_sampler_views[PIPE_MAX_SAMPLERS];
		struct pipe_sampler_view *vertex_sampler_views[PIPE_MAX_VERTEX_SAMPLERS];
		unsigned                 i;

		memset (&templat, 0, sizeof (templat));
		templat.target = PIPE_TEXTURE_2D;
		templat.format = PIPE_FORMAT_A8R8G8B8_UNORM;
		templat.width0 = 1;
		templat.height0 = 1;
		templat.depth0 = 1;
		templat.last_level = 0;
		templat.bind = PIPE_BIND_SAMPLER_VIEW;

		default_texture = screen->resource_create (screen, &templat);
		if (default_texture) {
			struct pipe_box box;
			uint32_t        zero = 0;
	 
			u_box_origin_2d (1, 1, &box);

			pipe->transfer_inline_write (pipe,
						     default_texture,
						     u_subresource (0, 0),
						     PIPE_TRANSFER_WRITE,
						     &box,
						     &zero,
						     sizeof (zero),
						     0);
		}

		u_sampler_view_default_template (&view_templat,
						 default_texture,
						 default_texture->format);
		view = pipe->create_sampler_view (pipe,
						  default_texture,
						  &view_templat);

		for (i = 0; i < PIPE_MAX_SAMPLERS; i++)
			fragment_sampler_views[i] = view;
		for (i = 0; i < PIPE_MAX_VERTEX_SAMPLERS; i++)
			vertex_sampler_views[i] = view;

		cso_set_fragment_sampler_views (cso,
						PIPE_MAX_SAMPLERS,
						fragment_sampler_views);
		cso_set_vertex_sampler_views (cso,
					      PIPE_MAX_VERTEX_SAMPLERS,
					      vertex_sampler_views);

		pipe_sampler_view_reference (&view, NULL);
	}

	/* vertex shader */
	{
		const uint semantic_names[] = { TGSI_SEMANTIC_POSITION,
						TGSI_SEMANTIC_GENERIC };
		const uint semantic_indexes[] = { 0, 0 };

		vs = util_make_vertex_passthrough_shader (pipe,
							  2,
							  semantic_names,
							  semantic_indexes);
		cso_set_vertex_shader_handle (cso, vs);
	}

	/* fragment shader */
	{
		fs = util_make_fragment_tex_shader (pipe,
						    TGSI_TEXTURE_2D,
						    TGSI_INTERPOLATE_LINEAR);
		cso_set_fragment_shader_handle (cso, fs);
	}

	/* vertex elements */
	{
		unsigned i;

		for (i = 0; i < 2; i++) {
			velems[i].src_offset = i * 4 * sizeof (float);
			velems[i].instance_divisor = 0;
			velems[i].vertex_buffer_index = 0;
			velems[i].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
		}
	}
}

GalliumContext::~GalliumContext ()
{
	pipe_resource_reference (&default_texture, NULL);

	cso_delete_vertex_shader (cso, vs);
	cso_delete_fragment_shader (cso, fs);

	cso_destroy_context (cso);
	pipe->destroy (pipe);
}

void
GalliumContext::Push (Group extents)
{
	cairo_matrix_t matrix;
	Rect           r = extents.r.RoundOut ();
        GalliumSurface *surface = new GalliumSurface (pipe, r.width, r.height);
        Surface        *cs = new Surface (surface, extents.r);

	Clear (surface);

	Top ()->GetMatrix (&matrix);

	Stack::Push (new Context::Node (cs, &matrix, &extents.r));
	cs->unref ();
	surface->unref ();
}

void
GalliumContext::Clear (GalliumSurface *dst)
{
	struct pipe_screen   *screen = pipe->screen;
	struct pipe_resource *texture = dst->Texture ();
	struct pipe_surface  *surface;
	const float          clear_color[4] = { .0f, .0f, .0f, .0f };

	surface = screen->get_tex_surface (screen,
					   dst->Texture (),
					   0,
					   0,
					   0,
					   PIPE_BIND_RENDER_TARGET);

	cso_save_framebuffer (cso);

	/* framebuffer */
	{
		struct pipe_framebuffer_state framebuffer;
		memset (&framebuffer, 0, sizeof (framebuffer));
		framebuffer.width = texture->width0;
		framebuffer.height = texture->height0;
		framebuffer.nr_cbufs = 1;
		framebuffer.cbufs[0] = surface;

		cso_set_framebuffer (cso, &framebuffer);
	}

	pipe->clear (pipe, PIPE_CLEAR_COLOR, clear_color, 0, 0);
	pipe->flush (pipe, PIPE_FLUSH_RENDER_CACHE, NULL);

	cso_restore_framebuffer (cso);

	pipe_surface_reference (&surface, NULL);
}

};
