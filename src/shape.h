/*
 * shape.h: This match the classes inside System.Windows.Shapes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *   Sebastien Pouliot  <sebastien@ximian.com>
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#ifndef __SHAPE_H__
#define __SHAPE_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "brush.h"
#include "geometry.h"
#include "moon-path.h"

//
// Helpers
//

cairo_fill_rule_t convert_fill_rule (FillRule fill_rule);
void calc_line_bounds (double x1, double x2, double y1, double y2, double thickness, Rect* bounds);


//
// Shape class 
// 
class Shape : public FrameworkElement {
 protected:
	virtual ~Shape ();

	Brush *stroke, *fill;
	Point origin;
	cairo_surface_t *cached_surface;
	int64_t cached_size;

	void DoDraw (cairo_t *cr, bool do_op);

	void SetupLineCaps (cairo_t *cr);
	void SetupLineJoinMiter (cairo_t *cr);
	virtual bool SetupLine (cairo_t* cr);
	bool SetupDashes (cairo_t *cr, double thickness);
	bool SetupDashes (cairo_t *cr, double thickness, double offset);
	bool Fill (cairo_t *cr, bool do_op);
	void Clip (cairo_t *cr);
	virtual bool DrawShape (cairo_t *cr, bool do_op) = 0;
//	virtual bool DrawDegenerateShape (cairo_t *cr, bool do_op) = 0;

	moon_path *path;
	virtual void InvalidatePathCache (bool free = false);
	void InvalidateSurfaceCache (void);
	bool IsCandidateForCaching (void);

	virtual Rect ComputeShapeBounds (bool logical);
	virtual Rect ComputeLargestRectangle ();
	
	cairo_matrix_t stretch_transform;
	Rect ComputeStretchBounds (Rect shape_bounds, Rect logical_bounds);
	Point ComputeOriginPoint (Rect shape_bounds);
	Rect extents;
 public: 
	static DependencyProperty* FillProperty;
	static DependencyProperty* StretchProperty;
	static DependencyProperty* StrokeProperty;
	static DependencyProperty* StrokeDashArrayProperty;
	static DependencyProperty* StrokeDashCapProperty;
	static DependencyProperty* StrokeDashOffsetProperty;
	static DependencyProperty* StrokeEndLineCapProperty;
	static DependencyProperty* StrokeLineJoinProperty;
	static DependencyProperty* StrokeMiterLimitProperty;
	static DependencyProperty* StrokeStartLineCapProperty;
	static DependencyProperty* StrokeThicknessProperty;

	Shape ();
	virtual Type::Kind GetObjectType () { return Type::SHAPE; };

	//
	// Overrides from UIElement.
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetTransformOrigin ();
	virtual Point GetOriginPoint () { return origin; }

	Rect ComputeLargestRectangleBounds ();
	
	//
	// new virtual methods for shapes
	//
	
	virtual bool IsStroked () { return stroke; }
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
	virtual bool ClipOnHeightAndWidth () { return false; }
	void Stroke (cairo_t *cr, bool do_op);
	bool NeedsClipping ();
	bool MixedHeightWidth (Value **width, Value **height);

	virtual void CacheInvalidateHint (void);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

	//
	bool IsEmpty () { return (flags & UIElement::SHAPE_EMPTY); };
	bool IsNormal () { return (flags & UIElement::SHAPE_NORMAL); };
	bool IsDegenerate () { return (flags & UIElement::SHAPE_DEGENERATE); };
	bool HasRadii () { return (flags & UIElement::SHAPE_RADII); };
	void SetShapeFlags (UIElementFlags sf) { flags &= ~UIElement::SHAPE_MASK; flags |= sf; };
	void AddShapeFlags (UIElementFlags sf) { flags |= sf; };
};

Brush	       *shape_get_fill			(Shape *shape);
void		shape_set_fill			(Shape *shape, Brush *value);
Brush	       *shape_get_stroke		(Shape *shape);
void		shape_set_stroke		(Shape *shape, Brush *value);
Stretch		shape_get_stretch		(Shape *shape);
void		shape_set_stretch		(Shape *shape, Stretch value);
PenLineCap	shape_get_stroke_dash_cap	(Shape *shape);
void		shape_set_stroke_dash_cap	(Shape *shape, PenLineCap value);
PenLineCap	shape_get_stroke_start_line_cap	(Shape *shape);
void		shape_set_stroke_start_line_cap	(Shape *shape, PenLineCap value);
PenLineCap	shape_get_stroke_end_line_cap	(Shape *shape);
void		shape_set_stroke_end_line_cap	(Shape *shape, PenLineCap value);
double		shape_get_stroke_dash_offset	(Shape *shape);
void		shape_set_stroke_dash_offset	(Shape *shape, double value);
double		shape_get_stroke_miter_limit	(Shape *shape);
void		shape_set_stroke_miter_limit	(Shape *shape, double value);
double		shape_get_stroke_thickness	(Shape *shape);
void		shape_set_stroke_thickness	(Shape *shape, double value);
PenLineJoin	shape_get_stroke_line_join	(Shape *shape);
void		shape_set_stroke_line_join	(Shape *shape, PenLineJoin value);
double	       *shape_get_stroke_dash_array	(Shape *shape, int *count);
void		shape_set_stroke_dash_array	(Shape *shape, double* dashes, int count);


//
// Ellipse
//
class Ellipse : public Shape {
 protected:
	virtual ~Ellipse () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeLargestRectangle ();
 public:
	Ellipse ();
	virtual Type::Kind GetObjectType () { return Type::ELLIPSE; };

	virtual void BuildPath ();
	virtual bool CanFill () { return true; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
};

Ellipse *ellipse_new (void);


//
// Rectangle class 
// 
class Rectangle : public Shape {
 protected:
	virtual ~Rectangle () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
 public:
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	Rectangle ();
	virtual Type::Kind GetObjectType () { return Type::RECTANGLE; };

	virtual void BuildPath ();
	virtual bool CanFill () { return true; }

	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	bool GetRadius (double *rx, double *ry);
	virtual Rect ComputeLargestRectangle ();
};

Rectangle *rectangle_new          (void);
double     rectangle_get_radius_x (Rectangle *rectangle);
void       rectangle_set_radius_x (Rectangle *rectangle, double value);
double     rectangle_get_radius_y (Rectangle *rectangle);
void       rectangle_set_radius_y (Rectangle *rectangle, double value);


//
// Line class 
// 
class Line : public Shape {
 protected:
	virtual ~Line () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical);
 public:
	static DependencyProperty* X1Property;
	static DependencyProperty* Y1Property;
	static DependencyProperty* X2Property;
	static DependencyProperty* Y2Property;

	virtual Type::Kind GetObjectType () { return Type::LINE; };
	
	virtual void BuildPath ();
	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	// Line has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();
};

Line *line_new  (void);
double line_get_x1 (Line *line);
void line_set_x1 (Line *line, double value);
double line_get_y1 (Line *line);
void line_set_y1 (Line *line, double value);
double line_get_x2 (Line *line);
void line_set_x2 (Line *line, double value);
double line_get_y2 (Line *line);
void line_set_y2 (Line *line, double value);


//
// Polygon
//
class Polygon : public Shape {
 protected:
	virtual ~Polygon () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical);
 public:
	static DependencyProperty* FillRuleProperty;
	static DependencyProperty* PointsProperty;

	Polygon () { };
	virtual Type::Kind GetObjectType () { return Type::POLYGON; };

	// Polygon has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual void BuildPath ();

	virtual FillRule GetFillRule ();

	virtual bool CanFill () { return true; }

	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);
};

Polygon	       *polygon_new (void);
FillRule	polygon_get_fill_rule	(Polygon *polygon);
void		polygon_set_fill_rule	(Polygon *polygon, FillRule value);
Point	       *polygon_get_points	(Polygon *polygon, int *count);
void		polygon_set_points	(Polygon *polygon, Point* points, int count);


//
// Polyline
//
class Polyline : public Shape {
 protected:
	virtual ~Polyline () {}
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical);
 public:
	static DependencyProperty* FillRuleProperty;
	static DependencyProperty* PointsProperty;

	Polyline () { };
	virtual Type::Kind GetObjectType () { return Type::POLYLINE; };

	// Polyline has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual void BuildPath ();

	virtual bool CanFill () { return true; }
	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual FillRule GetFillRule ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);
};

Polyline       *polyline_new		(void);
FillRule	polyline_get_fill_rule	(Polyline *polyline);
void		polyline_set_fill_rule	(Polyline *polyline, FillRule value);
Point	       *polyline_get_points	(Polyline *polyline, int *count);
void		polyline_set_points	(Polyline *polyline, Point* points, int count);


//
// Path
//
class Path : public Shape {
 protected:
	virtual ~Path () {}
	virtual bool SetupLine (cairo_t* cr);
	virtual bool DrawShape (cairo_t *cr, bool do_op);
	virtual Rect ComputeShapeBounds (bool logical);

 public:
	static DependencyProperty* DataProperty;

	Path () {};

	virtual Type::Kind GetObjectType () { return Type::PATH; };

	// Path has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual void Draw (cairo_t *cr);

	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual bool CanFill () { return true; }
	virtual FillRule GetFillRule ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

Path *path_new (void);
Geometry* path_get_data (Path *path);
void path_set_data (Path *path, Geometry *value);

void shape_init (void);

G_END_DECLS

#endif
