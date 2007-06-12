#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "transform.h"
#include "math.h"

void
Transform::OnPropertyChanged (DependencyProperty *prop)
{
	NotifyAttacheesOfPropertyChange (prop);
}

void
transform_get_transform (Transform *t, cairo_matrix_t *value)
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
		cairo_matrix_translate (value, center_x, center_y);
		cairo_matrix_rotate (value, radians);
		cairo_matrix_translate (value, -center_x, -center_y);
	}
	//printf ("Returning2 %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

RotateTransform *
rotate_transform_new ()
{
	return new RotateTransform ();
}

void
rotate_transform_set_angle (RotateTransform *t, double angle)
{
	t->SetValue (RotateTransform::AngleProperty, Value(angle));
}

double
rotate_transform_get_angle (RotateTransform *t)
{
	return t->GetValue (RotateTransform::AngleProperty)->AsDouble();
}

void
rotate_transform_set_center_x (RotateTransform *t, double centerX)
{
	t->SetValue (RotateTransform::CenterXProperty, Value(centerX));
}

double
rotate_transform_get_center_x (RotateTransform *t)
{
	return t->GetValue (RotateTransform::CenterXProperty)->AsDouble();
}

void
rotate_transform_set_center_y (RotateTransform *t, double centerY)
{
	t->SetValue (RotateTransform::CenterYProperty, Value(centerY));
}

double
rotate_transform_get_center_y (RotateTransform *t)
{
	return t->GetValue (RotateTransform::CenterYProperty)->AsDouble();
}




DependencyProperty* TranslateTransform::XProperty;
DependencyProperty* TranslateTransform::YProperty;

void
TranslateTransform::GetTransform (cairo_matrix_t *value)
{
	double x = translate_transform_get_x (this);
	double y = translate_transform_get_y (this);

	cairo_matrix_init_translate (value, x, y);
	//printf ("Returning3 %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

TranslateTransform *
translate_transform_new ()
{
	return new TranslateTransform ();
}

void
translate_transform_set_x (TranslateTransform *t, double x)
{
	t->SetValue (TranslateTransform::XProperty, Value(x));
}

double
translate_transform_get_x (TranslateTransform *t)
{
	return t->GetValue (TranslateTransform::XProperty)->AsDouble();
}

void
translate_transform_set_y (TranslateTransform *t, double y)
{
	t->SetValue (TranslateTransform::YProperty, Value(y));
}

double
translate_transform_get_y (TranslateTransform *t)
{
	return t->GetValue (TranslateTransform::YProperty)->AsDouble();
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
		cairo_matrix_translate (value, cx, cy);
		cairo_matrix_scale (value, sx, sy);
		cairo_matrix_translate (value, -cx, -cy);
	}
	//printf ("translate to %g %g -- %g %g\n", -cx, -cy, sx, sy);
	//printf ("Returning4 %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

ScaleTransform *
scale_transform_new ()
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
	printf ("Setting to %g\n", scaleY);
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



void
MatrixTransform::GetTransform (cairo_matrix_t *value)
{
	fprintf (stderr, "Error\n");
	exit (1);
#if notyet
  cairo_matrix_t matrix = matrix_transform_get_matrix (this);

  memcpy (value, &matrix, sizeof (cairo_matrix_t));
#endif
	printf ("Returning1 %g %g %g %g %g %g\n", value->xx, value->yx, value->xy, value->yy, value->x0, value->y0);
}

MatrixTransform *
matrix_transform_new ()
{
	return new MatrixTransform ();
}

void
TransformCollection::Add (void *data)
{
	Value *v = (Value *) data;
	Transform *transform = v->AsTransform ();

	Collection::Add (transform);
}

void
TransformCollection::Remove (void *data)
{
	Value *v = (Value *) data;
	Transform *transform = v->AsTransform ();

	Collection::Remove (transform);
}

DependencyProperty* TransformGroup::ChildrenProperty;

TransformGroup::TransformGroup ()
{
	children = NULL;
	TransformCollection *c = new TransformCollection ();

	this->SetValue (TransformGroup::ChildrenProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == children);
}

void
TransformGroup::OnPropertyChanged (DependencyProperty *prop)
{
	Transform::OnPropertyChanged (prop);

	if (prop == ChildrenProperty){
		// The new value has already been set, so unref the old collection

		TransformCollection *newcol = GetValue (prop)->AsTransformCollection();

		if (newcol != children){
			if (children){
				for (GSList *l = children->list; l != NULL; l = l->next){
					DependencyObject *dob = (DependencyObject *) l->data;
					
					base_unref (dob);
				}
				base_unref (children);
				g_slist_free (children->list);
			}

			children = newcol;
			if (children->closure)
				printf ("Warning we attached a property that was already attached\n");
			children->closure = this;
			
			base_ref (children);
		}
	}
}

void
TransformGroup::GetTransform (cairo_matrix_t *value)
{
	cairo_matrix_init_identity (value);
	for (GSList *w = children->list; w != NULL; w = w->next) {
		cairo_matrix_t child;
		Transform *t = (Transform *) w->data;
		t->GetTransform (&child);
		cairo_matrix_multiply (value, value, &child);
	}
}

TransformGroup *transform_group_new ()
{
	return new TransformGroup ();
}

void
transform_init ()
{
	/* RotateTransform fields */
	RotateTransform::AngleProperty   = DependencyObject::Register (Value::ROTATETRANSFORM, "Angle", new Value (0.0));
	RotateTransform::CenterXProperty = DependencyObject::Register (Value::ROTATETRANSFORM, "CenterX", new Value (0.0));
	RotateTransform::CenterYProperty = DependencyObject::Register (Value::ROTATETRANSFORM, "CenterY", new Value (0.0));
  
	/* TranslateTransform fields */
	TranslateTransform::XProperty = DependencyObject::Register (Value::TRANSLATETRANSFORM, "X", new Value (0.0));
	TranslateTransform::YProperty = DependencyObject::Register (Value::TRANSLATETRANSFORM, "Y", new Value (0.0));

	/* ScaleTransform fields */
	ScaleTransform::ScaleXProperty = DependencyObject::Register (Value::SCALETRANSFORM, "ScaleX", new Value (1.0));
	ScaleTransform::ScaleYProperty = DependencyObject::Register (Value::SCALETRANSFORM, "ScaleY", new Value (1.0));
	ScaleTransform::CenterXProperty = DependencyObject::Register (Value::SCALETRANSFORM, "CenterX", new Value (0.0));
	ScaleTransform::CenterYProperty = DependencyObject::Register (Value::SCALETRANSFORM, "CenterY", new Value (0.0));

	/* XXX MatrixTransform fields */

	/* TransformGroup fields */
	TransformGroup::ChildrenProperty = DependencyObject::Register (Value::TRANSFORMGROUP, "Children", Value::TRANSFORM_COLLECTION);
}
