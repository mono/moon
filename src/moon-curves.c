/*
 * moon-curves.c: A set of primitives + alghos for solving Y at X problmem
 *		  for cubic curves. This is used ONLY for KeySpline animation, 
 *		  not used in any way for drawing.
 *
 * Author:
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "moon-curves.h"

static void 
point_half_lerp (moon_point *dest, moon_point a, moon_point b)
{
	dest->x = a.x + (b.x - a.x) * 0.5;
	dest->y = a.y + (b.y - a.y) * 0.5;
}

void 
moon_quadratic_from_cubic (moon_quadratic *dest, moon_cubic *src)
{
	dest->c0.x = src->c0.x; dest->c0.y = src->c0.y;
	
	dest->c1.x = (src->c1.x + src->c2.x) / 2.0;
	dest->c1.y = (src->c1.y + src->c2.y) / 2.0;
	
	dest->c2.x = src->c3.x; dest->c2.y = src->c3.y;
}

double 
moon_quadratic_y_for_x (double x, moon_quadratic *src)
{
	double l = src->c2.x - src->c0.x;
	if (l <= 0)
		return 0.0;
	else {
		x = (x - src->c0.x) / l;
		return ((1 - x) * (1 - x)) * src->c0.y + ((2 * x) * (1 - x) * src->c1.y) + ((x * x) * src->c2.y);
	}
}

double 
moon_quadratic_array_y_for_x (moon_quadratic *qarr, double x, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (x < qarr [i].c2.x) 
			return moon_quadratic_y_for_x (x, &qarr [i]);
	} 

	g_warning ("Failed to find a matching quadratic segment for %.5f", x);
	return 0.0;
}

void 
moon_convert_cubics_to_quadratics (moon_quadratic *dest_array, moon_cubic *src_array, int count)
{
	int i = 0;
	for (i = 0; i < count; i++)
		moon_quadratic_from_cubic (&dest_array [i], &src_array [i]);
}

void 
moon_subdivide_cubic (moon_cubic *dest1, moon_cubic *dest2, moon_cubic *src)
{
	moon_point p01, p012, p0123;
	moon_point p12, p23, p123;
	
	point_half_lerp (&p01, src->c0, src->c1);
	point_half_lerp (&p12, src->c1, src->c2);
	point_half_lerp (&p23, src->c2, src->c3);
	
	point_half_lerp (&p012, p01, p12);
	
	point_half_lerp (&p123, p12, p23);
	point_half_lerp (&p0123, p012, p123);
	
	dest1->c0 = src->c0;
	dest1->c1 = p01;
	dest1->c2 = p012;
	dest1->c3 = p0123;

	dest2->c0 = p0123;
	dest2->c1 = p123;
	dest2->c2 = p23;
	dest2->c3 = src->c3;
}

static void 
recursive_subdivide_func (moon_cubic *b, int lvl, int current_lvl, int *current_pos, moon_cubic *src)
{
	moon_cubic b1, b2;
	moon_subdivide_cubic (&b1, &b2, src);
	
	if (current_lvl == lvl) {
		b [*current_pos] = b1;
		b [*current_pos + 1] = b2;
		*current_pos += 2;
	} else {
		recursive_subdivide_func (b, lvl, current_lvl + 1, current_pos, &b1);
		recursive_subdivide_func (b, lvl, current_lvl + 1, current_pos, &b2);
	}
}

void
moon_subdivide_cubic_at_level (moon_cubic *b, int lvl, moon_cubic *src)
{
	int pos = 0;
	recursive_subdivide_func (b, lvl, 1, &pos, src);
}
