#ifndef __SHAPE_H__
#define __SHAPE_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "runtime.h"
#include "geometry.h"

// http://graphics.stanford.edu/courses/cs248-98-fall/Final/q1.html
#define ARC_TO_BEZIER	0.55228475

//
// Shape class 
// 
class Shape : public FrameworkElement {
	void DoDraw (Surface *s, bool do_op);
 public: 
	double *stroke_dash_array;
	int stroke_dash_array_count;

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

	Shape () : stroke_dash_array (NULL), stroke_dash_array_count (0)
	{
		SetObjectType (DependencyObject::SHAPE);
	}

	//
	// Overrides from UIElement.
	//
	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();

	//
	// new virtual methods for shapes
	//
	
	//
	// Draw: draws the Shape on the surface (affine transforms are set before this
	// is called). 
	//
	// This is called multiple times: one for fills, one for strokes
	// if they are both set.   It will also be called to compute the bounding box.
	//
	virtual void Draw (Surface *s) = 0;
};

Brush		*shape_get_fill			(Shape *shape);
void		shape_set_fill			(Shape *shape, Brush *brush);
Brush		*shape_get_stroke		(Shape *shape);
void		shape_set_stroke		(Shape *shape, Brush *brush);
Stretch		shape_get_stretch		(Shape *shape);
void		shape_set_stretch		(Shape *shape, Stretch stretch);
PenLineCap	shape_get_stroke_dash_cap	(Shape *shape);
void		shape_set_stroke_dash_cap	(Shape *shape, PenLineCap cap);
PenLineCap	shape_get_stroke_start_line_cap	(Shape *shape);
void		shape_set_stroke_start_line_cap	(Shape *shape, PenLineCap cap);
PenLineCap	shape_get_stroke_end_line_cap	(Shape *shape);
void		shape_set_stroke_end_line_cap	(Shape *shape, PenLineCap cap);
double		shape_get_stroke_dash_offset	(Shape *shape);
void		shape_set_stroke_dash_offset	(Shape *shape, double offset);
double		shape_get_stroke_miter_limit	(Shape *shape);
void		shape_set_stroke_miter_limit	(Shape *shape, double limit);
double		shape_get_stroke_thickness	(Shape *shape);
void		shape_set_stroke_thickness	(Shape *shape, double thickness);
PenLineJoin	shape_get_stroke_line_join	(Shape *shape);
void		shape_set_stroke_line_join	(Shape *shape, PenLineJoin join);
void		shape_set_stroke_dash_array	(Shape *shape, double* dashes, int count);

//
// Ellipse
//
class Ellipse : public Shape {
 public:
	Ellipse () { SetObjectType (DependencyObject::ELLIPSE); };

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

	Rectangle () { SetObjectType (DependencyObject::RECTANGLE); };

	void Draw (Surface *s);

	virtual Point getxformorigin ();
};
Rectangle *rectangle_new          ();
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

	Line () { SetObjectType(DependencyObject::LINE); };
	
	void Draw (Surface *s);

	virtual Point getxformorigin ();
};
Line *line_new  ();
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
	FillRule fill_rule;
	Point *points;
	int count;

	static DependencyProperty* FillRuleProperty;
	static DependencyProperty* PointsProperty;

	Polygon () : fill_rule (FillRuleEvenOdd), points (NULL), count (0) { SetObjectType (DependencyObject::POLYGON); };

	void Draw (Surface *s);
};
Polygon *polygon_new ();
FillRule polygon_get_fill_rule (Polygon *polygon);
void polygon_set_fill_rule (Polygon *polygon, FillRule fill_rule);
void polygon_set_points (Polygon *polygon, Point* points, int count);

//
// Polyline
//
class Polyline : public Shape {
 public:
	FillRule fill_rule;
	Point *points;
	int count;

	static DependencyProperty* FillRuleProperty;
	static DependencyProperty* PointsProperty;

	Polyline () : fill_rule (FillRuleEvenOdd), points (NULL), count (0) { SetObjectType (DependencyObject::POLYLINE); };

	void Draw (Surface *s);
};
Polyline *polyline_new ();
FillRule polyline_get_fill_rule (Polyline *polyline);
void polyline_set_fill_rule (Polyline *polyline, FillRule fill_rule);
void polyline_set_points (Polyline *polyline, Point* points, int count);

//
// Path
//
class Path : public Shape {
 public:
	static DependencyProperty* DataProperty;

	Path () {};

	void Draw (Surface *s);
};
Path *path_new ();
Geometry* path_get_data (Path *path);
void path_set_data (Path *path, Geometry* data);


G_END_DECLS

#endif
