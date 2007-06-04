G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>
#include "runtime.h"

class Transform : public DependencyObject {
 protected:
 	Transform () { }

 public:
	virtual void GetValue (cairo_matrix_t *value) = 0;
};

void   transform_get_value (Transform *t, cairo_matrix_t *value);

class RotateTransform : public Transform {
 public:

	RotateTransform () { }

	/* these are dependency properties
	   double angle;
	   double center_x;
	   double center_y;
	*/

	virtual void GetValue (cairo_matrix_t *value);
};

void   rotate_transform_set_angle (RotateTransform *t, double angle);
double rotate_transform_get_angle (RotateTransform *t);

void   rotate_transform_set_center_x (RotateTransform *t, double centerX);
double rotate_transform_get_center_x (RotateTransform *t);

void   rotate_transform_set_center_y (RotateTransform *t, double centerY);
double rotate_transform_get_center_y (RotateTransform *t);

class TranslateTransform : public Transform {
 public:

	TranslateTransform () { }

	/* these are dependency properties
	   double x;
	   double y;
	*/

	virtual void GetValue (cairo_matrix_t *value);
};

void   translate_transform_set_x (RotateTransform *t, double x);
double translate_transform_get_x (RotateTransform *t);

void   translate_transform_set_y (RotateTransform *t, double y);
double translate_transform_get_y (RotateTransform *t);


class ScaleTransform : public Transform {
public:

	ScaleTransform () { }

	/* these are dependency properties
	   double scale_x;
	   double scale_y;
	   double center_x;
	   double center_y;
	*/

	virtual void GetValue (cairo_matrix_t *value);
};

void   scale_transform_set_scale_x (ScaleTransform *t, double scaleX);
double scale_transform_get_scale_x (ScaleTransform *t);

void   scale_transform_set_scale_y (ScaleTransform *t, double scaleY);
double scale_transform_get_scale_y (ScaleTransform *t);

void   scale_transform_set_center_x (ScaleTransform *t, double centerX);
double scale_transform_get_center_x (ScaleTransform *t);

void   scale_transform_set_center_y (ScaleTransform *t, double centerY);
double scale_transform_get_center_y (ScaleTransform *t);


class MatrixTransform : public Transform {
 public:

	MatrixTransform () { }

	/* these are dependency properties
	   Matrix matrix;
	*/

	virtual void GetValue (cairo_matrix_t *value);
};

void           matrix_transform_set_matrix (MatrixTransform *t, cairo_matrix_t matrix);
cairo_matrix_t matrix_transform_get_matrix (MatrixTransform *t);

G_END_DECLS
