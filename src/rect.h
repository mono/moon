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

	Rect GrowBy (double d)
	{
		Rect result = *this;
		result.x -= d;
		result.y -= d;
		result.w += 2*d;
		result.h += 2*d;

		return result;
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

	Rect Transform (cairo_matrix_t *xform)
	{
		double p1x, p1y;
		double p2x, p2y;

		p1x = x;
		p1y = y;

		p2x = x + w;
		p2y = y + h;

		cairo_matrix_transform_point (xform, &p1x, &p1y);
		cairo_matrix_transform_point (xform, &p2y, &p2y);

		double left, right;
		double top, bottom;

		left = p1x > p2x ? p2x : p1x;
		right = p1x > p2x ? p1x : p2x;

		top = p1y > p2y ? p2y : p1y;
		bottom = p1y > p2y ? p1y : p2y;

		return Rect (left, top, right - left, bottom - top);
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

class Region {
public:
	GdkRegion *gdkregion;

	Region ();
	Region (Rect rect);
	Region (GdkRegion *region);
	Region (double x, double y, double width, double height);
	
	~Region ();

	bool IsEmpty ();

	void Union (Rect rect);
	void Union (GdkRegion *region);
	void Union (GdkRectangle *rect);
	void Union (Region *region);

	void Intersect (Region *region);
	void Intersect (Rect rect);

	void Subtract (Region *region);
	void Subtract (Rect rect);

	void GetRectangles (GdkRectangle **rects, int *count);

	Rect ClipBox ();
	GdkOverlapType RectIn (Rect rect);

	void Draw (cairo_t *cr)
	{
		int i, count;
		GdkRectangle *rects;
	
		gdk_region_get_rectangles (gdkregion, &rects, &i);
		
		for (count = 0; count < i; count++)
			cairo_rectangle (cr, rects [count].x, rects [count].y, rects [count].width, rects [count].height);
		
		g_free (rects);
	}
};
     
G_BEGIN_DECLS

bool rect_from_str (const char *s, Rect *r);
Rect bounding_rect_for_transformed_rect (cairo_matrix_t *transform, Rect r);

G_END_DECLS

#endif /* __MOON_RECT_H__ */
