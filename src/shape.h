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
	bool IsCandidateForCaching (void);

	virtual Rect ComputeShapeBounds (bool logical) { return ComputeShapeBounds (logical, NULL); }
	virtual Rect ComputeShapeBounds (bool logical, cairo_matrix_t * matrix);

	virtual void ShiftPosition (Point p);
	virtual void TransformBounds (cairo_matrix_t *old, cairo_matrix_t *current);

	cairo_matrix_t stretch_transform;
	virtual Rect ComputeStretchBounds ();
	
	DoubleCollection *GetStrokeDashArray ();
	
 public: 
 	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *FillProperty;
 	/* @PropertyType=Stretch,DefaultValue=StretchNone,GenerateAccessors */
	static DependencyProperty *StretchProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *StrokeProperty;
 	/* @PropertyType=DoubleCollection,GenerateAccessors */
	static DependencyProperty *StrokeDashArrayProperty;
 	/* @PropertyType=PenLineCap,DefaultValue=PenLineCapFlat,GenerateAccessors */
	static DependencyProperty *StrokeDashCapProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *StrokeDashOffsetProperty;
 	/* @PropertyType=PenLineCap,DefaultValue=PenLineCapFlat,GenerateAccessors */
	static DependencyProperty *StrokeEndLineCapProperty;
 	/* @PropertyType=PenLineJoin,DefaultValue=PenLineJoinMiter,GenerateAccessors */
	static DependencyProperty *StrokeLineJoinProperty;
 	/* @PropertyType=double,DefaultValue=10.0,GenerateAccessors */
	static DependencyProperty *StrokeMiterLimitProperty;
 	/* @PropertyType=PenLineCap,DefaultValue=PenLineCapFlat,GenerateAccessors */
	static DependencyProperty *StrokeStartLineCapProperty;
 	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	static DependencyProperty *StrokeThicknessProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Shape ();
	
	//
	// Overrides from UIElement.
	//
	virtual Size MeasureOverride (Size Availablesize);
	virtual Size ArrangeOverride (Size finalSize);
	virtual void Render (cairo_t *cr, Region *region);
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
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

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

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
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
	static DependencyProperty *RadiusXProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *RadiusYProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Rectangle ();
	
	virtual Rect GetCoverageBounds ();
	virtual void BuildPath ();
	virtual bool CanFill () { return true; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

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
	static DependencyProperty *X1Property;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *Y1Property;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *X2Property;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *Y2Property;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Line () { SetObjectType (Type::LINE); }
	
	virtual void BuildPath ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
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
	static DependencyProperty *FillRuleProperty;
 	/* @PropertyType=PointCollection,GenerateAccessors */
	static DependencyProperty *PointsProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	Polygon ();
	
	// Polygon has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual void BuildPath ();
	
	virtual bool CanFill () { return true; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
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
	static DependencyProperty *FillRuleProperty;
 	/* @PropertyType=PointCollection,GenerateAccessors */
	static DependencyProperty *PointsProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Polyline ();
	
	// Polyline has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();
	
	virtual void BuildPath ();
	
	virtual bool CanFill () { return true; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
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
	static DependencyProperty *DataProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Path () { SetObjectType (Type::PATH); }
	
	// Path has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();
	
	virtual void Draw (cairo_t *cr);
	
	virtual bool CanFill () { return true; }
	virtual FillRule GetFillRule ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	//
	// Property Accessors
	//
	void SetData (Geometry *data);
	Geometry *GetData ();
};

#endif /* __SHAPE_H__ */
