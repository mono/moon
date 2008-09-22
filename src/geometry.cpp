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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include "utils.h"
#include "geometry.h"
#include "shape.h"

//
// Geometry
//

Geometry::~Geometry ()
{
	if (path)
		moon_path_destroy (path);
}

void
Geometry::Draw (Path *shape, cairo_t *cr)
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
		Build (shape);

	if (path)
		cairo_append_path (cr, &path->cairo);

	cairo_set_matrix (cr, &saved);
}

void
Geometry::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	// no need to clear the path for Geometry itself as FillRule and Transform properties are 
	// only used when drawing, i.e. they do not affect the path itself
	if ((args->property->GetOwnerType() != Type::GEOMETRY) && path)
		moon_path_clear (path);

	NotifyListenersOfPropertyChange (args);
}

//
// GeometryGroup
//

GeometryGroup::GeometryGroup ()
{
	SetValue (GeometryGroup::ChildrenProperty, Value::CreateUnref (new GeometryCollection ()));
}

void
GeometryGroup::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	NotifyListenersOfPropertyChange (prop);
}

void
GeometryGroup::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col != GetChildren ()) {
		Geometry::OnCollectionChanged (col, args);
		return;
	}
	
	NotifyListenersOfPropertyChange (GeometryGroup::ChildrenProperty);
}

void
GeometryGroup::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetChildren ()) {
		Geometry::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	NotifyListenersOfPropertyChange (GeometryGroup::ChildrenProperty);
}

void
GeometryGroup::Draw (Path *shape, cairo_t *cr)
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
	
	for (int i = 0; i < children->GetCount (); i++) {
		geometry = children->GetValueAt (i)->AsGeometry ();
		
		geometry->Draw (shape, cr);
	}
	
	cairo_set_matrix (cr, &saved);
}

Rect
GeometryGroup::ComputeBounds (Path *path, bool logical, cairo_matrix_t * matrix)
{
	GeometryCollection *children = GetChildren ();
	Rect bounds = Rect (0.0, 0.0, 0.0, 0.0);
	Geometry *geometry;
	
	for (int i = 0; i < children->GetCount (); i++) {
		geometry = children->GetValueAt (i)->AsGeometry ();
		
		bounds = bounds.Union (geometry->ComputeBounds (path, logical, matrix), logical);
	}
	
	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}
	
	//g_warning ("GeometryGroup::ComputeBounds - x %g y %g w %g h %g", bounds.x, bounds.y, bounds.w, bounds.h);
	return bounds;
}

#if 0
Point
Geometry::GetOriginPoint (Path *shape)
{
	double x = 0.0;
	double y = 0.0;

	if (!IsBuilt ())
		Build (shape);

	moon_get_origin (path, &x, &y);
	return Point (x, y);
}
#endif

//
// EllipseGeometry
//

void
EllipseGeometry::Build (Path *shape)
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
EllipseGeometry::ComputeBounds (Path *path, bool logical)
{
	// code written to minimize divisions
	double ht;
	if (logical)
		ht = 0.0;
	else
		ht = (path ? path->GetStrokeThickness () : 1.0) / 2.0;

	double hw = GetRadiusX () + ht;
	double hh = GetRadiusY () + ht;
	// point is at center, so left-top corner is minus half width / half height
	Point *pt = GetCenter ();
	double x = pt ? pt->x : 0.0;
	double y = pt ? pt->y : 0.0;
	Rect bounds;
	
	bounds = Rect (x - hw, y - hh, hw * 2.0, hh * 2.0);
	
	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}
	
	return bounds;
}

//
// LineGeometry
//

void
LineGeometry::Build (Path *shape)
{
	Point *p1 = GetStartPoint ();
	Point *p2 = GetEndPoint ();
	
	path = moon_path_renew (path, MOON_PATH_MOVE_TO_LENGTH + MOON_PATH_LINE_TO_LENGTH);
	moon_move_to (path, p1 ? p1->x : 0.0, p1 ? p1->y : 0.0);
	moon_line_to (path, p2 ? p2->x : 0.0, p2 ? p2->y : 0.0);
}

Rect
LineGeometry::ComputeBounds (Path *shape, bool logical)
{
	Point *p1 = GetStartPoint ();
	Point *p2 = GetEndPoint ();
	double thickness;
	PenLineCap start_cap;
	PenLineCap end_cap;
	Rect bounds;

	if (shape) {
		start_cap = shape->GetStrokeStartLineCap ();
		end_cap = shape->GetStrokeEndLineCap ();
		thickness = (logical) ? 0.0 : shape->GetStrokeThickness ();
	} else {
		start_cap = PenLineCapFlat;
		end_cap = PenLineCapFlat;
		thickness = 0.0;
	}
	
	calc_line_bounds (p1 ? p1->x : 0.0, p2 ? p2->x : 0.0, p1 ? p1->y : 0.0, p2 ? p2->y : 0.0, 
		thickness, start_cap, end_cap, &bounds);
	
	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}

	return bounds;
}

//
// PathGeometry
//

PathGeometry::PathGeometry ()
{
	logical_bounds_available = physical_bounds_available = false;
}

// special case for the XAML parser when Path Markup Language (PML) is being used
PathGeometry::PathGeometry (moon_path *pml)
{
	logical_bounds_available = physical_bounds_available = false;
	path = pml;
}

void
PathGeometry::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col != GetFigures ()) {
		Geometry::OnCollectionChanged (col, args);
		return;
	}
	
	logical_bounds_available = physical_bounds_available = false;
	if (path)
		moon_path_clear (path);
	
	NotifyListenersOfPropertyChange (PathGeometry::FiguresProperty);
}

void
PathGeometry::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetFigures ()) {
		Geometry::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	logical_bounds_available = physical_bounds_available = false;
	if (path)
		moon_path_clear (path);
	
	NotifyListenersOfPropertyChange (PathGeometry::FiguresProperty);
}

void
PathGeometry::Build (Path *shape)
{
	PathFigureCollection *figures;
	PathFigure *figure;
	
	path = moon_path_renew (path, 0);

	if (!(figures = GetFigures ()))
		return;
	
	for (int i = 0; i < figures->GetCount (); i++) {
		figure = figures->GetValueAt (i)->AsPathFigure ();
		
		if (!figure->IsBuilt ())
			figure->Build (shape);
		
		moon_merge (path, figure->path);
	}
}

Rect
PathGeometry::ComputeBounds (Path *shape, bool logical, cairo_matrix_t *matrix)
{
	Rect bounds;

	if (logical) {
		if (!logical_bounds_available) {
			logical_bounds = CacheBounds (shape, true, NULL);
			logical_bounds_available = true;
		}
		bounds = logical_bounds;
	} else {
		if (!physical_bounds_available) {
			physical_bounds = CacheBounds (shape, false, matrix);
			physical_bounds_available = true;
		}
		bounds = physical_bounds;
	}

	return bounds;
}

Rect
PathGeometry::CacheBounds (Path *shape, bool logical, cairo_matrix_t *matrix)
{
	if (!IsBuilt ())
		Build (shape);

	PathFigureCollection *figures = GetFigures ();
	if (!figures && (!path || (path->cairo.num_data == 0)))
		return Rect ();

	double thickness = (logical || !shape || !shape->IsStroked ()) ? 0.0 : shape->GetStrokeThickness ();
	
	cairo_t *cr = measuring_context_create ();
	cairo_set_line_width (cr, thickness);

	if (matrix) 
		cairo_set_matrix (cr, matrix);
	cairo_append_path (cr, &path->cairo);
	if (matrix) 
		cairo_identity_matrix (cr);
	
	double x1, y1, x2, y2;

	if (logical) {
		cairo_path_extents (cr, &x1, &y1, &x2, &y2);
	} else if (thickness > 0) {
		cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	} else {
		cairo_fill_extents (cr, &x1, &y1, &x2, &y2);
	}

	Rect bounds = Rect (MIN (x1, x2), MIN (y1, y2), fabs (x2 - x1), fabs (y2 - y1));

	measuring_context_destroy (cr);

	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}
	
	return bounds;
}

//
// RectangleGeometry
//

void
RectangleGeometry::Build (Path *shape)
{
	Rect *rect = GetRect ();
	if (!rect)
		return;
	
	double radius_x = 0, radius_y = 0;
	GetRadius (&radius_x, &radius_y);
	path = moon_path_renew (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH);
	moon_rounded_rectangle (path, rect->x, rect->y, rect->width, rect->height, radius_x, radius_y);
}

Rect
RectangleGeometry::ComputeBounds (Path *path, bool logical)
{
	Rect *rect = GetRect ();
	Rect bounds;
	
	if (!rect)
		return Rect (0.0, 0.0, 0.0, 0.0);
	
	double thickness;
	if (path && !logical) {
		thickness = path->IsStroked () ? path->GetStrokeThickness () : 0;
	} else
		thickness = 0.0;

	//bounds = *rect;
	bounds = rect->GrowBy (thickness / 2.0);

	
	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}

	return bounds;
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

PathFigure::PathFigure ()
{
	path = NULL;
	SetValue (PathFigure::SegmentsProperty, Value::CreateUnref (new PathSegmentCollection ()));
}

PathFigure::~PathFigure ()
{
	if (path)
		moon_path_destroy (path);
}

void
PathFigure::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::PATHFIGURE) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}

	if (path)
		moon_path_clear (path);

	NotifyListenersOfPropertyChange (args);
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
	
	NotifyListenersOfPropertyChange (PathFigure::SegmentsProperty);
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
	
	NotifyListenersOfPropertyChange (PathFigure::SegmentsProperty);
}

void
PathFigure::Build (Path *shape)
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
// ArcSegment
//

void
ArcSegment::Append (moon_path *path)
{
	Point *size = GetSize ();
	double width = size ? size->x : 0.0;
	double height = size ? size->y : 0.0;

	Point *end_point = GetPoint ();
	double ex = end_point ? end_point->x : 0.0;
	double ey = end_point ? end_point->y : 0.0;

	moon_arc_to (path, width, height, GetRotationAngle (), GetIsLargeArc (), GetSweepDirection (), ex, ey);
}

//
// BezierSegment
//

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
