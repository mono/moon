/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * moon-path.c: Path-based API, similar to cairo but without requiring a cairo_context_t
 *
 * Author:
 *	Sebastien Pouliot  <sebastien@ximian.com>
 *
 * Copyright 2007, 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "moon-path.h"


/**
 * moon_path_new:
 * @size: the number of items to hold
 *
 * The number of items varies for each operation (MOVE_TO, LINE_TO,
 * CURVE_TO and CLOSE_PATH). The caller has the responsability to
 * calculate the required number of items.
 *
 * Return value: the allocated #moon_path
 **/
moon_path*
moon_path_new (int size)
{
	moon_path* path = g_new0 (moon_path, 1);
	path->allocated = size;
	path->cairo.status = CAIRO_STATUS_SUCCESS;
	path->cairo.data = g_new0 (cairo_path_data_t, size);
	path->cairo.num_data = 0;
	return path;
}

/**
 * moon_path_renew:
 * @path: an existing #moon_path or NULL
 * @size: the number of items to hold
 *
 * The number of items varies for each operation (MOVE_TO, LINE_TO,
 * CURVE_TO and CLOSE_PATH). The caller has the responsability to
 * calculate the required number of items.
 *
 * Return value: the existing #moon_path (if large enough) or a new one
 **/
moon_path*
moon_path_renew (moon_path* path, int size)
{
	if (!path)
		return moon_path_new (size);

	if (path->allocated < size) {
		/* not enough space, destroy and recreate */
		moon_path_destroy (path);
		return moon_path_new (size);
	}

	/* we can reuse the already allocated structure */
	moon_path_clear (path);
	return path;
}

/**
 * moon_path_clear:
 * @path: an existing #moon_path
 *
 * Clear the #moon_path structure so it can be reused for a path
 * of the same size.
 **/
void
moon_path_clear (moon_path* path)
{
	g_return_if_fail (path != NULL);

	path->cairo.status = CAIRO_STATUS_SUCCESS;
	memset (path->cairo.data, 0, path->allocated * sizeof (cairo_path_data_t));
	path->cairo.num_data = 0;
}

/**
 * moon_path_destroy:
 * @path: a #moon_path
 *
 * Free the specified #moon_path
 **/
void
moon_path_destroy (moon_path* path)
{
	g_return_if_fail (path != NULL);

	if (path->allocated > 0)
		g_free (path->cairo.data);
	g_free (path);
}

/**
 * moon_get_current_point:
 * @path: a #moon_path
 * @x: pointer to a double (x coordinate)
 * @y: pointer to a double (y coordinate)
 *
 * Get the current point (x,y) on the moon_path. By default (empty path)
 * this is (0,0)
 **/
void
moon_get_current_point (moon_path *path, double *x, double *y)
{
	if (!path || !x || !y) {
		g_warning ("moon_get_current_point(%p,%p,%p)", path, x, y);
		return;
	}

	int pos = path->cairo.num_data - 1;
	if (pos > 0) {
		cairo_path_data_t *data = path->cairo.data;
		*x = data[pos].point.x;
		*y = data[pos].point.y;
	} else {
		*x = 0.0;
		*y = 0.0;
	}
}

/* this is a total soptimization, but I don't care ;-) */
static inline guint32
nearest_pow2 (guint32 num)
{
	guint32 n;
	
	if (num == 0)
		return 0;
	
	n = num - 1;
#if defined (__GNUC__) && defined (__i386__)
	__asm__("bsrl %1,%0\n\t"
		"jnz 1f\n\t"
		"movl $-1,%0\n"
		"1:" : "=r" (n) : "rm" (n));
	n = (1 << (n + 1));
#else
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
#endif
	
	return n;
}

static inline gboolean
moon_path_ensure_space (moon_path *path, int need)
{
	void *data;
	guint32 n;
	
	if (path->cairo.num_data + need <= path->allocated)
		return TRUE;
	
	n = nearest_pow2 (path->cairo.num_data + need);
	if (!(data = g_try_realloc (path->cairo.data, sizeof (cairo_path_data_t) * n)))
		return FALSE;
	
	path->cairo.data = (cairo_path_data_t *) data;
	path->allocated = n;
	
	return TRUE;
}

/**
 * moon_move_to:
 * @path: a #moon_path
 * @x: a double with the x coordinate
 * @y: a double with the y coordinate
 *
 * Record a move operation to x,y in the #moon_path.
 **/
void
moon_move_to (moon_path *path, double x, double y)
{
	g_return_if_fail (path != NULL);
	
	if (!moon_path_ensure_space (path, MOON_PATH_MOVE_TO_LENGTH))
		return;
	
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	
	data[pos].header.type = CAIRO_PATH_MOVE_TO;
	data[pos].header.length = MOON_PATH_MOVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y;
	path->cairo.num_data += MOON_PATH_MOVE_TO_LENGTH;
}

/**
 * moon_line_to:
 * @path: a #moon_path
 * @x: a double with the x coordinate
 * @y: a double with the y coordinate
 *
 * Record a line operation to x,y in the #moon_path.
 **/
void
moon_line_to (moon_path *path, double x, double y)
{
	g_return_if_fail (path != NULL);
	
	if (!moon_path_ensure_space (path, MOON_PATH_LINE_TO_LENGTH))
		return;
	
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y;
	path->cairo.num_data += MOON_PATH_LINE_TO_LENGTH;
}

/**
 * moon_curve_to:
 * @path: a #moon_path
 * @x1: a double with the x coordinate of the first point
 * @y1: a double with the y coordinate of the first point
 * @x2: a double with the x coordinate of the second point
 * @y2: a double with the y coordinate of the second point
 * @x3: a double with the x coordinate of the third point
 * @y3: a double with the y coordinate of the third point
 *
 * Record a cubic bezier curve operation (x1,y1 x2,y2 x3,y3) 
 * in the #moon_path.
 **/
void
moon_curve_to (moon_path *path, double x1, double y1, double x2, double y2, double x3, double y3)
{
	g_return_if_fail (path != NULL);
	
	if (!moon_path_ensure_space (path, MOON_PATH_CURVE_TO_LENGTH))
		return;
	
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x1;
	data[pos].point.y = y1;
	pos++;
	data[pos].point.x = x2;
	data[pos].point.y = y2;
	pos++;
	data[pos].point.x = x3;
	data[pos].point.y = y3;
	path->cairo.num_data += MOON_PATH_CURVE_TO_LENGTH;
}

/**
 * moon_quad_curve_to:
 * @path: a #moon_path
 * @x1: a double with the x coordinate of the first point
 * @y1: a double with the y coordinate of the first point
 * @x2: a double with the x coordinate of the second point
 * @y2: a double with the y coordinate of the second point
 *
 * Record the quadratic bezier curve operation (x1,y1 x2,y2) 
 * as a (transformed into) cubic bezier curve in the #moon_path.
 *
 * quadratic to cubic bezier, the original control point and the end control point are the same
 * http://web.archive.org/web/20020209100930/http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/node2.html
 **/
void
moon_quad_curve_to (moon_path* path, double x1, double y1, double x2, double y2)
{
	g_return_if_fail (path != NULL);

	double x0, y0;
	double x3 = x2;
	double y3 = y2;

	moon_get_current_point (path, &x0, &y0);

	x2 = x1 + (x2 - x1) / 3;
	y2 = y1 + (y2 - y1) / 3;
	x1 = x0 + 2 * (x1 - x0) / 3;
	y1 = y0 + 2 * (y1 - y0) / 3;

	moon_curve_to (path, x1, y1, x2, y2, x3, y3);
}

/**
 * moon_arc_to:
 * @path: a #moon_path
 * @width: a double with the horizontal size of the arc
 * @height: a double with the vertical size of the arc
 * @large: a boolean to indicate if this is a large arc
 * @sweep: a boolean to indicate the sweep direction
 * @ex: a double with the x coordinate of the end point
 * @ey: a double with the y coordinate of the end point
 *
 * Record the arc as multiple cubic curves operation
 * in the #moon_path.
 */
void
moon_arc_to (moon_path *path, double width, double height, double angle, gboolean large, gboolean sweep, double ex, double ey)
{
	g_return_if_fail (path != NULL);

	// from tests it seems that Silverlight closely follows SVG arc 
	// behavior (which is very different from the model used with GDI+)
	// some helpful stuff is available here:
	// http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes

	// get start point from the existing path
	double sx, sy;
	moon_get_current_point (path, &sx, &sy);

	// if start and end points are identical, then no arc is drawn
	// FIXME: what's the logic (if any) to compare points
	// e.g. 60 and 60.000002 are drawn while 80 and 80.000003 aren't
	if (IS_ZERO (ex - sx) && IS_ZERO (ey - sy))
		return;

	// Correction of out-of-range radii, see F6.6 (step 1)
	if (IS_ZERO (width) || IS_ZERO (height)) {
		// treat this as a straight line (to end point)
		moon_line_to (path, ex, ey);
		return;
	}

	// Silverlight "too small to be useful"
	if (IS_TOO_SMALL (width) || IS_TOO_SMALL (height)) {
		// yes it does mean there's a hole between "normal" FP values and "zero" FP values
		// and SL doesn't render anything in this twilight sonze
		return;
	}

	// Correction of out-of-range radii, see F6.6.1 (step 2)
	double rx = fabs (width);
	double ry = fabs (height);

	// convert angle into radians
	angle = angle * M_PI / 180.0;

	// variables required for F6.3.1
	double cos_phi = cos (angle);
	double sin_phi = sin (angle);
	double dx2 = (sx - ex) / 2.0;
	double dy2 = (sy - ey) / 2.0;
	double x1p = cos_phi * dx2 + sin_phi * dy2;
	double y1p = cos_phi * dy2 - sin_phi * dx2;
	double x1p2 = x1p * x1p;
	double y1p2 = y1p * y1p;
	double rx2 = rx * rx;
	double ry2 = ry * ry;

	// Correction of out-of-range radii, see F6.6.2 (step 4)
	double lambda = (x1p2 / rx2) + (y1p2 / ry2);
	if (lambda > 1.0) {
		// see F6.6.3
		double lambda_root = sqrt (lambda);
		rx *= lambda_root;
		ry *= lambda_root;
		// update rx2 and ry2
		rx2 = rx * rx;
		ry2 = ry * ry;
	}

	double cxp, cyp, cx, cy;
	double c = (rx2 * ry2) - (rx2 * y1p2) - (ry2 * x1p2);

	// check if there is no possible solution (i.e. we can't do a square root of a negative value)
	if (c < 0.0) {
		// scale uniformly until we have a single solution (see F6.2) i.e. when c == 0.0
		double scale = sqrt (1.0 - c / (rx2 * ry2));
		rx *= scale;
		ry *= scale;
		// update rx2 and ry2
		rx2 = rx * rx;
		ry2 = ry * ry;

		// step 2 (F6.5.2) - simplified since c == 0.0
		cxp = 0.0;
		cyp = 0.0;

		// step 3 (F6.5.3 first part) - simplified since cxp and cyp == 0.0
		cx = 0.0;
		cy = 0.0;
	} else {
		// complete c calculation
		c = sqrt (c / ((rx2 * y1p2) + (ry2 * x1p2)));

		// inverse sign if Fa == Fs
		if (large == sweep)
			c = -c;
		
		// step 2 (F6.5.2)
		cxp = c * ( rx * y1p / ry);
		cyp = c * (-ry * x1p / rx);

		// step 3 (F6.5.3 first part)
		cx = cos_phi * cxp - sin_phi * cyp;
		cy = sin_phi * cxp + cos_phi * cyp;
	}

	// step 3 (F6.5.3 second part) we now have the center point of the ellipse
	cx += (sx + ex) / 2.0;
	cy += (sy + ey) / 2.0;

	// step 4 (F6.5.4)
	// we dont' use arccos (as per w3c doc), see http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm
	// note: atan2 (0.0, 1.0) == 0.0
	double at = atan2 (((y1p - cyp) / ry), ((x1p - cxp) / rx));
	double theta1 = (at < 0.0) ? 2.0 * M_PI + at : at;

	double nat = atan2 (((-y1p - cyp) / ry), ((-x1p - cxp) / rx));
	double delta_theta = (nat < at) ? 2.0 * M_PI - at + nat : nat - at;

	if (sweep) {
		// ensure delta theta < 0 or else add 360 degrees
		if (delta_theta < 0.0)
			delta_theta += 2.0 * M_PI;
	} else {
		// ensure delta theta > 0 or else substract 360 degrees
		if (delta_theta > 0.0)
			delta_theta -= 2.0 * M_PI;
	}

	// add several cubic bezier to approximate the arc (smaller than 90 degrees)
	// we add one extra segment because we want something smaller than 90deg (i.e. not 90 itself)
	int segments = (int) (fabs (delta_theta / M_PI_2)) + 1;
	double delta = delta_theta / segments;

	// http://www.stillhq.com/ctpfaq/2001/comp.text.pdf-faq-2001-04.txt (section 2.13)
	double bcp = 4.0 / 3 * (1 - cos (delta / 2)) / sin (delta / 2);

	double cos_phi_rx = cos_phi * rx;
	double cos_phi_ry = cos_phi * ry;
	double sin_phi_rx = sin_phi * rx;
	double sin_phi_ry = sin_phi * ry;

	double cos_theta1 = cos (theta1);
	double sin_theta1 = sin (theta1);
	
	if (!moon_path_ensure_space (path, segments * MOON_PATH_CURVE_TO_LENGTH))
		return;
	
	int i;
	for (i = 0; i < segments; ++i) {
		// end angle (for this segment) = current + delta
		double theta2 = theta1 + delta;
		double cos_theta2 = cos (theta2);
		double sin_theta2 = sin (theta2);

		// first control point (based on start point sx,sy)
		double c1x = sx - bcp * (cos_phi_rx * sin_theta1 + sin_phi_ry * cos_theta1);
		double c1y = sy + bcp * (cos_phi_ry * cos_theta1 - sin_phi_rx * sin_theta1);

		// end point (for this segment)
		double ex = cx + (cos_phi_rx * cos_theta2 - sin_phi_ry * sin_theta2);
		double ey = cy + (sin_phi_rx * cos_theta2 + cos_phi_ry * sin_theta2);

		// second control point (based on end point ex,ey)
		double c2x = ex + bcp * (cos_phi_rx * sin_theta2 + sin_phi_ry * cos_theta2);
		double c2y = ey + bcp * (sin_phi_rx * sin_theta2 - cos_phi_ry * cos_theta2);

		moon_curve_to (path, c1x, c1y, c2x, c2y, ex, ey);

		// next start point is the current end point (same for angle)
		sx = ex;
		sy = ey;
		theta1 = theta2;
		// avoid recomputations
		cos_theta1 = cos_theta2;
		sin_theta1 = sin_theta2;
	}
}

/**
 * moon_ellipse:
 * @path: a #moon_path
 * @x: a double with the left-most coordinate of the ellipse
 * @y: a double with the top-most coordinate of the ellipse
 * @w: a double with the width of the ellipse
 * @h: a double with the height of the ellipse
 *
 * Record a series of basic operations that correspond to an ellipse in
 * the #moon_path. Note that the x,y aren't the center of the ellipse.
 **/
void
moon_ellipse (moon_path *path, double x, double y, double w, double h)
{
	g_return_if_fail (path != NULL);
	
	double rx = w / 2.0;
	double ry = h / 2.0;
	double cx = x + rx;
	double cy = y + ry;
	double brx = ARC_TO_BEZIER * rx;
	double bry = ARC_TO_BEZIER * ry;

	if (!moon_path_ensure_space (path, MOON_PATH_ELLIPSE_LENGTH))
		return;
	
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	
	data[pos].header.type = CAIRO_PATH_MOVE_TO;
	data[pos].header.length = MOON_PATH_MOVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx + rx;
	data[pos].point.y = cy;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx + rx;
	data[pos].point.y = cy + bry;
	pos++;
	data[pos].point.x = cx + brx;
	data[pos].point.y = cy + ry;
	pos++;
	data[pos].point.x = cx;
	data[pos].point.y = cy + ry;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx - brx;
	data[pos].point.y = cy + ry;
	pos++;
	data[pos].point.x = cx - rx;
	data[pos].point.y = cy + bry;
	pos++;
	data[pos].point.x = cx - rx;
	data[pos].point.y = cy;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx - rx;
	data[pos].point.y = cy - bry;
	pos++;
	data[pos].point.x = cx - brx;
	data[pos].point.y = cy - ry;
	pos++;
	data[pos].point.x = cx;
	data[pos].point.y = cy - ry;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx + brx;
	data[pos].point.y = cy - ry;
	pos++;
	data[pos].point.x = cx + rx;
	data[pos].point.y = cy - bry;
	pos++;
	data[pos].point.x = cx + rx;
	data[pos].point.y = cy;
	path->cairo.num_data += MOON_PATH_ELLIPSE_LENGTH;
}

/**
 * moon_rectangle:
 * @path: a #moon_path
 * @x: a double with the left-most coordinate of the rectangle
 * @y: a double with the top-most coordinate of the rectangle
 * @w: a double with the width of the rectangle
 * @h: a double with the height of the rectangle
 *
 * Record a series of basic operations that correspond to a rectangle 
 * in the #moon_path.
 **/
void
moon_rectangle (moon_path *path, double x, double y, double w, double h)
{
	g_return_if_fail (path != NULL);
	
	if (!moon_path_ensure_space (path, MOON_PATH_RECTANGLE_LENGTH))
		return;
	
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	
	data[pos].header.type = CAIRO_PATH_MOVE_TO;
	data[pos].header.length = MOON_PATH_MOVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y;
	pos++;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + w;
	data[pos].point.y = y;
	pos++;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + w;
	data[pos].point.y = y + h;
	pos++;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y + h;
	pos++;
	data[pos].header.type = CAIRO_PATH_CLOSE_PATH;
	data[pos].header.length = MOON_PATH_CLOSE_PATH_LENGTH;
	path->cairo.num_data += MOON_PATH_RECTANGLE_LENGTH;
}

/**
 * moon_rounded_rectangle:
 * @path: a #moon_path
 * @x: a double with the left-most coordinate of the rectangle
 * @y: a double with the top-most coordinate of the rectangle
 * @w: a double with the width of the rectangle
 * @h: a double with the height of the rectangle
 * @radius_x: a double with the x radius of the rounded corner
 * @radius_y: a double with the y radius of the rounded corner
 *
 * Record a series of basic operations that correspond to a rectangle 
 * with rounded corners in the #moon_path.
 **/
void
moon_rounded_rectangle (moon_path *path, double x, double y, double w, double h, double radius_x, double radius_y)
{
	g_return_if_fail (path != NULL);
	
	if (!moon_path_ensure_space (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH))
		return;
	
	if (radius_x < 0.0)
		radius_x = -radius_x;
	if (radius_y < 0.0)
		radius_y = -radius_y;

	// test limits (without using multiplications)
	if (radius_x > w - radius_x)
		radius_x = w / 2;
	if (radius_y > h - radius_y)
		radius_y = h / 2;

	// approximate (quite close) the arc using a bezier curve
	double c1 = ARC_TO_BEZIER * radius_x;
	double c2 = ARC_TO_BEZIER * radius_y;

	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	
	data[pos].header.type = CAIRO_PATH_MOVE_TO;
	data[pos].header.length = MOON_PATH_MOVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + radius_x;
	data[pos].point.y = y;
	pos++;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + w - radius_x;
	data[pos].point.y = y;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + w - radius_x + c1;
	data[pos].point.y = y;
	pos++;
	data[pos].point.x = x + w;
	data[pos].point.y = y + c2;
	pos++;
	data[pos].point.x = x + w;
	data[pos].point.y = y + radius_y;
	pos++;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + w;
	data[pos].point.y = y + h - radius_y;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + w;
	data[pos].point.y = y + h - radius_y + c2;
	pos++;
	data[pos].point.x = x + w + c1 - radius_x;
	data[pos].point.y = y + h;
	pos++;
	data[pos].point.x = x + w - radius_x;
	data[pos].point.y = y + h;
	pos++;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + radius_x;
	data[pos].point.y = y + h;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x + radius_x - c1;
	data[pos].point.y = y + h;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y + h - c2;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y + h - radius_y;
	pos++;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_LINE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y + radius_y;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y + radius_y - c2;
	pos++;
	data[pos].point.x = x + radius_x - c1;
	data[pos].point.y = y;
	pos++;
	data[pos].point.x = x + radius_x;
	data[pos].point.y = y;
	pos++;
	data[pos].header.type = CAIRO_PATH_CLOSE_PATH;
	data[pos].header.length = MOON_PATH_CLOSE_PATH_LENGTH;

	path->cairo.num_data += MOON_PATH_ROUNDED_RECTANGLE_LENGTH;
}

/**
 * moon_close_path:
 * @path: a #moon_path
 *
 * Record a close operation in the #moon_path.
 **/
void
moon_close_path (moon_path *path)
{
	g_return_if_fail (path != NULL);
	
	if (!moon_path_ensure_space (path, MOON_PATH_CLOSE_PATH_LENGTH))
		return;
	
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	
	data[pos].header.type = CAIRO_PATH_CLOSE_PATH;
	data[pos].header.length = MOON_PATH_CLOSE_PATH_LENGTH;
	path->cairo.num_data += MOON_PATH_CLOSE_PATH_LENGTH;
}

#if FALSE
/**
 * moon_get_origin
 * @path: a #moon_path
 * @ox: a pointer to the double for the minimal X value of the path
 * @oy: a pointer to the double for the minimal Y value of the path
 *
 * Get the origin point of the path.
 **/
void
moon_get_origin (moon_path *path, double *ox, double *oy)
{
	g_return_if_fail (path != NULL);

	int i = 0;
	double x = 0.0, y = 0.0;
	cairo_path_t *c_path;

	c_path = &path->cairo;
	for (; i < c_path->num_data; i+= c_path->data[i].header.length) {
		cairo_path_data_t *data = &c_path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_CURVE_TO:
			if (i == 0) {
				x = data[1].point.x; 
				y = data[1].point.y; 
			} else {
				x = MIN (x, data[1].point.x);
				y = MIN (y, data[1].point.y);
			}
			x = MIN (x, data[2].point.x);
			y = MIN (y, data[2].point.y);
			x = MIN (x, data[3].point.x);
			y = MIN (y, data[3].point.y);
			break;
		case CAIRO_PATH_LINE_TO:
		case CAIRO_PATH_MOVE_TO:
			if (i == 0) {
				x = data[1].point.x; 
				y = data[1].point.y; 
			} else {
				x = MIN (x, data[1].point.x);
				y = MIN (y, data[1].point.y);
			}
			break;
		default:
			break;
		}
	}

	if (ox) *ox = x;
	if (oy) *oy = y;
}
#endif

/**
 * moon_merge:
 * @path: a #moon_path
 * @subpath: the #moon_path to merge into path
 *
 * Merge 'subpath' into 'path'.
 **/
void
moon_merge (moon_path *path, moon_path *subpath)
{
	g_return_if_fail (path != NULL);
	g_return_if_fail (subpath != NULL);
	
	if (!moon_path_ensure_space (path, subpath->cairo.num_data))
		return;
	
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;

	memcpy (&data [pos], subpath->cairo.data, subpath->cairo.num_data * sizeof (cairo_path_data_t));
	path->cairo.num_data += subpath->cairo.num_data;
}

/**
 * cairo_path_display:
 * @path: a #cairo_path_t
 *
 * Display the content of the #cairo_path_t on the console.
 * For debugging purpose only.
 **/
void
cairo_path_display (cairo_path_t *path)
{
#if FALSE
	g_return_if_fail (path != NULL);

	int i = 0;
	g_warning ("path %p status %d, num_data %d", path, path->status, path->num_data);
	for (; i < path->num_data; i+= path->data[i].header.length) {
		cairo_path_data_t *data = &path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_CURVE_TO:
			g_warning ("\tCAIRO_PATH_CURVE_TO (size %d) (%g, %g) (%g, %g) (%g, %g)", data->header.length, 
				data[1].point.x, data[1].point.y, data[2].point.x, data[2].point.y, data[3].point.x, data[3].point.y);
			break;
		case CAIRO_PATH_LINE_TO:
			g_warning ("\tCAIRO_PATH_LINE_TO (size %d) (%g, %g)", data->header.length, data[1].point.x, data[1].point.y);
			break;
		case CAIRO_PATH_MOVE_TO:
			g_warning ("\tCAIRO_PATH_MOVE_TO (size %d) (%g, %g)", data->header.length, data[1].point.x, data[1].point.y);
			break;
		case CAIRO_PATH_CLOSE_PATH:
			g_warning ("\tCAIRO_PATH_CLOSE_PATH (size %d)", data->header.length);
			break;
		}
	}
#endif
}

/**
 * moon_path_display:
 * @path: a #moon_path
 *
 * Display the content of the #moon_path on the console.
 * For debugging purpose only.
 **/
void
moon_path_display (moon_path *path)
{
	cairo_path_display (&path->cairo);
}
