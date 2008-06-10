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
#include <errno.h>

#include "array.h"


DoubleArray *
double_array_new (int count, double *values)
{
	DoubleArray *p = (DoubleArray *) g_malloc0 (sizeof (DoubleArray) + count * sizeof (double));
	p->basic.count = count;
	p->basic.refcount = 1;
	memcpy (p->values, values, sizeof (double) * count);
	return p;
}

GArray *double_garray_from_str (const char *s, gint max)
{
	char *next = (char *)s;
	GArray *values = g_array_sized_new (false, true, sizeof (double), max > 0 ? max : 16);
	double coord = 0.0;
	guint end = max > 0 ? max : G_MAXINT;

	while (next && values->len < end) {
		while (g_ascii_isspace (*next) || *next == ',')
			next = g_utf8_next_char (next);
		
		if (next) {
			errno = 0;
			char *prev = next;
			coord = g_ascii_strtod (prev, &next);
			if (errno != 0 || next == prev)
				goto error;

			g_array_append_val (values, coord);
		}
	}

error:
	while (values->len < (guint) max) {
		coord = 0.0;
		g_array_append_val (values, coord);
	}

	return values;
}

double *
double_array_from_str (const char *s, int *count)
{
	GArray *values = double_garray_from_str (s, 0);

	if (values->len == 0) {
		*count = 0;
		return NULL;
	}

	double *doubles = new double [values->len];
	memcpy (doubles, values->data, values->len * sizeof (double));
	*count = values->len;
	g_array_free (values, true);

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
}


Point *
point_array_from_str (const char *s, int* count)
{
	int i, j, n = 0;
	GArray *values = double_garray_from_str (s, 0);

	n = values->len / 2;
	if (n == 0 || n % 1 == 1) {
		g_array_free (values, true);
		*count = 0;
		return NULL;
	}

	Point *points = new Point [n];
	for (i = 0, j = 0; j < n; j++) {
		points[j].x = g_array_index (values, double, i++);
		points[j].y = g_array_index (values, double, i++);
	}

	g_array_free (values, true);
	*count = n;
	return points;
}
