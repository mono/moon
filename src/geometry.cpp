/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * geometry.cpp: Geometry classes
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <math.h>

#include "utils.h"
#include "geometry.h"
#include "shape.h"

//
// Geometry
//

Geometry::Geometry ()
{
	SetObjectType (Type::GEOMETRY);

	path = NULL;
	local_bounds = Rect (0,0, -INFINITY, -INFINITY);
}

Geometry::~Geometry ()
{
	if (path)
		moon_path_destroy (path);
}

void
Geometry::Draw (cairo_t *cr)
{
	Transform *transform = GetTransform ();
	cairo_matrix_t saved;
	cairo_get_matrix (cr, &saved);

	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}

	if (!IsBuilt ())
		Build ();

	// Geometry is used for Clip so Fill (normally setting the fill rule) is never called
	cairo_set_fill_rule (cr, convert_fill_rule (GetFillRule ()));

	if (path)
		cairo_append_path (cr, &path->cairo);

	cairo_set_matrix (cr, &saved);
}

void
Geometry::InvalidateCache ()
{
	if (path)
		moon_path_clear (path);

	local_bounds = Rect (0, 0, -INFINITY, -INFINITY);
}

Rect
Geometry::GetBounds ()
{
	bool compute = local_bounds.IsEmpty (true);

	if (!IsBuilt ()) {
		Build ();
		compute = true;
	}

	if (compute)
		local_bounds = ComputePathBounds ();

	Rect bounds = local_bounds;

	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}

	return bounds;
}

Rect
Geometry::ComputePathBounds()
{
	if (!IsBuilt ())
		Build ();

	if (!path || (path->cairo.num_data == 0))
		return Rect ();

	cairo_t *cr = measuring_context_create ();

	cairo_append_path (cr, &path->cairo);
	
	double x1, y1, x2, y2;

	cairo_path_extents (cr, &x1, &y1, &x2, &y2);

	Rect bounds = Rect (MIN (x1, x2), MIN (y1, y2), fabs (x2 - x1), fabs (y2 - y1));

	measuring_context_destroy (cr);

	return bounds;
}

void
Geometry::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	// no need to clear the path for Geometry itself as FillRule and Transform properties are 
	// only used when drawing, i.e. they do not affect the path itself
	if (args->GetProperty ()->GetOwnerType() != Type::GEOMETRY && 
		args->GetId () != PathGeometry::FillRuleProperty && 
		args->GetId () != GeometryGroup::FillRuleProperty) {
		DependencyObject::OnPropertyChanged (args, error);

		// not sure why we're doing this inside this block.. seems like it should happen outside it?
		InvalidateCache ();

		return;
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
Geometry::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	NotifyListenersOfPropertyChange (prop, NULL);
	
	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
}

//
// GeometryCollection
//

GeometryCollection::GeometryCollection ()
{
	SetObjectType (Type::GEOMETRY_COLLECTION);
}

GeometryCollection::~GeometryCollection ()
{
}

//
// GeometryGroup
//

GeometryGroup::GeometryGroup ()
{
	SetObjectType (Type::GEOMETRYGROUP);
}

GeometryGroup::~GeometryGroup ()
{
}

void
GeometryGroup::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	InvalidateCache ();

	if (col != GetChildren ()) {
		Geometry::OnCollectionChanged (col, args);
		return;
	}
	
	NotifyListenersOfPropertyChange (GeometryGroup::ChildrenProperty, NULL);
}

void
GeometryGroup::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	InvalidateCache ();

	if (col != GetChildren ()) {
		Geometry::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	NotifyListenersOfPropertyChange (GeometryGroup::ChildrenProperty, NULL);
}

void
GeometryGroup::Draw (cairo_t *cr)
{
	Transform *transform = GetTransform ();
	cairo_matrix_t saved;
	cairo_get_matrix (cr, &saved);

	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}
	
	GeometryCollection *children = GetChildren ();
	Geometry *geometry;

	// GeometryGroup is used for Clip (as a Geometry) so Fill (normally setting the fill rule) is never called
	cairo_set_fill_rule (cr, convert_fill_rule (GetFillRule ()));
	
	for (int i = 0; i < children->GetCount (); i++) {
		geometry = children->GetValueAt (i)->AsGeometry ();
		
		geometry->Draw (cr);
	}
	
	cairo_set_matrix (cr, &saved);
}

Rect
GeometryGroup::ComputePathBounds ()
{
	GeometryCollection *children = GetChildren ();
	Rect bounds = Rect (0.0, 0.0, 0.0, 0.0);
	Geometry *geometry;
	
	for (int i = 0; i < children->GetCount (); i++) {
		geometry = children->GetValueAt (i)->AsGeometry ();
		
		bounds = bounds.Union (geometry->GetBounds (), true);
	}
	
	//g_warning ("GeometryGroup::ComputeBounds - x %g y %g w %g h %g", bounds.x, bounds.y, bounds.w, bounds.h);
	return bounds;
}

#if 0
Point
Geometry::GetOriginPoint ()
{
	double x = 0.0;
	double y = 0.0;

	if (!IsBuilt ())
		Build ();

	moon_get_origin (path, &x, &y);
	return Point (x, y);
}
#endif

//
// EllipseGeometry
//

EllipseGeometry::EllipseGeometry ()
{
	SetObjectType (Type::ELLIPSEGEOMETRY);
}

EllipseGeometry::~EllipseGeometry ()
{
}

void
EllipseGeometry::Build ()
{
	double rx = GetRadiusX ();
	double ry = GetRadiusY ();
	Point *pt = GetCenter ();
	double x = pt ? pt->x : 0.0;
	double y = pt ? pt->y : 0.0;
	
	path = moon_path_renew (path, MOON_PATH_ELLIPSE_LENGTH);
	moon_ellipse (path, x - rx, y - ry, rx * 2.0, ry * 2.0);
}

Rect
EllipseGeometry::ComputePathBounds ()
{
	// code written to minimize divisions

	double hw = GetRadiusX ();
	double hh = GetRadiusY ();
	// point is at center, so left-top corner is minus half width / half height
	Point *pt = GetCenter ();
	double x = pt ? pt->x : 0.0;
	double y = pt ? pt->y : 0.0;
	Rect bounds;
	
	bounds = Rect (x - hw, y - hh, hw * 2.0, hh * 2.0);
	
	return bounds;
}

//
// LineGeometry
//

LineGeometry::LineGeometry ()
{
	SetObjectType (Type::LINEGEOMETRY);
}

LineGeometry::~LineGeometry ()
{
}

void
LineGeometry::Build ()
{
	Point *p1 = GetStartPoint ();
	Point *p2 = GetEndPoint ();
	
	path = moon_path_renew (path, MOON_PATH_MOVE_TO_LENGTH + MOON_PATH_LINE_TO_LENGTH);
	moon_move_to (path, p1 ? p1->x : 0.0, p1 ? p1->y : 0.0);
	moon_line_to (path, p2 ? p2->x : 0.0, p2 ? p2->y : 0.0);
}

Rect
LineGeometry::ComputePathBounds ()
{
	Point *p1 = GetStartPoint ();
	Point *p2 = GetEndPoint ();
	PenLineCap start_cap;
	PenLineCap end_cap;
	Rect bounds;

	start_cap = PenLineCapFlat;
	end_cap = PenLineCapFlat;
	
	calc_line_bounds (p1 ? p1->x : 0.0, 
			  p2 ? p2->x : 0.0, 
			  p1 ? p1->y : 0.0, 
			  p2 ? p2->y : 0.0, 
			  0.0, 
			  start_cap, 
			  end_cap,
			  &bounds);
	
	return bounds;
}

//
// PathFigureCollection
//

PathFigureCollection::PathFigureCollection ()
{
	SetObjectType (Type::PATHFIGURE_COLLECTION);
}

PathFigureCollection::~PathFigureCollection ()
{
}

//
// PathGeometry
//

PathGeometry::PathGeometry ()
{
	SetObjectType (Type::PATHGEOMETRY);
}

PathGeometry::~PathGeometry ()
{
}

// special case for the XAML parser when Path Markup Language (PML) is being used
PathGeometry::PathGeometry (moon_path *pml)
{
	SetObjectType (Type::PATHGEOMETRY);
	path = pml;
}

void
PathGeometry::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col != GetFigures ()) {
		Geometry::OnCollectionChanged (col, args);
		return;
	}
	
	InvalidateCache ();
	
	NotifyListenersOfPropertyChange (PathGeometry::FiguresProperty, NULL);
}

void
PathGeometry::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetFigures ()) {
		Geometry::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	InvalidateCache ();
	
	NotifyListenersOfPropertyChange (PathGeometry::FiguresProperty, NULL);
}

void
PathGeometry::Build ()
{
	PathFigureCollection *figures;
	PathFigure *figure;
	
	path = moon_path_renew (path, 0);

	if (!(figures = GetFigures ()))
		return;
	
	for (int i = 0; i < figures->GetCount (); i++) {
		figure = figures->GetValueAt (i)->AsPathFigure ();
		
		if (!figure->IsBuilt ())
			figure->Build ();
		
		moon_merge (path, figure->path);
	}
}

Rect
PathGeometry::ComputePathBounds ()
{
	if (!IsBuilt ())
		Build ();

	PathFigureCollection *figures = GetFigures ();
	if (!figures && (!path || (path->cairo.num_data == 0)))
		return Rect ();

	cairo_t *cr = measuring_context_create ();

	cairo_append_path (cr, &path->cairo);
	
	double x1, y1, x2, y2;

	cairo_path_extents (cr, &x1, &y1, &x2, &y2);

	Rect bounds = Rect (MIN (x1, x2), MIN (y1, y2), fabs (x2 - x1), fabs (y2 - y1));

	measuring_context_destroy (cr);

	return bounds;
}

//
// RectangleGeometry
//

RectangleGeometry::RectangleGeometry ()
{
	SetObjectType (Type::RECTANGLEGEOMETRY);
}

RectangleGeometry::~RectangleGeometry ()
{
}

void
RectangleGeometry::Build ()
{
	Rect *rect = GetRect ();
	if (!rect)
		return;
	
	double radius_x = GetRadiusX ();
	double radius_y = GetRadiusY ();
	path = moon_path_renew (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH);
	moon_rounded_rectangle (path, rect->x, rect->y, rect->width, rect->height, radius_x, radius_y);
}

Rect
RectangleGeometry::ComputePathBounds ()
{
	Rect *rect = GetRect ();
	Rect bounds;
	
	if (!rect)
		return Rect (0.0, 0.0, 0.0, 0.0);
	
	bounds = *rect;
	
	return bounds;
}

//
// PathSegmentCollection
//

PathSegmentCollection::PathSegmentCollection ()
{
	SetObjectType (Type::PATHSEGMENT_COLLECTION);
}

PathSegmentCollection::~PathSegmentCollection ()
{
}

//
// PathFigure
//

PathFigure::PathFigure ()
{
	SetObjectType (Type::PATHFIGURE);
	path = NULL;
}

PathFigure::~PathFigure ()
{
	if (path)
		moon_path_destroy (path);
}

void
PathFigure::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::PATHFIGURE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	if (path)
		moon_path_clear (path);

	NotifyListenersOfPropertyChange (args, error);
}

void
PathFigure::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col != GetSegments ()) {
		DependencyObject::OnCollectionChanged (col, args);
		return;
	}
	
	if (path)
		moon_path_clear (path);
	
	NotifyListenersOfPropertyChange (PathFigure::SegmentsProperty, NULL);
}

void
PathFigure::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetSegments ()) {
		DependencyObject::OnCollectionItemChanged (col, obj, args);
		return;
	}

	if (path)
		moon_path_clear (path);
	
	NotifyListenersOfPropertyChange (PathFigure::SegmentsProperty, NULL);
}

void
PathFigure::Build ()
{
	PathSegmentCollection *segments = GetSegments ();
	PathSegment *segment;
	
	if (path)
		moon_path_clear (path);
	else
		path = moon_path_new (MOON_PATH_MOVE_TO_LENGTH + (segments->GetCount () * 4) + MOON_PATH_CLOSE_PATH_LENGTH);
	
	Point *start = GetStartPoint ();
	moon_move_to (path, start ? start->x : 0.0, start ? start->y : 0.0);
	
	for (int i = 0; i < segments->GetCount (); i++) {
		segment = segments->GetValueAt (i)->AsPathSegment ();
		
		segment->Append (path);
	}
	
	if (GetIsClosed ())
		moon_close_path (path);
}

//
// PathSegment
//

PathSegment::PathSegment ()
{
	SetObjectType (Type::PATHSEGMENT);
}

PathSegment::~PathSegment ()
{
}

void
PathSegment::Build ()
{
}

void
PathSegment::Append (moon_path *path)
{
}

//
// ArcSegment
//

ArcSegment::ArcSegment ()
{
	SetObjectType (Type::ARCSEGMENT);
}

ArcSegment::~ArcSegment ()
{
}

void
ArcSegment::Append (moon_path *path)
{
	Size *size = GetSize ();
	double width = size ? size->width : 0.0;
	double height = size ? size->height : 0.0;

	Point *end_point = GetPoint ();
	double ex = end_point ? end_point->x : 0.0;
	double ey = end_point ? end_point->y : 0.0;

	moon_arc_to (path, width, height, GetRotationAngle (), GetIsLargeArc (), GetSweepDirection (), ex, ey);
}

//
// BezierSegment
//

BezierSegment::BezierSegment ()
{
	SetObjectType (Type::BEZIERSEGMENT);
}

BezierSegment::~BezierSegment ()
{
}

void
BezierSegment::Append (moon_path *path)
{
	Point *p1 = GetPoint1 ();
	Point *p2 = GetPoint2 ();
	Point *p3 = GetPoint3 ();
	
	double x1 = p1 ? p1->x : 0.0;
	double y1 = p1 ? p1->y : 0.0;
	double x2 = p2 ? p2->x : 0.0;
	double y2 = p2 ? p2->y : 0.0;
	double x3 = p3 ? p3->x : 0.0;
	double y3 = p3 ? p3->y : 0.0;

	moon_curve_to (path, x1, y1, x2, y2, x3, y3);
}

//
// LineSegment
//

LineSegment::LineSegment ()
{
	SetObjectType (Type::LINESEGMENT);
}

LineSegment::~LineSegment ()
{
}

void
LineSegment::Append (moon_path *path)
{
	Point *p = GetPoint ();
	
	double x = p ? p->x : 0.0;
	double y = p ? p->y : 0.0;

	moon_line_to (path, x, y);
}

//
// PolyBezierSegment
//

PolyBezierSegment::PolyBezierSegment ()
{
	SetObjectType (Type::POLYBEZIERSEGMENT);
}

PolyBezierSegment::~PolyBezierSegment ()
{
}

void
PolyBezierSegment::Append (moon_path *path)
{
	PointCollection *col;
	GPtrArray *points;
	
	col = GetPoints ();
	
	// we need at least 3 points
	if (!col || (col->GetCount() % 3) != 0)
		return;

	points = col->Array();
	
	for (int i = 0; i < col->GetCount() - 2; i += 3) {
		moon_curve_to (path,
			       ((Value*)g_ptr_array_index(points, i))->AsPoint()->x,
			       ((Value*)g_ptr_array_index(points, i))->AsPoint()->y,

			       ((Value*)g_ptr_array_index(points, i+1))->AsPoint()->x,
			       ((Value*)g_ptr_array_index(points, i+1))->AsPoint()->y,

			       ((Value*)g_ptr_array_index(points, i+2))->AsPoint()->x,
			       ((Value*)g_ptr_array_index(points, i+2))->AsPoint()->y);
	}
}

int
PolyBezierSegment::GetPathSize ()
{
	PointCollection *points = GetPoints ();
	int n = points ? points->GetCount() : 0;

	return (n / 3) * MOON_PATH_CURVE_TO_LENGTH;
}

//
// PolyLineSegment
//

PolyLineSegment::PolyLineSegment ()
{
	SetObjectType (Type::POLYLINESEGMENT);
}

PolyLineSegment::~PolyLineSegment ()
{
}

void
PolyLineSegment::Append (moon_path *path)
{
	PointCollection *col;
	GPtrArray *points;

	col = GetPoints ();
	
	if (!col)
		return;

	points = col->Array();
	
	for (int i = 0; i < col->GetCount(); i++)
		moon_line_to (path,
			      ((Value*)g_ptr_array_index(points, i))->AsPoint()->x,
			      ((Value*)g_ptr_array_index(points, i))->AsPoint()->y);
}

int
PolyLineSegment::GetPathSize ()
{
	PointCollection *points = GetPoints ();
	int n = points ? points->GetCount() : 0;
	
	return n * MOON_PATH_LINE_TO_LENGTH;
}

//
// PolyQuadraticBezierSegment
//

// quadratic to cubic bezier, the original control point and the end control point are the same
// http://web.archive.org/web/20020209100930/http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/node2.html
//
// note: we dont call moon_quad_curve_to to avoid calling moon_get_current_point 
// on each curve (since all but the first one is known)

PolyQuadraticBezierSegment::PolyQuadraticBezierSegment ()
{
	SetObjectType (Type::POLYQUADRATICBEZIERSEGMENT);
}

PolyQuadraticBezierSegment::~PolyQuadraticBezierSegment ()
{
}

void
PolyQuadraticBezierSegment::Append (moon_path *path)
{
	PointCollection *col;
	GPtrArray *points;
	
	col = GetPoints ();
	
	if (!col || ((col->GetCount() % 2) != 0))
		return;
	
	// origin
	double x0 = 0.0;
	double y0 = 0.0;
	moon_get_current_point (path, &x0, &y0);
	
	points = col->Array();

	// we need at least 2 points
	for (int i = 0; i < col->GetCount() - 1; i+=2) {
		double x1 = ((Value*)g_ptr_array_index(points, i))->AsPoint()->x;
		double y1 = ((Value*)g_ptr_array_index(points, i))->AsPoint()->y;
		double x2 = ((Value*)g_ptr_array_index(points, i+1))->AsPoint()->x;
		double y2 = ((Value*)g_ptr_array_index(points, i+1))->AsPoint()->y;
		double x3 = x2;
		double y3 = y2;
		
		x2 = x1 + (x2 - x1) / 3;
		y2 = y1 + (y2 - y1) / 3;
		x1 = x0 + 2 * (x1 - x0) / 3;
		y1 = y0 + 2 * (y1 - y0) / 3;
		
		moon_curve_to (path, x1, y1, x2, y2, x3, y3);
		
		// set new origin
		x0 = x3;
		y0 = y3;
	}
}

int
PolyQuadraticBezierSegment::GetPathSize ()
{
	PointCollection* points = GetPoints ();

	int n = points ? points->GetCount() : 0;

	return (n / 2) * MOON_PATH_CURVE_TO_LENGTH;
}

//
// QuadraticBezierSegment
//

QuadraticBezierSegment::QuadraticBezierSegment ()
{
	SetObjectType (Type::QUADRATICBEZIERSEGMENT);
}

QuadraticBezierSegment::~QuadraticBezierSegment ()
{
}

void
QuadraticBezierSegment::Append (moon_path *path)
{
	Point *p1 = GetPoint1 ();
	Point *p2 = GetPoint2 ();

	double x1 = p1 ? p1->x : 0.0;
	double y1 = p1 ? p1->y : 0.0;
	double x2 = p2 ? p2->x : 0.0;
	double y2 = p2 ? p2->y : 0.0;

	moon_quad_curve_to (path, x1, y1, x2, y2);
}
