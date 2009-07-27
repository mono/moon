/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include "thickness.h"
#include "cornerradius.h"

// map to System.Windows.Rect
/* @IncludeInKinds */
struct Rect {
 public:
	double x, y, width, height;

	Rect () : x (0), y (0), width (0), height (0) {}

	// to please the current generator when used for a return value
	Rect (int zero) : x (0), y (0), width (0), height (0) {}

	Rect (double x, double y, double width, double height)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	Rect (const Point& p1, const Point& p2)
	{
		x = MIN (p1.x, p2.x);
		y = MIN (p1.y, p2.y);
		width = ABS (p1.x - p2.x);
		height = ABS (p1.y - p2.y);
	}

        Rect Transform (cairo_matrix_t *matrix);

	bool PointInside (double px, double py) const
	{
		return px >= x && px < (x + width) && py >= y && py < (y + height);
	}

	bool PointInside (const Point& p) const
	{
		return p.x >= x && p.x < (x + width) && p.y >= y && p.y < (y + height);
	}

	bool IntersectsWith (const Rect& rect) const
	{
		return ((x < rect.x + rect.width) && (x + width > rect.x) && (y < rect.y + rect.height) && (y + height > rect.y));
	}

	bool IsEmpty () const
	{
		return IsEmpty (false);
	}

	bool IsEmpty (bool logical) const
	{
		if (logical)
			return ((width <= 0.0) && (height <= 0.0));
		else
			return ((width <= 0.0) || (height <= 0.0));
	}
			
	Rect Intersection (const Rect& rect) const
	{
		Rect result = Rect ();
		result.x = x > rect.x ? x : rect.x;
		result.y = y > rect.y ? y : rect.y;
		result.width = MAX (0, ((x + width < rect.x + rect.width) ? (x + width) : (rect.x + rect.width)) - result.x);
		result.height = MAX (0, ((y + height < rect.y + rect.height) ? (y + height) : (rect.y + rect.height)) - result.y);
		return result;
	}

	Rect Union (const Rect& rect) const
	{
		return Union (rect, false);
	}

	// Note about the logical bool: there's now an override for both Rect::Union and
	// Rect::IsEmpty that takes a bool. That bool allows union of rectangle with one
	// empty extend. This is needed to compute logical bounds for example.
	Rect Union (const Rect& rect, bool logical) const
	{
		if (IsEmpty (logical))
			return Rect (rect);
		if (logical) {
			if ((rect.width <= 0.0) && (rect.height <= 0.0))
				return Rect (*this);
		} else {
			if ((rect.width <= 0.0) || (rect.height <= 0.0))
				return Rect (*this);	
		}
		Rect result = Rect ();
		result.x = x < rect.x ? x : rect.x;
		result.y = y < rect.y ? y : rect.y;
		result.width = ((x + width > rect.x + rect.width) ? (x + width) : (rect.x + rect.width)) - result.x;
		result.height = ((y + height > rect.y + rect.height) ? (y + height) : (rect.y + rect.height)) - result.y;
		return result;
	}

	Rect RoundOut () const
	{
		Rect result (floor (x), floor (y), ceil (x + width) - floor (x), ceil (y + height) - floor (y));
		return result;
	}

	Rect RoundIn () const
	{
		Rect result (ceil (x), ceil (y), floor (x + width) - ceil (x), floor (y + height) - ceil (y));
		return result;
	}

	Rect GrowBy (double left, double top, double right, double bottom) const
	{
		Rect result = *this;
		result.x -= left;
		result.y -= top;
		result.width += left + right;
		result.height += top + bottom;

		if (result.width < 0)
			result.width = 0;

		if (result.height < 0)
			result.height = 0;

		return result;
	}

	Rect GrowBy (double xd, double yd) const
	{
		return GrowBy (xd, yd, xd, yd);
	}

	Rect GrowBy (double d) const
	{
		return GrowBy (d, d, d, d);
	}

	Rect GrowBy (const Thickness &t) const
	{
		return GrowBy (t.left, t.top, t.right, t.bottom);
	}

	Rect ExtendTo (double x, double y) const
	{
		Rect result = *this;
		if (x < result.x || x > (result.x + result.width))
			result.width = MAX (ABS(x - result.x), ABS(x - result.x - result.width));
		if (y < result.y || y > (result.y + result.height))
			result.height = MAX (ABS(y - result.y), ABS(y - result.y - result.height));
		result.x = MIN (result.x, x);
		result.y = MIN (result.y, y);

		return result;
	}
	
	Rect ExtendTo (const Point& p) 
	{
		return ExtendTo (p.x, p.y);	
	}

	GdkRectangle 
	ToGdkRectangle () const
	{
		GdkRectangle gdk;
		Rect rect = RoundOut ();
		gdk.x = (gint)rect.x;
		gdk.y = (gint)rect.y;
		gdk.width = (gint)rect.width;
		gdk.height = (gint)rect.height;

		return gdk;
	}

	bool operator == (const Rect &rect)
	{
		return fabs(x-rect.x) < DBL_EPSILON && fabs(y-rect.y) < DBL_EPSILON && fabs(width-rect.width) < DBL_EPSILON && fabs(height-rect.height) < DBL_EPSILON;
	}

	bool operator != (const Rect &rect)
	{
		return !(*this == rect);
	}

	void Draw (cairo_t *cr) const 
	{
		cairo_rectangle (cr, x, y, width, height);
	}
	
	
	void Draw (cairo_t *cr, CornerRadius *round) const;

	Point GetTopLeft () const
	{
		return Point (x, y);
	}

	Point GetTopRight () const
	{
		return Point (x + width, y);
	}

	Point GetBottomLeft () const
	{
		return Point (x, y + height);
	}

	Point GetBottomRight () const
	{
		return Point (x + width, y + height);
	}

	//
	// FromStr
	//   Parses @s and return a new rect in @r.  Returns true if
	//   this was successful, false otherwise.
	//
	static bool FromStr (const char *s, Rect *r);
};

#endif /* __MOON_RECT_H__ */
