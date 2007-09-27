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

#include <gtk/gtk.h>

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


static bool
is_separator (char c)
{
	switch (c) {
	case ',':
	case ' ':
	case '\r':
	case '\n':
	case '\t':
		return true;
	default:
		return false;
	}
}


double *
double_array_from_str (const char *s, int *count)
{
	int n = 16;
	int pos = 0;
	double *doubles = (double*)g_malloc (n * sizeof (double));
	char *start = (char*)s;
	char *end = NULL;

	errno = 0;
	// note: we use g_ascii_strtod because in some locale, like french, 
	// comma is used as the decimal point
	double dv = g_ascii_strtod (start, &end);
	if (errno != 0 || (start == end))
		goto error;

	doubles[pos++] = dv;
	while (*end) {
		if (is_separator (*end)) {
			end++;
		} else {
			start = end;
			errno = 0;
			dv = g_ascii_strtod (start, &end);
			if (errno != 0 || (start == end))
				goto error;
			if (pos == n) {
				n <<= 1; // double array size
				doubles = (double*)g_realloc (doubles, n * sizeof (double));
			}
			doubles[pos++] = dv;
		}
	}

	*count = pos;
	return doubles;
error:
	g_free (doubles);
	*count = 0;
	return NULL;
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
	double *doubles = double_array_from_str (s, &n);
	if (!doubles) {
		*count = 0;
		return NULL;
	}
	// invalid if doubles are not in pair
	if ((n & 1) == 1) {
		g_free (doubles);
		*count = 0;
		return NULL;
	}

	n >>= 1; // 2 doubles for each point
	Point *points = new Point [n];
	for (i = 0, j = 0; j < n; j++) {
		points[j].x = doubles [i++];
		points[j].y = doubles [i++];
	}
	g_free (doubles);
	*count = n;
	return points;
}
