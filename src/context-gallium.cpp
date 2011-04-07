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
#include "effect.h"
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
#include "tgsi/tgsi_dump.h"

namespace Moonlight {

GalliumContext::Target::Target (MoonSurface  *moon,
				Rect         extents,
				GalliumPipe  *pipe) :
	Context::Target::Target (moon, extents)
{
	gpipe = pipe->ref ();
}

GalliumContext::Target::~Target ()
{
	gpipe->unref ();
}

cairo_surface_t *
GalliumContext::Target::Cairo ()
{
	cairo_surface_t *surface;

	surface = ((GalliumSurface *) native)->Cairo (gpipe);

	/* set device offset */
	if (!box.IsEmpty ())
		cairo_surface_set_device_offset (surface, -box.x, -box.y);

	return surface;
}
	
GalliumContext::GalliumContext (GalliumSurface *surface)
{
	AbsoluteTransform    transform = AbsoluteTransform ();
	struct pipe_resource *tex = surface->Texture ();
	struct pipe_screen   *screen = tex->screen;
	Rect                 r = Rect (0, 0, 32768, 32768);
	Target               *target;
	unsigned             i;

	pipe  = screen->context_create (screen, NULL);
	gpipe = new GalliumPipe (pipe);
	cso   = cso_create_context (pipe);

	target = new Target (surface, r, gpipe);
	Stack::Push (new Context::Node (target, &transform.m, NULL));
	target->unref ();

	constant_buffer = NULL;

	is_softpipe = strcmp ("softpipe", (*screen->get_name) (screen)) == 0;

	/* no-op depth/stencil/alpha */
	{
		struct pipe_depth_stencil_alpha_state depthstencil;
		memset (&depthstencil, 0, sizeof (depthstencil));
		cso_set_depth_stencil_alpha (cso, &depthstencil);
	}

	/* default sampler */
	memset (&default_sampler, 0, sizeof (struct pipe_sampler_state));
	default_sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	default_sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	default_sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	default_sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	default_sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
	default_sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
	default_sampler.normalized_coords = 1;

	for (i = 0; i < PIPE_MAX_SAMPLERS; i++)
		cso_single_sampler (cso, i, &default_sampler);
	cso_single_sampler_done (cso);

	/* vertex shaders */
	{
		const uint semantic_names[] = { TGSI_SEMANTIC_POSITION,
						TGSI_SEMANTIC_COLOR };
		const uint semantic_indices[] = { 0, 0 };

		color_vs =
			util_make_vertex_passthrough_shader (pipe,
							     2,
							     semantic_names,
							     semantic_indices);
	}
	{
		const uint semantic_names[] = { TGSI_SEMANTIC_POSITION,
						TGSI_SEMANTIC_GENERIC };
		const uint semantic_indices[] = { 0, 0 };

		tex_vs = util_make_vertex_passthrough_shader (pipe,
							      2,
							      semantic_names,
							      semantic_indices);
	}
	cso_set_vertex_shader_handle (cso, tex_vs);

	/* default fragment shader */
	default_fs = util_make_fragment_tex_shader (pipe,
						    TGSI_TEXTURE_2D,
						    TGSI_INTERPOLATE_LINEAR);
	cso_set_fragment_shader_handle (cso, default_fs);

	/* vertex elements */
	for (i = 0; i < 2; i++) {
		velems[i].src_offset = i * 4 * sizeof (float);
		velems[i].instance_divisor = 0;
		velems[i].vertex_buffer_index = 0;
		velems[i].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
	}

	/* blend src */
	memset (&blend_src, 0, sizeof (struct pipe_blend_state));
	blend_src.rt[0].colormask |= PIPE_MASK_RGBA;
	blend_src.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend_src.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend_src.rt[0].blend_enable = 0;
	blend_src.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_ZERO;
	blend_src.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;

	cso_set_blend (cso, &blend_src);

	/* blend over */
	memset (&blend_over, 0, sizeof (struct pipe_blend_state));
	blend_over.rt[0].colormask |= PIPE_MASK_RGBA;
	blend_over.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend_over.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend_over.rt[0].blend_enable = 1;
	blend_over.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	blend_over.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;

	/* blend fragment shader */
	blend_fs = NULL;

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

	/* convolve fragment shaders */
	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		convolve_fs[i] = NULL;

	/* convolution sampler */
	memset (&convolve_sampler, 0, sizeof (struct pipe_sampler_state));
	convolve_sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	convolve_sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	convolve_sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	convolve_sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	convolve_sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
	convolve_sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
	convolve_sampler.normalized_coords = 1;

	/* drop shadow fragment shaders */
	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		dropshadow_fs[i] = NULL;

	/* effect sampler */
	memset (&effect_sampler, 0, sizeof (struct pipe_sampler_state));
	effect_sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	effect_sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	effect_sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	effect_sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	effect_sampler.min_img_filter = PIPE_TEX_FILTER_LINEAR;
	effect_sampler.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
	effect_sampler.normalized_coords = 1;

	/* effect fragment shaders */
	effect_fs = cso_hash_create ();
}

GalliumContext::~GalliumContext ()
{
	unsigned i;

	while (cso_hash_size (effect_fs)) {
		struct cso_hash_iter iter;

		iter = cso_hash_first_node (effect_fs);
		if (!cso_hash_iter_is_null (iter)) {
			void *fs = cso_hash_iter_data (iter);

			cso_delete_fragment_shader (cso, fs);
		}

		cso_hash_erase (effect_fs, iter);
	}
	cso_hash_delete (effect_fs);

	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		if (convolve_fs[i])
			cso_delete_fragment_shader (cso, convolve_fs[i]);

	for (i = 0; i < 2; i++)
		if (project_fs[i])
			cso_delete_fragment_shader (cso, project_fs[i]);

	if (blend_fs)
		cso_delete_fragment_shader (cso, blend_fs);

	cso_delete_fragment_shader (cso, default_fs);
	cso_delete_vertex_shader (cso, tex_vs);
	cso_delete_vertex_shader (cso, color_vs);

	cso_release_all (cso);
	cso_destroy_context (cso);

	gpipe->unref ();
}

void
GalliumContext::SetFramebuffer ()
{
	struct pipe_framebuffer_state framebuffer;
	Context::Target               *target = Top ()->GetTarget ();
	MoonSurface                   *ms;
	Rect                          r = target->GetData (&ms);
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
	Context::Target           *target = Top ()->GetTarget ();
	Rect                      r = target->GetData (NULL);
	Rect                      clip;

	Top ()->GetClip (&clip);

	clip.x -= r.x;
	clip.y -= r.y;

	scissor.minx = MAX (clip.x, 0);
	scissor.miny = MAX (clip.y, 0);
	scissor.maxx = MIN (clip.x + clip.width, 32768);
	scissor.maxy = MIN (clip.y + clip.height, 32768);

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
	Context::Target            *target = Top ()->GetTarget ();
	MoonSurface                *ms;
	Rect                       r = target->GetData (&ms);
	GalliumSurface             *dst = (GalliumSurface *) ms;
	struct pipe_resource       *texture = dst->Texture ();

	memset (&viewport, 0, sizeof (viewport));
	viewport.scale[0] = texture->width0 * 0.5f;
	viewport.scale[1] = texture->height0 * 0.5f;
	viewport.scale[2] = 0.5f;
	viewport.scale[3] = 1.0f;
	viewport.translate[0] = texture->width0 * 0.5f;
	viewport.translate[1] = texture->height0 * 0.5f;
	viewport.translate[2] = 0.5f;
	viewport.translate[3] = 0.0f;

	cso_set_viewport (cso, &viewport);

	ms->unref ();
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
GalliumContext::SetupVertexData (Color *color)
{
	float rgba[4];

	vertices[0][0][0] = -1.0f;
	vertices[0][0][1] = -1.0f;
	vertices[0][0][2] = 0.0f;
	vertices[0][0][3] = 1.0f;

	vertices[1][0][0] = 1.0f;
	vertices[1][0][1] = -1.0f;
	vertices[1][0][2] = 0.0f;
	vertices[1][0][3] = 1.0f;

	vertices[2][0][0] = 1.0f;
	vertices[2][0][1] = 1.0f;
	vertices[2][0][2] = 0.0f;
	vertices[2][0][3] = 1.0f;

	vertices[3][0][0] = -1.0f;
	vertices[3][0][1] = 1.0f;
	vertices[3][0][2] = 0.0f;
	vertices[3][0][3] = 1.0f;

	rgba[0] = color->r * color->a;
	rgba[1] = color->g * color->a;
	rgba[2] = color->b * color->a;
	rgba[3] = color->a;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			vertices[i][1][j] = rgba[j];

	return pipe_user_buffer_create (pipe->screen,
					vertices,
					sizeof (vertices),
					PIPE_BIND_VERTEX_BUFFER);
}

struct pipe_resource *
GalliumContext::SetupVertexData (struct pipe_sampler_state *sampler,
				 const double              *matrix,
				 double                    x,
				 double                    y,
				 double                    width,
				 double                    height)
{
	Context::Target      *target = Top ()->GetTarget ();
	MoonSurface          *ms;
	Rect                 r = target->GetData (&ms);
	GalliumSurface       *dst = (GalliumSurface *) ms;
	struct pipe_resource *texture = dst->Texture ();
	double               dx = 2.0 / texture->width0;
	double               dy = 2.0 / texture->height0;
	double               p[4][4];
	float                dt = sampler->normalized_coords ? 0.0f : 1.0f;
	int                  i;

	p[0][0] = x;
	p[0][1] = y;
	p[0][2] = 0.0;
	p[0][3] = 1.0;

	p[1][0] = x + width;
	p[1][1] = y;
	p[1][2] = 0.0;
	p[1][3] = 1.0;

	p[2][0] = x + width;
	p[2][1] = y + height;
	p[2][2] = 0.0;
	p[2][3] = 1.0;

	p[3][0] = x;
	p[3][1] = y + height;
	p[3][2] = 0.0;
	p[3][3] = 1.0;

	if (matrix) {
		for (i = 0; i < 4; i++)
			Matrix3D::TransformPoint (p[i], matrix, p[i]);
	}

	for (i = 0; i < 4; i++) {
		vertices[i][0][0] = p[i][0] * dx - p[i][3];
		vertices[i][0][1] = p[i][1] * dy - p[i][3];
		vertices[i][0][2] = -p[i][2];
		vertices[i][0][3] = p[i][3];
	}

	vertices[0][1][0] = 0.0f;
	vertices[0][1][1] = 0.0f;
	vertices[0][1][2] = 0.0f;
	vertices[0][1][3] = 1.0f;

	vertices[1][1][0] = width * dt + (1.0f - dt);
	vertices[1][1][1] = 0.0f;
	vertices[1][1][2] = 0.0f;
	vertices[1][1][3] = 1.0f;

	vertices[2][1][0] = width * dt + (1.0f - dt);
	vertices[2][1][1] = height * dt + (1.0f - dt);
	vertices[2][1][2] = 0.0f;
	vertices[2][1][3] = 1.0f;

	vertices[3][1][0] = 0.0f;
	vertices[3][1][1] = height * dt + (1.0f - dt);
	vertices[3][1][2] = 0.0f;
	vertices[3][1][3] = 1.0f;

	ms->unref ();

	return pipe_user_buffer_create (pipe->screen,
					vertices,
					sizeof (vertices),
					PIPE_BIND_VERTEX_BUFFER);
}

void
GalliumContext::Push (Group extents)
{
	cairo_matrix_t matrix;
	Rect           r = extents.r;
        GalliumSurface *surface = new GalliumSurface (gpipe,
						      r.width,
						      r.height);
        Target         *target = new Target (surface, extents.r, gpipe);

	Top ()->GetMatrix (&matrix);

	Stack::Push (new Context::Node (target, &matrix, &extents.r));

	// memset is faster than using clear with softpipe
	if (is_softpipe) {
		void                     *data;
		GalliumSurface::Transfer *transfer =
			new GalliumSurface::Transfer (gpipe,
						      surface->Texture ());

		data = transfer->Map ();
		memset (data, 0, r.height * r.width * 4);
		transfer->Unmap ();
		delete transfer;
	}
	else {
		Color color = Color ();

		Clear (&color);
	}

	target->unref ();
	surface->unref ();
}

void
GalliumContext::Push (Group extents, MoonSurface *surface)
{
	cairo_matrix_t matrix;
	Target         *target = new Target (surface, extents.r, gpipe);

	Top ()->GetMatrix (&matrix);

	Stack::Push (new Context::Node (target, &matrix, &extents.r));
	target->unref ();
}

void
GalliumContext::Clear (Color *color)
{
	float clear_color[4] = { color->r, color->g, color->b, color->a };

	cso_save_framebuffer (cso);
	cso_save_rasterizer (cso);

	SetFramebuffer ();
	SetScissor ();
	SetRasterizer ();

	pipe->clear (pipe, PIPE_CLEAR_COLOR, clear_color, 0, 0);
	pipe->flush (pipe, PIPE_FLUSH_RENDER_CACHE, NULL);

	cso_restore_framebuffer (cso);
	cso_restore_rasterizer (cso);
}

void
GalliumContext::Blit (unsigned char *data,
		      int           stride)
{
	Context::Target          *target = Top ()->GetTarget ();
	MoonSurface              *ms;
	Rect                     r = target->GetData (&ms);
	GalliumSurface           *dst = (GalliumSurface *) ms;
	struct pipe_resource     *texture = dst->Texture ();
	unsigned char            *bits;
	GalliumSurface::Transfer *transfer =
		new GalliumSurface::Transfer (gpipe, texture);

	bits = (unsigned char *) transfer->Map ();
	for (unsigned int i = 0; i < texture->height0; i++)
		memcpy (bits + texture->width0 * 4 * i,
			data + stride * i,
			MIN (stride, texture->width0 * 4));

	transfer->Unmap ();
	delete transfer;

	ms->unref ();
}

void *
GalliumContext::GetBlendShader ()
{
	if (!blend_fs)
		blend_fs = util_make_fragment_passthrough_shader (pipe);

	return blend_fs;
}

void
GalliumContext::Paint (Color *color)
{
	struct pipe_resource *vbuf;

	cso_save_blend (cso);
	cso_save_vertex_shader (cso);
	cso_save_fragment_shader (cso);
	cso_save_framebuffer (cso);
	cso_save_viewport (cso);
	cso_save_rasterizer (cso);

	cso_set_blend (cso, &blend_over);
	cso_set_vertex_shader_handle (cso, color_vs);
	cso_set_fragment_shader_handle (cso, GetBlendShader ());

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();
	SetRasterizer ();

	vbuf = SetupVertexData (color);
	if (vbuf) {
		cso_set_vertex_elements (cso, 2, velems);
		util_draw_vertex_buffer (pipe, vbuf, 0,
					 PIPE_PRIM_TRIANGLE_FAN,
					 4,
					 2);

		pipe_resource_reference (&vbuf, NULL);
	}

	cso_restore_blend (cso);
	cso_restore_vertex_shader (cso);
	cso_restore_fragment_shader (cso);
	cso_restore_framebuffer (cso);
	cso_restore_viewport (cso);
	cso_restore_rasterizer (cso);
}

void
GalliumContext::Paint (MoonSurface *src,
		       double      alpha,
		       double      x,
		       double      y)
{
	double m[16];

	Matrix3D::Identity (m);

	Project (src, m, alpha, x, y);
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

	// software optimization that can hopefully be removed one day
	if (is_softpipe) {
		int x0, y0;

		if (Matrix3D::IsIntegerTranslation (matrix, &x0, &y0)) {
			cairo_matrix_t m;

			cairo_matrix_init_translate (&m, x0, y0);

			Context::Push (Context::Transform (m));
			Context::Paint (src, alpha, x, y);
			Context::Pop ();
			return;
		}
	}

	GetDeviceMatrix (m);
	Matrix3D::Multiply (m, matrix, m);

	cso_save_blend (cso);
	cso_save_samplers (cso);
	cso_save_fragment_sampler_views (cso);
	cso_save_vertex_shader (cso);
	cso_save_fragment_shader (cso);
	cso_save_framebuffer (cso);
	cso_save_viewport (cso);
	cso_save_rasterizer (cso);

	cso_set_blend (cso, &blend_over);
	cso_single_sampler (cso, 0, &project_sampler);
	cso_single_sampler_done (cso);
	cso_set_fragment_sampler_views (cso, 1, &view);
	cso_set_vertex_shader_handle (cso, tex_vs);
	cso_set_fragment_shader_handle (cso, GetProjectShader (alpha));

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();
	SetRasterizer ();

	SetConstantBuffer (cbuf, sizeof (cbuf));

	vbuf = SetupVertexData (&project_sampler,
				m,
				x,
				y,
				view->texture->width0,
				view->texture->height0);
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
	cso_restore_vertex_shader (cso);
	cso_restore_fragment_shader (cso);
	cso_restore_framebuffer (cso);
	cso_restore_viewport (cso);
	cso_restore_rasterizer (cso);
}

void *
GalliumContext::GetConvolveShader (unsigned size)
{
	struct ureg_program *ureg;
	struct ureg_src     tex, sampler;
	struct ureg_dst     out, val, off, tmp;
	unsigned            i;

	g_assert (size <= MAX_CONVOLVE_SIZE);

	if (convolve_fs[size])
		return convolve_fs[size];

	ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);

	sampler = ureg_DECL_sampler (ureg, 0);

	tex = ureg_DECL_fs_input (ureg,
				  TGSI_SEMANTIC_GENERIC, 0,
				  TGSI_INTERPOLATE_LINEAR);

	out = ureg_DECL_output (ureg,
				TGSI_SEMANTIC_COLOR,
				0);

	val = ureg_DECL_temporary (ureg);
	off = ureg_DECL_temporary (ureg);
	tmp = ureg_DECL_temporary (ureg);

	ureg_ADD (ureg, tmp, tex, ureg_DECL_constant (ureg, 0));

	tex = ureg_src (tmp);
	tmp = ureg_DECL_temporary (ureg);

	ureg_ADD (ureg, off, tex, ureg_DECL_constant (ureg, 2));
	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
	ureg_MUL (ureg, val, ureg_src (tmp), ureg_DECL_constant (ureg, 3));

	ureg_SUB (ureg, off, tex, ureg_DECL_constant (ureg, 2));
	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
	ureg_MAD (ureg, val, ureg_src (tmp), ureg_DECL_constant (ureg, 3),
		  ureg_src (val));

	for (i = 2; i <= size; i++) {
		ureg_ADD (ureg, off, tex, ureg_DECL_constant (ureg, i * 2));
		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
		ureg_MAD (ureg, val, ureg_src (tmp),
			  ureg_DECL_constant (ureg, i * 2 + 1),
			  ureg_src (val));
		ureg_SUB (ureg, off, tex, ureg_DECL_constant (ureg, i * 2));
		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
		ureg_MAD (ureg, val, ureg_src (tmp),
			  ureg_DECL_constant (ureg, i * 2 + 1),
			  ureg_src (val));
	}

	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, tex, sampler);
	ureg_MAD (ureg, out, ureg_src (tmp), ureg_DECL_constant (ureg, 1),
		  ureg_src (val));

	ureg_END (ureg);

	convolve_fs[size] = ureg_create_shader_and_destroy (ureg, pipe);

	return convolve_fs[size];
}

void
GalliumContext::Blur (MoonSurface *src,
		      double      radius,
		      double      x,
		      double      y)
{
	GalliumSurface           *surface = (GalliumSurface *) src;
	struct pipe_sampler_view *view = surface->SamplerView ();
	struct pipe_resource     *tex = surface->Texture ();
	GalliumSurface           *intermediate;
	struct pipe_resource     *vbuf;
	const double             precision = 1.0 / 256.0;
	double                   values[MAX_CONVOLVE_SIZE + 1];
	float                    cbuf[MAX_CONVOLVE_SIZE + 1][2][4];
	Rect                     r = Rect (0, 0, tex->width0, tex->height0);
	int                      size, i;
	double                   m[16];

	size = ComputeGaussianSamples (radius, precision, values);
	if (size == 0) {
		Matrix3D::Identity (m);
		Project (src, m, 1.0, x, y);
		return;
	}

	// our software implementation is faster than softpipe
	if (is_softpipe) {
		Context::Blur (src, radius, x, y);
		return;
	}

	GetDeviceMatrix (m);

	intermediate = new GalliumSurface (gpipe, r.width, r.height);

	for (i = 0; i <= size; i++) {
		cbuf[i][1][0] = values[i];
		cbuf[i][1][1] = values[i];
		cbuf[i][1][2] = values[i];
		cbuf[i][1][3] = values[i];
	}

	cso_save_blend (cso);
	cso_save_samplers (cso);
	cso_save_fragment_sampler_views (cso);
	cso_save_vertex_shader (cso);
	cso_save_fragment_shader (cso);
	cso_save_framebuffer (cso);
	cso_save_viewport (cso);
	cso_save_rasterizer (cso);

	cso_set_vertex_shader_handle (cso, tex_vs);
	cso_set_fragment_shader_handle (cso, GetConvolveShader (size));

	Context::Push (Group (r), intermediate);

	cso_set_blend (cso, &blend_src);
	cso_single_sampler (cso, 0, &convolve_sampler);
	cso_single_sampler_done (cso);
	cso_set_fragment_sampler_views (cso, 1, &view);

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();
	SetRasterizer ();

	for (i = 0; i <= size; i++) {
		cbuf[i][0][0] = i / r.width;
		cbuf[i][0][1] = 0.0f;
		cbuf[i][0][2] = 0.0f;
		cbuf[i][0][3] = 1.0f;
	}

	SetConstantBuffer (cbuf, sizeof (cbuf[0]) * (size + 1));

	vbuf = SetupVertexData (&convolve_sampler,
				NULL,
				0,
				0,
				tex->width0,
				tex->height0);
	if (vbuf) {
		cso_set_vertex_elements (cso, 2, velems);
		util_draw_vertex_buffer (pipe, vbuf, 0,
					 PIPE_PRIM_TRIANGLE_FAN,
					 4,
					 2);

		pipe_resource_reference (&vbuf, NULL);
	}

	r = Pop (&src);
	if (!r.IsEmpty ()) {
		GalliumSurface           *surface = (GalliumSurface *) src;
		struct pipe_sampler_view *view = surface->SamplerView ();
		struct pipe_resource     *tex = surface->Texture ();

		cso_set_blend (cso, &blend_over);
		cso_single_sampler (cso, 0, &convolve_sampler);
		cso_single_sampler_done (cso);
		cso_set_fragment_sampler_views (cso, 1, &view);

		SetViewport ();
		SetFramebuffer ();
		SetScissor ();
		SetRasterizer ();

		for (i = 0; i <= size; i++) {
			cbuf[i][0][0] = 0.0f;
			cbuf[i][0][1] = i / r.height;
			cbuf[i][0][2] = 0.0f;
			cbuf[i][0][3] = 1.0f;
		}

		SetConstantBuffer (cbuf, sizeof (cbuf[0]) * (size + 1));

		vbuf = SetupVertexData (&convolve_sampler,
					m,
					x,
					y,
					tex->width0,
					tex->height0);
		if (vbuf) {
			cso_set_vertex_elements (cso, 2, velems);
			util_draw_vertex_buffer (pipe, vbuf, 0,
						 PIPE_PRIM_TRIANGLE_FAN,
						 4,
						 2);

			pipe_resource_reference (&vbuf, NULL);
		}

		src->unref ();
	}

	cso_restore_blend (cso);
	cso_restore_samplers (cso);
	cso_restore_fragment_sampler_views (cso);
	cso_restore_vertex_shader (cso);
	cso_restore_fragment_shader (cso);
	cso_restore_framebuffer (cso);
	cso_restore_viewport (cso);
	cso_restore_rasterizer (cso);

	intermediate->unref ();
}

void *
GalliumContext::GetDropShadowShader (unsigned size)
{
	struct ureg_program *ureg;
	struct ureg_src     tex, one, col, img_sampler, sampler;
	struct ureg_dst     out, val, off, img, tmp;
	unsigned            i;

	g_assert (size <= MAX_CONVOLVE_SIZE);

	if (dropshadow_fs[size])
		return dropshadow_fs[size];

	ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);

	img_sampler = ureg_DECL_sampler (ureg, 0);
	sampler = ureg_DECL_sampler (ureg, 1);

	tex = ureg_DECL_fs_input (ureg,
				  TGSI_SEMANTIC_GENERIC, 0,
				  TGSI_INTERPOLATE_LINEAR);

	out = ureg_DECL_output (ureg,
				TGSI_SEMANTIC_COLOR,
				0);

	val = ureg_DECL_temporary (ureg);
	off = ureg_DECL_temporary (ureg);
	img = ureg_DECL_temporary (ureg);
	tmp = ureg_DECL_temporary (ureg);
	col = ureg_DECL_constant (ureg, 0);
	one = ureg_imm4f (ureg, 1.f, 1.f, 1.f, 1.f);

	ureg_TEX (ureg, img, TGSI_TEXTURE_2D, tex, img_sampler);

	ureg_ADD (ureg, off, tex, ureg_DECL_constant (ureg, 2));
	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
	ureg_MUL (ureg, val, ureg_src (tmp), ureg_DECL_constant (ureg, 3));

	ureg_SUB (ureg, off, tex, ureg_DECL_constant (ureg, 2));
	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
	ureg_MAD (ureg, val, ureg_src (tmp), ureg_DECL_constant (ureg, 3),
		  ureg_src (val));

	for (i = 2; i <= size; i++) {
		ureg_ADD (ureg, off, tex, ureg_DECL_constant (ureg, i * 2));
		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
		ureg_MAD (ureg, val, ureg_src (tmp),
			  ureg_DECL_constant (ureg, i * 2 + 1),
			  ureg_src (val));
		ureg_SUB (ureg, off, tex, ureg_DECL_constant (ureg, i * 2));
		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (off), sampler);
		ureg_MAD (ureg, val, ureg_src (tmp),
			  ureg_DECL_constant (ureg, i * 2 + 1),
			  ureg_src (val));
	}

	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, tex, sampler);
	ureg_MAD (ureg, val, ureg_src (tmp), ureg_DECL_constant (ureg, 1),
		  ureg_src (val));

	ureg_MUL (ureg, val, ureg_swizzle (ureg_src (val),
					   TGSI_SWIZZLE_W,
					   TGSI_SWIZZLE_W,
					   TGSI_SWIZZLE_W,
					   TGSI_SWIZZLE_W), col);
	ureg_SUB (ureg, tmp, one, ureg_swizzle (ureg_src (img),
						TGSI_SWIZZLE_W,
						TGSI_SWIZZLE_W,
						TGSI_SWIZZLE_W,
						TGSI_SWIZZLE_W));
	ureg_MAD (ureg, out, ureg_src (tmp), ureg_src (val), ureg_src (img));
	ureg_END (ureg);

	dropshadow_fs[size] = ureg_create_shader_and_destroy (ureg, pipe);

	return dropshadow_fs[size];
}

void
GalliumContext::DropShadow (MoonSurface *src,
			    double      dx,
			    double      dy,
			    double      radius,
			    Color       *color,
			    double      x,
			    double      y)
{
	GalliumSurface           *surface = (GalliumSurface *) src;
	struct pipe_sampler_view *img_view = surface->SamplerView ();
	struct pipe_resource     *tex = surface->Texture ();
	GalliumSurface           *intermediate;
	struct pipe_resource     *vbuf;
	const double             precision = 1.0 / 256.0;
	double                   values[MAX_CONVOLVE_SIZE + 1];
	float                    cbuf[MAX_CONVOLVE_SIZE + 1][2][4];
	Rect                     r = Rect (0, 0, tex->width0, tex->height0);
	int                      size, i;
	double                   m[16];

	// our software implementation is faster than softpipe
	if (is_softpipe) {
		Context::DropShadow (src, dx, dy, radius, color, x, y);
		return;
	}

	size = ComputeGaussianSamples (radius, precision, values);

	GetDeviceMatrix (m);

	intermediate = new GalliumSurface (gpipe, r.width, r.height);

	for (i = 0; i <= size; i++) {
		cbuf[i][1][0] = values[i];
		cbuf[i][1][1] = values[i];
		cbuf[i][1][2] = values[i];
		cbuf[i][1][3] = values[i];
	}

	cso_save_blend (cso);
	cso_save_samplers (cso);
	cso_save_fragment_sampler_views (cso);
	cso_save_vertex_shader (cso);
	cso_save_fragment_shader (cso);
	cso_save_framebuffer (cso);
	cso_save_viewport (cso);
	cso_save_rasterizer (cso);

	Context::Push (Group (r), intermediate);

	cso_set_blend (cso, &blend_src);
	cso_single_sampler (cso, 0, &convolve_sampler);
	cso_single_sampler_done (cso);
	cso_set_fragment_sampler_views (cso, 1, &img_view);
	cso_set_vertex_shader_handle (cso, tex_vs);
	cso_set_fragment_shader_handle (cso, GetConvolveShader (size));

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();
	SetRasterizer ();

	cbuf[0][0][0] = dx / r.width;
	cbuf[0][0][1] = dy / r.height;
	cbuf[0][0][2] = 0.0f;
	cbuf[0][0][3] = 0.0f;

	for (i = 1; i <= size; i++) {
		cbuf[i][0][0] = i / r.width;
		cbuf[i][0][1] = 0.0f;
		cbuf[i][0][2] = 0.0f;
		cbuf[i][0][3] = 0.0f;
	}

	SetConstantBuffer (cbuf, sizeof (cbuf[0]) * (size + 1));

	vbuf = SetupVertexData (&convolve_sampler,
				NULL,
				0,
				0,
				tex->width0,
				tex->height0);
	if (vbuf) {
		cso_set_vertex_elements (cso, 2, velems);
		util_draw_vertex_buffer (pipe, vbuf, 0,
					 PIPE_PRIM_TRIANGLE_FAN,
					 4,
					 2);

		pipe_resource_reference (&vbuf, NULL);
	}

	r = Pop (&src);
	if (!r.IsEmpty ()) {
		GalliumSurface           *surface = (GalliumSurface *) src;
		struct pipe_sampler_view *view = surface->SamplerView ();
		struct pipe_resource     *tex = surface->Texture ();
		struct pipe_sampler_view *views[] = { img_view, view };

		cso_set_blend (cso, &blend_over);
		cso_single_sampler (cso, 0, &convolve_sampler);
		cso_single_sampler (cso, 1, &convolve_sampler);
		cso_single_sampler_done (cso);
		cso_set_fragment_sampler_views (cso, 2, views);
		cso_set_vertex_shader_handle (cso, tex_vs);
		cso_set_fragment_shader_handle (cso,
						GetDropShadowShader (size));

		SetViewport ();
		SetFramebuffer ();
		SetScissor ();
		SetRasterizer ();

		cbuf[0][0][0] = color->r;
		cbuf[0][0][1] = color->g;
		cbuf[0][0][2] = color->b;
		cbuf[0][0][3] = color->a;

		for (i = 1; i <= size; i++) {
			cbuf[i][0][0] = 0.0f;
			cbuf[i][0][1] = i / r.height;
			cbuf[i][0][2] = 0.0f;
			cbuf[i][0][3] = 0.0f;
		}

		SetConstantBuffer (cbuf, sizeof (cbuf[0]) * (size + 1));

		vbuf = SetupVertexData (&convolve_sampler,
					m,
					x,
					y,
					tex->width0,
					tex->height0);
		if (vbuf) {
			cso_set_vertex_elements (cso, 2, velems);
			util_draw_vertex_buffer (pipe, vbuf, 0,
						 PIPE_PRIM_TRIANGLE_FAN,
						 4,
						 2);

			pipe_resource_reference (&vbuf, NULL);
		}

		src->unref ();
	}

	cso_restore_blend (cso);
	cso_restore_samplers (cso);
	cso_restore_fragment_sampler_views (cso);
	cso_restore_vertex_shader (cso);
	cso_restore_fragment_shader (cso);
	cso_restore_framebuffer (cso);
	cso_restore_viewport (cso);
	cso_restore_rasterizer (cso);

	intermediate->unref ();
}

static bool
ureg_check_aliasing (const struct ureg_dst *dst,
		     const struct ureg_src *src)
{
	unsigned writemask = dst->WriteMask;
	unsigned channelsWritten = 0x0;
   
	if (writemask == TGSI_WRITEMASK_X ||
	    writemask == TGSI_WRITEMASK_Y ||
	    writemask == TGSI_WRITEMASK_Z ||
	    writemask == TGSI_WRITEMASK_W ||
	    writemask == TGSI_WRITEMASK_NONE)
		return FALSE;

	if ((src->File != dst->File) || (src->Index != dst->Index))
		return false;

	if (writemask & TGSI_WRITEMASK_X) {
		if (channelsWritten & (1 << src->SwizzleX))
			return true;

		channelsWritten |= TGSI_WRITEMASK_X;
	}

	if (writemask & TGSI_WRITEMASK_Y) {
		if (channelsWritten & (1 << src->SwizzleY))
			return true;

		channelsWritten |= TGSI_WRITEMASK_Y;
	}

	if (writemask & TGSI_WRITEMASK_Z) {
		if (channelsWritten & (1 << src->SwizzleZ))
			return true;

		channelsWritten |= TGSI_WRITEMASK_Z;
	}

	if (writemask & TGSI_WRITEMASK_W) {
		if (channelsWritten & (1 << src->SwizzleW))
			return true;

		channelsWritten |= TGSI_WRITEMASK_W;
	}

	return false;
}

#define ERROR_IF(EXP)							\
	do { if (EXP) {							\
			ShaderEffect::ShaderError (ps,			\
				     "Shader error (" #EXP ") at "	\
				     "instruction %.2d", n);		\
			ureg_destroy (ureg); return default_fs; }	\
	} while (0)

void *
GalliumContext::GetEffectShader (PixelShader *ps)
{
	struct cso_hash_iter iter;
	void                *fs = NULL;
	struct ureg_program *ureg;
	d3d_version_t       version;
	d3d_op_t            op;
	int                 index;
	struct ureg_src     src_reg[D3DSPR_LAST][MAX_CONSTANTS];
	struct ureg_dst     dst_reg[D3DSPR_LAST][MAX_CONSTANTS];
	int                 n = 0;

	// TODO: release effect shaders when destroyed
	iter = cso_hash_find (effect_fs, (unsigned) (long) ps);
	if (!cso_hash_iter_is_null (iter))
		return cso_hash_iter_data (iter);

	if ((index = ps->GetVersion (0, &version)) < 0)
		return default_fs;

	if (version.type  != 0xffff ||
	    version.major != 2      ||
	    version.minor != 0) {
		ShaderEffect::ShaderError (ps, "Unsupported pixel shader");
		return default_fs;
	}

	ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);
	if (!ureg)
		return default_fs;

	for (int i = 0; i < D3DSPR_LAST; i++) {
		for (int j = 0; j < MAX_CONSTANTS; j++) {
			src_reg[i][j] = ureg_src_undef ();
			dst_reg[i][j] = ureg_dst_undef ();
		}
	}

	dst_reg[D3DSPR_COLOROUT][0] = ureg_DECL_output (ureg,
							TGSI_SEMANTIC_COLOR,
							0);

	/* validation and register allocation */
	for (int i = ps->GetOp (index, &op); i > 0; i = ps->GetOp (i, &op)) {
		d3d_destination_parameter_t reg;
		d3d_source_parameter_t      src;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		if (op.type == D3DSIO_END)
			break;

		switch (op.type) {
				// case D3DSIO_DEFB:
				// case D3DSIO_DEFI:
			case D3DSIO_DEF: {
				d3d_def_instruction_t def;

				i = ps->GetInstruction (i, &def);

				ERROR_IF (def.reg.writemask != 0xf);
				ERROR_IF (def.reg.dstmod != 0);
				ERROR_IF (def.reg.regnum >= MAX_CONSTANTS);

				src_reg[def.reg.regtype][def.reg.regnum] =
					ureg_DECL_immediate (ureg, def.v, 4);
			} break;
			case D3DSIO_DCL: {
				d3d_dcl_instruction_t dcl;

				i = ps->GetInstruction (i, &dcl);

				ERROR_IF (dcl.reg.dstmod != 0);
				ERROR_IF (dcl.reg.regnum >= MAX_CONSTANTS);
				ERROR_IF (dcl.reg.regnum >= MAX_SAMPLERS);
				ERROR_IF (dcl.reg.regtype != D3DSPR_SAMPLER &&
					  dcl.reg.regtype != D3DSPR_TEXTURE);

				switch (dcl.reg.regtype) {
					case D3DSPR_SAMPLER:
						src_reg[D3DSPR_SAMPLER][dcl.reg.regnum] =
							ureg_DECL_sampler (ureg, dcl.reg.regnum);
						break;
					case D3DSPR_TEXTURE:
						src_reg[D3DSPR_TEXTURE][dcl.reg.regnum] =
							ureg_DECL_fs_input (ureg,
									    TGSI_SEMANTIC_GENERIC,
									    dcl.reg.regnum,
									    TGSI_INTERPOLATE_LINEAR);
					default:
						break;
				}
			} break;
			default: {
				unsigned ndstparam = op.meta.ndstparam;
				unsigned nsrcparam = op.meta.nsrcparam;
				int      j = i;

				n++;

				while (ndstparam--) {
					j = ps->GetDestinationParameter (j, &reg);

					ERROR_IF (reg.regnum >= MAX_CONSTANTS);
					ERROR_IF (reg.dstmod != D3DSPD_NONE &&
						  reg.dstmod != D3DSPD_SATURATE);
					ERROR_IF (reg.regtype != D3DSPR_TEMP &&
						  reg.regtype != D3DSPR_COLOROUT);

					if (reg.regtype == D3DSPR_TEMP) {
						if (ureg_dst_is_undef (dst_reg[D3DSPR_TEMP][reg.regnum])) {
							struct ureg_dst tmp = ureg_DECL_temporary (ureg);

							dst_reg[D3DSPR_TEMP][reg.regnum] = tmp;
							src_reg[D3DSPR_TEMP][reg.regnum] = ureg_src (tmp);
						}
					}

					ERROR_IF (ureg_dst_is_undef (dst_reg[reg.regtype][reg.regnum]));
					ERROR_IF (op.type == D3DSIO_SINCOS && (reg.writemask & ~0x3) != 0);
				}

				while (nsrcparam--) {
					j = ps->GetSourceParameter (j, &src);

					ERROR_IF (src.regnum >= MAX_CONSTANTS);
					ERROR_IF (src.srcmod != D3DSPS_NONE &&
						  src.srcmod != D3DSPS_NEGATE &&
						  src.srcmod != D3DSPS_ABS);
					ERROR_IF (src.regtype != D3DSPR_TEMP &&
						  src.regtype != D3DSPR_CONST &&
						  src.regtype != D3DSPR_SAMPLER &&
						  src.regtype != D3DSPR_TEXTURE);

					if (src.regtype == D3DSPR_CONST) {
						if (ureg_src_is_undef (src_reg[D3DSPR_CONST][src.regnum]))
							src_reg[D3DSPR_CONST][src.regnum] =
								ureg_DECL_constant (ureg, src.regnum);
					}

					ERROR_IF (ureg_src_is_undef (src_reg[src.regtype][src.regnum]));
				}

				if (!op.meta.name) {
					ShaderEffect::ShaderError (ps, "Unknown shader instruction %.2d", n);
					ureg_destroy (ureg);
					return default_fs;
				}

				i += op.length;
			} break;
		}
	}

	for (int i = ps->GetOp (index, &op); i > 0; i = ps->GetOp (i, &op)) {
		d3d_destination_parameter_t reg[8];
		d3d_source_parameter_t      source[8];
		struct ureg_dst             dst[8];
		struct ureg_dst             src_tmp[8];
		struct ureg_src             src[8];
		int                         j = i;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		for (unsigned k = 0; k < op.meta.ndstparam; k++) {
			j = ps->GetDestinationParameter (j, &reg[k]);
			dst[k] = dst_reg[reg[k].regtype][reg[k].regnum];

			switch (reg[k].dstmod) {
				case D3DSPD_SATURATE:
					dst[k] = ureg_saturate (dst[k]);
					break;
			}

			dst[k] = ureg_writemask (dst[k], reg[k].writemask);
		}

		for (unsigned k = 0; k < op.meta.nsrcparam; k++) {
			j = ps->GetSourceParameter (j, &source[k]);
			src[k] = src_reg[source[k].regtype][source[k].regnum];
			src_tmp[k] = ureg_dst_undef ();

			switch (source[k].srcmod) {
				case D3DSPS_NEGATE:
					src[k] = ureg_negate (src[k]);
					break;
				case D3DSPS_ABS:
					src[k] = ureg_abs (src[k]);
					break;
			}

			src[k] = ureg_swizzle (src[k],
					       source[k].swizzle.x,
					       source[k].swizzle.y,
					       source[k].swizzle.z,
					       source[k].swizzle.w);

			if (op.type != D3DSIO_SINCOS) {
				if (op.meta.ndstparam && ureg_check_aliasing (&dst[0], &src[k])) {
					src_tmp[k] = ureg_DECL_temporary (ureg);
					ureg_MOV (ureg, src_tmp[k], src[k]);
					src[k] = ureg_src (src_tmp[k]);
				}
			}
		}

		i += op.length;

		switch (op.type) {
			case D3DSIO_NOP:
				ureg_NOP (ureg);
				break;
				// case D3DSIO_BREAK: break;
				// case D3DSIO_BREAKC: break;
				// case D3DSIO_BREAKP: break;
				// case D3DSIO_CALL: break;
				// case D3DSIO_CALLNZ: break;
				// case D3DSIO_LOOP: break;
				// case D3DSIO_RET: break;
				// case D3DSIO_ENDLOOP: break;
				// case D3DSIO_LABEL: break;
				// case D3DSIO_REP: break;
				// case D3DSIO_ENDREP: break;
				// case D3DSIO_IF: break;
				// case D3DSIO_IFC: break;
				// case D3DSIO_ELSE: break;
				// case D3DSIO_ENDIF: break;
			case D3DSIO_MOV:
				ureg_MOV (ureg, dst[0], src[0]);
				break;
			case D3DSIO_ADD:
				ureg_ADD (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_SUB:
				ureg_SUB (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_MAD:
				ureg_MAD (ureg, dst[0], src[0], src[1], src[2]);
				break;
			case D3DSIO_MUL:
				ureg_MUL (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_RCP:
				ureg_RCP (ureg, dst[0], src[0]);
				break;
			case D3DSIO_RSQ:
				ureg_RSQ (ureg, dst[0], src[0]);
				break;
			case D3DSIO_DP3:
				ureg_DP3 (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_DP4:
				ureg_DP4 (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_MIN:
				ureg_MIN (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_MAX:
				ureg_MAX (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_SLT:
				ureg_SLT (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_SGE:
				ureg_SGE (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_EXP:
				ureg_EXP (ureg, dst[0], src[0]);
				break;
			case D3DSIO_LOG:
				ureg_LOG (ureg, dst[0], src[0]);
				break;
			case D3DSIO_LIT:
				ureg_LIT (ureg, dst[0], src[0]);
				break;
			case D3DSIO_DST:
				ureg_DST (ureg, dst[0], src[0], src[1]);
				break;
			case D3DSIO_LRP:
				ureg_LRP (ureg, dst[0], src[0], src[1], src[2]);
				break;
			case D3DSIO_FRC:
				ureg_FRC (ureg, dst[0], src[0]);
				break;
				// case D3DSIO_M4x4: break;
				// case D3DSIO_M4x3: break;
				// case D3DSIO_M3x4: break;
				// case D3DSIO_M3x3: break;
				// case D3DSIO_M3x2: break;
			case D3DSIO_POW:
				ureg_POW (ureg, dst[0], src[0], src[1]);
				break;
				// case D3DSIO_CRS: break;
				// case D3DSIO_SGN: break;
			case D3DSIO_ABS:
				ureg_ABS (ureg, dst[0], src[0]);
				break;
			case D3DSIO_NRM:
				ureg_NRM (ureg, dst[0], src[0]);
				break;
			case D3DSIO_SINCOS:
				struct ureg_dst v1, v2, v3, v;

				v1 = ureg_DECL_temporary (ureg);
				v2 = ureg_DECL_temporary (ureg);
				v3 = ureg_DECL_temporary (ureg);
				v  = ureg_DECL_temporary (ureg);

				ureg_MOV (ureg, v1, src[0]);
				ureg_MOV (ureg, v2, src[1]);
				ureg_MOV (ureg, v3, src[2]);

				 // x * x
				ureg_MUL (ureg, ureg_writemask (v, TGSI_WRITEMASK_Z),
					  ureg_swizzle (ureg_src (v1),
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W),
					  ureg_swizzle (ureg_src (v1),
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W));

				ureg_MAD (ureg, ureg_writemask (v, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y),
					  ureg_swizzle (ureg_src (v),
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z),
					  ureg_src (v2),
					  ureg_swizzle (ureg_src (v2),
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_W));

				ureg_MAD (ureg, ureg_writemask (v, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y),
					  ureg_src (v),
					  ureg_swizzle (ureg_src (v),
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z),
					  ureg_src (v3));

				// partial sin( x/2 ) and final cos( x/2 )
				ureg_MAD (ureg, ureg_writemask (v, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y),
					  ureg_src (v),
					  ureg_swizzle (ureg_src (v),
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z),
					  ureg_swizzle (ureg_src (v3),
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_W));

				// sin( x/2 )
				ureg_MUL (ureg, ureg_writemask (v, TGSI_WRITEMASK_X),
					  ureg_src (v),
					  ureg_swizzle (ureg_src (v1),
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W));

				 // compute sin( x/2 ) * sin( x/2 ) and sin( x/2 ) * cos( x/2 )
				ureg_MUL (ureg, ureg_writemask (v1, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y),
					  ureg_src (v),
					  ureg_swizzle (ureg_src (v),
							TGSI_SWIZZLE_X,
							TGSI_SWIZZLE_X,
							TGSI_SWIZZLE_X,
							TGSI_SWIZZLE_X));

				// 2 * sin( x/2 ) * sin( x/2 ) and 2 * sin( x/2 ) * cos( x/2 )
				ureg_ADD (ureg, ureg_writemask (v, TGSI_WRITEMASK_X | TGSI_WRITEMASK_Y),
					  ureg_src (v1),
					  ureg_src (v1));

				// cos( x ) and sin( x )
				ureg_SUB (ureg, ureg_writemask (v, TGSI_WRITEMASK_X),
					  ureg_swizzle (ureg_src (v3),
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z,
							TGSI_SWIZZLE_Z),
					  ureg_src (v));

				ureg_MOV (ureg, dst[0], ureg_src (v));

				ureg_release_temporary (ureg, v1);
				ureg_release_temporary (ureg, v2);
				ureg_release_temporary (ureg, v3);
				ureg_release_temporary (ureg, v);
				break;
				// case D3DSIO_MOVA: break;
				// case D3DSIO_TEXCOORD: break;
				// case D3DSIO_TEXKILL: break;
			case D3DSIO_TEX:
				ureg_TEX (ureg, dst[0], TGSI_TEXTURE_2D, src[0], src[1]);
				break;
				// case D3DSIO_TEXBEM: break;
				// case D3DSIO_TEXBEML: break;
				// case D3DSIO_TEXREG2AR: break;
				// case D3DSIO_TEXREG2GB: break;
				// case D3DSIO_TEXM3x2PAD: break;
				// case D3DSIO_TEXM3x2TEX: break;
				// case D3DSIO_TEXM3x3PAD: break;
				// case D3DSIO_TEXM3x3TEX: break;
				// case D3DSIO_RESERVED0: break;
				// case D3DSIO_TEXM3x3SPEC: break;
				// case D3DSIO_TEXM3x3VSPEC: break;
				// case D3DSIO_EXPP: break;
				// case D3DSIO_LOGP: break;
			case D3DSIO_CND:
				ureg_CND (ureg, dst[0], src[0], src[1], src[2]);
				break;
				// case D3DSIO_TEXREG2RGB: break;
				// case D3DSIO_TEXDP3TEX: break;
				// case D3DSIO_TEXM3x2DEPTH: break;
				// case D3DSIO_TEXDP3: break;
				// case D3DSIO_TEXM3x3: break;
				// case D3DSIO_TEXDEPTH: break;
			case D3DSIO_CMP:
				/* direct3d does src0 >= 0, while TGSI does src0 < 0 */
				ureg_CMP (ureg, dst[0], src[0], src[2], src[1]);
				break;
				// case D3DSIO_BEM: break;
			case D3DSIO_DP2ADD:
				ureg_DP2A (ureg, dst[0], src[0], src[1], src[2]);
				break;
				// case D3DSIO_DSX: break;
				// case D3DSIO_DSY: break;
				// case D3DSIO_TEXLDD: break;
				// case D3DSIO_SETP: break;
				// case D3DSIO_TEXLDL: break;
			case D3DSIO_END:
				ureg_END (ureg);
				fs = ureg_create_shader_and_destroy (ureg, pipe);
				cso_hash_insert (effect_fs,
						 (unsigned) (long) ps,
						 fs);
				return fs;
			default:
				break;
		}

		for (unsigned k = 0; k < op.meta.nsrcparam; k++)
			if (!ureg_dst_is_undef (src_tmp[k]))
				ureg_release_temporary (ureg, src_tmp[k]);
	}

	return default_fs;
}

void
GalliumContext::ShaderEffect (MoonSurface *src,
			      PixelShader *shader,
			      Brush       **sampler,
			      int         *sampler_mode,
			      int         n_sampler,
			      Color       *constant,
			      int         n_constant,
			      int         *ddxUvDdyUvPtr,
			      double      x,
			      double      y)
{
	GalliumSurface           *surface = (GalliumSurface *) src;
	GalliumSurface           *input[PIPE_MAX_SAMPLERS];
	struct pipe_sampler_view *sampler_views[PIPE_MAX_SAMPLERS];
	struct pipe_resource     *tex = surface->Texture ();
	struct pipe_resource     *vbuf;
	float                    cbuf[MAX_CONSTANTS][4];
	double                   m[16];
	int                      i;

	g_assert (n_sampler <= PIPE_MAX_SAMPLERS);
	g_assert (n_constant <= MAX_CONSTANTS);
	g_assert (!ddxUvDdyUvPtr || *ddxUvDdyUvPtr < MAX_CONSTANTS);

	GetDeviceMatrix (m);

	for (i = 0; i < n_sampler; i++) {
		struct pipe_sampler_view *sampler_view = NULL;

		if (sampler[i]) {
			input[i] = new GalliumSurface (gpipe,
						       tex->width0,
						       tex->height0);
			if (input[i]) {
				cairo_surface_t *surface = input[i]->Cairo ();
				cairo_t         *cr = cairo_create (surface);
				Rect            area = Rect (0.0,
							     0.0,
							     tex->width0,
							     tex->height0);

				sampler[i]->SetupBrush (cr, area);

				cairo_paint (cr);
				cairo_destroy (cr);
				cairo_surface_destroy (surface);

				sampler_view = input[i]->SamplerView ();
			}
		}
		else {
			input[i] = NULL;
			sampler_view = surface->SamplerView ();
		}

		if (!sampler_view) {
			g_warning ("GalliumContext::ShaderEffect: failed to "
				   "generate input texture for sampler "
				   "register %d", i);
			sampler_view = surface->SamplerView ();
		}

		sampler_views[i] = sampler_view;
	}

	cso_save_blend (cso);
	cso_save_samplers (cso);
	cso_save_fragment_sampler_views (cso);
	cso_save_fragment_shader (cso);
	cso_save_framebuffer (cso);
	cso_save_viewport (cso);
	cso_save_rasterizer (cso);

	cso_set_blend (cso, &blend_over);
	for (i = 0; i < n_sampler; i++) {
		struct pipe_sampler_state sampler = effect_sampler;
		unsigned                  mode;

		switch (sampler_mode[i]) {
			case 2:
				mode = PIPE_TEX_FILTER_LINEAR;
				break;
			default:
				mode = PIPE_TEX_FILTER_NEAREST;
				break;
		}

		sampler.min_img_filter = mode;
		sampler.mag_img_filter = mode;

		cso_single_sampler (cso, i, &sampler);
	}
	cso_single_sampler_done (cso);
	cso_set_fragment_sampler_views (cso, n_sampler, sampler_views);
	cso_set_vertex_shader_handle (cso, tex_vs);
	cso_set_fragment_shader_handle (cso, GetEffectShader (shader));

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();
	SetRasterizer ();

	for (i = 0; i < n_constant; i++) {
		cbuf[i][0] = constant[i].r;
		cbuf[i][1] = constant[i].g;
		cbuf[i][2] = constant[i].b;
		cbuf[i][3] = constant[i].a;
	}

	if (ddxUvDdyUvPtr) {
		cbuf[*ddxUvDdyUvPtr][0] = 1.0f / tex->width0;
		cbuf[*ddxUvDdyUvPtr][1] = 0.0f;
		cbuf[*ddxUvDdyUvPtr][2] = 0.0f;
		cbuf[*ddxUvDdyUvPtr][3] = 1.0f / tex->height0;
	}

	SetConstantBuffer (cbuf, sizeof (cbuf[0]) * n_constant);

	vbuf = SetupVertexData (&effect_sampler,
				m,
				x,
				y,
				tex->width0,
				tex->height0);
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
	cso_restore_rasterizer (cso);

	for (i = 0; i < n_sampler; i++)
		if (input[i])
			input[i]->unref ();
}

void
GalliumContext::Flush ()
{
	pipe->flush (pipe, PIPE_FLUSH_RENDER_CACHE, NULL);
}

};
