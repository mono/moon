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
	Brush *fill, *stroke;
	Stretch stretch;
	PenLineCap stroke_dash_cap, stroke_start_line_cap, stroke_end_line_cap;
	double stroke_dash_offset, stroke_miter_limit, stroke_thickness;
	PenLineJoin stroke_line_join;
	double *stroke_dash_array;

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

	Shape () : fill (NULL), stroke (NULL), stretch (StretchFill), stroke_dash_cap (PenLineCapFlat), 
		stroke_dash_offset (0), stroke_end_line_cap (PenLineCapFlat), stroke_line_join (PenLineJoinMiter),
		stroke_miter_limit (0), stroke_start_line_cap (PenLineCapFlat), stroke_thickness (1), 
		stroke_dash_array (NULL) { SetObjectType (DependencyObject::SHAPE); }

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

	virtual void set_prop_from_str (const char *prop, const char *value);

	virtual Point getxformorigin ();
};
Rectangle *rectangle_new  (double x, double y, double w, double h);
double rectangle_get_radius_x (Rectangle *rectangle);
void rectangle_set_radius_x (Rectangle *rectangle, double value);
double rectangle_get_radius_y (Rectangle *rectangle);
void rectangle_set_radius_y (Rectangle *rectangle, double value);

//
// Line class 
// 
class Line : public Shape {
 public:
	double line_x1, line_y1, line_x2, line_y2;

	static DependencyProperty* X1Property;
	static DependencyProperty* Y1Property;
	static DependencyProperty* X2Property;
	static DependencyProperty* Y2Property;

	Line () : line_x1(0), line_y1(0), line_x2(0), line_y2(0) { SetObjectType(DependencyObject::LINE); };
	
	void Draw (Surface *s);

	virtual void set_prop_from_str (const char *prop, const char *value);

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
