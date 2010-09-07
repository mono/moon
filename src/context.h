/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_H__
#define __MOON_CONTEXT_H__

#include <cairo.h>

#include "list.h"
#include "rect.h"
#include "surface.h"

namespace Moonlight {

class Context : public Stack {
public:
	class Surface : public MoonSurface {
	public:
		Surface (MoonSurface *moon);
		Surface (MoonSurface *moon,
			 double      xOffset,
			 double      yOffset);
		virtual ~Surface ();

		cairo_surface_t *Cairo ();

		Surface *Similar (Rect r);
		MoonSurface *Native ();

	private:
		MoonSurface     *native;
		Point           offset;
		cairo_surface_t *surface;
	};

	class Node : public List::Node {
	public:
		Node (Surface *surface, cairo_matrix_t *matrix);
		Node (Rect extents, cairo_matrix_t *matrix);
		virtual ~Node ();

		cairo_t *Cairo ();

		void GetMatrix (cairo_matrix_t *matrix);
		Surface *GetSurface ();
		MoonSurface *GetData (Rect *extents);
		void SetData (MoonSurface *surface);
		bool Readonly ();

	private:
		Rect           box;
		cairo_matrix_t transform;
		cairo_t        *context;
		Surface        *target;
		MoonSurface    *data;
	};

	Context (MoonSurface *surface);
	Context (MoonSurface *surface, cairo_matrix_t *transform);

	void Push (cairo_matrix_t *transform);
	void Push (Rect extents);
	void Push (Rect extents, cairo_matrix_t *matrix);
	Node *Top ();
	void Pop ();
	Rect Pop (MoonSurface **surface);

	cairo_t *Cairo ();
	bool IsImmutable ();
	bool IsMutable () { return !IsImmutable (); }
};

};

#endif /* __MOON_CONTEXT_H__ */
