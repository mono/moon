/*
 * rect.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_RECT_H__
#define __MOON_RECT_H__

#include <cairo.h>

// map to System.Windows.Rect
struct Rect {
 public:
	double x, y, w, h;

	Rect () : x (0), y (0), w (0), h (0) {}
	Rect (double x, double y, double width, double height)
	{
		this->x = x;
		this->y = y;
		w = width;
		h = height;
	}

	Rect (const Rect &rect)
	{
		x = rect.x;
		y = rect.y;
		w = rect.w;
		h = rect.h;
	}

	bool PointInside (double px, double py)
	{
		return px > x && px < (x + w) && py > y && py < (y + h);
	}

	bool IntersectsWith (const Rect& rect)
	{
		return ((x < rect.x + rect.w) && (x + w > rect.x) && (y < rect.y + rect.h) && (y + h > rect.y));
	}

	Rect Intersection (const Rect& rect)
	{
		Rect result = Rect ();
		result.x = x > rect.x ? x : rect.x;
		result.y = y > rect.y ? y : rect.y;
		result.w = ((x + w < rect.x + rect.w) ? (x + w) : (rect.x + rect.w)) - result.x;
		result.h = ((y + h < rect.y + rect.h) ? (y + h) : (rect.y + rect.h)) - result.y;
		return result;
	}

	Rect Union (const Rect& rect)
	{
		Rect result = Rect ();
		result.x = x < rect.x ? x : rect.x;
		result.y = y < rect.y ? y : rect.y;
		result.w = ((x + w > rect.x + rect.w) ? (x + w) : (rect.x + rect.w)) - result.x;
		result.h = ((y + h > rect.y + rect.h) ? (y + h) : (rect.y + rect.h)) - result.y;
		return result;
	}

	bool operator == (const Rect &rect)
	{
		return x == rect.x && y == rect.y && w == rect.w && h == rect.h;
	}

	bool operator != (const Rect &rect)
	{
		return !(*this == rect);
	}
};

G_BEGIN_DECLS

Rect rect_from_str (const char *s);
Rect bounding_rect_for_transformed_rect (cairo_matrix_t *transform, Rect r);

G_END_DECLS

#endif /* __MOON_RECT_H__ */
