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

Rect
rect_from_str (const char *s)
{
	GArray *values = double_garray_from_str (s, 4);
	Rect r = Rect (g_array_index (values, double, 0), 
		       g_array_index (values, double, 1),
		       g_array_index (values, double, 2),
		       g_array_index (values, double, 3));
	
	g_array_free (values, true);
	return r;
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
