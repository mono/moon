/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * transform.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include "transform.h"
#include "math.h"


//
// GeneralTransform
//

void
GeneralTransform::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() == Type::DEPENDENCY_OBJECT) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}

	need_update = true;
	//
	// If the transform changes, we need to notify our owners
	// that they must repaint (all of our properties have
	// a visible effect.
	//
	// There is no need to override this on the base classes
	// as they are sealed, so no new properties can be added
	// and I do not believe that we can create new instances
	// of transform from C#, and in that case, we would only
	// be slower.
	//
	NotifyListenersOfPropertyChange (args);
}

void
GeneralTransform::UpdateTransform ()
{
	g_warning ("GeneralTransform::UpdateTransform has been called. The derived class should have overridden it.");
}

void
GeneralTransform::MaybeUpdateTransform ()
{
	if (need_update) {
		UpdateTransform ();
		need_update = false;
	}
}

void
GeneralTransform::GetTransform (cairo_matrix_t *value)
{
	MaybeUpdateTransform ();
	*value = _matrix;
}

Point
GeneralTransform::Transform (Point point)
{
	MaybeUpdateTransform ();
	return point.Transform (&_matrix);
}

void
general_transform_transform_point (GeneralTransform *t, Point *p, Point *r)
{
	*r = t->Transform (*p);
}


//
// RotateTransform
//

void
RotateTransform::UpdateTransform ()
{
	double angle, center_x, center_y;
	double radians;

	angle = GetAngle ();
	center_x = GetCenterX ();
	center_y = GetCenterY ();
	
	radians = angle / 180.0 * M_PI;

	if (center_x == 0.0 && center_y == 0.0) {
		cairo_matrix_init_rotate (&_matrix, radians);
	}
	else {
		cairo_matrix_init_translate (&_matrix, center_x, center_y);
		cairo_matrix_rotate (&_matrix, radians);
		cairo_matrix_translate (&_matrix, -center_x, -center_y);
	}
	//printf ("Returning2 %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

void
RotateTransform::SetAngle (double angle)
{
	SetValue (RotateTransform::AngleProperty, Value (angle));
}

double
RotateTransform::GetAngle ()
{
	return GetValue (RotateTransform::AngleProperty)->AsDouble ();
}

void
RotateTransform::SetCenterX (double centerX)
{
	SetValue (RotateTransform::CenterXProperty, Value (centerX));
}

double
RotateTransform::GetCenterX ()
{
	return GetValue (RotateTransform::CenterXProperty)->AsDouble ();
}

void
RotateTransform::SetCenterY (double centerY)
{
	SetValue (RotateTransform::CenterYProperty, Value (centerY));
}

double
RotateTransform::GetCenterY ()
{
	return GetValue (RotateTransform::CenterYProperty)->AsDouble ();
}


//
// TranslateTransform
//

void
TranslateTransform::UpdateTransform ()
{
	double x = GetX ();
	double y = GetY ();

	cairo_matrix_init_translate (&_matrix, x, y);
	//printf ("translating dx %g dy %g", x, y);
	//printf ("TranslateTransform %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

void
TranslateTransform::SetX (double x)
{
	SetValue (TranslateTransform::XProperty, Value (x));
}

double
TranslateTransform::GetX ()
{
	return GetValue (TranslateTransform::XProperty)->AsDouble ();
}

void
TranslateTransform::SetY (double y)
{
	SetValue (TranslateTransform::YProperty, Value (y));
}

double
TranslateTransform::GetY ()
{
	return GetValue (TranslateTransform::YProperty)->AsDouble ();
}


//
// ScaleTransform
//
void
ScaleTransform::UpdateTransform ()
{
	double sx = GetScaleX ();
	double sy = GetScaleY ();

	// XXX you don't want to know.  don't make these 0.00001, or
	// else cairo spits out errors about non-invertable matrices
	// (or worse, crashes)
	//
	// the 0.0 scales are caused in at least one instance by us
	// being too aggressive at starting animations at time=0 when
	// they're supposed to (unset, or 0:0:0 BeginTime)
	//
	if (sx == 0.0) sx = 0.00002;
	if (sy == 0.0) sy = 0.00002;

	double cx = GetCenterX ();
	double cy = GetCenterY ();

	if (cx == 0.0 && cy == 0.0) {
		cairo_matrix_init_scale (&_matrix, sx, sy);
	}
	else {
		cairo_matrix_init_translate (&_matrix, cx, cy);
		cairo_matrix_scale (&_matrix, sx, sy);
		cairo_matrix_translate (&_matrix, -cx, -cy);
	}
	//printf ("scaling sx %g sy %g at center cx %g cy %g\n", sx, sy, cx, cy);
	//printf ("ScaleTransform %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

void
ScaleTransform::SetCenterX (double centerX)
{
	SetValue (ScaleTransform::CenterXProperty, Value (centerX));
}

double
ScaleTransform::GetCenterX ()
{
	return GetValue (ScaleTransform::CenterXProperty)->AsDouble ();
}

void
ScaleTransform::SetCenterY (double centerY)
{
	SetValue (ScaleTransform::CenterYProperty, Value (centerY));
}

double
ScaleTransform::GetCenterY ()
{
	return GetValue (ScaleTransform::CenterYProperty)->AsDouble ();
}

void
ScaleTransform::SetScaleX (double scaleX)
{
	SetValue (ScaleTransform::ScaleXProperty, Value (scaleX));
}

double
ScaleTransform::GetScaleX ()
{
	return GetValue (ScaleTransform::ScaleXProperty)->AsDouble ();
}

void
ScaleTransform::SetScaleY (double scaleY)
{
	SetValue (ScaleTransform::ScaleYProperty, Value (scaleY));
}

double
ScaleTransform::GetScaleY ()
{
	return GetValue (ScaleTransform::ScaleYProperty)->AsDouble ();
}


//
// SkewTransform
//

void
SkewTransform::UpdateTransform ()
{
	double cx = GetCenterX ();
	double cy = GetCenterY ();

	bool translation = ((cx != 0.0) || (cy != 0.0));
	if (translation)
		cairo_matrix_init_translate (&_matrix, cx, cy);
	else
		cairo_matrix_init_identity (&_matrix);

	double ax = GetAngleX ();
	if (ax != 0.0)
		_matrix.xy = tan (ax * M_PI / 180);

	double ay = GetAngleY ();
	if (ay != 0.0)
		_matrix.yx = tan (ay * M_PI / 180);

	if (translation)
		cairo_matrix_translate (&_matrix, -cx, -cy);

	//printf ("SkewTransform %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

void
SkewTransform::SetAngleX (double angleX)
{
	SetValue (SkewTransform::AngleXProperty, Value (angleX));
}

double
SkewTransform::GetAngleX ()
{
	return GetValue (SkewTransform::AngleXProperty)->AsDouble ();
}

void
SkewTransform::SetAngleY (double angleY)
{
	SetValue (SkewTransform::AngleYProperty, Value (angleY));
}

double
SkewTransform::GetAngleY ()
{
	return GetValue (SkewTransform::AngleYProperty)->AsDouble ();
}

void
SkewTransform::SetCenterX (double centerX)
{
	SetValue (SkewTransform::CenterXProperty, Value (centerX));
}

double
SkewTransform::GetCenterX ()
{
	return GetValue (SkewTransform::CenterXProperty)->AsDouble ();
}

void
SkewTransform::SetCenterY (double centerY)
{
	SetValue (SkewTransform::CenterYProperty, Value (centerY));
}

double
SkewTransform::GetCenterY ()
{
	return GetValue (SkewTransform::CenterYProperty)->AsDouble ();
}


//
// Matrix
//

Matrix::Matrix ()
{
	cairo_matrix_init_identity (&matrix);
}

Matrix::Matrix(cairo_matrix_t *m)
{
	matrix = *m;
}

void
Matrix::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::MATRIX) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == Matrix::M11Property)
		matrix.xx = args->new_value->AsDouble ();
	else if (args->property == Matrix::M12Property)
		matrix.yx = args->new_value->AsDouble ();
	else if (args->property == Matrix::M21Property)
		matrix.xy = args->new_value->AsDouble ();
	else if (args->property == Matrix::M22Property)
		matrix.yy = args->new_value->AsDouble ();
	else if (args->property == Matrix::OffsetXProperty)
		matrix.x0 = args->new_value->AsDouble ();
	else if (args->property == Matrix::OffsetYProperty)
		matrix.y0 = args->new_value->AsDouble ();

	NotifyListenersOfPropertyChange (args);
}

cairo_matrix_t
Matrix::GetUnderlyingMatrix ()
{
	return matrix;
}

void
Matrix::SetM11 (double m11)
{
	SetValue (Matrix::M11Property, Value (m11));
}

double
Matrix::GetM11 ()
{
	return GetValue (Matrix::M11Property)->AsDouble ();
}

void
Matrix::SetM12 (double m12)
{
	SetValue (Matrix::M12Property, Value (m12));
}

double
Matrix::GetM12 ()
{
	return GetValue (Matrix::M12Property)->AsDouble ();
}

void
Matrix::SetM21 (double m21)
{
	SetValue (Matrix::M21Property, Value (m21));
}

double
Matrix::GetM21 ()
{
	return GetValue (Matrix::M21Property)->AsDouble ();
}

void
Matrix::SetM22 (double m22)
{
	SetValue (Matrix::M22Property, Value (m22));
}

double
Matrix::GetM22 ()
{
	return GetValue (Matrix::M22Property)->AsDouble ();
}

void
Matrix::SetOffsetX (double offsetX)
{
	SetValue (Matrix::OffsetXProperty, Value (offsetX));
}

double
Matrix::GetOffsetX ()
{
	return GetValue (Matrix::OffsetXProperty)->AsDouble ();
}

void
Matrix::SetOffsetY (double offsetY)
{
	SetValue (Matrix::OffsetYProperty, Value (offsetY));
}

double
Matrix::GetOffsetY ()
{
	return GetValue (Matrix::OffsetYProperty)->AsDouble ();
}


//
// MatrixTransform
//

void
MatrixTransform::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	need_update = true;

	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);

	NotifyListenersOfPropertyChange (MatrixTransform::MatrixProperty);
}

void
MatrixTransform::UpdateTransform ()
{
	Matrix *matrix = GetValue (MatrixTransform::MatrixProperty)->AsMatrix();
	if (matrix)
		_matrix = matrix->GetUnderlyingMatrix();
	else
		cairo_matrix_init_identity (&_matrix);
}

void
MatrixTransform::SetMatrix (Matrix *matrix)
{
	SetValue (MatrixTransform::MatrixProperty, Value (matrix));
}

Matrix *
MatrixTransform::GetMatrix ()
{
	Value *value = GetValue (MatrixTransform::MatrixProperty);
	
	return value ? value->AsMatrix () : NULL;
}


//
// TransformGroup
//

TransformGroup::TransformGroup ()
{
	SetValue (TransformGroup::ChildrenProperty, Value::CreateUnref (new TransformCollection ()));
}

TransformGroup::~TransformGroup ()
{
}

void
TransformGroup::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::TRANSFORMGROUP) {
		Transform::OnPropertyChanged (args);
		return;
	}

	if (args->property == TransformGroup::ChildrenProperty) {
		need_update = true;
	}

	NotifyListenersOfPropertyChange (args);
}

void
TransformGroup::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col != GetValue (TransformGroup::ChildrenProperty)->AsCollection ()) {
		Transform::OnCollectionChanged (col, args);
		return;
	}
	
	need_update = true;
	NotifyListenersOfPropertyChange (TransformGroup::ChildrenProperty);
}

void
TransformGroup::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetValue (TransformGroup::ChildrenProperty)->AsCollection ()) {
		Transform::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	need_update = true;
	NotifyListenersOfPropertyChange (TransformGroup::ChildrenProperty);
}

void
TransformGroup::UpdateTransform ()
{
	TransformCollection *children = GetValue (TransformGroup::ChildrenProperty)->AsTransformCollection ();
	
	cairo_matrix_init_identity (&_matrix);
	
	for (int i = 0; i < children->GetCount (); i++) {
		Transform *transform = children->GetValueAt (i)->AsTransform ();
		cairo_matrix_t matrix;
		
		transform->GetTransform (&matrix);
		cairo_matrix_multiply (&_matrix, &_matrix, &matrix);
	}
}

void
TransformGroup::SetChildren (TransformCollection *children)
{
	SetValue (TransformGroup::ChildrenProperty, Value (children));
}

TransformCollection *
TransformGroup::GetChildren ()
{
	Value *value = GetValue (TransformGroup::ChildrenProperty);
	
	return value ? value->AsTransformCollection () : NULL;
}
