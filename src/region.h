/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * region.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_REGION_H__
#define __MOON_REGION_H__

#include <cairo.h>
#include <gdk/gdk.h> // for GdkRegion
#include "rect.h"

class Region {
	GdkRegion *gdkregion;

public:
	Region ();
	Region (Rect rect);
	Region (GdkRegion *region);
	Region (Region *region);
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

	void Offset (int dx, int dy);

	Rect ClipBox ();
	GdkOverlapType RectIn (Rect rect);

	void Draw (cairo_t *cr);
};
     
#endif /* __MOON_REGION_H__ */
