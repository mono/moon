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
	struct Clip {
	public:
		Clip () : r (0) {}
		Clip (Rect clip) : r (clip) {}

		Rect r;
	};

	struct Group {
	public:
		Group (Rect group) : r (group) {}

		Rect r;
	};

	struct Transform {
	public:
		Transform (cairo_matrix_t matrix) : m (matrix) {}

		cairo_matrix_t m;
	};

	struct AbsoluteTransform {
	public:
		AbsoluteTransform () { cairo_matrix_init_identity (&m); }
		AbsoluteTransform (cairo_matrix_t matrix) : m (matrix) {}

		cairo_matrix_t m;
	};

	class Surface : public MoonSurface {
	public:
		Surface (MoonSurface *moon,
			 Rect        extents);
		virtual ~Surface ();

		cairo_surface_t *Cairo ();

		Rect GetData (MoonSurface **surface);

	private:
		MoonSurface     *native;
		Rect            box;
		cairo_surface_t *surface;
	};

	class Node : public List::Node {
	public:
		Node (Surface        *surface,
		      cairo_matrix_t *matrix,
		      const Rect     *clip);
		virtual ~Node ();

		cairo_t *Cairo ();

		void GetMatrix (cairo_matrix_t *matrix);
		void GetClip (Rect *clip);
		Surface *GetSurface ();

	private:
		Surface        *target;
		Rect           box;
		cairo_matrix_t transform;
		cairo_t        *context;
	};

	Context (MoonSurface *surface);

	void Push (Transform transform);
	void Push (AbsoluteTransform transform);
	void Push (Clip clip);
	void Push (Group extents);
	void Push (Group extents, MoonSurface *surface);
	Node *Top ();
	void Pop ();
	Rect Pop (MoonSurface **surface);

	cairo_t *Cairo ();

	bool IsImmutable ();
	bool IsMutable () { return !IsImmutable (); }
};

};

#endif /* __MOON_CONTEXT_H__ */
