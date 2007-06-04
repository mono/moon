#ifndef __SHAPE_H__
#define __SHAPE_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "runtime.h"

//
// Shape class 
// 
class Shape : public FrameworkElement {
	void DoDraw (Surface *s, bool do_op);
 public: 
	Brush *fill, *stroke;
	Stretch stretch;
	PenLineCap stroke_dash_cap, stroke_start_line_cap, stroke_end_line_cap;
	double stroke_dash_offset, stroke_miter_limit, stroke_thickness;
	PenLineJoin stroke_line_join;
	double *stroke_dash_array;

	Shape () : fill (NULL), stroke (NULL), stretch (StretchFill), stroke_dash_cap (PenLineCapFlat), 
		stroke_dash_offset (0), stroke_end_line_cap (PenLineCapFlat), stroke_line_join (PenLineJoinMiter),
		stroke_miter_limit (0), stroke_start_line_cap (PenLineCapFlat), stroke_thickness (1), 
		stroke_dash_array (NULL) {}

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

	virtual void set_prop_from_str (const char *prop, const char *value);
};

void shape_set_fill   (Shape *shape, Brush *brush);
void shape_set_stroke (Shape *shape, Brush *brush);
void shape_set_stretch (Shape *shape, Stretch stretch);
void shape_set_stroke_dash_cap (Shape *shape, PenLineCap cap);
void shape_set_stroke_start_line_cap (Shape *shape, PenLineCap cap);
void shape_set_stroke_end_line_cap (Shape *shape, PenLineCap cap);
void shape_set_stroke_dash_offset (Shape *shape, double offset);
void shape_set_stroke_miter_limit (Shape *shape, double limit);
void shape_set_stroke_thickness (Shape *shape, double thickness);
void shape_set_stroke_line_join (Shape *shape, PenLineJoin join);
void shape_set_stroke_dash_array (Shape *shape, double* dashes);

//
// Ellipse
//
class Ellipse : public Shape {
 public:
	Ellipse () {};

	void Draw (Surface *s);
};

//
// Rectangle class 
// 
class Rectangle : public Shape {
 public:
	double radius_x, radius_y;	// for rounded-corner rectangles

	Rectangle () : radius_x(0), radius_y(0) {};

	void Draw (Surface *s);

	virtual void set_prop_from_str (const char *prop, const char *value);

	virtual Point getxformorigin ();
};
Rectangle *rectangle_new  (double x, double y, double w, double h);

//
// Line class 
// 
class Line : public Shape {
 public:
	double line_x1, line_y1, line_x2, line_y2;

	Line (double px1, double py1, double px2, double py2) :
		line_x1(px1), line_y1(py1), line_x2(px2), line_y2(py2) {};
	
	void Draw (Surface *s);

	virtual void set_prop_from_str (const char *prop, const char *value);

	virtual Point getxformorigin ();
};
Line *line_new  (double x1, double y1, double x2, double y2);

//
// Polygon
//
class Polygon : public Shape {
 public:
	FillRule fill_rule;
	Point *points;
	int count;

	Polygon () : fill_rule (FillRuleEvenOdd), points (NULL), count (0) {};
};
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

	Polyline () : fill_rule (FillRuleEvenOdd), points (NULL), count (0) {};
};
void polyline_set_fill_rule (Polyline *polyline, FillRule fill_rule);
void polyline_set_points (Polyline *polyline, Point* points, int count);

G_END_DECLS

#endif
