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
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	GeneralTransform () : need_update (true) { }
	
	virtual Type::Kind GetObjectType () { return Type::GENERALTRANSFORM; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	virtual void GetTransform (cairo_matrix_t *value);
	
	Point Transform (Point point);
};


/* @Namespace=System.Windows.Media */
class Transform : public GeneralTransform {
protected:
	virtual ~Transform () {}

public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Transform () { }
	
	virtual Type::Kind GetObjectType () { return Type::TRANSFORM; }
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
	
	/* @GenerateCBinding,GeneratePInvoke */
	RotateTransform () { }
	
	virtual Type::Kind GetObjectType () { return Type::ROTATETRANSFORM; }
	
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


/* @Namespace=System.Windows.Media */
class TranslateTransform : public Transform {
 protected:
	virtual ~TranslateTransform () { }
	virtual void UpdateTransform ();
	
 public:
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *XProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *YProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TranslateTransform () { }
	
	virtual Type::Kind GetObjectType () { return Type::TRANSLATETRANSFORM; }
	
	//
	// Property Accessors
	//
	void SetX (double x);
	double GetX ();
	
	void SetY (double y);
	double GetY ();
};


/* @Namespace=System.Windows.Media */
class ScaleTransform : public Transform {
 protected:
	virtual ~ScaleTransform () {}
	virtual void UpdateTransform ();

 public:
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterXProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterYProperty;
	/* @PropertyType=double,DefaultValue=1.0 */
	static DependencyProperty *ScaleXProperty;
	/* @PropertyType=double,DefaultValue=1.0 */
	static DependencyProperty *ScaleYProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ScaleTransform () { }
	virtual Type::Kind GetObjectType () { return Type::SCALETRANSFORM; }
	
	//
	// Property Accessors
	//
	void SetCenterX (double centerX);
	double GetCenterX ();
	
	void SetCenterY (double centerY);
	double GetCenterY ();
	
	void SetScaleX (double scaleX);
	double GetScaleX ();
	
	void SetScaleY (double scaleY);
	double GetScaleY ();
};


/* @Namespace=System.Windows.Media */
class SkewTransform : public Transform {
 protected:
	virtual ~SkewTransform () {}
	virtual void UpdateTransform ();
	
 public:
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *AngleXProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *AngleYProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterXProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *CenterYProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	SkewTransform () { }
	
	virtual Type::Kind GetObjectType () { return Type::SKEWTRANSFORM; }
	
	//
	// Property Accessors
	//
	void SetAngleX (double angleX);
	double GetAngleX ();
	
	void SetAngleY (double angleY);
	double GetAngleY ();
	
	void SetCenterX (double centerX);
	double GetCenterX ();
	
	void SetCenterY (double centerY);
	double GetCenterY ();
};


/* @Namespace=None */ // The managed Matrix is a struct
/* @ManagedDependencyProperties=Manual */
class Matrix : public DependencyObject {
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
	
	/* @GenerateCBinding,GeneratePInvoke */
	Matrix ();
	Matrix (cairo_matrix_t *m);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	virtual Type::Kind GetObjectType () { return Type::MATRIX; }
	
	cairo_matrix_t GetUnderlyingMatrix ();
	
	//
	// Property Accessors
	//
	void SetM11 (double m11);
	double GetM11 ();
	
	void SetM12 (double m12);
	double GetM12 ();
	
	void SetM21 (double m21);
	double GetM21 ();
	
	void SetM22 (double m22);
	double GetM22 ();
	
	void SetOffsetX (double offsetX);
	double GetOffsetX ();
	
	void SetOffsetY (double offsetY);
	double GetOffsetY ();
};


/* @Namespace=System.Windows.Media */
class MatrixTransform : public Transform {
 protected:
	virtual ~MatrixTransform () {}
	
	virtual void UpdateTransform ();
	
 public:
	/* @PropertyType=Matrix */
	static DependencyProperty *MatrixProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	MatrixTransform () { }
	
	virtual Type::Kind GetObjectType () { return Type::MATRIXTRANSFORM; }
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	//
	// Property Accessors
	//
	void SetMatrix (Matrix *matrix);
	Matrix *GetMatrix ();
};


/* @Namespace=System.Windows.Media */
class TransformCollection : public DependencyObjectCollection {
 protected:
	virtual ~TransformCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TransformCollection () { }
	
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
	
	/* @GenerateCBinding,GeneratePInvoke */
	TransformGroup ();
	virtual Type::Kind GetObjectType() { return Type::TRANSFORMGROUP; }
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	//
	// Property Accessors
	//
	void SetChildren (TransformCollection *children);
	TransformCollection *GetChildren ();
};


G_BEGIN_DECLS

void   general_transform_transform_point (GeneralTransform *t, Point *p, Point *r);

G_END_DECLS

#endif
