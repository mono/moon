/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

#include "rect.h"
#include "utils.h"
#include "moon-path.h"

bool
Rect::FromStr (const char *s, Rect *r)
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
Rect::Transform (cairo_matrix_t *transform)
{
	Rect rect = *this;

	if (!transform)
		return rect;

	double p1_x = rect.x;            double p1_y = rect.y;
	double p2_x = rect.x+rect.width; double p2_y = rect.y;
	double p3_x = rect.x+rect.width; double p3_y = rect.y+rect.height;
	double p4_x = rect.x;            double p4_y = rect.y+rect.height;

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


void Rect::Draw (cairo_t *cr, CornerRadius *round) const
{
	if (round) {
		Rect paint = *this;
		
		double top_adj = MAX (round->topLeft + round->topRight - paint.width, 0) / 2;
		double bottom_adj = MAX (round->bottomLeft + round->bottomRight - paint.width, 0) / 2;
		double left_adj = MAX (round->topLeft + round->bottomLeft - paint.height, 0) / 2;
		double right_adj = MAX (round->topRight + round->bottomRight - paint.height, 0) / 2;
		
		double tlt = round->topLeft - top_adj;
		cairo_move_to (cr, paint.x + tlt, paint.y);
		
		double trt = round->topRight - top_adj;
		double trr = round->topRight - right_adj;
		cairo_line_to (cr, paint.x + paint.width - trt, paint.y);
		cairo_curve_to (cr, 
				paint.x + paint.width - trt +  trt * ARC_TO_BEZIER, paint.y,
				paint.x + paint.width, paint.y + trr * ARC_TO_BEZIER,
				paint.x + paint.width, paint.y + trr);
		
		double brr = round->bottomRight - right_adj;
		double brb = round->bottomRight - bottom_adj;
		cairo_line_to (cr, paint.x + paint.width, paint.y + paint.height - brr);
		cairo_curve_to (cr,
				paint.x + paint.width, paint.y + paint.height - brr + brr * ARC_TO_BEZIER, 
				paint.x + paint.width + brb * ARC_TO_BEZIER - brb,  paint.y + paint.height,
				paint.x + paint.width - brb, paint.y + paint.height);
		
		double blb = round->bottomLeft - bottom_adj;
		double bll = round->bottomLeft - left_adj;
		cairo_line_to (cr, paint.x + blb, paint.y + paint.height);
		cairo_curve_to (cr,
				paint.x + blb - blb * ARC_TO_BEZIER, paint.y + paint.height,
				paint.x, paint.y + paint.height - bll * ARC_TO_BEZIER,
				paint.x, paint.y + paint.height - bll);
		
		double tll = round->topLeft - left_adj;
		cairo_line_to (cr, paint.x, paint.y + tll);
		cairo_curve_to (cr,
				paint.x, paint.y + tll - tll * ARC_TO_BEZIER,
				paint.x + tlt - tlt * ARC_TO_BEZIER, paint.y,
				paint.x + tlt, paint.y);
	} else {
		Draw (cr);
	}
}

