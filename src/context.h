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
#include "color.h"
#include "brush.h"
#include "surface.h"

namespace Moonlight {

class PixelShader;

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
		Surface ();
		Surface (MoonSurface *moon,
			 Rect        extents);
		virtual ~Surface ();

		virtual cairo_surface_t *Cairo ();

		Rect GetData (MoonSurface **surface);

	protected:
		MoonSurface     *native;
		Rect            box;
		cairo_surface_t *surface;
		Point           device_offset;
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

	virtual ~Context () {}

	void Push (Transform transform);
	void Push (AbsoluteTransform transform);
	void Push (Clip clip);
	Node *Top ();
	void Pop ();
	Rect Pop (MoonSurface **surface);

	cairo_t *Cairo ();

	bool IsImmutable ();
	bool IsMutable () { return !IsImmutable (); }

	virtual void Push (Group extents);
	virtual void Push (Group extents, MoonSurface *surface);

	virtual void Clear (Color *color);

	virtual void Project (MoonSurface  *src,
			      const double *matrix,
			      double       alpha,
			      double       x,
			      double       y);

	virtual void Blur (MoonSurface *src,
			   double      radius,
			   double      x,
			   double      y);

	virtual void DropShadow (MoonSurface *src,
				 double      dx,
				 double      dy,
				 double      radius,
				 Color       *color,
				 double      x,
				 double      y);

	virtual void ShaderEffect (MoonSurface *src,
				   PixelShader *shader,
				   Brush       **sampler,
				   int         *sampler_mode,
				   int         n_sampler,
				   Color       *constant,
				   int         n_constant,
				   int         *ddxUvDdyUvPtr,
				   double      x,
				   double      y);
};

};

#endif /* __MOON_CONTEXT_H__ */
