/*
 * runtime.cpp: Core surface and canvas definitions.
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
#include "runtime.h"

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
Rectangle::Draw (Surface *s)
{
	static int n;
	
	cairo_rectangle (s->cairo, x, y, w, h);
}

Point
Rectangle::getxformorigin ()
{
	return Point (x + w * user_xform_origin.x, y + h * user_xform_origin.y);
}

Rectangle *
rectangle_new (double x, double y, double w, double h)
{
	Rectangle *rect = new Rectangle (x, y, w, h);

	return rect;
}

void
Line::Draw (Surface *s)
{
	cairo_move_to (s->cairo, line_x1, line_y1);
	cairo_line_to (s->cairo, line_x2, line_y2);
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

