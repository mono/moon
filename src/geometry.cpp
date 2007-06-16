/*
 * geometry.cpp: Geometry classes
 *
 * Author:
 *	Sebastien Pouliot  <sebastien@ximian.com>
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

#include "geometry.h"
#include "shape.h"

#include "rsvg.h"

//
// Geometry
//

DependencyProperty* Geometry::FillRuleProperty;
DependencyProperty* Geometry::TransformProperty;

FillRule
geometry_get_fill_rule (Geometry *geometry)
{
	return (FillRule) geometry->GetValue (Geometry::FillRuleProperty)->AsInt32();
}

void
geometry_set_fill_rule (Geometry *geometry, FillRule fill_rule)
{
	geometry->SetValue (Geometry::FillRuleProperty, Value (fill_rule));
}

Transform*
geometry_get_transform (Geometry *geometry)
{
	Value *value = geometry->GetValue (Geometry::TransformProperty);
	return value ? value->AsTransform() : NULL;
}

void
geometry_set_transform (Geometry *geometry, Transform *transform)
{
	geometry->SetValue (Geometry::TransformProperty, Value (transform));
}

void
Geometry::Draw (Surface *s)
{
	cairo_set_fill_rule (s->cairo, convert_fill_rule (geometry_get_fill_rule (this)));
	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (s->cairo, &matrix);
	}
}

//
// GeometryGroup
//

DependencyProperty* GeometryGroup::ChildrenProperty;

GeometryGroup*
geometry_group_new ()
{
	return new GeometryGroup ();
}

GeometryGroup::GeometryGroup ()
{
	children = NULL;
	GeometryCollection *c = new GeometryCollection ();

	this->SetValue (GeometryGroup::ChildrenProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == children);
}

GeometryGroup::~GeometryGroup ()
{
	if (children)
		base_unref (children);
}

void
GeometryGroup::OnPropertyChanged (DependencyProperty *prop)
{
	Geometry::OnPropertyChanged (prop);

	if (prop == ChildrenProperty){
		// The new value has already been set, so unref the old collection
		GeometryCollection *newcol = GetValue (prop)->AsGeometryCollection();

		if (newcol != children) {
			if (children) 
				base_unref (children);

			children = newcol;
			if (children) {
				if (children->closure)
					printf ("Warning we attached a property that was already attached\n");
				children->closure = this;
			
				base_ref (children);
			}
		}
	}
}

void
GeometryGroup::Draw (Surface *s)
{
	Geometry::Draw (s);

	for (GList *g = children->list; g != NULL; g = g->next) {
		Geometry *geometry = (Geometry*) g->data;
		geometry->Draw (s);
	}
}

GeometryCollection*
geometry_group_get_children (GeometryGroup *geometry_group)
{
	Value *value = geometry_group->GetValue (GeometryGroup::ChildrenProperty);
	return (GeometryCollection*) (value ? value->AsGeometryCollection() : NULL);
}

void
geometry_group_set_children (GeometryGroup *geometry_group, GeometryCollection* geometry_collection)
{
	geometry_group->SetValue (GeometryGroup::ChildrenProperty, Value (geometry_collection));
}

//
// GeometryCollection
//

GeometryCollection*
geometry_collection_new ()
{
	return new GeometryCollection ();
}

//
// PathFigureCollection
//

PathFigureCollection*
path_figure_collection_new ()
{
	return new PathFigureCollection ();
}

//
// PathSegmentCollection
//

PathSegmentCollection*
path_segment_collection_new ()
{
	return new PathSegmentCollection ();
}

//
// EllipseGeometry
//

DependencyProperty* EllipseGeometry::CenterProperty;
DependencyProperty* EllipseGeometry::RadiusXProperty;
DependencyProperty* EllipseGeometry::RadiusYProperty;

Point*
ellipse_geometry_get_center (EllipseGeometry *ellipse_geometry)
{
	Value *value = ellipse_geometry->GetValue (EllipseGeometry::CenterProperty);
	return (value ? value->AsPoint() : NULL);
}

void
ellipse_geometry_set_center (EllipseGeometry *ellipse_geometry, Point *point)
{
	ellipse_geometry->SetValue (EllipseGeometry::CenterProperty, Value (*point));
}

double
ellipse_geometry_get_radius_x (EllipseGeometry *ellipse_geometry)
{
	return ellipse_geometry->GetValue (EllipseGeometry::RadiusXProperty)->AsDouble();
}

void
ellipse_geometry_set_radius_x (EllipseGeometry *ellipse_geometry, double radius_x)
{
	ellipse_geometry->SetValue (EllipseGeometry::RadiusXProperty, Value (radius_x));
}

double
ellipse_geometry_get_radius_y (EllipseGeometry *ellipse_geometry)
{
	return ellipse_geometry->GetValue (EllipseGeometry::RadiusYProperty)->AsDouble();
}

void
ellipse_geometry_set_radius_y (EllipseGeometry *ellipse_geometry, double radius_y)
{
	ellipse_geometry->SetValue (EllipseGeometry::RadiusYProperty, Value (radius_y));
}

EllipseGeometry*
ellipse_geometry_new ()
{
	return new EllipseGeometry ();
}

void
EllipseGeometry::Draw (Surface *s)
{
	Geometry::Draw (s);

	Point *pt = ellipse_geometry_get_center (this);
	double rx = ellipse_geometry_get_radius_x (this);
	double ry = ellipse_geometry_get_radius_y (this);

	moon_ellipse (s->cairo, pt->x - rx, pt->y - ry, rx * 2, ry * 2);
}

//
// LineGeometry
//

DependencyProperty* LineGeometry::EndPointProperty;
DependencyProperty* LineGeometry::StartPointProperty;

Point*
line_geometry_get_end_point (LineGeometry* line_geometry)
{
	Value *value = line_geometry->GetValue (LineGeometry::EndPointProperty);
	return (value ? value->AsPoint() : new Point ());
}

void
line_geometry_set_end_point (LineGeometry* line_geometry, Point *end_point)
{
	line_geometry->SetValue (LineGeometry::EndPointProperty, Value (*end_point));
}

Point*
line_geometry_get_start_point (LineGeometry* line_geometry)
{
	Value *value = line_geometry->GetValue (LineGeometry::StartPointProperty);
	return (value ? value->AsPoint() : new Point ());
}

void
line_geometry_set_start_point (LineGeometry* line_geometry, Point *start_point)
{
	line_geometry->SetValue (LineGeometry::StartPointProperty, Value (*start_point));
}

LineGeometry*
line_geometry_new ()
{
	return new LineGeometry ();
}

void
LineGeometry::Draw (Surface *s)
{
	Geometry::Draw (s);

	Point *p1 = line_geometry_get_start_point (this);
	Point *p2 = line_geometry_get_end_point (this);

	cairo_move_to (s->cairo, p1->x, p1->y);
	cairo_line_to (s->cairo, p2->x, p2->y);
}

//
// PathGeometry
//

DependencyProperty* PathGeometry::FiguresProperty;

PathGeometry*
path_geometry_new ()
{
	return new PathGeometry ();
}

PathGeometry::PathGeometry ()
{
	children = NULL;
	PathFigureCollection *c = new PathFigureCollection ();

	this->SetValue (PathGeometry::FiguresProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == children);
}

PathGeometry::~PathGeometry ()
{
	if (children)
		base_unref (children);
}

void
PathGeometry::OnPropertyChanged (DependencyProperty *prop)
{
	Geometry::OnPropertyChanged (prop);

	if (prop == FiguresProperty){
		// The new value has already been set, so unref the old collection
		PathFigureCollection *newcol = GetValue (prop)->AsPathFigureCollection();

		if (newcol != children) {
			if (children) 
				base_unref (children);

			children = newcol;
			if (children) {
				if (children->closure)
					printf ("Warning we attached a property that was already attached\n");
				children->closure = this;
			
				base_ref (children);
			}
		}
	}
}

void
PathGeometry::Draw (Surface *s)
{
	Geometry::Draw (s);

	for (GList *coll = children->list; coll != NULL; coll = coll->next) {
		PathFigure *pf = (PathFigure*) coll->data;
		pf->Draw (s);
	}
}

PathFigureCollection*
path_geometry_get_figures (PathGeometry *path_geometry)
{
	Value *value = path_geometry->GetValue (PathGeometry::FiguresProperty);
	return (PathFigureCollection*) (value ? value->AsPathFigureCollection() : NULL);
}

void
path_geometry_set_figures (PathGeometry *path_geometry, PathFigureCollection* collection)
{
	path_geometry->SetValue (PathGeometry::FiguresProperty, Value (collection));
}

//
// RectangleGeometry
//

DependencyProperty* RectangleGeometry::RadiusXProperty;
DependencyProperty* RectangleGeometry::RadiusYProperty;
DependencyProperty* RectangleGeometry::RectProperty;

double
rectangle_geometry_get_radius_x (RectangleGeometry *rectangle_geometry)
{
	return rectangle_geometry->GetValue (RectangleGeometry::RadiusXProperty)->AsDouble();
}

void
rectangle_geometry_set_radius_x (RectangleGeometry *rectangle_geometry, double radius_x)
{
	rectangle_geometry->SetValue (RectangleGeometry::RadiusXProperty, Value (radius_x));
}

double
rectangle_geometry_get_radius_y (RectangleGeometry *rectangle_geometry)
{
	return rectangle_geometry->GetValue (RectangleGeometry::RadiusYProperty)->AsDouble();
}

void
geometry_set_radius_y (RectangleGeometry *rectangle_geometry, double radius_y)
{
	rectangle_geometry->SetValue (RectangleGeometry::RadiusYProperty, Value (radius_y));
}

Rect*
rectangle_geometry_get_rect (RectangleGeometry *rectangle_geometry)
{
	Value *value = rectangle_geometry->GetValue (RectangleGeometry::RectProperty);
	return (value ? value->AsRect() : NULL);
}

void
rectangle_geometry_set_rect (RectangleGeometry *rectangle_geometry, Rect *rect)
{
	rectangle_geometry->SetValue (RectangleGeometry::RectProperty, Value (*rect));
}

RectangleGeometry*
rectangle_geometry_new ()
{
	return new RectangleGeometry ();
}

void
RectangleGeometry::Draw (Surface *s)
{
	Geometry::Draw (s);

	Rect *rect = rectangle_geometry_get_rect (this);
	double radius_x = rectangle_geometry_get_radius_x  (this);
	if (radius_x != 0) {
		double radius_y = rectangle_geometry_get_radius_y (this);
		if (radius_y != 0) {
			moon_rounded_rectangle (s->cairo, rect->x, rect->y, rect->w, rect->h, radius_x, radius_y);
			return;
		}
	}
	// normal rectangle
	cairo_rectangle (s->cairo, rect->x, rect->y, rect->w, rect->h);
}

//
// PathFigure
//

DependencyProperty* PathFigure::IsClosedProperty;
DependencyProperty* PathFigure::IsFilledProperty;
DependencyProperty* PathFigure::SegmentsProperty;
DependencyProperty* PathFigure::StartPointProperty;

PathFigure*
path_figure_new ()
{
	return new PathFigure ();
}

PathFigure::PathFigure ()
{
	children = NULL;
	PathSegmentCollection *c = new PathSegmentCollection ();

	this->SetValue (PathFigure::SegmentsProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == children);
}

PathFigure::~PathFigure ()
{
	if (children)
		base_unref (children);
}

void
PathFigure::OnPropertyChanged (DependencyProperty *prop)
{
	DependencyObject::OnPropertyChanged (prop);

	if (prop == SegmentsProperty){
		// The new value has already been set, so unref the old collection
		PathSegmentCollection *newcol = GetValue (prop)->AsPathSegmentCollection();

		if (newcol != children) {
			if (children) 
				base_unref (children);

			children = newcol;
			if (children) {
				if (children->closure)
					printf ("Warning we attached a property that was already attached\n");
				children->closure = this;
			
				base_ref (children);
			}
		}
	}
}

void
PathFigure::Draw (Surface *s)
{
	Point *start = path_figure_get_start_point (this);

	// should not be required because of the cairo_move_to
	//cairo_new_sub_path (s->cairo);
	cairo_move_to (s->cairo, start->x, start->y);

	for (GList *coll = children->list; coll != NULL; coll = coll->next) {
		PathSegment *ps = (PathSegment*) coll->data;
		ps->Draw (s);
	}

	if (path_figure_get_is_closed (this)) {
		cairo_close_path (s->cairo);
	}
}

bool
path_figure_get_is_closed (PathFigure *path_figure)
{
	return path_figure->GetValue (PathFigure::IsClosedProperty)->AsBool();
}

void
path_figure_set_is_closed (PathFigure *path_figure, bool closed)
{
	path_figure->SetValue (PathFigure::IsClosedProperty, Value (closed));
}

bool
path_figure_get_is_filled (PathFigure *path_figure)
{
	// FIXME
	g_warning ("Ignored in Mix, will be removed in 1.0. See http://blogs.msdn.com/jstegman/archive/2007/06/06/more-v-1-0-changes.aspx");
	return path_figure->GetValue (PathFigure::IsFilledProperty)->AsBool();
}

void
path_figure_set_is_filled (PathFigure *path_figure, bool filled)
{
	// FIXME
	g_warning ("Ignored in Mix, will be removed in 1.0. See http://blogs.msdn.com/jstegman/archive/2007/06/06/more-v-1-0-changes.aspx");
	path_figure->SetValue (PathFigure::IsFilledProperty, Value (filled));
}

PathSegmentCollection*
path_figure_get_segments (PathGeometry *path_geometry)
{
	Value *value = path_geometry->GetValue (PathFigure::SegmentsProperty);
	return (PathSegmentCollection*) (value ? value->AsPathSegmentCollection() : NULL);
}

void
path_figure_set_segments (PathGeometry *path_geometry, PathSegmentCollection* collection)
{
	path_geometry->SetValue (PathFigure::SegmentsProperty, Value (collection));
}

Point*
path_figure_get_start_point (PathFigure *path_figure)
{
	Value *value = path_figure->GetValue (PathFigure::StartPointProperty);
	return (value ? value->AsPoint() : new Point (0, 0));
}

void
path_figure_set_start_point (PathFigure *path_figure, Point *point)
{
	path_figure->SetValue (PathFigure::StartPointProperty, Value (*point));
}

//
// ArcSegment
//

DependencyProperty* ArcSegment::IsLargeArcProperty;
DependencyProperty* ArcSegment::PointProperty;
DependencyProperty* ArcSegment::RotationAngleProperty;
DependencyProperty* ArcSegment::SizeProperty;
DependencyProperty* ArcSegment::SweepDirectionProperty;

ArcSegment*
arc_segment_new ()
{
	return new ArcSegment ();
}

bool
arc_segment_get_is_large_arc (ArcSegment *segment)
{
	return segment->GetValue (ArcSegment::IsLargeArcProperty)->AsBool();
}

void
arc_segment_set_is_large_arc (ArcSegment *segment, bool large)
{
	segment->SetValue (ArcSegment::IsLargeArcProperty, Value (large));
}

Point*
arc_segment_get_point (ArcSegment *segment)
{
	Value *value = segment->GetValue (ArcSegment::PointProperty);
	return (value ? value->AsPoint() : NULL);
}

void
arc_segment_set_point (ArcSegment *segment, Point *point)
{
	segment->SetValue (ArcSegment::PointProperty, Value (*point));
}

double
arc_segment_get_rotation_angle (ArcSegment *segment)
{
	return segment->GetValue (ArcSegment::RotationAngleProperty)->AsDouble();
}

void
arc_segment_set_rotation_angle (ArcSegment *segment, double angle)
{
	segment->SetValue (ArcSegment::RotationAngleProperty, Value (angle));
}

Point*
arc_segment_get_size (ArcSegment *segment)
{
	Value *value = segment->GetValue (ArcSegment::SizeProperty);
	return (value ? value->AsPoint() : NULL);
}

void
arc_segment_set_size (ArcSegment *segment, Point *size)
{
	segment->SetValue (ArcSegment::SizeProperty, Value (*size));
}

SweepDirection
arc_segment_get_sweep_direction (ArcSegment *segment)
{
	return (SweepDirection) segment->GetValue (ArcSegment::SweepDirectionProperty)->AsInt32();
}

void
arc_segment_set_sweep_direction (ArcSegment *segment, SweepDirection direction)
{
	segment->SetValue (ArcSegment::SweepDirectionProperty, Value (direction));
}

void
ArcSegment::Draw (Surface *s)
{
	Point *size = arc_segment_get_size (this);
	double angle = arc_segment_get_rotation_angle (this);
	int large = arc_segment_get_is_large_arc (this) ? 1 : 0;
	int direction = arc_segment_get_sweep_direction (this) == SweepDirectionCounterclockwise ? 0 : 1;
	Point* p = arc_segment_get_point (this);

	// FIXME: there's no cairo_arc_to so we reuse librsvg code (see rsvg.cpp)
	rsvg_arc_to (s->cairo, size->x, size->y, angle, large, direction, p->x, p->y); 
}

//
// BezierSegment
//

DependencyProperty* BezierSegment::Point1Property;
DependencyProperty* BezierSegment::Point2Property;
DependencyProperty* BezierSegment::Point3Property;

BezierSegment*
bezier_segment_new ()
{
	return new BezierSegment ();
}

Point*
bezier_segment_get_point1 (BezierSegment *segment)
{
	Value *value = segment->GetValue (BezierSegment::Point1Property);
	return (value ? value->AsPoint() : NULL);
}

void
bezier_segment_set_point1 (BezierSegment *segment, Point *point)
{
	segment->SetValue (BezierSegment::Point1Property, Value (*point));
}

Point*
bezier_segment_get_point2 (BezierSegment *segment)
{
	Value *value = segment->GetValue (BezierSegment::Point2Property);
	return (value ? value->AsPoint() : NULL);
}

void
bezier_segment_set_point2 (BezierSegment *segment, Point *point)
{
	segment->SetValue (BezierSegment::Point2Property, Value (*point));
}

Point*
bezier_segment_get_point3 (BezierSegment *segment)
{
	Value *value = segment->GetValue (BezierSegment::Point3Property);
	return (value ? value->AsPoint() : NULL);
}

void
bezier_segment_set_point3 (BezierSegment *segment, Point *point)
{
	segment->SetValue (BezierSegment::Point3Property, Value (*point));
}

void
BezierSegment::Draw (Surface *s)
{
	Point *p1 = bezier_segment_get_point1 (this);
	Point *p2 = bezier_segment_get_point2 (this);
	Point *p3 = bezier_segment_get_point3 (this);

	double x1 = p1 ? p1->x : 0.0;
	double y1 = p1 ? p1->y : 0.0;
	double x2 = p2 ? p2->x : 0.0;
	double y2 = p2 ? p2->y : 0.0;
	double x3 = p3 ? p3->x : 0.0;
	double y3 = p3 ? p3->y : 0.0;

	cairo_curve_to (s->cairo, x1, y1, x2, y2, x3, y3);
}

//
// LineSegment
//

DependencyProperty* LineSegment::PointProperty;

LineSegment*
line_segment_new ()
{
	return new LineSegment ();
}

Point*
line_segment_get_point (LineSegment *segment)
{
	Value *value = segment->GetValue (LineSegment::PointProperty);
	return (value ? value->AsPoint() : NULL);
}

void
line_segment_set_point (LineSegment *segment, Point *point)
{
	segment->SetValue (LineSegment::PointProperty, Value (*point));
}

void
LineSegment::Draw (Surface *s)
{
	Point *p = line_segment_get_point (this);

	double x = p ? p->x : 0.0;
	double y = p ? p->y : 0.0;

	cairo_line_to (s->cairo, x, y);
}

//
// PolyBezierSegment
//

DependencyProperty* PolyBezierSegment::PointsProperty;

PolyBezierSegment*
poly_bezier_segment_new ()
{
	return new PolyBezierSegment ();
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight PolyBezierSegment.Points only has a setter (no getter), so it's
 * use is only internal.
 */
Point*
poly_bezier_segment_get_points (PolyBezierSegment *segment, int *count)
{
	Value *value = segment->GetValue (PolyBezierSegment::PointsProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	PointArray *pa = value->AsPointArray();
	*count = pa->basic.count;
	return pa->points;
}

void
poly_bezier_segment_set_points (PolyBezierSegment *segment, Point *points, int count)
{
	segment->SetValue (PolyBezierSegment::PointsProperty, Value (points, count));
}

void
PolyBezierSegment::Draw (Surface *s)
{
	int count = 0;
	Point* points = poly_bezier_segment_get_points (this, &count);

	// we need at least 3 points
	for (int i=0; i < count - 2; i+=3) {
		cairo_curve_to (s->cairo, points[i].x, points[i].y, points[i+1].x, points[i+1].y,
			points[i+2].x, points[i+2].y);
	}
}

//
// PolyLineSegment
//

DependencyProperty* PolyLineSegment::PointsProperty;

PolyLineSegment*
poly_line_segment_new ()
{
	return new PolyLineSegment ();
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight PolyLineSegment.Points only has a setter (no getter), so it's
 * use is only internal.
 */
Point*
poly_line_segment_get_points (PolyLineSegment *segment, int *count)
{
	Value *value = segment->GetValue (PolyLineSegment::PointsProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	PointArray *pa = value->AsPointArray();
	*count = pa->basic.count;
	return pa->points;
}

void
poly_line_segment_set_points (PolyLineSegment *segment, Point *points, int count)
{
	segment->SetValue (PolyLineSegment::PointsProperty, Value (points, count));
}

void
PolyLineSegment::Draw (Surface *s)
{
	int count = 0;
	Point* points = poly_line_segment_get_points (this, &count);

	for (int i=0; i < count; i++) {
		cairo_line_to (s->cairo, points[i].x, points[i].y);
	}
}

//
// PolyQuadraticBezierSegment
//

DependencyProperty* PolyQuadraticBezierSegment::PointsProperty;

PolyQuadraticBezierSegment*
poly_quadratic_bezier_segment_new ()
{
	return new PolyQuadraticBezierSegment ();
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight PolyQuadraticBezierSegment.Points only has a setter (no getter),
 * so it's use is only internal.
 */
Point*
poly_quadratic_bezier_segment_get_points (PolyQuadraticBezierSegment *segment, int *count)
{
	Value *value = segment->GetValue (PolyQuadraticBezierSegment::PointsProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	PointArray *pa = value->AsPointArray();
	*count = pa->basic.count;
	return pa->points;
}

void
poly_quadratic_bezier_segment_set_points (PolyQuadraticBezierSegment *segment, Point *points, int count)
{
	segment->SetValue (PolyQuadraticBezierSegment::PointsProperty, Value (points, count));
}

// quadratic to cubic bezier, the original control point and the end control point are the same
// http://web.archive.org/web/20020209100930/http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/node2.html
void
PolyQuadraticBezierSegment::Draw (Surface *s)
{
	int count = 0;
	Point* points = poly_quadratic_bezier_segment_get_points (this, &count);
	if (!points)
		return;

	// origin
	double x0 = 0.0;
	double y0 = 0.0;
	cairo_get_current_point (s->cairo, &x0, &y0);

	// we need at least 2 points
	for (int i=0; i < count - 1; i+=2) {
		double x1 = points[i].x;
		double y1 = points[i].y;
		double x2 = points[i+1].x;
		double y2 = points[i+1].y;
		double x3 = x2;
		double y3 = y2;

		x2 = x1 + (x2 - x1) / 3;
		y2 = y1 + (y2 - y1) / 3;
		x1 = x0 + 2 * (x1 - x0) / 3;
		y1 = y0 + 2 * (y1 - y0) / 3;

		cairo_curve_to (s->cairo, x1, y1, x2, y2, x3, y3);

		// set new origin
		x0 = x3;
		y0 = y3;
	}
}

//
// QuadraticBezierSegment
//

DependencyProperty* QuadraticBezierSegment::Point1Property;
DependencyProperty* QuadraticBezierSegment::Point2Property;

QuadraticBezierSegment*
quadratic_bezier_segment_new ()
{
	return new QuadraticBezierSegment ();
}

Point*
quadratic_bezier_segment_get_point1 (QuadraticBezierSegment *segment)
{
	Value *value = segment->GetValue (QuadraticBezierSegment::Point1Property);
	return (value ? value->AsPoint() : NULL);
}

void
quadratic_bezier_segment_set_point1 (QuadraticBezierSegment *segment, Point *point)
{
	segment->SetValue (QuadraticBezierSegment::Point1Property, Value (*point));
}

Point*
quadratic_bezier_segment_get_point2 (QuadraticBezierSegment *segment)
{
	Value *value = segment->GetValue (QuadraticBezierSegment::Point2Property);
	return (value ? value->AsPoint() : NULL);
}

void
quadratic_bezier_segment_set_point2 (QuadraticBezierSegment *segment, Point *point)
{
	segment->SetValue (QuadraticBezierSegment::Point2Property, Value (*point));
}

void
QuadraticBezierSegment::Draw (Surface *s)
{
	Point *p1 = quadratic_bezier_segment_get_point1 (this);
	Point *p2 = quadratic_bezier_segment_get_point2 (this);

	// quadratic to cubic bezier, the original control point and the end control point are the same
	// http://web.archive.org/web/20020209100930/http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/node2.html
	double x0 = 0.0;
	double y0 = 0.0;
	cairo_get_current_point (s->cairo, &x0, &y0);

	double x1 = p1 ? p1->x : 0.0;
	double y1 = p1 ? p1->y : 0.0;
	double x2 = p2 ? p2->x : 0.0;
	double y2 = p2 ? p2->y : 0.0;
	double x3 = x2;
	double y3 = y2;

	x2 = x1 + (x2 - x1) / 3;
	y2 = y1 + (y2 - y1) / 3;
	x1 = x0 + 2 * (x1 - x0) / 3;
	y1 = y0 + 2 * (y1 - y0) / 3;

	cairo_curve_to (s->cairo, x1, y1, x2, y2, x3, y3);
}

//
// 
//

void
geometry_init ()
{
	/* Geometry fields */
	Geometry::FillRuleProperty = DependencyObject::Register (Value::GEOMETRY, "FillRule", new Value (FillRuleEvenOdd));
	Geometry::TransformProperty = DependencyObject::Register (Value::GEOMETRY, "Transform", Value::TRANSFORM);

	/* GeometryGroup fields */
	GeometryGroup::ChildrenProperty = DependencyObject::Register (Value::GEOMETRYGROUP, "Children", Value::GEOMETRY_COLLECTION);

	/* EllipseGeometry fields */
	EllipseGeometry::CenterProperty = DependencyObject::Register (Value::ELLIPSEGEOMETRY, "Center", Value::POINT);
	EllipseGeometry::RadiusXProperty = DependencyObject::Register (Value::ELLIPSEGEOMETRY, "RadiusX", new Value (0.0));
	EllipseGeometry::RadiusYProperty = DependencyObject::Register (Value::ELLIPSEGEOMETRY, "RadiusY", new Value (0.0));

	/* LineGeometry fields */
	LineGeometry::EndPointProperty = DependencyObject::Register (Value::LINEGEOMETRY, "EndPoint", Value::POINT);
	LineGeometry::StartPointProperty = DependencyObject::Register (Value::LINEGEOMETRY, "StartPoint", Value::POINT);

	/* PathGeometry */
	PathGeometry::FiguresProperty = DependencyObject::Register (Value::PATHGEOMETRY, "Figures", Value::PATHFIGURE_COLLECTION);

	/* RectangleGeometry fields */
	RectangleGeometry::RadiusXProperty = DependencyObject::Register (Value::RECTANGLEGEOMETRY, "RadiusX", new Value (0.0));
	RectangleGeometry::RadiusYProperty = DependencyObject::Register (Value::RECTANGLEGEOMETRY, "RadiusY", new Value (0.0));
	RectangleGeometry::RectProperty = DependencyObject::Register (Value::RECTANGLEGEOMETRY, "Rect", Value::RECT);

	/* PathFigure fields */
	PathFigure::IsClosedProperty = DependencyObject::Register (Value::PATHFIGURE, "IsClosed", new Value (false));
	PathFigure::IsFilledProperty = DependencyObject::Register (Value::PATHFIGURE, "IsFilled", new Value (true));
	PathFigure::SegmentsProperty = DependencyObject::Register (Value::PATHFIGURE, "Segments", Value::PATHSEGMENT_COLLECTION);
	PathFigure::StartPointProperty = DependencyObject::Register (Value::PATHFIGURE, "StartPoint", Value::POINT);

	/* ArcSegment fields */
	ArcSegment::IsLargeArcProperty = DependencyObject::Register (Value::ARCSEGMENT, "IsLargeArc", new Value (false));
	ArcSegment::PointProperty = DependencyObject::Register (Value::ARCSEGMENT, "Point", Value::POINT);
	ArcSegment::RotationAngleProperty = DependencyObject::Register (Value::ARCSEGMENT, "RotationAngle", new Value (0.0));
	ArcSegment::SizeProperty = DependencyObject::Register (Value::ARCSEGMENT, "Size", Value::POINT);
	ArcSegment::SweepDirectionProperty = DependencyObject::Register (Value::ARCSEGMENT, "SweepDirection", new Value (SweepDirectionCounterclockwise));

	/* BezierSegment fields */
	BezierSegment::Point1Property = DependencyObject::Register (Value::BEZIERSEGMENT, "Point1", Value::POINT);
	BezierSegment::Point2Property = DependencyObject::Register (Value::BEZIERSEGMENT, "Point2", Value::POINT);
	BezierSegment::Point3Property = DependencyObject::Register (Value::BEZIERSEGMENT, "Point3", Value::POINT);

	/* LineSegment fields */
	LineSegment::PointProperty = DependencyObject::Register (Value::LINESEGMENT, "Point", Value::POINT);

	/* PolyBezierSegment fields */
	PolyBezierSegment::PointsProperty = DependencyObject::Register (Value::POLYBEZIERSEGMENT, "Points", Value::POINT_ARRAY);

	/* PolyLineSegment fields */
	PolyLineSegment::PointsProperty = DependencyObject::Register (Value::POLYLINESEGMENT, "Points", Value::POINT_ARRAY);

	/* PolyQuadraticBezierSegment field */
	PolyQuadraticBezierSegment::PointsProperty = DependencyObject::Register (Value::POLYQUADRATICBEZIERSEGMENT, "Points", Value::POINT_ARRAY);

	/* QuadraticBezierSegment field */
	QuadraticBezierSegment::Point1Property = DependencyObject::Register (Value::QUADRATICBEZIERSEGMENT, "Point1", Value::POINT);
	QuadraticBezierSegment::Point2Property = DependencyObject::Register (Value::QUADRATICBEZIERSEGMENT, "Point2", Value::POINT);
}
