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
#include <cairo.h>

#include "shape.h"
#include "cutil.h"

#include <sys/time.h>
//
// SL-Cairo convertion and helper routines
//

static cairo_line_join_t
convert_line_join (PenLineJoin pen_line_join)
{
	switch (pen_line_join) {
	case PenLineJoinMiter:
		return CAIRO_LINE_JOIN_MITER;
	case PenLineJoinBevel:
		return CAIRO_LINE_JOIN_BEVEL;
	case PenLineJoinRound:
		return CAIRO_LINE_JOIN_ROUND;
	}
}

static cairo_line_cap_t
convert_line_cap (PenLineCap pen_line_cap)
{
	switch (pen_line_cap) {
	case PenLineCapFlat:
		return CAIRO_LINE_CAP_BUTT;
	case PenLineCapSquare:
		return CAIRO_LINE_CAP_SQUARE;
	case PenLineCapRound:
	case PenLineCapTriangle: 		/* FIXME: Triangle doesn't exist in Cairo */
		return CAIRO_LINE_CAP_ROUND;
	}
}

cairo_fill_rule_t
convert_fill_rule (FillRule fill_rule)
{
	switch (fill_rule) {
	case FillRuleEvenOdd:
		return CAIRO_FILL_RULE_EVEN_ODD;
	case FillRuleNonzero:
		return CAIRO_FILL_RULE_WINDING;
	}
}

void
moon_ellipse (cairo_t *cr, double x, double y, double w, double h)
{
	double rx = w / 2;
	double ry = h / 2;
	double cx = x + rx;
	double cy = y + ry;

	cairo_move_to (cr, cx + rx, cy);

	/* an approximate of the ellipse by drawing a curve in each
	 * quadrants */
	cairo_curve_to (cr,
			cx + rx, cy - ARC_TO_BEZIER * ry,
			cx + ARC_TO_BEZIER * rx, cy - ry,
			cx, cy - ry);
        
	cairo_curve_to (cr,
			cx - ARC_TO_BEZIER * rx, cy - ry,
			cx - rx, cy - ARC_TO_BEZIER * ry,
			cx - rx, cy);

	cairo_curve_to (cr,
			cx - rx, cy + ARC_TO_BEZIER * ry,
			cx - ARC_TO_BEZIER * rx, cy + ry,
			cx, cy + ry);
                
	cairo_curve_to (cr,
			cx + ARC_TO_BEZIER * rx, cy + ry,
			cx + rx, cy + ARC_TO_BEZIER * ry,
			cx + rx, cy);

	cairo_close_path (cr);
}

void
moon_rounded_rectangle (cairo_t *cr, double x, double y, double w, double h, double radius_x, double radius_y)
{
	// test limits (without using multiplications)
	if (radius_x > w - radius_x)
		radius_x = w / 2;
	if (radius_y > h - radius_y)
		radius_y = h / 2;

	// approximate (quite close) the arc using a bezier curve
	double c1 = ARC_TO_BEZIER * radius_x;
	double c2 = ARC_TO_BEZIER * radius_y;

	cairo_new_path (cr);
	cairo_move_to (cr, x + radius_x, y);
	cairo_rel_line_to (cr, w - 2 * radius_x, 0.0);
	cairo_rel_curve_to (cr, c1, 0.0, radius_x, c2, radius_x, radius_y);
	cairo_rel_line_to (cr, 0, h - 2 * radius_y);
	cairo_rel_curve_to (cr, 0.0, c2, c1 - radius_x, radius_y, -radius_x, radius_y);
	cairo_rel_line_to (cr, -w + 2 * radius_x, 0);
	cairo_rel_curve_to (cr, -c1, 0, -radius_x, -c2, -radius_x, -radius_y);
	cairo_rel_line_to (cr, 0, -h + 2 * radius_y);
	cairo_rel_curve_to (cr, 0.0, -c2, radius_x - c1, -radius_y, radius_x, -radius_y);
	cairo_close_path (cr);
}

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

Shape::Shape ()
{
	stroke = NULL;
	fill = NULL;
}

Shape::~Shape ()
{
	if (stroke != NULL) {
		stroke->Detach (NULL, this);
		stroke->unref ();
	}
	
	if (fill != NULL) {
		fill->Detach (NULL, this);
		fill->unref ();
	}
}

//
// This routine is useful for Shape derivatives: it can be used
// to either get the bounding box from cairo, or to paint it
//
void 
Shape::DoDraw (Surface *s, bool do_op, bool consider_fill)
{
	cairo_set_matrix (s->cairo, &absolute_xform);

	//printf ("Draw, xform: %g %g %g %g %g %g\n", 
	//	absolute_xform.xy,
	//	absolute_xform.xx,
	//	absolute_xform.yx,
	//	absolute_xform.yy,
	//	absolute_xform.x0,
	//	absolute_xform.y0);

	// getting bounds, using cairo_stroke_extents, doesn't requires us to fill (consider_fill)
	// unless there is no stroke brush assigned, which requires us to fill and use cairo_fill_extents
	// also not every shapes can be filled, e.g. polylines, CallFill
	if ((consider_fill || !stroke) && CanFill ()) {
		if (fill) {
			Draw (s);
			if (do_op) {
				fill->SetupBrush (s->cairo, this);
				cairo_fill (s->cairo);
			}
		}
	}
	
	if (stroke) {
		double thickness = shape_get_stroke_thickness (this);
		if (thickness == 0)
			return;

		cairo_set_line_width (s->cairo, thickness);

		int count = 0;
		double offset = 0.0;
		double *dashes = shape_get_stroke_dash_array (this, &count);
		if (dashes && (count > 0)) {
			/* FIXME: cairo doesn't support line cap for dashes */
			offset = shape_get_stroke_dash_offset (this);
			// special case or cairo stops drawing
			if ((count == 1) && (*dashes == 0.0))
				count = 0;
		}
		cairo_set_dash (s->cairo, dashes, count, offset);

		cairo_set_miter_limit (s->cairo, shape_get_stroke_miter_limit (this));
		cairo_set_line_join (s->cairo, convert_line_join (shape_get_stroke_line_join (this)));

		/* FIXME: cairo doesn't have separate line cap for the start and end */
		PenLineCap cap = shape_get_stroke_end_line_cap (this);
		if (cap == PenLineCapFlat) {
			cap = shape_get_stroke_start_line_cap (this);
		}
		cairo_set_line_cap (s->cairo, convert_line_cap (cap));

		Draw (s);
		if (do_op) {
			stroke->SetupBrush (s->cairo, this);
			cairo_stroke (s->cairo);
		}
	}
}

void
Shape::render (Surface *s, int x, int y, int width, int height)
{
	cairo_save (s->cairo);
	DoDraw (s, true, true);
	cairo_restore (s->cairo);
}

void 
Shape::getbounds ()
{
	Surface *s = item_get_surface (this);
	
	if (s == NULL)
		return;
	
	cairo_save (s->cairo);
	// dont do the operation and don't do the fill setup
	DoDraw (s, false, false);

	if (stroke)
		cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);
	else
		cairo_fill_extents (s->cairo, &x1, &y1, &x2, &y2);

	cairo_new_path (s->cairo);
	cairo_restore (s->cairo);

	// The extents are in the coordinates of the transform, translate to device coordinates
	x_cairo_matrix_transform_bounding_box (&absolute_xform, &x1, &y1, &x2, &y2);
}

bool
Shape::inside_object (Surface *s, double x, double y)
{
	bool ret = false;

	cairo_save (s->cairo);
	// don't do the operation but do consider filling
	DoDraw (s, false, true);
	double nx = x;
	double ny = y;

	cairo_matrix_t inverse = absolute_xform;
	cairo_matrix_invert (&inverse);

	cairo_matrix_transform_point (&inverse, &nx, &ny);

	if (cairo_in_stroke (s->cairo, nx, ny) || (CanFill () && cairo_in_fill (s->cairo, nx, ny)))
		ret = TRUE;
	
	cairo_new_path (s->cairo);

	cairo_restore (s->cairo);
	return ret;
}

void
Shape::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::SHAPE) {
		if (prop == Shape::StrokeProperty) {
			if (stroke != NULL) {
				stroke->Detach (NULL, this);
				stroke->unref ();
			}
			
			if ((stroke = shape_get_stroke (this)) != NULL) {
				stroke->Attach (NULL, this);
				stroke->ref ();
			}
		} else if (prop == Shape::FillProperty) {
			if (fill != NULL) {
				fill->Detach (NULL, this);
				fill->unref ();
			}
			
			if ((fill = shape_get_fill (this)) != NULL) {
				fill->Attach (NULL, this);
				fill->ref ();
			}
		}
		
		FullInvalidate (false);
	}
	
	FrameworkElement::OnPropertyChanged (prop);
	
	if ((prop == UIElement::RenderTransformOriginProperty) ||
	    (prop == UIElement::RenderTransformProperty)) {
		update_xform ();
	}
}

Brush *
shape_get_fill (Shape *shape)
{
	Value *value = shape->GetValue (Shape::FillProperty);
	return (value ? value->AsBrush() : NULL);
}

void 
shape_set_fill (Shape *shape, Brush *value)
{
	shape->SetValue (Shape::FillProperty, Value (value));
}

Brush *
shape_get_stroke (Shape *shape)
{
	Value *value = shape->GetValue (Shape::StrokeProperty);
	return (value ? value->AsBrush() : NULL);
}

void 
shape_set_stroke (Shape *shape, Brush *value)
{
	shape->SetValue (Shape::StrokeProperty, Value (value));
}

Stretch
shape_get_stretch (Shape *shape)
{
	return (Stretch) shape->GetValue (Shape::StretchProperty)->AsInt32();
}

void
shape_set_stretch (Shape *shape, Stretch value)
{
	shape->SetValue (Shape::StretchProperty, Value (value));
}

PenLineCap
shape_get_stroke_dash_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeDashCapProperty)->AsInt32();
}

void
shape_set_stroke_dash_cap (Shape *shape, PenLineCap value)
{
	shape->SetValue (Shape::StrokeDashCapProperty, Value (value));
}

PenLineCap
shape_get_stroke_start_line_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeStartLineCapProperty)->AsInt32();
}

void
shape_set_stroke_start_line_cap (Shape *shape, PenLineCap value)
{
	shape->SetValue (Shape::StrokeStartLineCapProperty, Value (value));
}

PenLineCap
shape_get_stroke_end_line_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeEndLineCapProperty)->AsInt32();
}

void
shape_set_stroke_end_line_cap (Shape *shape, PenLineCap value)
{
	shape->SetValue (Shape::StrokeEndLineCapProperty, Value (value));
}

double
shape_get_stroke_dash_offset (Shape *shape)
{
	return shape->GetValue (Shape::StrokeDashOffsetProperty)->AsDouble();
}

void
shape_set_stroke_dash_offset (Shape *shape, double value)
{
	shape->SetValue (Shape::StrokeDashOffsetProperty, Value (value));
}

double
shape_get_stroke_miter_limit (Shape *shape)
{
	return shape->GetValue (Shape::StrokeMiterLimitProperty)->AsDouble();
}

void
shape_set_stroke_miter_limit (Shape *shape, double value)
{
	shape->SetValue (Shape::StrokeMiterLimitProperty, Value (value));
}

double
shape_get_stroke_thickness (Shape *shape)
{
	return shape->GetValue (Shape::StrokeThicknessProperty)->AsDouble();
}

void
shape_set_stroke_thickness (Shape *shape, double value)
{
	shape->SetValue (Shape::StrokeThicknessProperty, Value (value));
}

PenLineJoin
shape_get_stroke_line_join (Shape *shape)
{
	return (PenLineJoin) shape->GetValue (Shape::StrokeLineJoinProperty)->AsInt32();
}

void
shape_set_stroke_line_join (Shape *shape, PenLineJoin value)
{
	shape->SetValue (Shape::StrokeLineJoinProperty, Value (value));
}

/*
 * note: We return a reference, not a copy, of the dashes. Not a big issue as
 * Silverlight Shape.StrokeDashArray only has a setter (no getter), so it's 
 * use is only internal.
 */
double*
shape_get_stroke_dash_array (Shape *shape, int *count)
{
	Value *value = shape->GetValue (Shape::StrokeDashArrayProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	DoubleArray *da = value->AsDoubleArray();
	*count = da->basic.count;
	return da->values;
}

void
shape_set_stroke_dash_array (Shape *shape, double* dashes, int count)
{
	shape->SetValue (Shape::StrokeDashArrayProperty, Value (dashes, count));
}

//
// Ellipse
//

Ellipse::Ellipse ()
{
	SetValue (Shape::StretchProperty, Value (StretchFill));
}

void
Ellipse::Draw (Surface *s)
{
	Stretch stretch = shape_get_stretch (this);
	if (stretch == StretchNone)
		return;

	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);

	switch (stretch) {
	case StretchUniform:
		w = h = (w < h) ? w : h;
		break;
	case StretchUniformToFill:
		// this gets an ellipse larger than it's dimension, relative
		// scaling is ok but we need to clip to it's original size
		cairo_rectangle (s->cairo, 0, 0, w, h);
		cairo_clip (s->cairo);
		w = h = (w > h) ? w : h;
		break;
	}

	moon_ellipse (s->cairo, 0, 0, w, h);
}

Point
Ellipse::getxformorigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		framework_element_get_height (this) * user_xform_origin.y);
}

Ellipse *
ellipse_new (void)
{
	return new Ellipse ();
}

//
// Rectangle
//

DependencyProperty* Rectangle::RadiusXProperty;
DependencyProperty* Rectangle::RadiusYProperty;

Rectangle::Rectangle ()
{
	SetValue (Shape::StretchProperty, Value (StretchFill));
}

void
Rectangle::Draw (Surface *s)
{
	double w, h;

	Stretch stretch = shape_get_stretch (this);
	if (stretch == StretchNone) {
		// this gets us a single colored point at X,Y
		w = 0.5;
		h = 0.5;
	} else {
		w = framework_element_get_width (this);
		h = framework_element_get_height (this);

		switch (stretch) {
		case StretchUniform:
			w = h = (w < h) ? w : h;
			break;
		case StretchUniformToFill:
			// this gets an ellipse larger than it's dimension, relative
			// scaling is ok but we need to clip to it's original size
			cairo_rectangle (s->cairo, 0, 0, w, h);
			cairo_clip (s->cairo);
			w = h = (w > h) ? w : h;
			break;
		}

		double radius_x = rectangle_get_radius_x (this);
		if (radius_x != 0) {
			double radius_y = rectangle_get_radius_y (this);
			if (radius_y != 0) {
				moon_rounded_rectangle (s->cairo, 0, 0, w, h, radius_x, radius_y);
				return;
			}
		}
	}
	// normal rectangle
	cairo_new_path (s->cairo);
	cairo_rectangle (s->cairo, 0, 0, w, h);
	cairo_close_path (s->cairo);
}

Point
Rectangle::getxformorigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		framework_element_get_height (this) * user_xform_origin.y);
}

void
Rectangle::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::RECTANGLE) {
		FullInvalidate (false);
		return;
	}
	
	Shape::OnPropertyChanged (prop);
}

double
rectangle_get_radius_x (Rectangle *rectangle)
{
	return rectangle->GetValue (Rectangle::RadiusXProperty)->AsDouble();
}

void
rectangle_set_radius_x (Rectangle *rectangle, double value)
{
	rectangle->SetValue (Rectangle::RadiusXProperty, Value (value));
}

double
rectangle_get_radius_y (Rectangle *rectangle)
{
	return rectangle->GetValue (Rectangle::RadiusYProperty)->AsDouble();
}

void
rectangle_set_radius_y (Rectangle *rectangle, double value)
{
	rectangle->SetValue (Rectangle::RadiusYProperty, Value (value));
}

Rectangle *
rectangle_new (void)
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
	// Note: Shape::StretchProperty has no effect on lines
	// Note: SL 1.1 alpha considers X2,Y2 as the start point (when drawing end line caps)
	//	This doesn't affect us because Cairo doesn't support separate start/end line caps
	cairo_move_to (s->cairo, line_get_x1 (this), line_get_y1 (this));
	cairo_line_to (s->cairo, line_get_x2 (this), line_get_y2 (this));
}

void
Line::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::LINE) {
		FullInvalidate (false);
		return;
	}
	
	Shape::OnPropertyChanged (prop);
}

double
line_get_x1 (Line *line)
{
	return line->GetValue (Line::X1Property)->AsDouble();
}

void
line_set_x1 (Line *line, double value)
{
	line->SetValue (Line::X1Property, Value (value));
}

double
line_get_y1 (Line *line)
{
	return line->GetValue (Line::Y1Property)->AsDouble();
}

void
line_set_y1 (Line *line, double value)
{
	line->SetValue (Line::Y1Property, Value (value));
}

double
line_get_x2 (Line *line)
{
	return line->GetValue (Line::X2Property)->AsDouble();
}

void
line_set_x2 (Line *line, double value)
{
	line->SetValue (Line::X2Property, Value (value));
}

double
line_get_y2 (Line *line)
{
	return line->GetValue (Line::Y2Property)->AsDouble();
}

void
line_set_y2 (Line *line, double value)
{
	line->SetValue (Line::Y2Property, Value (value));
}

Line *
line_new (void)
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
	int i, count = 0;
	Point *points = polygon_get_points (this, &count);

	if (!points || (count < 1))
		return;

	cairo_new_path (s->cairo);
	cairo_set_fill_rule (s->cairo, convert_fill_rule (polygon_get_fill_rule (this)));

	Stretch stretch = shape_get_stretch (this);
	switch (stretch) {
	case StretchNone:
		cairo_move_to (s->cairo, points [0].x, points [0].y);
		for (i = 1; i < count; i++)
			cairo_line_to (s->cairo, points [i].x, points [i].y);
		break;
		// Draw a line from the last point back to the first point if they're not the same
		if ((points [0].x != points [count-1].x) && (points [0].y != points [count-1].y)) {
			cairo_line_to (s->cairo, points [0].x, points [0].y);
		}
	default:
		double x = points [0].x;
		double y = points [0].y;
		cairo_move_to (s->cairo, 0, 0);
		for (i = 1; i < count; i++)
			cairo_line_to (s->cairo, points [i].x - x, points [i].y - y);
		break;
		// Draw a line from the last point back to the first point if they're not the same
		if ((points [count-1].x != 0) && (points [count-1].y != 0)) {
			cairo_line_to (s->cairo, 0, 0);
		}
	}

	cairo_close_path (s->cairo);
}

void
Polygon::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::POLYGON) {
		FullInvalidate (false);
		return;
	}
	
	Shape::OnPropertyChanged (prop);
}

FillRule
polygon_get_fill_rule (Polygon *polygon)
{
	return (FillRule) polygon->GetValue (Polygon::FillRuleProperty)->AsInt32();
}

void
polygon_set_fill_rule (Polygon *polygon, FillRule value)
{
	polygon->SetValue (Polygon::FillRuleProperty, Value (value));
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight Polygon.Points only has a setter (no getter), so it's use is 
 * only internal.
 */
Point *
polygon_get_points (Polygon *polygon, int *count)
{
	Value *value = polygon->GetValue (Polygon::PointsProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	PointArray *pa = value->AsPointArray();
	*count = pa->basic.count;
	return pa->points;
}

void
polygon_set_points (Polygon *polygon, Point* points, int count)
{
	polygon->SetValue (Polygon::PointsProperty, Value (points, count));
}

Polygon *
polygon_new (void)
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
	int i, count = 0;
	Point *points = polyline_get_points (this, &count);

	if (!points || (count < 1))
		return;

	cairo_set_fill_rule (s->cairo, convert_fill_rule (polyline_get_fill_rule (this)));
	cairo_new_path (s->cairo);

	Stretch stretch = shape_get_stretch (this);
	switch (stretch) {
	case StretchNone:
		cairo_move_to (s->cairo, points [0].x, points [0].y);
		for (i = 1; i < count; i++)
			cairo_line_to (s->cairo, points [i].x, points [i].y);
		break;
	default:
		double x = points [0].x;
		double y = points [0].y;
		cairo_move_to (s->cairo, 0, 0);
		for (i = 1; i < count; i++)
			cairo_line_to (s->cairo, points [i].x - x, points [i].y - y);
		break;
	}
}

void
Polyline::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::POLYLINE) {
		FullInvalidate (false);
		return;
	}
	
	Shape::OnPropertyChanged (prop);
}

FillRule
polyline_get_fill_rule (Polyline *polyline)
{
	return (FillRule) polyline->GetValue (Polyline::FillRuleProperty)->AsInt32();
}

void
polyline_set_fill_rule (Polyline *polyline, FillRule value)
{
	polyline->SetValue (Polyline::FillRuleProperty, Value (value));
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight Polyline.Points only has a setter (no getter), so it's use is 
 * only internal.
 */
Point *
polyline_get_points (Polyline *polyline, int *count)
{
	Value *value = polyline->GetValue (Polyline::PointsProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	PointArray *pa = value->AsPointArray();
	*count = pa->basic.count;
	return pa->points;
}

void
polyline_set_points (Polyline *polyline, Point* points, int count)
{
	polyline->SetValue (Polyline::PointsProperty, Value (points, count));
}

Polyline *
polyline_new (void)
{
	return new Polyline ();
}

//
// Path
//

DependencyProperty* Path::DataProperty;

Path::~Path ()
{
g_warning ("~Path %p", this);
	CleanupCache ();
}

void
Path::Draw (Surface *s)
{
	Geometry* data = path_get_data (this);
	if (!data)
		return;

	// compute and cache the path as some options, like stretch, can be very expansive
	// to compute multiple times (e.g. fill, stroke and getbounds)
	if (!path)
		BuildPath (s, data);

	cairo_new_path (s->cairo);
	cairo_append_path (s->cairo, path);
}

void
Path::BuildPath (Surface *s, Geometry* geometry)
{
	geometry->Draw (s);

	path = cairo_copy_path (s->cairo);

	Stretch stretch = shape_get_stretch (this);
	if (stretch == StretchNone)
		return;

	/* NOTE: this looks complex but avoid a *lot* of changes in geometry 
	 * (resulting in something even more complex).
	 */
	double minx = G_MAXDOUBLE;
	double miny = G_MAXDOUBLE;
	double maxx = G_MINDOUBLE;
	double maxy = G_MINDOUBLE;

	// find origin (minimums) and actual width/height (maximums - minimums)
	for (int i=0; i < path->num_data; i+= path->data[i].header.length) {
		cairo_path_data_t *data = &path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_CURVE_TO:
			// minimum
			if (minx > data[3].point.x)
				minx = data[3].point.x;
			if (miny > data[3].point.y)
				miny = data[3].point.y;
			if (minx > data[2].point.x)
				minx = data[2].point.x;
			if (miny > data[2].point.y)
				miny = data[2].point.y;
			// maximum
			if (maxx < data[3].point.x)
				maxx = data[3].point.x;
			if (maxy < data[3].point.y)
				maxy = data[3].point.y;
			if (maxx < data[2].point.x)
				maxx = data[2].point.x;
			if (maxy < data[2].point.y)
				maxy = data[2].point.y;
			/* fallthru */
		case CAIRO_PATH_LINE_TO:
		case CAIRO_PATH_MOVE_TO:
			// minimum
			if (minx > data[1].point.x)
				minx = data[1].point.x;
			if (miny > data[1].point.y)
				miny = data[1].point.y;
			// maximum
			if (maxx < data[1].point.x)
				maxx = data[1].point.x;
			if (maxy < data[1].point.y)
				maxy = data[1].point.y;
			break;
		case CAIRO_PATH_CLOSE_PATH:
			break;
		}
	}

	double actual_height = maxy - miny;
	double actual_width = maxx - minx;

	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
	double requested_height = (vh ? vh->AsDouble () : actual_height);
	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
	double requested_width = (vw ? vw->AsDouble () : actual_width);

	double sh = vh ? (requested_height / actual_height) : 1.0;
	double sw = vw ? (requested_width / actual_width) : 1.0;
	switch (stretch) {
	case StretchFill:
		break;
	case StretchUniform:
		sw = sh = (sw < sh) ? sw : sh;
		if (sw < sh) {
		} else {
		}
		break;
	case StretchUniformToFill:
		cairo_new_path (s->cairo);
		cairo_rectangle (s->cairo, 0, 0, requested_width, requested_height);
		cairo_clip (s->cairo);
		sw = sh = (sw > sh) ? sw : sh;
		break;
	}

	bool stretch_horz = (vw || (sw != 1.0));
	bool stretch_vert = (vh || (sh != 1.0));

	// substract origin (min[x|y]) and scale to requested dimensions (if specified)
	for (int i=0; i < path->num_data; i+= path->data[i].header.length) {
		cairo_path_data_t *data = &path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_CURVE_TO:
			data[3].point.x -= minx;
			data[3].point.y -= miny;
			data[2].point.x -= minx;
			data[2].point.y -= miny;
			if (stretch_horz) {
				data[3].point.x *= sw;
				data[2].point.x *= sw;
			}
			if (stretch_vert) {
				data[3].point.y *= sh;
				data[2].point.y *= sh;
			}
			/* fallthru */
		case CAIRO_PATH_LINE_TO:
		case CAIRO_PATH_MOVE_TO:
			data[1].point.x -= minx;
			data[1].point.y -= miny;
			if (stretch_horz)
				data[1].point.x *= sw;
			if (stretch_vert)
				data[1].point.y *= sh;
			break;
		case CAIRO_PATH_CLOSE_PATH:
			break;
			break;
		}
	}
}

void
Path::CleanupCache ()
{
	if (path) {
		cairo_path_destroy (path);
		path = NULL;
	}
}

/*
 * Paths are filled by default but PathFigure, inside a collection in a 
 * PathGeometry, can be unfilled (IsFilled property). In this case PathGeometry
 * will have deal with filling, or not, each of it's figure.
 */
bool
Path::CanFill ()
{
	Geometry* data = path_get_data (this);
	return (data ? data->CanFill () : false);
}

void
Path::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::PATH) {
		CleanupCache ();
		FullInvalidate (false);
		return;
	} else if (prop == Shape::StretchProperty) {
		CleanupCache ();
	}
	
	Shape::OnPropertyChanged (prop);
}

Geometry *
path_get_data (Path *path)
{
	Value *value = path->GetValue (Path::DataProperty);
	return (value ? value->AsGeometry() : NULL);
}

void
path_set_data (Path *path, Geometry *value)
{
	path->SetValue (Path::DataProperty, Value (value));
}

Path *
path_new (void)
{
	return new Path ();
}


//
// 
//

void
shape_init (void)
{
	/* Shape fields */
	Shape::FillProperty = DependencyObject::Register (Type::SHAPE, "Fill", Type::BRUSH);
	Shape::StretchProperty = DependencyObject::Register (Type::SHAPE, "Stretch", new Value (StretchNone));
	Shape::StrokeProperty = DependencyObject::Register (Type::SHAPE, "Stroke", Type::BRUSH);
	Shape::StrokeDashArrayProperty = DependencyObject::Register (Type::SHAPE, "StrokeDashArray", Type::DOUBLE_ARRAY);
	Shape::StrokeDashCapProperty = DependencyObject::Register (Type::SHAPE, "StrokeDashCap", new Value (PenLineCapFlat));
	Shape::StrokeDashOffsetProperty = DependencyObject::Register (Type::SHAPE, "StrokeDashOffset", new Value (0.0));
	Shape::StrokeEndLineCapProperty = DependencyObject::Register (Type::SHAPE, "StrokeEndLineCap", new Value (PenLineCapFlat));
	Shape::StrokeLineJoinProperty = DependencyObject::Register (Type::SHAPE, "StrokeLineJoin", new Value (PenLineJoinMiter));
	Shape::StrokeMiterLimitProperty = DependencyObject::Register (Type::SHAPE, "StrokeMiterLimit", new Value (10.0));
	Shape::StrokeStartLineCapProperty = DependencyObject::Register (Type::SHAPE, "StrokeStartLineCap", new Value (PenLineCapFlat));
	Shape::StrokeThicknessProperty = DependencyObject::Register (Type::SHAPE, "StrokeThickness", new Value (1.0));

	/* Rectangle fields */
	Rectangle::RadiusXProperty = DependencyObject::Register (Type::RECTANGLE, "RadiusX", new Value (0.0));
	Rectangle::RadiusYProperty = DependencyObject::Register (Type::RECTANGLE, "RadiusY", new Value (0.0));

	/* Line fields */
	Line::X1Property = DependencyObject::Register (Type::LINE, "X1", new Value (0.0));
	Line::Y1Property = DependencyObject::Register (Type::LINE, "Y1", new Value (0.0));
	Line::X2Property = DependencyObject::Register (Type::LINE, "X2", new Value (0.0));
	Line::Y2Property = DependencyObject::Register (Type::LINE, "Y2", new Value (0.0));

	/* Polygon fields */
	Polygon::FillRuleProperty = DependencyObject::Register (Type::POLYGON, "FillRule", new Value (FillRuleEvenOdd));
	Polygon::PointsProperty = DependencyObject::Register (Type::POLYGON, "Points", Type::POINT_ARRAY);

	/* Polyline fields */
	Polyline::FillRuleProperty = DependencyObject::Register (Type::POLYLINE, "FillRule", new Value (FillRuleEvenOdd));
	Polyline::PointsProperty = DependencyObject::Register (Type::POLYLINE, "Points", Type::POINT_ARRAY);

	/* Path fields */
	Path::DataProperty = DependencyObject::Register (Type::PATH, "Data", Type::GEOMETRY);
}
