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

#include <gtk/gtk.h>

#include "array.h"
#include "point.h"

Point
point_from_str (const char *s)
{
	GArray *values = double_garray_from_str (s, 2);
	Point p = Point (g_array_index (values, double, 0), g_array_index (values, double, 1));

	g_array_free (values, true);

	return p;
}

