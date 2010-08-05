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

#include <cairo.h>
#include "enums.h"
#include "rect.h"
#include "transform.h"
#include "moon-path.h"

namespace Moonlight {

//
// Geometry
//
/* @Namespace=System.Windows.Media */
class Geometry : public DependencyObject {
 protected:
#if 0
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

	Rect local_bounds;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	Geometry ();

	virtual ~Geometry ();
	virtual Rect ComputePathBounds ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
 public:
 	/* @PropertyType=Transform,GenerateAccessors */
	const static int TransformProperty;

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subprop_args);

	virtual void Draw (cairo_t *cr);

	/* @GenerateCBinding,GeneratePInvoke */
	Rect GetBounds ();
	void InvalidateCache ();

	//virtual Point GetOriginPoint (Path *path);

	virtual bool IsFilled () { return true; }

	virtual void Build () {}
	virtual bool IsBuilt () { return path && path->cairo.num_data != 0; }
	virtual cairo_path_t *GetCairoPath () { return (path) ? &path->cairo : NULL; }
	
	//
	// Property Accessors
	//
	virtual FillRule GetFillRule () { return FillRuleNonzero; };
	
	void SetTransform (Transform *transform);
	Transform *GetTransform ();
};


//
// GeometryCollection
//
/* @Namespace=System.Windows.Media */
class GeometryCollection : public DependencyObjectCollection {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	GeometryCollection ();

	virtual ~GeometryCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
 public:

	virtual Type::Kind GetElementType () { return Type::GEOMETRY; }
};


//
// GeometryGroup
//
/* @ContentProperty="Children" */
/* @Namespace=System.Windows.Media */
class GeometryGroup : public Geometry {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	GeometryGroup ();

	virtual ~GeometryGroup ();
	virtual Rect ComputePathBounds ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
 public:
 	/* @PropertyType=FillRule,DefaultValue=FillRuleEvenOdd,GenerateAccessors */
	const static int FillRuleProperty;
 	/* @PropertyType=GeometryCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int ChildrenProperty;
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	virtual void Draw (cairo_t *cr);
	
	//
	// Property Accessors
	//
	void SetFillRule (FillRule rule);
	virtual FillRule GetFillRule ();
	
	void SetChildren (GeometryCollection *children);
	GeometryCollection *GetChildren ();
};


//
// EllipseGeometry
//
/* @Namespace=System.Windows.Media */
class EllipseGeometry : public Geometry {
 protected:
	virtual void Build ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	EllipseGeometry ();
	
	virtual ~EllipseGeometry ();
	virtual Rect ComputePathBounds ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int CenterProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RadiusXProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RadiusYProperty;

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


//
// LineGeometry
//
/* @Namespace=System.Windows.Media */
class LineGeometry : public Geometry {
 protected:
	virtual void Build ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	LineGeometry ();
	
	virtual ~LineGeometry ();
	virtual Rect ComputePathBounds ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int EndPointProperty;
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int StartPointProperty;
	
	//
	// Property Accessors
	//
	void SetEndPoint (Point *point);
	Point *GetEndPoint ();
	
	void SetStartPoint (Point *point);
	Point *GetStartPoint ();
};


//
// PathFigureCollection
//
/* @Namespace=System.Windows.Media */
class PathFigureCollection : public DependencyObjectCollection {
 protected:
	virtual ~PathFigureCollection ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	PathFigureCollection ();

	virtual Type::Kind GetElementType () { return Type::PATHFIGURE; }
};


//
// PathGeometry
//
/* @ContentProperty="Figures" */
/* @Namespace=System.Windows.Media */
class PathGeometry : public Geometry {
 protected:
	virtual void Build ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	PathGeometry ();

	virtual ~PathGeometry ();
	virtual Rect ComputePathBounds ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
 	/* @PropertyType=FillRule,DefaultValue=FillRuleEvenOdd,GenerateAccessors */
	const static int FillRuleProperty;
 	/* @PropertyType=PathFigureCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int FiguresProperty;

	/* @SkipFactories */
	PathGeometry (moon_path *pml_path);

	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	// this is an element-by-element decision
	virtual bool IsFilled () { return true; }
	
	//
	// Property Accessors
	//
	void SetFillRule (FillRule rule);
	virtual FillRule GetFillRule ();
	
	void SetFigures (PathFigureCollection *figures);
	PathFigureCollection *GetFigures ();
};


//
// RectangleGeometry
//
/* @Namespace=System.Windows.Media */
class RectangleGeometry : public Geometry {
 protected:
	virtual void Build ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	RectangleGeometry ();
	
	virtual ~RectangleGeometry ();
	virtual Rect ComputePathBounds ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RadiusXProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RadiusYProperty;
 	/* @PropertyType=Rect,DefaultValue=Rect(),GenerateAccessors */
	const static int RectProperty;
	
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


//
// PathSegmentCollection
//
/* @Namespace=System.Windows.Media */
class PathSegmentCollection : public DependencyObjectCollection {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PathSegmentCollection ();
	
	virtual ~PathSegmentCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	virtual Type::Kind GetElementType () { return Type::PATHSEGMENT; }
};


//
// PathFigure
//
/* @ContentProperty="Segments" */
/* @Namespace=System.Windows.Media */
class PathFigure : public DependencyObject {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PathFigure ();
	
	virtual ~PathFigure ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
 	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsClosedProperty;
 	/* @PropertyType=PathSegmentCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int SegmentsProperty;
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int StartPointProperty;
	/* @PropertyType=bool,DefaultValue=true,Version=2,GenerateAccessors */
	const static int IsFilledProperty;
	
	moon_path *path;
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void Build ();
	
	virtual bool IsBuilt () { return path && path->cairo.num_data != 0; }
	virtual cairo_path_t *GetCairoPath () { return (path) ? &path->cairo : NULL; }
	
	//
	// Property Accessors
	//
	void SetIsClosed (bool closed);
	bool GetIsClosed ();
	
	void SetIsFilled (bool value);
	bool GetIsFilled ();

	void SetSegments (PathSegmentCollection *segments);
	PathSegmentCollection *GetSegments ();
	
	void SetStartPoint (Point *point);
	Point *GetStartPoint ();
};


//
// PathSegment
//
/* @Namespace=System.Windows.Media */
class PathSegment : public DependencyObject {
 protected:
	virtual void Build ();
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	PathSegment ();

	virtual ~PathSegment ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	virtual void Append (moon_path *path);
	virtual int GetPathSize () { return 0; }
};


//
// ArcSegment
//
/* @Namespace=System.Windows.Media */
class ArcSegment : public PathSegment {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	ArcSegment ();
	
	virtual ~ArcSegment ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
 	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsLargeArcProperty;
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int PointProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RotationAngleProperty;
 	/* @PropertyType=Size,DefaultValue=Size(),GenerateAccessors */
	const static int SizeProperty;
 	/* @PropertyType=SweepDirection,DefaultValue=SweepDirectionCounterclockwise,GenerateAccessors */
	const static int SweepDirectionProperty;
	
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
	
	void SetSize (Size *size);
	Size *GetSize ();
	
	void SetSweepDirection (SweepDirection direction);
	SweepDirection GetSweepDirection ();
};


//
// BezierSegment
//
/* @Namespace=System.Windows.Media */
class BezierSegment : public PathSegment {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	BezierSegment ();
	
	virtual ~BezierSegment ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int Point1Property;
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int Point2Property;
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int Point3Property;
	
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


//
// LineSegment
//
/* @Namespace=System.Windows.Media */
class LineSegment : public PathSegment {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	LineSegment ();
	
	virtual ~LineSegment ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
 public:
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int PointProperty;
	
	virtual int GetPathSize () { return MOON_PATH_LINE_TO_LENGTH; }
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoint (Point *point);
	Point *GetPoint ();
};


//
// PolyBezierSegment
//
/* @Namespace=System.Windows.Media */
class PolyBezierSegment : public PathSegment {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PolyBezierSegment ();

	virtual ~PolyBezierSegment ();
	
	PointCollection *GetPoints ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
 	/* @PropertyType=PointCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int PointsProperty;
	
	virtual int GetPathSize ();
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	/* @GenerateCBinding */
	void SetPoints (PointCollection *points);
};


//
// PolyLineSegment
//
/* @Namespace=System.Windows.Media */
class PolyLineSegment : public PathSegment {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PolyLineSegment ();

	virtual ~PolyLineSegment ();
	
	PointCollection *GetPoints ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
 	/* @PropertyType=PointCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int PointsProperty;
	
	virtual int GetPathSize ();
	
	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoints (PointCollection *points);
};


//
// PolyQuadraticBezierSegment
//
/* @Namespace=System.Windows.Media */
class PolyQuadraticBezierSegment : public PathSegment {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PolyQuadraticBezierSegment ();

	virtual ~PolyQuadraticBezierSegment ();
	
	PointCollection *GetPoints ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
 	/* @PropertyType=PointCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int PointsProperty;
	
	virtual int GetPathSize ();

	virtual void Append (moon_path *path);
	
	//
	// Property Accessors
	//
	void SetPoints (PointCollection *points);
};


//
// QuadraticBezierSegment
//
/* @Namespace=System.Windows.Media */
class QuadraticBezierSegment : public PathSegment {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	QuadraticBezierSegment ();
	
	virtual ~QuadraticBezierSegment ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int Point1Property;
 	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int Point2Property;
	
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

};
#endif
