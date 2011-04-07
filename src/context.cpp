/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <stdlib.h>

#include "context.h"
#include "projection.h"
#include "cpu.h"
#include "yuv-converter.h"
#include "effect.h"

namespace Moonlight {

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

	g_free (tmp_data);
}

#define MIN_X -32768
#define MIN_Y MIN_X
#define MAX_W 65536
#define MAX_H MAX_W

Context::Cairo::Cairo ()
{
	r = Rect (MIN_X, MIN_Y, MAX_W, MAX_H);
}

Context::Target::Target ()
{
	native = NULL;
	box    = Rect ();
	cairo  = NULL;
	init   = NULL;
}

Context::Target::Target (MoonSurface *moon,
			 Rect        extents)
{
	native = moon->ref ();
	box    = extents;
	cairo  = NULL;
	init   = moon->ref ();
}

Context::Target::~Target ()
{
	native->unref ();

	if (cairo)
		cairo->unref ();

	if (init)
		init->unref ();
}

Rect
Context::Target::GetData (MoonSurface **ref)
{
	if (ref)
		*ref = native->ref ();

	return box;
}

cairo_surface_t *
Context::Target::Cairo ()
{
	cairo_surface_t *surface;

	if (cairo)
		return cairo->Cairo ();

	surface = native->Cairo ();

	/* set device offset */
	if (!box.IsEmpty ())
		cairo_surface_set_device_offset (surface, -box.x, -box.y);

	return surface;
}

void
Context::Target::SetCairoTarget (Target *target)
{
	Target *old = cairo;

	if (target)
		cairo = (Target *) target->ref ();
	else
		cairo = NULL;

	if (old)
		old->unref ();
}

Context::Target *
Context::Target::GetCairoTarget ()
{
	return cairo;
}

void
Context::Target::SetInit (MoonSurface *src)
{
	MoonSurface *old = init;

	if (src)
		init = src->ref ();
	else
		init = NULL;

	if (old)
		old->unref ();
}

MoonSurface *
Context::Target::GetInit ()
{
	return init;
}

Context::Node::Node (Target         *surface,
		     cairo_matrix_t *matrix,
		     const Rect     *clip)
{
	target    = (Target *) surface->ref ();
	box       = clip ? *clip : Rect (MIN_X, MIN_Y, MAX_W, MAX_H);
	transform = *matrix;
	context   = NULL;
}

Context::Node::~Node ()
{
	if (context) {
		cairo_surface_flush (cairo_get_target (context));
		cairo_destroy (context);
	}

	if (target)
		target->unref ();
}

cairo_t *
Context::Node::Cairo ()
{
	Target *surface = GetTarget ();

	if (!context) {
		cairo_surface_t *dst;

		if (box.IsEmpty ())
			dst = cairo_image_surface_create (CAIRO_FORMAT_A1, 1, 1);
		else
			dst = surface->Cairo ();

		context = cairo_create (dst);
		box.Draw (context);
		cairo_clip (context);
		cairo_set_matrix (context, &transform);

		cairo_surface_destroy (dst);
	}

	return context;
}

void
Context::Node::GetMatrix (cairo_matrix_t *matrix)
{
	*matrix = transform;
}

void
Context::Node::GetClip (Rect *clip)
{
	*clip = box;
}

Context::Target *
Context::Node::GetTarget ()
{
	return target;
}

void
Context::Cache::Release ()
{
	GList *contexts = this->contexts;
	this->contexts = NULL;

	for (GList *l = g_list_first (contexts); l; l = l->next)
		static_cast<Context *> (l->data)->Remove (this)->unref ();

	g_list_free (contexts);
}

Context::Context ()
{
	cache = g_hash_table_new (g_direct_hash, g_direct_equal);

	g_assert (posix_memalign ((void **)(&rgb_uv), 16, 96) == 0);
	have_mmx = CPU::HaveMMX ();
	have_sse2 = CPU::HaveSSE2 ();
}

Context::Context (MoonSurface *surface)
{
	AbsoluteTransform transform = AbsoluteTransform ();
	Rect              r = Rect (0, 0, 32768, 32768);
	Target            *target;

	target = new Target (surface, r);
	Stack::Push (new Context::Node (target, &transform.m, NULL));
	target->unref ();

	cache = g_hash_table_new (g_direct_hash, g_direct_equal);

	g_assert (posix_memalign ((void **)(&rgb_uv), 16, 96) == 0);
	have_mmx = CPU::HaveMMX ();
	have_sse2 = CPU::HaveSSE2 ();
}

Context::~Context ()
{
	GHashTableIter iter;
	gpointer       k, v;

	g_hash_table_iter_init (&iter, cache);
	while (g_hash_table_iter_next (&iter, &k, &v)) {
		Cache       *key = static_cast<Cache *> (k);
		MoonSurface *surface = static_cast<MoonSurface *> (v);
		
		key->contexts = g_list_remove (key->contexts, this);
		surface->unref ();
	}
	g_hash_table_destroy (cache);

	free (rgb_uv);
}

void
Context::Replace (Cache *key, MoonSurface *surface)
{
	MoonSurface *old = Lookup (key);

	g_hash_table_replace (cache, key, surface->ref ());

	if (old)
		old->unref ();
	else
		key->contexts = g_list_append (key->contexts, this);
}

MoonSurface *
Context::Remove (Cache *key)
{
	MoonSurface *surface = Lookup (key);

	if (surface) {
		g_hash_table_remove (cache, key);
		key->contexts = g_list_remove (key->contexts, this);
	}

	return surface;
}

MoonSurface *
Context::Lookup (Cache *key)
{
	return static_cast<MoonSurface *> (g_hash_table_lookup (cache, key));
}

void
Context::Push (Transform transform)
{
	cairo_matrix_t matrix;
	Rect           box;

	Top ()->GetMatrix (&matrix);
	Top ()->GetClip (&box);

	cairo_matrix_multiply (&matrix, &transform.m, &matrix);

	Stack::Push (new Context::Node (Top ()->GetTarget (), &matrix, &box));
}

void
Context::Push (AbsoluteTransform transform)
{
	cairo_matrix_t matrix = transform.m;
	Rect           box;

	Top ()->GetClip (&box);

	Stack::Push (new Context::Node (Top ()->GetTarget (), &matrix, &box));
}

void
Context::Push (Clip clip)
{
	cairo_matrix_t matrix;
	Rect           box;

	Top ()->GetMatrix (&matrix);
	Top ()->GetClip (&box);

	box = box.Intersection (clip.r);

	Stack::Push (new Context::Node (Top ()->GetTarget (), &matrix, &box));
}

void
Context::Push (Group extents)
{
	g_warning ("Context::Push has been called. The derived class should have overridden it.");
	Push (Clip ());
}

void
Context::Push (Group extents, MoonSurface *surface)
{
	cairo_matrix_t matrix;
	Target         *target = new Target (surface, extents.r);

	Top ()->GetMatrix (&matrix);

	Stack::Push (new Context::Node (target, &matrix, &extents.r));
	target->unref ();
}

cairo_t *
Context::Push (Cairo extents)
{
	Push (Clip (extents.r));
	return Top ()->Cairo ();
}

Context::Node *
Context::Top ()
{
	return (Node *) Stack::Top ();
}

void
Context::Pop ()
{
	delete Stack::Pop ();
}

Rect
Context::Pop (MoonSurface **ref)
{
	Node *node = (Node *) Stack::Pop ();
	Rect r = Rect ();

	if (Top ()->GetTarget () != node->GetTarget ())
		r = node->GetTarget ()->GetData (ref);

	delete node;

	return r;
}

bool
Context::IsImmutable ()
{
	Rect box;

	Top ()->GetClip (&box);

	return box.IsEmpty ();
}

void
Context::GetMatrix (double *out)
{
	Context::Target *target = Top ()->GetTarget ();
	Rect            r = target->GetData (NULL);
	cairo_matrix_t  ctm;

	Top ()->GetMatrix (&ctm);

	Matrix3D::Affine (out,
			  ctm.xx, ctm.xy,
			  ctm.yx, ctm.yy,
			  ctm.x0, ctm.y0);
}

void
Context::GetDeviceMatrix (double *out)
{
	Context::Target *target = Top ()->GetTarget ();
	Rect            r = target->GetData (NULL);
	double          viewport[16];
	double          m[16];

	GetMatrix (m);

	Matrix3D::Translate (viewport, -r.x, -r.y, 0.0);
	Matrix3D::Multiply (out, m, viewport);
}

int
Context::ComputeGaussianSamples (double radius,
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

int **
Context::CreateFilterTable (double radius,
			    int    *size)
{
	int    **table;
	double values[MAX_BLUR_RADIUS + 1];
	int    n;

	n = ComputeGaussianSamples (radius, 1.0 / 256.0, values);
	if (n) {
		int  entries = n * 2 + 1;
		int  ptr_size = sizeof (int *) * entries;
		int  value_size = sizeof (int) * 256;
		int  data_size = value_size * entries;
		char *bytes;

		bytes = (char *) g_malloc (ptr_size + data_size);
		table = (int **) bytes;

		for (int i = 0; i < entries; i++)
			table[i] = (int *) (bytes + ptr_size + i * value_size);
	}
	else {
		int  ptr_size = sizeof (int *);
		int  data_size = sizeof (int) * 256;
		char *bytes;

		bytes = (char *) g_malloc (ptr_size + data_size);
		table = (int **) bytes;

		table[0] = (int *) (bytes + ptr_size);

		for (int i = 0; i < 256; i++)
			table[0][i] = i << 16;

		*size = n;
		return table;
	}

	for (int j = 0; j < 256; j++) {
		int *center = table[n];

		center[j] = (int) (values[0] * (double) (j << 16));
	}

	for (int i = 1; i <= n; i++) {
		int *left  = table[n - i];
		int *right = table[n + i];

		for (int j = 0; j < 256; j++)
			left[j] = right[j] = (int)
				(values[i] * (double) (j << 16));
	}

	*size = n;
	return table;
}

void
Context::Clear (Color *color)
{
	cairo_t *cr = Context::Push (Cairo ());

	cairo_save (cr);
	cairo_set_operator (cr , CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
	cairo_paint (cr);
	cairo_restore (cr);

	Pop ();
}

void
Context::Blit (unsigned char *data,
	       int           stride)
{
	g_warning ("Context::Blit has been called. The derived class should have overridden it.");
}

void
Context::BlitYV12 (unsigned char *data[],
		   int           stride[])
{
	Context::Target *target = Top ()->GetTarget ();
	Rect            r = target->GetData (NULL);
	guint8          *rgb_buffer;
	gint32          rgb_stride = r.width * 4;

	if (rgb_stride % 64) {
		int remain = rgb_stride % 64;
		rgb_stride += 64 - remain;
	}

	if (posix_memalign ((void **) (&rgb_buffer), 16, r.height * rgb_stride)) {
		g_warning ("Could not allocate memory for video RGB buffer");
		return;
	}

	YUVConverter::YV12ToBGRA (data,
				  stride,
				  rgb_stride >> 2,
				  (int) r.height,
				  rgb_buffer,
				  rgb_stride,
				  rgb_uv,
				  have_mmx,
				  have_sse2);

	Blit (rgb_buffer, rgb_stride);

	free (rgb_buffer);
}

void
Context::Blend (Color *color)
{
	cairo_t *cr = Context::Push (Cairo ());

	cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
	cairo_paint (cr);

	Pop ();
}

void
Context::Blend (MoonSurface *src,
		double      alpha,
		double      x,
		double      y)
{
	cairo_surface_t *surface = src->Cairo ();
	cairo_t         *cr = Context::Push (Cairo ());

	cairo_set_source_surface (cr, surface, x, y);
	cairo_paint_with_alpha (cr, alpha);
	cairo_surface_destroy (surface);

	Pop ();
}

void
Context::Project (MoonSurface  *src,
		  const double *matrix,
		  double       alpha,
		  double       x,
		  double       y)
{
	g_warning ("Context::Project has been called. The derived class should have overridden it.");
}

void
Context::Blur (MoonSurface *src,
	       double      radius,
	       double      x,
	       double      y)
{
	const cairo_format_t format = CAIRO_FORMAT_ARGB32;
	cairo_surface_t      *surface = src->Cairo ();
	cairo_t              *cr = Push (Context::Cairo ());
	unsigned char        *data;
	int                  width, height, stride, n;
	int                  **table;

	g_assert (cairo_surface_get_type (surface) ==
		  CAIRO_SURFACE_TYPE_IMAGE);

	table = CreateFilterTable (radius, &n);

	width  = cairo_image_surface_get_width (surface);
	height = cairo_image_surface_get_height (surface);
	stride = cairo_image_surface_get_stride (surface);
	data   = (unsigned char *) g_malloc (stride * height);

	if (n) {
		cairo_surface_t *image;

		sw_filter_blur (cairo_image_surface_get_data (surface),
				data,
				width,
				height,
				stride,
				n,
				table);

		image = cairo_image_surface_create_for_data (data,
							     format,
							     width,
							     height,
							     stride);

		cairo_set_source_surface (cr, image, x, y);
		cairo_surface_destroy (image);
	}
	else {
		cairo_set_source_surface (cr, surface, x, y);
	}

	cairo_paint (cr);

	g_free (data);
	cairo_surface_destroy (surface);
	g_free (table);

	Pop ();
}

void
Context::DropShadow (MoonSurface *src,
		     double      dx,
		     double      dy,
		     double      radius,
		     Color       *color,
		     double      x,
		     double      y)
{
	const cairo_format_t format = CAIRO_FORMAT_ARGB32;
	cairo_surface_t      *surface = src->Cairo ();
	cairo_surface_t      *image;
	cairo_t              *cr = Push (Context::Cairo ());
	unsigned char        *data;
	int                  width, height, stride, n;
	int                  **table;
	int                  rgba[4];

	g_assert (cairo_surface_get_type (surface) ==
		  CAIRO_SURFACE_TYPE_IMAGE);

	table = CreateFilterTable (radius, &n);

	width  = cairo_image_surface_get_width (surface);
	height = cairo_image_surface_get_height (surface);
	stride = cairo_image_surface_get_stride (surface);
	data   = (unsigned char *) g_malloc (stride * height);
	
	rgba[SW_RED]   = (int) (color->r * 255.0);
	rgba[SW_GREEN] = (int) (color->g * 255.0);
	rgba[SW_BLUE]  = (int) (color->b * 255.0);
	rgba[SW_ALPHA] = (int) (color->a * 255.0);

	sw_filter_drop_shadow (cairo_image_surface_get_data (surface),
			       data,
			       width,
			       height,
			       stride,
			       (int) (dx + 0.5),
			       (int) (dy + 0.5),
			       n,
			       table,
			       rgba);

	image = cairo_image_surface_create_for_data (data,
						     format,
						     width,
						     height,
						     stride);

	cairo_set_source_surface (cr, image, x, y);
	cairo_surface_destroy (image);

	cairo_paint (cr);

	g_free (data);
	cairo_surface_destroy (surface);
	g_free (table);

	Pop ();
}

void
Context::ShaderEffect (MoonSurface *src,
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
	g_warning ("Context::ShaderEffect has been called. The derived class should have overridden it.");
}

void
Context::Flush ()
{
	g_warning ("Context::Flush has been called. The derived class should have overridden it.");
}

};
