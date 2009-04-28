/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * shape.h: This match the classes inside System.Windows.Shapes
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <glib.h>
#include <cairo.h>

#include "geometry.h"
#include "frameworkelement.h"
#include "moon-path.h"

class Brush;

//
// Helpers
//

G_BEGIN_DECLS

cairo_fill_rule_t convert_fill_rule (FillRule fill_rule);
void calc_line_bounds (double x1, double x2, double y1, double y2, double thickness, PenLineCap start_cap, PenLineCap end_cap, Rect* bounds);

G_END_DECLS


//
// Shape class 
// 
/* @Namespace=System.Windows.Shapes */
class Shape : public FrameworkElement {
 protected:
	virtual ~Shape ();

	Brush *stroke, *fill;
	cairo_surface_t *cached_surface;
	gint64 cached_size;
	bool needs_clip;

	void DoDraw (cairo_t *cr, bool do_op);

	void SetupLineCaps (cairo_t *cr);
	void SetupLineJoinMiter (cairo_t *cr);
	virtual bool SetupLine (cairo_t* cr);
	bool SetupDashes (cairo_t *cr, double thickness);
	bool SetupDashes (cairo_t *cr, double thickness, double offset);
	bool Fill (cairo_t *cr, bool do_op);
	void Clip (cairo_t *cr);
	virtual bool DrawShape (cairo_t *cr, bool do_op) { g_warning ("%s does not implement DrawShape ().", GetTypeName ()); return false; }
//	virtual bool DrawDegenerateShape (cairo_t *cr, bool do_op) = 0;

	moon_path *path;
	virtual void InvalidatePathCache (bool free = false);
	void InvalidateSurfaceCache (void);
	void InvalidateNaturalBounds ();
	void InvalidateFillBounds ();
	void InvalidateStrokeBounds ();
	void InvalidateStretch ();
	
	Rect GetStretchExtents ();
	Rect GetNaturalBounds ();
	
	bool IsCandidateForCaching (void);

	virtual Rect ComputeShapeBounds (bool logical) { return ComputeShapeBounds (logical, NULL); }
	virtual Rect ComputeShapeBounds (bool logical, cairo_matrix_t * matrix);

	virtual void ShiftPosition (Point p);
	virtual void TransformBounds (cairo_matrix_t *old, cairo_matrix_t *current);

	virtual Rect ComputeStretchBounds ();
	
	DoubleCollection *GetStrokeDashArray ();
	Rect natural_bounds;
 public: 
	cairo_matrix_t stretch_transform;
 	/* @PropertyType=Brush,GenerateAccessors */
	const static int FillProperty;
 	/* @PropertyType=Stretch,AutoCreator=Shape::CreateDefaultStretch,GenerateAccessors */
	const static int StretchProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	const static int StrokeProperty;
 	/* @PropertyType=DoubleCollection,GenerateAccessors */
	const static int StrokeDashArrayProperty;
 	/* @PropertyType=PenLineCap,DefaultValue=PenLineCapFlat,GenerateAccessors */
	const static int StrokeDashCapProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int StrokeDashOffsetProperty;
 	/* @PropertyType=PenLineCap,DefaultValue=PenLineCapFlat,GenerateAccessors */
	const static int StrokeEndLineCapProperty;
 	/* @PropertyType=PenLineJoin,DefaultValue=PenLineJoinMiter,GenerateAccessors */
	const static int StrokeLineJoinProperty;
 	/* @PropertyType=double,DefaultValue=10.0,GenerateAccessors */
	const static int StrokeMiterLimitProperty;
 	/* @PropertyType=PenLineCap,DefaultValue=PenLineCapFlat,GenerateAccessors */
	const static int StrokeStartLineCapProperty;
 	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int StrokeThicknessProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Shape ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual Transform *GetGeometryTransform ();
	
	//
	// Overrides from UIElement.
	//
	virtual Size ComputeActualSize ();
	virtual Size MeasureOverride (Size Availablesize);
	virtual Size ArrangeOverride (Size finalSize);
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetOriginPoint () { return extents.GetTopLeft (); }
	
	//
	// new virtual methods for shapes
	//
	virtual bool IsStroked () { return stroke; }
	virtual bool IsFilled () { return fill; }
	virtual bool CanFill () { return false; }
	virtual bool CanFindElement () { return IsFilled () || IsStroked (); }
	virtual FillRule GetFillRule () { return FillRuleNonzero; }
	
	//
	// Draw: draws the Shape in the cairo context (affine transforms are set before this
	// is called). 
	//
	// This is called multiple times: one for fills, one for strokes
	// if they are both set.   It will also be called to compute the bounding box.
	//
	virtual void Draw (cairo_t *cr);
	virtual void BuildPath () {};
	void Stroke (cairo_t *cr, bool do_op);
	bool NeedsClipping ();

	virtual void CacheInvalidateHint (void);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual Point GetTransformOrigin ();

	// State helpers
	bool IsEmpty () { return (flags & UIElement::SHAPE_EMPTY); };
	bool IsNormal () { return (flags & UIElement::SHAPE_NORMAL); };
	bool IsDegenerate () { return (flags & UIElement::SHAPE_DEGENERATE); };
	bool HasRadii () { return (flags & UIElement::SHAPE_RADII); };
	void SetShapeFlags (UIElementFlags sf) { flags &= ~UIElement::SHAPE_MASK; flags |= sf; };
	void AddShapeFlags (UIElementFlags sf) { flags |= sf; };
	
	//
	// Property Accessors
	//
	void SetFill (Brush *fill);
	Brush *GetFill ();
	
	void SetStroke (Brush *stroke);
	Brush *GetStroke ();
	
	void SetStretch (Stretch stretch);
	Stretch GetStretch ();
	
	void SetStrokeDashArray (DoubleCollection *dashes);
	
	void SetStrokeDashCap (PenLineCap cap);
	PenLineCap GetStrokeDashCap ();
	
	void SetStrokeDashOffset (double offset);
	double GetStrokeDashOffset ();
	
	void SetStrokeEndLineCap (PenLineCap cap);
	PenLineCap GetStrokeEndLineCap ();
	
	void SetStrokeLineJoin (PenLineJoin join);
	PenLineJoin GetStrokeLineJoin ();
	
	void SetStrokeMiterLimit (double limit);
	double GetStrokeMiterLimit ();
	
	void SetStrokeStartLineCap (PenLineCap cap);
	PenLineCap GetStrokeStartLineCap ();
	
	void SetStrokeThickness (double thickness);
	double GetStrokeThickness ();

	static Value* CreateDefaultStretch (DependencyObject *instance, DependencyProperty *property);
};


//
// Ellipse
//
/* @Namespace=System.Windows.Shapes */
class Ellipse : public Shape {
 protected:
	virtual ~Ellipse () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical);
	virtual Rect ComputeStretchBounds ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	Ellipse ();
	
	virtual void BuildPath ();
	virtual bool CanFill () { return true; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
};


//
// Rectangle class 
// 
/* @Namespace=System.Windows.Shapes */
class Rectangle : public Shape {
 protected:
	virtual ~Rectangle () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical);
	virtual Rect ComputeStretchBounds ();

 public:
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RadiusXProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RadiusYProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Rectangle ();
	
	virtual Rect GetCoverageBounds ();
	virtual void BuildPath ();
	virtual bool CanFill () { return true; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	//
	// Property Accessors
	//
	void SetRadiusX (double radius);
	double GetRadiusX ();
	
	void SetRadiusY (double radius);
	double GetRadiusY ();
};


//
// Line class 
// 
/* @Namespace=System.Windows.Shapes */
class Line : public Shape {
 protected:
	virtual ~Line () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical);
	
 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int X1Property;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int Y1Property;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int X2Property;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int Y2Property;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Line () { SetObjectType (Type::LINE); }
	
	virtual void BuildPath ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	// Line has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();
	
	//
	// Property Accessors
	//
	void SetX1 (double x1);
	double GetX1 ();
	
	void SetY1 (double y1);
	double GetY1 ();
	
	void SetX2 (double x2);
	double GetX2 ();
	
	void SetY2 (double y2);
	double GetY2 ();
};


//
// Polygon
//
/* @Namespace=System.Windows.Shapes */
class Polygon : public Shape {
 protected:
	virtual ~Polygon () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	
	PointCollection *GetPoints ();
	
 public:
 	/* @PropertyType=FillRule,DefaultValue=FillRuleEvenOdd,GenerateAccessors */
	const static int FillRuleProperty;
 	/* @PropertyType=PointCollection,AutoCreateValue,GenerateAccessors */
	const static int PointsProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	Polygon ();
	
	// Polygon has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual void BuildPath ();
	
	virtual bool CanFill () { return true; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	//
	// Property Accessors
	//
	void SetFillRule (FillRule rule);
	virtual FillRule GetFillRule ();
	
	void SetPoints (PointCollection *points);
};


//
// Polyline
//
/* @Namespace=System.Windows.Shapes */
class Polyline : public Shape {
 protected:
	virtual ~Polyline () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	
	PointCollection *GetPoints ();
	
 public:
 	/* @PropertyType=FillRule,DefaultValue=FillRuleEvenOdd,GenerateAccessors */
	const static int FillRuleProperty;
 	/* @PropertyType=PointCollection,AutoCreateValue,GenerateAccessors */
	const static int PointsProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Polyline ();
	
	// Polyline has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();
	
	virtual void BuildPath ();
	
	virtual bool CanFill () { return true; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	//
	// Property Accessors
	//
	void SetFillRule (FillRule rule);
	virtual FillRule GetFillRule ();
	
	void SetPoints (PointCollection *points);
};


//
// Path
//
/* @Namespace=System.Windows.Shapes */
class Path : public Shape {
 protected:
	virtual ~Path () {}
	virtual bool SetupLine (cairo_t *cr);
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical, cairo_matrix_t *matrix);

 public:
 	/* @PropertyType=Geometry,GenerateAccessors */
	const static int DataProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Path () { SetObjectType (Type::PATH); }
	
	// Path has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();
	
	virtual void Draw (cairo_t *cr);
	
	virtual bool CanFill () { return true; }
	virtual FillRule GetFillRule ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	//
	// Property Accessors
	//
	void SetData (Geometry *data);
	Geometry *GetData ();
};

#endif /* __SHAPE_H__ */
