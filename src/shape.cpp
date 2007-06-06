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

//
// Shape
//

DependencyProperty* Shape::FillProperty;
DependencyProperty* Shape::StretchProperty;
DependencyProperty* Shape::StrokeProperty;
DependencyProperty* Shape::StrokeDashArrayProperty;
DependencyProperty* Shape::StrokeDashCapProperty;
DependencyProperty* Shape::StrokeDashOffsetProperty;
DependencyProperty* Shape::StrokeEndLineCapProperty;
DependencyProperty* Shape::StrokeLineJoinProperty;
DependencyProperty* Shape::StrokeMiterLimitProperty;
DependencyProperty* Shape::StrokeStartLineCapProperty;
DependencyProperty* Shape::StrokeThicknessProperty;

//
// This routine is useful for Shape derivatives: it can be used
// to either get the bounding box from cairo, or to paint it
//
void 
Shape::DoDraw (Surface *s, bool do_op)
{
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);

	if (fill){
		fill->SetupBrush (s->cairo);
		Draw (s);
		if (do_op)
			cairo_fill (s->cairo);
	}

	if (stroke){
		cairo_set_line_width (s->cairo, shape_get_stroke_thickness (this));
		if (stroke_dash_array) {
			double offset = shape_get_stroke_dash_offset (this);
	                cairo_set_dash (s->cairo, stroke_dash_array, stroke_dash_array_count, offset);
		}
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
	/*
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
	*/
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

Stretch
shape_get_stretch (Shape *shape)
{
	return (Stretch) shape->GetValue (Shape::StretchProperty)->u.i32;
}

void
shape_set_stretch (Shape *shape, Stretch stretch)
{
	shape->SetValue (Shape::StretchProperty, Value (stretch));
}

PenLineCap
shape_get_stroke_dash_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeDashCapProperty)->u.i32;
}

void
shape_set_stroke_dash_cap (Shape *shape, PenLineCap cap)
{
	shape->SetValue (Shape::StrokeDashCapProperty, Value (cap));
}

PenLineCap
shape_get_stroke_start_line_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeStartLineCapProperty)->u.i32;
}

void
shape_set_stroke_start_line_cap (Shape *shape, PenLineCap cap)
{
	shape->SetValue (Shape::StrokeStartLineCapProperty, Value (cap));
}

PenLineCap
shape_get_stroke_end_line_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeEndLineCapProperty)->u.i32;
}

void
shape_set_stroke_end_line_cap (Shape *shape, PenLineCap cap)
{
	shape->SetValue (Shape::StrokeEndLineCapProperty, Value (cap));
}

double
shape_get_stroke_dash_offset (Shape *shape)
{
	return shape->GetValue (Shape::StrokeDashOffsetProperty)->u.d;
}

void
shape_set_stroke_dash_offset (Shape *shape, double offset)
{
	shape->SetValue (Shape::StrokeDashOffsetProperty, Value (offset));
}

double
shape_get_stroke_miter_limit (Shape *shape)
{
	return shape->GetValue (Shape::StrokeMiterLimitProperty)->u.d;
}

void
shape_set_stroke_miter_limit (Shape *shape, double limit)
{
	shape->SetValue (Shape::StrokeMiterLimitProperty, Value (limit));
}

double
shape_get_stroke_thickness (Shape *shape)
{
	return shape->GetValue (Shape::StrokeThicknessProperty)->u.d;
}

void
shape_set_stroke_thickness (Shape *shape, double thickness)
{
	shape->SetValue (Shape::StrokeThicknessProperty, Value (thickness));
}

PenLineJoin
shape_get_stroke_line_join (Shape *shape)
{
	return (PenLineJoin) shape->GetValue (Shape::StrokeLineJoinProperty)->u.i32;
}

void
shape_set_stroke_line_join (Shape *shape, PenLineJoin join)
{
	shape->SetValue (Shape::StrokeLineJoinProperty, Value (join));
}

void
shape_set_stroke_dash_array (Shape *shape, double* dashes, int count)
{
	// FIXME - move to DependencyObject
	shape->stroke_dash_array = dashes;
	shape->stroke_dash_array_count = count;
}

//
// Ellipse
//

void
Ellipse::Draw (Surface *s)
{
	double rx, ry, cx, cy;

	rx = framework_element_get_width (this) / 2;
	ry = framework_element_get_height (this) / 2;
	cx = rx;
	cy = ry;

	cairo_move_to (s->cairo, cx + rx, cy);

	/* an approximate of the ellipse by drawing a curve in each
	 * quadrants */
	cairo_curve_to (s->cairo,
			cx + rx, cy - ARC_TO_BEZIER * ry,
			cx + ARC_TO_BEZIER * rx, cy - ry,
			cx, cy - ry);
        
	cairo_curve_to (s->cairo,
			cx - ARC_TO_BEZIER * rx, cy - ry,
			cx - rx, cy - ARC_TO_BEZIER * ry,
			cx - rx, cy);

	cairo_curve_to (s->cairo,
			cx - rx, cy + ARC_TO_BEZIER * ry,
			cx - ARC_TO_BEZIER * rx, cy + ry,
			cx, cy + ry);
                
	cairo_curve_to (s->cairo,
			cx + ARC_TO_BEZIER * rx, cy + ry,
			cx + rx, cy + ARC_TO_BEZIER * ry,
			cx + rx, cy);

	cairo_close_path (s->cairo);
}

Ellipse *
ellipse_new ()
{
	return new Ellipse ();
}

//
// Rectangle
//

DependencyProperty* Rectangle::RadiusXProperty;
DependencyProperty* Rectangle::RadiusYProperty;

void
Rectangle::Draw (Surface *s)
{
	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);
	double radius_x = rectangle_get_radius_x (this);
	if (radius_x != 0) {
		double radius_y = rectangle_get_radius_y (this);
		if (radius_y != 0) {
			// approximate (quite close) the arc using a bezier curve
			double c1 = ARC_TO_BEZIER * radius_x;
			double c2 = ARC_TO_BEZIER * radius_y;
			cairo_move_to (s->cairo, radius_x, 0);
			cairo_rel_line_to (s->cairo, w - 2 * radius_x, 0.0);
			cairo_rel_curve_to (s->cairo, c1, 0.0, radius_x, c2, radius_x, radius_y);
			cairo_rel_line_to (s->cairo, 0, h - 2 * radius_y);
			cairo_rel_curve_to (s->cairo, 0.0, c2, c1 - radius_x, radius_y, -radius_x, radius_y);
			cairo_rel_line_to (s->cairo, -w + 2 * radius_x, 0);
			cairo_rel_curve_to (s->cairo, -c1, 0, -radius_x, -c2, -radius_x, -radius_y);
			cairo_rel_line_to (s->cairo, 0, -h + 2 * radius_y);
			cairo_rel_curve_to (s->cairo, 0.0, -c2, radius_x - c1, -radius_y, radius_x, -radius_y);
			cairo_close_path (s->cairo);
			return;
		}
	}

	// normal rectangle
	cairo_rectangle (s->cairo, 0, 0, w, h);
}

void
Rectangle::set_prop_from_str (const char *prop, const char *value)
{
	if (!g_strcasecmp (prop, "radiusx"))
		rectangle_set_radius_x (this, strtod (value, NULL));
	else if (!g_strcasecmp (prop, "radiusy"))
		rectangle_set_radius_y (this, strtod (value, NULL));
	else
		Shape::set_prop_from_str (prop, value);
}

Point
Rectangle::getxformorigin ()
{
	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		framework_element_get_height (this) * user_xform_origin.y);
}

double
rectangle_get_radius_x (Rectangle *rectangle)
{
	return rectangle->GetValue (Rectangle::RadiusXProperty)->u.d;
}

void
rectangle_set_radius_x (Rectangle *rectangle, double value)
{
	rectangle->SetValue (Rectangle::RadiusXProperty, Value (value));
}

double
rectangle_get_radius_y (Rectangle *rectangle)
{
	return rectangle->GetValue (Rectangle::RadiusYProperty)->u.d;
}

void
rectangle_set_radius_y (Rectangle *rectangle, double value)
{
	rectangle->SetValue (Rectangle::RadiusYProperty, Value (value));
}

Rectangle *
rectangle_new ()
{
	return new Rectangle ();
}

//
// Line
//

DependencyProperty* Line::X1Property;
DependencyProperty* Line::Y1Property;
DependencyProperty* Line::X2Property;
DependencyProperty* Line::Y2Property;

void
Line::Draw (Surface *s)
{
	cairo_move_to (s->cairo, line_get_x1 (this), line_get_y1 (this));
	cairo_line_to (s->cairo, line_get_x2 (this), line_get_y2 (this));
}

void
Line::set_prop_from_str (const char *prop, const char *value)
{
	if (!g_strcasecmp (prop, "x1"))
		line_set_x1 (this, strtod (value, NULL));
	else if (!g_strcasecmp (prop, "y1"))
		line_set_y1 (this, strtod (value, NULL));
	else if (!g_strcasecmp (prop, "x2"))
		line_set_x2 (this, strtod (value, NULL));
	else if (!g_strcasecmp (prop, "y2"))
		line_set_y2 (this, strtod (value, NULL));
	else
		Shape::set_prop_from_str (prop, value);
}

Point
Line::getxformorigin ()
{
	double x1 = line_get_x1 (this);
	double y1 = line_get_y1 (this);
	return Point (x1 + (line_get_x2 (this)- x1) * user_xform_origin.x, 
		      y1 + (line_get_y2 (this) - y1) * user_xform_origin.y);
}

double
line_get_x1 (Line *line)
{
	return line->GetValue (Line::X1Property)->u.d;
}

void
line_set_x1 (Line *line, double value)
{
	line->SetValue (Line::X1Property, Value (value));
}

double
line_get_y1 (Line *line)
{
	return line->GetValue (Line::Y1Property)->u.d;
}

void
line_set_y1 (Line *line, double value)
{
	line->SetValue (Line::Y1Property, Value (value));
}

double
line_get_x2 (Line *line)
{
	return line->GetValue (Line::X2Property)->u.d;
}

void
line_set_x2 (Line *line, double value)
{
	line->SetValue (Line::X2Property, Value (value));
}

double
line_get_y2 (Line *line)
{
	return line->GetValue (Line::Y2Property)->u.d;
}

void
line_set_y2 (Line *line, double value)
{
	line->SetValue (Line::Y2Property, Value (value));
}

Line *
line_new ()
{
	return new Line ();
}

//
// Polygon
//

DependencyProperty* Polygon::FillRuleProperty;
DependencyProperty* Polygon::PointsProperty;

void
Polygon::Draw (Surface *s)
{
	int i;

	if (!points || (count < 1))
		return;

	cairo_move_to (s->cairo, points [0].x, points [0].y);

	for (i = 1; i < count; i++) {
		cairo_line_to (s->cairo, points [i].x, points [i].y);
	}

	// Draw a line from the last point back to the first point if they're not the same
	if ((points [0].x != points [count-1].x) && (points [0].y != points [count-1].y)) {
		cairo_line_to (s->cairo, points [0].x, points [0].y);
	}

	cairo_close_path (s->cairo);
}

FillRule
polygon_get_fill_rule (Polygon *polygon)
{
	return (FillRule) polygon->GetValue (Polygon::FillRuleProperty)->u.i32;
}

void
polygon_set_fill_rule (Polygon *polygon, FillRule fill_rule)
{
	polygon->SetValue (Polygon::FillRuleProperty, Value (fill_rule));
}

void
polygon_set_points (Polygon *polygon, Point* points, int count)
{
	// FIXME - should we do a copy of them ?
	polygon->points = points;
	polygon->count = count;
}

Polygon *
polygon_new ()
{
	return new Polygon ();
}

//
// Polyline
//

DependencyProperty* Polyline::FillRuleProperty;
DependencyProperty* Polyline::PointsProperty;

void
Polyline::Draw (Surface *s)
{
	int i;

	if (!points || (count < 1))
		return;

	cairo_move_to (s->cairo, points [0].x, points [0].y);

	for (i = 1; i < count; i++) {
		cairo_line_to (s->cairo, points [i].x, points [i].y);
	}
}

FillRule
polyline_get_fill_rule (Polyline *polyline)
{
	return (FillRule) polyline->GetValue (Polyline::FillRuleProperty)->u.i32;
}

void
polyline_set_fill_rule (Polyline *polyline, FillRule fill_rule)
{
	polyline->SetValue (Polyline::FillRuleProperty, Value (fill_rule));
}

void
polyline_set_points (Polyline *polyline, Point* points, int count)
{
	// FIXME - should we do a copy of them ?
	polyline->points = points;
	polyline->count = count;
}

Polyline *
polyline_new ()
{
	return new Polyline ();
}

//
// Path
//

DependencyProperty* Path::DataProperty;

void
Path::Draw (Surface *s)
{
}

Geometry*
path_get_data (Path *path)
{
	return (Geometry*) path->GetValue (Path::DataProperty)->u.dependency_object;
}

void
path_set_data (Path *path, Geometry* data)
{
	path->SetValue (Path::DataProperty, Value (data));
}

Path*
path_new ()
{
	return new Path ();
}


//
// 
//

void
shape_init ()
{
	/* Shape fields */
	Shape::FillProperty = DependencyObject::Register (DependencyObject::SHAPE, "Fill", new Value ());
	Shape::StretchProperty = DependencyObject::Register (DependencyObject::SHAPE, "Stretch", new Value (StretchFill));
	Shape::StrokeProperty = DependencyObject::Register (DependencyObject::SHAPE, "Stroke", new Value ());
	Shape::StrokeDashArrayProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeDashArray", new Value ());
	Shape::StrokeDashCapProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeDashCap", new Value (PenLineCapFlat));
	Shape::StrokeDashOffsetProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeDashOffset", new Value (0.0));
	Shape::StrokeEndLineCapProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeEndLineCap", new Value (PenLineCapFlat));
	Shape::StrokeLineJoinProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeLineJoin", new Value (PenLineJoinMiter));
	Shape::StrokeMiterLimitProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeMiterLimit", new Value (10.0));
	Shape::StrokeStartLineCapProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeStartLineCap", new Value (PenLineCapFlat));
	Shape::StrokeThicknessProperty = DependencyObject::Register (DependencyObject::SHAPE, "StrokeThickness", new Value (1.0));

	/* Rectangle fields */
	Rectangle::RadiusXProperty = DependencyObject::Register (DependencyObject::RECTANGLE, "RadiusX", new Value (0.0));
	Rectangle::RadiusYProperty = DependencyObject::Register (DependencyObject::RECTANGLE, "RadiusY", new Value (0.0));

	/* Line fields */
	Line::X1Property = DependencyObject::Register (DependencyObject::LINE, "X1", new Value (0.0));
	Line::Y1Property = DependencyObject::Register (DependencyObject::LINE, "Y1", new Value (0.0));
	Line::X2Property = DependencyObject::Register (DependencyObject::LINE, "X2", new Value (0.0));
	Line::Y2Property = DependencyObject::Register (DependencyObject::LINE, "Y2", new Value (0.0));

	/* Polygon fields */
	Polygon::FillRuleProperty = DependencyObject::Register (DependencyObject::POLYGON, "Fill", new Value (FillRuleEvenOdd));
	Polygon::PointsProperty = DependencyObject::Register (DependencyObject::POLYGON, "Points", new Value ());

	/* Polyline fields */
	Polyline::FillRuleProperty = DependencyObject::Register (DependencyObject::POLYLINE, "Fill", new Value (FillRuleEvenOdd));
	Polyline::PointsProperty = DependencyObject::Register (DependencyObject::POLYLINE, "Points", new Value ());

	/* Path fields */
	Path::DataProperty = DependencyObject::Register (DependencyObject::PATH, "Data", new Value ());
}
