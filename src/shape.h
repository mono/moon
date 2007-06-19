#ifndef __SHAPE_H__
#define __SHAPE_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "runtime.h"
#include "brush.h"
#include "geometry.h"

// http://graphics.stanford.edu/courses/cs248-98-fall/Final/q1.html
#define ARC_TO_BEZIER	0.55228475

//
// Helpers
//

cairo_fill_rule_t convert_fill_rule (FillRule fill_rule);

void moon_ellipse (cairo_t *cr, double x, double y, double w, double h);

void moon_rounded_rectangle (cairo_t *cr, double x, double y, double w, double h, double radius_x, double radius_y);

//
// Shape class 
// 
class Shape : public FrameworkElement {
	void DoDraw (Surface *s, bool do_op, bool consider_fill);
	Brush *stroke, *fill;
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
	~Shape ();
	virtual Type::Kind GetObjectType () { return Type::SHAPE; };

	//
	// Overrides from UIElement.
	//
	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual bool inside_object (Surface *s, double x, double y);
	
	//
	// new virtual methods for shapes
	//
	
	virtual bool CanFill () { return true; }

	//
	// Draw: draws the Shape on the surface (affine transforms are set before this
	// is called). 
	//
	// This is called multiple times: one for fills, one for strokes
	// if they are both set.   It will also be called to compute the bounding box.
	//
	virtual void Draw (Surface *s) = 0;

	virtual void OnPropertyChanged (DependencyProperty *prop);
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
	Ellipse () { };
	virtual Type::Kind GetObjectType () { return Type::ELLIPSE; };

	virtual Point getxformorigin ();

	void Draw (Surface *s);
};
Ellipse *ellipse_new ();

//
// Rectangle class 
// 
class Rectangle : public Shape {
 public:
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	Rectangle () { };
	virtual Type::Kind GetObjectType () { return Type::RECTANGLE; };

	void Draw (Surface *s);

	virtual Point getxformorigin ();

	virtual void OnPropertyChanged (DependencyProperty *prop);
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

	Line () { };
	virtual Type::Kind GetObjectType () { return Type::LINE; };
	
	void Draw (Surface *s);

	// Line has no center to compute, it's always 0,0 because it provides it's own start and end
	// virtual Point getxformorigin ();

	virtual bool CanFill () { return false; }
	virtual void OnPropertyChanged (DependencyProperty *prop);
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
	// virtual Point getxformorigin ();

	void Draw (Surface *s);
	virtual void OnPropertyChanged (DependencyProperty *prop);
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
	// virtual Point getxformorigin ();

	void Draw (Surface *s);

	virtual void OnPropertyChanged (DependencyProperty *prop);
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
	// virtual Point getxformorigin ();

	void Draw (Surface *s);

	virtual bool CanFill ();
	virtual void OnPropertyChanged (DependencyProperty *prop);
};

Path *path_new (void);
Geometry* path_get_data (Path *path);
void path_set_data (Path *path, Geometry *value);


G_END_DECLS

#endif
