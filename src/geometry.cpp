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

Geometry::~Geometry ()
{
	if (path)
		moon_path_destroy (path);
}

void
Geometry::Draw (Path *shape, cairo_t *cr)
{
	cairo_set_fill_rule (cr, convert_fill_rule (geometry_get_fill_rule (this)));
	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}

	if (!path || (path->cairo.num_data == 0)) {
		Build (shape);
		// note: shape can be NULL when Geometry is used for clipping
		if (shape)
			StretchAdjust (shape);
	}
	if (path)
		cairo_append_path (cr, &path->cairo);
}

void
Geometry::OnPropertyChanged (DependencyProperty *prop)
{
	// no need to clear the path for Geometry itself as FillRule and Transform properties are 
	// only used when drawing, i.e. they do not affect the path itself
	if ((prop->type != Type::GEOMETRY) && path)
		moon_path_clear (path);

	NotifyAttacheesOfPropertyChange (prop);
}

static void
path_get_extents (cairo_path_t *path, double *min_x, double *min_y, double *max_x, double *max_y)
{
	if (!path)
		return;

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

	if (min_x) *min_x = minx;
	if (min_y) *min_y = miny;
	if (max_x) *max_x = maxx;
	if (max_y) *max_y = maxy;
}

static void
path_stretch_adjust (Path *shape, cairo_path_t *path)
{
	if (!path) {
		shape->SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	double w = framework_element_get_width (shape);
	double h = framework_element_get_height (shape);
	if ((w < 0.0) || (h < 0.0)) {
		shape->SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	Stretch stretch = shape_get_stretch (shape);
	if (stretch == StretchNone)
		return;

	/* NOTE: this looks complex but avoid a *lot* of changes in geometry 
	 * (resulting in something even more complex).
	 */
	double minx, maxx, miny, maxy;
	path_get_extents (path, &minx, &miny, &maxx, &maxy);

	double actual_height = maxy - miny;
	double actual_width = maxx - minx;

	Value *vh = shape->GetValueNoDefault (FrameworkElement::HeightProperty);
	Value *vw = shape->GetValueNoDefault (FrameworkElement::WidthProperty);

	double sh = (vh && ((int)actual_height > 0.0)) ? (h / actual_height) : 1.0;
	double sw = (vw && ((int)actual_width > 0.0)) ? (w / actual_width) : 1.0;
	switch (stretch) {
	case StretchFill:
		break;
	case StretchUniform:
		sw = sh = (sw < sh) ? sw : sh;
		break;
	case StretchUniformToFill:
		sw = sh = (sw > sh) ? sw : sh;
		break;
	case StretchNone:
		/* not reached */
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
		}
	}
}

void
Geometry::StretchAdjust (Path *shape)
{
	path_stretch_adjust (shape, &path->cairo);
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
GeometryGroup::Draw (Path *shape, cairo_t *cr)
{
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

		if (!geometry->IsBuilt ())
			geometry->Build (shape);
		cairo_append_path (cr, geometry->GetCairoPath ());
	}

	// can be NULL for clipping
	if (!shape)
		return;

	Stretch stretch = shape_get_stretch (shape);
	if (stretch != StretchNone) {
		// Group must be processed as a single item
		cairo_path_t* cp = cairo_copy_path (cr);
		path_stretch_adjust (shape, cp);
		cairo_new_path (cr);
		cairo_append_path (cr, cp);
		cairo_path_destroy (cp);
	}
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
//g_warning ("GeometryGroup::ComputeBounds - x %g y %g w %g h %g", bounds.x, bounds.y, bounds.w, bounds.h);
	return bounds;
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
EllipseGeometry::Build (Path *shape)
{
	Point *pt = ellipse_geometry_get_center (this);
	double rx = ellipse_geometry_get_radius_x (this);
	double ry = ellipse_geometry_get_radius_y (this);

	path = moon_path_renew (path, MOON_PATH_ELLIPSE_LENGTH);
	moon_ellipse (path, pt->x - rx, pt->y - ry, rx * 2.0, ry * 2.0);
}

Rect
EllipseGeometry::ComputeBounds (Path *path)
{
	// code written to minimize divisions
	double ht = shape_get_stroke_thickness (path) / 2.0;
	double hw = ellipse_geometry_get_radius_x (this) + ht;
	double hh = ellipse_geometry_get_radius_y (this) + ht;
	// point is at center, so left-top corner is minus half width / half height
	Point *pt = ellipse_geometry_get_center (this);
	return Rect (pt->x - hw, pt->y - hh, hw * 2.0, hh * 2.0);
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
LineGeometry::ComputeBounds (Path *path)
{
	Point *p1 = line_geometry_get_start_point (this);
	Point *p2 = line_geometry_get_end_point (this);
	double thickness = shape_get_stroke_thickness (path);

	if (thickness <= 0.0)
		return Rect (0.0, 0.0, 0.0, 0.0);

	double dx = p1->x - p2->x;
	double dy = p1->y - p2->y;

	if (thickness <= 1.0)
		return Rect (MIN (p1->x, p2->x), MIN (p1->y, p2->y), fabs (dx), fabs (dy));

	thickness /= 2.0;
	// vertical line
	if (dx == 0.0)
		return Rect (p1->x - thickness, MIN (p1->y, p2->y), thickness * 2.0, fabs (dy));
	// horizontal line
	if (dy == 0.0)
		return Rect (MIN (p1->x, p2->x), p1->y - thickness, fabs (dx), thickness * 2.0);

	// slopped line
	double m = fabs (dy / dx);
	double tx = (m > 1.0) ? thickness : thickness * m;
	double ty = (m < 1.0) ? thickness : thickness / m;
	return Rect (MIN (p1->x, p2->x) - tx / 2.0, MIN (p1->y, p2->y) - ty / 2.0, fabs (dx) + tx, fabs (dy) + ty);
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
	this->SetValue (PathGeometry::FiguresProperty, Value::CreateUnref (new PathFigureCollection ()));
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

// FIXME: we should cache the path in the group (i.e. a Build method) to avoid
// rebuilding and reapplying the strech every time
void
PathGeometry::Draw (Path *shape, cairo_t *cr)
{
	cairo_set_fill_rule (cr, convert_fill_rule (geometry_get_fill_rule (this)));
	Transform* transform = geometry_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform->GetTransform (&matrix);
		cairo_transform (cr, &matrix);
	}

	PathFigureCollection *children = GetValue (PathGeometry::FiguresProperty)->AsPathFigureCollection();
	Collection::Node *node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathFigure *pf = (PathFigure *) node->obj;
		if (!pf->IsBuilt ())
			pf->Build (shape);
		cairo_append_path (cr, pf->GetCairoPath ());
	}

	// can be NULL for clipping
	if (!shape)
		return;

	Stretch stretch = shape_get_stretch (shape);
	if (stretch != StretchNone) {
		cairo_path_t* cp = cairo_copy_path (cr);
		path_stretch_adjust (shape, cp);
		cairo_new_path (cr);
		cairo_append_path (cr, cp);
		cairo_path_destroy (cp);
	}
}

Rect
PathGeometry::ComputeBounds (Path *shape)
{
	Rect bounds = Rect (0.0, 0.0, 0.0, 0.0);
	PathFigureCollection *children = GetValue (PathGeometry::FiguresProperty)->AsPathFigureCollection();
	Collection::Node *node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		PathFigure *pf = (PathFigure *) node->obj;
		bounds = bounds.Union (pf->ComputeBounds (shape));
	}

//g_warning ("PathGeometry::ComputeBounds - x %g y %g w %g h %g", bounds.x, bounds.y, bounds.w, bounds.h);
	// some AA glitches occurs when no stroke is present or when drawning unfilled curves
	// (e.g. arcs) adding 1.0 will cover the extra pixels used by Cairo's AA
	return bounds.GrowBy (1.0);
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
	if (!rect)
		return Rect (0.0, 0.0, 0.0, 0.0);

	double thickness = shape_get_stroke_thickness (path);
	// UIElement::SHAPE_DEGENERATE flags may be unset at this stage
	if ((thickness > rect->w) || (thickness > rect->h))
		thickness += 2.0;

	return rect->GrowBy (thickness / 2.0);
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
	path_size = 0;
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

	if (prop == SegmentsProperty){
		PathSegmentCollection *newcol = GetValue (prop)->AsPathSegmentCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	if (path)
		moon_path_clear (path);
	NotifyAttacheesOfPropertyChange (prop);
}

void
PathFigure::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	if (path)
		moon_path_clear (path);
	// PathFigure only has one collection, so let's save the hash lookup
	//if (col == GetValue (PathFigure::SegmentsProperty)->AsPathSegmentCollection())
		NotifyAttacheesOfPropertyChange (PathFigure::SegmentsProperty);
}

void
PathFigure::Build (Path *shape)
{
	PathSegmentCollection *children = GetValue (PathFigure::SegmentsProperty)->AsPathSegmentCollection ();
	Collection::Node *node;

	path_size = MOON_PATH_MOVE_TO_LENGTH;
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

	double minx, miny, maxx, maxy;
	path_get_extents (&path->cairo, &minx, &miny, &maxx, &maxy);

	Rect bounds = Rect (minx, miny, maxx - minx, maxy - miny);
	double thickness = shape_get_stroke_thickness (shape);
	return bounds.GrowBy (thickness / 2.0);
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

void
PathSegment::OnPropertyChanged (DependencyProperty *prop)
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
ArcSegment::Append (moon_path *path)
{
	Point *size = arc_segment_get_size (this);
	double angle = arc_segment_get_rotation_angle (this);
	int large = arc_segment_get_is_large_arc (this) ? 1 : 0;
	int direction = arc_segment_get_sweep_direction (this) == SweepDirectionCounterclockwise ? 0 : 1;
	Point* p = arc_segment_get_point (this);

	// FIXME: there's no cairo_arc_to so we reuse librsvg code (see rsvg.cpp)
	rsvg_arc_to (path, size->x, size->y, angle, large, direction, p->x, p->y); 
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
