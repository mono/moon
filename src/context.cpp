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

namespace Moonlight {

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
Context::Blur (MoonSurface  *src,
	       double       radius,
	       double       x,
	       double       y)
{
	g_warning ("Context::Blur has been called. The derived class should have overridden it.");
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
	g_warning ("Context::DropShadow has been called. The derived class should have overridden it.");
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
