/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __MOONLIGHT_PROJECTION_H__
#define __MOONLIGHT_PROJECTION_H__

#include <glib.h>
#include <math.h>
#include "enums.h"
#include "dependencyobject.h"

#define VIEWPORT_SCALE            (65536.0)
#define VIEWPORT_SCALE_RECIPROCAL (1.0 / VIEWPORT_SCALE)

namespace Moonlight {

/* @Namespace=None */ // The managed Matrix3D is a struct
/* @ManagedDependencyProperties=Manual */
/* @ManagedEvents=None */
class Matrix3D : public DependencyObject {
private:
	double matrix[16];

protected:
	virtual ~Matrix3D () {}
	
public:
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int M11Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M12Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M13Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M14Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M21Property;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int M22Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M23Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M24Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M31Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M32Property;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int M33Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int M34Property;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OffsetXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OffsetYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OffsetZProperty;
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int M44Property;
	
	/* @GeneratePInvoke,SkipFactories */
	Matrix3D ();

	/* @SkipFactories */
	Matrix3D (double *m);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	/* @GeneratePInvoke */
	gpointer GetMatrixValues () { return matrix; }
	
	//
	// Property Accessors
	//
	void SetM11 (double m11);
	double GetM11 ();
	
	void SetM12 (double m12);
	double GetM12 ();

	void SetM13 (double m13);
	double GetM13 ();

	void SetM14 (double m14);
	double GetM14 ();

	void SetM21 (double m21);
	double GetM21 ();
	
	void SetM22 (double m22);
	double GetM22 ();

	void SetM23 (double m23);
	double GetM23 ();

	void SetM24 (double m24);
	double GetM24 ();

	void SetM31 (double m31);
	double GetM31 ();
	
	void SetM32 (double m32);
	double GetM32 ();

	void SetM33 (double m33);
	double GetM33 ();

	void SetM34 (double m34);
	double GetM34 ();

	void SetOffsetX (double offsetX);
	double GetOffsetX ();
	
	void SetOffsetY (double offsetY);
	double GetOffsetY ();

	void SetOffsetZ (double offsetY);
	double GetOffsetZ ();

	void SetM44 (double m44);
	double GetM44 ();

	static void Identity (double *out);
	static void Init (double *out, const double *m);
	static void TransformPoint (double *out, const double *m, const double *in);
	static Rect TransformBounds (const double *m, Rect bounds);
	static void Multiply (double *out, const double *a, const double *b);
	static void Translate (double *out, double tx, double ty, double tz);
	static void Scale (double *out, double sx, double sy, double sz);
	static void RotateX (double *out, double theta);
	static void RotateY (double *out, double theta);
	static void RotateZ (double *out, double theta);
	static void Perspective (double *out, double fieldOfViewY, double aspectRatio, double zNearPlane, double zFarPlane);
	static void Viewport (double *out, double width, double height);
	static bool Inverse (double *out, const double *m);
	static void Affine (double *out, double xx, double xy, double yx, double yy, double x0, double y0);
	static bool IsTranslation (const double *m);
	static bool IsIntegerTranslation (const double *m, int *x0, int *y0);
};

/* @Namespace=System.Windows.Media.Media3D */
class UnmanagedMatrix3D : public Matrix3D {

 protected:
	virtual ~UnmanagedMatrix3D () {}
	
 public:
	/* @GeneratePInvoke,SkipFactories */
	UnmanagedMatrix3D () {  SetObjectType (Type::UNMANAGEDMATRIX3D); }
};

/* @Namespace=System.Windows.Media */
class Projection : public DependencyObject {
public:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	Projection ();

	virtual Matrix3D *GetProjectionMatrix ();

	virtual void SetObjectSize (double width, double height) {}
	void GetTransform (double *value);
	virtual double DistanceFromXYPlane () { return NAN; }

protected:
	virtual ~Projection () {}

	virtual void UpdateProjection ();
	void MaybeUpdateProjection ();

	bool need_update;
};

/* @Namespace=System.Windows.Media */
class PlaneProjection : public Projection {
public:
	/* @GeneratePInvoke */
	PlaneProjection ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	const static int CenterOfRotationXProperty;
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	const static int CenterOfRotationYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int CenterOfRotationZProperty;

	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int GlobalOffsetXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int GlobalOffsetYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int GlobalOffsetZProperty;

	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int LocalOffsetXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int LocalOffsetYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int LocalOffsetZProperty;

	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RotationXProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RotationYProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int RotationZProperty;

	/* @PropertyType=Matrix3D,AutoCreateValue,GenerateAccessors,ManagedSetterAccess=Internal */
	const static int ProjectionMatrixProperty;

	//
	// Property Accessors
	//
	double GetCenterOfRotationX ();
	void SetCenterOfRotationX (double value);

	double GetCenterOfRotationY ();
	void SetCenterOfRotationY (double value);

	double GetCenterOfRotationZ ();
	void SetCenterOfRotationZ (double value);

	double GetGlobalOffsetX ();
	void SetGlobalOffsetX (double value);

	double GetGlobalOffsetY ();
	void SetGlobalOffsetY (double value);

	double GetGlobalOffsetZ ();
	void SetGlobalOffsetZ (double value);

	double GetLocalOffsetX ();
	void SetLocalOffsetX (double value);

	double GetLocalOffsetY ();
	void SetLocalOffsetY (double value);

	double GetLocalOffsetZ ();
	void SetLocalOffsetZ (double value);

	double GetRotationX ();
	void SetRotationX (double value);

	double GetRotationY ();
	void SetRotationY (double value);

	double GetRotationZ ();
	void SetRotationZ (double value);

	Matrix3D* GetProjectionMatrix ();
	void SetProjectionMatrix (Matrix3D* value);

	void SetObjectSize (double width, double height);
	double DistanceFromXYPlane ();

protected:
	virtual ~PlaneProjection () {}

	void UpdateProjection ();

	double objectWidth;
	double objectHeight;
};

/* @Namespace=System.Windows.Media */
/* @ContentProperty="ProjectionMatrix" */
class Matrix3DProjection : public Projection {
public:
	/* @GeneratePInvoke */
	Matrix3DProjection () { SetObjectType (Type::MATRIX3DPROJECTION); }

	/* @PropertyType=Matrix3D,AutoCreateValue,GenerateAccessors */
	const static int ProjectionMatrixProperty;


	//
	// Property Accessors
	//
	void SetProjectionMatrix (Matrix3D *matrix);
	Matrix3D *GetProjectionMatrix ();

protected:
	virtual ~Matrix3DProjection () {}
};

};
#endif
