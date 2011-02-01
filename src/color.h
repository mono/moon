/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * color.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_COLOR_H__
#define __MOON_COLOR_H__

#include <glib.h>
#include <math.h>

#include "moonbuild.h"

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
struct Color {
	double r, g, b, a;
	
	Color () : r(0.0), g(0.0), b(0.0), a(0.0) {}

	Color (guint32 argb)
	{
		a = (argb >> 24) / 255.0f;
		r = ((argb >> 16) & 0xFF) / 255.0f;
		g = ((argb >> 8) & 0xFF) / 255.0f;
		b = (argb & 0xFF) / 255.0f;
	}
	
	Color (double r, double g, double b, double a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	Color operator+ (const Color &color)
	{
		return Color (r + color.r,
			      g + color.g,
			      b + color.b,
			      a + color.a);
	}

	Color operator- (const Color &color)
	{
		return Color (r - color.r,
			      g - color.g,
			      b - color.b,
			      a - color.a);
	}

	Color operator* (double v)
	{
		return Color (r * v,
			      g * v,
			      b * v,
			      a * v);
	}

	bool operator!= (const Color &v) const
	{
		return !(*this == v);
	}

	bool operator== (const Color &v) const
	{
		return (fabs (r-v.r) < DBL_EPSILON && fabs (g-v.g) < DBL_EPSILON && fabs (b-v.b) < DBL_EPSILON && fabs (a-v.a) < DBL_EPSILON);
	}
};

G_BEGIN_DECLS

const char *color_to_string (Color *color);

Color *color_from_str  (const char *name) MOON_API;

G_END_DECLS

#endif /* __MOON_COLOR_H__ */
