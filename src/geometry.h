/*
 * geometry.h: Geometry classes
 *
 * Author:
 *	Sebastien Pouliot  <sebastien@ximian.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>
#include "runtime.h"
#include "transform.h"

//
// Geometry
//
class Geometry : public DependencyObject {
 public:
	static DependencyProperty* FillRuleProperty;
	static DependencyProperty* TransformProperty;

	Geometry () { };
	Value::Kind GetObjectType () { return Value::GEOMETRY; };

	virtual void Draw (Surface *s);

	virtual bool CanFill () { return true; };
};
FillRule geometry_get_fill_rule (Geometry *geometry);
void geometry_set_fill_rule (Geometry *geometry, FillRule fill_rule);
Transform* geometry_get_transform (Geometry *geometry);
void geometry_set_transform (Geometry *geometry, Transform *transform);

//
// GeometryGroup
//
class GeometryGroup : public Geometry {
 public:
	static DependencyProperty* ChildrenProperty;

	GeometryGroup () { };
	Value::Kind GetObjectType () { return Value::GEOMETRYGROUP; };

	virtual void Draw (Surface *s);
};
GeometryGroup* geometry_group_new ();
// TODO get|set GeometryCollection

//
// EllipseGeometry
//
class EllipseGeometry : public Geometry {
 public:
	static DependencyProperty* CenterProperty;
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	EllipseGeometry () { };
	Value::Kind GetObjectType () { return Value::ELLIPSEGEOMETRY; };

	virtual void Draw (Surface *s);
};
EllipseGeometry* ellipse_geometry_new ();
Point* ellipse_geometry_get_center (EllipseGeometry *ellipse_geometry);
void ellipse_geometry_set_center (EllipseGeometry *ellipse_geometry, Point *point);
double ellipse_geometry_get_radius_x (EllipseGeometry *ellipse_geometry);
void ellipse_geometry_set_radius_x (EllipseGeometry *ellipse_geometry, double radius_x);
double ellipse_geometry_get_radius_y (EllipseGeometry *ellipse_geometry);
void ellipse_geometry_set_radius_y (EllipseGeometry *ellipse_geometry, double radius_y);

//
// LineGeometry
//
class LineGeometry : public Geometry {
 public:
	static DependencyProperty* EndPointProperty;
	static DependencyProperty* StartPointProperty;

	LineGeometry () { };
	Value::Kind GetObjectType () { return Value::LINEGEOMETRY; };

	virtual void Draw (Surface *s);
};
LineGeometry* line_geometry_new ();
Point* line_geometry_get_end_point (LineGeometry* line_geometry);
void line_geometry_set_end_point (LineGeometry* line_geometry, Point *end_point);
Point* line_geometry_get_start_point (LineGeometry* line_geometry);
void line_geometry_set_start_point (LineGeometry* line_geometry, Point *start_point);

//
// PathGeometry
//
class PathGeometry : public Geometry {
 public:
	static DependencyProperty* FiguresProperty;

	PathGeometry () { };
	Value::Kind GetObjectType () { return Value::PATHGEOMETRY; };

	virtual void Draw (Surface *s);

	// this is an element-by-element decision
	virtual bool CanFill () { return false; }
};
PathGeometry* path_geometry_new ();
// TODO get|set PathFigureCollection

//
// RectangleGeometry
//
class RectangleGeometry : public Geometry {
 public:
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;
	static DependencyProperty* RectProperty;

	RectangleGeometry () { };
	Value::Kind GetObjectType () { return Value::RECTANGLEGEOMETRY; };

	virtual void Draw (Surface *s);
};
RectangleGeometry* rectangle_geometry_new ();
double rectangle_geometry_get_radius_x (RectangleGeometry *rectangle_geometry);
void rectangle_geometry_set_radius_x (RectangleGeometry *rectangle_geometry, double radius_x);
double rectangle_geometry_get_radius_y (RectangleGeometry *rectangle_geometry);
void _geometry_set_radius_y (RectangleGeometry *rectangle_geometry, double radius_y);
Rect* rectangle_geometry_get_rect (RectangleGeometry *rectangle_geometry);
void rectangle_geometry_set_rect (RectangleGeometry *rectangle_geometry, Rect *rect);

//
// PathFigure
//
class PathFigure : public DependencyObject {
 public:
	static DependencyProperty* IsClosedProperty;
	static DependencyProperty* IsFilledProperty;
	static DependencyProperty* SegmentsProperty;
	static DependencyProperty* StartPointProperty;

	PathFigure () { }
	Value::Kind GetObjectType () { return Value::PATHFIGURE; };

	virtual void Draw (Surface *s);
};
PathFigure* path_figure_new ();
bool	path_figure_get_is_closed	(PathFigure *path_figure);
void	path_figure_set_is_closed	(PathFigure *path_figure, bool closed);
bool	path_figure_get_is_filled	(PathFigure *path_figure);
void	path_figure_set_is_filled	(PathFigure *path_figure, bool filled);
Point*	path_figure_get_start_point	(PathFigure *path_figure);
void	path_figure_set_start_point	(PathFigure *path_figure, Point *point);

//
// PathSegment
//
class PathSegment : public DependencyObject {
};

//
// ArcSegment
//
class ArcSegment : public PathSegment {
 public:
	static DependencyProperty* IsLargeArcProperty;
	static DependencyProperty* PointProperty;
	static DependencyProperty* RotationAngleProperty;
	static DependencyProperty* SizeProperty;
	static DependencyProperty* SweepDirectionProperty;

	ArcSegment () { }
	Value::Kind GetObjectType () { return Value::ARCSEGMENT; };
};
ArcSegment	*arc_segment_new		();
bool		arc_segment_get_is_large_arc	(ArcSegment *segment);
void		arc_segment_set_is_large_arc	(ArcSegment *segment, bool large);
Point*		arc_segment_get_point		(ArcSegment *segment);
void		arc_segment_set_point		(ArcSegment *segment, Point *point);
double		arc_segment_get_rotation_angle	(ArcSegment *segment);
void		arc_segment_set_rotation_angle	(ArcSegment *segment, double angle);
Point*		arc_segment_get_size		(ArcSegment *segment);
void		arc_segment_set_size		(ArcSegment *segment, Point *size);
SweepDirection	arc_segment_get_sweep_direction	(ArcSegment *segment);
void		arc_segment_set_sweep_direction	(ArcSegment *segment, SweepDirection direction);

//
// BezierSegment
//
class BezierSegment : public PathSegment {
 public:
	static DependencyProperty* Point1Property;
	static DependencyProperty* Point2Property;
	static DependencyProperty* Point3Property;

	BezierSegment () { }
	Value::Kind GetObjectType () { return Value::BEZIERSEGMENT; };
};
BezierSegment	*bezier_segment_new		();
Point*		bezier_segment_get_point1	(BezierSegment *segment);
void		bezier_segment_set_point1	(BezierSegment *segment, Point *point);
Point*		bezier_segment_get_point2	(BezierSegment *segment);
void		bezier_segment_set_point2	(BezierSegment *segment, Point *point);
Point*		bezier_segment_get_point3	(BezierSegment *segment);
void		bezier_segment_set_point3	(BezierSegment *segment, Point *point);

//
// LineSegment
//
class LineSegment : public PathSegment {
 public:
	static DependencyProperty* PointProperty;

	LineSegment () { }
	Value::Kind GetObjectType () { return Value::LINESEGMENT; };
};
LineSegment	*line_segment_new	();
Point*		line_segment_get_point	(LineSegment *segment);
void		line_segment_set_point	(LineSegment *segment, Point *point);

//
// PolyBezierSegment
//

class PolyBezierSegment : public PathSegment {
 public:
	static DependencyProperty* PointsProperty;

	PolyBezierSegment () { }
	Value::Kind GetObjectType () { return Value::POLYBEZIERSEGMENT; };
};
PolyBezierSegment	*poly_bezier_segment_new	();
Point*			poly_bezier_segment_get_points	(PolyBezierSegment *segment, int *count);
void			poly_bezier_segment_set_points	(PolyBezierSegment *segment, Point *points, int count);

//
// PolyLineSegment
//

class PolyLineSegment : public PathSegment {
 public:
	static DependencyProperty* PointsProperty;

	PolyLineSegment () { }
	Value::Kind GetObjectType () { return Value::POLYLINESEGMENT; };
};
PolyLineSegment	*poly_line_segment_new	();
Point*		poly_line_segment_get_points	(PolyLineSegment *segment, int *count);
void		poly_line_segment_set_points	(PolyLineSegment *segment, Point *points, int count);

//
// PolyQuadraticBezierSegment
//

class PolyQuadraticBezierSegment : public PathSegment {
 public:
	static DependencyProperty* PointsProperty;

	PolyQuadraticBezierSegment () { }
	Value::Kind GetObjectType () { return Value::POLYBEZIERSEGMENT; };
};
PolyQuadraticBezierSegment	*poly_quadratic_segment_new	();
Point*	poly_quadratic_segment_get_points	(PolyQuadraticBezierSegment *segment, int *count);
void	poly_quadratic_segment_set_points	(PolyQuadraticBezierSegment *segment, Point *points, int count);

//
// QuadraticBezierSegment
//

class QuadraticBezierSegment : public PathSegment {
 public:
	static DependencyProperty* Point1Property;
	static DependencyProperty* Point2Property;

	QuadraticBezierSegment () { }
	Value::Kind GetObjectType () { return Value::QUADRATICBEZIERSEGMENT; };
};
QuadraticBezierSegment	*quadratic_bezier_segment_new	();
Point*	quadratic_bezier_segment_get_point1	(QuadraticBezierSegment *segment);
void	quadratic_bezier_segment_set_point1	(QuadraticBezierSegment *segment, Point *point);
Point*	quadratic_bezier_segment_get_point2	(QuadraticBezierSegment *segment);
void	quadratic_bezier_segment_set_point2	(QuadraticBezierSegment *segment, Point *point);

G_END_DECLS

#endif
