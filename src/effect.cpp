/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <cairo.h>
#include <glib.h>

#include "effect.h"
#include "application.h"
#include "eventargs.h"
#include "uri.h"

struct st_context *Effect::st_context;

cairo_user_data_key_t Effect::textureKey;
cairo_user_data_key_t Effect::surfaceKey;

#ifdef USE_GALLIUM
#undef CLAMP

#include "pipe/internal/p_winsys_screen.h"/* port to just p_screen */
#include "pipe/p_format.h"
#include "pipe/p_context.h"
#include "pipe/p_inlines.h"
#include "softpipe/sp_winsys.h"
#include "cso_cache/cso_context.h"
#include "pipe/p_state.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_inlines.h"
#include "util/u_draw_quad.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_simple_shaders.h"
#include "tgsi/tgsi_text.h"
#include "tgsi/tgsi_ureg.h"
#include "trace/tr_screen.h"
#include "trace/tr_context.h"

struct pipe_screen;
struct pipe_context;

struct st_winsys
{
	struct pipe_winsys base;

	struct pipe_screen  *(*screen_create)  (struct st_winsys *);
	struct pipe_context *(*context_create) (struct pipe_screen *screen);

	void     *user_data;
	unsigned user_stride;
};

struct st_softpipe_buffer
{
	struct pipe_buffer base;
	boolean userBuffer;  /** Is this a user-space buffer? */
	void *data;
	void *mapped;
};

struct cso_context;
struct pipe_screen;
struct pipe_context;
struct st_winsys;

struct st_context {
	struct pipe_reference reference;

	struct st_device *st_dev;

	struct pipe_context *pipe;

	struct cso_context *cso;

	void *vs;
	void *fs;

	struct pipe_texture *default_texture;
	struct pipe_texture *sampler_textures[PIPE_MAX_SAMPLERS];

	unsigned num_vertex_buffers;
	struct pipe_vertex_buffer vertex_buffers[PIPE_MAX_ATTRIBS];

	unsigned num_vertex_elements;
	struct pipe_vertex_element vertex_elements[PIPE_MAX_ATTRIBS];

	struct pipe_framebuffer_state framebuffer;
};

struct st_device {
	struct pipe_reference reference;

	struct st_winsys st_ws;

	struct pipe_screen *screen;
};

static void
st_device_really_destroy (struct st_device *st_dev)
{
	if (st_dev->screen)
		st_dev->screen->destroy (st_dev->screen);

	FREE (st_dev);
}

static void
st_device_reference (struct st_device **ptr, struct st_device *st_dev)
{
	struct st_device *old_dev = *ptr;

	if (pipe_reference (&(*ptr)->reference, &st_dev->reference))
		st_device_really_destroy (old_dev);
	*ptr = st_dev;
}

static struct st_device *
st_device_create_from_st_winsys (const struct st_winsys *st_ws)
{
	struct st_device *st_dev;

	if (!st_ws->screen_create || !st_ws->context_create)
		return NULL;

	st_dev = CALLOC_STRUCT (st_device);
	if (!st_dev)
		return NULL;

	pipe_reference_init (&st_dev->reference, 1);
	st_dev->st_ws = *st_ws;

	st_dev->screen = st_ws->screen_create (&st_dev->st_ws);
	if (!st_dev->screen) {
		st_device_reference (&st_dev, NULL);
		return NULL;
	}

	return st_dev;
}

static void
st_context_really_destroy (struct st_context *st_ctx)
{
	unsigned i;

	if (st_ctx) {
		struct st_device *st_dev = st_ctx->st_dev;

		if (st_ctx->cso) {
			cso_delete_vertex_shader (st_ctx->cso, st_ctx->vs);
			cso_delete_fragment_shader (st_ctx->cso, st_ctx->fs);

			cso_destroy_context (st_ctx->cso);
		}

		if (st_ctx->pipe)
			st_ctx->pipe->destroy (st_ctx->pipe);

		for(i = 0; i < PIPE_MAX_SAMPLERS; ++i)
			pipe_texture_reference (&st_ctx->sampler_textures[i], NULL);
		pipe_texture_reference (&st_ctx->default_texture, NULL);

		FREE (st_ctx);

		st_device_reference (&st_dev, NULL);
	}
}

static struct st_context *
st_context_create (struct st_device *st_dev)
{
	struct st_context *st_ctx;

	st_ctx = CALLOC_STRUCT (st_context);
	if (!st_ctx)
		return NULL;

	pipe_reference_init (&st_ctx->reference, 1);

	st_device_reference (&st_ctx->st_dev, st_dev);

	st_ctx->pipe = st_dev->st_ws.context_create (st_dev->screen);
	if (!st_ctx->pipe) {
		st_context_really_destroy (st_ctx);
		return NULL;
	}

	st_ctx->cso = cso_create_context (st_ctx->pipe);
	if (!st_ctx->cso) {
		st_context_really_destroy (st_ctx);
		return NULL;
	}

	/* disable blending/masking */
	{
		struct pipe_blend_state blend;
		memset(&blend, 0, sizeof(blend));
		blend.blend_enable = 1;
		blend.colormask |= PIPE_MASK_RGBA;
		blend.rgb_src_factor = PIPE_BLENDFACTOR_ONE;
		blend.alpha_src_factor = PIPE_BLENDFACTOR_ONE;
		blend.rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
		blend.alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
		cso_set_blend(st_ctx->cso, &blend);
	}

	/* no-op depth/stencil/alpha */
	{
		struct pipe_depth_stencil_alpha_state depthstencil;
		memset(&depthstencil, 0, sizeof(depthstencil));
		cso_set_depth_stencil_alpha(st_ctx->cso, &depthstencil);
	}

	/* rasterizer */
	{
		struct pipe_rasterizer_state rasterizer;
		memset(&rasterizer, 0, sizeof(rasterizer));
		rasterizer.front_winding = PIPE_WINDING_CW;
		rasterizer.cull_mode = PIPE_WINDING_NONE;
		cso_set_rasterizer(st_ctx->cso, &rasterizer);
	}

	/* clip */
	{
		struct pipe_clip_state clip;
		memset(&clip, 0, sizeof(clip));
		st_ctx->pipe->set_clip_state(st_ctx->pipe, &clip);
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
		cso_set_viewport(st_ctx->cso, &viewport);
	}

	/* samplers */
	{
		struct pipe_sampler_state sampler;
		unsigned i;
		memset(&sampler, 0, sizeof(sampler));
		sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
		sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
		sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
		sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NEAREST;
		sampler.min_img_filter = PIPE_TEX_MIPFILTER_NEAREST;
		sampler.mag_img_filter = PIPE_TEX_MIPFILTER_NEAREST;
		sampler.normalized_coords = 1;
		for (i = 0; i < PIPE_MAX_SAMPLERS; i++)
			cso_single_sampler(st_ctx->cso, i, &sampler);
		cso_single_sampler_done(st_ctx->cso);
	}

	/* default textures */
	{
		struct pipe_screen *screen = st_dev->screen;
		struct pipe_texture templat;
		struct pipe_transfer *transfer;
		unsigned i;

		memset( &templat, 0, sizeof( templat ) );
		templat.target = PIPE_TEXTURE_2D;
		templat.format = PIPE_FORMAT_A8R8G8B8_UNORM;
		templat.width0 = 1;
		templat.height0 = 1;
		templat.depth0 = 1;
		templat.last_level = 0;

		st_ctx->default_texture = screen->texture_create( screen, &templat );
		if(st_ctx->default_texture) {
			transfer = screen->get_tex_transfer(screen,
							    st_ctx->default_texture,
							    0, 0, 0,
							    PIPE_TRANSFER_WRITE,
							    0, 0,
							    st_ctx->default_texture->width0,
							    st_ctx->default_texture->height0);
			if (transfer) {
				uint32_t *map;
				map = (uint32_t *) screen->transfer_map(screen, transfer);
				if(map) {
					*map = 0x00000000;
					screen->transfer_unmap(screen, transfer);
				}
				screen->tex_transfer_destroy(transfer);
			}
		}

		for (i = 0; i < PIPE_MAX_SAMPLERS; i++)
			pipe_texture_reference(&st_ctx->sampler_textures[i], st_ctx->default_texture);

		cso_set_sampler_textures(st_ctx->cso, PIPE_MAX_SAMPLERS, st_ctx->sampler_textures);
	}

	/* vertex shader */
	{
		const uint semantic_names[] = { TGSI_SEMANTIC_POSITION,
						TGSI_SEMANTIC_GENERIC };
		const uint semantic_indexes[] = { 0, 0 };

		st_ctx->vs = util_make_vertex_passthrough_shader (st_ctx->pipe,
								  2,
								  semantic_names,
								  semantic_indexes);
		cso_set_vertex_shader_handle (st_ctx->cso, st_ctx->vs);
	}

	/* fragment shader */
	{
		st_ctx->fs = util_make_fragment_tex_shader (st_ctx->pipe);
		cso_set_fragment_shader_handle (st_ctx->cso, st_ctx->fs);
	}

	return st_ctx;
}

static void
st_context_reference (struct st_context **ptr, struct st_context *st_ctx)
{
	struct st_context *old_ctx = *ptr;

	if (pipe_reference (&(*ptr)->reference, &st_ctx->reference))
		st_context_really_destroy (old_ctx);
	*ptr = st_ctx;
}

static struct st_softpipe_buffer *
st_softpipe_buffer (struct pipe_buffer *buf)
{
	return (struct st_softpipe_buffer *) buf;
}

static void *
st_softpipe_buffer_map (struct pipe_winsys *winsys,
			struct pipe_buffer *buf,
			unsigned flags)
{
	struct st_softpipe_buffer *st_softpipe_buf = st_softpipe_buffer (buf);
	st_softpipe_buf->mapped = st_softpipe_buf->data;
	return st_softpipe_buf->mapped;
}

static void
st_softpipe_buffer_unmap (struct pipe_winsys *winsys,
			  struct pipe_buffer *buf)
{
	struct st_softpipe_buffer *st_softpipe_buf = st_softpipe_buffer (buf);
	st_softpipe_buf->mapped = NULL;
}

static void
st_softpipe_buffer_destroy (struct pipe_buffer *buf)
{
	struct st_softpipe_buffer *oldBuf = st_softpipe_buffer(buf);

	if (oldBuf->data) {
		if (!oldBuf->userBuffer)
			align_free (oldBuf->data);

		oldBuf->data = NULL;
	}

	FREE (oldBuf);
}

static void
st_softpipe_flush_frontbuffer (struct pipe_winsys *winsys,
			       struct pipe_surface *surf,
			       void *context_private)
{
}

static const char *
st_softpipe_get_name (struct pipe_winsys *winsys)
{
	return "moon-softpipe";
}

static struct pipe_buffer *
st_softpipe_buffer_create (struct pipe_winsys *winsys,
			   unsigned alignment,
			   unsigned usage,
			   unsigned size)
{
	struct st_softpipe_buffer *buffer = CALLOC_STRUCT (st_softpipe_buffer);

	pipe_reference_init (&buffer->base.reference, 1);
	buffer->base.alignment = alignment;
	buffer->base.usage = usage;
	buffer->base.size = size;

	buffer->data = align_malloc (size, alignment);

	return &buffer->base;
}

/**
 * Create buffer which wraps user-space data.
 */
static struct pipe_buffer *
st_softpipe_user_buffer_create (struct pipe_winsys *winsys,
				void *ptr,
				unsigned bytes)
{
	struct st_softpipe_buffer *buffer;

	buffer = CALLOC_STRUCT (st_softpipe_buffer);
	if (!buffer)
		return NULL;

	pipe_reference_init (&buffer->base.reference, 1);
	buffer->base.size = bytes;
	buffer->userBuffer = TRUE;
	buffer->data = ptr;

	return &buffer->base;
}

static struct pipe_buffer *
st_softpipe_surface_buffer_create (struct pipe_winsys *winsys,
				   unsigned width, unsigned height,
				   enum pipe_format format,
				   unsigned usage,
				   unsigned tex_usage,
				   unsigned *stride)
{
	struct st_winsys *st_ws = (struct st_winsys *) winsys;
	const unsigned alignment = 64;
	unsigned nblocksy;

	nblocksy = pf_get_nblocksy (format, height);

	if (st_ws->user_data && st_ws->user_stride)
	{
		*stride = st_ws->user_stride;
		return winsys->user_buffer_create (winsys,
						   st_ws->user_data,
						   *stride * nblocksy);
	}
	else
	{
		*stride = align (pf_get_stride (format, width), alignment);
		return winsys->buffer_create (winsys, alignment,
					      usage,
					      *stride * nblocksy);
	}
}

static void
st_softpipe_fence_reference (struct pipe_winsys *winsys,
			     struct pipe_fence_handle **ptr,
			     struct pipe_fence_handle *fence)
{
}

static int
st_softpipe_fence_signalled (struct pipe_winsys *winsys,
			     struct pipe_fence_handle *fence,
			     unsigned flag)
{
	return 0;
}

static int
st_softpipe_fence_finish (struct pipe_winsys *winsys,
			  struct pipe_fence_handle *fence,
			  unsigned flag)
{
	return 0;
}

static void
st_softpipe_destroy (struct pipe_winsys *winsys)
{
}

static struct pipe_screen *
st_softpipe_screen_create (struct st_winsys *st_ws)
{
	return softpipe_create_screen (&st_ws->base);
}

static struct pipe_context *
st_softpipe_context_create (struct pipe_screen *screen)
{
	return softpipe_create (screen);
}

const struct st_winsys st_softpipe_winsys = {
	{
		st_softpipe_destroy,
		st_softpipe_get_name,
		NULL,
		st_softpipe_flush_frontbuffer,
		st_softpipe_buffer_create,
		st_softpipe_user_buffer_create,
		st_softpipe_surface_buffer_create,
		st_softpipe_buffer_map,
		st_softpipe_buffer_unmap,
		st_softpipe_buffer_destroy,
		st_softpipe_fence_reference,
		st_softpipe_fence_signalled,
		st_softpipe_fence_finish
	},
	st_softpipe_screen_create,
	st_softpipe_context_create
};

static void
st_texture_destroy_callback (void *data)
{
	struct pipe_texture *texture = (struct pipe_texture *) data;

	pipe_texture_reference(&texture, NULL);
}

static void
st_surface_destroy_callback (void *data)
{
	struct pipe_surface *surface = (struct pipe_surface *) data;

	pipe_surface_reference (&surface, NULL);
}
#endif

void
Effect::Initialize ()
{

#ifdef USE_GALLIUM
	struct st_device *dev;
	dev = st_device_create_from_st_winsys (&st_softpipe_winsys);
	st_context = st_context_create (dev);
	st_device_reference (&dev, NULL);
#endif

}

void
Effect::Shutdown ()
{

#ifdef USE_GALLIUM
	st_context_reference (&st_context, NULL);
#endif

}

Effect::Effect ()
{
	SetObjectType (Type::EFFECT);

	vs = fs = NULL;
	constants = NULL;
}

Effect::~Effect ()
{

#ifdef USE_GALLIUM
	struct st_context *ctx = st_context;

	if (fs)
		ctx->pipe->delete_fs_state (ctx->pipe, fs);
	if (vs)
		ctx->pipe->delete_vs_state (ctx->pipe, vs);

	if (constants)
		pipe_buffer_reference (&constants, NULL);
#endif

}

double
Effect::GetPaddingTop ()
{
	return 0.0;
}

double
Effect::GetPaddingBottom ()
{
	return 0.0;
}

double
Effect::GetPaddingLeft ()
{
	return 0.0;
}

double
Effect::GetPaddingRight ()
{
	return 0.0;
}

struct pipe_texture *
Effect::GetShaderTexture (cairo_surface_t *surface)
{

#ifdef USE_GALLIUM
	struct st_context   *ctx = st_context;
	struct pipe_texture *tex, templat;

	tex = (struct pipe_texture *) cairo_surface_get_user_data (surface, &textureKey);
	if (tex)
		return tex;

	if (cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE)
		return NULL;

	memset (&templat, 0, sizeof (templat));
	templat.format = PIPE_FORMAT_A8R8G8B8_UNORM;
	templat.width0 = cairo_image_surface_get_width (surface);
	templat.height0 = cairo_image_surface_get_height (surface);
	templat.depth0 = 1;
	templat.last_level = 0;
	templat.target = PIPE_TEXTURE_2D;
	templat.tex_usage = PIPE_TEXTURE_USAGE_SAMPLER | PIPE_TEXTURE_USAGE_DISPLAY_TARGET;

	ctx->st_dev->st_ws.user_data   = cairo_image_surface_get_data (surface);
	ctx->st_dev->st_ws.user_stride = cairo_image_surface_get_stride (surface);

	tex = ctx->st_dev->screen->texture_create (ctx->st_dev->screen, &templat);

	ctx->st_dev->st_ws.user_data   = NULL;
	ctx->st_dev->st_ws.user_stride = 0;

	cairo_surface_set_user_data (surface,
				     &textureKey,
				     (void *) tex,
				     st_texture_destroy_callback);

	return tex;
#else
	return NULL;
#endif

}

struct pipe_surface *
Effect::GetShaderSurface (cairo_surface_t *surface)
{
	
#ifdef USE_GALLIUM
	struct st_context   *ctx = st_context;
	struct pipe_surface *sur;
	struct pipe_texture *tex;

	sur = (struct pipe_surface *) cairo_surface_get_user_data (surface, &surfaceKey);
	if (sur)
		return sur;

	if (cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE)
		return NULL;

	tex = GetShaderTexture (surface);
	if (!tex)
		return NULL;

	sur = ctx->st_dev->screen->get_tex_surface (ctx->st_dev->screen, tex, 0, 0, 0,
						    PIPE_BUFFER_USAGE_GPU_WRITE |
						    PIPE_BUFFER_USAGE_GPU_READ);

	cairo_surface_set_user_data (surface,
				     &surfaceKey,
				     (void *) sur,
				     st_surface_destroy_callback);

	return sur;
#else
	return NULL;
#endif

}

struct pipe_buffer *
Effect::GetShaderVertexBuffer (float    x1,
			       float    y1,
			       float    x2,
			       float    y2,
			       unsigned n_attrib,
			       float    **ptr)
{
	
#ifdef USE_GALLIUM
	struct st_context  *ctx = st_context;
	struct pipe_buffer *buffer;
	float              *verts;
	int                stride = (1 + n_attrib) * 4;
	int                idx;

	buffer = pipe_buffer_create (ctx->pipe->screen, 32,
				     PIPE_BUFFER_USAGE_VERTEX,
				     sizeof (float) * stride * 4);
	if (!buffer)
		return NULL;

	verts = (float *) pipe_buffer_map (ctx->pipe->screen,
					   buffer,
					   PIPE_BUFFER_USAGE_CPU_WRITE);
	if (!verts)
	{
		pipe_buffer_reference (&buffer, NULL);
		return NULL;
	}

	idx = 0;
	verts[idx + 0] = x1;
	verts[idx + 1] = y2;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	idx += stride;
	verts[idx + 0] = x1;
	verts[idx + 1] = y1;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	idx += stride;
	verts[idx + 0] = x2;
	verts[idx + 1] = y1;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	idx += stride;
	verts[idx + 0] = x2;
	verts[idx + 1] = y2;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	if (ptr)
		*ptr = verts;
	else
		pipe_buffer_unmap (ctx->pipe->screen, buffer);

	return buffer;
#else
	return NULL;
#endif

}

bool
Effect::Composite (cairo_surface_t *dst,
		   cairo_surface_t *src,
		   int             src_x,
		   int             src_y,
		   int             x,
		   int             y,
		   unsigned int    width,
		   unsigned int    height)
{
	g_warning ("Effect::Compiste has been called. The derived class should have overridden it.");
	return 0;
}

void
Effect::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() == Type::DEPENDENCY_OBJECT) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	need_update = true;
	//
	// If the effect changes, we need to notify our owners
	// that they must repaint (all of our properties have
	// a visible effect.
	//
	NotifyListenersOfPropertyChange (args, error);
}

void
Effect::UpdateShader ()
{
	g_warning ("Effect::UpdateShader has been called. The derived class should have overridden it.");
}

void
Effect::MaybeUpdateShader ()
{
	if (need_update) {
		UpdateShader ();
		need_update = false;
	}
}

BlurEffect::BlurEffect ()
{
	SetObjectType (Type::BLUREFFECT);
}

double
BlurEffect::GetPaddingTop ()
{
	return GetRadius ();
}

double
BlurEffect::GetPaddingBottom ()
{
	return GetRadius ();
}

double
BlurEffect::GetPaddingLeft ()
{
	return GetRadius ();
}

double
BlurEffect::GetPaddingRight ()
{
	return GetRadius ();
}

Rect
BlurEffect::GrowDirtyRectangle (Rect bounds, Rect rect)
{
	return rect.GrowBy (GetRadius ());
}

bool
BlurEffect::Composite (cairo_surface_t *dst,
		       cairo_surface_t *src,
		       int             src_x,
		       int             src_y,
		       int             x,
		       int             y,
		       unsigned int    width,
		       unsigned int    height)
{

#ifdef USE_GALLIUM
	struct st_context   *ctx = st_context;
	struct pipe_texture *texture;
	struct pipe_surface *surface;
	struct pipe_buffer  *vertices;
	float               *verts;
	int                 idx;

	MaybeUpdateShader ();

	if (!fs)
		return 0;

	surface = GetShaderSurface (dst);
	if (!surface)
		return 0;

	texture = GetShaderTexture (src);
	if (!texture)
		return 0;

	if (cso_set_fragment_shader_handle (ctx->cso, fs) != PIPE_OK)
		return 0;

	vertices = GetShaderVertexBuffer ((2.0 / surface->width)  * x - 1.0,
					  (2.0 / surface->height) * y - 1.0,
					  (2.0 / surface->width)  * (x + width)  - 1.0,
					  (2.0 / surface->height) * (y + height) - 1.0,
					  1,
					  &verts);
	if (!vertices)
		return 0;

	double s1 = src_x + 0.5;
	double t1 = src_y + 0.5;
	double s2 = src_x + width  + 0.5;
	double t2 = src_y + height + 0.5;

	idx = 4;
	verts[idx + 0] = s1;
	verts[idx + 1] = t2;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 0.f;

	idx += 8;
	verts[idx + 0] = s1;
	verts[idx + 1] = t1;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	idx += 8;
	verts[idx + 0] = s2;
	verts[idx + 1] = t1;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	idx += 8;
	verts[idx + 0] = s2;
	verts[idx + 1] = t2;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	pipe_buffer_unmap (ctx->pipe->screen, vertices);

	struct pipe_viewport_state viewport;
	memset(&viewport, 0, sizeof(struct pipe_viewport_state));
	viewport.scale[0] =  surface->width / 2.f;
	viewport.scale[1] =  surface->height / 2.f;
	viewport.scale[2] =  1.0;
	viewport.scale[3] =  1.0;
	viewport.translate[0] = surface->width / 2.f;
	viewport.translate[1] = surface->height / 2.f;
	viewport.translate[2] = 0.0;
	viewport.translate[3] = 0.0;
	cso_set_viewport(ctx->cso, &viewport);

	struct pipe_framebuffer_state fb;
	memset(&fb, 0, sizeof(struct pipe_framebuffer_state));
	fb.width = surface->width;
	fb.height = surface->height;
	fb.nr_cbufs = 1;
	fb.cbufs[0] = surface;
	memcpy(&ctx->framebuffer, &fb, sizeof(struct pipe_framebuffer_state));
	cso_set_framebuffer(ctx->cso, &fb);

	struct pipe_sampler_state sampler;
	memset(&sampler, 0, sizeof(struct pipe_sampler_state));
	sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	sampler.min_img_filter = PIPE_TEX_MIPFILTER_NEAREST;
	sampler.mag_img_filter = PIPE_TEX_MIPFILTER_NEAREST;
	sampler.normalized_coords = 0;
	cso_single_sampler(ctx->cso, 0, &sampler);
	cso_single_sampler_done(ctx->cso);

	cso_set_sampler_textures( ctx->cso, 1, &texture );

	struct pipe_constant_buffer kbuf;

	if (constants)
	{
		kbuf.buffer = constants;
		ctx->pipe->set_constant_buffer (ctx->pipe,
						PIPE_SHADER_FRAGMENT,
						0, &kbuf);
	}

	util_draw_vertex_buffer (ctx->pipe, vertices, 0, PIPE_PRIM_QUADS, 4, 2);

	struct pipe_fence_handle *fence = NULL;
	ctx->pipe->flush (ctx->pipe, PIPE_FLUSH_RENDER_CACHE, &fence);
	if (fence)
	{
		/* TODO: allow asynchronous operation */
		ctx->pipe->screen->fence_finish (ctx->pipe->screen, fence, 0);
		ctx->pipe->screen->fence_reference (ctx->pipe->screen, &fence, NULL);
	}

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, NULL);

	cso_set_sampler_textures (ctx->cso, 0, NULL);

	memset (&fb, 0, sizeof (struct pipe_framebuffer_state));
	memcpy (&ctx->framebuffer, &fb, sizeof (struct pipe_framebuffer_state));
	cso_set_framebuffer (ctx->cso, &fb);

	pipe_buffer_reference (&vertices, NULL);

	cso_set_fragment_shader_handle (ctx->cso, ctx->fs);

	return 1;
#else
	return 0;
#endif

}

void
BlurEffect::UpdateShader ()
{

#ifdef USE_GALLIUM
	struct st_context *ctx = st_context;

	double radius = GetRadius ();
	double sigma = radius / 2.0;
	double alpha = radius;
	double scale, xy_scale;
	int    size, half_size;

	if (constants)
		pipe_buffer_reference (&constants, NULL);

	if (fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, fs);
		fs = NULL;
	}

	if (vs) {
		ctx->pipe->delete_vs_state (ctx->pipe, vs);
		vs = NULL;
	}

	scale = 1.0f / (2.0f * M_PI * sigma * sigma);
	half_size = alpha + 0.5f;

	if (half_size == 0)
		half_size = 1;

	size = half_size * 2 + 1;
	xy_scale = 2.0f * radius / size;

	if (size < 3)
		return;

	struct tgsi_token *tokens;

	const char fs_convolution_asm[] =
			"FRAG\n"
			"DCL IN[0], GENERIC[0], PERSPECTIVE\n"
			"DCL OUT[0], COLOR, CONSTANT\n"
			"DCL TEMP[0..4], CONSTANT\n"
			"DCL ADDR[0], CONSTANT\n"
			"DCL CONST[0..%d], CONSTANT\n"
			"DCL SAMP[0], CONSTANT\n"
			"0: MOV TEMP[0], CONST[0].xxxx\n"
			"1: MOV TEMP[1], CONST[0].xxxx\n"
			"2: BGNLOOP :14\n"
			"3: SGE TEMP[0].z, TEMP[0].yyyy, CONST[0].wwww\n"
			"4: IF TEMP[0].zzzz :7\n"
			"5: BRK\n"
			"6: ENDIF\n"
			"7: ARL ADDR[0].x, TEMP[0].yyyy\n"
			"8: MOV TEMP[3], CONST[ADDR[0]+1]\n"
			"9: ADD TEMP[4].xy, IN[0], TEMP[3]\n"
			"10: TEX TEMP[2], TEMP[4], SAMP[0], 2D\n"
			"11: MOV TEMP[3], CONST[ADDR[0]+%d]\n"
			"12: MAD TEMP[1], TEMP[2], TEMP[3], TEMP[1]\n"
			"13: ADD TEMP[0].y, TEMP[0].yyyy, CONST[0].yyyy\n"
			"14: ENDLOOP :2\n"
			"15: MOV OUT[0], TEMP[1]\n"
			"16: END\n";
	gchar *text;

	text = g_strdup_printf (fs_convolution_asm,
				1 + 2 * size * size,
				1 + size * size);

	tokens = (struct tgsi_token *) g_malloc (sizeof (struct tgsi_token) * 1024);

	if (tgsi_text_translate (text, tokens, 1024))
	{
		struct pipe_shader_state state;
		memset (&state, 0, sizeof (struct pipe_shader_state));
		state.tokens = tokens;

		fs = ctx->pipe->create_fs_state (ctx->pipe, &state);
	}

	g_free (tokens);
	g_free (text);

	constants = pipe_buffer_create (ctx->pipe->screen, 16,
					PIPE_BUFFER_USAGE_CONSTANT,
					sizeof (float) * (4 + 4 * 2 * size * size));

	if (constants)
	{
		float *kernel;

		kernel = (float *) pipe_buffer_map (ctx->pipe->screen,
						    constants,
						    PIPE_BUFFER_USAGE_CPU_WRITE);
		if (kernel)
		{
			double sum = 0.0;
			int    idx, i, j;

			kernel[0] = 0.f;
			kernel[1] = 1.f;
			kernel[2] = 0.f;
			kernel[3] = size * size;

			idx = 4;
			for (j = 0; j < size; ++j) {
				for (i = 0; i < size; ++i) {
					int index = j * size + i;

					kernel[idx + index * 4 + 0] = i - (size / 2);
					kernel[idx + index * 4 + 1] = j - (size / 2);
					kernel[idx + index * 4 + 2] = 0.f;
					kernel[idx + index * 4 + 3] = 0.f;
				}
			}

			idx = 4 + 4 * size * size;
			for (j = 0; j < size; ++j) {
				double fy = xy_scale * (j - half_size);

				for (i = 0; i < size; ++i) {
					int    index = j * size + i;
					double fx = xy_scale * (i - half_size);
					double weight;

					weight = scale * exp (-((fx * fx + fy * fy) /
								(2.0f * sigma * sigma)));

					sum += weight;
					kernel[idx + index * 4] = weight;
				}
			}

			if (sum != 0.0) {
				for (j = 0; j < size; ++j) {
					for (i = 0; i < size; ++i) {
						int    index = j * size + i;
						double weight;

						weight = kernel[idx + index * 4] / sum;

						kernel[idx + index * 4 + 0] = weight;
						kernel[idx + index * 4 + 1] = weight;
						kernel[idx + index * 4 + 2] = weight;
						kernel[idx + index * 4 + 3] = weight;
					}
				}
			}

			pipe_buffer_unmap (ctx->pipe->screen, constants);
		}
	}
#endif

}

DropShadowEffect::DropShadowEffect ()
{
	SetObjectType (Type::DROPSHADOWEFFECT);
}

ShaderEffect::ShaderEffect ()
{
	SetObjectType (Type::SHADEREFFECT);
}

bool
ShaderEffect::Composite (cairo_surface_t *dst,
			 cairo_surface_t *src,
			 int             src_x,
			 int             src_y,
			 int             x,
			 int             y,
			 unsigned int    width,
			 unsigned int    height)
{

#ifdef USE_GALLIUM
	struct st_context   *ctx = st_context;
	struct pipe_texture *texture;
	struct pipe_surface *surface;
	struct pipe_buffer  *vertices;
	float               *verts;
	int                 idx;

	MaybeUpdateShader ();

	if (!fs)
		return 0;

	surface = GetShaderSurface (dst);
	if (!surface)
		return 0;

	texture = GetShaderTexture (src);
	if (!texture)
		return 0;

	if (cso_set_fragment_shader_handle (ctx->cso, fs) != PIPE_OK)
		return 0;

	vertices = GetShaderVertexBuffer ((2.0 / surface->width)  * x - 1.0,
					  (2.0 / surface->height) * y - 1.0,
					  (2.0 / surface->width)  * (x + width)  - 1.0,
					  (2.0 / surface->height) * (y + height) - 1.0,
					  1,
					  &verts);
	if (!vertices)
		return 0;

	double s1 = src_x + 0.5;
	double t1 = src_y + 0.5;
	double s2 = src_x + width  + 0.5;
	double t2 = src_y + height + 0.5;

	idx = 4;
	verts[idx + 0] = s1;
	verts[idx + 1] = t2;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 0.f;

	idx += 8;
	verts[idx + 0] = s1;
	verts[idx + 1] = t1;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	idx += 8;
	verts[idx + 0] = s2;
	verts[idx + 1] = t1;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	idx += 8;
	verts[idx + 0] = s2;
	verts[idx + 1] = t2;
	verts[idx + 2] = 0.f;
	verts[idx + 3] = 1.f;

	pipe_buffer_unmap (ctx->pipe->screen, vertices);

	struct pipe_viewport_state viewport;
	memset(&viewport, 0, sizeof(struct pipe_viewport_state));
	viewport.scale[0] =  surface->width / 2.f;
	viewport.scale[1] =  surface->height / 2.f;
	viewport.scale[2] =  1.0;
	viewport.scale[3] =  1.0;
	viewport.translate[0] = surface->width / 2.f;
	viewport.translate[1] = surface->height / 2.f;
	viewport.translate[2] = 0.0;
	viewport.translate[3] = 0.0;
	cso_set_viewport(ctx->cso, &viewport);

	struct pipe_framebuffer_state fb;
	memset(&fb, 0, sizeof(struct pipe_framebuffer_state));
	fb.width = surface->width;
	fb.height = surface->height;
	fb.nr_cbufs = 1;
	fb.cbufs[0] = surface;
	memcpy(&ctx->framebuffer, &fb, sizeof(struct pipe_framebuffer_state));
	cso_set_framebuffer(ctx->cso, &fb);

	struct pipe_sampler_state sampler;
	memset(&sampler, 0, sizeof(struct pipe_sampler_state));
	sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	sampler.min_img_filter = PIPE_TEX_MIPFILTER_NEAREST;
	sampler.mag_img_filter = PIPE_TEX_MIPFILTER_NEAREST;
	sampler.normalized_coords = 0;
	cso_single_sampler(ctx->cso, 0, &sampler);
	cso_single_sampler_done(ctx->cso);

	cso_set_sampler_textures( ctx->cso, 1, &texture );

	struct pipe_constant_buffer kbuf;

	if (constants)
	{
		kbuf.buffer = constants;
		ctx->pipe->set_constant_buffer (ctx->pipe,
						PIPE_SHADER_FRAGMENT,
						0, &kbuf);
	}

	util_draw_vertex_buffer (ctx->pipe, vertices, 0, PIPE_PRIM_QUADS, 4, 2);

	struct pipe_fence_handle *fence = NULL;
	ctx->pipe->flush (ctx->pipe, PIPE_FLUSH_RENDER_CACHE, &fence);
	if (fence)
	{
		/* TODO: allow asynchronous operation */
		ctx->pipe->screen->fence_finish (ctx->pipe->screen, fence, 0);
		ctx->pipe->screen->fence_reference (ctx->pipe->screen, &fence, NULL);
	}

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, NULL);

	cso_set_sampler_textures (ctx->cso, 0, NULL);

	memset (&fb, 0, sizeof (struct pipe_framebuffer_state));
	memcpy (&ctx->framebuffer, &fb, sizeof (struct pipe_framebuffer_state));
	cso_set_framebuffer (ctx->cso, &fb);

	pipe_buffer_reference (&vertices, NULL);

	cso_set_fragment_shader_handle (ctx->cso, ctx->fs);

	return 1;
#else
	return 0;
#endif

}

typedef enum _shader_instruction_opcode_type {
	D3DSIO_NOP = 0,
	D3DSIO_MOV = 1,
	D3DSIO_ADD = 2,
	D3DSIO_SUB = 3,
	D3DSIO_MAD = 4,
	D3DSIO_MUL = 5,
	D3DSIO_RCP = 6,
	D3DSIO_RSQ = 7,
	D3DSIO_DP3 = 8,
	D3DSIO_DP4 = 9,
	D3DSIO_MIN = 10,
	D3DSIO_MAX = 11,
	D3DSIO_SLT = 12,
	D3DSIO_SGE = 13,
	D3DSIO_EXP = 14,
	D3DSIO_LOG = 15,
	D3DSIO_LIT = 16,
	D3DSIO_DST = 17,
	D3DSIO_LRP = 18,
	D3DSIO_FRC = 19,
	D3DSIO_M4x4 = 20,
	D3DSIO_M4x3 = 21,
	D3DSIO_M3x4 = 22,
	D3DSIO_M3x3 = 23,
	D3DSIO_M3x2 = 24,
	D3DSIO_CALL = 25,
	D3DSIO_CALLNZ = 26,
	D3DSIO_LOOP = 27,
	D3DSIO_RET = 28,
	D3DSIO_ENDLOOP = 29,
	D3DSIO_LABEL = 30,
	D3DSIO_DCL = 31,
	D3DSIO_POW = 32,
	D3DSIO_CRS = 33,
	D3DSIO_SGN = 34,
	D3DSIO_ABS = 35,
	D3DSIO_NRM = 36,
	D3DSIO_SINCOS = 37,
	D3DSIO_REP = 38,
	D3DSIO_ENDREP = 39,
	D3DSIO_IF = 40,
	D3DSIO_IFC = 41,
	D3DSIO_ELSE = 42,
	D3DSIO_ENDIF = 43,
	D3DSIO_BREAK = 44,
	D3DSIO_BREAKC = 45,
	D3DSIO_MOVA = 46,
	D3DSIO_DEFB = 47,
	D3DSIO_DEFI = 48,
	D3DSIO_TEXCOORD = 64,
	D3DSIO_TEXKILL = 65,
	D3DSIO_TEX = 66,
	D3DSIO_TEXBEM = 67,
	D3DSIO_TEXBEML = 68,
	D3DSIO_TEXREG2AR = 69,
	D3DSIO_TEXREG2GB = 70,
	D3DSIO_TEXM3x2PAD = 71,
	D3DSIO_TEXM3x2TEX = 72,
	D3DSIO_TEXM3x3PAD = 73,
	D3DSIO_TEXM3x3TEX = 74,
	D3DSIO_RESERVED0 = 75,
	D3DSIO_TEXM3x3SPEC = 76,
	D3DSIO_TEXM3x3VSPEC = 77,
	D3DSIO_EXPP = 78,
	D3DSIO_LOGP = 79,
	D3DSIO_CND = 80,
	D3DSIO_DEF = 81,
	D3DSIO_TEXREG2RGB = 82,
	D3DSIO_TEXDP3TEX = 83,
	D3DSIO_TEXM3x2DEPTH = 84,
	D3DSIO_TEXDP3 = 85,
	D3DSIO_TEXM3x3 = 86,
	D3DSIO_TEXDEPTH = 87,
	D3DSIO_CMP = 88,
	D3DSIO_BEM = 89,
	D3DSIO_DP2ADD = 90,
	D3DSIO_DSX = 91,
	D3DSIO_DSY = 92,
	D3DSIO_TEXLDD = 93,
	D3DSIO_SETP = 94,
	D3DSIO_TEXLDL = 95,
	D3DSIO_BREAKP = 96,
	D3DSIO_PHASE = 0xfffd,
	D3DSIO_COMMENT = 0xfffe,
	D3DSIO_END = 0xffff
} shader_instruction_opcode_type_t;

typedef enum _shader_param_register_type {
	D3DSPR_TEMP = 0,
	D3DSPR_INPUT = 1,
	D3DSPR_CONST = 2,
	D3DSPR_TEXTURE = 3,
	D3DSPR_RASTOUT = 4,
	D3DSPR_ATTROUT = 5,
	D3DSPR_OUTPUT = 6,
	D3DSPR_CONSTINT = 7,
	D3DSPR_COLOROUT = 8,
	D3DSPR_DEPTHOUT = 9,
	D3DSPR_SAMPLER = 10,
	D3DSPR_CONST2 = 11,
	D3DSPR_CONST3 = 12,
	D3DSPR_CONST4 = 13,
	D3DSPR_CONSTBOOL = 14,
	D3DSPR_LOOP = 15,
	D3DSPR_TEMPFLOAT16 = 16,
	D3DSPR_MISCTYPE = 17,
	D3DSPR_LABEL = 18,
	D3DSPR_PREDICATE = 19,
	D3DSPR_LAST = 20
} shader_param_register_type_t;

typedef enum _shader_param_dstmod_type {
	D3DSPD_NONE = 0,
	D3DSPD_SATURATE = 1,
	D3DSPD_PARTIAL_PRECISION = 2,
	D3DSPD_CENTRIOD = 4,
} shader_param_dstmod_type_t;

typedef enum _shader_param_srcmod_type {
	D3DSPS_NONE = 0,
	D3DSPS_NEGATE = 1,
	D3DSPS_BIAS = 2,
	D3DSPS_NEGATE_BIAS = 3,
	D3DSPS_SIGN = 4,
	D3DSPS_NEGATE_SIGN = 5,
	D3DSPS_COMP = 6,
	D3DSPS_X2 = 7,
	D3DSPS_NEGATE_X2 = 8,
	D3DSPS_DZ = 9,
	D3DSPS_DW = 10,
	D3DSPS_ABS = 11,
	D3DSPS_NEGATE_ABS = 12,
	D3DSPS_NOT = 13,
	D3DSPS_LAST = 14
} shader_param_srcmod_type_t;

#define REGNUM_MAX 32

#ifdef USE_GALLIUM
static INLINE struct ureg_dst
ureg_d3d_dstmod (struct ureg_dst dst,
		 unsigned int    mod)
{
	switch (mod) {
		case D3DSPD_SATURATE:
			return ureg_saturate (dst);
		default:
			return dst;
	}
}

static INLINE struct ureg_dst
ureg_d3d_dst (struct ureg_dst             map[][REGNUM_MAX],
	      d3d_destination_parameter_t *dst)
{
	return ureg_writemask (ureg_d3d_dstmod (map[dst->regtype][dst->regnum],
						dst->dstmod),
			       dst->writemask);
}

static INLINE struct ureg_src
ureg_d3d_srcmod (struct ureg_src src,
		 unsigned int    mod)
{
	switch (mod) {
		case D3DSPS_NEGATE:
			return ureg_negate (src);
		case D3DSPS_ABS:
			return ureg_abs (src);
		default:
			return src;
	}
}

static INLINE struct ureg_src
ureg_d3d_src (struct ureg_src        map[][REGNUM_MAX],
	      d3d_source_parameter_t *src)
{
	return ureg_swizzle (ureg_d3d_srcmod (map[src->regtype][src->regnum],
					      src->srcmod),
			     src->swizzle.x,
			     src->swizzle.y,
			     src->swizzle.z,
			     src->swizzle.w);
}
#endif

void
ShaderEffect::UpdateShader ()
{

#ifdef USE_GALLIUM
	PixelShader         *ps = GetPixelShader ();
	struct st_context   *ctx = st_context;
	struct ureg_program *ureg;
	d3d_version_t       version;
	d3d_op_t            op;
	int                 index;
	struct ureg_src     src_reg[D3DSPR_LAST][REGNUM_MAX];
	struct ureg_dst     dst_reg[D3DSPR_LAST][REGNUM_MAX];

	DumpShader ();

	if (fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, fs);
		fs = NULL;
	}

	if (!ps)
		return;

	if ((index = ps->GetVersion (0, &version)) < 0)
		return;

	if (version.type  != 0xffff ||
	    version.major != 2      ||
	    version.minor != 0) {
		g_warning ("Unsupported Pixel Shader");
		return;
	}

	ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);
	if (!ureg)
		return;

	for (int i = 0; i < D3DSPR_LAST; i++) {
		for (int j = 0; j < REGNUM_MAX; j++) {
			src_reg[i][j] = ureg_src_undef ();
			dst_reg[i][j] = ureg_dst_undef ();
		}
	}

	dst_reg[D3DSPR_COLOROUT][0] = ureg_DECL_output (ureg,
							TGSI_SEMANTIC_COLOR,
							0);

	/* validation and register allocation */
	for (int i = ps->GetOp (index, &op); i > 0; i = ps->GetOp (i, &op)) {
		if (op.type == D3DSIO_END)
			break;

		switch (op.type) {
			case D3DSIO_COMMENT:
				i += op.comment_length;
				break;
				// case D3DSIO_DEFB:
				// case D3DSIO_DEFI:
			case D3DSIO_DEF: {
				d3d_def_instruction_t def;

				i = ps->GetInstruction (i, &def);

				assert (def.reg.writemask == 0xf);
				assert (def.reg.dstmod == 0);

				src_reg[def.reg.regtype][def.reg.regnum] =
					ureg_DECL_immediate (ureg, def.v, 4);
			} break;
			case D3DSIO_DCL: {
				d3d_dcl_instruction_t dcl;

				i = ps->GetInstruction (i, &dcl);

				assert (dcl.reg.dstmod == 0);
				assert (dcl.reg.regnum < REGNUM_MAX);

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
									    TGSI_INTERPOLATE_PERSPECTIVE);
						break;
					default:
						g_warning ("Invalid Register Type");
						ureg_destroy (ureg);
						return;
				}
			} break;
			case D3DSIO_NOP:
				// case D3DSIO_BREAK:
				// case D3DSIO_BREAKC:
				// case D3DSIO_BREAKP:
				// case D3DSIO_CALL:
				// case D3DSIO_CALLNZ:
				// case D3DSIO_LOOP:
				// case D3DSIO_RET:
				// case D3DSIO_ENDLOOP:
				// case D3DSIO_LABEL:
				// case D3DSIO_REP:
				// case D3DSIO_ENDREP:
				// case D3DSIO_IF:
				// case D3DSIO_IFC:
				// case D3DSIO_ELSE:
				// case D3DSIO_ENDIF:
				break;
			case D3DSIO_MOV:
			case D3DSIO_ADD:
			case D3DSIO_SUB:
			case D3DSIO_MAD:
			case D3DSIO_MUL:
			case D3DSIO_RCP:
			case D3DSIO_RSQ:
			case D3DSIO_DP3:
			case D3DSIO_DP4:
			case D3DSIO_MIN:
			case D3DSIO_MAX:
			case D3DSIO_SLT:
			case D3DSIO_SGE:
			case D3DSIO_EXP:
			case D3DSIO_LOG:
			case D3DSIO_LIT:
			case D3DSIO_DST:
			case D3DSIO_LRP:
			case D3DSIO_FRC:
				// case D3DSIO_M4x4:
				// case D3DSIO_M4x3:
				// case D3DSIO_M3x4:
				// case D3DSIO_M3x3:
				// case D3DSIO_M3x2:
			case D3DSIO_POW:
				// case D3DSIO_CRS:
				// case D3DSIO_SGN:
			case D3DSIO_ABS:
			case D3DSIO_NRM:
			case D3DSIO_SINCOS:
				// case D3DSIO_MOVA:
				// case D3DSIO_TEXCOORD:
				// case D3DSIO_TEXKILL:
			case D3DSIO_TEX:
				// case D3DSIO_TEXBEM:
				// case D3DSIO_TEXBEML:
				// case D3DSIO_TEXREG2AR:
				// case D3DSIO_TEXREG2GB:
				// case D3DSIO_TEXM3x2PAD:
				// case D3DSIO_TEXM3x2TEX:
				// case D3DSIO_TEXM3x3PAD:
				// case D3DSIO_TEXM3x3TEX:
				// case D3DSIO_RESERVED0:
				// case D3DSIO_TEXM3x3SPEC:
				// case D3DSIO_TEXM3x3VSPEC:
				// case D3DSIO_EXPP:
				// case D3DSIO_LOGP:
			case D3DSIO_CND:
				// case D3DSIO_TEXREG2RGB:
				// case D3DSIO_TEXDP3TEX:
				// case D3DSIO_TEXM3x2DEPTH:
				// case D3DSIO_TEXDP3:
				// case D3DSIO_TEXM3x3:
				// case D3DSIO_TEXDEPTH:
			case D3DSIO_CMP:
				// case D3DSIO_BEM:
				// case D3DSIO_DP2ADD:
				// case D3DSIO_DSX:
				// case D3DSIO_DSY:
				// case D3DSIO_TEXLDD:
				// case D3DSIO_SETP:
				// case D3DSIO_TEXLDL:
			{
				d3d_destination_parameter_t reg;

				ps->GetDestinationParameter (i, &reg);

				assert (reg.dstmod == 0);
				assert (reg.regnum < REGNUM_MAX);

				if (reg.regtype == D3DSPR_TEMP) {
					if (ureg_dst_is_undef (dst_reg[D3DSPR_TEMP][reg.regnum])) {
						struct ureg_dst tmp = ureg_DECL_temporary (ureg);

						dst_reg[D3DSPR_TEMP][reg.regnum] = tmp;
						src_reg[D3DSPR_TEMP][reg.regnum] = ureg_src (tmp);
					}
				}
				i += op.length;
			} break;
			default:
				g_warning ("Unknown Instruction: %d", op.type);
				ureg_destroy (ureg);
				return;
		}
	}

	for (int i = ps->GetOp (index, &op); i > 0; i = ps->GetOp (i, &op)) {
		d3d_destination_parameter_t reg;
		d3d_source_parameter_t      src1, src2, src3;

		switch (op.type) {
			case D3DSIO_COMMENT:
				i += op.comment_length;
				break;
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
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_MOV (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_ADD:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_ADD (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_SUB:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_SUB (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_MAD:
				i = ps->GetInstruction (i, &reg, &src1, &src2, &src3);
				ureg_MAD (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2),
					  ureg_d3d_src (src_reg, &src3));
				break;
			case D3DSIO_MUL:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_MUL (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_RCP:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_RCP (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_RSQ:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_RSQ (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_DP3:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_DP3 (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_DP4:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_DP4 (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_MIN:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_MIN (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_MAX:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_MAX (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_SLT:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_SLT (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_SGE:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_SGE (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_EXP:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_EXP (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_LOG:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_LOG (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_LIT:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_LIT (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_DST:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_DST (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
			case D3DSIO_LRP:
				i = ps->GetInstruction (i, &reg, &src1, &src2, &src3);
				ureg_LRP (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2),
					  ureg_d3d_src (src_reg, &src3));
				break;
			case D3DSIO_FRC:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_FRC (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
				// case D3DSIO_M4x4: break;
				// case D3DSIO_M4x3: break;
				// case D3DSIO_M3x4: break;
				// case D3DSIO_M3x3: break;
				// case D3DSIO_M3x2: break;
			case D3DSIO_POW:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_POW (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
				break;
				// case D3DSIO_CRS: break;
				// case D3DSIO_SGN: break;
			case D3DSIO_ABS:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_ABS (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_NRM:
				i = ps->GetInstruction (i, &reg, &src1);
				ureg_NRM (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1));
				break;
			case D3DSIO_SINCOS:
				i = ps->GetInstruction (i, &reg, &src1, &src2, &src3);
				if (reg.writemask & 0x1) {
					d3d_destination_parameter_t r = reg;

					r.writemask = 0x1;
					ureg_COS (ureg,
						  ureg_d3d_dst (dst_reg, &r),
						  ureg_d3d_src (src_reg, &src1));
				}
				if (reg.writemask & 0x2) {
					d3d_destination_parameter_t r = reg;

					r.writemask = 0x2;
					ureg_SIN (ureg,
						  ureg_d3d_dst (dst_reg, &r),
						  ureg_d3d_src (src_reg, &src1));
				}
				break;
				// case D3DSIO_MOVA: break;
				// case D3DSIO_TEXCOORD: break;
				// case D3DSIO_TEXKILL: break;
			case D3DSIO_TEX:
				i = ps->GetInstruction (i, &reg, &src1, &src2);
				ureg_TEX (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  TGSI_TEXTURE_2D,
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2));
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
				i = ps->GetInstruction (i, &reg, &src1, &src2, &src3);
				ureg_CND (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2),
					  ureg_d3d_src (src_reg, &src3));
				break;
				// case D3DSIO_TEXREG2RGB: break;
				// case D3DSIO_TEXDP3TEX: break;
				// case D3DSIO_TEXM3x2DEPTH: break;
				// case D3DSIO_TEXDP3: break;
				// case D3DSIO_TEXM3x3: break;
				// case D3DSIO_TEXDEPTH: break;
			case D3DSIO_CMP:
				i = ps->GetInstruction (i, &reg, &src1, &src2, &src3);
				ureg_CMP (ureg,
					  ureg_d3d_dst (dst_reg, &reg),
					  ureg_d3d_src (src_reg, &src1),
					  ureg_d3d_src (src_reg, &src2),
					  ureg_d3d_src (src_reg, &src3));
				break;
				// case D3DSIO_BEM: break;
				// case D3DSIO_DP2ADD: break;
				// case D3DSIO_DSX: break;
				// case D3DSIO_DSY: break;
				// case D3DSIO_TEXLDD: break;
				// case D3DSIO_SETP: break;
				// case D3DSIO_TEXLDL: break;
			case D3DSIO_END:
				ureg_END (ureg);
				fs = ureg_create_shader_and_destroy (ureg, ctx->pipe);
				if (!fs)
					g_warning ("Pixel Shader Construction "
						   "Failed");
				return;
			default:
				i += op.length;
				break;
		}
	}

	g_warning ("Incomplete Pixel Shader");
#endif

}

static INLINE void d3d_print_regtype (unsigned int type)
{
	const char *type_str[] = {
		" TEMP",
		"INPUT",
		"CONST",
		"  TEX",
		" ROUT",
		" AOUT",
		"  OUT",
		"CTINT",
		" COUT",
		" DOUT",
		" SAMP",
		"CONS2",
		"CONS3",
		"CONS4",
		"CBOOL",
		" LOOP",
		" TF16",
		" MISC",
		"LABEL",
		" PRED"
	};

	if (type >= G_N_ELEMENTS (type_str))
		printf ("0x%.3x", type);

	printf ("%s", type_str[type]);
}

static INLINE void
d3d_print_srcmod (unsigned int mod)
{
	const char *srcmod_str[] = {
		"     ",
		"  neg",
		" bias",
		"nbias",
		" sign",
		"nsign",
		" comp",
		"  pow",
		" npow",
		"   dz",
		"   dw",
		"  abs",
		" nabs",
		"  not"
	};

	if (mod >= G_N_ELEMENTS (srcmod_str))
		printf ("0x%.3x", (int) mod);

	printf ("%s", srcmod_str[mod]);
}

static INLINE void
d3d_print_src_param (d3d_source_parameter_t *src)
{
	const char *swizzle_str[] = { "x", "y", "z", "w" };

	d3d_print_srcmod (src->srcmod);
	printf ("( ");
	d3d_print_regtype (src->regtype);
	printf (":%.2u ).%s%s%s%s ",
		src->regnum,
		swizzle_str[src->swizzle.x],
		swizzle_str[src->swizzle.y],
		swizzle_str[src->swizzle.z],
		swizzle_str[src->swizzle.w]);
}

static INLINE void
d3d_print_dstmod (unsigned int mod)
{
	const char *dstmod_str[] = {
		"   ",
		"sat",
		"prt",
		"cnt"
	};

	if (mod >= G_N_ELEMENTS (dstmod_str))
		printf ("0x%.1x", mod);

	printf ("%s", dstmod_str[mod]);
}

static INLINE void
d3d_print_dst_param (d3d_destination_parameter_t *dst)
{
	d3d_print_dstmod (dst->dstmod);
	printf ("( ");
	d3d_print_regtype (dst->regtype);
	printf (":%.2u ).%s%s%s%s%s%s%s%s ",
		dst->regnum,
		dst->writemask & 0x1 ? "x" : "",
		dst->writemask & 0x2 ? "y" : "",
		dst->writemask & 0x4 ? "z" : "",
		dst->writemask & 0x8 ? "w" : "",
		dst->writemask & 0x1 ? "" : " ",
		dst->writemask & 0x2 ? "" : " ",
		dst->writemask & 0x4 ? "" : " ",
		dst->writemask & 0x8 ? "" : " ");
}

void
ShaderEffect::DumpShader ()
{
	PixelShader   *ps = GetPixelShader ();
	d3d_version_t version;
	d3d_op_t      op;
	int           i;

	if (!ps)
		return;

	if ((i = ps->GetVersion (0, &version)) < 0)
		return;

	if (version.type != 0xffff) {
		printf ("0x%x %d.%d\n", version.type, version.major,
			version.minor);
		return;
	}
	else if (version.major < 2) {
		printf ("PS %d.%d\n", version.major, version.minor);
		return;
	}

	printf ("PS %d.%d\n", version.major, version.minor);

	while ((i = ps->GetOp (i, &op)) > 0) {
		d3d_destination_parameter_t reg;
		d3d_source_parameter_t      src1, src2, src3;
		const char                  *op1_str = NULL;
		const char                  *op2_str = NULL;
		const char                  *op3_str = NULL;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		switch (op.type) {
			case D3DSIO_DEF: {
				d3d_def_instruction_t def;

				if (ps->GetInstruction (i, &def) != 1) {
					printf ("DEF ");
					d3d_print_dst_param (&def.reg);
					printf ("     ( %f %f %f %f )\n",
						def.v[0], def.v[1], def.v[2],
						def.v[3]);
				}
			} break;
			case D3DSIO_DCL: {
				d3d_dcl_instruction_t dcl;

				if (ps->GetInstruction (i, &dcl) != -1) {
					printf ("DCL ");
					d3d_print_dst_param (&dcl.reg);
					printf ("     ( 0x%x 0x%x )\n",
						dcl.usage, dcl.usageindex);
				}
			} break;
			case D3DSIO_NOP: printf ("NOP\n"); break;
			case D3DSIO_MOV: op1_str = "MOV"; break;
			case D3DSIO_ADD: op2_str = "ADD"; break;
			case D3DSIO_SUB: op2_str = "SUB"; break;
			case D3DSIO_MAD: op3_str = "MAD"; break;
			case D3DSIO_MUL: op2_str = "MUL"; break;
			case D3DSIO_RCP: op1_str = "RCP"; break;
			case D3DSIO_RSQ: op1_str = "RSQ"; break;
			case D3DSIO_DP3: op2_str = "DP3"; break;
			case D3DSIO_DP4: op2_str = "DP4"; break;
			case D3DSIO_MIN: op2_str = "MIN"; break;
			case D3DSIO_MAX: op2_str = "MAX"; break;
			case D3DSIO_SLT: op2_str = "SLT"; break;
			case D3DSIO_SGE: op2_str = "SGE"; break;
			case D3DSIO_EXP: op1_str = "EXP"; break;
			case D3DSIO_LOG: op2_str = "LOG"; break;
			case D3DSIO_LIT: op1_str = "LIT"; break;
			case D3DSIO_DST: op2_str = "DST"; break;
			case D3DSIO_LRP: op3_str = "LRP"; break;
			case D3DSIO_FRC: op1_str = "FRC"; break;
			case D3DSIO_POW: op2_str = "POW"; break;
			case D3DSIO_ABS: op1_str = "ABS"; break;
			case D3DSIO_NRM: op1_str = "NRM"; break;
			case D3DSIO_SINCOS: op3_str = "SIN"; break;
			case D3DSIO_TEX: op2_str = "TEX"; break;
			case D3DSIO_CND: op3_str = "CND"; break;
			case D3DSIO_CMP: op3_str = "CMP"; break;
			case D3DSIO_END:
				printf ("END\n");
				return;
			default:
				printf ("%d\n", op.type);
		}

		if (op1_str) {
			if (ps->GetInstruction (i, &reg, &src1) != -1) {
				printf ("%s ", op1_str);
				d3d_print_dst_param (&reg);
				d3d_print_src_param (&src1);
				printf ("\n");
			}
		}
		else if (op2_str) {
			if (ps->GetInstruction (i, &reg, &src1, &src2) != -1) {
				printf ("%s ", op2_str);
				d3d_print_dst_param (&reg);
				d3d_print_src_param (&src1);
				d3d_print_src_param (&src2);
				printf ("\n");
			}
		}
		else if (op3_str) {
			if (ps->GetInstruction (i, &reg,
						&src1, &src2, &src3) != -1) {
				printf ("%s ", op3_str);
				d3d_print_dst_param (&reg);
				d3d_print_src_param (&src1);
				d3d_print_src_param (&src2);
				d3d_print_src_param (&src3);
				printf ("\n");
			}
		}

		i += op.length;
	}
}

PixelShader::PixelShader ()
{
	SetObjectType (Type::PIXELSHADER);

	tokens = NULL;
}

PixelShader::~PixelShader ()
{
	g_free (tokens);
}

void
PixelShader::OnPropertyChanged (PropertyChangedEventArgs *args,
				MoonError                *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::PIXELSHADER) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == PixelShader::UriSourceProperty) {
		Application *application = Application::GetCurrent ();
		Uri *uri = GetUriSource ();
		char *path;

		g_free (tokens);
		tokens = NULL;

		if (!Uri::IsNullOrEmpty (uri) &&
		    (path = application->GetResourceAsPath (GetResourceBase (),
							    uri))) {
			GError *error = NULL;
			gsize  nbytes;

			if (!g_file_get_contents (path,
						  (char **) &tokens,
						  &nbytes,
						  &error)) {
				g_warning ("%s", error->message);
				g_error_free (error);
			}

			ntokens = nbytes / sizeof (unsigned long);
			g_free (path);
		}
		else {
			g_warning ("invalid uri: %s", uri->ToString ());
		}
	}

	NotifyListenersOfPropertyChange (args, error);
}

int
PixelShader::GetToken (int           index,
		       unsigned long *token)
{
	if (!tokens || index < 0 || index >= (int) ntokens) {
		if (index >= 0)
			g_warning ("incomplete pixel shader");

		return -1;
	}

	if (token)
		*token = *(tokens + index);

	return index + 1;
}

int
PixelShader::GetToken (int   index,
		       float *token)
{
	return GetToken (index, (unsigned long *) token);
}

/* major version */
#define D3D_VERSION_MAJOR_SHIFT 8
#define D3D_VERSION_MAJOR_MASK  0xff

/* minor version */
#define D3D_VERSION_MINOR_SHIFT 0
#define D3D_VERSION_MINOR_MASK  0xff

/* shader type */
#define D3D_VERSION_TYPE_SHIFT 16
#define D3D_VERSION_TYPE_MASK  0xffff

int
PixelShader::GetVersion (int	       index,
			 d3d_version_t *value)
{
	unsigned long token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	value->major = (token >> D3D_VERSION_MAJOR_SHIFT) &
		D3D_VERSION_MAJOR_MASK;
	value->minor = (token >> D3D_VERSION_MINOR_SHIFT) &
		D3D_VERSION_MINOR_MASK;
	value->type  = (token >> D3D_VERSION_TYPE_SHIFT) &
		D3D_VERSION_TYPE_MASK;

	return index;
}

/* instruction type */
#define D3D_OP_TYPE_SHIFT 0
#define D3D_OP_TYPE_MASK  0xffff

/* instruction length */
#define D3D_OP_LENGTH_SHIFT 24
#define D3D_OP_LENGTH_MASK  0xf

/* comment length */
#define D3D_OP_COMMENT_LENGTH_SHIFT 16
#define D3D_OP_COMMENT_LENGTH_MASK  0xffff

int
PixelShader::GetOp (int      index,
		    d3d_op_t *value)
{
	unsigned long token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	value->type = (token >> D3D_OP_TYPE_SHIFT) & D3D_OP_TYPE_MASK;
	value->length = (token >> D3D_OP_LENGTH_SHIFT) & D3D_OP_LENGTH_MASK;
	value->comment_length = (token >> D3D_OP_COMMENT_LENGTH_SHIFT) &
		D3D_OP_COMMENT_LENGTH_MASK;

	return index;
}

/* register number */
#define D3D_DP_REGNUM_MASK 0x7ff

/* register type */
#define D3D_DP_REGTYPE_SHIFT1 28
#define D3D_DP_REGTYPE_MASK1  0x7
#define D3D_DP_REGTYPE_SHIFT2 8
#define D3D_DP_REGTYPE_MASK2  0x18

/* write mask */
#define D3D_DP_WRITEMASK_SHIFT 16
#define D3D_DP_WRITEMASK_MASK  0xf

/* destination modifier */
#define D3D_DP_DSTMOD_SHIFT 20
#define D3D_DP_DSTMOD_MASK  0x7

int
PixelShader::GetDestinationParameter (int                         index,
				      d3d_destination_parameter_t *value)
{
	unsigned long token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	if (!value)
		return index;

	value->regnum = token & D3D_DP_REGNUM_MASK;
	value->regtype =
		((token >> D3D_DP_REGTYPE_SHIFT1) & D3D_DP_REGTYPE_MASK1) |
		((token >> D3D_DP_REGTYPE_SHIFT2) & D3D_DP_REGTYPE_MASK2);
	value->writemask = (token >> D3D_DP_WRITEMASK_SHIFT) &
		D3D_DP_WRITEMASK_MASK;
	value->dstmod = (token >> D3D_DP_DSTMOD_SHIFT) & D3D_DP_DSTMOD_MASK;

	return index;
}

/* register number */
#define D3D_SP_REGNUM_MASK 0x7ff

/* register type */
#define D3D_SP_REGTYPE_SHIFT1 28
#define D3D_SP_REGTYPE_MASK1  0x7
#define D3D_SP_REGTYPE_SHIFT2 8
#define D3D_SP_REGTYPE_MASK2  0x18

/* swizzle */
#define D3D_SP_SWIZZLE_X_SHIFT 16
#define D3D_SP_SWIZZLE_X_MASK  0x3
#define D3D_SP_SWIZZLE_Y_SHIFT 18
#define D3D_SP_SWIZZLE_Y_MASK  0x3
#define D3D_SP_SWIZZLE_Z_SHIFT 20
#define D3D_SP_SWIZZLE_Z_MASK  0x3
#define D3D_SP_SWIZZLE_W_SHIFT 22
#define D3D_SP_SWIZZLE_W_MASK  0x3

/* source modifier */
#define D3D_SP_SRCMOD_SHIFT 24
#define D3D_SP_SRCMOD_MASK  0x7

int
PixelShader::GetSourceParameter (int                    index,
				 d3d_source_parameter_t *value)
{
	unsigned long token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	if (!value)
		return index;

	value->regnum = token & D3D_SP_REGNUM_MASK;
	value->regtype =
		((token >> D3D_SP_REGTYPE_SHIFT1) & D3D_SP_REGTYPE_MASK1) |
		((token >> D3D_SP_REGTYPE_SHIFT2) & D3D_SP_REGTYPE_MASK2);
	value->swizzle.x = (token >> D3D_SP_SWIZZLE_X_SHIFT) &
		D3D_SP_SWIZZLE_X_MASK;
	value->swizzle.y = (token >> D3D_SP_SWIZZLE_Y_SHIFT) &
		D3D_SP_SWIZZLE_Y_MASK;
	value->swizzle.z = (token >> D3D_SP_SWIZZLE_Z_SHIFT) &
		D3D_SP_SWIZZLE_Z_MASK;
	value->swizzle.w = (token >> D3D_SP_SWIZZLE_W_SHIFT) &
		D3D_SP_SWIZZLE_W_MASK;
	value->srcmod = (token >> D3D_SP_SRCMOD_SHIFT) & D3D_SP_SRCMOD_MASK;

	return index;
}

int
PixelShader::GetInstruction (int                   index,
			     d3d_def_instruction_t *value)
{
	index = GetDestinationParameter (index, &value->reg);
	index = GetToken (index, &value->v[0]);
	index = GetToken (index, &value->v[1]);
	index = GetToken (index, &value->v[2]);
	index = GetToken (index, &value->v[3]);

	return index;
}

/* DCL usage */
#define D3D_DCL_USAGE_SHIFT 0
#define D3D_DCL_USAGE_MASK  0xf

/* DCL usage index */
#define D3D_DCL_USAGEINDEX_SHIFT 16
#define D3D_DCL_USAGEINDEX_MASK  0xf

int
PixelShader::GetInstruction (int                   index,
			     d3d_dcl_instruction_t *value)
{
	unsigned long token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	index = GetDestinationParameter (index, &value->reg);

	if (!value)
		return index;

	value->usage = (token >> D3D_DCL_USAGE_SHIFT) & D3D_DCL_USAGE_MASK;
	value->usageindex = (token >> D3D_DCL_USAGEINDEX_SHIFT) &
		D3D_DCL_USAGEINDEX_MASK;

	return index;
}

int
PixelShader::GetInstruction (int                        index,
			    d3d_destination_parameter_t *reg,
			    d3d_source_parameter_t      *src)
{
	index = GetDestinationParameter (index, reg);
	index = GetSourceParameter (index, src);

	return index;
}

int
PixelShader::GetInstruction (int                         index,
			     d3d_destination_parameter_t *reg,
			     d3d_source_parameter_t      *src1,
			     d3d_source_parameter_t      *src2)
{
	index = GetDestinationParameter (index, reg);
	index = GetSourceParameter (index, src1);
	index = GetSourceParameter (index, src2);

	return index;
}

int
PixelShader::GetInstruction (int                         index,
			     d3d_destination_parameter_t *reg,
			     d3d_source_parameter_t      *src1,
			     d3d_source_parameter_t      *src2,
			     d3d_source_parameter_t      *src3)
{
	index = GetDestinationParameter (index, reg);
	index = GetSourceParameter (index, src1);
	index = GetSourceParameter (index, src2);
	index = GetSourceParameter (index, src3);

	return index;
}
