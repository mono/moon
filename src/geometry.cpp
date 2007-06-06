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

//
// Geometry
//

DependencyProperty* Geometry::FillRuleProperty;
DependencyProperty* Geometry::TransformProperty;

FillRule
geometry_get_fill_rule (Geometry *geometry)
{
	return (FillRule) geometry->GetValue (Geometry::FillRuleProperty)->u.i32;
}

void
geometry_set_fill_rule (Geometry *geometry, FillRule fill_rule)
{
	geometry->SetValue (Geometry::FillRuleProperty, Value (fill_rule));
}

Transform*
geometry_get_transform (Geometry *geometry)
{
	return (Transform*) geometry->GetValue (Geometry::TransformProperty)->u.dependency_object;
}

void
geometry_set_transform (Geometry *geometry, Transform *transform)
{
	geometry->SetValue (Geometry::TransformProperty, Value (transform));
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
	return (value ? value->u.point : NULL);
}

void
ellipse_geometry_set_center (EllipseGeometry *ellipse_geometry, Point *point)
{
	ellipse_geometry->SetValue (EllipseGeometry::CenterProperty, Value (point));
}

double
ellipse_geometry_get_radius_x (EllipseGeometry *ellipse_geometry)
{
	return ellipse_geometry->GetValue (EllipseGeometry::RadiusXProperty)->u.d;
}

void
ellipse_geometry_set_radius_x (EllipseGeometry *ellipse_geometry, double radius_x)
{
	ellipse_geometry->SetValue (EllipseGeometry::RadiusXProperty, Value (radius_x));
}

double
ellipse_geometry_get_radius_y (EllipseGeometry *ellipse_geometry)
{
	return ellipse_geometry->GetValue (EllipseGeometry::RadiusYProperty)->u.d;
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

//
// LineGeometry
//

DependencyProperty* LineGeometry::EndPointProperty;
DependencyProperty* LineGeometry::StartPointProperty;

Point*
line_geometry_get_end_point (LineGeometry* line_geometry)
{
	Value *value = line_geometry->GetValue (LineGeometry::EndPointProperty);
	return (value ? value->u.point : NULL);
}

void
line_geometry_set_end_point (LineGeometry* line_geometry, Point *end_point)
{
	line_geometry->SetValue (LineGeometry::EndPointProperty, Value (end_point));
}

Point*
line_geometry_get_start_point (LineGeometry* line_geometry)
{
	Value *value = line_geometry->GetValue (LineGeometry::StartPointProperty);
	return (value ? value->u.point : NULL);
}

void
line_geometry_set_start_point (LineGeometry* line_geometry, Point *start_point)
{
	line_geometry->SetValue (LineGeometry::StartPointProperty, Value (start_point));
}

LineGeometry*
line_geometry_new ()
{
	return new LineGeometry ();
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

//
// RectangleGeometry
//

DependencyProperty* RectangleGeometry::RadiusXProperty;
DependencyProperty* RectangleGeometry::RadiusYProperty;
DependencyProperty* RectangleGeometry::RectProperty;

double
rectangle_geometry_get_radius_x (RectangleGeometry *rectangle_geometry)
{
	return rectangle_geometry->GetValue (RectangleGeometry::RadiusXProperty)->u.d;
}

void
rectangle_geometry_set_radius_x (RectangleGeometry *rectangle_geometry, double radius_x)
{
	rectangle_geometry->SetValue (RectangleGeometry::RadiusXProperty, Value (radius_x));
}

double
rectangle_geometry_get_radius_y (RectangleGeometry *rectangle_geometry)
{
	return rectangle_geometry->GetValue (RectangleGeometry::RadiusYProperty)->u.d;
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
	return (value ? value->u.rect : NULL);
}

void
rectangle_geometry_set_rect (RectangleGeometry *rectangle_geometry, Rect *rect)
{
	rectangle_geometry->SetValue (RectangleGeometry::RectProperty, Value (rect));
}

RectangleGeometry*
rectangle_geometry_new ()
{
	return new RectangleGeometry ();
}

//
// 
//

void
geometry_init ()
{
	/* Geometry fields */
	Geometry::FillRuleProperty = DependencyObject::Register (DependencyObject::GEOMETRY, "FillRule", new Value (FillRuleEvenOdd));
	Geometry::TransformProperty = DependencyObject::Register (DependencyObject::GEOMETRY, "Transform", Value::DEPENDENCY_OBJECT);

	/* GeometryGroup fields */
	GeometryGroup::ChildrenProperty = DependencyObject::Register (DependencyObject::GEOMETRYGROUP, "Children", Value::DEPENDENCY_OBJECT);

	/* EllipseGeometry fields */
	EllipseGeometry::CenterProperty = DependencyObject::Register (DependencyObject::ELLIPSEGEOMETRY, "Center", Value::POINT);
	EllipseGeometry::RadiusXProperty = DependencyObject::Register (DependencyObject::ELLIPSEGEOMETRY, "RadiusX", new Value (0.0));
	EllipseGeometry::RadiusYProperty = DependencyObject::Register (DependencyObject::ELLIPSEGEOMETRY, "RadiusY", new Value (0.0));

	/* LineGeometry fields */
	LineGeometry::EndPointProperty = DependencyObject::Register (DependencyObject::LINEGEOMETRY, "EndPoint", Value::POINT);
	LineGeometry::StartPointProperty = DependencyObject::Register (DependencyObject::LINEGEOMETRY, "StartPoint", Value::POINT);

	/* PathGeometry */
	PathGeometry::FiguresProperty = DependencyObject::Register (DependencyObject::PATHGEOMETRY, "Figures", Value::DEPENDENCY_OBJECT);

	/* RectangleGeometry fields */
	RectangleGeometry::RadiusXProperty = DependencyObject::Register (DependencyObject::RECTANGLEGEOMETRY, "RadiusX", new Value (0.0));
	RectangleGeometry::RadiusYProperty = DependencyObject::Register (DependencyObject::RECTANGLEGEOMETRY, "RadiusY", new Value (0.0));
	RectangleGeometry::RectProperty = DependencyObject::Register (DependencyObject::RECTANGLEGEOMETRY, "Rect", Value::RECT);
}
