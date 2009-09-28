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

#include <math.h>

#include "transform.h"


//
// GeneralTransform
//

void
GeneralTransform::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() == Type::DEPENDENCY_OBJECT) {
		DependencyObject::OnPropertyChanged (args, error);
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
	NotifyListenersOfPropertyChange (args, error);
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

Matrix*
GeneralTransform::GetMatrix ()
{
	MaybeUpdateTransform ();
	return new Matrix (&_matrix);
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

//
// Matrix
//

Matrix::Matrix ()
{
	SetObjectType (Type::MATRIX);
	cairo_matrix_init_identity (&matrix);
}

Matrix::Matrix(cairo_matrix_t *m)
{
	SetObjectType (Type::MATRIX);
	matrix = *m;
	SetM11 (matrix.xx);
	SetM12 (matrix.yx);
	SetM21 (matrix.xy);
	SetM22 (matrix.yy);
	SetOffsetX (matrix.x0);
	SetOffsetY (matrix.y0);
}

void
Matrix::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::MATRIX) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Matrix::M11Property)
		matrix.xx = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix::M12Property)
		matrix.yx = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix::M21Property)
		matrix.xy = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix::M22Property)
		matrix.yy = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix::OffsetXProperty)
		matrix.x0 = args->GetNewValue()->AsDouble ();
	else if (args->GetId () == Matrix::OffsetYProperty)
		matrix.y0 = args->GetNewValue()->AsDouble ();

	NotifyListenersOfPropertyChange (args, error);
}

cairo_matrix_t
Matrix::GetUnderlyingMatrix ()
{
	return matrix;
}

//
// MatrixTransform
//

void
MatrixTransform::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	need_update = true;

	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);

	NotifyListenersOfPropertyChange (MatrixTransform::MatrixProperty, NULL);
}

void
MatrixTransform::UpdateTransform ()
{
	Matrix *matrix = GetMatrix ();
	if (matrix)
		_matrix = matrix->GetUnderlyingMatrix();
	else
		cairo_matrix_init_identity (&_matrix);
}

//
// TransformGroup
//

TransformGroup::TransformGroup ()
{
	SetObjectType (Type::TRANSFORMGROUP);
}

void
TransformGroup::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::TRANSFORMGROUP) {
		Transform::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == TransformGroup::ChildrenProperty) {
		need_update = true;
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
TransformGroup::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col != GetChildren ()) {
		Transform::OnCollectionChanged (col, args);
		return;
	}
	
	need_update = true;
	NotifyListenersOfPropertyChange (TransformGroup::ChildrenProperty, NULL);
}

void
TransformGroup::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetChildren ()) {
		Transform::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	// Unit tests shows that the "cache" matrix value (exposed in SL2) is not updated when child items are changed.
	// However SL2 does re-compute this value (if dirty) before drawing anything that depends on it.
	// Currently Moonlight behave differently by always returning the "up to date" matrix
	need_update = true;
	NotifyListenersOfPropertyChange (TransformGroup::ChildrenProperty, NULL);
}

void
TransformGroup::UpdateTransform ()
{
	TransformCollection *children = GetChildren ();
	
	cairo_matrix_init_identity (&_matrix);
	
	for (int i = 0; i < children->GetCount (); i++) {
		Transform *transform = children->GetValueAt (i)->AsTransform ();
		cairo_matrix_t matrix;
		
		transform->GetTransform (&matrix);
		cairo_matrix_multiply (&_matrix, &_matrix, &matrix);
	}
}
