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
#include <stdio.h>
#include "rect.h"

namespace Moonlight {

class Region {
	cairo_region_t *cairo_region;
	cairo_status_t status;

public:
	Region ();
	Region (Rect rect);
	Region (cairo_region_t *cairo_region);
	Region (Region *region);
	Region (double x, double y, double width, double height);
	
	~Region ();

	bool IsEmpty ();

	void Union (Rect rect);
	void Union (GdkRegion *region);
	void Union (Region *region);

	void Intersect (Region *region);
	void Intersect (Rect rect);

	void Subtract (Region *region);
	void Subtract (Rect rect);

	
	int GetRectangleCount ();
	Rect GetRectangle (int index);

	void Offset (int dx, int dy);

	Rect GetExtents ();
	cairo_region_overlap_t RectIn (Rect rect);

	void Draw (cairo_t *cr);
};
     
};
#endif /* __MOON_REGION_H__ */
