/*
 * shape.h: This match the classes inside System.Windows.Shapes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
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


//
// Shape class 
// 
class Shape : public FrameworkElement {
 protected:
	Brush *stroke, *fill;
	void DoDraw (cairo_t *cr, bool do_op, bool consider_fill);

	moon_path *path;
	virtual void InvalidatePathCache (bool free = false);
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
	virtual ~Shape ();
	virtual Type::Kind GetObjectType () { return Type::SHAPE; };

	//
	// Overrides from UIElement.
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetTransformOrigin ();
	
	//
	// new virtual methods for shapes
	//
	
	virtual bool IsFilled () { return fill; }
	virtual bool IsStroked () { return stroke; }
	virtual bool CanFill () { return false; }
	virtual FillRule GetFillRule () { return FillRuleNonzero; }

	// Stroke[Start|End]LineCap properties are ignored for some shapes
	// e.g. Ellipse, Rectangle, Polygon and (closed) Path
	virtual bool NeedsLineCaps () { return true; }

	// StrokeLineJoin & StrokeMiterLimit properties are ignored for some shapes
	// e.g. Ellipse, Rectangle (with rounded corners), Line
	virtual bool NeedsLineJoin () { return true; }

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
	bool NeedsClipping ();
	bool MixedHeightWidth (Value **width, Value **height);

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop);

	//
	bool IsEmpty () { return (flags & UIElement::SHAPE_EMPTY); };
	bool IsNormal () { return (flags & UIElement::SHAPE_NORMAL); };
	bool IsDegenerate () { return (flags & UIElement::SHAPE_DEGENERATE); };
	void SetShapeFlags (UIElementFlags sf) { flags &= ~UIElement::SHAPE_MASK; flags |= sf; };
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
 public:
	Ellipse ();
	virtual Type::Kind GetObjectType () { return Type::ELLIPSE; };

	virtual void BuildPath ();
	virtual bool CanFill () { return true; }
	virtual bool NeedsLineCaps () { return false; }
	virtual bool NeedsLineJoin () { return false; }

	virtual void OnPropertyChanged (DependencyProperty *prop);
};

Ellipse *ellipse_new (void);


//
// Rectangle class 
// 
class Rectangle : public Shape {
 public:
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	Rectangle ();
	virtual Type::Kind GetObjectType () { return Type::RECTANGLE; };

	virtual void BuildPath ();

	virtual void OnPropertyChanged (DependencyProperty *prop);

	bool GetRadius (double *rx, double *ry);
	virtual bool CanFill () { return true; }
	virtual bool NeedsLineCaps () { return false; }
	// technically we could override NeedsLineJoin to check for round corners
	// but that would be just as expensive as setting the unneeded values
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
 public:
	static DependencyProperty* X1Property;
	static DependencyProperty* Y1Property;
	static DependencyProperty* X2Property;
	static DependencyProperty* Y2Property;

	virtual Type::Kind GetObjectType () { return Type::LINE; };
	
	virtual void BuildPath ();
	virtual void ComputeBounds ();
	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual void OnPropertyChanged (DependencyProperty *prop);

	// Line has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual bool IsFilled () { return false; }
	virtual bool NeedsLineJoin () { return false; }
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
	virtual bool NeedsLineCaps () { return false; }

	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop);
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
 public:
	static DependencyProperty* FillRuleProperty;
	static DependencyProperty* PointsProperty;

	Polyline () { };
	virtual Type::Kind GetObjectType () { return Type::POLYLINE; };

	// Polyline has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual void BuildPath ();

	virtual bool CanFill () { return true; }
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual FillRule GetFillRule ();

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop);
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
 public:
	static DependencyProperty* DataProperty;

	Path () {};

	virtual Type::Kind GetObjectType () { return Type::PATH; };

	// Path has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point GetTransformOrigin ();

	virtual void Draw (cairo_t *cr);

	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool ClipOnHeightAndWidth () { return true; }

	virtual bool CanFill () { return true; }
	virtual FillRule GetFillRule ();

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop);
};

Path *path_new (void);
Geometry* path_get_data (Path *path);
void path_set_data (Path *path, Geometry *value);

void shape_init (void);

G_END_DECLS

#endif
