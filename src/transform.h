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

namespace Moonlight {

/* @Namespace=System.Windows.Media */
class GeneralTransform : public DependencyObject {
 protected:
	cairo_matrix_t _matrix;
	bool need_update;
	
	/* @GeneratePInvoke,ManagedAccess=Protected */
	GeneralTransform () : DependencyObject (Type::GENERALTRANSFORM), need_update (true) { }
	
	virtual ~GeneralTransform () {};
	
	virtual void UpdateTransform ();
	void MaybeUpdateTransform ();

	/* @SkipFactories */
	GeneralTransform (Type::Kind object_type) : DependencyObject (object_type), need_update (true) { }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	virtual void GetTransform (cairo_matrix_t *value);
	
	/* @GeneratePInvoke */
	Matrix *GetMatrix ();
	
	Point Transform (Point point);

	/* @GenerateJSBinding,Version=3.0 */
	Point TransformXY (double x, double y);
};


/* @Namespace=System.Windows.Media */
class Transform : public GeneralTransform {
 protected:
	/* @GeneratePInvoke,ManagedAccess=Internal */
	Transform () : GeneralTransform (Type::TRANSFORM) { }

	virtual ~Transform () {}

	/* @SkipFactories */
	Transform (Type::Kind object_type) : GeneralTransform (object_type) { }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @GeneratePInvoke */
	GeneralTransform *GetInverse ();
	
	/* @GeneratePInvoke */
	bool TryTransform (Point inPoint, /* @MarshalAs=Point,IsOut */ Point *outPoint);
};


/* @Namespace=System.Windows.Media */
class RotateTransform : public Transform {
 protected:
	/* @GeneratePInvoke */
	RotateTransform () { SetObjectType (Type::ROTATETRANSFORM); }

	virtual ~RotateTransform () {}
	virtual void UpdateTransform ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int AngleProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterYProperty;
	
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
class CompositeTransform : public Transform {
protected:
	virtual ~CompositeTransform () {}
	virtual void UpdateTransform ();

	/* @GeneratePInvoke */
	CompositeTransform ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterYProperty;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int ScaleXProperty;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int ScaleYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int SkewXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int SkewYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RotationProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int TranslateXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int TranslateYProperty;
	
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

	void SetSkewX (double value);
	double GetSkewX ();

	void SetSkewY (double value);
	double GetSkewY ();

	void SetRotation (double value);
	double GetRotation ();

	void SetTranslateX (double value);
	double GetTranslateX ();

	void SetTranslateY (double value);
	double GetTranslateY ();
};


/* @Namespace=System.Windows.Media */
class TranslateTransform : public Transform {
 protected:
	/* @GeneratePInvoke */
	TranslateTransform () { SetObjectType (Type::TRANSLATETRANSFORM); }

	virtual ~TranslateTransform () { }
	virtual void UpdateTransform ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int XProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int YProperty;
	
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
	/* @GeneratePInvoke */
	ScaleTransform () { SetObjectType (Type::SCALETRANSFORM); }
	
	virtual ~ScaleTransform () {}
	virtual void UpdateTransform ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterYProperty;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int ScaleXProperty;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int ScaleYProperty;
	
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
	/* @GeneratePInvoke */
	SkewTransform () { SetObjectType (Type::SKEWTRANSFORM); }

	virtual ~SkewTransform () {}
	virtual void UpdateTransform ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int AngleXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int AngleYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterYProperty;
	
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
/* @ManagedEvents=None */
class Matrix : public DependencyObject {
	cairo_matrix_t matrix;
	
 protected:
	virtual ~Matrix () {}

 public:
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int M11Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M12Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M21Property;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int M22Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OffsetXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OffsetYProperty;

	/* @GeneratePInvoke,SkipFactories */
	Matrix ();

	/* @SkipFactories */
	Matrix (cairo_matrix_t *m);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	cairo_matrix_t GetUnderlyingMatrix ();

	/* @GeneratePInvoke */
	cairo_matrix_t *GetMatrixValues () { return &matrix; }
	
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
// this type does not really exists - its purpose is to let the unmanaged (1.x) matrix be a dependency object
// and the later (2.x) managed code use a struct (non-DO) for the matrix
class UnmanagedMatrix : public Matrix {

 protected:
	virtual ~UnmanagedMatrix () {}
	
 public:
	/* @GeneratePInvoke,SkipFactories */
	UnmanagedMatrix () {  SetObjectType (Type::UNMANAGEDMATRIX); }
};

/* @Namespace=System.Windows.Media */
class MatrixTransform : public Transform {
 protected:
	/* @GeneratePInvoke */
	MatrixTransform () { SetObjectType (Type::MATRIXTRANSFORM); }
	
	virtual ~MatrixTransform () {}
	
	virtual void UpdateTransform ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=Matrix,AutoCreateValue,GenerateAccessors */
	const static int MatrixProperty;
	
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
	/* @GeneratePInvoke */
	TransformCollection () { SetObjectType (Type::TRANSFORM_COLLECTION); }
	
	virtual ~TransformCollection () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	virtual Type::Kind GetElementType () { return Type::TRANSFORM; }
};


/* @ContentProperty="Children" */
/* @Namespace=System.Windows.Media */
class TransformGroup : public Transform {
 protected:
	/* @GeneratePInvoke */
	TransformGroup () : Transform (Type::TRANSFORMGROUP) { }
	
	virtual ~TransformGroup () {}
	
	virtual void UpdateTransform ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=TransformCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int ChildrenProperty;
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	//
	// Property Accessors
	//
	void SetChildren (TransformCollection *children);
	TransformCollection *GetChildren ();
};


G_BEGIN_DECLS

/* @GeneratePInvoke */
void   general_transform_transform_point (GeneralTransform *t, /* @MarshalAs=Point,IsRef */ Point *p, /* @MarshalAs=Point,IsRef */ Point *r);

G_END_DECLS

};
#endif
