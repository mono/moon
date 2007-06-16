#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>
#include "runtime.h"

class Transform : public DependencyObject {
 public:
 	Transform () { }
	virtual Value::Kind GetObjectType () { return Value::TRANSFORM; };
	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void GetTransform (cairo_matrix_t *value);
};

Transform* transform_new ();
void   transform_get_transform (Transform *t, cairo_matrix_t *value);

class RotateTransform : public Transform {
 public:

	RotateTransform () { }
	virtual Value::Kind GetObjectType () { return Value::ROTATETRANSFORM; };

	static DependencyProperty* AngleProperty;
	static DependencyProperty* CenterXProperty;
	static DependencyProperty* CenterYProperty;

	virtual void GetTransform (cairo_matrix_t *value);
};

RotateTransform * rotate_transform_new ();

void   rotate_transform_set_angle (RotateTransform *t, double angle);
double rotate_transform_get_angle (RotateTransform *t);

void   rotate_transform_set_center_x (RotateTransform *t, double centerX);
double rotate_transform_get_center_x (RotateTransform *t);

void   rotate_transform_set_center_y (RotateTransform *t, double centerY);
double rotate_transform_get_center_y (RotateTransform *t);

class TranslateTransform : public Transform {
 public:

	TranslateTransform () {  }
	virtual Value::Kind GetObjectType () { return Value::TRANSLATETRANSFORM; };

	static DependencyProperty* XProperty;
	static DependencyProperty* YProperty;

	virtual void GetTransform (cairo_matrix_t *value);
};

TranslateTransform *translate_transform_new ();
void   translate_transform_set_x (TranslateTransform *t, double x);
double translate_transform_get_x (TranslateTransform *t);

void   translate_transform_set_y (TranslateTransform *t, double y);
double translate_transform_get_y (TranslateTransform *t);


class ScaleTransform : public Transform {
public:

	ScaleTransform () {  }
	virtual Value::Kind GetObjectType () { return Value::SCALETRANSFORM; };

	static DependencyProperty* ScaleXProperty;
	static DependencyProperty* ScaleYProperty;
	static DependencyProperty* CenterXProperty;
	static DependencyProperty* CenterYProperty;

	virtual void GetTransform (cairo_matrix_t *value);
};

ScaleTransform * scale_transform_new ();
void   scale_transform_set_scale_x (ScaleTransform *t, double scaleX);
double scale_transform_get_scale_x (ScaleTransform *t);

void   scale_transform_set_scale_y (ScaleTransform *t, double scaleY);
double scale_transform_get_scale_y (ScaleTransform *t);

void   scale_transform_set_center_x (ScaleTransform *t, double centerX);
double scale_transform_get_center_x (ScaleTransform *t);

void   scale_transform_set_center_y (ScaleTransform *t, double centerY);
double scale_transform_get_center_y (ScaleTransform *t);


class SkewTransform : public Transform {
public:

	SkewTransform () {  }
	virtual Value::Kind GetObjectType () { return Value::SKEWTRANSFORM; };

	static DependencyProperty* AngleXProperty;
	static DependencyProperty* AngleYProperty;
	static DependencyProperty* CenterXProperty;
	static DependencyProperty* CenterYProperty;

	virtual void GetTransform (cairo_matrix_t *value);
};

SkewTransform * skew_transform_new ();
void   skew_transform_set_angle_x (SkewTransform *t, double angleX);
double skew_transform_get_angle_x (SkewTransform *t);

void   skew_transform_set_angle_y (SkewTransform *t, double angleY);
double skew_transform_get_angle_y (SkewTransform *t);

void   skew_transform_set_center_x (SkewTransform *t, double centerX);
double skew_transform_get_center_x (SkewTransform *t);

void   skew_transform_set_center_y (SkewTransform *t, double centerY);
double skew_transform_get_center_y (SkewTransform *t);

class MatrixTransform : public Transform {
 public:
	static DependencyProperty* MatrixProperty;

	MatrixTransform () { }
	virtual Value::Kind GetObjectType () { return Value::MATRIXTRANSFORM; };

	/* these are dependency properties
	   Matrix matrix;
	*/

	virtual void GetTransform (cairo_matrix_t *value);
};

MatrixTransform *matrix_transform_new ();
void	matrix_transform_set_matrix (MatrixTransform *t, Matrix* matrix);
Matrix*	matrix_transform_get_matrix (MatrixTransform *t);


class TransformCollection : public Collection {
 public:
	TransformCollection () {}
	virtual Value::Kind GetObjectType () { return Value::TRANSFORM_COLLECTION; }
};
TransformCollection* transform_collection_new ();

class TransformGroup : public Transform {
public:
	static DependencyProperty* ChildrenProperty;

	TransformGroup ();
	~TransformGroup ();
	virtual Value::Kind GetObjectType() { return Value::TRANSFORMGROUP; };

	
	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void GetTransform (cairo_matrix_t *value);
};

TransformGroup *transform_group_new ();


G_END_DECLS

#endif
