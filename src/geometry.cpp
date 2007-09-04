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

#include "runtime.h"
#include "geometry.h"
#include "rect.h"
#include "shape.h"
#include "array.h"

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
Geometry::Draw (Path *path, cairo_t *cr)
{
	cairo_set_fill_rule (cr, convert_fill_rule (geometry_get_fill_rule (this)));
	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}
}

void
Geometry::OnPropertyChanged (DependencyProperty *prop)
{
	NotifyAttacheesOfPropertyChange (prop);
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
	this->SetValue (GeometryGroup::ChildrenProperty, Value (new GeometryCollection ()));
}

void
GeometryGroup::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::GEOMETRYGROUP) {
		Geometry::OnPropertyChanged (prop);
		return;
	}

	if (prop == ChildrenProperty) {
		GeometryCollection *newcol = GetValue (prop)->AsGeometryCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttacheesOfPropertyChange (prop);
}

void
GeometryGroup::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	NotifyAttacheesOfPropertyChange (subprop);
}

void
GeometryGroup::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	// GeometryGroup only has one collection, so let's save the hash lookup
	//if (col == GetValue (GeometryGroup::ChildrenProperty)->AsGeometryCollection())
		NotifyAttacheesOfPropertyChange (GeometryGroup::ChildrenProperty);
}

void
GeometryGroup::Draw (Path *path, cairo_t *cr)
{
	GeometryCollection *children = geometry_group_get_children (this);
	Collection::Node *node;
	
	Geometry::Draw (path, cr);
	
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Geometry *geometry = (Geometry *) node->obj;
		geometry->Draw (path, cr);
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
EllipseGeometry::Draw (Path *path, cairo_t *cr)
{
	Geometry::Draw (path, cr);

	Point *pt = ellipse_geometry_get_center (this);
	double rx = ellipse_geometry_get_radius_x (this);
	double ry = ellipse_geometry_get_radius_y (this);

	moon_ellipse (cr, pt->x - rx, pt->y - ry, rx * 2, ry * 2);
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
LineGeometry::Draw (Path *path, cairo_t *cr)
{
	Geometry::Draw (path, cr);

	Point *p1 = line_geometry_get_start_point (this);
	Point *p2 = line_geometry_get_end_point (this);

	cairo_move_to (cr, p1->x, p1->y);
	cairo_line_to (cr, p2->x, p2->y);
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
	this->SetValue (PathGeometry::FiguresProperty, Value::CreateUnrefPtr (new PathFigureCollection ()));
}

void
PathGeometry::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::PATHGEOMETRY) {
		Geometry::OnPropertyChanged (prop);
		return;
	}

	if (prop == FiguresProperty){
		PathFigureCollection *newcol = GetValue (prop)->AsPathFigureCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttacheesOfPropertyChange (prop);
}

void
PathGeometry::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	// PathGeometry only has one collection, so let's save the hash lookup
	//if (col == GetValue (PathGeometry::FiguresProperty)->AsPathFigureCollection ())
		NotifyAttacheesOfPropertyChange (PathGeometry::FiguresProperty);
}

void
PathGeometry::Draw (Path *path, cairo_t *cr)
{
	PathFigureCollection *children = GetValue (PathGeometry::FiguresProperty)->AsPathFigureCollection();
	Collection::Node *node;
	
	Geometry::Draw (path, cr);
	
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathFigure *pf = (PathFigure *) node->obj;
		pf->Draw (path, cr);
	}
}

PathFigureCollection *
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
RectangleGeometry::Draw (Path *path, cairo_t *cr)
{
	Geometry::Draw (path, cr);

	Rect *rect = rectangle_geometry_get_rect (this);
	if (!rect)
		return;

	double half_thick = 0.0;
	// path is optional (e.g. not available for clipping)
	if (path) {
		double thick = shape_get_stroke_thickness (path);
		if ((thick > rect->w) || (thick > rect->h)) {
			half_thick = thick / 2.0;
			rect->x -= half_thick;
			rect->y -= half_thick;
			rect->w += thick;
			rect->h += thick;
/* FIXME
 * - this doesn't match MS-SL if mixed with some "normal" (non-degenerated) geometry
 */
			path->SetShapeFlags (UIElement::SHAPE_DEGENERATE);
		}
	}

	double radius_x, radius_y;
	if (GetRadius (&radius_x, &radius_y)) {
		moon_rounded_rectangle (cr, rect->x, rect->y, rect->w, rect->h, radius_x + half_thick, radius_y + half_thick);
	} else {
		cairo_rectangle (cr, rect->x, rect->y, rect->w, rect->h);
	}
}

bool
RectangleGeometry::GetRadius (double *rx, double *ry)
{
	Value *value = GetValueNoDefault (RectangleGeometry::RadiusXProperty);
	if (!value)
		return false;
	*rx = value->AsDouble ();

	value = GetValueNoDefault (RectangleGeometry::RadiusYProperty);
	if (!value)
		return false;
	*ry = value->AsDouble ();

	return ((*rx != 0.0) && (*ry != 0.0));
}

//
// PathFigure
//

DependencyProperty* PathFigure::IsClosedProperty;
DependencyProperty* PathFigure::SegmentsProperty;
DependencyProperty* PathFigure::StartPointProperty;

PathFigure*
path_figure_new ()
{
	return new PathFigure ();
}

PathFigure::PathFigure ()
{
	this->SetValue (PathFigure::SegmentsProperty, Value::CreateUnrefPtr (new PathSegmentCollection ()));
}

PathFigure::~PathFigure ()
{
}

void
PathFigure::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::PATHFIGURE) {
		DependencyObject::OnPropertyChanged (prop);
		return;
	}

	if (prop == SegmentsProperty){
		PathSegmentCollection *newcol = GetValue (prop)->AsPathSegmentCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttacheesOfPropertyChange (prop);
}

void
PathFigure::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	// PathFigure only has one collection, so let's save the hash lookup
	//if (col == GetValue (PathFigure::SegmentsProperty)->AsPathSegmentCollection())
		NotifyAttacheesOfPropertyChange (PathFigure::SegmentsProperty);
}

void
PathFigure::Draw (Path *path, cairo_t *cr)
{
	PathSegmentCollection *children = GetValue (PathFigure::SegmentsProperty)->AsPathSegmentCollection ();
	Point *start = path_figure_get_start_point (this);
	Collection::Node *node;
	
	// should not be required because of the cairo_move_to
	//cairo_new_sub_path (cr);
	cairo_move_to (cr, start->x, start->y);
	
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathSegment *ps = (PathSegment *) node->obj;
		ps->Draw (cr);
	}
	
	if (path_figure_get_is_closed (this))
		cairo_close_path (cr);
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
// PathSegment
//

void PathSegment::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::DEPENDENCY_OBJECT) {
		DependencyObject::OnPropertyChanged (prop);
		return;
	}

	NotifyAttacheesOfPropertyChange (prop);
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
ArcSegment::Draw (cairo_t *cr)
{
	Point *size = arc_segment_get_size (this);
	double angle = arc_segment_get_rotation_angle (this);
	int large = arc_segment_get_is_large_arc (this) ? 1 : 0;
	int direction = arc_segment_get_sweep_direction (this) == SweepDirectionCounterclockwise ? 0 : 1;
	Point* p = arc_segment_get_point (this);

	// FIXME: there's no cairo_arc_to so we reuse librsvg code (see rsvg.cpp)
	rsvg_arc_to (cr, size->x, size->y, angle, large, direction, p->x, p->y); 
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
BezierSegment::Draw (cairo_t *cr)
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

	cairo_curve_to (cr, x1, y1, x2, y2, x3, y3);
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
LineSegment::Draw (cairo_t *cr)
{
	Point *p = line_segment_get_point (this);

	double x = p ? p->x : 0.0;
	double y = p ? p->y : 0.0;

	cairo_line_to (cr, x, y);
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
PolyBezierSegment::Draw (cairo_t *cr)
{
	int count = 0;
	Point* points = poly_bezier_segment_get_points (this, &count);

	// we need at least 3 points
	if (!points || (count % 3) != 0)
		return;

	for (int i=0; i < count - 2; i+=3) {
		cairo_curve_to (cr, points[i].x, points[i].y, points[i+1].x, points[i+1].y,
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
PolyLineSegment::Draw (cairo_t *cr)
{
	int count = 0;
	Point* points = poly_line_segment_get_points (this, &count);

	for (int i=0; i < count; i++) {
		cairo_line_to (cr, points[i].x, points[i].y);
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
PolyQuadraticBezierSegment::Draw (cairo_t *cr)
{
	int count = 0;
	Point* points = poly_quadratic_bezier_segment_get_points (this, &count);
	if (!points || ((count % 2) != 0))
		return;

	// origin
	double x0 = 0.0;
	double y0 = 0.0;
	cairo_get_current_point (cr, &x0, &y0);

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

		cairo_curve_to (cr, x1, y1, x2, y2, x3, y3);

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
QuadraticBezierSegment::Draw (cairo_t *cr)
{
	Point *p1 = quadratic_bezier_segment_get_point1 (this);
	Point *p2 = quadratic_bezier_segment_get_point2 (this);

	// quadratic to cubic bezier, the original control point and the end control point are the same
	// http://web.archive.org/web/20020209100930/http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/node2.html
	double x0 = 0.0;
	double y0 = 0.0;
	cairo_get_current_point (cr, &x0, &y0);

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

	cairo_curve_to (cr, x1, y1, x2, y2, x3, y3);
}

//
// 
//

void
geometry_init ()
{
	/* Geometry fields */
	Geometry::FillRuleProperty = DependencyObject::Register (Type::GEOMETRY, "FillRule", new Value (FillRuleEvenOdd));
	Geometry::TransformProperty = DependencyObject::Register (Type::GEOMETRY, "Transform", Type::TRANSFORM);

	/* GeometryGroup fields */
	GeometryGroup::ChildrenProperty = DependencyObject::Register (Type::GEOMETRYGROUP, "Children", Type::GEOMETRY_COLLECTION);

	/* EllipseGeometry fields */
	EllipseGeometry::CenterProperty = DependencyObject::Register (Type::ELLIPSEGEOMETRY, "Center", Type::POINT);
	EllipseGeometry::RadiusXProperty = DependencyObject::Register (Type::ELLIPSEGEOMETRY, "RadiusX", new Value (0.0));
	EllipseGeometry::RadiusYProperty = DependencyObject::Register (Type::ELLIPSEGEOMETRY, "RadiusY", new Value (0.0));

	/* LineGeometry fields */
	LineGeometry::EndPointProperty = DependencyObject::Register (Type::LINEGEOMETRY, "EndPoint", Type::POINT);
	LineGeometry::StartPointProperty = DependencyObject::Register (Type::LINEGEOMETRY, "StartPoint", Type::POINT);

	/* PathGeometry */
	PathGeometry::FiguresProperty = DependencyObject::Register (Type::PATHGEOMETRY, "Figures", Type::PATHFIGURE_COLLECTION);

	/* RectangleGeometry fields */
	RectangleGeometry::RadiusXProperty = DependencyObject::Register (Type::RECTANGLEGEOMETRY, "RadiusX", new Value (0.0));
	RectangleGeometry::RadiusYProperty = DependencyObject::Register (Type::RECTANGLEGEOMETRY, "RadiusY", new Value (0.0));
	RectangleGeometry::RectProperty = DependencyObject::Register (Type::RECTANGLEGEOMETRY, "Rect", Type::RECT);

	/* PathFigure fields */
	PathFigure::IsClosedProperty = DependencyObject::Register (Type::PATHFIGURE, "IsClosed", new Value (false));
	PathFigure::SegmentsProperty = DependencyObject::Register (Type::PATHFIGURE, "Segments", Type::PATHSEGMENT_COLLECTION);
	PathFigure::StartPointProperty = DependencyObject::Register (Type::PATHFIGURE, "StartPoint", Type::POINT);

	/* ArcSegment fields */
	ArcSegment::IsLargeArcProperty = DependencyObject::Register (Type::ARCSEGMENT, "IsLargeArc", new Value (false));
	ArcSegment::PointProperty = DependencyObject::Register (Type::ARCSEGMENT, "Point", Type::POINT);
	ArcSegment::RotationAngleProperty = DependencyObject::Register (Type::ARCSEGMENT, "RotationAngle", new Value (0.0));
	ArcSegment::SizeProperty = DependencyObject::Register (Type::ARCSEGMENT, "Size", Type::POINT);
	ArcSegment::SweepDirectionProperty = DependencyObject::Register (Type::ARCSEGMENT, "SweepDirection", new Value (SweepDirectionCounterclockwise));

	/* BezierSegment fields */
	BezierSegment::Point1Property = DependencyObject::Register (Type::BEZIERSEGMENT, "Point1", Type::POINT);
	BezierSegment::Point2Property = DependencyObject::Register (Type::BEZIERSEGMENT, "Point2", Type::POINT);
	BezierSegment::Point3Property = DependencyObject::Register (Type::BEZIERSEGMENT, "Point3", Type::POINT);

	/* LineSegment fields */
	LineSegment::PointProperty = DependencyObject::Register (Type::LINESEGMENT, "Point", Type::POINT);

	/* PolyBezierSegment fields */
	PolyBezierSegment::PointsProperty = DependencyObject::Register (Type::POLYBEZIERSEGMENT, "Points", Type::POINT_ARRAY);

	/* PolyLineSegment fields */
	PolyLineSegment::PointsProperty = DependencyObject::Register (Type::POLYLINESEGMENT, "Points", Type::POINT_ARRAY);

	/* PolyQuadraticBezierSegment field */
	PolyQuadraticBezierSegment::PointsProperty = DependencyObject::Register (Type::POLYQUADRATICBEZIERSEGMENT, "Points", Type::POINT_ARRAY);

	/* QuadraticBezierSegment field */
	QuadraticBezierSegment::Point1Property = DependencyObject::Register (Type::QUADRATICBEZIERSEGMENT, "Point1", Type::POINT);
	QuadraticBezierSegment::Point2Property = DependencyObject::Register (Type::QUADRATICBEZIERSEGMENT, "Point2", Type::POINT);
}
