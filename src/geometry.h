/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * geometry.h: Geometry classes
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <glib.h>

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
/* @Namespace=System.Windows.Media */
class Geometry : public DependencyObject {
 protected:
#if FALSE
	enum GeometryFlags {
		GEOMETRY_NORMAL		= 0x01,	// normal drawing
		GEOMETRY_DEGENERATE	= 0x02,	// degenerate drawing, use the Stroke brush for filling
		GEOMETRY_NEEDS_FILL	= 0x04,	// filling, if specified, is needed (e.g. LineGeometry doesn't need it)
		GEOMETRY_NEEDS_CAPS	= 0x08,	// Stroke[Start|End]LineCap
		GEOMETRY_NEEDS_JOIN	= 0x10,	// StrokeLineJoin, StrokeMiterLimit
		GEOMETRY_MASK		= GEOMETRY_NORMAL | GEOMETRY_DEGENERATE | GEOMETRY_NEEDS_FILL | GEOMETRY_NEEDS_CAPS | GEOMETRY_NEEDS_JOIN
	};
	
	bool IsDegenerate () { return (flags & Geometry::GEOMETRY_DEGENERATE); }
	void SetGeometryFlags (GeometryFlags sf) { flags &= ~Geometry::GEOMETRY_MASK; flags |= sf; }

	int flags;
#endif
	moon_path *path;
	
	virtual ~Geometry ();
	
 public:
 	/* @PropertyType=FillRule,DefaultValue=FillRuleEvenOdd */
	static DependencyProperty *FillRuleProperty;
 	/* @PropertyType=Transform */
	static DependencyProperty *TransformProperty;

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Geometry () : path (NULL) { }
	
	virtual Type::Kind GetObjectType () { return Type::GEOMETRY; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual void Draw (Path *path, cairo_t *cr);
	virtual Rect ComputeBounds (Path *path, bool logical) { return Rect (0.0, 0.0, 0.0, 0.0); }
	virtual Rect ComputeBounds (Path *path, bool logical, cairo_matrix_t *matrix) { return ComputeBounds (path, logical); }

//	virtual Point GetOriginPoint (Path *path);

	virtual bool IsFilled () { return true; }

	virtual void Build (Path *path) {}
	virtual bool IsBuilt () { return path && path->cairo.num_data != 0; }
	virtual cairo_path_t *GetCairoPath () { return (path) ? &path->cairo : NULL; }
	
	//
	// Property Accessors
	//
	void SetFillRule (FillRule rule);
	FillRule GetFillRule ();
	
	void SetTransform (Transform *transform);
	Transform *GetTransform ();
};

FillRule geometry_get_fill_rule (Geometry *geometry);
void geometry_set_fill_rule (Geometry *geometry, FillRule fill_rule);
Transform *geometry_get_transform (Geometry *geometry);
void geometry_set_transform (Geometry *geometry, Transform *transform);


//
// GeometryCollection
//
/* @Namespace=System.Windows.Media */
class GeometryCollection : public DependencyObjectCollection {
 protected:
	virtual ~GeometryCollection () {}

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	GeometryCollection () { }
	
	virtual Type::Kind GetObjectType () { return Type::GEOMETRY_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::GEOMETRY; }
};


//
// GeometryGroup
//
/* @ContentProperty="Children" */
/* @Namespace=System.Windows.Media */
class GeometryGroup : public Geometry {
 protected:
	virtual ~GeometryGroup () {}

 public:
 	/* @PropertyType=GeometryCollection */
	static DependencyProperty *ChildrenProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	GeometryGroup ();
	
	virtual Type::Kind GetObjectType () { return Type::GEOMETRYGROUP; }
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subprop_args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	virtual void Draw (Path *path, cairo_t *cr);
	virtual Rect ComputeBounds (Path *path, bool logical) { return ComputeBounds (path, logical, NULL); }
	virtual Rect ComputeBounds (Path *path, bool logical, cairo_matrix_t *matrix);
	
	//
	// Property Accessors
	//
	void SetChildren (GeometryCollection *children);
	GeometryCollection *GetChildren ();
};

GeometryCollection *geometry_group_get_children (GeometryGroup *group);
void geometry_group_set_children (GeometryGroup *group, GeometryCollection *children);


//
// EllipseGeometry
//
/* @Namespace=System.Windows.Media */
class EllipseGeometry : public Geometry {
 protected:
	virtual void Build (Path *path);
	
	virtual ~EllipseGeometry () {}
	
 public:
 	/* @PropertyType=Point */
	static DependencyProperty *CenterProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *RadiusXProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *RadiusYProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	EllipseGeometry () { }
	
	virtual Type::Kind GetObjectType () { return Type::ELLIPSEGEOMETRY; }

	virtual Rect ComputeBounds (Path *path, bool logical);
	
	//
	// Property Accessors
	//
	void SetCenter (Point *center);
	Point *GetCenter ();
	
	void SetRadiusX (double radius);
	double GetRadiusX ();
	
	void SetRadiusY (double radius);
	double GetRadiusY ();
};

Point *ellipse_geometry_get_center (EllipseGeometry *ellipse);
void ellipse_geometry_set_center (EllipseGeometry *ellipse, Point *point);

double ellipse_geometry_get_radius_x (EllipseGeometry *ellipse);
void ellipse_geometry_set_radius_x (EllipseGeometry *ellipse, double radius_x);

double ellipse_geometry_get_radius_y (EllipseGeometry *ellipse);
void ellipse_geometry_set_radius_y (EllipseGeometry *ellipse, double radius_y);


//
// LineGeometry
//
/* @Namespace=System.Windows.Media */
class LineGeometry : public Geometry {
 protected:
	virtual void Build (Path *path);
	
	virtual ~LineGeometry () {}
	
 public:
 	/* @PropertyType=Point */
	static DependencyProperty *EndPointProperty;
 	/* @PropertyType=Point */
	static DependencyProperty *StartPointProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	LineGeometry () { }
	
	virtual Type::Kind GetObjectType () { return Type::LINEGEOMETRY; }
	
	virtual Rect ComputeBounds (Path *path, bool logical);
	
	//
	// Property Accessors
	//
	void SetEndPoint (Point *point);
	Point *GetEndPoint ();
	
	void SetStartPoint (Point *point);
	Point *GetStartPoint ();
};

Point *line_geometry_get_end_point (LineGeometry *line);
void line_geometry_set_end_point (LineGeometry *line, Point *point);

Point *line_geometry_get_start_point (LineGeometry *line);
void line_geometry_set_start_point (LineGeometry *line, Point *point);


//
// PathFigureCollection
//
/* @Namespace=System.Windows.Media */
class PathFigureCollection : public DependencyObjectCollection {
 protected:
	virtual ~PathFigureCollection () {}

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	PathFigureCollection () { }
	
	virtual Type::Kind GetObjectType () { return Type::PATHFIGURE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::PATHFIGURE; }
};


//
// PathGeometry
//
/* @ContentProperty="Figures" */
/* @Namespace=System.Windows.Media */
class PathGeometry : public Geometry {
	int logical_bounds_available:1;
	int physical_bounds_available:1;
	Rect logical_bounds;
	Rect physical_bounds;
	Rect CacheBounds (Path *path, bool logical, cairo_matrix_t *matrix);
 protected:
	virtual void Build (Path *path);
	
	virtual ~PathGeometry () {}
	
 public:
 	/* @PropertyType=PathFigureCollection */
	static DependencyProperty *FiguresProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PathGeometry ();
	PathGeometry (moon_path *pml_path);

	virtual Type::Kind GetObjectType () { return Type::PATHGEOMETRY; }
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual Rect ComputeBounds (Path *path, bool logical) { return ComputeBounds (path, logical, NULL); }
	virtual Rect ComputeBounds (Path *path, bool logical, cairo_matrix_t *matrix);
	
	// this is an element-by-element decision
	virtual bool IsFilled () { return true; }
	
	//
	// Property Accessors
	//
	void SetFigures (PathFigureCollection *figures);
	PathFigureCollection *GetFigures ();
};

PathFigureCollection *path_geometry_get_figures (PathGeometry *path);
void path_geometry_set_figures (PathGeometry *path, PathFigureCollection *figures);


//
// RectangleGeometry
//
/* @Namespace=System.Windows.Media */
class RectangleGeometry : public Geometry {
 protected:
	virtual void Build (Path *path);
	
	virtual ~RectangleGeometry () {}
	
 public:
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *RadiusXProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *RadiusYProperty;
 	/* @PropertyType=Rect */
	static DependencyProperty *RectProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	RectangleGeometry () { }
	
	virtual Type::Kind GetObjectType () { return Type::RECTANGLEGEOMETRY; }
	
	virtual Rect ComputeBounds (Path *path, bool logical);
	
	bool GetRadius (double *rx, double *ry);
	
	//
	// Property Accesors
	//
	void SetRadiusX (double radius);
	double GetRadiusX ();
	
	void SetRadiusY (double radius);
	double GetRadiusY ();
	
	void SetRect (Rect *rect);
	Rect *GetRect ();
};

double rectangle_geometry_get_radius_x (RectangleGeometry *rectangle);
void rectangle_geometry_set_radius_x (RectangleGeometry *rectangle, double radius);

double rectangle_geometry_get_radius_y (RectangleGeometry *rectangle);
void rectangle_geometry_set_radius_y (RectangleGeometry *rectangle, double radius);

Rect *rectangle_geometry_get_rect (RectangleGeometry *rectangle);
void rectangle_geometry_set_rect (RectangleGeometry *rectangle, Rect *rect);


//
// PathSegmentCollection
//
/* @Namespace=System.Windows.Media */
class PathSegmentCollection : public DependencyObjectCollection {
 protected:
	virtual ~PathSegmentCollection () {}

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	PathSegmentCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::PATHSEGMENT_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::PATHSEGMENT; }
};


//
// PathFigure
//
/* @ContentProperty="Segments" */
/* @Namespace=System.Windows.Media */
class PathFigure : public DependencyObject {
 protected:
	virtual ~PathFigure ();
	
 public:
 	/* @PropertyType=bool,DefaultValue=false */
	static DependencyProperty *IsClosedProperty;
 	/* @PropertyType=PathSegmentCollection */
	static DependencyProperty *SegmentsProperty;
 	/* @PropertyType=Point */
	static DependencyProperty *StartPointProperty;
	/* @PropertyType=bool,Version=2 */
	static DependencyProperty *IsFilledProperty;
	
	moon_path *path;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PathFigure ();
	
	virtual Type::Kind GetObjectType () { return Type::PATHFIGURE; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void Build (Path *shape);
	
	virtual bool IsBuilt () { return path && path->cairo.num_data != 0; }
	virtual cairo_path_t *GetCairoPath () { return (path) ? &path->cairo : NULL; }
	
	//
	// Property Accessors
	//
	void SetIsClosed (bool closed);
	bool GetIsClosed ();
	
	void SetSegments (PathSegmentCollection *segments);
	PathSegmentCollection *GetSegments ();
	
	void SetStartPoint (Point *point);
	Point *GetStartPoint ();
};

bool path_figure_get_is_closed (PathFigure *figure);
void path_figure_set_is_closed (PathFigure *figure, bool closed);

PathSegmentCollection *path_figure_get_segments (PathFigure *figure);
void path_figure_set_segments (PathFigure *figure, PathSegmentCollection *segments);

Point *path_figure_get_start_point (PathFigure *figure);
void path_figure_set_start_point (PathFigure *figure, Point *point);


//
// PathSegment
//
/* @Namespace=System.Windows.Media */
class PathSegment : public DependencyObject {
 protected:
	virtual void Build (Path *path) {}
	
	virtual ~PathSegment () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	PathSegment () {}
	
	virtual Type::Kind GetObjectType () { return Type::PATHSEGMENT; }
	
	virtual void Append (moon_path *path) {}
	virtual int GetPathSize () { return 0; }
};


//
// ArcSegment
//
/* @Namespace=System.Windows.Media */
class ArcSegment : public PathSegment {
 protected:
	virtual ~ArcSegment () {}

 public:
 	/* @PropertyType=bool,DefaultValue=false */
	static DependencyProperty *IsLargeArcProperty;
 	/* @PropertyType=Point */
	static DependencyProperty *PointProperty;
 	/* @PropertyType=double,DefaultValue=0.0*/
	static DependencyProperty *RotationAngleProperty;
 	/* @PropertyType=Point,ManagedPropertyType=Size */
	static DependencyProperty *SizeProperty;
 	/* @PropertyType=SweepDirection,DefaultValue=SweepDirectionCounterclockwise */
	static DependencyProperty *SweepDirectionProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ArcSegment () { }
	
	virtual Type::Kind GetObjectType () { return Type::ARCSEGMENT; }
	virtual int GetPathSize () { return 4 * MOON_PATH_CURVE_TO_LENGTH; } // non-optimal size, depends on angle
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetIsLargeArc (bool large);
	bool GetIsLargeArc ();
	
	void SetPoint (Point *point);
	Point *GetPoint ();
	
	void SetRotationAngle (double angle);
	double GetRotationAngle ();
	
	void SetSize (Point *size);
	Point *GetSize ();
	
	void SetSweepDirection (SweepDirection direction);
	SweepDirection GetSweepDirection ();
};

bool arc_segment_get_is_large_arc (ArcSegment *segment);
void arc_segment_set_is_large_arc (ArcSegment *segment, bool large);

Point *arc_segment_get_point (ArcSegment *segment);
void arc_segment_set_point (ArcSegment *segment, Point *point);

double arc_segment_get_rotation_angle (ArcSegment *segment);
void arc_segment_set_rotation_angle (ArcSegment *segment, double angle);

Point *arc_segment_get_size (ArcSegment *segment);
void arc_segment_set_size (ArcSegment *segment, Point *size);

SweepDirection arc_segment_get_sweep_direction (ArcSegment *segment);
void arc_segment_set_sweep_direction (ArcSegment *segment, SweepDirection direction);


//
// BezierSegment
//
/* @Namespace=System.Windows.Media */
class BezierSegment : public PathSegment {
 protected:
	virtual ~BezierSegment () {}

 public:
 	/* @PropertyType=Point */
	static DependencyProperty *Point1Property;
 	/* @PropertyType=Point */
	static DependencyProperty *Point2Property;
 	/* @PropertyType=Point */
	static DependencyProperty *Point3Property;
	
	/* @GenerateCBinding,GeneratePInvoke */
	BezierSegment () { }
	
	virtual Type::Kind GetObjectType () { return Type::BEZIERSEGMENT; }
	virtual int GetPathSize () { return MOON_PATH_CURVE_TO_LENGTH; }
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoint1 (Point *point);
	Point *GetPoint1 ();
	
	void SetPoint2 (Point *point);
	Point *GetPoint2 ();
	
	void SetPoint3 (Point *point);
	Point *GetPoint3 ();
};

Point *bezier_segment_get_point1 (BezierSegment *segment);
void bezier_segment_set_point1 (BezierSegment *segment, Point *point);

Point *bezier_segment_get_point2 (BezierSegment *segment);
void bezier_segment_set_point2 (BezierSegment *segment, Point *point);

Point *bezier_segment_get_point3 (BezierSegment *segment);
void bezier_segment_set_point3 (BezierSegment *segment, Point *point);


//
// LineSegment
//
/* @Namespace=System.Windows.Media */
class LineSegment : public PathSegment {
 protected:
	virtual ~LineSegment () {}

 public:
 	/* @PropertyType=Point */
	static DependencyProperty *PointProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	LineSegment () { }
	
	virtual Type::Kind GetObjectType () { return Type::LINESEGMENT; }
	virtual int GetPathSize () { return MOON_PATH_LINE_TO_LENGTH; }
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoint (Point *point);
	Point *GetPoint ();
};

Point *line_segment_get_point (LineSegment *segment);
void line_segment_set_point (LineSegment *segment, Point *point);


//
// PolyBezierSegment
//

/* @Namespace=System.Windows.Media */
class PolyBezierSegment : public PathSegment {
 protected:
	virtual ~PolyBezierSegment () {}
	
	PointCollection *GetPoints ();
	
 public:
 	/* @PropertyType=PointCollection */
	static DependencyProperty *PointsProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PolyBezierSegment () { }
	
	virtual Type::Kind GetObjectType () { return Type::POLYBEZIERSEGMENT; }
	virtual int GetPathSize ();
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoints (PointCollection *points);
};

void poly_bezier_segment_set_points (PolyBezierSegment *segment, PointCollection *points);


//
// PolyLineSegment
//

/* @Namespace=System.Windows.Media */
class PolyLineSegment : public PathSegment {
 protected:
	virtual ~PolyLineSegment () {}
	
	PointCollection *GetPoints ();
	
 public:
 	/* @PropertyType=PointCollection */
	static DependencyProperty *PointsProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PolyLineSegment () { }
	
	virtual Type::Kind GetObjectType () { return Type::POLYLINESEGMENT; }
	virtual int GetPathSize ();
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoints (PointCollection *points);
};

void poly_line_segment_set_points (PolyLineSegment *segment, PointCollection *points);


//
// PolyQuadraticBezierSegment
//

/* @Namespace=System.Windows.Media */
class PolyQuadraticBezierSegment : public PathSegment {
 protected:
	virtual ~PolyQuadraticBezierSegment () {}
	
	PointCollection *GetPoints ();
	
 public:
 	/* @PropertyType=PointCollection */
	static DependencyProperty *PointsProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PolyQuadraticBezierSegment () { }
	
	virtual Type::Kind GetObjectType () { return Type::POLYQUADRATICBEZIERSEGMENT; }
	virtual int GetPathSize ();

	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoints (PointCollection *points);
};

void poly_quadratic_bezier_segment_set_points (PolyQuadraticBezierSegment *segment, PointCollection *points);


//
// QuadraticBezierSegment
//

/* @Namespace=System.Windows.Media */
class QuadraticBezierSegment : public PathSegment {
 protected:
	virtual ~QuadraticBezierSegment () {}

 public:
 	/* @PropertyType=Point */
	static DependencyProperty *Point1Property;
 	/* @PropertyType=Point */
	static DependencyProperty *Point2Property;
	
	/* @GenerateCBinding,GeneratePInvoke */
	QuadraticBezierSegment () { }
	
	virtual Type::Kind GetObjectType () { return Type::QUADRATICBEZIERSEGMENT; }
	virtual int GetPathSize () { return MOON_PATH_CURVE_TO_LENGTH; }
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoint1 (Point *point);
	Point *GetPoint1 ();
	
	void SetPoint2 (Point *point);
	Point *GetPoint2 ();
};

Point *quadratic_bezier_segment_get_point1 (QuadraticBezierSegment *segment);
void quadratic_bezier_segment_set_point1 (QuadraticBezierSegment *segment, Point *point);

Point *quadratic_bezier_segment_get_point2 (QuadraticBezierSegment *segment);
void quadratic_bezier_segment_set_point2 (QuadraticBezierSegment *segment, Point *point);

G_END_DECLS

#endif
