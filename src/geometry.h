#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>
#include "runtime.h"

//
// Geometry
//
class Geometry {
 public:
	FillRule fill_rule;
	Transform *transform;

	Geometry () : fill_rule (FillRuleEvenOdd) {};
};

//
// GeometryGroup
//
class GeometryGroup : public Geometry {
 public:
	// GeometryCollection

	GeometryGroup () {};
};
GeometryGroup* geometry_group_new ();

//
// EllipseGeometry
//
class EllipseGeometry : public Geometry {
 public:
	Point center;
	double radius_x, radius_y;

	EllipseGeometry () {};
};
EllipseGeometry* ellipse_geometry_new ();

//
// LineGeometry
//
class LineGeometry : public Geometry {
 public:
	Point end, start;

	LineGeometry () {};
};
LineGeometry* line_geometry_new ();

//
// PathGeometry
//
class PathGeometry : public Geometry {
 public:
	// PathFigureCollection

	PathGeometry () {};
};
PathGeometry* path_geometry_new ();

//
// RectangleGeometry
//
class RectangleGeometry : public Geometry {
 public:
	double radius_x, radius_y;
	Rect *rect;

	RectangleGeometry () : radius_x (0), radius_y (0) {};
};
RectangleGeometry* rectangle_geometry_new ();


G_END_DECLS

#endif
