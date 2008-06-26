/*
 * transform.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <glib.h>

G_BEGIN_DECLS

#include <cairo.h>
#include "collection.h"

class Transform : public DependencyObject {
 protected:
	cairo_matrix_t _matrix;
	bool need_update;
	
	virtual ~Transform () {}

 public:
	Transform () : need_update (true) { }
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual Type::Kind GetObjectType () { return Type::TRANSFORM; };
	virtual void GetTransform (cairo_matrix_t *value) {
		if (need_update) {
			UpdateTransform ();
			need_update = false;
		}
		*value = _matrix;
	};

	virtual void UpdateTransform ();
};

Transform *transform_new (void);
void   transform_get_transform (Transform *transform, cairo_matrix_t *value);


class RotateTransform : public Transform {
 protected:
	virtual ~RotateTransform () {}
	
 public:
	static DependencyProperty *AngleProperty;
	static DependencyProperty *CenterXProperty;
	static DependencyProperty *CenterYProperty;
	
	RotateTransform () { }
	virtual Type::Kind GetObjectType () { return Type::ROTATETRANSFORM; };
	virtual void UpdateTransform ();
	
	//
	// Property Accessors
	//
	void SetAngle (double angle);
	double GetAngle ();
	
	void SetCenterX (double centerX);
	double GetCenterX ();
	
	void SetCenterY (double centerY);
	double GetCenterY ();
};

RotateTransform *rotate_transform_new (void);

void   rotate_transform_set_angle (RotateTransform *transform, double angle);
double rotate_transform_get_angle (RotateTransform *transform);

void   rotate_transform_set_center_x (RotateTransform *transform, double centerX);
double rotate_transform_get_center_x (RotateTransform *transform);

void   rotate_transform_set_center_y (RotateTransform *transform, double centerY);
double rotate_transform_get_center_y (RotateTransform *transform);


class TranslateTransform : public Transform {
 protected:
	virtual ~TranslateTransform () {}
	
 public:
	static DependencyProperty *XProperty;
	static DependencyProperty *YProperty;
	
	TranslateTransform () {  }
	virtual Type::Kind GetObjectType () { return Type::TRANSLATETRANSFORM; };
	virtual void UpdateTransform ();
	
	//
	// Property Accessors
	//
	void SetX (double x);
	double GetX ();
	
	void SetY (double y);
	double GetY ();
};

TranslateTransform *translate_transform_new (void);
void   translate_transform_set_x (TranslateTransform *transform, double x);
double translate_transform_get_x (TranslateTransform *transform);

void   translate_transform_set_y (TranslateTransform *transform, double y);
double translate_transform_get_y (TranslateTransform *transform);


class ScaleTransform : public Transform {
 protected:
	virtual ~ScaleTransform () {}

 public:

	ScaleTransform () {  }
	virtual Type::Kind GetObjectType () { return Type::SCALETRANSFORM; };
	virtual void UpdateTransform ();

	static DependencyProperty* ScaleXProperty;
	static DependencyProperty* ScaleYProperty;
	static DependencyProperty* CenterXProperty;
	static DependencyProperty* CenterYProperty;
};

ScaleTransform *scale_transform_new (void);
void   scale_transform_set_scale_x (ScaleTransform *transform, double scaleX);
double scale_transform_get_scale_x (ScaleTransform *transform);

void   scale_transform_set_scale_y (ScaleTransform *transform, double scaleY);
double scale_transform_get_scale_y (ScaleTransform *transform);

void   scale_transform_set_center_x (ScaleTransform *transform, double centerX);
double scale_transform_get_center_x (ScaleTransform *transform);

void   scale_transform_set_center_y (ScaleTransform *transform, double centerY);
double scale_transform_get_center_y (ScaleTransform *transform);


class SkewTransform : public Transform {
 protected:
	virtual ~SkewTransform () {}

public:

	SkewTransform () {  }
	virtual Type::Kind GetObjectType () { return Type::SKEWTRANSFORM; };
	virtual void UpdateTransform ();

	static DependencyProperty* AngleXProperty;
	static DependencyProperty* AngleYProperty;
	static DependencyProperty* CenterXProperty;
	static DependencyProperty* CenterYProperty;
};

SkewTransform *skew_transform_new (void);
void   skew_transform_set_angle_x (SkewTransform *transform, double angleX);
double skew_transform_get_angle_x (SkewTransform *transform);

void   skew_transform_set_angle_y (SkewTransform *transform, double angleY);
double skew_transform_get_angle_y (SkewTransform *transform);

void   skew_transform_set_center_x (SkewTransform *transform, double centerX);
double skew_transform_get_center_x (SkewTransform *transform);

void   skew_transform_set_center_y (SkewTransform *transform, double centerY);
double skew_transform_get_center_y (SkewTransform *transform);

class Matrix : public DependencyObject {
private:
	cairo_matrix_t matrix;

protected:
	virtual ~Matrix () {}

public:
	static DependencyProperty *M11Property;
	static DependencyProperty *M12Property;
	static DependencyProperty *M21Property;
	static DependencyProperty *M22Property;
	static DependencyProperty *OffsetXProperty;
	static DependencyProperty *OffsetYProperty;

	Matrix ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual Type::Kind GetObjectType () { return Type::MATRIX; }

	cairo_matrix_t GetUnderlyingMatrix ();
};

Matrix *matrix_new (void);
double matrix_get_m11 (Matrix *matrix);
void matrix_set_m11 (Matrix *matrix, double value);
double matrix_get_m12 (Matrix *matrix);
void matrix_set_m12 (Matrix *matrix, double value);
double matrix_get_m21 (Matrix *matrix);
void matrix_set_m21 (Matrix *matrix, double value);
double matrix_get_m22 (Matrix *matrix);
void matrix_set_m22 (Matrix *matrix, double value);
double matrix_get_offset_x (Matrix *matrix);
void matrix_set_offset_x (Matrix *matrix, double value);
double matrix_get_offset_y (Matrix *matrix);
void matrix_set_offset_y (Matrix *matrix, double value);

class MatrixTransform : public Transform {
 protected:
	virtual ~MatrixTransform () {}

 public:
	static DependencyProperty* MatrixProperty;

	MatrixTransform () {}
	virtual Type::Kind GetObjectType () { return Type::MATRIXTRANSFORM; };

	/* these are dependency properties
	   Matrix matrix;
	*/
	virtual void UpdateTransform ();
};

MatrixTransform *matrix_transform_new (void);
void	matrix_transform_set_matrix (MatrixTransform *transform, Matrix* matrix);
Matrix*	matrix_transform_get_matrix (MatrixTransform *transform);


class TransformCollection : public Collection {
 protected:
	virtual ~TransformCollection () {}

 public:
	TransformCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TRANSFORM_COLLECTION; }
};
TransformCollection* transform_collection_new ();

/* @ContentProperty="Children" */
class TransformGroup : public Transform {
protected:
	virtual ~TransformGroup ();

public:
	static DependencyProperty* ChildrenProperty;

	TransformGroup ();
	virtual Type::Kind GetObjectType() { return Type::TRANSFORMGROUP; };

	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void UpdateTransform ();
};

TransformGroup *transform_group_new (void);

void transform_init (void);

G_END_DECLS

#endif
