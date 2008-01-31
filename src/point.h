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

struct Point {
public:
	double x, y;

	Point () : x(0), y(0) {}

	Point (double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	Point (const Point &point)
	{
		x = point.x;
		y = point.y;
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
};

G_BEGIN_DECLS

bool point_from_str (const char *s, Point *p);

G_END_DECLS

#endif /* __MOON_POINT_H__ */
