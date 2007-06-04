/*
 * shape.cpp: This match the classes inside System.Windows.Shapes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "shape.h"

void
FrameworkElement::set_prop_from_str (const char *prop, const char *value)
{
	if (!g_strcasecmp (prop, "canvas.left"))
		x = strtod (value, NULL);
	else if (!g_strcasecmp (prop, "canvas.top"))
		y = strtod (value, NULL);
	else if (!g_strcasecmp (prop, "width"))
		w = strtod (value, NULL);
	else if (!g_strcasecmp (prop, "height"))
		h = strtod (value, NULL);

	// FIXME: else ignore ? or keep somewhere in a collection ?
}

//
// This routine is useful for Shape derivatives: it can be used
// to either get the bounding box from cairo, or to paint it
//
void 
Shape::DoDraw (Surface *s, bool do_op)
{
	cairo_save (s->cairo);
	if (absolute_xform != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) absolute_xform);

	if (fill){
		fill->SetupBrush (s->cairo);
		Draw (s);
		if (do_op)
			cairo_fill (s->cairo);
	}

	if (stroke){
		stroke->SetupBrush (s->cairo);
		Draw (s);
		if (do_op)
			cairo_stroke (s->cairo);
	}
	cairo_restore (s->cairo);
}

void
Shape::render (Surface *s, int x, int y, int width, int height)
{
	DoDraw (s, TRUE);
}

void 
Shape::getbounds ()
{
	double res [6];

	Surface *s = item_get_surface (this);

	// not yet attached
	if (s == NULL)
		return;

	DoDraw (s, FALSE);
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);

	cairo_new_path (s->cairo);
}

void
Shape::set_prop_from_str (const char *prop, const char *value)
{
	if (!g_strcasecmp ("fill", prop)) {
		SolidColorBrush *fill = solid_brush_from_str (value);
		if (fill)
			shape_set_fill (this, fill);
	} else if (!g_strcasecmp ("stroke", prop)) {
		SolidColorBrush *stroke = solid_brush_from_str (value);
		if (stroke)
			shape_set_stroke (this, stroke);
	} else
		FrameworkElement::set_prop_from_str (prop, value);
}

void 
shape_set_fill (Shape *shape, Brush *fill)
{
	if (shape->fill != NULL)
		base_unref (shape->fill);

	base_ref (fill);
	shape->fill = fill;
}

void 
shape_set_stroke (Shape *shape, Brush *stroke)
{
	if (shape->stroke != NULL)
		base_unref (shape->stroke);

	base_ref (stroke);
	shape->stroke = stroke;
}

void
shape_set_stretch (Shape *shape, Stretch stretch)
{
	shape->stretch = stretch;
}

void
shape_set_stroke_dash_cap (Shape *shape, PenLineCap cap)
{
	shape->stroke_dash_cap = cap;
}

void
shape_set_stroke_start_line_cap (Shape *shape, PenLineCap cap)
{
	shape->stroke_start_line_cap = cap;
}

void
shape_set_stroke_end_line_cap (Shape *shape, PenLineCap cap)
{
	shape->stroke_end_line_cap = cap;
}

void
shape_set_stroke_dash_offset (Shape *shape, double offset)
{
	shape->stroke_dash_offset = offset;
}

void
shape_set_stroke_miter_limit (Shape *shape, double limit)
{
	shape->stroke_miter_limit = limit;
}

void
shape_set_stroke_thickness (Shape *shape, double thickness)
{
	shape->stroke_thickness = thickness;
}

void
shape_set_stroke_line_join (Shape *shape, PenLineJoin join)
{
	shape->stroke_line_join = join;
}

void
shape_set_stroke_dash_array (Shape *shape, double* dashes)
{
	shape->stroke_dash_array = dashes;
}

void
Ellipse::Draw (Surface *s)
{
	// TODO
}

Ellipse *
ellipse_new ()
{
	return new Ellipse ();
}

void
Rectangle::Draw (Surface *s)
{
//	static int n;
	// TODO - check radius_x and radius_y for rounded-corner rectangles
	cairo_rectangle (s->cairo, x, y, w, h);
}

void
Rectangle::set_prop_from_str (const char *prop, const char *value)
{
	if (!g_strcasecmp (prop, "radiusx"))
		radius_x = strtod (value, NULL);
	else if (!g_strcasecmp (prop, "radiusy"))
		radius_y = strtod (value, NULL);
	else
		Shape::set_prop_from_str (prop, value);
}

Point
Rectangle::getxformorigin ()
{
	return Point (x + w * user_xform_origin.x, y + h * user_xform_origin.y);
}

Rectangle *
rectangle_new (double x, double y, double w, double h)
{
	Rectangle *rect = new Rectangle ();
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
	return rect;
}

void
Line::Draw (Surface *s)
{
	cairo_move_to (s->cairo, line_x1, line_y1);
	cairo_line_to (s->cairo, line_x2, line_y2);
}

void
Line::set_prop_from_str (const char *prop, const char *value)
{
	if (!g_strcasecmp (prop, "x1"))
		line_x1 = strtod (value, NULL);
	else if (!g_strcasecmp (prop, "y1"))
		line_y1 = strtod (value, NULL);
	else if (!g_strcasecmp (prop, "x2"))
		line_x2 = strtod (value, NULL);
	else if (!g_strcasecmp (prop, "y2"))
		line_y2 = strtod (value, NULL);
	else
		Shape::set_prop_from_str (prop, value);
}

Point
Line::getxformorigin ()
{
	return Point (line_x1 + (line_x2-line_x1) * user_xform_origin.x, 
		      line_y1 + (line_y2-line_y1) * user_xform_origin.y);
}

Line *
line_new (double x1, double y1, double x2, double y2)
{
	return new Line (x1, y1, x2, y2);
}


void
polygon_set_fill_rule (Polygon *polygon, FillRule fill_rule)
{
	polygon->fill_rule = fill_rule;
}

void
polygon_set_points (Polygon *polygon, Point* points, int count)
{
	// FIXME - should we do a copy of them ?
	polygon->points = points;
	polygon->count = count;
}

void
polyline_set_fill_rule (Polyline *polyline, FillRule fill_rule)
{
	polyline->fill_rule = fill_rule;
}

void
polyline_set_points (Polyline *polyline, Point* points, int count)
{
	// FIXME - should we do a copy of them ?
	polyline->points = points;
	polyline->count = count;
}
