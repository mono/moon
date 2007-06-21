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
	virtual Type::Kind GetObjectType () { return Type::GEOMETRY; };

	virtual void OnPropertyChanged (DependencyProperty *prop);

	virtual void Draw (Surface *s);

	virtual bool CanFill () { return true; };
};
FillRule geometry_get_fill_rule (Geometry *geometry);
void geometry_set_fill_rule (Geometry *geometry, FillRule fill_rule);
Transform* geometry_get_transform (Geometry *geometry);
void geometry_set_transform (Geometry *geometry, Transform *transform);

//
// GeometryCollection
//
class GeometryCollection : public Collection {
 public:
	GeometryCollection () {}
	virtual Type::Kind GetObjectType () { return Type::GEOMETRY_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::GEOMETRY; }

	virtual void OnSubPropertyChanged  (DependencyProperty *prop, DependencyProperty *subprop);
};
GeometryCollection* geometry_collection_new ();

//
// GeometryGroup
//
class GeometryGroup : public Geometry {
 public:
	static DependencyProperty* ChildrenProperty;

	GeometryGroup ();
	~GeometryGroup ();
	virtual Type::Kind GetObjectType () { return Type::GEOMETRYGROUP; };

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);

	virtual void Draw (Surface *s);
};
GeometryGroup		*geometry_group_new		();
GeometryCollection	*geometry_group_get_children	(GeometryGroup *geometry_group);
void			geometry_group_set_children	(GeometryGroup *geometry_group, GeometryCollection* geometry_collection);

//
// EllipseGeometry
//
class EllipseGeometry : public Geometry {
 public:
	static DependencyProperty* CenterProperty;
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	EllipseGeometry () { };
	virtual Type::Kind GetObjectType () { return Type::ELLIPSEGEOMETRY; };

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
	virtual Type::Kind GetObjectType () { return Type::LINEGEOMETRY; };

	virtual void Draw (Surface *s);
};
LineGeometry* line_geometry_new ();
Point* line_geometry_get_end_point (LineGeometry* line_geometry);
void line_geometry_set_end_point (LineGeometry* line_geometry, Point *end_point);
Point* line_geometry_get_start_point (LineGeometry* line_geometry);
void line_geometry_set_start_point (LineGeometry* line_geometry, Point *start_point);

//
// PathFigureCollection
//
class PathFigureCollection : public Collection {
 public:
	PathFigureCollection () {}
	virtual Type::Kind GetObjectType () { return Type::PATHFIGURE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::PATHFIGURE; }

	virtual void OnSubPropertyChanged  (DependencyProperty *prop, DependencyProperty *subprop);
};
PathFigureCollection* path_figure_collection_new ();

//
// PathGeometry
//
class PathGeometry : public Geometry {
 public:
	static DependencyProperty* FiguresProperty;

	PathGeometry ();
	~PathGeometry ();
	virtual Type::Kind GetObjectType () { return Type::PATHGEOMETRY; };

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void Draw (Surface *s);

	// this is an element-by-element decision
	virtual bool CanFill () { return true; }
};
PathGeometry		*path_geometry_new ();
PathFigureCollection	*path_geometry_get_figures	(PathFigureCollection *path_geometry);
void			path_geometry_set_figures	(PathFigureCollection *path_geometry, PathFigureCollection* collection);

//
// RectangleGeometry
//
class RectangleGeometry : public Geometry {
 public:
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;
	static DependencyProperty* RectProperty;

	RectangleGeometry () { };
	virtual Type::Kind GetObjectType () { return Type::RECTANGLEGEOMETRY; };

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
// PathSegmentCollection
//
class PathSegmentCollection : public Collection {
 public:
	PathSegmentCollection () {}
	virtual Type::Kind GetObjectType () { return Type::PATHSEGMENT_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::PATHSEGMENT; }

	virtual void OnSubPropertyChanged  (DependencyProperty *prop, DependencyProperty *subprop);
};
PathSegmentCollection* path_segment_collection_new ();

//
// PathFigure
//
class PathFigure : public DependencyObject {
 public:
	static DependencyProperty* IsClosedProperty;
	static DependencyProperty* IsFilledProperty;
	static DependencyProperty* SegmentsProperty;
	static DependencyProperty* StartPointProperty;

	PathFigure ();
	~PathFigure ();
	virtual Type::Kind GetObjectType () { return Type::PATHFIGURE; };

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void Draw (Surface *s);
};
PathFigure* path_figure_new ();
bool	path_figure_get_is_closed	(PathFigure *path_figure);
void	path_figure_set_is_closed	(PathFigure *path_figure, bool closed);
bool	path_figure_get_is_filled	(PathFigure *path_figure);
void	path_figure_set_is_filled	(PathFigure *path_figure, bool filled);
Point*	path_figure_get_start_point	(PathFigure *path_figure);
void	path_figure_set_start_point	(PathFigure *path_figure, Point *point);
PathSegmentCollection	*path_figure_get_segments	(PathFigure *path_figure);
void			path_figure_set_segments	(PathFigure *path_figure, PathSegmentCollection* collection);

//
// PathSegment
//
class PathSegment : public DependencyObject {
 public:
	virtual void Draw (Surface *s) {}
	virtual Type::Kind GetObjectType () { return Type::PATHSEGMENT; };
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
	virtual Type::Kind GetObjectType () { return Type::ARCSEGMENT; };

	virtual void Draw (Surface *s);
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
	virtual Type::Kind GetObjectType () { return Type::BEZIERSEGMENT; };

	virtual void Draw (Surface *s);
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
	virtual Type::Kind GetObjectType () { return Type::LINESEGMENT; };

	virtual void Draw (Surface *s);
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
	virtual Type::Kind GetObjectType () { return Type::POLYBEZIERSEGMENT; };

	virtual void Draw (Surface *s);
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
	virtual Type::Kind GetObjectType () { return Type::POLYLINESEGMENT; };

	virtual void Draw (Surface *s);
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
	virtual Type::Kind GetObjectType () { return Type::POLYQUADRATICBEZIERSEGMENT; };

	virtual void Draw (Surface *s);
};
PolyQuadraticBezierSegment	*poly_quadratic_bezier_segment_new	();
Point*	poly_quadratic_bezier_segment_get_points	(PolyQuadraticBezierSegment *segment, int *count);
void	poly_quadratic_bezier_segment_set_points	(PolyQuadraticBezierSegment *segment, Point *points, int count);

//
// QuadraticBezierSegment
//

class QuadraticBezierSegment : public PathSegment {
 public:
	static DependencyProperty* Point1Property;
	static DependencyProperty* Point2Property;

	QuadraticBezierSegment () { }
	virtual Type::Kind GetObjectType () { return Type::QUADRATICBEZIERSEGMENT; };

	virtual void Draw (Surface *s);
};
QuadraticBezierSegment	*quadratic_bezier_segment_new	();
Point*	quadratic_bezier_segment_get_point1	(QuadraticBezierSegment *segment);
void	quadratic_bezier_segment_set_point1	(QuadraticBezierSegment *segment, Point *point);
Point*	quadratic_bezier_segment_get_point2	(QuadraticBezierSegment *segment);
void	quadratic_bezier_segment_set_point2	(QuadraticBezierSegment *segment, Point *point);

G_END_DECLS

#endif
