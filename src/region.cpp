/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * region.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>

#include "region.h"


Region::Region ()
{ 
	gdkregion = gdk_region_new (); 
}

Region::Region (double x, double y, double width, double height)
{
	gdkregion = gdk_region_new ();
	Union (Rect (x, y, width, height));
}

Region::Region (Rect rect)
{
	gdkregion = gdk_region_new ();
	Union (rect);
}

Region::Region (GdkRegion *region)
{
	gdkregion = gdk_region_copy (region);
}

Region::Region (Region *region)
{
	gdkregion = gdk_region_copy (region->gdkregion);
}

Region::~Region ()
{
	gdk_region_destroy (gdkregion);
	gdkregion = NULL;
}

bool
Region::IsEmpty ()
{
	return gdk_region_empty (gdkregion);
}

void 
Region::Union (Rect rect)
{
	GdkRectangle gdkrect = rect.ToGdkRectangle ();
	gdk_region_union_with_rect (gdkregion, &gdkrect);
}

void
Region::Union (GdkRectangle *rect)
{
	gdk_region_union_with_rect (gdkregion, rect);
}

void 
Region::Union (Region *region)
{
	gdk_region_union (gdkregion, region->gdkregion);
}

GdkOverlapType
Region::RectIn (Rect rect)
{
	GdkRectangle gdkrect = rect.ToGdkRectangle ();
	return gdk_region_rect_in (gdkregion, &gdkrect);
}

void
Region::Intersect (Region *region)
{
	gdk_region_intersect (gdkregion, region->gdkregion);
}

void
Region::Intersect (Rect rect)
{
	Region tmp = Region (rect);
	Intersect (&tmp);
}


void
Region::Subtract (Region *region)
{
	gdk_region_subtract (gdkregion, region->gdkregion);
}

void
Region::Subtract (Rect rect)
{
	Region tmp = Region (rect);
	Subtract (&tmp);
}

void
Region::Offset (int dx, int dy)
{
	gdk_region_offset (gdkregion, dx, dy);
}

void
Region::GetRectangles (GdkRectangle **rects, int *count)
{
	gdk_region_get_rectangles (gdkregion, rects, count);
	//if (*count > 10) {
	//	*count = 1;
	//	gdk_region_get_clipbox (gdkregion, *rects);
	//}
}

Rect 
Region::ClipBox ()
{
	GdkRectangle clip;
	gdk_region_get_clipbox (gdkregion, &clip);
	return Rect (clip.x, clip.y, clip.width, clip.height);
}

void 
Region::Draw (cairo_t *cr)
{
	int i, count;
	GdkRectangle *rects;

	gdk_region_get_rectangles (gdkregion, &rects, &i);

	for (count = 0; count < i; count++)
		cairo_rectangle (cr, rects [count].x, rects [count].y, rects [count].width, rects [count].height);

	g_free (rects);
}

