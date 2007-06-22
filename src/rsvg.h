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

#ifndef __RSVG_H__
#define __RSVG_H__

G_BEGIN_DECLS

void rsvg_arc_to (cairo_t *ctx, double rx, double ry, double x_axis_rotation, int large_arc_flag, int sweep_flag,
	double x, double y);

G_END_DECLS

#endif
