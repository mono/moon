/*
 * array.h: array classes to aid marshalling.
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_ARRAY_H__
#define __MOON_ARRAY_H__

#include "runtime.h"

//
// Arrays derive from this format
//
struct BasicArray {
public:
	guint32 count;
	guint32 refcount;	// Double purpose: refcount and pad. 
};


struct PointArray {
 public:
	BasicArray basic;
	Point points [0];
};


struct DoubleArray {
 public:
	BasicArray basic;
	double values [0];
};



G_BEGIN_DECLS

PointArray *point_array_new (int count, Point *points);
Point* point_array_from_str (const char *s, int* count);

double* double_array_from_str   (const char *s, int* count);
DoubleArray *double_array_new   (int count, double *values);

G_END_DECLS

#endif /* __MOON_ARRAY_H__ */
