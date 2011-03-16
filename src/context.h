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

class MOON_API Context : public Stack {
public:
	struct Clip {
	public:
		Clip () : r (0) {}
		Clip (Rect clip) : r (clip.RoundOut ()) {}

		Rect r;
	};

	struct Group {
	public:
		Group (Rect group) : r (group.RoundOut ()) {}

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

	struct Cairo {
	public:
		Cairo ();
		Cairo (Rect cairo) : r (cairo.RoundOut ()) {}

		Rect r;
	};

	class Target : public MoonSurface {
	public:
		Target ();
		Target (MoonSurface *moon,
			Rect        extents);
		virtual ~Target ();

		virtual cairo_surface_t *Cairo ();

		Rect GetData (MoonSurface **surface);

		void SetCairoTarget (Target *target);
		Target *GetCairoTarget ();

		void SetInit (MoonSurface *src);
		MoonSurface *GetInit ();

	protected:
		MoonSurface *native;
		Rect        box;
		Target      *cairo;
		MoonSurface *init;
	};

	class Node : public List::Node {
	public:
		Node (Target         *surface,
		      cairo_matrix_t *matrix,
		      const Rect     *clip);
		virtual ~Node ();

		cairo_t *Cairo ();

		void GetMatrix (cairo_matrix_t *matrix);
		void GetClip (Rect *clip);
		Target *GetTarget ();

	private:
		Target         *target;
		Rect           box;
		cairo_matrix_t transform;
		cairo_t        *context;
	};

	class Cache {
	public:
		Cache () : contexts (NULL) {}
		~Cache () { Release (); }

		void Release ();

	private:
		GList *contexts;

		friend class Context;
	};

	Context ();
	Context (MoonSurface *surface);
	virtual ~Context ();

	void Replace (Cache *key, MoonSurface *surface);
	MoonSurface *Remove (Cache *key);
	MoonSurface *Lookup (Cache *key);

	void Push (Transform transform);
	void Push (AbsoluteTransform transform);
	void Push (Clip clip);
	Node *Top ();
	void Pop ();

	bool IsImmutable ();
	bool IsMutable () { return !IsImmutable (); }
	void GetMatrix (double *out);

	virtual void Push (Group extents);
	virtual void Push (Group extents, MoonSurface *surface);
	virtual cairo_t *Push (Cairo extents);
	virtual Rect Pop (MoonSurface **surface);

	virtual void Clear (Color *color);

	virtual void Blit (unsigned char *data,
			   int           stride);

	virtual void Blend (MoonSurface *src,
			    double      alpha,
			    double      x,
			    double      y);

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

	virtual void Flush ();

private:
	GHashTable *cache;
};

};

#endif /* __MOON_CONTEXT_H__ */
