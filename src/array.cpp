/*
 * array.h: array classes to aid marshalling.
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "array.h"

static char**
split_str (const char* s, int *count)
{
	int n;
	// FIXME - what are all the valid separators ? I've seen ',' and ' '
	char** values = g_strsplit_set (s, ", ", 0);
	if (count) {
		// count non-NULL entries (which means we must skip NULLs later too)
		for (n = 0; values[n]; n++);
		*count = n;
	}
	return values;
}


DoubleArray *
double_array_new (int count, double *values)
{
	DoubleArray *p = (DoubleArray *) g_malloc0 (sizeof (DoubleArray) + count * sizeof (double));
	p->basic.count = count;
	p->basic.refcount = 1;
	memcpy (p->values, values, sizeof (double) * count);
	return p;
}

double *
double_array_from_str (const char *s, int *count)
{
	char **values = split_str (s, count);
	int i, n;
	
	double *doubles = new double [*count];
	for (i = 0, n = 0; i < *count; i++) {
		char *value = values[i];
		if (value)
			doubles[n++] = strtod (value, NULL);
	}

	g_strfreev (values);
	return doubles;
}


PointArray *
point_array_new (int count, Point *points)
{
	PointArray *p = (PointArray *) g_malloc0 (sizeof (PointArray) + count * sizeof (Point));
	p->basic.count = count;
	p->basic.refcount = 1;
	memcpy (p->points, points, sizeof (Point) * count);
	return p;
};


Point *
point_array_from_str (const char *s, int* count)
{
	int i, j, n = 0;
	bool x = true;
	char** values = split_str (s, &n);

	*count = (n >> 1); // 2 doubles for each point
	Point *points = new Point [*count];
	for (i = 0, j = 0; i < n; i++) {
		char *value = values[i];
		if (value) {
			if (x) {
				points[j].x = strtod (value, NULL);
				x = false;
			} else {
				points[j++].y = strtod (value, NULL);
				x = true;
			}
		}
	}

	g_strfreev (values);
	return points;
}

