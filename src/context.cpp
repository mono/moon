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

Context::Surface::Surface (MoonSurface *moon,
			   Rect        extents)
{
	native        = moon->ref ();
	box           = extents;
	surface       = NULL;
	device_offset = Point ();
}

Context::Surface::~Surface ()
{
	if (surface) {

		/* restore device offset */
		if (!box.IsEmpty ())
			cairo_surface_set_device_offset (surface,
							 device_offset.x,
							 device_offset.y);

		cairo_surface_destroy (surface);
	}

	native->unref ();
}

Rect
Context::Surface::GetData (MoonSurface **ref)
{
	if (ref)
		*ref = native->ref ();

	return box;
}

cairo_surface_t *
Context::Surface::Cairo ()
{
	if (!surface) {
		surface = native->Cairo ();

		/* replace device offset */
		if (!box.IsEmpty ()) {
			cairo_surface_get_device_offset (surface,
							 &device_offset.x,
							 &device_offset.y);
			cairo_surface_set_device_offset (surface,
							 -box.x,
							 -box.y);
		}
	}

	return cairo_surface_reference (surface);
}

Context::Node::Node (Surface        *surface,
		     cairo_matrix_t *matrix,
		     const Rect     *clip)
{
	target    = (Surface *) surface->ref ();
	box       = clip ? *clip : Rect (MIN_X, MIN_Y, MAX_W, MAX_H);
	transform = *matrix;
	context   = NULL;
}

Context::Node::~Node ()
{
	if (context)
		cairo_destroy (context);

	if (target)
		target->unref ();
}

cairo_t *
Context::Node::Cairo ()
{
	Surface *surface = GetSurface ();

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
	return target;
}

void
Context::Push (Transform transform)
{
	cairo_matrix_t matrix;
	Rect           box;

	Top ()->GetMatrix (&matrix);
	Top ()->GetClip (&box);

	cairo_matrix_multiply (&matrix, &transform.m, &matrix);

	Stack::Push (new Context::Node (Top ()->GetSurface (), &matrix, &box));
}

void
Context::Push (AbsoluteTransform transform)
{
	cairo_matrix_t matrix = transform.m;
	Rect           box;

	Top ()->GetClip (&box);

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
Context::Push (Group extents)
{
	g_warning ("Context::Push has been called. The derived class should have overridden it.");
	Push (Clip ());
}

void
Context::Push (Group extents, MoonSurface *surface)
{
	cairo_matrix_t matrix;
	Surface        *cs = new Surface (surface, extents.r);

	Top ()->GetMatrix (&matrix);

	Stack::Push (new Context::Node (cs, &matrix, &extents.r));
	cs->unref ();
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

	if (Top ()->GetSurface () != node->GetSurface ())
		r = node->GetSurface ()->GetData (ref);

	delete node;

	return r;
}

cairo_t *
Context::Cairo ()
{
	return Top ()->Cairo ();
}

bool
Context::IsImmutable ()
{
	Rect box;

	Top ()->GetClip (&box);

	return box.IsEmpty ();
}

};
