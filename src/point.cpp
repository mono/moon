/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * point.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>

#include <glib.h>

#include "point.h"
#include "utils.h"

bool
Point::FromStr (const char *s, Point *p)
{
	GArray *values = double_garray_from_str (s, 2);

	if (!values)
		return false;

	*p = Point (g_array_index (values, double, 0), g_array_index (values, double, 1));

	g_array_free (values, true);

	return true;
}

Point
Point::Transform (cairo_matrix_t *matrix)
{
	double nx = x;
	double ny = y;

	cairo_matrix_transform_point (matrix, &nx, &ny);

	return Point (nx, ny);
}
