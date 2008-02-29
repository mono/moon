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
#include <math.h>
#include <gtk/gtk.h>

#include "point.h"

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

        Rect Transform (cairo_matrix_t *matrix);

	bool PointInside (double px, double py)
	{
		return px > x && px < (x + w) && py > y && py < (y + h);
	}

	bool PointInside (Point p)
	{
		return p.x > x && p.x < (x + w) && p.y > y && p.y < (y + h);
	}

	bool IntersectsWith (const Rect& rect)
	{
		return ((x < rect.x + rect.w) && (x + w > rect.x) && (y < rect.y + rect.h) && (y + h > rect.y));
	}

	bool IsEmpty ()
	{
		return ((w <= 0.0) || (h <= 0.0));
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
		if (IsEmpty ())
			return Rect (rect);
		if ((rect.w <= 0.0) || (rect.h <= 0.0))
			return Rect (*this);
		Rect result = Rect ();
		result.x = x < rect.x ? x : rect.x;
		result.y = y < rect.y ? y : rect.y;
		result.w = ((x + w > rect.x + rect.w) ? (x + w) : (rect.x + rect.w)) - result.x;
		result.h = ((y + h > rect.y + rect.h) ? (y + h) : (rect.y + rect.h)) - result.y;
		return result;
	}

	Rect RoundOut ()
	{
		Rect result (floor (x), floor (y), ceil (x + w) - floor (x), ceil (y + h) - floor (y));
		return result;
	}

	Rect RoundIn ()
	{
		Rect result (ceil (x), ceil (y), floor (x + w) - ceil (x), floor (y + h) - ceil (y));
		return result;
	}

	Rect GrowBy (double xd, double yd)
	{
		Rect result = *this;
		result.x -= xd;
		result.y -= yd;
		result.w += 2*xd;
		result.h += 2*yd;

		return result;
	}

	Rect GrowBy (double d)
	{
		return GrowBy (d, d);
	}

	GdkRectangle 
	ToGdkRectangle ()
	{
		GdkRectangle gdk;
		Rect rect = RoundOut ();
		gdk.x = (gint)rect.x;
		gdk.y = (gint)rect.y;
		gdk.width = (gint)rect.w;
		gdk.height = (gint)rect.h;

		return gdk;
	}

	bool operator == (const Rect &rect)
	{
		return x == rect.x && y == rect.y && w == rect.w && h == rect.h;
	}

	bool operator != (const Rect &rect)
	{
		return !(*this == rect);
	}

	void Draw (cairo_t *cr) 
	{
		cairo_rectangle (cr, x, y, w, h);
	}
};

G_BEGIN_DECLS

bool rect_from_str (const char *s, Rect *r);

G_END_DECLS

#endif /* __MOON_RECT_H__ */
