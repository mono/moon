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

#include "projection.h"
#include "context-gallium.h"

#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"
#include "util/u_box.h"
#include "util/u_sampler.h"
#include "util/u_simple_shaders.h"
#include "util/u_draw_quad.h"
#include "pipe/p_screen.h"
#include "tgsi/tgsi_ureg.h"

namespace Moonlight {

GalliumContext::GalliumContext (GalliumSurface *surface)
{
	AbsoluteTransform   transform = AbsoluteTransform ();
	Surface             *cs = new Surface (surface, Rect ());
	struct pipe_screen  *screen = surface->Screen ();
	unsigned            i;

	pipe = screen->context_create (screen, NULL);
	cso  = cso_create_context (pipe);

	Stack::Push (new Context::Node (cs, &transform.m, NULL));
	cs->unref ();

	constant_buffer = NULL;

	is_softpipe = strcmp ("softpipe", (*screen->get_name) (screen)) == 0;

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
	for (i = 0; i < 2; i++) {
		velems[i].src_offset = i * 4 * sizeof (float);
		velems[i].instance_divisor = 0;
		velems[i].vertex_buffer_index = 0;
		velems[i].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
	}

	for (i = 0; i < 4; i++) {
		vertices[i][1][2] = 0.0f; /* r */
		vertices[i][1][3] = 1.0f; /* q */
	}

	/* blend over */
	memset (&blend_over, 0, sizeof (struct pipe_blend_state));
	blend_over.rt[0].colormask |= PIPE_MASK_RGBA;
	blend_over.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend_over.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend_over.rt[0].blend_enable = 1;
	blend_over.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	blend_over.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;

	/* perspective transform sampler */
	memset (&project_sampler, 0, sizeof (struct pipe_sampler_state));
	project_sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	project_sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	project_sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	project_sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	project_sampler.min_img_filter = PIPE_TEX_FILTER_LINEAR;
	project_sampler.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
	project_sampler.normalized_coords = 1;

	/* perspective transform fragment shaders */
	for (i = 0; i < 2; i++)
		project_fs[i] = NULL;
}

GalliumContext::~GalliumContext ()
{
	unsigned i;

	for (i = 0; i < 2; i++)
		if (project_fs[i])
			cso_delete_fragment_shader (cso, project_fs[i]);

	pipe_resource_reference (&default_texture, NULL);

	cso_delete_vertex_shader (cso, vs);
	cso_delete_fragment_shader (cso, fs);

	cso_destroy_context (cso);
	pipe->destroy (pipe);
}

void
GalliumContext::SetFramebuffer ()
{
	struct pipe_framebuffer_state framebuffer;
	Surface                       *cs = Top ()->GetSurface ();
	MoonSurface                   *ms;
	Rect                          r = cs->GetData (&ms);
	GalliumSurface                *dst = (GalliumSurface *) ms;
	struct pipe_resource          *texture = dst->Texture ();
	struct pipe_surface           *surface;

	surface = (*pipe->screen->get_tex_surface) (pipe->screen,
						    texture,
						    0, 0, 0,
						    PIPE_BIND_RENDER_TARGET);

	memset (&framebuffer, 0, sizeof (framebuffer));
	framebuffer.width = texture->width0;
	framebuffer.height = texture->height0;
	framebuffer.nr_cbufs = 1;
	framebuffer.cbufs[0] = surface;

	cso_set_framebuffer (cso, &framebuffer);

	pipe_surface_reference (&surface, NULL);
	ms->unref ();
}

void
GalliumContext::SetScissor ()
{
	struct pipe_scissor_state scissor;
	Surface                   *cs = Top ()->GetSurface ();
	Rect                      r = cs->GetData (NULL);
	Rect                      clip;

	Top ()->GetClip (&clip);

	scissor.minx = clip.x - r.x;
	scissor.miny = clip.y - r.y;
	scissor.maxx = clip.x + clip.width;
	scissor.maxy = clip.y + clip.height;

	(*pipe->set_scissor_state) (pipe, &scissor);
}

void
GalliumContext::SetRasterizer ()
{
	struct pipe_rasterizer_state rasterizer;

	memset (&rasterizer, 0, sizeof (rasterizer));
	rasterizer.cull_face = PIPE_FACE_NONE;
	rasterizer.gl_rasterization_rules = 1;
	rasterizer.scissor = 1;

	cso_set_rasterizer (cso, &rasterizer);
}

void
GalliumContext::SetViewport ()
{
	struct pipe_viewport_state viewport;
	Surface                    *cs = Top ()->GetSurface ();
	Rect                       r = cs->GetData (NULL);

	memset (&viewport, 0, sizeof (viewport));
	viewport.scale[0] = VIEWPORT_SCALE;
	viewport.scale[1] = VIEWPORT_SCALE;
	viewport.scale[2] = 1.0f;
	viewport.scale[3] = 1.0f;
	viewport.translate[0] = -r.x;
	viewport.translate[1] = -r.y;
	viewport.translate[2] = 0.0f;
	viewport.translate[3] = 0.0f;

	cso_set_viewport (cso, &viewport);
}

void
GalliumContext::SetConstantBuffer (const void *data, int bytes)
{
	struct pipe_resource **cbuf = &constant_buffer;

	// always get a new buffer
	pipe_resource_reference (cbuf, NULL);

	*cbuf = pipe_buffer_create (pipe->screen, 
				    PIPE_BIND_CONSTANT_BUFFER,
				    bytes);

	if (*cbuf)
		pipe_buffer_write (pipe, *cbuf, 0, bytes, data);

	(*pipe->set_constant_buffer) (pipe,
				      PIPE_SHADER_FRAGMENT,
				      0, *cbuf);
}

struct pipe_resource *
GalliumContext::SetupVertexData (struct pipe_resource      *texture,
				 struct pipe_sampler_state *sampler,
				 const double              *matrix,
				 double                    x,
				 double                    y)
{
	double p[4][4];
	float  dt = sampler->normalized_coords ? 0.0f : 1.0f;
	int    i;

	p[0][0] = x;
	p[0][1] = y;
	p[0][2] = 0.0;
	p[0][3] = 1.0;

	p[1][0] = x + texture->width0;
	p[1][1] = y;
	p[1][2] = 0.0;
	p[1][3] = 1.0;

	p[2][0] = x + texture->width0;
	p[2][1] = y + texture->height0;
	p[2][2] = 0.0;
	p[2][3] = 1.0;

	p[3][0] = x;
	p[3][1] = y + texture->height0;
	p[3][2] = 0.0;
	p[3][3] = 1.0;

	if (matrix) {
		for (i = 0; i < 4; i++)
			Matrix3D::TransformPoint (p[i], matrix, p[i]);
	}

	for (i = 0; i < 4; i++) {
		vertices[i][0][0] = p[i][0] * VIEWPORT_SCALE_RECIPROCAL;
		vertices[i][0][1] = p[i][1] * VIEWPORT_SCALE_RECIPROCAL;
		vertices[i][0][2] = p[i][2];
		vertices[i][0][3] = p[i][3];
	}

	vertices[0][1][0] = 0.0f;
	vertices[0][1][1] = 0.0f;

	vertices[1][1][0] = texture->width0 * dt + (1.0f - dt);
	vertices[1][1][1] = 0.0f;

	vertices[2][1][0] = texture->width0 * dt + (1.0f - dt);
	vertices[2][1][1] = texture->height0 * dt + (1.0f - dt);

	vertices[3][1][0] = 0.0f;
	vertices[3][1][1] = texture->height0 * dt + (1.0f - dt);

	return pipe_user_buffer_create (pipe->screen,
					vertices,
					sizeof (vertices),
					PIPE_BIND_VERTEX_BUFFER);
}

void
GalliumContext::TransformMatrix (double *out, const double *matrix)
{
	cairo_matrix_t ctm;
	double         m[16];

	Top ()->GetMatrix (&ctm);

	Matrix3D::Affine (m,
			  ctm.xx, ctm.xy,
			  ctm.yx, ctm.yy,
			  ctm.x0, ctm.y0);
	Matrix3D::Multiply (out, matrix, m);
}

void
GalliumContext::Push (Group extents)
{
	cairo_matrix_t matrix;
	Rect           r = extents.r.RoundOut ();
        GalliumSurface *surface = new GalliumSurface (pipe, r.width, r.height);
        Surface        *cs = new Surface (surface, extents.r);
	const float    clear_color[4] = { .0f, .0f, .0f, .0f };

	Top ()->GetMatrix (&matrix);

	Stack::Push (new Context::Node (cs, &matrix, &extents.r));

	cso_save_framebuffer (cso);

	SetFramebuffer ();

	pipe->clear (pipe, PIPE_CLEAR_COLOR, clear_color, 0, 0);
	pipe->flush (pipe, PIPE_FLUSH_RENDER_CACHE, NULL);

	cso_restore_framebuffer (cso);

	cs->unref ();
	surface->unref ();
}

void *
GalliumContext::GetProjectShader (double opacity)
{
	unsigned            index = opacity < 1.0 ? 1 : 0;
	struct ureg_program *ureg;
	struct ureg_src     tex, sampler, alpha;
	struct ureg_dst     out, tmp;

	if (project_fs[index])
		return project_fs[index];

	ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);

	sampler = ureg_DECL_sampler (ureg, 0);

	tex = ureg_DECL_fs_input (ureg,
				  TGSI_SEMANTIC_GENERIC, 0,
				  TGSI_INTERPOLATE_PERSPECTIVE);

	out = ureg_DECL_output (ureg,
				TGSI_SEMANTIC_COLOR,
				0);

	if (opacity < 1.0) {
		tmp   = ureg_DECL_temporary (ureg);
		alpha = ureg_DECL_constant (ureg, 0);

		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, tex, sampler);
		ureg_MUL (ureg, out, ureg_src (tmp), alpha);
	}
	else {
		ureg_TEX (ureg, out, TGSI_TEXTURE_2D, tex, sampler);
	}

	ureg_END (ureg);

	project_fs[index] = ureg_create_shader_and_destroy (ureg, pipe);

	return project_fs[index];
}

void
GalliumContext::Project (MoonSurface  *src,
			 const double *matrix,
			 double       alpha,
			 double       x,
			 double       y)
{
	GalliumSurface           *surface = (GalliumSurface *) src;
	struct pipe_sampler_view *view = surface->SamplerView ();
	struct pipe_resource     *vbuf;
	float                    cbuf[4] = { alpha, alpha, alpha, alpha };
	double                   m[16];

	TransformMatrix (m, matrix);

	// software optimization that can hopefully be removed one day
	if (is_softpipe) {
		int x0, y0;

		if (Matrix3D::IsIntegerTranslation (m, &x0, &y0)) {
			cairo_surface_t *cs = src->Cairo ();
			cairo_t         *cr = Cairo ();
			Rect            r = Rect (x,
						  y,
						  view->texture->width0,
						  view->texture->height0);

			cairo_save (cr);
			cairo_identity_matrix (cr);
			r.Transform (m).RoundOut ().Draw (cr);
			cairo_clip (cr);
			cairo_translate (cr, x0, y0);
			cairo_set_source_surface (cr, cs, r.x, r.y);
			cairo_paint_with_alpha (cr, alpha);
			cairo_restore (cr);
			cairo_surface_destroy (cs);

			return;
		}
	}

	cso_save_blend (cso);
	cso_save_samplers (cso);
	cso_save_fragment_sampler_views (cso);
	cso_save_fragment_shader (cso);
	cso_save_framebuffer (cso);
	cso_save_viewport (cso);

	cso_set_blend (cso, &blend_over);
	cso_single_sampler (cso, 0, &project_sampler);
	cso_single_sampler_done (cso);
	cso_set_fragment_sampler_views (cso, 1, &view);
	cso_set_fragment_shader_handle (cso, GetProjectShader (alpha));

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();
	SetRasterizer ();

	SetConstantBuffer (cbuf, sizeof (cbuf));

	vbuf = SetupVertexData (view->texture, &project_sampler, m, x, y);
	if (vbuf) {
		cso_set_vertex_elements (cso, 2, velems);
		util_draw_vertex_buffer (pipe, vbuf, 0,
					 PIPE_PRIM_TRIANGLE_FAN,
					 4,
					 2);

		pipe_resource_reference (&vbuf, NULL);
	}

	cso_restore_blend (cso);
	cso_restore_samplers (cso);
	cso_restore_fragment_sampler_views (cso);
	cso_restore_fragment_shader (cso);
	cso_restore_framebuffer (cso);
	cso_restore_viewport (cso);
}

};
