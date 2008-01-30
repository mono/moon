/*
 * rect.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "array.h"
#include "rect.h"

bool
rect_from_str (const char *s, Rect *r)
{
	GArray *values = double_garray_from_str (s, 4);

	if (!values)
		return false;

	*r = Rect (g_array_index (values, double, 0), 
		       g_array_index (values, double, 1),
		       g_array_index (values, double, 2),
		       g_array_index (values, double, 3));
	
	g_array_free (values, true);
	return true;
}

Rect
bounding_rect_for_transformed_rect (cairo_matrix_t *transform, Rect rect)
{
	double p1_x = rect.x;        double p1_y = rect.y;
	double p2_x = rect.x+rect.w; double p2_y = rect.y;
	double p3_x = rect.x+rect.w; double p3_y = rect.y+rect.h;
	double p4_x = rect.x;        double p4_y = rect.y+rect.h;

	cairo_matrix_transform_point (transform, &p1_x, &p1_y);
	cairo_matrix_transform_point (transform, &p2_x, &p2_y);
	cairo_matrix_transform_point (transform, &p3_x, &p3_y);
	cairo_matrix_transform_point (transform, &p4_x, &p4_y);

#define MIN2(v1,v2) ((v1)>(v2)?(v2):(v1))
#define MIN4(v1,v2,v3,v4) (MIN2(MIN2(MIN2(v1,v2),v3),v4))

#define MAX2(v1,v2) ((v1)>(v2)?(v1):(v2))
#define MAX4(v1,v2,v3,v4) (MAX2(MAX2(MAX2(v1,v2),v3),v4))

	double l = MIN4(p1_x,p2_x,p3_x,p4_x);
	double t = MIN4(p1_y,p2_y,p3_y,p4_y);
	double r = MAX4(p1_x,p2_x,p3_x,p4_x);
	double b = MAX4(p1_y,p2_y,p3_y,p4_y);

	return Rect (l, t, r-l, b-t);
}

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
