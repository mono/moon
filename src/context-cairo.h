/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-cairo.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_CAIRO_H__
#define __MOON_CONTEXT_CAIRO_H__

#include "context.h"
#include "surface-cairo.h"

namespace Moonlight {

class MOON_API CairoContext : public Context {
public:
	CairoContext (CairoSurface *surface);

	void Push (Group extents);

	void Blit (unsigned char *data,
		   int           stride);

	void Blend (Color *color);

	void Blend (MoonSurface *src,
		    double      alpha,
		    double      x,
		    double      y);

	void Flush ();
};

};

#endif /* __MOON_CONTEXT_CAIRO_H__ */
