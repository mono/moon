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

#include "rect.h"

Rect
rect_from_str (const char *s)
{
	// FIXME - not robust enough for production
	char *next = NULL;
	double x = strtod (s, &next);
	double y = 0.0;
	if (next) {
		++next;
		y = strtod (next, &next);
	}
	double w = 0.0;
	if (next) {
		++next;
		w = strtod (next, &next);
	}
	double h = 0.0;
	if (next) {
		++next;
		h = strtod (next, &next);
	}
	return Rect (x, y, w, h);
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
