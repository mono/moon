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
#include "projection.h"
#include "debug.h"

using namespace Moonlight;

GalliumContext *Effect::st_context;
pipe_screen    *Effect::screen;

cairo_user_data_key_t Effect::textureKey;
cairo_user_data_key_t Effect::surfaceKey;

int Effect::filtertable0[256];

#ifdef USE_GALLIUM
#undef CLAMP

#define __MOON_GALLIUM__
#include "context-gallium.h"

extern "C" {

#include "pipe/p_format.h"
#include "pipe/p_context.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_state.h"
#include "util/u_inlines.h"
#include "util/u_simple_screen.h"
#include "util/u_draw_quad.h"
#include "util/u_format.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_sampler.h"
#include "util/u_simple_shaders.h"
#include "util/u_debug.h"
#include "softpipe/sp_public.h"
#ifdef USE_LLVM
#include "llvmpipe/lp_public.h"
#endif
#define template templat
#include "state_tracker/sw_winsys.h"
#include "cso_cache/cso_context.h"
#include "tgsi/tgsi_ureg.h"
#include "tgsi/tgsi_dump.h"

}

#if MAX_SAMPLERS > PIPE_MAX_SAMPLERS
#error MAX_SAMPLERS is too large
#endif

static void
st_set_fragment_sampler_texture (GalliumContext *st_ctx,
				 unsigned index,
				 struct pipe_resource *texture)
{
      struct pipe_sampler_view templat;

      if (!texture)
	      texture = st_ctx->default_texture;

      pipe_sampler_view_reference (&st_ctx->fragment_sampler_views[index], NULL);

      u_sampler_view_default_template (&templat,
				       texture,
				       texture->format);

      st_ctx->fragment_sampler_views[index] = st_ctx->pipe->create_sampler_view (st_ctx->pipe,
										 texture,
										 &templat);

      st_ctx->pipe->set_fragment_sampler_views (st_ctx->pipe,
						PIPE_MAX_SAMPLERS,
						st_ctx->fragment_sampler_views);
}

struct moon_sw_winsys
{
	struct sw_winsys base;

	void     *user_data;
	unsigned user_stride;
};

static boolean
moon_ws_is_displaytarget_format_supported (struct sw_winsys *ws,
					   unsigned tex_usage,
					   enum pipe_format format)
{
	return FALSE;
}

static void *
moon_ws_displaytarget_map (struct sw_winsys *ws,
			   struct sw_displaytarget *dt,
			   unsigned flags)
{
	return (void *) dt;
}

static void
moon_ws_displaytarget_unmap (struct sw_winsys *ws,
			     struct sw_displaytarget *dt)
{
}

static void
moon_ws_displaytarget_destroy (struct sw_winsys *winsys,
			       struct sw_displaytarget *dt)
{
}

static struct sw_displaytarget *
moon_ws_displaytarget_create (struct sw_winsys *winsys,
			      unsigned tex_usage,
			      enum pipe_format format,
			      unsigned width,
			      unsigned height,
			      unsigned alignment,
			      unsigned *stride)
{
	struct moon_sw_winsys *ws = (struct moon_sw_winsys *) winsys;

	*stride = ws->user_stride;
	return (struct sw_displaytarget *) ws->user_data;
}

static void
moon_ws_displaytarget_display (struct sw_winsys *winsys,
			       struct sw_displaytarget *dt,
			       void *context_private)
{
}

static void
moon_ws_destroy (struct sw_winsys *winsys)
{
	FREE (winsys);
}

static struct pipe_screen *
moon_sw_screen_create (const char *driver)
{
	static struct moon_sw_winsys *winsys;
	struct pipe_screen *screen = NULL;

	winsys = CALLOC_STRUCT (moon_sw_winsys);
	if (!winsys)
		return NULL;

	winsys->base.destroy = moon_ws_destroy;
	winsys->base.is_displaytarget_format_supported =
		moon_ws_is_displaytarget_format_supported;
	winsys->base.displaytarget_create = moon_ws_displaytarget_create;
	winsys->base.displaytarget_map = moon_ws_displaytarget_map;
	winsys->base.displaytarget_unmap = moon_ws_displaytarget_unmap;
	winsys->base.displaytarget_display = moon_ws_displaytarget_display;
	winsys->base.displaytarget_destroy = moon_ws_displaytarget_destroy;

	if (!driver) {
		const char *default_driver;

#ifdef USE_LLVM
		default_driver = "llvmpipe";
#else
		default_driver = "softpipe";
#endif

		driver = debug_get_option ("GALLIUM_DRIVER", default_driver);
	}

#ifdef USE_LLVM
	if (strcmp (driver, "llvmpipe") == 0) {
		screen = llvmpipe_create_screen (&winsys->base);
	}
#endif

	if (strcmp (driver, "softpipe") == 0) {
		screen = softpipe_create_screen (&winsys->base);
	}

	if (screen)
		screen->winsys = (struct pipe_winsys *) winsys;

	return screen;
}

static struct pipe_resource *
moon_sw_resource_create (struct pipe_screen *screen,
			 const struct pipe_resource *templat,
			 void *data,
			 unsigned stride)
{
	struct moon_sw_winsys *ws = (struct moon_sw_winsys *) screen->winsys;

	ws->user_data = data;
	ws->user_stride = stride;

	return screen->resource_create (screen, templat);
}

static void
st_resource_destroy_callback (void *data)
{
	struct pipe_resource *resource = (struct pipe_resource *) data;

	pipe_resource_reference (&resource, NULL);
}

static void
st_surface_destroy_callback (void *data)
{
	struct pipe_surface *surface = (struct pipe_surface *) data;

	pipe_surface_reference (&surface, NULL);
}

static INLINE void
ureg_convolution (struct ureg_program *ureg,
		  struct ureg_dst     out,
		  struct ureg_src     sampler,
		  struct ureg_src     tex,
		  int                 size,
		  int                 base)
{
	struct ureg_dst val, tmp;
	int             i;

	val = ureg_DECL_temporary (ureg);
	tmp = ureg_DECL_temporary (ureg);

	ureg_ADD (ureg, tmp, tex, ureg_DECL_constant (ureg, base));
	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (tmp), sampler);
	ureg_MUL (ureg, val, ureg_src (tmp),
		  ureg_DECL_constant (ureg, base + size + 1));

	ureg_SUB (ureg, tmp, tex, ureg_DECL_constant (ureg, base));
	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (tmp), sampler);
	ureg_MAD (ureg, val, ureg_src (tmp),
		  ureg_DECL_constant (ureg, base + size + 1),
		  ureg_src (val));

	for (i = 1; i < size; i++) {
		ureg_ADD (ureg, tmp, tex, ureg_DECL_constant (ureg, base + i));
		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (tmp), sampler);
		ureg_MAD (ureg, val, ureg_src (tmp),
			  ureg_DECL_constant (ureg, base + size + i + 1),
			  ureg_src (val));
		ureg_SUB (ureg, tmp, tex, ureg_DECL_constant (ureg, base + i));
		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, ureg_src (tmp), sampler);
		ureg_MAD (ureg, val, ureg_src (tmp),
			  ureg_DECL_constant (ureg, base + size + i + 1),
			  ureg_src (val));
	}

	ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, tex, sampler);
	ureg_MAD (ureg, out, ureg_src (tmp),
		  ureg_DECL_constant (ureg, base + size),
		  ureg_src (val));

	ureg_release_temporary (ureg, tmp);
	ureg_release_temporary (ureg, val);
}
#endif

#define sw_filter_sample(src, filter)		\
	(*filter)[(int) (src)]

#define sw_filter_sample_8888_init(t, f, sample)	 \
	sample[0] = sw_filter_sample (t[0], f);	 \
	sample[1] = sw_filter_sample (t[1], f);	 \
	sample[2] = sw_filter_sample (t[2], f);	 \
	sample[3] = sw_filter_sample (t[3], f)

#define sw_filter_sample_8888_add(t, f, sample)		 \
	sample[0] += sw_filter_sample (t[0], f);		 \
	sample[1] += sw_filter_sample (t[1], f);		 \
	sample[2] += sw_filter_sample (t[2], f);		 \
	sample[3] += sw_filter_sample (t[3], f)

#define sw_filter_sample_output(sample)		\
	((unsigned char) ((sample) >> 16))

#define sw_filter_sample_8888_write(dst, sample)	\
	dst[0] = sw_filter_sample_output (sample[0]);	\
	dst[1] = sw_filter_sample_output (sample[1]);	\
	dst[2] = sw_filter_sample_output (sample[2]);	\
	dst[3] = sw_filter_sample_output (sample[3])

#define SW_RED   2
#define SW_GREEN 1
#define SW_BLUE  0
#define SW_ALPHA 3

static inline void
sw_filter_process_8888x8888 (unsigned char *s,
			     unsigned char *d,
			     int           size,
			     int           stride,
			     int           filter_size,
			     int           **filter)
{
	unsigned char *end = d + size * stride;
	unsigned char *s1 = s + (size - 1) * stride;
	unsigned char *t0 = s - filter_size * stride;
	unsigned char *t1 = s + filter_size * stride;
	unsigned char *t;
	int           **fend = filter + filter_size * 2;
	int           **f;
	int           sample[4];

	while (t0 < s) {
		t = t1;
		f = fend;

		sw_filter_sample_8888_init (t, f, sample);

		while (t > s) {
			t -= stride;
			f--;

			sw_filter_sample_8888_add (t, f, sample);
		}

		sw_filter_sample_8888_write (d, sample);

		d  += stride;
		t0 += stride;
		t1 += stride;
	}

	while (t1 <= s1) {
		t = t0;
		f = filter;

		sw_filter_sample_8888_init (t, f, sample);

		while (t < t1) {
			t += stride;
			f++;

			sw_filter_sample_8888_add (t, f, sample);
		}

		sw_filter_sample_8888_write (d, sample);

		d  += stride;
		t0 += stride;
		t1 += stride;
	}

	while (d < end) {
		t = t0;
		f = filter;

		sw_filter_sample_8888_init (t, f, sample);

		while (t < s1) {
			t += stride;
			f++;

			sw_filter_sample_8888_add (t, f, sample);
		}

		sw_filter_sample_8888_write (d, sample);

		d  += stride;
		t0 += stride;
	}
}

static void
sw_filter_blur (unsigned char *src,
		unsigned char *dst,
		int           width,
		int           height,
		int           stride,
		int           filter_size,
		int           **filter)
{
	unsigned char *tmp_data = (unsigned char *) g_malloc (stride * height);
	int           x = 0;
	int           y = 0;

	while (y < height) {
		sw_filter_process_8888x8888 (src + y * stride,
					     tmp_data + y * stride,
					     width,
					     4,
					     filter_size,
					     filter);

		y++;
	}

	while (x < width) {
		sw_filter_process_8888x8888 (tmp_data + x * 4,
					     dst + x * 4,
					     height,
					     stride,
					     filter_size,
					     filter);

		x++;
	}

	g_free (tmp_data);
}

static void
sw_filter_process_8888x8 (unsigned char *s,
			  unsigned char *d,
			  int           size,
			  int           s_stride,
			  int           d_stride,
			  int           s_offset,
			  int           filter_size,
			  int           **filter)
{
	unsigned char *end = d + size * d_stride;
	unsigned char *s1 = s + (size - 1) * s_stride;
	unsigned char *t0 = s - filter_size * s_stride;
	unsigned char *t1 = s + filter_size * s_stride;
	unsigned char *t;
	unsigned char *tend;
	int           **f;
	int           sample;

	t0 += s_offset * s_stride;
	t1 += s_offset * s_stride;

	while (d < end) {
		sample = 0;
		t      = t0;
		tend   = MIN (t1, s1);
		f      = filter;

		while (t < s) {
			t += s_stride;
			f++;
		}

		while (t <= tend) {
			sample += sw_filter_sample (t[SW_ALPHA], f);

			t += s_stride;
			f++;
		}

		d[0] = sw_filter_sample_output (sample);

		d  += d_stride;
		t0 += s_stride;
		t1 += s_stride;
	}
}

static void
sw_filter_process_8x8888_over_black (unsigned char *s,
				     unsigned char *d,
				     int           size,
				     int           s_stride,
				     int           d_stride,
				     int           filter_size,
				     int           **filter)
{
	unsigned char *end = d + size * d_stride;
	unsigned char *s1 = s + (size - 1) * s_stride;
	unsigned char *t0 = s - filter_size * s_stride;
	unsigned char *t1 = s + filter_size * s_stride;
	unsigned char *t;
	int           **fend = filter + filter_size * 2;
	int           **f;
	int           alpha, sample;

	while (t0 < s) {
		alpha = 255 - d[SW_ALPHA];
		if (alpha) {
			t = t1;
			f = fend;

			sample = sw_filter_sample (t[0], f);

			while (t > s) {
				t -= s_stride;
				f--;

				sample += sw_filter_sample (t[0], f);
			}

			d[SW_ALPHA] += ((sample >> 16) * alpha) >> 8;
		}

		d  += d_stride;
		t0 += s_stride;
		t1 += s_stride;
	}

	while (t1 <= s1) {
		alpha = 255 - d[SW_ALPHA];
		if (alpha) {
			t = t0;
			f = filter;

			sample = sw_filter_sample (t[0], f);

			while (t < t1) {
				t += s_stride;
				f++;

				sample += sw_filter_sample (t[0], f);
			}

			d[SW_ALPHA] += ((sample >> 16) * alpha) >> 8;
		}

		d  += d_stride;
		t0 += s_stride;
		t1 += s_stride;
	}

	while (d < end) {
		alpha = 255 - d[SW_ALPHA];
		if (alpha) {
			t = t0;
			f = filter;

			sample = sw_filter_sample (t[0], f);

			while (t < s1) {
				t += s_stride;
				f++;

				sample += sw_filter_sample (t[0], f);
			}

			d[SW_ALPHA] += ((sample >> 16) * alpha) >> 8;
		}

		d  += d_stride;
		t0 += s_stride;
	}
}

static void
sw_filter_process_8x8888_over_color (unsigned char *s,
				     unsigned char *d,
				     int           size,
				     int           s_stride,
				     int           d_stride,
				     int           filter_size,
				     int           **filter,
				     int           *color)
{
	unsigned char *end = d + size * d_stride;
	unsigned char *s1 = s + (size - 1) * s_stride;
	unsigned char *t0 = s - filter_size * s_stride;
	unsigned char *t1 = s + filter_size * s_stride;
	unsigned char *t;
	int           **fend = filter + filter_size * 2;
	int           **f;
	int           alpha, sample;

	while (t0 < s) {
		alpha = 255 - d[SW_ALPHA];
		if (alpha) {
			t = t1;
			f = fend;

			sample = sw_filter_sample (t[0], f);

			while (t > s) {
				t -= s_stride;
				f--;

				sample += sw_filter_sample (t[0], f);
			}

			d[0] += ((sample >> 16) * color[0] * alpha) >> 16;
			d[1] += ((sample >> 16) * color[1] * alpha) >> 16;
			d[2] += ((sample >> 16) * color[2] * alpha) >> 16;
			d[3] += ((sample >> 16) * color[3] * alpha) >> 16;
		}

		d  += d_stride;
		t0 += s_stride;
		t1 += s_stride;
	}

	while (t1 <= s1) {
		alpha = 255 - d[SW_ALPHA];
		if (alpha) {
			t = t0;
			f = filter;

			sample = sw_filter_sample (t[0], f);

			while (t < t1) {
				t += s_stride;
				f++;

				sample += sw_filter_sample (t[0], f);
			}

			d[0] += ((sample >> 16) * color[0] * alpha) >> 16;
			d[1] += ((sample >> 16) * color[1] * alpha) >> 16;
			d[2] += ((sample >> 16) * color[2] * alpha) >> 16;
			d[3] += ((sample >> 16) * color[3] * alpha) >> 16;
		}

		d  += d_stride;
		t0 += s_stride;
		t1 += s_stride;
	}

	while (d < end) {
		alpha = 255 - d[SW_ALPHA];
		if (alpha) {
			t = t0;
			f = filter;

			sample = sw_filter_sample (t[0], f);

			while (t < s1) {
				t += s_stride;
				f++;

				sample += sw_filter_sample (t[0], f);
			}

			d[0] += ((sample >> 16) * color[0] * alpha) >> 16;
			d[1] += ((sample >> 16) * color[1] * alpha) >> 16;
			d[2] += ((sample >> 16) * color[2] * alpha) >> 16;
			d[3] += ((sample >> 16) * color[3] * alpha) >> 16;
		}

		d  += d_stride;
		t0 += s_stride;
	}
}

static void
sw_filter_drop_shadow (unsigned char *src,
		       unsigned char *dst,
		       int           width,
		       int           height,
		       int           stride,
		       int           src_x,
		       int           src_y,
		       int           filter_size,
		       int           **filter,
		       int           *color)
{
	unsigned char *tmp_data = (unsigned char *) g_malloc (width * height);
	int           dst_x = 0;
	int           dst_y = 0;
	int           bottom;

	while (src_y < 0) {
		memset (tmp_data + dst_y * width, 0, width);

		src_y++;
		dst_y++;
	}

	bottom = height - src_y;

	while (dst_y < bottom) {
		sw_filter_process_8888x8 (src + src_y * stride,
					  tmp_data + dst_y * width,
					  width,
					  4,
					  1,
					  src_x,
					  filter_size,
					  filter);

		src_y++;
		dst_y++;
	}

	while (dst_y < height) {
		memset (tmp_data + dst_y * width, 0, width);

		dst_y++;
	}

	memcpy (dst, src, height * stride);

	if (color[SW_RED]   != 0 ||
	    color[SW_GREEN] != 0 ||
	    color[SW_BLUE]  != 0 ||
	    color[SW_ALPHA] != 255) {
		while (dst_x < width) {
			sw_filter_process_8x8888_over_color (tmp_data + dst_x,
							     dst + dst_x * 4,
							     height,
							     width,
							     stride,
							     filter_size,
							     filter,
							     color);

			dst_x++;
		}
	}
	else {
		while (dst_x < width) {
			sw_filter_process_8x8888_over_black (tmp_data + dst_x,
							     dst + dst_x * 4,
							     height,
							     width,
							     stride,
							     filter_size,
							     filter);

			dst_x++;
		}
	}

	free (tmp_data);
}

void
Effect::Initialize ()
{

#ifdef USE_GALLIUM
	screen = moon_sw_screen_create (NULL);
	g_assert (screen);
	st_context = new GalliumContext (screen);
	g_assert (st_context);
#endif

	for (int i = 0; i < 256; i++)
		filtertable0[i] = i << 16;
}

int
Effect::CalculateGaussianSamples (double radius,
				  double precision,
				  double *row)
{
	double sigma = radius / 3.0;
	double coeff = 2.0 * sigma * sigma;
	double sum = 0.0;
	double norm;
	int    width = (int) ceil (radius);
	int    i;

	if (sigma <= 0.0 || width <= 0)
		return 0;

	norm = 1.0 / (sqrt (2.0 * M_PI) * sigma);

	for (i = 1; i <= width; i++) {
		row[i] = norm * exp (-i * i / coeff);
		sum += row[i];
	}

	*row = norm;
	sum = sum * 2.0 + norm;

	for (i = 0; i <= width; i++) {
		row[i] /= sum;
		
		if (row[i] < precision)
			return i;
	}

	return width;
}

void
Effect::UpdateFilterValues (double radius,
			    double *values,
			    int    ***table,
			    int    *size)
{
	int n;

	n = CalculateGaussianSamples (radius, 1.0 / 256.0, values);
	if (n != *size) {
		*size = n;
		g_free (*table);

		if (n) {
			int  entries = n * 2 + 1;
			int  ptr_size = sizeof (int *) * entries;
			int  value_size = sizeof (int) * 256;
			int  data_size = value_size * entries;
			char *bytes;

			bytes = (char *) g_malloc (ptr_size + data_size);
			*table = (int **) bytes;

			for (int i = 0; i < entries; i++)
				(*table)[i] = (int *)
					(bytes + ptr_size + i * value_size);
		}
		else {
			*table = NULL;
		}
	}

	if (!n)
		return;

	for (int j = 0; j < 256; j++) {
		int *center = (*table)[n];

		center[j] = (int) (values[0] * (double) (j << 16));
	}

	for (int i = 1; i <= n; i++) {
		int *left  = (*table)[n - i];
		int *right = (*table)[n + i];

		for (int j = 0; j < 256; j++)
			left[j] = right[j] = (int)
				(values[i] * (double) (j << 16));
	}
}

void
Effect::Shutdown ()
{

#ifdef USE_GALLIUM
	delete st_context;
	screen->destroy (screen);
#endif

}

Effect::Effect ()
{
	SetObjectType (Type::EFFECT);

	need_update = true;
}

struct pipe_resource *
Effect::GetShaderTexture (cairo_surface_t *surface)
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	struct pipe_resource *tex, templat;

	tex = (struct pipe_resource *) cairo_surface_get_user_data (surface, &textureKey);
	if (tex)
		return tex;

	if (cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE)
		return NULL;

	memset (&templat, 0, sizeof (templat));
	templat.format = PIPE_FORMAT_B8G8R8A8_UNORM;
	templat.width0 = cairo_image_surface_get_width (surface);
	templat.height0 = cairo_image_surface_get_height (surface);
	templat.depth0 = 1;
	templat.last_level = 0;
	templat.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_DISPLAY_TARGET;
	templat.target = PIPE_TEXTURE_2D;

	switch (cairo_image_surface_get_format (surface)) {
		case CAIRO_FORMAT_ARGB32:
			templat.format = PIPE_FORMAT_B8G8R8A8_UNORM;
			break;
		case CAIRO_FORMAT_RGB24:
		case CAIRO_FORMAT_A8:
		case CAIRO_FORMAT_A1:
			return NULL;
	}

	tex = moon_sw_resource_create (ctx->pipe->screen,
				       &templat,
				       cairo_image_surface_get_data (surface),
				       cairo_image_surface_get_stride (surface));

	cairo_surface_set_user_data (surface,
				     &textureKey,
				     (void *) tex,
				     st_resource_destroy_callback);

	return tex;
#else
	return NULL;
#endif

}

struct pipe_surface *
Effect::GetShaderSurface (cairo_surface_t *surface)
{
	
#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	struct pipe_surface  *sur;
	struct pipe_resource *tex;

	sur = (struct pipe_surface *) cairo_surface_get_user_data (surface, &surfaceKey);
	if (sur)
		return sur;

	if (cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE)
		return NULL;

	tex = GetShaderTexture (surface);
	if (!tex)
		return NULL;

	sur = ctx->pipe->screen->get_tex_surface (ctx->pipe->screen, tex,
						  0, 0, 0,
						  PIPE_BIND_TRANSFER_WRITE |
						  PIPE_BIND_TRANSFER_READ);

	cairo_surface_set_user_data (surface,
				     &surfaceKey,
				     (void *) sur,
				     st_surface_destroy_callback);

	return sur;
#else
	return NULL;
#endif

}

struct pipe_resource *
Effect::CreateVertexBuffer (struct pipe_resource *texture,
			    const double         *matrix,
			    double               x,
			    double               y,
			    double               width,
			    double               height,
			    double               s,
			    double               t)
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	struct pipe_resource *vertices;
	float                *verts;
	struct pipe_transfer *transfer = NULL;
	double               p1[4] = { x, y, 0.0, 1.0 };
	double               p2[4] = { x + width, y, 0.0, 1.0 };
	double               p3[4] = { x + width, y + height, 0.0, 1.0 };
	double               p4[4] = { x, y + height, 0.0, 1.0 };

	vertices = pipe_buffer_create (ctx->pipe->screen,
				       PIPE_BIND_VERTEX_BUFFER,
				       sizeof (float) * 8 * 4);
	if (!vertices)
		return NULL;

	verts = (float *) pipe_buffer_map (ctx->pipe,
					   vertices,
					   PIPE_TRANSFER_WRITE,
					   &transfer);
	if (!verts)
	{
		pipe_resource_reference (&vertices, NULL);
		return NULL;
	}

	if (matrix) {
		Matrix3D::TransformPoint (p1, matrix, p1);
		Matrix3D::TransformPoint (p2, matrix, p2);
		Matrix3D::TransformPoint (p3, matrix, p3);
		Matrix3D::TransformPoint (p4, matrix, p4);
	}

	*verts++ = p1[0] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p1[1] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p1[2];
	*verts++ = p1[3];

	*verts++ = 0.0;
	*verts++ = 0.0;
	*verts++ = 0.f;
	*verts++ = 0.f;

	*verts++ = p2[0] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p2[1] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p2[2];
	*verts++ = p2[3];

	*verts++ = s;
	*verts++ = 0.0;
	*verts++ = 0.f;
	*verts++ = 0.f;

	*verts++ = p3[0] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p3[1] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p3[2];
	*verts++ = p3[3];

	*verts++ = s;
	*verts++ = t;
	*verts++ = 0.f;
	*verts++ = 0.f;

	*verts++ = p4[0] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p4[1] * VIEWPORT_SCALE_RECIPROCAL;
	*verts++ = p4[2];
	*verts++ = p4[3];

	*verts++ = 0.0;
	*verts++ = t;
	*verts++ = 0.f;
	*verts++ = 0.f;

	pipe_buffer_unmap (ctx->pipe, vertices, transfer);

	return vertices;

#else
	return NULL;
#endif

}

void
Effect::DrawVertexBuffer (struct pipe_surface  *dst,
			  struct pipe_resource *vertices,
			  double               dstX,
			  double               dstY,
			  const Rect           *clip)
{

#ifdef USE_GALLIUM
	GalliumContext                *ctx = st_context;
	struct pipe_fence_handle      *fence = NULL;
	struct pipe_framebuffer_state fb;
	struct pipe_rasterizer_state  rasterizer;
	struct pipe_viewport_state    viewport;

	memset (&viewport, 0, sizeof (struct pipe_viewport_state));
	viewport.scale[0] = VIEWPORT_SCALE;
	viewport.scale[1] = VIEWPORT_SCALE;
	viewport.scale[2] = 1.0;
	viewport.scale[3] = 1.0;
	viewport.translate[0] = dstX;
	viewport.translate[1] = dstY;
	viewport.translate[2] = 0.0;
	viewport.translate[3] = 0.0;
	cso_set_viewport (ctx->cso, &viewport);

	memset (&rasterizer, 0, sizeof (rasterizer));
	rasterizer.cull_face = PIPE_FACE_NONE;
	rasterizer.gl_rasterization_rules = 1;
	if (clip) {
		struct pipe_scissor_state scissor;

		scissor.minx = clip->x;
		scissor.miny = clip->y;
		scissor.maxx = clip->x + clip->width;
		scissor.maxy = clip->y + clip->height;

		(*ctx->pipe->set_scissor_state) (ctx->pipe, &scissor);

		rasterizer.scissor = 1;
	}
	cso_set_rasterizer (ctx->cso, &rasterizer);

	cso_save_framebuffer (ctx->cso);

	memset (&fb, 0, sizeof (struct pipe_framebuffer_state));
	fb.width = dst->width;
	fb.height = dst->height;
	fb.nr_cbufs = 1;
	fb.cbufs[0] = dst;
	cso_set_framebuffer (ctx->cso, &fb);

	cso_set_vertex_elements (ctx->cso, 2, ctx->velems);
	util_draw_vertex_buffer (ctx->pipe, vertices, 0,
				 PIPE_PRIM_TRIANGLE_FAN,
				 4,
				 2);
	
	ctx->pipe->flush (ctx->pipe, PIPE_FLUSH_RENDER_CACHE, &fence);
	if (fence) {
		/* TODO: allow asynchronous operation */
		ctx->pipe->screen->fence_finish (ctx->pipe->screen, fence, 0);
		ctx->pipe->screen->fence_reference (ctx->pipe->screen, &fence, NULL);
	}

	cso_restore_framebuffer (ctx->cso);
#endif

}

bool
Effect::Composite (pipe_surface_t  *dst,
		   pipe_resource_t *src,
		   const double    *matrix,
		   double          dstX,
		   double          dstY,
		   const Rect      *clip,
		   double          x,
		   double          y,
		   double          width,
		   double          height)
{
	g_warning ("Effect::Composite has been called. The derived class should have overridden it.");
	return 0;
}

bool
Effect::Render (Context      *ctx,
		MoonSurface  *src,
		const double *matrix,
		double       x,
		double       y,
		double       width,
		double       height)
{
	struct pipe_surface  *surface;
	struct pipe_resource *texture;
	cairo_surface_t      *dst;
	cairo_surface_t      *cs;
	cairo_t              *cr = ctx->Cairo ();
	double               dstX, dstY;
	bool                 status = 0;

	dst = cairo_get_group_target (cr);
	cairo_surface_get_device_offset (dst, &dstX, &dstY);

	MaybeUpdateShader ();

	cs = src->Cairo ();

	surface = GetShaderSurface (dst);
	texture = GetShaderTexture (cs);

	if (surface && texture)
		status = Composite (surface, texture, matrix,
				    dstX, dstY,
				    NULL,
				    x, y, width, height);

	cairo_surface_destroy (cs);

	return status;
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

	fs = NULL;
	horz_pass_constant_buffer = NULL;
	vert_pass_constant_buffer = NULL;
	filter_size = 0;
	nfiltervalues = 0;
	filtertable = NULL;
	need_filter_update = true;
}

BlurEffect::~BlurEffect ()
{
	g_free (filtertable);
	Clear ();
}

void
BlurEffect::Clear ()
{

#ifdef USE_GALLIUM
	GalliumContext *ctx = st_context;

	pipe_resource_reference (&horz_pass_constant_buffer, NULL);
	pipe_resource_reference (&vert_pass_constant_buffer, NULL);

	if (fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, fs);
		fs = NULL;
	}
#endif

}

void
BlurEffect::MaybeUpdateFilter ()
{
	if (need_filter_update) {
		UpdateFilterValues (MIN (GetRadius (), MAX_BLUR_RADIUS),
				    filtervalues,
				    &filtertable,
				    &nfiltervalues);
		need_filter_update = false;
	}
}

void
BlurEffect::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::BLUREFFECT) {
		Effect::OnPropertyChanged (args, error);
		return;
	}

	need_update = need_filter_update = true;

	NotifyListenersOfPropertyChange (args, error);
}

Thickness
BlurEffect::Padding ()
{
	MaybeUpdateFilter ();

	return Thickness (nfiltervalues);
}

bool
BlurEffect::Composite (pipe_surface_t  *dst,
		       pipe_resource_t *src,
		       const double    *matrix,
		       double          dstX,
		       double          dstY,
		       const Rect      *clip,
		       double          x,
		       double          y,
		       double          width,
		       double          height)
{
	
#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	struct pipe_screen   *screen = ctx->pipe->screen;
	GalliumSurface       *intermediate;
	struct pipe_resource *intermediate_texture;
	struct pipe_surface  *intermediate_surface;
	struct pipe_resource *vertices, *intermediate_vertices;
	double               s = src->width0;
	double               t = src->height0;

	MaybeUpdateFilter ();
	MaybeUpdateShader ();

	if (!fs)
		return 0;

	intermediate = new GalliumSurface (ctx->pipe,
					   src->width0,
					   src->height0);
	if (!intermediate)
		return 0;

	intermediate_texture = intermediate->Texture ();
	if (!intermediate_texture) {
		intermediate->unref ();
		return 0;
	}

	intermediate_surface =
		screen->get_tex_surface (screen,
					 intermediate_texture,
					 0,
					 0,
					 0,
					 PIPE_BIND_RENDER_TARGET);
	if (!intermediate_surface) {
		intermediate->unref ();
		return 0;
	}

	vertices = CreateVertexBuffer (intermediate_texture, matrix,
				       x, y,
				       width, height,
				       s, t);
	if (!vertices) {
		pipe_surface_reference (&intermediate_surface, NULL);
		intermediate->unref ();
		return 0;
	}

	intermediate_vertices = CreateVertexBuffer (src, NULL,
						    0.0, 0.0,
						    s, t,
						    s, t);
	if (!intermediate_vertices) {
		pipe_resource_reference (&vertices, NULL);
		pipe_surface_reference (&intermediate_surface, NULL);
		intermediate->unref ();
		return 0;
	}

	if (cso_set_fragment_shader_handle (ctx->cso, fs) != PIPE_OK) {
		pipe_resource_reference (&intermediate_vertices, NULL);
		pipe_resource_reference (&vertices, NULL);
		pipe_surface_reference (&intermediate_surface, NULL);
		intermediate->unref ();
		return 0;
	}

	struct pipe_sampler_state sampler;
	memset(&sampler, 0, sizeof(struct pipe_sampler_state));
	sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
	sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
	sampler.normalized_coords = 0;
	cso_single_sampler(ctx->cso, 0, &sampler);
	cso_single_sampler_done(ctx->cso);

	st_set_fragment_sampler_texture (ctx, 0, src);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, horz_pass_constant_buffer);

	struct pipe_blend_state blend;
	memset (&blend, 0, sizeof (blend));
	blend.rt[0].colormask |= PIPE_MASK_RGBA;
	blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].blend_enable = 0;
	blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_ZERO;
	blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;
	cso_set_blend (ctx->cso, &blend);

	DrawVertexBuffer (intermediate_surface, intermediate_vertices,
			  0.0, 0.0);

	st_set_fragment_sampler_texture (ctx, 0, intermediate_texture);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, vert_pass_constant_buffer);

	memset (&blend, 0, sizeof (blend));
	blend.rt[0].colormask |= PIPE_MASK_RGBA;
	blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].blend_enable = 1;
	blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	cso_set_blend (ctx->cso, &blend);

	DrawVertexBuffer (dst, vertices, dstX, dstY, clip);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, NULL);

	st_set_fragment_sampler_texture (ctx, 0, NULL);

	pipe_resource_reference (&intermediate_vertices, NULL);
	pipe_resource_reference (&vertices, NULL);
	pipe_surface_reference (&intermediate_surface, NULL);
	intermediate->unref ();

	cso_set_fragment_shader_handle (ctx->cso, ctx->fs);

	return 1;
#else
	return 0;
#endif

}

bool
BlurEffect::Render (Context      *ctx,
		    MoonSurface  *src,
		    const double *matrix,
		    double       x,
		    double       y,
		    double       width,
		    double       height)
{
	cairo_surface_t *cs = src->Cairo ();

	MaybeUpdateFilter ();

	/* table based filter code when possible */
	if (cairo_surface_get_type (cs) == CAIRO_SURFACE_TYPE_IMAGE) {
		int           pw = cairo_image_surface_get_width (cs);
		int           ph = cairo_image_surface_get_height (cs);
		int           stride = cairo_image_surface_get_stride (cs);
		unsigned char *data = (unsigned char *) g_malloc (stride * ph);
		Rect          r = Rect (x, y, width, height);
		cairo_t       *cr = ctx->Cairo ();

		cairo_save (cr);
		cairo_identity_matrix (cr);
		r.RoundOut ().Draw (cr);
		cairo_clip (cr);

		if (nfiltervalues) {
			const cairo_format_t format = CAIRO_FORMAT_ARGB32;
			cairo_surface_t      *image;

			sw_filter_blur (cairo_image_surface_get_data (cs),
					data,
					pw,
					ph,
					stride,
					nfiltervalues,
					filtertable);

			image = cairo_image_surface_create_for_data (data,
								     format,
								     pw,
								     ph,
								     stride);

			cairo_set_source_surface (cr, image, r.x, r.y);
			cairo_surface_destroy (image);
		}
		else {
			cairo_set_source_surface (cr, cs, r.x, r.y);
		}

		cairo_paint (cr);
		cairo_restore (cr);

		g_free (data);
		cairo_surface_destroy (cs);

		return 1;
	}

	cairo_surface_destroy (cs);

	return Effect::Render (ctx, src, matrix, x, y, width, height);
}

void
BlurEffect::UpdateShader ()
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	int                  width = nfiltervalues;
	float                *horz, *vert;
	struct pipe_transfer *horz_transfer = NULL, *vert_transfer = NULL;
	int                  i;

	if (width != filter_size && width > 0) {
		struct ureg_program *ureg;
		struct ureg_src     sampler, tex;
		struct ureg_dst     out;

		Clear ();

		filter_size = width;

		ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);
		if (!ureg)
			return;

		sampler = ureg_DECL_sampler (ureg, 0);

		tex = ureg_DECL_fs_input (ureg,
					  TGSI_SEMANTIC_GENERIC, 0,
					  TGSI_INTERPOLATE_LINEAR);

		out = ureg_DECL_output (ureg,
					TGSI_SEMANTIC_COLOR,
					0);

		ureg_convolution (ureg, out, sampler, tex, width, 0);

		ureg_END (ureg);

#if LOGGING
		if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_EFFECT)) {
			printf ("BlurEffect::UpdateShader: TGSI shader:\n");
			tgsi_dump (ureg_get_tokens (ureg, NULL), 0);
		}
#endif

		fs = ureg_create_shader_and_destroy (ureg, ctx->pipe);
		if (!fs)
			return;

		horz_pass_constant_buffer =
			pipe_buffer_create (ctx->pipe->screen,
					    PIPE_BIND_CONSTANT_BUFFER,
					    sizeof (float) * 4 * (width * 2 + 1));
		if (!horz_pass_constant_buffer) {
			Clear ();
			return;
		}

		vert_pass_constant_buffer =
			pipe_buffer_create (ctx->pipe->screen,
					    PIPE_BIND_CONSTANT_BUFFER,
					    sizeof (float) * 4 * (width * 2 + 1));
		if (!vert_pass_constant_buffer) {
			Clear ();
			return;
		}
	}
	else {
		filter_size = width;
		if (filter_size == 0)
			return;
	}

	horz = (float *) pipe_buffer_map (ctx->pipe,
					  horz_pass_constant_buffer,
					  PIPE_TRANSFER_WRITE,
					  &horz_transfer);
	if (!horz) {
		Clear ();
		return;
	}

	vert = (float *) pipe_buffer_map (ctx->pipe,
					  vert_pass_constant_buffer,
					  PIPE_TRANSFER_WRITE,
					  &vert_transfer);
	if (!vert) {
		pipe_buffer_unmap (ctx->pipe, horz_pass_constant_buffer, horz_transfer);
		Clear ();
		return;
	}

	for (i = 1; i <= width; i++) {
		*horz++ = i;
		*horz++ = 0.f;
		*horz++ = 0.f;
		*horz++ = 1.f;

		*vert++ = 0.f;
		*vert++ = i;
		*vert++ = 0.f;
		*vert++ = 1.f;
	}

	for (i = 0; i <= width; i++) {
		*horz++ = filtervalues[i];
		*horz++ = filtervalues[i];
		*horz++ = filtervalues[i];
		*horz++ = filtervalues[i];

		*vert++ = filtervalues[i];
		*vert++ = filtervalues[i];
		*vert++ = filtervalues[i];
		*vert++ = filtervalues[i];
	}

	pipe_buffer_unmap (ctx->pipe, horz_pass_constant_buffer, horz_transfer);
	pipe_buffer_unmap (ctx->pipe, vert_pass_constant_buffer, vert_transfer);
#endif

}

DropShadowEffect::DropShadowEffect ()
{
	SetObjectType (Type::DROPSHADOWEFFECT);

	horz_fs = NULL;
	vert_fs = NULL;
	horz_pass_constant_buffer = NULL;
	vert_pass_constant_buffer = NULL;
	filter_size = -1;
	nfiltervalues = 0;
	filtertable = NULL;
	need_filter_update = true;
}

DropShadowEffect::~DropShadowEffect ()
{
	g_free (filtertable);
	Clear ();
}

void
DropShadowEffect::Clear ()
{

#ifdef USE_GALLIUM
	GalliumContext *ctx = st_context;

	pipe_resource_reference (&horz_pass_constant_buffer, NULL);
	pipe_resource_reference (&vert_pass_constant_buffer, NULL);

	if (horz_fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, horz_fs);
		horz_fs = NULL;
	}

	if (vert_fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, vert_fs);
		vert_fs = NULL;
	}
#endif

}

void
DropShadowEffect::MaybeUpdateFilter ()
{
	if (need_filter_update) {
		UpdateFilterValues (MIN (GetBlurRadius (), MAX_BLUR_RADIUS),
				    filtervalues,
				    &filtertable,
				    &nfiltervalues);

		need_filter_update = false;
	}
}

void
DropShadowEffect::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::DROPSHADOWEFFECT) {
		Effect::OnPropertyChanged (args, error);
		return;
	}

	need_update = need_filter_update = true;

	NotifyListenersOfPropertyChange (args, error);
}

Thickness
DropShadowEffect::Padding ()
{
	double direction = GetDirection () * (M_PI / 180.0);
	double depth = CLAMP (GetShadowDepth (), 0.0, MAX_SHADOW_DEPTH);
	double left;
	double top;
	double right;
	double bottom;

	MaybeUpdateFilter ();

	left   = -cos (direction) * depth + nfiltervalues;
	top    =  sin (direction) * depth + nfiltervalues;
	right  =  cos (direction) * depth + nfiltervalues;
	bottom = -sin (direction) * depth + nfiltervalues;

	return Thickness (left   < 1.0 ? 1.0 : ceil (left),
			  top    < 1.0 ? 1.0 : ceil (top),
			  right  < 1.0 ? 1.0 : ceil (right),
			  bottom < 1.0 ? 1.0 : ceil (bottom));
}

bool
DropShadowEffect::Composite (pipe_surface_t  *dst,
			     pipe_resource_t *src,
			     const double    *matrix,
			     double          dstX,
			     double          dstY,
			     const Rect      *clip,
			     double          x,
			     double          y,
			     double          width,
			     double          height)
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	struct pipe_screen   *screen = ctx->pipe->screen;
	GalliumSurface       *intermediate;
	struct pipe_resource *intermediate_texture;
	struct pipe_surface  *intermediate_surface;
	struct pipe_resource *vertices, *intermediate_vertices;
	double               s = src->width0;
	double               t = src->height0;

	MaybeUpdateFilter ();
	MaybeUpdateShader ();

	if (!vert_fs || !horz_fs)
		return 0;

	intermediate = new GalliumSurface (ctx->pipe,
					   src->width0,
					   src->height0);
	if (!intermediate)
		return 0;

	intermediate_texture = intermediate->Texture ();
	if (!intermediate_texture) {
		intermediate->unref ();
		return 0;
	}

	intermediate_surface =
		screen->get_tex_surface (screen,
					 intermediate_texture,
					 0,
					 0,
					 0,
					 PIPE_BIND_RENDER_TARGET);;
	if (!intermediate_surface) {
		intermediate->unref ();
		return 0;
	}

	vertices = CreateVertexBuffer (intermediate_texture, matrix,
				       x, y,
				       width, height,
				       s, t);
	if (!vertices) {
		pipe_surface_reference (&intermediate_surface, NULL);
		intermediate->unref ();
		return 0;
	}

	intermediate_vertices = CreateVertexBuffer (src, NULL,
						    0.0, 0.0,
						    s, t,
						    s, t);
	if (!intermediate_vertices) {
		pipe_resource_reference (&vertices, NULL);
		pipe_surface_reference (&intermediate_surface, NULL);
		intermediate->unref ();
		return 0;
	}

	if (cso_set_fragment_shader_handle (ctx->cso, horz_fs) != PIPE_OK) {
		pipe_resource_reference (&intermediate_vertices, NULL);
		pipe_resource_reference (&vertices, NULL);
		pipe_surface_reference (&intermediate_surface, NULL);
		intermediate->unref ();
		return 0;
	}

	struct pipe_sampler_state sampler;
	memset (&sampler, 0, sizeof (struct pipe_sampler_state));
	sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
	sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
	sampler.normalized_coords = 0;
	cso_single_sampler (ctx->cso, 0, &sampler);
	cso_single_sampler (ctx->cso, 1, &sampler);
	cso_single_sampler_done (ctx->cso);

	st_set_fragment_sampler_texture (ctx, 0, src);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, horz_pass_constant_buffer);

	struct pipe_blend_state blend;
	memset (&blend, 0, sizeof (blend));
	blend.rt[0].colormask |= PIPE_MASK_RGBA;
	blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].blend_enable = 0;
	blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_ZERO;
	blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ZERO;
	cso_set_blend (ctx->cso, &blend);

	DrawVertexBuffer (intermediate_surface, intermediate_vertices,
			  0.0, 0.0);

	if (cso_set_fragment_shader_handle (ctx->cso, vert_fs) != PIPE_OK) {
		pipe_resource_reference (&intermediate_vertices, NULL);
		pipe_resource_reference (&vertices, NULL);
		pipe_surface_reference (&intermediate_surface, NULL);
		intermediate->unref ();
		return 0;
	}

	st_set_fragment_sampler_texture (ctx, 1, intermediate_texture);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, vert_pass_constant_buffer);

	memset (&blend, 0, sizeof (blend));
	blend.rt[0].colormask |= PIPE_MASK_RGBA;
	blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].blend_enable = 1;
	blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	cso_set_blend (ctx->cso, &blend);

	DrawVertexBuffer (dst, vertices, dstX, dstY, clip);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, NULL);

	st_set_fragment_sampler_texture (ctx, 1, NULL);
	st_set_fragment_sampler_texture (ctx, 0, NULL);

	pipe_resource_reference (&intermediate_vertices, NULL);
	pipe_resource_reference (&vertices, NULL);
	pipe_surface_reference (&intermediate_surface, NULL);
	intermediate->unref ();

	cso_set_fragment_shader_handle (ctx->cso, ctx->fs);

	return 1;
#else
	return 0;
#endif

}

bool
DropShadowEffect::Render (Context      *ctx,
			  MoonSurface  *src,
			  const double *matrix,
			  double       x,
			  double       y,
			  double       width,
			  double       height)
{
	cairo_surface_t *cs = src->Cairo ();

	MaybeUpdateFilter ();

	/* table based filter code when possible */
	if (cairo_surface_get_type (cs) == CAIRO_SURFACE_TYPE_IMAGE) {
		const cairo_format_t format = CAIRO_FORMAT_ARGB32;
		cairo_surface_t      *image;

		int           pw = cairo_image_surface_get_width (cs);
		int           ph = cairo_image_surface_get_height (cs);
		int           stride = cairo_image_surface_get_stride (cs);
		unsigned char *data = (unsigned char *) g_malloc (stride * ph);
		Rect          r = Rect (x, y, width, height);
		cairo_t       *cr = ctx->Cairo ();

		double direction = GetDirection () * (M_PI / 180.0);
		double depth = CLAMP (GetShadowDepth (), 0.0, MAX_SHADOW_DEPTH);
		double dx = -cos (direction) * depth;
		double dy = sin (direction) * depth;
		Color  *color = GetColor ();
		double opacity = CLAMP (GetOpacity (), 0.0, 1.0);
		int    rgba[4];
		int    *table0 = filtertable0;
		int    **table = filtertable ? filtertable : &table0;

		rgba[SW_RED]   = (int) ((color->r * opacity) * 255.0);
		rgba[SW_GREEN] = (int) ((color->g * opacity) * 255.0);
		rgba[SW_BLUE]  = (int) ((color->b * opacity) * 255.0);
		rgba[SW_ALPHA] = (int) (opacity * 255.0);

		sw_filter_drop_shadow (cairo_image_surface_get_data (cs),
				       data,
				       pw,
				       ph,
				       stride,
				       (int) (dx + 0.5),
				       (int) (dy + 0.5),
				       nfiltervalues,
				       table,
				       rgba);

		image = cairo_image_surface_create_for_data (data,
							     format,
							     pw,
							     ph,
							     stride);

		cairo_save (cr);
		cairo_identity_matrix (cr);
		r.RoundOut ().Draw (cr);
		cairo_clip (cr);

		cairo_set_source_surface (cr, image, r.x, r.y);
		cairo_surface_destroy (image);

		cairo_paint (cr);
		cairo_restore (cr);

		g_free (data);
		cairo_surface_destroy (cs);

		return 1;
	}

	cairo_surface_destroy (cs);

	return Effect::Render (ctx, src, matrix, x, y, width, height);
}

void
DropShadowEffect::UpdateShader ()
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	Color                *color = GetColor ();
	double               direction = GetDirection () * (M_PI / 180.0);
	double               opacity = CLAMP (GetOpacity (), 0.0, 1.0);
	double               depth = CLAMP (GetShadowDepth (), 0.0, MAX_SHADOW_DEPTH);
	double               dx = -cos (direction) * depth;
	double               dy = sin (direction) * depth;
	int                  width = nfiltervalues;
	float                *horz, *vert;
	struct pipe_transfer *horz_transfer = NULL, *vert_transfer = NULL;
	int                  i;

	if (width != filter_size) {
		struct ureg_program *ureg;
		struct ureg_src     sampler, intermediate_sampler, tex, col, one, off;
		struct ureg_dst     out, shd, img, tmp;

		Clear ();

		filter_size = width;

		ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);
		if (!ureg)
			return;

		sampler = ureg_DECL_sampler (ureg, 0);

		tex = ureg_DECL_fs_input (ureg,
					  TGSI_SEMANTIC_GENERIC, 0,
					  TGSI_INTERPOLATE_LINEAR);

		out = ureg_DECL_output (ureg,
					TGSI_SEMANTIC_COLOR,
					0);

		tmp = ureg_DECL_temporary (ureg);
		off = ureg_DECL_constant (ureg, 0);

		ureg_ADD (ureg, tmp, tex, off);

		if (width)
			ureg_convolution (ureg, out, sampler, ureg_src (tmp), width, 1);
		else
			ureg_TEX (ureg, out, TGSI_TEXTURE_2D, ureg_src (tmp), sampler);

		ureg_END (ureg);

#if LOGGING
		if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_EFFECT)) {
			printf ("DropShadowEffect::UpdateShader: horizontal pass TGSI shader:\n");
			tgsi_dump (ureg_get_tokens (ureg, NULL), 0);
		}
#endif

		horz_fs = ureg_create_shader_and_destroy (ureg, ctx->pipe);
		if (!horz_fs)
			return;

		ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);
		if (!ureg) {
			Clear ();
			return;
		}

		sampler = ureg_DECL_sampler (ureg, 0);
		intermediate_sampler = ureg_DECL_sampler (ureg, 1);

		tex = ureg_DECL_fs_input (ureg,
					  TGSI_SEMANTIC_GENERIC, 0,
					  TGSI_INTERPOLATE_LINEAR);

		out = ureg_DECL_output (ureg,
					TGSI_SEMANTIC_COLOR,
					0);

		shd = ureg_DECL_temporary (ureg);
		img = ureg_DECL_temporary (ureg);
		col = ureg_DECL_constant (ureg, 0);
		one = ureg_imm4f (ureg, 1.f, 1.f, 1.f, 1.f);

		ureg_TEX (ureg, img, TGSI_TEXTURE_2D, tex, sampler);

		if (width)
			ureg_convolution (ureg, shd, intermediate_sampler, tex, width, 1);
		else
			ureg_TEX (ureg, shd, TGSI_TEXTURE_2D, tex, intermediate_sampler);

		ureg_MUL (ureg, shd, ureg_swizzle (ureg_src (shd),
						   TGSI_SWIZZLE_W,
						   TGSI_SWIZZLE_W,
						   TGSI_SWIZZLE_W,
						   TGSI_SWIZZLE_W), col);
		tmp = ureg_DECL_temporary (ureg);
		ureg_SUB (ureg, tmp, one, ureg_swizzle (ureg_src (img),
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W,
							TGSI_SWIZZLE_W));
		ureg_MAD (ureg, out, ureg_src (tmp), ureg_src (shd), ureg_src (img));
		ureg_END (ureg);

#if LOGGING
		if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_EFFECT)) {
			printf ("DropShadowEffect::UpdateShader: vertical pass TGSI shader:\n");
			tgsi_dump (ureg_get_tokens (ureg, NULL), 0);
		}
#endif

		vert_fs = ureg_create_shader_and_destroy (ureg, ctx->pipe);
		if (!vert_fs) {
			Clear ();
			return;
		}

		horz_pass_constant_buffer =
			pipe_buffer_create (ctx->pipe->screen,
					    PIPE_BIND_CONSTANT_BUFFER,
					    sizeof (float) * 4 * (width * 2 + 2));
		if (!horz_pass_constant_buffer) {
			Clear ();
			return;
		}

		vert_pass_constant_buffer =
			pipe_buffer_create (ctx->pipe->screen,
					    PIPE_BIND_CONSTANT_BUFFER,
					    sizeof (float) * 4 * (width * 2 + 2));
		if (!vert_pass_constant_buffer) {
			Clear ();
			return;
		}
	}

	horz = (float *) pipe_buffer_map (ctx->pipe,
					  horz_pass_constant_buffer,
					  PIPE_TRANSFER_WRITE,
					  &horz_transfer);
	if (!horz) {
		Clear ();
		return;
	}

	vert = (float *) pipe_buffer_map (ctx->pipe,
					  vert_pass_constant_buffer,
					  PIPE_TRANSFER_WRITE,
					  &vert_transfer);
	if (!vert) {
		pipe_buffer_unmap (ctx->pipe, horz_pass_constant_buffer, horz_transfer);
		Clear ();
		return;
	}

	*horz++ = dx;
	*horz++ = dy;
	*horz++ = 0.f;
	*horz++ = 0.f;

	*vert++ = color->r * opacity;
	*vert++ = color->g * opacity;
	*vert++ = color->b * opacity;
	*vert++ = opacity;

	if (width) {
		for (i = 1; i <= width; i++) {
			*horz++ = i;
			*horz++ = 0.f;
			*horz++ = 0.f;
			*horz++ = 1.f;

			*vert++ = 0.f;
			*vert++ = i;
			*vert++ = 0.f;
			*vert++ = 1.f;
		}

		for (i = 0; i <= width; i++) {
			*horz++ = filtervalues[i];
			*horz++ = filtervalues[i];
			*horz++ = filtervalues[i];
			*horz++ = filtervalues[i];

			*vert++ = filtervalues[i];
			*vert++ = filtervalues[i];
			*vert++ = filtervalues[i];
			*vert++ = filtervalues[i];
		}
	}

	pipe_buffer_unmap (ctx->pipe, horz_pass_constant_buffer, horz_transfer);
	pipe_buffer_unmap (ctx->pipe, vert_pass_constant_buffer, vert_transfer);
#endif

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
		const Uri *uri = GetUriSource ();
		char *path;

		g_free (tokens);
		tokens = NULL;

		if (!Uri::IsNullOrEmpty (uri) &&
		    (path = application->GetResourceAsPath (GetResourceBase (),
							    uri))) {
			SetTokensFromPath (path);
			g_free (path);
		}
		else {
			g_warning ("invalid uri: %s", uri ? uri->ToString () : "null");
		}
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
PixelShader::SetTokensFromPath (const char *path)
{
	GError *error = NULL;
	gchar  *bytes;
	gsize  nbytes;

	if (!g_file_get_contents (path,
				  (char **) &bytes,
				  &nbytes,
				  &error)) {
		g_warning ("%s", error->message);
		g_error_free (error);
		return;
	}

	g_free (tokens);
	tokens = (guint32 *) bytes;
	ntokens = nbytes / sizeof (guint32);
}

int
PixelShader::GetToken (int     index,
		       guint32 *token)
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
	return GetToken (index, (guint32 *) token);
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
	guint32 token;

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
	const d3d_op_metadata_t metadata[] = {
		{ "NOP", 0, 0 }, /* D3DSIO_NOP 0 */
		{ "MOV", 1, 1 }, /* D3DSIO_MOV 1 */
		{ "ADD", 1, 2 }, /* D3DSIO_ADD 2 */
		{ "SUB", 1, 2 }, /* D3DSIO_SUB 3 */
		{ "MAD", 1, 3 }, /* D3DSIO_MAD 4 */
		{ "MUL", 1, 2 }, /* D3DSIO_MUL 5 */
		{ "RCP", 1, 1 }, /* D3DSIO_RCP 6 */
		{ "RSQ", 1, 1 }, /* D3DSIO_RSQ 7 */
		{ "DP3", 1, 2 }, /* D3DSIO_DP3 8 */
		{ "DP4", 1, 2 }, /* D3DSIO_DP4 9 */
		{ "MIN", 1, 2 }, /* D3DSIO_MIN 10 */
		{ "MAX", 1, 2 }, /* D3DSIO_MAX 11 */
		{ "SLT", 1, 2 }, /* D3DSIO_SLT 12 */
		{ "SGE", 1, 2 }, /* D3DSIO_SGE 13 */
		{ "EXP", 1, 1 }, /* D3DSIO_EXP 14 */
		{ "LOG", 1, 1 }, /* D3DSIO_LOG 15 */
		{ "LIT", 1, 1 }, /* D3DSIO_LIT 16 */
		{ "DST", 1, 2 }, /* D3DSIO_DST 17 */
		{ "LRP", 1, 3 }, /* D3DSIO_LRP 18 */
		{ "FRC", 1, 1 }, /* D3DSIO_FRC 19 */
		{  NULL, 0, 0 }, /* D3DSIO_M4x4 20 */
		{  NULL, 0, 0 }, /* D3DSIO_M4x3 21 */
		{  NULL, 0, 0 }, /* D3DSIO_M3x4 22 */
		{  NULL, 0, 0 }, /* D3DSIO_M3x3 23 */
		{  NULL, 0, 0 }, /* D3DSIO_M3x2 24 */
		{  NULL, 0, 0 }, /* D3DSIO_CALL 25 */
		{  NULL, 0, 0 }, /* D3DSIO_CALLNZ 26 */
		{  NULL, 0, 0 }, /* D3DSIO_LOOP 27 */
		{  NULL, 0, 0 }, /* D3DSIO_RET 28 */
		{  NULL, 0, 0 }, /* D3DSIO_ENDLOOP 29 */
		{  NULL, 0, 0 }, /* D3DSIO_LABEL 30 */
		{ "DCL", 0, 0 }, /* D3DSIO_DCL 31 */
		{ "POW", 1, 2 }, /* D3DSIO_POW 32 */
		{  NULL, 0, 0 }, /* D3DSIO_CRS 33 */
		{  NULL, 0, 0 }, /* D3DSIO_SGN 34 */
		{ "ABS", 1, 1 }, /* D3DSIO_ABS 35 */
		{ "NRM", 1, 1 }, /* D3DSIO_NRM 36 */
		{ "SIN", 1, 3 }, /* D3DSIO_SINCOS 37 */
		{  NULL, 0, 0 }, /* D3DSIO_REP 38 */
		{  NULL, 0, 0 }, /* D3DSIO_ENDREP 39 */
		{  NULL, 0, 0 }, /* D3DSIO_IF 40 */
		{  NULL, 0, 0 }, /* D3DSIO_IFC 41 */
		{  NULL, 0, 0 }, /* D3DSIO_ELSE 42 */
		{  NULL, 0, 0 }, /* D3DSIO_ENDIF 43 */
		{  NULL, 0, 0 }, /* D3DSIO_BREAK 44 */
		{  NULL, 0, 0 }, /* D3DSIO_BREAKC 45 */
		{  NULL, 0, 0 }, /* D3DSIO_MOVA 46 */
		{  NULL, 0, 0 }, /* D3DSIO_DEFB 47 */
		{  NULL, 0, 0 }, /* D3DSIO_DEFI 48 */
		{  NULL, 0, 0 }, /* 49 */
		{  NULL, 0, 0 }, /* 50 */
		{  NULL, 0, 0 }, /* 51 */
		{  NULL, 0, 0 }, /* 52 */
		{  NULL, 0, 0 }, /* 53 */
		{  NULL, 0, 0 }, /* 54 */
		{  NULL, 0, 0 }, /* 55 */
		{  NULL, 0, 0 }, /* 56 */
		{  NULL, 0, 0 }, /* 57 */
		{  NULL, 0, 0 }, /* 58 */
		{  NULL, 0, 0 }, /* 59 */
		{  NULL, 0, 0 }, /* 60 */
		{  NULL, 0, 0 }, /* 61 */
		{  NULL, 0, 0 }, /* 62 */
		{  NULL, 0, 0 }, /* 63 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXCOORD 64 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXKILL 65 */
		{ "TEX", 1, 2 }, /* D3DSIO_TEX 66 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXBEM 67 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXBEML 68 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXREG2AR 69 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXREG2GB 70 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x2PAD 71 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x2TEX 72 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3PAD 73 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3TEX 74 */
		{  NULL, 0, 0 }, /* D3DSIO_RESERVED0 75 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3SPEC 76 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3VSPEC 77 */
		{  NULL, 0, 0 }, /* D3DSIO_EXPP 78 */
		{  NULL, 0, 0 }, /* D3DSIO_LOGP 79 */
		{ "CND", 1, 3 }, /* D3DSIO_CND 80 */
		{ "DEF", 0, 0 }, /* D3DSIO_DEF 81 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXREG2RGB 82 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXDP3TEX 83 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x2DEPTH 84 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXDP3 85 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXM3x3 86 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXDEPTH 87 */
		{ "CMP", 1, 3 }, /* D3DSIO_CMP 88 */
		{  NULL, 0, 0 }, /* D3DSIO_BEM 89 */
		{ "D2A", 1, 3 }, /* D3DSIO_DP2ADD 90 */
		{  NULL, 0, 0 }, /* D3DSIO_DSX 91 */
		{  NULL, 0, 0 }, /* D3DSIO_DSY 92 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXLDD 93 */
		{  NULL, 0, 0 }, /* D3DSIO_SETP 94 */
		{  NULL, 0, 0 }, /* D3DSIO_TEXLDL 95 */
		{  NULL, 0, 0 }, /* D3DSIO_BREAKP 96 */
		{  NULL, 0, 0 }  /* 97 */
	};
	guint32 token;

	if ((index = GetToken (index, &token)) < 0)
		return -1;

	value->type = (token >> D3D_OP_TYPE_SHIFT) & D3D_OP_TYPE_MASK;
	value->length = (token >> D3D_OP_LENGTH_SHIFT) & D3D_OP_LENGTH_MASK;
	value->comment_length = (token >> D3D_OP_COMMENT_LENGTH_SHIFT) &
		D3D_OP_COMMENT_LENGTH_MASK;

	if (value->type < G_N_ELEMENTS (metadata))
		value->meta = metadata[value->type];
	else
		value->meta = metadata[G_N_ELEMENTS (metadata) - 1];

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
	guint32 token;

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
	guint32 token;

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
	guint32 token;

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

ShaderEffect::ShaderEffect ()
{
	int i;

	SetObjectType (Type::SHADEREFFECT);

	fs = NULL;

	constant_buffer = NULL;

	for (i = 0; i < MAX_SAMPLERS; i++) {
		sampler_input[i] = NULL;

#ifdef USE_GALLIUM
		sampler_filter[i] = PIPE_TEX_FILTER_NEAREST;
#endif

	}
}

void
ShaderEffect::Clear ()
{

#ifdef USE_GALLIUM
	GalliumContext *ctx = st_context;

	pipe_resource_reference (&constant_buffer, NULL);

	if (fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, fs);
		fs = NULL;
	}
#endif

}

void
ShaderEffect::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::SHADEREFFECT) {
		Effect::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == ShaderEffect::PixelShaderProperty)
		need_update = true;

	NotifyListenersOfPropertyChange (args, error);
}

Thickness
ShaderEffect::Padding ()
{
	Value *left   = GetValue (ShaderEffect::PaddingLeftProperty);
	Value *top    = GetValue (ShaderEffect::PaddingTopProperty);
	Value *right  = GetValue (ShaderEffect::PaddingRightProperty);
	Value *bottom = GetValue (ShaderEffect::PaddingBottomProperty);

	return Thickness (left   ? ceil (left->AsDouble ())   : 0.0,
			  top    ? ceil (top->AsDouble ())    : 0.0,
			  right  ? ceil (right->AsDouble ())  : 0.0,
			  bottom ? ceil (bottom->AsDouble ()) : 0.0);
}

pipe_resource_t *
ShaderEffect::GetShaderConstantBuffer (float           **ptr,
				       pipe_transfer_t **ptr_transfer)
{

#ifdef USE_GALLIUM
	GalliumContext *ctx = st_context;

	if (!constant_buffer) {
		constant_buffer =
			pipe_buffer_create (ctx->pipe->screen,
					    PIPE_BIND_CONSTANT_BUFFER,
					    4 * sizeof (float) * MAX_CONSTANTS);
		if (!constant_buffer)
			return NULL;
	}

	if (ptr) {
		float *v;

		v = (float *) pipe_buffer_map (ctx->pipe,
					       constant_buffer,
					       PIPE_TRANSFER_WRITE,
					       ptr_transfer);
		if (!v) {
			if (constant_buffer)
				pipe_resource_reference (&constant_buffer, NULL);
		}

		*ptr = v;
	}

	return constant_buffer;
#else
	return NULL;
#endif

}

void
ShaderEffect::UpdateShaderConstant (int reg, double x, double y, double z, double w)
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	struct pipe_resource *constants;
	float                *v;
	struct pipe_transfer *transfer = NULL;

	if (reg >= MAX_CONSTANTS) {
		g_warning ("UpdateShaderConstant: invalid register number %d", reg);
		return;
	}

	constants = GetShaderConstantBuffer (&v, &transfer);
	if (!constants)
		return;

	v[reg * 4 + 0] = x;
	v[reg * 4 + 1] = y;
	v[reg * 4 + 2] = z;
	v[reg * 4 + 3] = w;

	pipe_buffer_unmap (ctx->pipe, constants, transfer);
#endif

}

void
ShaderEffect::UpdateShaderSampler (int reg, int mode, Brush *input)
{

#ifdef USE_GALLIUM
	if (reg >= MAX_SAMPLERS) {
		g_warning ("UpdateShaderSampler: invalid register number %d", reg);
		return;
	}

	sampler_input[reg] = input;

	switch (mode) {
		case 2:
			sampler_filter[reg] = PIPE_TEX_FILTER_LINEAR;
			break;
		default:
			sampler_filter[reg] = PIPE_TEX_FILTER_NEAREST;
			break;
	}
#endif

}

bool
ShaderEffect::Composite (pipe_surface_t  *dst,
			 pipe_resource_t *src,
			 const double    *matrix,
			 double          dstX,
			 double          dstY,
			 const Rect      *clip,
			 double          x,
			 double          y,
			 double          width,
			 double          height)
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	GalliumSurface       *input[PIPE_MAX_SAMPLERS];
	struct pipe_resource *vertices;
	struct pipe_resource *constants;
	unsigned int         i;
	Value                *ddxDdyReg;
	double               s = src->width0;
	double               t = src->height0;

	if (!fs)
		return 0;

	ddxDdyReg = GetValue (ShaderEffect::DdxUvDdyUvRegisterIndexProperty);
	if (ddxDdyReg)
		UpdateShaderConstant (ddxDdyReg->AsInt32 (),
				      1.0 / s,
				      0.0,
				      0.0,
				      1.0 / t);

	constants = GetShaderConstantBuffer (NULL, NULL);
	if (!constants)
		return 0;

	vertices = CreateVertexBuffer (src, matrix, x, y, width, height);
	if (!vertices)
		return 0;

	if (cso_set_fragment_shader_handle (ctx->cso, fs) != PIPE_OK) {
		pipe_resource_reference (&vertices, NULL);
		return 0;
	}

	struct pipe_sampler_state sampler;
	memset(&sampler, 0, sizeof(struct pipe_sampler_state));
	sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
	sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	sampler.normalized_coords = 1;

	for (i = 0; i <= sampler_last; i++) {
		sampler.min_img_filter = sampler_filter[i];
		sampler.mag_img_filter = sampler_filter[i];

		cso_single_sampler (ctx->cso, i, &sampler);
	}
	cso_single_sampler_done (ctx->cso);

	for (i = 0; i <= sampler_last; i++) {
		struct pipe_resource *sampler_texture = NULL;

		if (sampler_input[i]) {
			input[i] = new GalliumSurface (ctx->pipe,
						       src->width0,
						       src->height0);
			if (input[i]) {
				cairo_surface_t *surface = input[i]->Cairo ();
				cairo_t *cr = cairo_create (surface);
				Rect area = Rect (0.0, 0.0, s, t);

				sampler_input[i]->SetupBrush (cr, area);
				cairo_paint (cr);
				cairo_destroy (cr);
				cairo_surface_destroy (surface);

				sampler_texture = input[i]->Texture ();
			}
		}
		else {
			input[i] = NULL;
			sampler_texture = src;
		}

		if (!sampler_texture) {
			g_warning ("Composite: failed to generate input texture for sampler register %d", i);
			sampler_texture = src;
		}

		st_set_fragment_sampler_texture (ctx, i, sampler_texture);
	}

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, constants);

	struct pipe_blend_state blend;
	memset (&blend, 0, sizeof (blend));
	blend.rt[0].colormask |= PIPE_MASK_RGBA;
	blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].blend_enable = 1;
	blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	cso_set_blend (ctx->cso, &blend);

	DrawVertexBuffer (dst, vertices, dstX, dstY, clip);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, NULL);

	for (i = 0; i <= sampler_last; i++)
		st_set_fragment_sampler_texture (ctx, i, NULL);

	for (i = 0; i <= sampler_last; i++)
		if (input[i])
			input[i]->unref ();

	pipe_resource_reference (&vertices, NULL);

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

#ifdef USE_GALLIUM
static INLINE bool
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
#endif

#define ERROR_IF(EXP)							\
	do { if (EXP) {							\
			ShaderError ("Shader error (" #EXP ") at "	\
				     "instruction %.2d", n);		\
			ureg_destroy (ureg); return; }			\
	} while (0)

void
ShaderEffect::UpdateShader ()
{

#ifdef USE_GALLIUM
	PixelShader         *ps = GetPixelShader ();
	GalliumContext      *ctx = st_context;
	struct ureg_program *ureg;
	d3d_version_t       version;
	d3d_op_t            op;
	int                 index;
	struct ureg_src     src_reg[D3DSPR_LAST][MAX_REGS];
	struct ureg_dst     dst_reg[D3DSPR_LAST][MAX_REGS];
	int                 n = 0;

	if (fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, fs);
		fs = NULL;
	}

	sampler_last = 0;

	if (!ps)
		return;

	if ((index = ps->GetVersion (0, &version)) < 0)
		return;

	if (version.type  != 0xffff ||
	    version.major != 2      ||
	    version.minor != 0) {
		ShaderError ("Unsupported pixel shader");
		return;
	}

	ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);
	if (!ureg)
		return;

	for (int i = 0; i < D3DSPR_LAST; i++) {
		for (int j = 0; j < MAX_REGS; j++) {
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
				ERROR_IF (def.reg.regnum >= MAX_REGS);

				src_reg[def.reg.regtype][def.reg.regnum] =
					ureg_DECL_immediate (ureg, def.v, 4);
			} break;
			case D3DSIO_DCL: {
				d3d_dcl_instruction_t dcl;

				i = ps->GetInstruction (i, &dcl);

				ERROR_IF (dcl.reg.dstmod != 0);
				ERROR_IF (dcl.reg.regnum >= MAX_REGS);
				ERROR_IF (dcl.reg.regnum >= MAX_SAMPLERS);
				ERROR_IF (dcl.reg.regtype != D3DSPR_SAMPLER &&
					  dcl.reg.regtype != D3DSPR_TEXTURE);

				switch (dcl.reg.regtype) {
					case D3DSPR_SAMPLER:
						src_reg[D3DSPR_SAMPLER][dcl.reg.regnum] =
							ureg_DECL_sampler (ureg, dcl.reg.regnum);
						sampler_last = MAX (sampler_last, dcl.reg.regnum);
						break;
					case D3DSPR_TEXTURE:
						src_reg[D3DSPR_TEXTURE][dcl.reg.regnum] =
							ureg_DECL_fs_input (ureg,
									    TGSI_SEMANTIC_GENERIC,
									    dcl.reg.regnum,
									    TGSI_INTERPOLATE_LINEAR);
						sampler_last = MAX (sampler_last, dcl.reg.regnum);
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

					ERROR_IF (reg.regnum >= MAX_REGS);
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

					ERROR_IF (src.regnum >= MAX_REGS);
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
					ShaderError ("Unknown shader instruction %.2d", n);
					ureg_destroy (ureg);
					return;
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

#if LOGGING
				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_EFFECT)) {
					printf ("ShaderEffect::UpdateShader: Direct3D shader:\n");
					ShaderError (NULL);
					printf ("ShaderEffect::UpdateShader: TGSI shader:\n");
					tgsi_dump (ureg_get_tokens (ureg, NULL), 0);
				}
#endif

				fs = ureg_create_shader_and_destroy (ureg, ctx->pipe);
				return;
			default:
				break;
		}

		for (unsigned k = 0; k < op.meta.nsrcparam; k++)
			if (!ureg_dst_is_undef (src_tmp[k]))
				ureg_release_temporary (ureg, src_tmp[k]);
	}

	ShaderError ("Incomplete pixel shader");
#endif

}

static inline void
d3d_print_regtype (unsigned int type)
{
	const char *type_str[] = {
		"TEMP",
		"INPUT",
		"CONST",
		"TEX",
		"ROUT",
		"AOUT",
		"OUT",
		"CTINT",
		"COUT",
		"DOUT",
		"SAMP",
		"CONS2",
		"CONS3",
		"CONS4",
		"CBOOL",
		"LOOP",
		"TF16",
		"MISC",
		"LABEL",
		"PRED"
	};

	if (type >= G_N_ELEMENTS (type_str))
		printf ("0x%x", type);

	printf ("%s", type_str[type]);
}

static void
d3d_print_srcmod (unsigned int mod)
{
	const char *srcmod_str[] = {
		"",
		"-",
		"bias ",
		"-bias ",
		"sign ",
		"-sign ",
		"comp ",
		"pow ",
		"npow ",
		"dz ",
		"dw ",
		"abs ",
		"-abs ",
		"not "
	};

	if (mod >= G_N_ELEMENTS (srcmod_str))
		printf ("0x%x ", (int) mod);
	else
		printf ("%s", srcmod_str[mod]);
}

static void
d3d_print_src_param (d3d_source_parameter_t *src)
{
	const char *swizzle_str[] = { "x", "y", "z", "w" };

	d3d_print_srcmod (src->srcmod);
	d3d_print_regtype (src->regtype);
	printf ("[%d]", src->regnum);
	if (src->swizzle.x != 0 ||
	    src->swizzle.y != 1 ||
	    src->swizzle.z != 2 ||
	    src->swizzle.w != 3)
		printf (".%s%s%s%s",
			swizzle_str[src->swizzle.x],
			swizzle_str[src->swizzle.y],
			swizzle_str[src->swizzle.z],
			swizzle_str[src->swizzle.w]);
}

static void
d3d_print_dstmod (unsigned int mod)
{
	const char *dstmod_str[] = {
		"",
		"_SAT",
		"_PRT",
		"_CNT"
	};

	if (mod >= G_N_ELEMENTS (dstmod_str))
		printf ("_0x%x ", mod);
	else
		printf ("%s ", dstmod_str[mod]);
}

static void
d3d_print_dst_param (d3d_destination_parameter_t *dst)
{
	d3d_print_dstmod (dst->dstmod);
	d3d_print_regtype (dst->regtype);
	printf ("[%d]", dst->regnum);
	if (dst->writemask != 0xf)
		printf (".%s%s%s%s",
			dst->writemask & 0x1 ? "x" : "",
			dst->writemask & 0x2 ? "y" : "",
			dst->writemask & 0x4 ? "z" : "",
			dst->writemask & 0x8 ? "w" : "");
}

void
ShaderEffect::ShaderError (const char *format, ...)
{
	PixelShader   *ps = GetPixelShader ();
	d3d_version_t version;
	d3d_op_t      op;
	int           i;
	int           n = 0;

	if (format) {
		va_list ap;

		printf ("Moonlight: ");
		va_start (ap, format);
		vprintf (format, ap);
		va_end (ap);
		printf (":\n");
	}

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
		d3d_source_parameter_t      src;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		switch (op.type) {
			case D3DSIO_DEF: {
				d3d_def_instruction_t def;

				if (ps->GetInstruction (i, &def) != 1) {
					printf ("%s ", op.meta.name);
					d3d_print_dst_param (&def.reg);
					printf (" { %10.4f, %10.4f, %10.4f, %10.4f }\n",
						def.v[0], def.v[1], def.v[2],
						def.v[3]);
				}
			} break;
			case D3DSIO_DCL: {
				d3d_dcl_instruction_t dcl;

				if (ps->GetInstruction (i, &dcl) != -1) {
					printf ("%s", op.meta.name);
					d3d_print_dst_param (&dcl.reg);
					printf ("\n");
				}
			} break;
			case D3DSIO_END:
				printf ("%3d: END\n", n + 1);
				return;
			default: {
				unsigned ndstparam = op.meta.ndstparam;
				unsigned nsrcparam = op.meta.nsrcparam;
				int      j = i;

				n++;

				if (op.meta.name)
					printf ("%3d: %s", n, op.meta.name);
				else
					printf ("%3d: %d", n, op.type);

				while (ndstparam--) {
					j = ps->GetDestinationParameter (j, &reg);
					d3d_print_dst_param (&reg);
				}

				if (nsrcparam--) {
					printf (", ");
					j = ps->GetSourceParameter (j, &src);
					d3d_print_src_param (&src);
				}

				while (nsrcparam--) {
					j = ps->GetSourceParameter (j, &src);
					printf (", ");
					d3d_print_src_param (&src);
				}

				printf ("\n");
			} break;
		}

		i += op.length;
	}
}

TransformEffect::TransformEffect ()
{
	SetObjectType (Type::TRANSFORMEFFECT);
	fs = NULL;
	opacity = 1.0;
	constant_buffer = NULL;
	type = PERSPECTIVE;
}

TransformEffect::~TransformEffect ()
{
	Clear ();
}

void
TransformEffect::Clear ()
{

#ifdef USE_GALLIUM
	GalliumContext *ctx = st_context;

	pipe_resource_reference (&constant_buffer, NULL);

	if (fs) {
		ctx->pipe->delete_fs_state (ctx->pipe, fs);
		fs = NULL;
	}
#endif

}

bool
TransformEffect::Render (Context      *ctx,
			 MoonSurface  *src,
			 const double *matrix,
			 double       x,
			 double       y,
			 double       width,
			 double       height)
{
	cairo_surface_t *cs = src->Cairo ();

	if (cairo_surface_get_type (cs)         == CAIRO_SURFACE_TYPE_IMAGE &&
	    cairo_image_surface_get_width (cs)  == width &&
	    cairo_image_surface_get_height (cs) == height) {
		int x0, y0;

		if (Matrix3D::IsIntegerTranslation (matrix, &x0, &y0)) {
			cairo_t *cr = ctx->Cairo ();
			Rect    r = Rect (x, y, width, height);

			cairo_save (cr);
			cairo_identity_matrix (cr);
			r.Transform (matrix).RoundOut ().Draw (cr);
			cairo_clip (cr);
			cairo_translate (cr, x0, y0);
			cairo_set_source_surface (cr, cs, r.x, r.y);
			cairo_paint_with_alpha (cr, opacity);
			cairo_restore (cr);
			cairo_surface_destroy (cs);

			return 1;
		}
	}

	cairo_surface_destroy (cs);

	return Effect::Render (ctx, src, matrix, x, y, width, height);
}

void
TransformEffect::SetType (int value)
{
	if (type != value)
		need_update = true;

	type = value;
}

void
TransformEffect::SetOpacity (double value)
{
	if (IS_TRANSLUCENT (opacity) != IS_TRANSLUCENT (value))
		need_update = true;

	if (opacity == value)
		return;

	opacity = value;

#ifdef USE_GALLIUM
	if (constant_buffer) {
		GalliumContext       *ctx = st_context;
		float                *v;
		struct pipe_transfer *transfer = NULL;

		v = (float *) pipe_buffer_map (ctx->pipe,
					       constant_buffer,
					       PIPE_TRANSFER_WRITE,
					       &transfer);
		if (v) {
			v[0] = opacity;
			v[1] = opacity;
			v[2] = opacity;
			v[3] = opacity;

			pipe_buffer_unmap (ctx->pipe,
					   constant_buffer,
					   transfer);
		}
	}
#endif

}

bool
TransformEffect::Composite (pipe_surface_t  *dst,
			    pipe_resource_t *src,
			    const double    *matrix,
			    double          dstX,
			    double          dstY,
			    const Rect      *clip,
			    double          x,
			    double          y,
			    double          width,
			    double          height)

{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	struct pipe_resource *vertices;

	vertices = CreateVertexBuffer (src, matrix, x, y, width, height);
	if (!vertices)
		return 0;

	struct pipe_sampler_state sampler;
	memset (&sampler, 0, sizeof (struct pipe_sampler_state));
	sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_BORDER;
	sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
	sampler.min_img_filter = PIPE_TEX_FILTER_LINEAR;
	sampler.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
	sampler.normalized_coords = 1;
	cso_single_sampler (ctx->cso, 0, &sampler);
	cso_single_sampler_done (ctx->cso);

	st_set_fragment_sampler_texture (ctx, 0, src);

	struct pipe_blend_state blend;
	memset (&blend, 0, sizeof (blend));
	blend.rt[0].colormask |= PIPE_MASK_RGBA;
	blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
	blend.rt[0].blend_enable = 1;
	blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
	cso_set_blend (ctx->cso, &blend);

	if (cso_set_fragment_shader_handle (ctx->cso, fs) != PIPE_OK) {
		pipe_resource_reference (&vertices, NULL);
		return 0;
	}

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, constant_buffer);

	DrawVertexBuffer (dst, vertices, dstX, dstY, clip);

	ctx->pipe->set_constant_buffer (ctx->pipe,
					PIPE_SHADER_FRAGMENT,
					0, NULL);

	cso_set_fragment_shader_handle (ctx->cso, ctx->fs);

	st_set_fragment_sampler_texture (ctx, 0, NULL);
	pipe_resource_reference (&vertices, NULL);

	return 1;
#else
	return 0;
#endif

}

#ifdef USE_GALLIUM
static int
tgsi_interpolate_type (int type)
{
	switch (type) {
		case TransformEffect::PERSPECTIVE:
			return TGSI_INTERPOLATE_PERSPECTIVE;
		default:
			return TGSI_INTERPOLATE_LINEAR;
	}
}
#endif

void
TransformEffect::UpdateShader ()
{

#ifdef USE_GALLIUM
	GalliumContext       *ctx = st_context;
	float                *v;
	struct pipe_transfer *transfer = NULL;
	struct ureg_program  *ureg;
	struct ureg_src      tex, sampler;
	struct ureg_dst      out;

	Clear ();

	ureg = ureg_create (TGSI_PROCESSOR_FRAGMENT);
	if (!ureg)
		return;

	sampler = ureg_DECL_sampler (ureg, 0);

	tex = ureg_DECL_fs_input (ureg,
				  TGSI_SEMANTIC_GENERIC, 0,
				  tgsi_interpolate_type (type));

	out = ureg_DECL_output (ureg,
				TGSI_SEMANTIC_COLOR,
				0);

	if (IS_TRANSLUCENT (opacity))
		constant_buffer =
			pipe_buffer_create (ctx->pipe->screen,
					    PIPE_BIND_CONSTANT_BUFFER,
					    4 * sizeof (float));

	if (constant_buffer) {
		struct ureg_src alpha;
		struct ureg_dst tmp;

		tmp   = ureg_DECL_temporary (ureg);
		alpha = ureg_DECL_constant (ureg, 0);

		ureg_TEX (ureg, tmp, TGSI_TEXTURE_2D, tex, sampler);
		ureg_MUL (ureg, out, ureg_src (tmp), alpha);

		v = (float *) pipe_buffer_map (ctx->pipe,
					       constant_buffer,
					       PIPE_TRANSFER_WRITE,
					       &transfer);
		if (v) {
			v[0] = opacity;
			v[1] = opacity;
			v[2] = opacity;
			v[3] = opacity;

			pipe_buffer_unmap (ctx->pipe,
					   constant_buffer,
					   transfer);
		}
	}
	else {
		ureg_TEX (ureg, out, TGSI_TEXTURE_2D, tex, sampler);
	}

	ureg_END (ureg);

#if LOGGING
	if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_EFFECT)) {
		printf ("TransformEffect::UpdateShader: TGSI shader:\n");
		tgsi_dump (ureg_get_tokens (ureg, NULL), 0);
	}
#endif

	fs = ureg_create_shader_and_destroy (ureg, ctx->pipe);
#endif

}
