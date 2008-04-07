/*
 * geometry.h: Geometry classes
 *
 * Author:
 *	Sebastien Pouliot  <sebastien@ximian.com>
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>
#include "enums.h"
#include "rect.h"
#include "transform.h"
#include "moon-path.h"

//
// Geometry
//
class Geometry : public DependencyObject {
 protected:
	virtual ~Geometry ();

 protected:
	enum GeometryFlags {
		GEOMETRY_NORMAL		= 0x01,	// normal drawing
		GEOMETRY_DEGENERATE	= 0x02,	// degenerate drawing, use the Stroke brush for filling
		GEOMETRY_NEEDS_FILL	= 0x04,	// filling, if specified, is needed (e.g. LineGeometry doesn't need it)
		GEOMETRY_NEEDS_CAPS	= 0x08,	// Stroke[Start|End]LineCap
		GEOMETRY_NEEDS_JOIN	= 0x10,	// StrokeLineJoin, StrokeMiterLimit
		GEOMETRY_MASK		= GEOMETRY_NORMAL | GEOMETRY_DEGENERATE | GEOMETRY_NEEDS_FILL | GEOMETRY_NEEDS_CAPS | GEOMETRY_NEEDS_JOIN
	};
	bool IsDegenerate () { return (flags & Geometry::GEOMETRY_DEGENERATE); };
	void SetGeometryFlags (GeometryFlags sf) { flags &= ~Geometry::GEOMETRY_MASK; flags |= sf; };

	int flags;
	moon_path *path;
 public:
	static DependencyProperty* FillRuleProperty;
	static DependencyProperty* TransformProperty;

	Geometry () : flags (GEOMETRY_NORMAL), path (NULL) {};
	virtual Type::Kind GetObjectType () { return Type::GEOMETRY; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual void Draw (Path *path, cairo_t *cr);
	virtual Rect ComputeBounds (Path *path, bool logical) { return Rect (0.0, 0.0, 0.0, 0.0); };

//	virtual Point GetOriginPoint (Path *path);

	virtual bool IsFilled () { return true; };

	virtual void Build (Path *path) {}
	virtual bool IsBuilt () { return path && path->cairo.num_data != 0; }
	virtual cairo_path_t* GetCairoPath () { return (path) ? &path->cairo : NULL; }
};
FillRule geometry_get_fill_rule (Geometry *geometry);
void geometry_set_fill_rule (Geometry *geometry, FillRule fill_rule);
Transform* geometry_get_transform (Geometry *geometry);
void geometry_set_transform (Geometry *geometry, Transform *transform);

//
// GeometryCollection
//
class GeometryCollection : public Collection {
 protected:
	virtual ~GeometryCollection () {}

 public:
	GeometryCollection () {}
	virtual Type::Kind GetObjectType () { return Type::GEOMETRY_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::GEOMETRY; }
};
GeometryCollection* geometry_collection_new ();

//
// GeometryGroup
//
/* @ContentProperty="Children" */
class GeometryGroup : public Geometry {
 protected:
	virtual ~GeometryGroup () {}

 public:
	static DependencyProperty* ChildrenProperty;

	GeometryGroup ();
	virtual Type::Kind GetObjectType () { return Type::GEOMETRYGROUP; };

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subprop_args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);

	virtual void Draw (Path *path, cairo_t *cr);
	virtual Rect ComputeBounds (Path *path, bool logical);
};
GeometryGroup		*geometry_group_new		();
GeometryCollection	*geometry_group_get_children	(GeometryGroup *geometry_group);
void			geometry_group_set_children	(GeometryGroup *geometry_group, GeometryCollection* geometry_collection);

//
// EllipseGeometry
//
class EllipseGeometry : public Geometry {
 protected:
	virtual ~EllipseGeometry () {}

 protected:
	virtual void Build (Path *path);
 public:
	static DependencyProperty* CenterProperty;
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	EllipseGeometry () { };
	virtual Type::Kind GetObjectType () { return Type::ELLIPSEGEOMETRY; };

	virtual Rect ComputeBounds (Path *path, bool logical);
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
 protected:
	virtual ~LineGeometry () {}

 protected:
	virtual void Build (Path *path);
 public:
	static DependencyProperty* EndPointProperty;
	static DependencyProperty* StartPointProperty;

	LineGeometry () { };
	virtual Type::Kind GetObjectType () { return Type::LINEGEOMETRY; };

	virtual Rect ComputeBounds (Path *path, bool logical);
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
 protected:
	virtual ~PathFigureCollection () {}

 public:
	PathFigureCollection () {}
	virtual Type::Kind GetObjectType () { return Type::PATHFIGURE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::PATHFIGURE; }
};
PathFigureCollection* path_figure_collection_new ();

//
// PathGeometry
//
/* @ContentProperty="Figures" */
class PathGeometry : public Geometry {
 protected:
	virtual ~PathGeometry () {}
	virtual void Build (Path *path);

 public:
	static DependencyProperty* FiguresProperty;

	PathGeometry () {}
	virtual Type::Kind GetObjectType () { return Type::PATHGEOMETRY; };

	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);
	virtual Rect ComputeBounds (Path *path, bool logical);

	// this is an element-by-element decision
	virtual bool IsFilled () { return true; }
};
PathGeometry		*path_geometry_new ();
PathFigureCollection	*path_geometry_get_figures	(PathFigureCollection *path_geometry);
void			path_geometry_set_figures	(PathFigureCollection *path_geometry, PathFigureCollection* collection);

//
// RectangleGeometry
//
class RectangleGeometry : public Geometry {
 protected:
	virtual ~RectangleGeometry () {}
	virtual void Build (Path *path);

 public:
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;
	static DependencyProperty* RectProperty;

	RectangleGeometry () { };
	virtual Type::Kind GetObjectType () { return Type::RECTANGLEGEOMETRY; };

	virtual Rect ComputeBounds (Path *path, bool logical);

	bool GetRadius (double *rx, double *ry);
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
 protected:
	virtual ~PathSegmentCollection () {}

 public:
	PathSegmentCollection () {}
	virtual Type::Kind GetObjectType () { return Type::PATHSEGMENT_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::PATHSEGMENT; }
};
PathSegmentCollection* path_segment_collection_new ();

//
// PathFigure
//
/* @ContentProperty="Segments" */
class PathFigure : public DependencyObject {
 protected:
	virtual ~PathFigure ();

 public:
	static DependencyProperty* IsClosedProperty;
	static DependencyProperty* SegmentsProperty;
	static DependencyProperty* StartPointProperty;

	moon_path *path;

	PathFigure ();
	virtual Type::Kind GetObjectType () { return Type::PATHFIGURE; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);
	virtual void Build (Path *shape);

	virtual bool IsBuilt () { return path && path->cairo.num_data != 0; }
	virtual cairo_path_t* GetCairoPath () { return (path) ? &path->cairo : NULL; }

	Rect ComputeBounds (Path *shape, bool logical);
};
PathFigure* path_figure_new ();
bool	path_figure_get_is_closed	(PathFigure *path_figure);
void	path_figure_set_is_closed	(PathFigure *path_figure, bool closed);
Point*	path_figure_get_start_point	(PathFigure *path_figure);
void	path_figure_set_start_point	(PathFigure *path_figure, Point *point);
PathSegmentCollection	*path_figure_get_segments	(PathFigure *path_figure);
void			path_figure_set_segments	(PathFigure *path_figure, PathSegmentCollection* collection);

//
// PathSegment
//
class PathSegment : public DependencyObject {
 protected:
	virtual ~PathSegment () {}

	virtual void Build (Path *path) {};
 public:
	virtual Type::Kind GetObjectType () { return Type::PATHSEGMENT; }

	virtual void Append (moon_path *path) {}
	virtual int GetSize () { return 0; }
};

//
// ArcSegment
//
class ArcSegment : public PathSegment {
 protected:
	virtual ~ArcSegment () {}

 public:
	static DependencyProperty* IsLargeArcProperty;
	static DependencyProperty* PointProperty;
	static DependencyProperty* RotationAngleProperty;
	static DependencyProperty* SizeProperty;
	static DependencyProperty* SweepDirectionProperty;

	ArcSegment () { }
	virtual Type::Kind GetObjectType () { return Type::ARCSEGMENT; };
	virtual int GetSize () { return 4 * MOON_PATH_CURVE_TO_LENGTH; } // non-optimal size, depends on angle

	virtual void Append (moon_path *path);
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
 protected:
	virtual ~BezierSegment () {}

 public:
	static DependencyProperty* Point1Property;
	static DependencyProperty* Point2Property;
	static DependencyProperty* Point3Property;

	BezierSegment () { }
	virtual Type::Kind GetObjectType () { return Type::BEZIERSEGMENT; };
	virtual int GetSize () { return MOON_PATH_CURVE_TO_LENGTH; }

	virtual void Append (moon_path *path);
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
 protected:
	virtual ~LineSegment () {}

 public:
	static DependencyProperty* PointProperty;

	LineSegment () { }
	virtual Type::Kind GetObjectType () { return Type::LINESEGMENT; };
	virtual int GetSize () { return MOON_PATH_LINE_TO_LENGTH; }

	virtual void Append (moon_path *path);
};
LineSegment	*line_segment_new	();
Point*		line_segment_get_point	(LineSegment *segment);
void		line_segment_set_point	(LineSegment *segment, Point *point);

//
// PolyBezierSegment
//

class PolyBezierSegment : public PathSegment {
 protected:
	virtual ~PolyBezierSegment () {}

 public:
	static DependencyProperty* PointsProperty;

	PolyBezierSegment () { }
	virtual Type::Kind GetObjectType () { return Type::POLYBEZIERSEGMENT; };
	virtual int GetSize ();

	virtual void Append (moon_path *path);
};
PolyBezierSegment	*poly_bezier_segment_new	();
Point*			poly_bezier_segment_get_points	(PolyBezierSegment *segment, int *count);
void			poly_bezier_segment_set_points	(PolyBezierSegment *segment, Point *points, int count);

//
// PolyLineSegment
//

class PolyLineSegment : public PathSegment {
 protected:
	virtual ~PolyLineSegment () {}

 public:
	static DependencyProperty* PointsProperty;

	PolyLineSegment () { }
	virtual Type::Kind GetObjectType () { return Type::POLYLINESEGMENT; };
	virtual int GetSize ();

	virtual void Append (moon_path *path);
};
PolyLineSegment	*poly_line_segment_new	();
Point*		poly_line_segment_get_points	(PolyLineSegment *segment, int *count);
void		poly_line_segment_set_points	(PolyLineSegment *segment, Point *points, int count);

//
// PolyQuadraticBezierSegment
//

class PolyQuadraticBezierSegment : public PathSegment {
 protected:
	virtual ~PolyQuadraticBezierSegment () {}

 public:
	static DependencyProperty* PointsProperty;

	PolyQuadraticBezierSegment () { }
	virtual Type::Kind GetObjectType () { return Type::POLYQUADRATICBEZIERSEGMENT; };
	virtual int GetSize ();

	virtual void Append (moon_path *path);
};
PolyQuadraticBezierSegment	*poly_quadratic_bezier_segment_new	();
Point*	poly_quadratic_bezier_segment_get_points	(PolyQuadraticBezierSegment *segment, int *count);
void	poly_quadratic_bezier_segment_set_points	(PolyQuadraticBezierSegment *segment, Point *points, int count);

//
// QuadraticBezierSegment
//

class QuadraticBezierSegment : public PathSegment {
 protected:
	virtual ~QuadraticBezierSegment () {}

 public:
	static DependencyProperty* Point1Property;
	static DependencyProperty* Point2Property;

	QuadraticBezierSegment () { }
	virtual Type::Kind GetObjectType () { return Type::QUADRATICBEZIERSEGMENT; };
	virtual int GetSize () { return MOON_PATH_CURVE_TO_LENGTH; }

	virtual void Append (moon_path *path);
};
QuadraticBezierSegment	*quadratic_bezier_segment_new	();
Point*	quadratic_bezier_segment_get_point1	(QuadraticBezierSegment *segment);
void	quadratic_bezier_segment_set_point1	(QuadraticBezierSegment *segment, Point *point);
Point*	quadratic_bezier_segment_get_point2	(QuadraticBezierSegment *segment);
void	quadratic_bezier_segment_set_point2	(QuadraticBezierSegment *segment, Point *point);

void geometry_init (void);

G_END_DECLS

#endif
