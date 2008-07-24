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

void
Transform::OnPropertyChanged (PropertyChangedEventArgs *args)
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
Transform::UpdateTransform ()
{
	g_warning ("Transform:UpdateTransform has been called. The derived class should have overridden it.");
}

void
transform_get_transform (Transform *t, cairo_matrix_t *value)
{
	t->GetTransform (value);
}

Transform *
transform_new (void)
{
	return new Transform ();
}



DependencyProperty *RotateTransform::CenterXProperty;
DependencyProperty *RotateTransform::CenterYProperty;
DependencyProperty *RotateTransform::AngleProperty;

void
RotateTransform::UpdateTransform ()
{
	double angle, center_x, center_y;
	double radians;

	angle = rotate_transform_get_angle (this);
	center_x = rotate_transform_get_center_x (this);
	center_y = rotate_transform_get_center_y (this);

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

RotateTransform *
rotate_transform_new (void)
{
	return new RotateTransform ();
}

void
rotate_transform_set_angle (RotateTransform *transform, double angle)
{
	transform->SetAngle (angle);
}

double
rotate_transform_get_angle (RotateTransform *transform)
{
	return transform->GetAngle ();
}

void
rotate_transform_set_center_x (RotateTransform *transform, double centerX)
{
	transform->SetCenterX (centerX);
}

double
rotate_transform_get_center_x (RotateTransform *transform)
{
	return transform->GetCenterX ();
}

void
rotate_transform_set_center_y (RotateTransform *transform, double centerY)
{
	transform->SetCenterY (centerY);
}

double
rotate_transform_get_center_y (RotateTransform *transform)
{
	return transform->GetCenterY ();
}


DependencyProperty *TranslateTransform::XProperty;
DependencyProperty *TranslateTransform::YProperty;

void
TranslateTransform::UpdateTransform ()
{
	double x = translate_transform_get_x (this);
	double y = translate_transform_get_y (this);

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

TranslateTransform *
translate_transform_new (void)
{
	return new TranslateTransform ();
}

void
translate_transform_set_x (TranslateTransform *transform, double x)
{
	transform->SetX (x);
}

double
translate_transform_get_x (TranslateTransform *transform)
{
	return transform->GetX ();
}

void
translate_transform_set_y (TranslateTransform *transform, double y)
{
	transform->SetY (y);
}

double
translate_transform_get_y (TranslateTransform *transform)
{
	return transform->GetY ();
}



DependencyProperty* ScaleTransform::CenterXProperty;
DependencyProperty* ScaleTransform::CenterYProperty;
DependencyProperty* ScaleTransform::ScaleXProperty;
DependencyProperty* ScaleTransform::ScaleYProperty;

void
ScaleTransform::UpdateTransform ()
{
	double sx = scale_transform_get_scale_x (this);
	double sy = scale_transform_get_scale_y (this);

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

	double cx = scale_transform_get_center_x (this);
	double cy = scale_transform_get_center_y (this);

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

ScaleTransform *
scale_transform_new (void)
{
	return new ScaleTransform ();
}

void
scale_transform_set_scale_x (ScaleTransform *t, double scaleX)
{
	t->SetValue (ScaleTransform::ScaleXProperty, Value(scaleX));
}

double
scale_transform_get_scale_x (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::ScaleXProperty)->AsDouble();
}

void
scale_transform_set_scale_y (ScaleTransform *t, double scaleY)
{
	t->SetValue (ScaleTransform::ScaleYProperty, Value(scaleY));
}

double
scale_transform_get_scale_y (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::ScaleYProperty)->AsDouble();
}

void
scale_transform_set_center_x (ScaleTransform *t, double centerX)
{
	t->SetValue (ScaleTransform::CenterXProperty, Value(centerX));
}

double
scale_transform_get_center_x (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::CenterXProperty)->AsDouble();
}

void
scale_transform_set_center_y (ScaleTransform *t, double centerY)
{
	t->SetValue (ScaleTransform::CenterYProperty, Value(centerY));
}

double
scale_transform_get_center_y (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::CenterYProperty)->AsDouble();
}



DependencyProperty* SkewTransform::AngleXProperty;
DependencyProperty* SkewTransform::AngleYProperty;
DependencyProperty* SkewTransform::CenterXProperty;
DependencyProperty* SkewTransform::CenterYProperty;

void
SkewTransform::UpdateTransform ()
{
	double cx = skew_transform_get_center_x (this);
	double cy = skew_transform_get_center_y (this);

	bool translation = ((cx != 0.0) || (cy != 0.0));
	if (translation)
		cairo_matrix_init_translate (&_matrix, cx, cy);
	else
		cairo_matrix_init_identity (&_matrix);

	double ax = skew_transform_get_angle_x (this);
	if (ax != 0.0)
		_matrix.xy = tan (ax * M_PI / 180);

	double ay = skew_transform_get_angle_y (this);
	if (ay != 0.0)
		_matrix.yx = tan (ay * M_PI / 180);

	if (translation)
		cairo_matrix_translate (&_matrix, -cx, -cy);

	//printf ("SkewTransform %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

SkewTransform *
skew_transform_new (void)
{
	return new SkewTransform ();
}

void
skew_transform_set_angle_x (SkewTransform *t, double angleX)
{
	t->SetValue (SkewTransform::AngleXProperty, Value(angleX));
}

double
skew_transform_get_angle_x (SkewTransform *t)
{
	return t->GetValue (SkewTransform::AngleXProperty)->AsDouble();
}

void
skew_transform_set_angle_y (SkewTransform *t, double angleY)
{
	t->SetValue (SkewTransform::AngleYProperty, Value(angleY));
}

double
skew_transform_get_angle_y (SkewTransform *t)
{
	return t->GetValue (SkewTransform::AngleYProperty)->AsDouble();
}

void
skew_transform_set_center_x (SkewTransform *t, double centerX)
{
	t->SetValue (SkewTransform::CenterXProperty, Value(centerX));
}

double
skew_transform_get_center_x (SkewTransform *t)
{
	return t->GetValue (SkewTransform::CenterXProperty)->AsDouble();
}

void
skew_transform_set_center_y (SkewTransform *t, double centerY)
{
	t->SetValue (SkewTransform::CenterYProperty, Value(centerY));
}

double
skew_transform_get_center_y (SkewTransform *t)
{
	return t->GetValue (SkewTransform::CenterYProperty)->AsDouble();
}

DependencyProperty *Matrix::M11Property;
DependencyProperty *Matrix::M12Property;
DependencyProperty *Matrix::M21Property;
DependencyProperty *Matrix::M22Property;
DependencyProperty *Matrix::OffsetXProperty;
DependencyProperty *Matrix::OffsetYProperty;

Matrix::Matrix ()
{
	cairo_matrix_init_identity (&matrix);
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

Matrix *
matrix_new (void)
{
	return new Matrix ();
}

double
matrix_get_m11 (Matrix *matrix)
{
	return matrix->GetValue (Matrix::M11Property)->AsDouble ();
}

void
matrix_set_m11 (Matrix *matrix, double value)
{
	matrix->SetValue (Matrix::M11Property, Value (value));
}

double
matrix_get_m12 (Matrix *matrix)
{
	return matrix->GetValue (Matrix::M12Property)->AsDouble ();
}

void
matrix_set_m12 (Matrix *matrix, double value)
{
	matrix->SetValue (Matrix::M12Property, Value (value));
}

double
matrix_get_m21 (Matrix *matrix)
{
	return matrix->GetValue (Matrix::M21Property)->AsDouble ();
}

void
matrix_set_m21 (Matrix *matrix, double value)
{
	matrix->SetValue (Matrix::M21Property, Value (value));
}

double
matrix_get_m22 (Matrix *matrix)
{
	return matrix->GetValue (Matrix::M22Property)->AsDouble ();
}

void
matrix_set_m22 (Matrix *matrix, double value)
{
	matrix->SetValue (Matrix::M22Property, Value (value));
}

double
matrix_get_offset_x (Matrix *matrix)
{
	return matrix->GetValue (Matrix::OffsetXProperty)->AsDouble ();
}

void
matrix_set_offset_x (Matrix *matrix, double value)
{
	matrix->SetValue (Matrix::OffsetXProperty, Value (value));
}

double
matrix_get_offset_y (Matrix *matrix)
{
	return matrix->GetValue (Matrix::OffsetYProperty)->AsDouble ();
}

void
matrix_set_offset_y (Matrix *matrix, double value)
{
	matrix->SetValue (Matrix::OffsetYProperty, Value (value));
}

DependencyProperty* MatrixTransform::MatrixProperty;

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

MatrixTransform *
matrix_transform_new (void)
{
	return new MatrixTransform ();
}

void
matrix_transform_set_matrix (MatrixTransform *t, Matrix *matrix)
{
	t->SetValue (MatrixTransform::MatrixProperty, Value (matrix));
}

Matrix *
matrix_transform_get_matrix (MatrixTransform *t)
{
	return t->GetValue (MatrixTransform::MatrixProperty)->AsMatrix();
}

DependencyProperty *TransformGroup::ChildrenProperty;

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

TransformGroup *
transform_group_new (void)
{
	return new TransformGroup ();
}

//
// TransformCollection
//

TransformCollection *
transform_collection_new (void)
{
	return new TransformCollection ();
}

void
transform_init (void)
{
	/* RotateTransform fields */
	RotateTransform::AngleProperty   = DependencyProperty::Register (Type::ROTATETRANSFORM, "Angle", new Value (0.0));
	RotateTransform::CenterXProperty = DependencyProperty::Register (Type::ROTATETRANSFORM, "CenterX", new Value (0.0));
	RotateTransform::CenterYProperty = DependencyProperty::Register (Type::ROTATETRANSFORM, "CenterY", new Value (0.0));
  
	/* TranslateTransform fields */
	TranslateTransform::XProperty = DependencyProperty::Register (Type::TRANSLATETRANSFORM, "X", new Value (0.0));
	TranslateTransform::YProperty = DependencyProperty::Register (Type::TRANSLATETRANSFORM, "Y", new Value (0.0));

	/* ScaleTransform fields */
	ScaleTransform::ScaleXProperty = DependencyProperty::Register (Type::SCALETRANSFORM, "ScaleX", new Value (1.0));
	ScaleTransform::ScaleYProperty = DependencyProperty::Register (Type::SCALETRANSFORM, "ScaleY", new Value (1.0));
	ScaleTransform::CenterXProperty = DependencyProperty::Register (Type::SCALETRANSFORM, "CenterX", new Value (0.0));
	ScaleTransform::CenterYProperty = DependencyProperty::Register (Type::SCALETRANSFORM, "CenterY", new Value (0.0));

	/* SkewTransform fields */
	SkewTransform::AngleXProperty = DependencyProperty::Register (Type::SKEWTRANSFORM, "AngleX", new Value (0.0));
	SkewTransform::AngleYProperty = DependencyProperty::Register (Type::SKEWTRANSFORM, "AngleY", new Value (0.0));
	SkewTransform::CenterXProperty = DependencyProperty::Register (Type::SKEWTRANSFORM, "CenterX", new Value (0.0));
	SkewTransform::CenterYProperty = DependencyProperty::Register (Type::SKEWTRANSFORM, "CenterY", new Value (0.0));
	
	/* Matrix fields */
	Matrix::M11Property = DependencyProperty::Register (Type::MATRIX, "M11", new Value (1.0));
	Matrix::M12Property = DependencyProperty::Register (Type::MATRIX, "M12", new Value (0.0));
	Matrix::M21Property = DependencyProperty::Register (Type::MATRIX, "M21", new Value (0.0));
	Matrix::M22Property = DependencyProperty::Register (Type::MATRIX, "M22", new Value (1.0));
	Matrix::OffsetXProperty = DependencyProperty::Register (Type::MATRIX, "OffsetX", new Value (0.0));
	Matrix::OffsetYProperty = DependencyProperty::Register (Type::MATRIX, "OffsetY", new Value (0.0));

	/* MatrixTransform fields */
	MatrixTransform::MatrixProperty = DependencyProperty::Register (Type::MATRIXTRANSFORM, "Matrix", Type::MATRIX);

	/* TransformGroup fields */
	TransformGroup::ChildrenProperty = DependencyProperty::Register (Type::TRANSFORMGROUP, "Children", Type::TRANSFORM_COLLECTION);
}
