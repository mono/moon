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
#include "array.h"

//
// Geometry
//

DependencyProperty *Geometry::FillRuleProperty;
DependencyProperty *Geometry::TransformProperty;


Geometry::~Geometry ()
{
	if (path)
		moon_path_destroy (path);
}

void
Geometry::Draw (Path *shape, cairo_t *cr)
{
	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}

	if (!IsBuilt ())
		Build (shape);

	if (path)
		cairo_append_path (cr, &path->cairo);
}

void
Geometry::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	// no need to clear the path for Geometry itself as FillRule and Transform properties are 
	// only used when drawing, i.e. they do not affect the path itself
	if ((args->property->type != Type::GEOMETRY) && path)
		moon_path_clear (path);

	NotifyListenersOfPropertyChange (args);
}

void
Geometry::SetFillRule (FillRule rule)
{
	SetValue (Geometry::FillRuleProperty, Value (rule));
}

FillRule
Geometry::GetFillRule ()
{
	return (FillRule) GetValue (Geometry::FillRuleProperty)->AsInt32 ();
}

void
Geometry::SetTransform (Transform *transform)
{
	SetValue (Geometry::TransformProperty, Value (transform));
}

Transform *
Geometry::GetTransform ()
{
	Value *value = GetValue (Geometry::TransformProperty);
	
	return value ? value->AsTransform () : NULL;
}

FillRule
geometry_get_fill_rule (Geometry *geometry)
{
	return geometry->GetFillRule ();
}

void
geometry_set_fill_rule (Geometry *geometry, FillRule rule)
{
	geometry->SetFillRule (rule);
}

Transform *
geometry_get_transform (Geometry *geometry)
{
	return geometry->GetTransform ();
}

void
geometry_set_transform (Geometry *geometry, Transform *transform)
{
	geometry->SetTransform (transform);
}



//
// GeometryGroup
//

DependencyProperty *GeometryGroup::ChildrenProperty;


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
GeometryGroup::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	// GeometryGroup only has one collection, so let's save the hash lookup
	//if (col == GetValue (GeometryGroup::ChildrenProperty)->AsGeometryCollection())
		NotifyListenersOfPropertyChange (GeometryGroup::ChildrenProperty);
}

void
GeometryGroup::Draw (Path *shape, cairo_t *cr)
{
	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}
	
	GeometryCollection *children = GetChildren ();
	Collection::Node *node;
	
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Geometry *geometry = (Geometry *) node->obj;
		
		geometry->Draw (shape, cr);
	}
}

Rect
GeometryGroup::ComputeBounds (Path *path, bool logical, cairo_matrix_t * matrix)
{
	GeometryCollection *children = GetChildren ();
	Rect bounds = Rect (0.0, 0.0, 0.0, 0.0);
	
	Collection::Node *node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Geometry *geometry = (Geometry *) node->obj;
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

void
GeometryGroup::SetChildren (GeometryCollection *children)
{
	SetValue (GeometryGroup::ChildrenProperty, Value (children));
}

GeometryCollection *
GeometryGroup::GetChildren ()
{
	Value *value = GetValue (GeometryGroup::ChildrenProperty);
	
	return value ? value->AsGeometryCollection () : NULL;
}


GeometryGroup *
geometry_group_new (void)
{
	return new GeometryGroup ();
}

GeometryCollection *
geometry_group_get_children (GeometryGroup *group)
{
	return group->GetChildren ();
}

void
geometry_group_set_children (GeometryGroup *group, GeometryCollection *children)
{
	group->SetChildren (children);
}


//
// GeometryCollection
//

GeometryCollection *
geometry_collection_new (void)
{
	return new GeometryCollection ();
}


//
// PathFigureCollection
//

PathFigureCollection *
path_figure_collection_new (void)
{
	return new PathFigureCollection ();
}


//
// PathSegmentCollection
//

PathSegmentCollection *
path_segment_collection_new (void)
{
	return new PathSegmentCollection ();
}


//
// EllipseGeometry
//

DependencyProperty *EllipseGeometry::CenterProperty;
DependencyProperty *EllipseGeometry::RadiusXProperty;
DependencyProperty *EllipseGeometry::RadiusYProperty;


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

void
EllipseGeometry::SetCenter (Point *center)
{
	SetValue (EllipseGeometry::CenterProperty, Value (*center));
}

Point *
EllipseGeometry::GetCenter ()
{
	Value *value = GetValue (EllipseGeometry::CenterProperty);
	
	return value ? value->AsPoint () : NULL;
}

void
EllipseGeometry::SetRadiusX (double radius)
{
	SetValue (EllipseGeometry::RadiusXProperty, Value (radius));
}

double
EllipseGeometry::GetRadiusX ()
{
	return GetValue (EllipseGeometry::RadiusXProperty)->AsDouble ();
}

void
EllipseGeometry::SetRadiusY (double radius)
{
	SetValue (EllipseGeometry::RadiusYProperty, Value (radius));
}

double
EllipseGeometry::GetRadiusY ()
{
	return GetValue (EllipseGeometry::RadiusYProperty)->AsDouble ();
}


EllipseGeometry *
ellipse_geometry_new (void)
{
	return new EllipseGeometry ();
}

Point *
ellipse_geometry_get_center (EllipseGeometry *ellipse)
{
	return ellipse->GetCenter ();
}

void
ellipse_geometry_set_center (EllipseGeometry *ellipse, Point *center)
{
	ellipse->SetCenter (center);
}

double
ellipse_geometry_get_radius_x (EllipseGeometry *ellipse)
{
	return ellipse->GetRadiusX ();
}

void
ellipse_geometry_set_radius_x (EllipseGeometry *ellipse, double radius)
{
	ellipse->SetRadiusX (radius);
}

double
ellipse_geometry_get_radius_y (EllipseGeometry *ellipse)
{
	return ellipse->GetRadiusY ();
}

void
ellipse_geometry_set_radius_y (EllipseGeometry *ellipse, double radius)
{
	ellipse->SetRadiusY (radius);
}



//
// LineGeometry
//

DependencyProperty *LineGeometry::EndPointProperty;
DependencyProperty *LineGeometry::StartPointProperty;


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
	Rect bounds;
	
	if (shape && !logical)
		thickness = shape->GetStrokeThickness ();
	else
		thickness = 0.0;
	
	calc_line_bounds (p1 ? p1->x : 0.0, p2 ? p2->x : 0.0, p1 ? p1->y : 0.0, p2 ? p2->y : 0.0, thickness, &bounds);
	
	Transform *transform = GetTransform ();
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}

	return bounds;
}

void
LineGeometry::SetEndPoint (Point *point)
{
	SetValue (LineGeometry::EndPointProperty, Value (*point));
}

Point *
LineGeometry::GetEndPoint ()
{
	Value *value = GetValue (LineGeometry::EndPointProperty);
	
	return value ? value->AsPoint () : NULL;
}

void
LineGeometry::SetStartPoint (Point *point)
{
	SetValue (LineGeometry::StartPointProperty, Value (*point));
}

Point *
LineGeometry::GetStartPoint ()
{
	Value *value = GetValue (LineGeometry::StartPointProperty);
	
	return value ? value->AsPoint () : NULL;
}


LineGeometry *
line_geometry_new (void)
{
	return new LineGeometry ();
}

Point *
line_geometry_get_end_point (LineGeometry *line)
{
	return line->GetEndPoint ();
}

void
line_geometry_set_end_point (LineGeometry *line, Point *point)
{
	line->SetEndPoint (point);
}

Point *
line_geometry_get_start_point (LineGeometry *line)
{
	return line->GetStartPoint ();
}

void
line_geometry_set_start_point (LineGeometry *line, Point *point)
{
	line->SetStartPoint (point);
}


//
// PathGeometry
//

DependencyProperty *PathGeometry::FiguresProperty;

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
PathGeometry::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	logical_bounds_available = physical_bounds_available = false;
	if (path)
		moon_path_clear (path);

	// PathGeometry only has one collection, so let's save the hash lookup
	//if (col == GetValue (PathGeometry::FiguresProperty)->AsPathFigureCollection ())
	        NotifyListenersOfPropertyChange (PathGeometry::FiguresProperty);
}

void
PathGeometry::Build (Path *shape)
{
	path = moon_path_renew (path, 0);

	PathFigureCollection *figures = GetFigures ();
	if (!figures)
		return;
	
	Collection::Node *node = (Collection::Node *) figures->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathFigure *figure = (PathFigure *) node->obj;
		
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

void
PathGeometry::SetFigures (PathFigureCollection *figures)
{
	SetValue (PathGeometry::FiguresProperty, Value (figures));
}

PathFigureCollection *
PathGeometry::GetFigures ()
{
	Value *value = GetValue (PathGeometry::FiguresProperty);
	
	return value ? value->AsPathFigureCollection () : NULL;
}

PathGeometry *
path_geometry_new (void)
{
	return new PathGeometry ();
}

PathFigureCollection *
path_geometry_get_figures (PathGeometry *path)
{
	return path->GetFigures ();
}

void
path_geometry_set_figures (PathGeometry *path, PathFigureCollection *figures)
{
	path->SetFigures (figures);
}


//
// RectangleGeometry
//

DependencyProperty *RectangleGeometry::RadiusXProperty;
DependencyProperty *RectangleGeometry::RadiusYProperty;
DependencyProperty *RectangleGeometry::RectProperty;


void
RectangleGeometry::Build (Path *shape)
{
	Rect *rect = GetRect ();
	
	if (!rect)
		return;
	
	double half_thick = 0.0;
	// shape is optional (e.g. not available for clipping)
	if (shape) {
		double thick = shape->GetStrokeThickness ();
		if ((thick > rect->w) || (thick > rect->h)) {
			half_thick = thick / 2.0;
			rect->x -= half_thick;
			rect->y -= half_thick;
			rect->w += thick;
			rect->h += thick;
/* FIXME
 * - this doesn't match MS-SL if mixed with some "normal" (non-degenerated) geometry
 */
			shape->SetShapeFlags (UIElement::SHAPE_DEGENERATE);
		}
	}

	double radius_x, radius_y;
	if (GetRadius (&radius_x, &radius_y)) {
		path = moon_path_renew (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH);
		moon_rounded_rectangle (path, rect->x, rect->y, rect->w, rect->h, radius_x + half_thick, radius_y + half_thick);
	} else {
		path = moon_path_renew (path, MOON_PATH_RECTANGLE_LENGTH);
		moon_rectangle (path, rect->x, rect->y, rect->w, rect->h);
	}
}

Rect
RectangleGeometry::ComputeBounds (Path *path, bool logical)
{
	Rect *rect = GetRect ();
	Rect bounds;
	
	if (!rect)
		return Rect (0.0, 0.0, 0.0, 0.0);
	
	double thickness;
	if (path && !logical)
		thickness = path->GetStrokeThickness ();
	else
		thickness = 0.0;
	
	// UIElement::SHAPE_DEGENERATE flags may be unset at this stage
	if ((thickness > rect->w) || (thickness > rect->h))
		thickness += 2.0;
	
	if (! logical)
		bounds = rect->GrowBy (thickness / 2.0);
	else
		bounds = *rect;
	
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

void
RectangleGeometry::SetRadiusX (double radius)
{
	SetValue (RectangleGeometry::RadiusXProperty, Value (radius));
}

double
RectangleGeometry::GetRadiusX ()
{
	return GetValue (RectangleGeometry::RadiusXProperty)->AsDouble ();
}

void
RectangleGeometry::SetRadiusY (double radius)
{
	SetValue (RectangleGeometry::RadiusYProperty, Value (radius));
}

double
RectangleGeometry::GetRadiusY ()
{
	return GetValue (RectangleGeometry::RadiusYProperty)->AsDouble ();
}

void
RectangleGeometry::SetRect (Rect *rect)
{
	SetValue (RectangleGeometry::RectProperty, Value (*rect));
}

Rect *
RectangleGeometry::GetRect ()
{
	Value *value = GetValue (RectangleGeometry::RectProperty);
	
	return value ? value->AsRect () : NULL;
}


RectangleGeometry *
rectangle_geometry_new (void)
{
	return new RectangleGeometry ();
}

double
rectangle_geometry_get_radius_x (RectangleGeometry *rectangle)
{
	return rectangle->GetRadiusX ();
}

void
rectangle_geometry_set_radius_x (RectangleGeometry *rectangle, double radius)
{
	rectangle->SetRadiusX (radius);
}

double
rectangle_geometry_get_radius_y (RectangleGeometry *rectangle)
{
	return rectangle->GetRadiusY ();
}

void
rectangle_geometry_set_radius_y (RectangleGeometry *rectangle, double radius)
{
	rectangle->SetRadiusY (radius);
}

Rect *
rectangle_geometry_get_rect (RectangleGeometry *rectangle)
{
	return rectangle->GetRect ();
}

void
rectangle_geometry_set_rect (RectangleGeometry *rectangle, Rect *rect)
{
	rectangle->SetRect (rect);
}


//
// PathFigure
//

DependencyProperty *PathFigure::IsClosedProperty;
DependencyProperty *PathFigure::SegmentsProperty;
DependencyProperty *PathFigure::StartPointProperty;


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
	if (args->property->type != Type::PATHFIGURE) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}

	if (path)
		moon_path_clear (path);

	NotifyListenersOfPropertyChange (args);
}

void
PathFigure::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	if (path)
		moon_path_clear (path);
	// PathFigure only has one collection, so let's save the hash lookup
	//if (col == GetValue (PathFigure::SegmentsProperty)->AsPathSegmentCollection())
		NotifyListenersOfPropertyChange (PathFigure::SegmentsProperty);
}

void
PathFigure::Build (Path *shape)
{
	PathSegmentCollection *segments = GetSegments ();
	
	if (path)
		moon_path_clear (path);
	else
		path = moon_path_new (MOON_PATH_MOVE_TO_LENGTH + (segments->list->Length () * 4) + MOON_PATH_CLOSE_PATH_LENGTH);
	
	Point *start = GetStartPoint ();
	moon_move_to (path, start ? start->x : 0.0, start ? start->y : 0.0);
	
	Collection::Node *node = (Collection::Node *) segments->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathSegment *segment = (PathSegment *) node->obj;
		
		segment->Append (path);
	}
	
	if (GetIsClosed ())
		moon_close_path (path);
}

Rect
PathFigure::ComputeBounds (cairo_t *cr, Path *shape, bool logical, double thickness, cairo_matrix_t *matrix)
{
	if (!IsBuilt ())
		Build (shape);

	if (!path)
		return Rect (0.0, 0.0, 0.0, 0.0);

	if (matrix) 
		cairo_set_matrix (cr, matrix);

	cairo_append_path (cr, &path->cairo);
	if (matrix) 
		cairo_identity_matrix (cr);
	
	double x1, y1, x2, y2;

	if (logical || !shape) {
		cairo_path_extents (cr, &x1, &y1, &x2, &y2);
	} else {
		if (thickness > 0.0) {
			cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
		} else if (shape->CanFill() && shape->IsFilled ()) {
			cairo_fill_extents (cr, &x1, &y1, &x2, &y2);
		} else {
			return Rect (0.0, 0.0, 0.0, 0.0);
		}
	}

	return Rect (MIN (x1, x2), MIN (y1, y2), fabs (x2 - x1), fabs (y2 - y1));
}

void
PathFigure::SetIsClosed (bool closed)
{
	SetValue (PathFigure::IsClosedProperty, Value (closed));
}

bool
PathFigure::GetIsClosed ()
{
	return GetValue (PathFigure::IsClosedProperty)->AsBool ();
}

void
PathFigure::SetSegments (PathSegmentCollection *segments)
{
	SetValue (PathFigure::SegmentsProperty, Value (segments));
}

PathSegmentCollection *
PathFigure::GetSegments ()
{
	Value *value = GetValue (PathFigure::SegmentsProperty);
	
	return value ? value->AsPathSegmentCollection () : NULL;
}

void
PathFigure::SetStartPoint (Point *point)
{
	SetValue (PathFigure::StartPointProperty, Value (*point));
}

Point *
PathFigure::GetStartPoint ()
{
	Value *value = GetValue (PathFigure::StartPointProperty);
	
	return value ? value->AsPoint () : NULL;
}


PathFigure *
path_figure_new (void)
{
	return new PathFigure ();
}

bool
path_figure_get_is_closed (PathFigure *figure)
{
	return figure->GetIsClosed ();
}

void
path_figure_set_is_closed (PathFigure *figure, bool closed)
{
	figure->SetIsClosed (closed);
}

PathSegmentCollection *
path_figure_get_segments (PathFigure *figure)
{
	return figure->GetSegments ();
}

void
path_figure_set_segments (PathFigure *figure, PathSegmentCollection *segments)
{
	figure->SetSegments (segments);
}

Point *
path_figure_get_start_point (PathFigure *figure)
{
	return figure->GetStartPoint ();
}

void
path_figure_set_start_point (PathFigure *figure, Point *point)
{
	figure->SetStartPoint (point);
}


//
// ArcSegment
//

DependencyProperty *ArcSegment::IsLargeArcProperty;
DependencyProperty *ArcSegment::PointProperty;
DependencyProperty *ArcSegment::RotationAngleProperty;
DependencyProperty *ArcSegment::SizeProperty;
DependencyProperty *ArcSegment::SweepDirectionProperty;


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

void
ArcSegment::SetIsLargeArc (bool large)
{
	SetValue (ArcSegment::IsLargeArcProperty, Value (large));
}

bool
ArcSegment::GetIsLargeArc ()
{
	return GetValue (ArcSegment::IsLargeArcProperty)->AsBool ();
}

void
ArcSegment::SetPoint (Point *point)
{
	SetValue (ArcSegment::PointProperty, Value (*point));
}

Point *
ArcSegment::GetPoint ()
{
	Value *value = GetValue (ArcSegment::PointProperty);
	
	return value ? value->AsPoint () : NULL;
}

void
ArcSegment::SetRotationAngle (double angle)
{
	SetValue (ArcSegment::RotationAngleProperty, Value (angle));
}

double
ArcSegment::GetRotationAngle ()
{
	return GetValue (ArcSegment::RotationAngleProperty)->AsDouble ();
}

void
ArcSegment::SetSize (Point *size)
{
	SetValue (ArcSegment::SizeProperty, Value (*size));
}

Point *
ArcSegment::GetSize ()
{
	Value *value = GetValue (ArcSegment::SizeProperty);
	
	return value ? value->AsPoint () : NULL;
}

void
ArcSegment::SetSweepDirection (SweepDirection direction)
{
	SetValue (ArcSegment::SweepDirectionProperty, Value (direction));
}

SweepDirection
ArcSegment::GetSweepDirection ()
{
	return (SweepDirection) GetValue (ArcSegment::SweepDirectionProperty)->AsInt32 ();
}



ArcSegment *
arc_segment_new (void)
{
	return new ArcSegment ();
}

bool
arc_segment_get_is_large_arc (ArcSegment *segment)
{
	return segment->GetIsLargeArc ();
}

void
arc_segment_set_is_large_arc (ArcSegment *segment, bool large)
{
	segment->SetIsLargeArc (large);
}

Point *
arc_segment_get_point (ArcSegment *segment)
{
	return segment->GetPoint ();
}

void
arc_segment_set_point (ArcSegment *segment, Point *point)
{
	segment->SetPoint (point);
}

double
arc_segment_get_rotation_angle (ArcSegment *segment)
{
	return segment->GetRotationAngle ();
}

void
arc_segment_set_rotation_angle (ArcSegment *segment, double angle)
{
	segment->SetRotationAngle (angle);
}

Point *
arc_segment_get_size (ArcSegment *segment)
{
	return segment->GetSize ();
}

void
arc_segment_set_size (ArcSegment *segment, Point *size)
{
	segment->SetSize (size);
}

SweepDirection
arc_segment_get_sweep_direction (ArcSegment *segment)
{
	return segment->GetSweepDirection ();
}

void
arc_segment_set_sweep_direction (ArcSegment *segment, SweepDirection direction)
{
	segment->SetSweepDirection (direction);
}


//
// BezierSegment
//

DependencyProperty *BezierSegment::Point1Property;
DependencyProperty *BezierSegment::Point2Property;
DependencyProperty *BezierSegment::Point3Property;


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

void
BezierSegment::SetPoint1 (Point *point)
{
	SetValue (BezierSegment::Point1Property, Value (*point));
}

Point *
BezierSegment::GetPoint1 ()
{
	Value *value = GetValue (BezierSegment::Point1Property);
	
	return value ? value->AsPoint () : NULL;
}

void
BezierSegment::SetPoint2 (Point *point)
{
	SetValue (BezierSegment::Point2Property, Value (*point));
}

Point *
BezierSegment::GetPoint2 ()
{
	Value *value = GetValue (BezierSegment::Point2Property);
	
	return value ? value->AsPoint () : NULL;
}

void
BezierSegment::SetPoint3 (Point *point)
{
	SetValue (BezierSegment::Point3Property, Value (*point));
}

Point *
BezierSegment::GetPoint3 ()
{
	Value *value = GetValue (BezierSegment::Point3Property);
	
	return value ? value->AsPoint () : NULL;
}


BezierSegment *
bezier_segment_new (void)
{
	return new BezierSegment ();
}

Point *
bezier_segment_get_point1 (BezierSegment *segment)
{
	return segment->GetPoint1 ();
}

void
bezier_segment_set_point1 (BezierSegment *segment, Point *point)
{
	segment->SetPoint1 (point);
}

Point *
bezier_segment_get_point2 (BezierSegment *segment)
{
	return segment->GetPoint2 ();
}

void
bezier_segment_set_point2 (BezierSegment *segment, Point *point)
{
	segment->SetPoint2 (point);
}

Point *
bezier_segment_get_point3 (BezierSegment *segment)
{
	return segment->GetPoint3 ();
}

void
bezier_segment_set_point3 (BezierSegment *segment, Point *point)
{
	segment->SetPoint3 (point);
}


//
// LineSegment
//

DependencyProperty *LineSegment::PointProperty;


void
LineSegment::Append (moon_path *path)
{
	Point *p = GetPoint ();
	
	double x = p ? p->x : 0.0;
	double y = p ? p->y : 0.0;

	moon_line_to (path, x, y);
}

void
LineSegment::SetPoint (Point *point)
{
	SetValue (LineSegment::PointProperty, Value (*point));
}

Point *
LineSegment::GetPoint ()
{
	Value *value = GetValue (LineSegment::PointProperty);
	
	return value ? value->AsPoint () : NULL;
}


LineSegment *
line_segment_new (void)
{
	return new LineSegment ();
}

Point *
line_segment_get_point (LineSegment *segment)
{
	return segment->GetPoint ();
}

void
line_segment_set_point (LineSegment *segment, Point *point)
{
	segment->SetPoint (point);
}


//
// PolyBezierSegment
//

DependencyProperty *PolyBezierSegment::PointsProperty;

void
PolyBezierSegment::Append (moon_path *path)
{
	Point *points;
	int n = 0;
	
	points = GetPoints (&n);
	
	// we need at least 3 points
	if (!points || (n % 3) != 0)
		return;
	
	for (int i = 0; i < n - 2; i += 3) {
		moon_curve_to (path, points[i].x, points[i].y, points[i+1].x, points[i+1].y,
			       points[i+2].x, points[i+2].y);
	}
}

int
PolyBezierSegment::GetPathSize ()
{
	int n = 0;
	
	GetPoints (&n);
	
	return (n / 3) * MOON_PATH_CURVE_TO_LENGTH;
}

void
PolyBezierSegment::SetPoints (Point *points, int n)
{
	SetValue (PolyBezierSegment::PointsProperty, Value (points, n));
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight PolyBezierSegment.Points only has a setter (no getter), so it's
 * use is only internal.
 */
Point *
PolyBezierSegment::GetPoints (int *n)
{
	Value *value = GetValue (PolyBezierSegment::PointsProperty);
	
	if (!value) {
		*n = 0;
		return NULL;
	}
	
	PointArray *array = value->AsPointArray ();
	*n = array->basic.count;
	
	return array->points;
}


PolyBezierSegment *
poly_bezier_segment_new (void)
{
	return new PolyBezierSegment ();
}

void
poly_bezier_segment_set_points (PolyBezierSegment *segment, Point *points, int n)
{
	segment->SetPoints (points, n);
}


//
// PolyLineSegment
//

DependencyProperty *PolyLineSegment::PointsProperty;


void
PolyLineSegment::Append (moon_path *path)
{
	Point *points;
	int n = 0;
	
	points = GetPoints (&n);
	
	for (int i = 0; i < n; i++)
		moon_line_to (path, points[i].x, points[i].y);
}

int
PolyLineSegment::GetPathSize ()
{
	int n = 0;
	
	GetPoints (&n);
	
	return n * MOON_PATH_LINE_TO_LENGTH;
}

void
PolyLineSegment::SetPoints (Point *points, int n)
{
	SetValue (PolyLineSegment::PointsProperty, Value (points, n));
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight PolyLineSegment.Points only has a setter (no getter), so it's
 * use is only internal.
 */
Point *
PolyLineSegment::GetPoints (int *n)
{
	Value *value = GetValue (PolyLineSegment::PointsProperty);
	
	if (!value) {
		*n = 0;
		return NULL;
	}
	
	PointArray *array = value->AsPointArray ();
	*n = array->basic.count;
	
	return array->points;
}


PolyLineSegment *
poly_line_segment_new (void)
{
	return new PolyLineSegment ();
}

void
poly_line_segment_set_points (PolyLineSegment *segment, Point *points, int n)
{
	segment->SetPoints (points, n);
}



//
// PolyQuadraticBezierSegment
//

DependencyProperty *PolyQuadraticBezierSegment::PointsProperty;


// quadratic to cubic bezier, the original control point and the end control point are the same
// http://web.archive.org/web/20020209100930/http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/node2.html
//
// note: we dont call moon_quad_curve_to to avoid calling moon_get_current_point 
// on each curve (since all but the first one is known)
void
PolyQuadraticBezierSegment::Append (moon_path *path)
{
	Point *points;
	int n = 0;
	
	points = GetPoints (&n);
	
	if (!points || ((n % 2) != 0))
		return;
	
	// origin
	double x0 = 0.0;
	double y0 = 0.0;
	moon_get_current_point (path, &x0, &y0);
	
	// we need at least 2 points
	for (int i = 0; i < n - 1; i+=2) {
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
		
		moon_curve_to (path, x1, y1, x2, y2, x3, y3);
		
		// set new origin
		x0 = x3;
		y0 = y3;
	}
}

int
PolyQuadraticBezierSegment::GetPathSize ()
{
	int n = 0;
	
	GetPoints (&n);
	
	return (n / 2) * MOON_PATH_CURVE_TO_LENGTH;
}

void
PolyQuadraticBezierSegment::SetPoints (Point *points, int n)
{
	SetValue (PolyQuadraticBezierSegment::PointsProperty, Value (points, n));
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight PolyQuadraticBezierSegment.Points only has a setter (no getter), so it's
 * use is only internal.
 */
Point *
PolyQuadraticBezierSegment::GetPoints (int *n)
{
	Value *value = GetValue (PolyQuadraticBezierSegment::PointsProperty);
	
	if (!value) {
		*n = 0;
		return NULL;
	}
	
	PointArray *array = value->AsPointArray ();
	*n = array->basic.count;
	
	return array->points;
}


PolyQuadraticBezierSegment *
poly_quadratic_bezier_segment_new (void)
{
	return new PolyQuadraticBezierSegment ();
}

void
poly_quadratic_bezier_segment_set_points (PolyQuadraticBezierSegment *segment, Point *points, int n)
{
	segment->SetPoints (points, n);
}


//
// QuadraticBezierSegment
//

DependencyProperty *QuadraticBezierSegment::Point1Property;
DependencyProperty *QuadraticBezierSegment::Point2Property;

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

void
QuadraticBezierSegment::SetPoint1 (Point *point)
{
	SetValue (QuadraticBezierSegment::Point1Property, Value (*point));
}

Point *
QuadraticBezierSegment::GetPoint1 ()
{
	Value *value = GetValue (QuadraticBezierSegment::Point1Property);
	
	return value ? value->AsPoint () : NULL;
}

void
QuadraticBezierSegment::SetPoint2 (Point *point)
{
	SetValue (QuadraticBezierSegment::Point2Property, Value (*point));
}

Point *
QuadraticBezierSegment::GetPoint2 ()
{
	Value *value = GetValue (QuadraticBezierSegment::Point2Property);
	
	return value ? value->AsPoint () : NULL;
}


QuadraticBezierSegment *
quadratic_bezier_segment_new (void)
{
	return new QuadraticBezierSegment ();
}

Point *
quadratic_bezier_segment_get_point1 (QuadraticBezierSegment *segment)
{
	return segment->GetPoint1 ();
}

void
quadratic_bezier_segment_set_point1 (QuadraticBezierSegment *segment, Point *point)
{
	segment->SetPoint1 (point);
}

Point *
quadratic_bezier_segment_get_point2 (QuadraticBezierSegment *segment)
{
	return segment->GetPoint2 ();
}

void
quadratic_bezier_segment_set_point2 (QuadraticBezierSegment *segment, Point *point)
{
	segment->SetPoint2 (point);
}



//
// Type Initialization 
//

void
geometry_init (void)
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
