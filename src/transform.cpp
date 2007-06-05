#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "transform.h"
#include "math.h"

void
transform_get_value (Transform *t, cairo_matrix_t *value)
{
  t->GetTransform (value);
}



DependencyProperty* RotateTransform::CenterXProperty;
DependencyProperty* RotateTransform::CenterYProperty;
DependencyProperty* RotateTransform::AngleProperty;

void
RotateTransform::GetTransform (cairo_matrix_t *value)
{
	double angle, center_x, center_y;
	double radians;

	angle = rotate_transform_get_angle (this);
	center_x = rotate_transform_get_center_x (this);
	center_y = rotate_transform_get_center_y (this);

	radians = angle / 180.0 * M_PI;

	if (center_x == 0.0 && center_y == 0.0) {
	  cairo_matrix_init_rotate (value, radians);
	}
	else {
	  cairo_matrix_init_identity (value);
	  cairo_matrix_translate (value, -center_x, -center_y);
	  cairo_matrix_rotate (value, radians);
	  cairo_matrix_translate (value, center_x, center_y);
	}
}

void
rotate_transform_set_angle (RotateTransform *t, double angle)
{
	t->SetValue (RotateTransform::AngleProperty, Value(angle));
}

double
rotate_transform_get_angle (RotateTransform *t)
{
	return t->GetValue (RotateTransform::AngleProperty)->u.d;
}

void
rotate_transform_set_center_x (RotateTransform *t, double centerX)
{
	t->SetValue (RotateTransform::CenterXProperty, Value(centerX));
}

double
rotate_transform_get_center_x (RotateTransform *t)
{
	return t->GetValue (RotateTransform::CenterXProperty)->u.d;
}

void
rotate_transform_set_center_y (RotateTransform *t, double centerY)
{
	t->SetValue (RotateTransform::CenterYProperty, Value(centerY));
}

double
rotate_transform_get_center_y (RotateTransform *t)
{
	return t->GetValue (RotateTransform::CenterYProperty)->u.d;
}




DependencyProperty* TranslateTransform::XProperty;
DependencyProperty* TranslateTransform::YProperty;

void
TranslateTransform::GetTransform (cairo_matrix_t *value)
{
  double x = translate_transform_get_x (this);
  double y = translate_transform_get_y (this);

  cairo_matrix_init_translate (value, x, y);
}

void
translate_transform_set_x (TranslateTransform *t, double x)
{
	t->SetValue (TranslateTransform::XProperty, Value(x));
}

double
translate_transform_get_x (TranslateTransform *t)
{
	return t->GetValue (TranslateTransform::XProperty)->u.d;
}

void
translate_transform_set_y (TranslateTransform *t, double y)
{
	t->SetValue (TranslateTransform::YProperty, Value(y));
}

double
translate_transform_get_y (TranslateTransform *t)
{
	return t->GetValue (TranslateTransform::YProperty)->u.d;
}



DependencyProperty* ScaleTransform::CenterXProperty;
DependencyProperty* ScaleTransform::CenterYProperty;
DependencyProperty* ScaleTransform::ScaleXProperty;
DependencyProperty* ScaleTransform::ScaleYProperty;

void
ScaleTransform::GetTransform (cairo_matrix_t *value)
{
	double sx = scale_transform_get_scale_x (this);
	double sy = scale_transform_get_scale_y (this);

	double cx = scale_transform_get_center_x (this);
	double cy = scale_transform_get_center_y (this);

	if (cx == 0.0 && cy == 0.0) {
	  cairo_matrix_init_scale (value, sx, sy);
	}
	else {
	  cairo_matrix_init_identity (value);
	  cairo_matrix_translate (value, -cx, -cy);
	  cairo_matrix_scale (value, sx, sy);
	  cairo_matrix_scale (value, cx, cy);
	}
}

void
scale_transform_set_scale_x (ScaleTransform *t, double scaleX)
{
	t->SetValue (ScaleTransform::ScaleXProperty, Value(scaleX));
}

double
scale_transform_get_scale_x (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::ScaleXProperty)->u.d;
}

void
scale_transform_set_scale_y (ScaleTransform *t, double scaleY)
{
	t->SetValue (ScaleTransform::ScaleYProperty, Value(scaleY));
}

double
scale_transform_get_scale_y (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::ScaleYProperty)->u.d;
}

void
scale_transform_set_center_x (ScaleTransform *t, double centerX)
{
	t->SetValue (ScaleTransform::CenterXProperty, Value(centerX));
}

double
scale_transform_get_center_x (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::CenterXProperty)->u.d;
}

void
scale_transform_set_center_y (ScaleTransform *t, double centerY)
{
	t->SetValue (ScaleTransform::CenterYProperty, Value(centerY));
}

double
scale_transform_get_center_y (ScaleTransform *t)
{
	return t->GetValue (ScaleTransform::CenterYProperty)->u.d;
}



void
MatrixTransform::GetTransform (cairo_matrix_t *value)
{
#if notyet
  cairo_matrix_t matrix = matrix_transform_get_matrix (this);

  memcpy (value, &matrix, sizeof (cairo_matrix_t));
#endif
}


void
transform_init ()
{
	/* RotateTransform fields */
	DependencyObject::Register (DependencyObject::ROTATETRANSFORM, "Angle", new Value (0.0));
	DependencyObject::Register (DependencyObject::ROTATETRANSFORM, "CenterX", new Value (0.0));
	DependencyObject::Register (DependencyObject::ROTATETRANSFORM, "CenterY", new Value (0.0));
  
	/* TranslateTransform fields */
	DependencyObject::Register (DependencyObject::TRANSLATETRANSFORM, "X", new Value (0.0));
	DependencyObject::Register (DependencyObject::TRANSLATETRANSFORM, "Y", new Value (0.0));

	/* ScaleTransform fields */
	DependencyObject::Register (DependencyObject::SCALETRANSFORM, "ScaleX", new Value (1.0));
	DependencyObject::Register (DependencyObject::SCALETRANSFORM, "ScaleY", new Value (1.0));
	DependencyObject::Register (DependencyObject::SCALETRANSFORM, "CenterX", new Value (0.0));
	DependencyObject::Register (DependencyObject::SCALETRANSFORM, "CenterY", new Value (0.0));

	/* XXX MatrixTransform fields */
}
