/*
 * moon-path.c: Path-based API, similar to cairo but without requiring a cairo_context_t
 *
 * Author:
 *	Sebastien Pouliot  <sebastien@ximian.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
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
	if (!path)
		g_warning ("moon_path_clear(NULL)");

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
	if (!path)
		g_warning ("moon_path_destory(NULL)");

	if (path->allocated > 0)
		g_free (path->cairo.data);
	g_free (path);
}

#define CHECK_SPACE(path,size)	(path->cairo.num_data + size <= path->allocated)

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
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	
	if (!CHECK_SPACE (path, MOON_PATH_MOVE_TO_LENGTH)) {
		while (n < pos + MOON_PATH_MOVE_TO_LENGTH)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}
	
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
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	
	if (!CHECK_SPACE (path, MOON_PATH_LINE_TO_LENGTH)) {
		while (n < pos + MOON_PATH_LINE_TO_LENGTH)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}
	
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
 * @x: a double with the x coordinate
 * @y: a double with the y coordinate
 *
 * Record a cubic bezier curve operation (x1,y1 x2,y2 x3,y3) 
 * in the #moon_path.
 **/
void
moon_curve_to (moon_path *path, double x1, double y1, double x2, double y2, double x3, double y3)
{
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	
	if (!CHECK_SPACE (path, MOON_PATH_CURVE_TO_LENGTH)) {
		while (n < pos + MOON_PATH_CURVE_TO_LENGTH)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}
	
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
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	double rx = w / 2.0;
	double ry = h / 2.0;
	double cx = x + rx;
	double cy = y + ry;
	double brx = ARC_TO_BEZIER * rx;
	double bry = ARC_TO_BEZIER * ry;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	
	if (!CHECK_SPACE (path, MOON_PATH_ELLIPSE_LENGTH)) {
		while (n < pos + MOON_PATH_ELLIPSE_LENGTH)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}
	
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
	data[pos].point.y = cy - bry;
	pos++;
	data[pos].point.x = cx + brx;
	data[pos].point.y = cy - ry;
	pos++;
	data[pos].point.x = cx;
	data[pos].point.y = cy - ry;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx - brx;
	data[pos].point.y = cy - ry;
	pos++;
	data[pos].point.x = cx - rx;
	data[pos].point.y = cy - bry;
	pos++;
	data[pos].point.x = cx - rx;
	data[pos].point.y = cy;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx - rx;
	data[pos].point.y = cy + bry;
	pos++;
	data[pos].point.x = cx - brx;
	data[pos].point.y = cy + ry;
	pos++;
	data[pos].point.x = cx;
	data[pos].point.y = cy + ry;
	pos++;
	data[pos].header.type = CAIRO_PATH_CURVE_TO;
	data[pos].header.length = MOON_PATH_CURVE_TO_LENGTH;
	pos++;
	data[pos].point.x = cx + brx;
	data[pos].point.y = cy + ry;
	pos++;
	data[pos].point.x = cx + rx;
	data[pos].point.y = cy + bry;
	pos++;
	data[pos].point.x = cx + rx;
	data[pos].point.y = cy;
	pos++;
	data[pos].header.type = CAIRO_PATH_CLOSE_PATH;
	data[pos].header.length = MOON_PATH_CLOSE_PATH_LENGTH;
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
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	
	if (!CHECK_SPACE (path, MOON_PATH_RECTANGLE_LENGTH)) {
		while (n < pos + MOON_PATH_RECTANGLE_LENGTH)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}
	
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
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	
	if (!CHECK_SPACE (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH)) {
		while (n < pos + MOON_PATH_ROUNDED_RECTANGLE_LENGTH)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}
	
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
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	
	if (!CHECK_SPACE (path, MOON_PATH_CLOSE_PATH_LENGTH)) {
		while (n < pos + MOON_PATH_CLOSE_PATH_LENGTH)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}
	
	data[pos].header.type = CAIRO_PATH_CLOSE_PATH;
	data[pos].header.length = MOON_PATH_CLOSE_PATH_LENGTH;
	path->cairo.num_data += MOON_PATH_CLOSE_PATH_LENGTH;
}

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
	int i = 0;
	double x = 0.0, y = 0.0;
	cairo_path_t *c_path;

	g_return_if_fail (path != NULL);

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

	if (*ox) *ox = x;
	if (*oy) *oy = y;
}

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
	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	int n = 1;
	
	g_return_if_fail (path != NULL);
	g_return_if_fail (subpath != NULL);
	
	if (!CHECK_SPACE (path, subpath->cairo.num_data)) {
		while (n < pos + subpath->cairo.num_data)
			n <<= 1;
		
		data = g_realloc (data, sizeof (cairo_path_data_t) * n);
		path->cairo.data = data;
		path->allocated = n;
	}

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
	int i = 0;
	g_warning ("path %p status %d, num_data %d", path, path->status, path->num_data);
	for (; i < path->num_data; i+= path->data[i].header.length) {
		cairo_path_data_t *data = &path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_CURVE_TO:
			g_warning ("\tCAIRO_PATH_CURVE_TO (%d) %g,%g - %g,%g - %g,%g", data->header.length, 
				data[1].point.x, data[1].point.y, data[2].point.x, data[2].point.y, data[3].point.x, data[3].point.y);
			break;
		case CAIRO_PATH_LINE_TO:
			g_warning ("\tCAIRO_PATH_LINE_TO (%d) %g,%g", data->header.length, data[1].point.x, data[1].point.y);
			break;
		case CAIRO_PATH_MOVE_TO:
			g_warning ("\tCAIRO_PATH_MOVE_TO (%d) %g,%g", data->header.length, data[1].point.x, data[1].point.y);
			break;
		case CAIRO_PATH_CLOSE_PATH:
			g_warning ("\tCAIRO_PATH_CLOSE_PATH (%d)", data->header.length);
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
