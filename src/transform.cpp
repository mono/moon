#include "transform.h"
#include "math.h"

void
transform_get_value (Transform *t, cairo_matrix_t *value)
{
  t->GetValue (value);
}

void
RotateTransform::GetValue (cairo_matrix_t *value)
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
  do_set_value (t, "RotateTransform::Angle", (Value)angle);
}

double
rotate_transform_get_angle (RotateTransform *t)
{
  return 0.0;
}

void
rotate_transform_set_center_x (RotateTransform *t, double centerX)
{
  do_set_value (t, "RotateTransform::CenterX", (Value)centerX);
}

double
rotate_transform_get_center_x (RotateTransform *t)
{
  return 0.0;
}

void
rotate_transform_set_center_y (RotateTransform *t, double centerY)
{
  do_set_value (t, "RotateTransform::CenterY", (Value)centerY);
}

double
rotate_transform_get_center_y (RotateTransform *t)
{
  return 0.0;
}



void
TranslateTransform::GetValue (cairo_matrix_t *value)
{
  double x = translate_transform_get_x (this);
  double y = translate_transform_get_y (this);

  cairo_matrix_init_translate (value, x, y);
}

void
translate_transform_set_x (TranslateTransform *t, double x)
{
  do_set_value (t, "TranslateTransform::X", (Value)y);
}

double
translate_transform_get_x (TranslateTransform *t)
{
  return 0.0;
}

void
translate_transform_set_y (TranslateTransform *t, double y)
{
  do_set_value (t, "TranslateTransform::Y", (Value)y);
}

double
translate_transform_get_y (TranslateTransform *t)
{
  return 0.0;
}


void
ScaleTransform::GetValue (cairo_matrix_t *value)
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
  do_set_value (t, "ScaleTransform::ScaleX", (Value)scaleX);
}

double
scale_transform_get_scale_x (ScaleTransform *t)
{
  return 0.0;
}

void
scale_transform_set_scale_y (ScaleTransform *t, double scaleY)
{
  do_set_value (t, "ScaleTransform::ScaleY", (Value)scaleX);
}

double
scale_transform_get_scale_y (ScaleTransform *t)
{
  return 0.0;
}

void
scale_transform_set_center_x (ScaleTransform *t, double centerX)
{
  do_set_value (t, "ScaleTransform::CenterX", (Value)centerX);
}

double
scale_transform_get_center_x (ScaleTransform *t)
{
  return 0.0;
}

void
scale_transform_set_center_y (ScaleTransform *t, double centerY)
{
  do_set_value (t, "ScaleTransform::CenterY", (Value)centerY);
}

double
scale_transform_get_center_y (ScaleTransform *t)
{
  return 0.0;
}



void
MatrixTransform::GetValue (cairo_matrix_t *value)
{
  cairo_matrix_t matrix = matrix_transform_get_matrix (this);

  memcpy (value, &matrix, sizeof (cairo_matrix_t);
}

void
scale_transform_set_scale_x (ScaleTransform *t, double scaleX)
{
  do_set_value (t, "ScaleTransform::ScaleX", (Value)scaleX);
}

double
scale_transform_get_scale_x (ScaleTransform *t)
{
  return 0.0;
}

void
scale_transform_set_scale_y (ScaleTransform *t, double scaleY)
{
  do_set_value (t, "ScaleTransform::ScaleY", (Value)scaleX);
}

double
scale_transform_get_scale_y (ScaleTransform *t)
{
  return 0.0;
}

void
scale_transform_set_center_x (ScaleTransform *t, double centerX)
{
  do_set_value (t, "ScaleTransform::CenterX", (Value)centerX);
}

double
scale_transform_get_center_x (ScaleTransform *t)
{
  return 0.0;
}

void
scale_transform_set_center_y (ScaleTransform *t, double centerY)
{
  do_set_value (t, "ScaleTransform::CenterY", (Value)centerY);
}

double
scale_transform_get_center_y (ScaleTransform *t)
{
  return 0.0;
}
