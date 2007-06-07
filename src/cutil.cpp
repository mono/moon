//
// Code in this file are internal Cairo utilities that are not
// exposed by Cairo.
//
// This code is licensed under these terms:
/* Copyright © 2002 University of Southern California
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *      Carl D. Worth <cworth@cworth.org>
 *      Robert O'Callahan <rocallahan@novell.com>
 */
#include <config.h>
#include <glib.h>
#include <cairo.h>
#include "cutil.h"

void
x_cairo_matrix_transform_bounding_box (const cairo_matrix_t *matrix,
				       double *x1, double *y1,
				       double *x2, double *y2)
{
	int i;
	double quad_x[4], quad_y[4];
	double min_x, max_x;
	double min_y, max_y;
	
	quad_x[0] = *x1;
	quad_y[0] = *y1;
	cairo_matrix_transform_point (matrix, &quad_x[0], &quad_y[0]);
	
	quad_x[1] = *x2;
	quad_y[1] = *y1;
	cairo_matrix_transform_point (matrix, &quad_x[1], &quad_y[1]);
	
	quad_x[2] = *x1;
	quad_y[2] = *y2;
	cairo_matrix_transform_point (matrix, &quad_x[2], &quad_y[2]);
	
	quad_x[3] = *x2;
	quad_y[3] = *y2;
	cairo_matrix_transform_point (matrix, &quad_x[3], &quad_y[3]);
	
	min_x = max_x = quad_x[0];
	min_y = max_y = quad_y[0];
	
	for (i=1; i < 4; i++) {
		if (quad_x[i] < min_x)
			min_x = quad_x[i];
		if (quad_x[i] > max_x)
			max_x = quad_x[i];
		
		if (quad_y[i] < min_y)
			min_y = quad_y[i];
		if (quad_y[i] > max_y)
			max_y = quad_y[i];
	}
	
	*x1 = min_x;
	*y1 = min_y;
	*x2 = max_x;
	*y2 = max_y;
}
