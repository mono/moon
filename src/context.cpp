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

Context::Node::Node (MoonSurface *surface)
{
	box      = Rect ();
	context  = NULL;
	bitmap   = surface->ref ();
	readonly = false;

	cairo_matrix_init_identity (&matrix);
}

Context::Node::Node (MoonSurface *surface, cairo_matrix_t *transform)
{
	box      = Rect ();
	matrix   = *transform;
	context  = NULL;
	bitmap   = surface->ref ();
	readonly = false;
}

Context::Node::Node (Rect extents)
{
	box      = extents;
	context  = NULL;
	bitmap   = NULL;
	readonly = false;

	cairo_matrix_init_identity (&matrix);
}

Context::Node::Node (Rect extents, cairo_matrix_t *transform)
{
	box      = extents;
	matrix   = *transform;
	context  = NULL;
	bitmap   = NULL;
	readonly = false;
}

Context::Node::~Node ()
{
	if (context)
		cairo_destroy (context);

	if (bitmap)
		bitmap->unref ();
}

cairo_t *
Context::Node::GetCr ()
{
	if (readonly)
		return NULL;

	if (!context) {
		cairo_surface_t *surface;
		Rect            r = box.RoundOut ();

		if (!GetBitmap ())
			return NULL;

		surface = bitmap->Cairo ();

		if (!r.IsEmpty ())
			cairo_surface_set_device_offset (surface, -r.x, -r.y);

		context = cairo_create (surface);
		cairo_set_matrix (context, &matrix);

		cairo_surface_destroy (surface);
	}

	return context;
}

MoonSurface *
Context::Node::GetBitmap ()
{
	if (!bitmap) {
		MoonSurface *base;
		Rect        r = box.RoundOut ();

		if (!prev) {
			g_warning ("ContextNode::GetBitmap no base node.");
			return NULL;
		}

		base = ((Context::Node *) prev)->GetBitmap ();
		if (!base) {
			g_warning ("ContextNode::GetBitmap no base bitmap.");
			return NULL;
		}

		bitmap = base->Similar (r.width, r.height);
	}

	return bitmap;
}

void
Context::Node::SetBitmap (MoonSurface *surface)
{
	MoonSurface *ref;

	if (context) {
		g_warning ("ContextNode::SetBitmap context present.");
		return;
	}

	ref = surface->ref ();

	if (bitmap)
		bitmap->unref ();

	bitmap   = ref;
	readonly = true;
}

Rect
Context::Node::GetBitmapExtents (void)
{
	return box.RoundOut ();
}
  
};
