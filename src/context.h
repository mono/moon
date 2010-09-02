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
	class Node : public List::Node {
	public:
		Node (MoonSurface *surface);
		Node (MoonSurface *surface, cairo_matrix_t *transform);
		Node (Rect extents);
		Node (Rect extents, cairo_matrix_t *transform);
		virtual ~Node ();

		cairo_t *GetCr ();
		MoonSurface *GetBitmap ();
		void SetBitmap (MoonSurface *surface);
		Rect GetBitmapExtents (void);

	private:
		Rect           box;
		cairo_matrix_t matrix;
		cairo_t        *context;
		MoonSurface    *bitmap;
		bool           readonly;
	};

	Node *Pop () { return (Node *) Stack::Pop (); }
	Node *Top () { return (Node *) Stack::Top (); }
};

};

#endif /* __MOON_CONTEXT_H__ */
