/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * point.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_POINT_H__
#define __MOON_POINT_H__

#include <glib.h>
#include <cairo.h>
#include <math.h>

/* @IncludeInKinds */
struct Point {
public:
	double x, y;

	Point () : x(0), y(0) {}

	Point (double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	Point operator+ (const Point &point)
	{
		return Point (x + point.x,
			      y + point.y);
	}

	Point operator- (const Point &point)
	{
		return Point (x - point.x,
			      y - point.y);
	}

	Point operator* (double v)
	{
		return Point (x * v, y * v);
	}

	bool operator == (const Point &point) const
	{
		return fabs (point.x-x) < DBL_EPSILON && fabs (point.y-y) < DBL_EPSILON;
	}

	bool operator != (const Point &point) const
	{
		return !(*this == point);
	}

	Point Transform (cairo_matrix_t *matrix);

	//
	// FromStr
	//   Parses @s and return a new point in @p.  Returns true if
	//   this was successful, false otherwise.
	//
	static bool FromStr (const char *s, Point *p);
};

#endif /* __MOON_POINT_H__ */
