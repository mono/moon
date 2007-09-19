

#include "moon-path.h"


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

void
moon_path_destroy (moon_path* path)
{
	if (path->allocated > 0)
		g_free (path->cairo.data);
	g_free (path);
}

#define CHECK_SPACE(path,size)	(path->cairo.num_data + size <= path->allocated)

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


void
moon_move_to (moon_path *path, double x, double y)
{
	if (!path || !CHECK_SPACE (path, MOON_PATH_MOVE_TO_LENGTH)) {
		g_warning ("moon_move_to(%p,%g,%g)", path, x, y);
		return;
	}

	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	data[pos].header.type = CAIRO_PATH_MOVE_TO;
	data[pos].header.length = MOON_PATH_MOVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y;
	path->cairo.num_data += MOON_PATH_MOVE_TO_LENGTH;
}

void
moon_line_to (moon_path *path, double x, double y)
{
	if (!path || !CHECK_SPACE (path, MOON_PATH_LINE_TO_LENGTH)) {
		g_warning ("moon_line_to(%p,%g,%g)", path, x, y);
		return;
	}

	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	data[pos].header.type = CAIRO_PATH_LINE_TO;
	data[pos].header.length = MOON_PATH_MOVE_TO_LENGTH;
	pos++;
	data[pos].point.x = x;
	data[pos].point.y = y;
	path->cairo.num_data += MOON_PATH_MOVE_TO_LENGTH;
}

void
moon_curve_to (moon_path *path, double x1, double y1, double x2, double y2, double x3, double y3)
{
	if (!path || !CHECK_SPACE (path, MOON_PATH_CURVE_TO_LENGTH)) {
		g_warning ("moon_curve_to(%p,%g,%g,%g,%g,%g,%g)", path, x1, y1, x2, y2, x3, y3);
		return;
	}

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

void
moon_ellipse (moon_path *path, double x, double y, double w, double h)
{
	if (!path || !CHECK_SPACE (path, MOON_PATH_ELLIPSE_LENGTH)) {
		g_warning ("moon_ellipse(%p,%g,%g,%g,%g)", path, x, y, w, h);
		return;
	}

	double rx = w / 2.0;
	double ry = h / 2.0;
	double cx = x + rx;
	double cy = y + ry;
	double brx = ARC_TO_BEZIER * rx;
	double bry = ARC_TO_BEZIER * ry;

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

void
moon_rectangle (moon_path *path, double x, double y, double w, double h)
{
	if (!path || !CHECK_SPACE (path, MOON_PATH_RECTANGLE_LENGTH)) {
		g_warning ("moon_rectangle(%p,%g,%g,%g,%g)", path, x, y, w, h);
		return;
	}

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

void
moon_rounded_rectangle (moon_path *path, double x, double y, double w, double h, double radius_x, double radius_y)
{
	if (!path || !CHECK_SPACE (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH)) {
		g_warning ("moon_rounded_rectangle(%p,%g,%g,%g,%g,%g,%g)", path, x, y, w, h, radius_x, radius_y);
		return;
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

void
moon_close_path (moon_path *path)
{
	if (!path || !CHECK_SPACE (path, MOON_PATH_CLOSE_PATH_LENGTH)) {
		g_warning ("moon_close_path(%p)", path);
		return;
	}

	cairo_path_data_t *data = path->cairo.data;
	int pos = path->cairo.num_data;
	data[pos].header.type = CAIRO_PATH_CLOSE_PATH;
	data[pos].header.length = MOON_PATH_CLOSE_PATH_LENGTH;
	path->cairo.num_data += MOON_PATH_CLOSE_PATH_LENGTH;
}

void
cairo_path_display (cairo_path_t *path)
{
#if TRUE
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

void
moon_path_display (moon_path *path)
{
	cairo_path_display (&path->cairo);
}
