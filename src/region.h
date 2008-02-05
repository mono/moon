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
#include <gtk/gtk.h>
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
     
#endif /* __MOON_REGION_H__ */
