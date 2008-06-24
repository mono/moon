/*
 * moon-curves.h: A set of primitives + alghos for solving Y at X problmem
 *		  for cubic curves. This is used ONLY for KeySpline animation, 
 *		  not used in any way for drawing.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CURVES_H__
#define __MOON_CURVES_H__

#include <math.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
	double x;
	double y;
} moon_point;

typedef struct
{
	moon_point c0;
	moon_point c1;
	moon_point c2;
	moon_point c3;
} moon_cubic;

typedef struct
{
	moon_point c0;
	moon_point c1;
	moon_point c2;
} moon_quadratic;

void	moon_quadratic_from_cubic		(moon_quadratic *dest, moon_cubic *src);
double	moon_quadratic_y_for_x			(double x, moon_quadratic *src);
double	moon_quadratic_array_y_for_x		(moon_quadratic *qarr, double x, int count);
void	moon_convert_cubics_to_quadratics	(moon_quadratic *dest_array, moon_cubic *src_array, int count);
void	moon_subdivide_cubic			(moon_cubic *dest1, moon_cubic *dest2, moon_cubic *src);
void	moon_subdivide_cubic_at_level		(moon_cubic *b, int lvl, moon_cubic *src);

G_END_DECLS

#endif



