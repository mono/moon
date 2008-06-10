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

#include <glib.h>
#include <cairo.h>
#include <math.h>
#include <gdk/gdk.h> // for GdkRectangle

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

	Rect (Point p1, Point p2)
	{
		x = MIN (p1.x, p2.x);
		y = MIN (p1.y, p2.y);
		w = ABS (p1.x - p2.x);
		h = ABS (p1.y - p2.y);
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
		return IsEmpty (false);
	}

	bool IsEmpty (bool logical)
	{
		if (logical)
			return ((w <= 0.0) && (h <= 0.0));
		else
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
		return Union (rect, false);
	}


	// Note about the logical bool: there's now an override for both Rect::Union and
	// Rect::IsEmpty that takes a bool. That bool allows union of rectangle with one
	// empty extend. This is needed to compute logical bounds for example.
	Rect Union (const Rect& rect, bool logical)
	{
		if (IsEmpty (logical))
			return Rect (rect);
		if (logical) {
			if ((rect.w <= 0.0) && (rect.h <= 0.0))
				return Rect (*this);
		} else {
			if ((rect.w <= 0.0) || (rect.h <= 0.0))
				return Rect (*this);	
		}
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

	Rect ExtendTo (double x, double y)
	{
		Rect result = *this;
		if (x < result.x || x > (result.x + result.w))
			result.w = MAX (ABS(x - result.x), ABS(x - result.x - result.w));
		if (y < result.y || y > (result.y + result.h))
			result.h = MAX (ABS(y - result.y), ABS(y - result.y - result.h));
		result.x = MIN (result.x, x);
		result.y = MIN (result.y, y);

		return result;
	}
	
	Rect ExtendTo (Point p) 
	{
		return ExtendTo (p.x, p.y);	
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
