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

#include "context.h"

namespace Moonlight {

#define MIN_X -32768
#define MIN_Y MIN_X
#define MAX_W 65536
#define MAX_H MAX_W

Context::Surface::Surface (MoonSurface *moon)
{
	native  = moon->ref ();
	offset  = Point (NAN, NAN);
	surface = NULL;
}

Context::Surface::Surface (MoonSurface *moon,
			   double      xOffset,
			   double      yOffset)
{
	native  = moon->ref ();
	offset  = Point (xOffset, yOffset);
	surface = NULL;
}

Context::Surface::~Surface ()
{
	if (surface)
		cairo_surface_destroy (surface);

	native->unref ();
}

Context::Surface *
Context::Surface::Similar (Rect r)
{
	MoonSurface      *moon = native->Similar (r.width, r.height);
	Context::Surface *cs = new Surface (moon, -r.x, -r.y);

	moon->unref ();

	return cs;
}

MoonSurface *
Context::Surface::Native ()
{
	return native->ref ();
}

cairo_surface_t *
Context::Surface::Cairo ()
{
	if (!surface) {
		surface = native->Cairo ();

		if (!isnan (offset.x) && !isnan (offset.y))
			cairo_surface_set_device_offset (surface,
							 offset.x,
							 offset.y);
	}

	return cairo_surface_reference (surface);
}

Context::Node::Node (Surface        *surface,
		     cairo_matrix_t *matrix,
		     const Rect     *clip)
{
	box       = clip ? *clip : Rect (MIN_X, MIN_Y, MAX_W, MAX_H);
	transform = *matrix;
	context   = NULL;
	target    = (Surface *) surface->ref ();
	data      = NULL;
}

Context::Node::Node (Rect           extents,
		     cairo_matrix_t *matrix)
{
	box       = extents;
	transform = *matrix;
	context   = NULL;
	target    = NULL;
	data      = NULL;
}

Context::Node::~Node ()
{
	if (context)
		cairo_destroy (context);

	if (target)
		target->unref ();

	if (data)
		data->unref ();
}

cairo_t *
Context::Node::Cairo ()
{
	Surface *surface = GetSurface ();

	if (!surface)
		return NULL;

	if (!context) {
		cairo_surface_t *dst = surface->Cairo ();

		context = cairo_create (dst);
		box.RoundOut ().Draw (context);
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

Context::Surface *
Context::Node::GetSurface ()
{
	if (!target) {
		if (!GetData (NULL))
			return NULL;
	}

	return target;
}

MoonSurface *
Context::Node::GetData (Rect *extents)
{
	Rect r = box.RoundOut ();

	if (!data) {
		Surface *base;

		if (target)
			return NULL;

		if (!prev) {
			g_warning ("ContextNode::GetData no base node.");
			return NULL;
		}

		base = ((Context::Node *) prev)->GetSurface ();
		if (!base) {
			g_warning ("ContextNode::GetData no base surface.");
			return NULL;
		}

		target = base->Similar (r);
		if (!target) {
			g_warning ("ContextNode::GetData failed.");
			return NULL;
		}

		data = target->Native ();
	}

	if (extents)
		*extents = r;

	return data;
}

void
Context::Node::SetData (MoonSurface *source)
{
	MoonSurface *ref;

	if (target) {
		g_warning ("ContextNode::SetData target surface present.");
		return;
	}

	ref = source->ref ();

	if (data)
		data->unref ();

	data = ref;
}

bool
Context::Node::Readonly (void)
{
	return GetSurface () ? false : true;
}

Context::Context (MoonSurface *surface)
{
	Surface        *cs = new Surface (surface);
	cairo_matrix_t matrix;

	cairo_matrix_init_identity (&matrix);
	Stack::Push (new Context::Node (cs, &matrix, NULL));
	cs->unref ();
}

Context::Context (MoonSurface *surface, cairo_matrix_t *transform)
{
	Surface *cs = new Surface (surface);

	Stack::Push (new Context::Node (cs, transform, NULL));
	cs->unref ();
}

void
Context::Push (cairo_matrix_t *transform)
{
	cairo_matrix_t matrix;
	Rect           box;

	Top ()->GetMatrix (&matrix);
	Top ()->GetClip (&box);

	cairo_matrix_multiply (&matrix, transform, &matrix);

	Stack::Push (new Context::Node (Top ()->GetSurface (), &matrix, &box));
}

void
Context::Push (Clip clip)
{
	cairo_matrix_t matrix;
	Rect           box;

	Top ()->GetMatrix (&matrix);
	Top ()->GetClip (&box);

	box = box.Intersection (clip.r);

	Stack::Push (new Context::Node (Top ()->GetSurface (), &matrix, &box));
}

void
Context::Push (Rect extents)
{
	cairo_matrix_t matrix;

	Top ()->GetMatrix (&matrix);
	Stack::Push (new Context::Node (extents, &matrix));
}

void
Context::Push (Rect extents, cairo_matrix_t *matrix)
{
	Stack::Push (new Context::Node (extents, matrix));
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
	Node *node;
	Rect extents = Rect ();

	node = (Node *) Stack::Pop ();
	if (node) {
		MoonSurface *surface;

		surface = node->GetData (&extents);
		if (surface)
			*ref = surface->ref ();

		delete node;
	}

	return extents;
}

cairo_t *
Context::Cairo ()
{
	return Top ()->Cairo ();
}

bool
Context::IsImmutable ()
{
	return Top ()->Readonly ();
}

};
