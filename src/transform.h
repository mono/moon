/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

/* @Namespace=System.Windows.Media */
class GeneralTransform : public DependencyObject {
 protected:
	cairo_matrix_t _matrix;
	bool need_update;
	
	virtual ~GeneralTransform () {};
	
	virtual void UpdateTransform ();
	void MaybeUpdateTransform ();
	
 public:
	/* @GenerateCBinding */
	GeneralTransform () : need_update (true) { }
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual Type::Kind GetObjectType () { return Type::GENERALTRANSFORM; };
	
	virtual void GetTransform (cairo_matrix_t *value);
	
	Point Transform (Point point);
};

void   general_transform_get_transform (GeneralTransform *transform, cairo_matrix_t *value);
void   general_transform_transform_point (GeneralTransform *t, Point *p, Point *r);


/* @Namespace=System.Windows.Media */
class Transform : public GeneralTransform {
protected:
	virtual ~Transform () {}

public:
	/* @GenerateCBinding */
	Transform () { }

	virtual Type::Kind GetObjectType () { return Type::TRANSFORM; };
};


/* @Namespace=System.Windows.Media */
class RotateTransform : public Transform {
 protected:
	virtual ~RotateTransform () {}
	virtual void UpdateTransform ();
	
 public:
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *AngleProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterXProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterYProperty;
	
	/* @GenerateCBinding */
	RotateTransform () { }
	virtual Type::Kind GetObjectType () { return Type::ROTATETRANSFORM; };
	
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

void   rotate_transform_set_angle (RotateTransform *transform, double angle);
double rotate_transform_get_angle (RotateTransform *transform);

void   rotate_transform_set_center_x (RotateTransform *transform, double centerX);
double rotate_transform_get_center_x (RotateTransform *transform);

void   rotate_transform_set_center_y (RotateTransform *transform, double centerY);
double rotate_transform_get_center_y (RotateTransform *transform);


/* @Namespace=System.Windows.Media */
class TranslateTransform : public Transform {
 protected:
	virtual ~TranslateTransform () {}
	virtual void UpdateTransform ();
	
 public:
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *XProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *YProperty;
	
	/* @GenerateCBinding */
	TranslateTransform () {  }
	virtual Type::Kind GetObjectType () { return Type::TRANSLATETRANSFORM; };
	
	//
	// Property Accessors
	//
	void SetX (double x);
	double GetX ();
	
	void SetY (double y);
	double GetY ();
};

void   translate_transform_set_x (TranslateTransform *transform, double x);
double translate_transform_get_x (TranslateTransform *transform);

void   translate_transform_set_y (TranslateTransform *transform, double y);
double translate_transform_get_y (TranslateTransform *transform);


/* @Namespace=System.Windows.Media */
class ScaleTransform : public Transform {
 protected:
	virtual ~ScaleTransform () {}
	virtual void UpdateTransform ();

 public:

	/* @GenerateCBinding */
	ScaleTransform () {  }
	virtual Type::Kind GetObjectType () { return Type::SCALETRANSFORM; };

 	/* @PropertyType=double,DefaultValue=1.0 */
	static DependencyProperty *ScaleXProperty;
 	/* @PropertyType=double,DefaultValue=1.0 */
	static DependencyProperty *ScaleYProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterXProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterYProperty;
};

void   scale_transform_set_scale_x (ScaleTransform *transform, double scaleX);
double scale_transform_get_scale_x (ScaleTransform *transform);

void   scale_transform_set_scale_y (ScaleTransform *transform, double scaleY);
double scale_transform_get_scale_y (ScaleTransform *transform);

void   scale_transform_set_center_x (ScaleTransform *transform, double centerX);
double scale_transform_get_center_x (ScaleTransform *transform);

void   scale_transform_set_center_y (ScaleTransform *transform, double centerY);
double scale_transform_get_center_y (ScaleTransform *transform);


/* @Namespace=System.Windows.Media */
class SkewTransform : public Transform {
 protected:
	virtual ~SkewTransform () {}
	virtual void UpdateTransform ();

 public:
	
	/* @GenerateCBinding */
	SkewTransform () {  }
	virtual Type::Kind GetObjectType () { return Type::SKEWTRANSFORM; };

 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *AngleXProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *AngleYProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterXProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterYProperty;
};

void   skew_transform_set_angle_x (SkewTransform *transform, double angleX);
double skew_transform_get_angle_x (SkewTransform *transform);

void   skew_transform_set_angle_y (SkewTransform *transform, double angleY);
double skew_transform_get_angle_y (SkewTransform *transform);

void   skew_transform_set_center_x (SkewTransform *transform, double centerX);
double skew_transform_get_center_x (SkewTransform *transform);

void   skew_transform_set_center_y (SkewTransform *transform, double centerY);
double skew_transform_get_center_y (SkewTransform *transform);

/* @Namespace=None */
class Matrix : public DependencyObject {
private:
	cairo_matrix_t matrix;

protected:
	virtual ~Matrix () {}

public:
 	/* @PropertyType=double,DefaultValue=1.0 */
	static DependencyProperty *M11Property;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *M12Property;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *M21Property;
 	/* @PropertyType=double,DefaultValue=1.0 */
	static DependencyProperty *M22Property;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *OffsetXProperty;
 	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *OffsetYProperty;

	/* @GenerateCBinding */
	Matrix ();
	Matrix (cairo_matrix_t *m);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual Type::Kind GetObjectType () { return Type::MATRIX; }

	cairo_matrix_t GetUnderlyingMatrix ();
};

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


/* @Namespace=System.Windows.Media */
class MatrixTransform : public Transform {
 protected:
	virtual ~MatrixTransform () {}

	virtual void UpdateTransform ();
 public:
 	/* @PropertyType=Matrix */
	static DependencyProperty *MatrixProperty;

	/* @GenerateCBinding */
	MatrixTransform () {}
	virtual Type::Kind GetObjectType () { return Type::MATRIXTRANSFORM; };

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

void	matrix_transform_set_matrix (MatrixTransform *transform, Matrix *matrix);
Matrix *matrix_transform_get_matrix (MatrixTransform *transform);


/* @Namespace=System.Windows.Media */
class TransformCollection : public DependencyObjectCollection {
 protected:
	virtual ~TransformCollection () {}

 public:
	/* @GenerateCBinding */
	TransformCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::TRANSFORM_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::TRANSFORM; }
};


/* @ContentProperty="Children" */
/* @Namespace=System.Windows.Media */
class TransformGroup : public Transform {
protected:
	virtual ~TransformGroup ();

	virtual void UpdateTransform ();
public:
 	/* @PropertyType=TransformCollection */
	static DependencyProperty *ChildrenProperty;

	/* @GenerateCBinding */
	TransformGroup ();
	virtual Type::Kind GetObjectType() { return Type::TRANSFORMGROUP; };
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
};

G_END_DECLS

#endif
