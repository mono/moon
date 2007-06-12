/* This is (modified) LGPL code from librsvg 2.16.1

   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Raph Levien <raph@artofcode.com>
*/

#include <config.h>
#include <glib.h>
#include <math.h>
#include <cairo.h>
#include "rsvg.h"

static void
rsvg_path_arc_segment (cairo_t *ctx,
		      double xc, double yc,
		      double th0, double th1,
		      double rx, double ry, double x_axis_rotation)
{
  double sin_th, cos_th;
  double a00, a01, a10, a11;
  double x1, y1, x2, y2, x3, y3;
  double t;
  double th_half;

  sin_th = sin (x_axis_rotation * (M_PI / 180.0));
  cos_th = cos (x_axis_rotation * (M_PI / 180.0)); 
  /* inverse transform compared with rsvg_path_arc */
  a00 = cos_th * rx;
  a01 = -sin_th * ry;
  a10 = sin_th * rx;
  a11 = cos_th * ry;

  th_half = 0.5 * (th1 - th0);
  t = (8.0 / 3.0) * sin (th_half * 0.5) * sin (th_half * 0.5) / sin (th_half);
  x1 = xc + cos (th0) - t * sin (th0);
  y1 = yc + sin (th0) + t * cos (th0);
  x3 = xc + cos (th1);
  y3 = yc + sin (th1);
  x2 = x3 + t * sin (th1);
  y2 = y3 - t * cos (th1);
  cairo_curve_to (ctx,
				  a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
				  a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
				  a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

/**
 * rsvg_path_arc: Add an RSVG arc to the path context.
 * @ctx: Path context.
 * @rx: Radius in x direction (before rotation).
 * @ry: Radius in y direction (before rotation).
 * @x_axis_rotation: Rotation angle for axes.
 * @large_arc_flag: 0 for arc length <= 180, 1 for arc >= 180.
 * @sweep: 0 for "negative angle", 1 for "positive angle".
 * @x: New x coordinate.
 * @y: New y coordinate.
 *
 **/
void
rsvg_arc_to (cairo_t *ctx, double rx, double ry, double x_axis_rotation, int large_arc_flag, int sweep_flag,
	double x, double y)
{
  double sin_th, cos_th;
  double a00, a01, a10, a11;
  double x0, y0, x1, y1, xc, yc;
  double d, sfactor, sfactor_sq;
  double th0, th1, th_arc;
  int i, n_segs;

  /* Check that neither radius is zero, since its isn't either
     geometrically or mathematically meaningful and will
     cause divide by zero and subsequent NaNs.  We should
     really do some ranged check ie -0.001 < x < 000.1 rather
     can just a straight check again zero.
  */
  if ((rx == 0.0) || (ry == 0.0)) return;

  sin_th = sin (x_axis_rotation * (M_PI / 180.0));
  cos_th = cos (x_axis_rotation * (M_PI / 180.0));
  a00 = cos_th / rx;
  a01 = sin_th / rx;
  a10 = -sin_th / ry;
  a11 = cos_th / ry;

	double cpx, cpy;
	cairo_get_current_point (ctx, &cpx, &cpy);

  x0 = a00 * cpx + a01 * cpy;
  y0 = a10 * cpx + a11 * cpy;
  x1 = a00 * x + a01 * y;
  y1 = a10 * x + a11 * y;
  /* (x0, y0) is current point in transformed coordinate space.
     (x1, y1) is new point in transformed coordinate space.

     The arc fits a unit-radius circle in this space.
  */
  d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
  sfactor_sq = 1.0 / d - 0.25;
  if (sfactor_sq < 0) sfactor_sq = 0;
  sfactor = sqrt (sfactor_sq);
  if (sweep_flag == large_arc_flag) sfactor = -sfactor;
  xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
  yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
  /* (xc, yc) is center of the circle. */

  th0 = atan2 (y0 - yc, x0 - xc);
  th1 = atan2 (y1 - yc, x1 - xc);

  th_arc = th1 - th0;
  if (th_arc < 0 && sweep_flag)
    th_arc += 2 * M_PI;
  else if (th_arc > 0 && !sweep_flag)
    th_arc -= 2 * M_PI;

  n_segs = ceil (fabs (th_arc / (M_PI * 0.5 + 0.001)));

  for (i = 0; i < n_segs; i++)
    rsvg_path_arc_segment (ctx, xc, yc,
			  th0 + i * th_arc / n_segs,
			  th0 + (i + 1) * th_arc / n_segs,
			  rx, ry, x_axis_rotation);

//  ctx->cpx = x;
//  ctx->cpy = y;
}
