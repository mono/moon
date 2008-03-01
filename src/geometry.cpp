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
#include <math.h>

#include "runtime.h"
#include "geometry.h"
#include "rect.h"
#include "shape.h"
#include "array.h"

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

Geometry::~Geometry ()
{
	if (path)
		moon_path_destroy (path);
}

void
Geometry::Draw (Path *shape, cairo_t *cr)
{
	cairo_save (cr);
	cairo_set_fill_rule (cr, convert_fill_rule (geometry_get_fill_rule (this)));
	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}

	if (!path || (path->cairo.num_data == 0))
		Build (shape);

	if (path)
		cairo_append_path (cr, &path->cairo);

	cairo_restore (cr);
}

/*
Rect
Geometry::ComputeBounds (Path *shape)
{
	if (!path || (path->cairo.num_data == 0))
		Build (shape);

	return path ? path_get_bounds (shape, &path->cairo) : Rect (0, 0, 0, 0);
}
*/

void
Geometry::OnPropertyChanged (DependencyProperty *prop)
{
	// no need to clear the path for Geometry itself as FillRule and Transform properties are 
	// only used when drawing, i.e. they do not affect the path itself
	if ((prop->type != Type::GEOMETRY) && path)
		moon_path_clear (path);

	NotifyAttachersOfPropertyChange (prop);
}

static Rect
path_get_bounds (Path *shape, cairo_path_t *path)
{
	if (!path)
		return Rect (0.0, 0.0, 0.0, 0.0);

	double thickness = shape && shape_get_stroke (shape) ? shape_get_stroke_thickness (shape) : 0;
	
	cairo_t *cr = measuring_context_create ();
	cairo_set_line_width (cr, thickness);
	cairo_append_path (cr, path);
	
	double x1, y1, x2, y2;
	
	if (thickness > 0.0)
		cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	else
		cairo_fill_extents (cr, &x1, &y1, &x2, &y2);

	measuring_context_destroy (cr);

	return Rect (MIN (x1, x2), MIN (y1, y2), fabs (x2 - x1), fabs (y2 - y1));
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
	this->SetValue (GeometryGroup::ChildrenProperty, Value::CreateUnref (new GeometryCollection ()));
}

void
GeometryGroup::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop)
{
	NotifyAttachersOfPropertyChange (prop);
}

void
GeometryGroup::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	// GeometryGroup only has one collection, so let's save the hash lookup
	//if (col == GetValue (GeometryGroup::ChildrenProperty)->AsGeometryCollection())
		NotifyAttachersOfPropertyChange (GeometryGroup::ChildrenProperty);
}

void
GeometryGroup::Draw (Path *shape, cairo_t *cr)
{
	cairo_save (cr);
	cairo_set_fill_rule (cr, convert_fill_rule (geometry_get_fill_rule (this)));
	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}

	GeometryCollection *children = geometry_group_get_children (this);
	Collection::Node *node;
	
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Geometry *geometry = (Geometry *) node->obj;
		
		geometry->Draw  (shape, cr);
	}
	cairo_restore (cr);
}

Rect
GeometryGroup::ComputeBounds (Path *path)
{
	Rect bounds = Rect (0.0, 0.0, 0.0, 0.0);
	GeometryCollection *children = geometry_group_get_children (this);
	Collection::Node *node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Geometry *geometry = (Geometry *) node->obj;
		bounds = bounds.Union (geometry->ComputeBounds (path));
	}

	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}

//g_warning ("GeometryGroup::ComputeBounds - x %g y %g w %g h %g", bounds.x, bounds.y, bounds.w, bounds.h);
	return bounds;
}

#if FALSE
Point
Geometry::GetOriginPoint (Path *shape)
{
	double x = 0.0;
	double y = 0.0;

	if (!path || (path->cairo.num_data == 0))
		Build (shape);

	moon_get_origin (path, &x, &y);
	return Point (x, y);
}
#endif

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
EllipseGeometry::Build (Path *shape)
{
	double rx = ellipse_geometry_get_radius_x (this);
	double ry = ellipse_geometry_get_radius_y (this);
	Point *pt = ellipse_geometry_get_center (this);
	double x = pt ? pt->x : 0.0;
	double y = pt ? pt->y : 0.0;

	path = moon_path_renew (path, MOON_PATH_ELLIPSE_LENGTH);
	moon_ellipse (path, x - rx, y - ry, rx * 2.0, ry * 2.0);
}

Rect
EllipseGeometry::ComputeBounds (Path *path)
{
	// code written to minimize divisions
	double ht = (path ? shape_get_stroke_thickness (path) : 1.0) / 2.0;
	double hw = ellipse_geometry_get_radius_x (this) + ht;
	double hh = ellipse_geometry_get_radius_y (this) + ht;
	// point is at center, so left-top corner is minus half width / half height
	Point *pt = ellipse_geometry_get_center (this);
	double x = pt ? pt->x : 0.0;
	double y = pt ? pt->y : 0.0;
	Rect bounds;

	bounds = Rect (x - hw, y - hh, hw * 2.0, hh * 2.0);

	Transform* transform = geometry_get_transform (this);
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
LineGeometry::Build (Path *shape)
{
	Point *p1 = line_geometry_get_start_point (this);
	Point *p2 = line_geometry_get_end_point (this);

	path = moon_path_renew (path, MOON_PATH_MOVE_TO_LENGTH + MOON_PATH_LINE_TO_LENGTH);
	moon_move_to (path, p1->x, p1->y);
	moon_line_to (path, p2->x, p2->y);
}

Rect
LineGeometry::ComputeBounds (Path *shape)
{
	Rect bounds;
	Point *p1 = line_geometry_get_start_point (this);
	Point *p2 = line_geometry_get_end_point (this);
	double thickness = shape_get_stroke_thickness (shape);

	calc_line_bounds (p1->x, p2->x, p1->y, p2->y, thickness, &bounds);

	Transform* transform = geometry_get_transform (this);
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

DependencyProperty* PathGeometry::FiguresProperty;

PathGeometry*
path_geometry_new ()
{
	return new PathGeometry ();
}

void
PathGeometry::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	// PathGeometry only has one collection, so let's save the hash lookup
	//if (col == GetValue (PathGeometry::FiguresProperty)->AsPathFigureCollection ())
		NotifyAttachersOfPropertyChange (PathGeometry::FiguresProperty);
}

void
PathGeometry::Build (Path *shape)
{
	Value *v = GetValue (PathGeometry::FiguresProperty);
	if (!v)
		return;

	path = moon_path_renew (path, 0);

	PathFigureCollection *children = v->AsPathFigureCollection();
	Collection::Node *node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathFigure *pf = (PathFigure *) node->obj;
		if (!pf->IsBuilt ())
			pf->Build (shape);
		moon_merge (path, pf->path);
	}
}

Rect
PathGeometry::ComputeBounds (Path *shape)
{
	Rect bounds = Rect (0.0, 0.0, 0.0, 0.0);
	Value *v = GetValue (PathGeometry::FiguresProperty);
	if (!v)
		return bounds;

	PathFigureCollection *children = v->AsPathFigureCollection();
	Collection::Node *node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathFigure *pf = (PathFigure *) node->obj;
		bounds = bounds.Union (pf->ComputeBounds (shape));
	}
	
	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		bounds = bounds.Transform (&matrix);
	}

//g_warning ("PathGeometry::ComputeBounds - x %g y %g w %g h %g", bounds.x, bounds.y, bounds.w, bounds.h);
	// some AA glitches occurs when no stroke is present or when drawning unfilled curves
	// (e.g. arcs) adding 1.0 will cover the extra pixels used by Cairo's AA
	return bounds;
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
RectangleGeometry::Build (Path *shape)
{
	Rect *rect = rectangle_geometry_get_rect (this);
	if (!rect)
		return;

	double half_thick = 0.0;
	// shape is optional (e.g. not available for clipping)
	if (shape) {
		double thick = shape_get_stroke_thickness (shape);
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
RectangleGeometry::ComputeBounds (Path *path)
{
	Rect *rect = rectangle_geometry_get_rect (this);
	Rect bounds;

	if (!rect)
		return Rect (0.0, 0.0, 0.0, 0.0);

	double thickness = shape_get_stroke_thickness (path);
	// UIElement::SHAPE_DEGENERATE flags may be unset at this stage
	if ((thickness > rect->w) || (thickness > rect->h))
		thickness += 2.0;

	bounds = rect->GrowBy (thickness / 2.0);

	Transform* transform = geometry_get_transform (this);
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
	path = NULL;
	this->SetValue (PathFigure::SegmentsProperty, Value::CreateUnref (new PathSegmentCollection ()));
}

PathFigure::~PathFigure ()
{
	if (path)
		moon_path_destroy (path);
}

void
PathFigure::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::PATHFIGURE) {
		DependencyObject::OnPropertyChanged (prop);
		return;
	}

	if (path)
		moon_path_clear (path);
	NotifyAttachersOfPropertyChange (prop);
}

void
PathFigure::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	if (path)
		moon_path_clear (path);
	// PathFigure only has one collection, so let's save the hash lookup
	//if (col == GetValue (PathFigure::SegmentsProperty)->AsPathSegmentCollection())
		NotifyAttachersOfPropertyChange (PathFigure::SegmentsProperty);
}

void
PathFigure::Build (Path *shape)
{
	PathSegmentCollection *children = GetValue (PathFigure::SegmentsProperty)->AsPathSegmentCollection ();
	Collection::Node *node;

	int path_size = MOON_PATH_MOVE_TO_LENGTH;
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathSegment *ps = (PathSegment *) node->obj;
		path_size += ps->GetSize ();
	}
	bool close = path_figure_get_is_closed (this);
	if (close)
		path_size += MOON_PATH_CLOSE_PATH_LENGTH;

	path = moon_path_renew (path, path_size);

	Point *start = path_figure_get_start_point (this);
	moon_move_to (path, start->x, start->y);
	
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathSegment *ps = (PathSegment *) node->obj;
		ps->Append (path);
	}
	
	if (close)
		moon_close_path (path);
}

Rect
PathFigure::ComputeBounds (Path *shape)
{
	if (!path || (path->cairo.num_data == 0))
		Build (shape);

	return path_get_bounds (shape, &path->cairo);
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

// is it true only for arcs or for everything ? if so using the same values ?
#define IS_ZERO(x)	(fabs(x) < 0.000019)
#define IS_TOO_SMALL(x)	(fabs(x) < 0.000117)

void
ArcSegment::Append (moon_path *path)
{
	// from tests it seems that Silverlight closely follows SVG arc 
	// behavior (which is very different from the model used with GDI+)
	// some helpful stuff is available here:
	// http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes

	// get start point from the existing path
	double sx, sy;
	moon_get_current_point (path, &sx, &sy);
	// end point
	Point* ep = arc_segment_get_point (this);

	// if start and end points are identical, then no arc is drawn
	// FIXME: what's the logic (if any) to compare points
	// e.g. 60 and 60.000002 are drawn while 80 and 80.000003 aren't
	if (IS_ZERO (ep->x - sx) && IS_ZERO (ep->y - sy))
		return;

	Point *size = arc_segment_get_size (this);
	// Correction of out-of-range radii, see F6.6 (step 1)
	if (IS_ZERO (size->x) || IS_ZERO (size->y)) {
		// treat this as a straight line (to end point)
		moon_line_to (path, ep->x, ep->y);
		return;
	}

	// Silverlight "too small to be useful"
	if (IS_TOO_SMALL (size->x) || IS_TOO_SMALL (size->y)) {
		// yes it does mean there's a hole between "normal" FP values and "zero" FP values
		// and SL doesn't render anything in this twilight sonze
		return;
	}

	// Correction of out-of-range radii, see F6.6.1 (step 2)
	double rx = fabs (size->x);
	double ry = fabs (size->y);

	// convert angle into radians
	double angle = arc_segment_get_rotation_angle (this) * M_PI / 180.0;

	// variables required for F6.3.1
	double cos_phi = cos (angle);
	double sin_phi = sin (angle);
	double dx2 = (sx - ep->x) / 2.0;
	double dy2 = (sy - ep->y) / 2.0;
	double x1p = cos_phi * dx2 + sin_phi * dy2;
	double y1p = cos_phi * dy2 - sin_phi * dx2;
	double x1p2 = x1p * x1p;
	double y1p2 = y1p * y1p;
	double rx2 = rx * rx;
	double ry2 = ry * ry;

	// Correction of out-of-range radii, see F6.6.2 (step 4)
	double lambda = (x1p2 / rx2) + (y1p2 / ry2);
	if (lambda > 1.0) {
		// see F6.6.3
		double lambda_root = sqrt (lambda);
		rx *= lambda_root;
		ry *= lambda_root;
		// update rx2 and ry2
		rx2 = rx * rx;
		ry2 = ry * ry;
	}

	bool sweep = arc_segment_get_sweep_direction (this);
	double cxp, cyp, cx, cy;
	double c = (rx2 * ry2) - (rx2 * y1p2) - (ry2 * x1p2);

	// check if there is no possible solution (i.e. we can't do a square root of a negative value)
	if (c < 0.0) {
		// scale uniformly until we have a single solution (see F6.2) i.e. when c == 0.0
		double scale = sqrt (1.0 - c / (rx2 * ry2));
		rx *= scale;
		ry *= scale;
		// update rx2 and ry2
		rx2 = rx * rx;
		ry2 = ry * ry;

		// step 2 (F6.5.2) - simplified since c == 0.0
		cxp = 0.0;
		cyp = 0.0;

		// step 3 (F6.5.3 first part) - simplified since cxp and cyp == 0.0
		cx = 0.0;
		cy = 0.0;
	} else {
		// complete c calculation
		c = sqrt (c / ((rx2 * y1p2) + (ry2 * x1p2)));

		// inverse sign if Fa == Fs
		if (arc_segment_get_is_large_arc (this) == sweep)
			c = -c;

		// step 2 (F6.5.2)
		cxp = c * ( rx * y1p / ry);
		cyp = c * (-ry * x1p / rx);

		// step 3 (F6.5.3 first part)
		cx = cos_phi * cxp - sin_phi * cyp;
		cy = sin_phi * cxp + cos_phi * cyp;
	}

	// step 3 (F6.5.3 second part) we now have the center point of the ellipse
	cx += (sx + ep->x) / 2.0;
	cy += (sy + ep->y) / 2.0;

	// step 4 (F6.5.4)
	// we dont' use arccos (as per w3c doc), see http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm
	// note: atan2 (0.0, 1.0) == 0.0
	double at = atan2 (((y1p - cyp) / ry), ((x1p - cxp) / rx));
	double theta1 = (at < 0.0) ? 2.0 * M_PI + at : at;

	double nat = atan2 (((-y1p - cyp) / ry), ((-x1p - cxp) / rx));
	double delta_theta = (nat < at) ? 2.0 * M_PI - at + nat : nat - at;

	if (sweep) {
		// ensure delta theta < 0 or else add 360 degrees
		if (delta_theta < 0.0)
			delta_theta += 2.0 * M_PI;
	} else {
		// ensure delta theta > 0 or else substract 360 degrees
		if (delta_theta > 0.0)
			delta_theta -= 2.0 * M_PI;
	}

	// add several cubic bezier to approximate the arc (smaller than 90 degrees)
	// we add one extra segment because we want something smaller than 90deg (i.e. not 90 itself)
	int segments = (int) (fabs (delta_theta / M_PI_2)) + 1;
	double delta = delta_theta / segments;

	// http://www.stillhq.com/ctpfaq/2001/comp.text.pdf-faq-2001-04.txt (section 2.13)
	double bcp = 4.0 / 3 * (1 - cos (delta / 2)) / sin (delta / 2);

	double cos_phi_rx = cos_phi * rx;
	double cos_phi_ry = cos_phi * ry;
	double sin_phi_rx = sin_phi * rx;
	double sin_phi_ry = sin_phi * ry;

	double cos_theta1 = cos (theta1);
	double sin_theta1 = sin (theta1);

	for (int i = 0; i < segments; ++i) {
		// end angle (for this segment) = current + delta
		double theta2 = theta1 + delta;
		double cos_theta2 = cos (theta2);
		double sin_theta2 = sin (theta2);

		// first control point (based on start point sx,sy)
		double c1x = sx - bcp * (cos_phi_rx * sin_theta1 + sin_phi_ry * cos_theta1);
		double c1y = sy + bcp * (cos_phi_ry * cos_theta1 - sin_phi_rx * sin_theta1);

		// end point (for this segment)
		double ex = cx + (cos_phi_rx * cos_theta2 - sin_phi_ry * sin_theta2);
		double ey = cy + (sin_phi_rx * cos_theta2 + cos_phi_ry * sin_theta2);

		// second control point (based on end point ex,ey)
		double c2x = ex + bcp * (cos_phi_rx * sin_theta2 + sin_phi_ry * cos_theta2);
		double c2y = ey + bcp * (sin_phi_rx * sin_theta2 - cos_phi_ry * cos_theta2);

		moon_curve_to (path, c1x, c1y, c2x, c2y, ex, ey);

		// next start point is the current end point (same for angle)
		sx = ex;
		sy = ey;
		theta1 = theta2;
		// avoid recomputations
		cos_theta1 = cos_theta2;
		sin_theta1 = sin_theta2;
	}
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
BezierSegment::Append (moon_path *path)
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

	moon_curve_to (path, x1, y1, x2, y2, x3, y3);
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
LineSegment::Append (moon_path *path)
{
	Point *p = line_segment_get_point (this);

	double x = p ? p->x : 0.0;
	double y = p ? p->y : 0.0;

	moon_line_to (path, x, y);
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
PolyBezierSegment::Append (moon_path *path)
{
	int count = 0;
	Point* points = poly_bezier_segment_get_points (this, &count);

	// we need at least 3 points
	if (!points || (count % 3) != 0)
		return;

	for (int i=0; i < count - 2; i+=3) {
		moon_curve_to (path, points[i].x, points[i].y, points[i+1].x, points[i+1].y,
			points[i+2].x, points[i+2].y);
	}
}

int
PolyBezierSegment::GetSize ()
{
	int count = 0;
	poly_bezier_segment_get_points (this, &count);
	return (count / 3) * MOON_PATH_CURVE_TO_LENGTH;
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
PolyLineSegment::Append (moon_path *path)
{
	int count = 0;
	Point* points = poly_line_segment_get_points (this, &count);

	for (int i=0; i < count; i++) {
		moon_line_to (path, points[i].x, points[i].y);
	}
}

int
PolyLineSegment::GetSize ()
{
	int count = 0;
	poly_line_segment_get_points (this, &count);
	return count * MOON_PATH_LINE_TO_LENGTH;
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
PolyQuadraticBezierSegment::Append (moon_path *path)
{
	int count = 0;
	Point* points = poly_quadratic_bezier_segment_get_points (this, &count);
	if (!points || ((count % 2) != 0))
		return;

	// origin
	double x0 = 0.0;
	double y0 = 0.0;
	moon_get_current_point (path, &x0, &y0);

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

		moon_curve_to (path, x1, y1, x2, y2, x3, y3);

		// set new origin
		x0 = x3;
		y0 = y3;
	}
}

int
PolyQuadraticBezierSegment::GetSize ()
{
	int count = 0;
	poly_quadratic_bezier_segment_get_points (this, &count);
	return (count / 2) * MOON_PATH_CURVE_TO_LENGTH;
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
QuadraticBezierSegment::Append (moon_path *path)
{
	Point *p1 = quadratic_bezier_segment_get_point1 (this);
	Point *p2 = quadratic_bezier_segment_get_point2 (this);

	// quadratic to cubic bezier, the original control point and the end control point are the same
	// http://web.archive.org/web/20020209100930/http://www.icce.rug.nl/erikjan/bluefuzz/beziers/beziers/node2.html
	double x0 = 0.0;
	double y0 = 0.0;
	moon_get_current_point (path, &x0, &y0);

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

	moon_curve_to (path, x1, y1, x2, y2, x3, y3);
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
