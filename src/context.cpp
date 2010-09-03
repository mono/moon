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
	box     = Rect ();
	context = NULL;
	target  = surface->ref ();
	data    = NULL;

	cairo_matrix_init_identity (&matrix);
}

Context::Node::Node (MoonSurface *surface, cairo_matrix_t *transform)
{
	box     = Rect ();
	matrix  = *transform;
	context = NULL;
	target  = surface->ref ();
	data    = NULL;
}

Context::Node::Node (Rect extents)
{
	box     = extents;
	context = NULL;
	target  = NULL;
	data    = NULL;

	cairo_matrix_init_identity (&matrix);
}

Context::Node::Node (Rect extents, cairo_matrix_t *transform)
{
	box     = extents;
	matrix  = *transform;
	context = NULL;
	target  = NULL;
	data    = NULL;
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
	MoonSurface *surface = GetSurface ();

	if (surface != target)
		return NULL;

	if (!context) {
		cairo_surface_t *dst;
		Rect            r = box.RoundOut ();

		dst = target->Cairo ();

		if (!r.IsEmpty ())
			cairo_surface_set_device_offset (dst, -r.x, -r.y);

		context = cairo_create (dst);
		cairo_set_matrix (context, &matrix);

		cairo_surface_destroy (dst);
	}

	return context;
}

MoonSurface *
Context::Node::GetSurface ()
{
	return target ? target : GetData (NULL);
}

MoonSurface *
Context::Node::GetData (Rect *extents)
{
	Rect r = box.RoundOut ();

	if (!data) {
		MoonSurface *base;

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

		data = base->Similar (r.width, r.height);
		if (!data) {
			g_warning ("ContextNode::GetData failed.");
			return NULL;
		}

		target = data->ref ();
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
	MoonSurface *surface = GetSurface ();

	if (surface != target)
		return true;

	return false;
}

Context::Context (MoonSurface *surface)
{
	Stack::Push (new Context::Node (surface));
}

Context::Context (MoonSurface *surface, cairo_matrix_t *transform)
{
	Stack::Push (new Context::Node (surface, transform));
}

void
Context::Push (Rect extents)
{
	Stack::Push (new Context::Node (extents));
}

void
Context::Push (Rect extents, cairo_matrix_t *transform)
{
	Stack::Push (new Context::Node (extents, transform));
}

Context::Node *
Context::Top ()
{
	return (Node *) Stack::Top ();
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
