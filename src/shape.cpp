/*
 * shape.cpp: This match the classes inside System.Windows.Shapes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <cairo.h>
#include <math.h>

#include "runtime.h"
#include "shape.h"
#include "array.h"

#include <sys/time.h>
//
// SL-Cairo convertion and helper routines
//

static cairo_line_join_t
convert_line_join (PenLineJoin pen_line_join)
{
	switch (pen_line_join) {
	case PenLineJoinMiter:
		return CAIRO_LINE_JOIN_MITER;
	case PenLineJoinBevel:
		return CAIRO_LINE_JOIN_BEVEL;
	case PenLineJoinRound:
		return CAIRO_LINE_JOIN_ROUND;
	default:
		/* g++ is stupid */
		g_assert_not_reached ();
	}
}

static cairo_line_cap_t
convert_line_cap (PenLineCap pen_line_cap)
{
	switch (pen_line_cap) {
	case PenLineCapFlat:
		return CAIRO_LINE_CAP_BUTT;
	case PenLineCapSquare:
		return CAIRO_LINE_CAP_SQUARE;
	case PenLineCapRound:
	case PenLineCapTriangle: 		/* FIXME: Triangle doesn't exist in Cairo */
		return CAIRO_LINE_CAP_ROUND;
	default:
		/* g++ is stupid */
		g_assert_not_reached ();
	}
}

cairo_fill_rule_t
convert_fill_rule (FillRule fill_rule)
{
	switch (fill_rule) {
	case FillRuleEvenOdd:
		return CAIRO_FILL_RULE_EVEN_ODD;
	case FillRuleNonzero:
		return CAIRO_FILL_RULE_WINDING;
	default:
		/* g++ is stupid */
		g_assert_not_reached ();
	}
}

void
moon_ellipse (cairo_t *cr, double x, double y, double w, double h)
{
	double rx = w / 2;
	double ry = h / 2;
	double cx = x + rx;
	double cy = y + ry;

	// cairo_new_sub_path (cr); unrequired with the cairo_move_to call
	cairo_move_to (cr, cx + rx, cy);

	/* an approximate of the ellipse by drawing a curve in each
	 * quadrants */
	cairo_curve_to (cr,
			cx + rx, cy - ARC_TO_BEZIER * ry,
			cx + ARC_TO_BEZIER * rx, cy - ry,
			cx, cy - ry);
        
	cairo_curve_to (cr,
			cx - ARC_TO_BEZIER * rx, cy - ry,
			cx - rx, cy - ARC_TO_BEZIER * ry,
			cx - rx, cy);

	cairo_curve_to (cr,
			cx - rx, cy + ARC_TO_BEZIER * ry,
			cx - ARC_TO_BEZIER * rx, cy + ry,
			cx, cy + ry);
                
	cairo_curve_to (cr,
			cx + ARC_TO_BEZIER * rx, cy + ry,
			cx + rx, cy + ARC_TO_BEZIER * ry,
			cx + rx, cy);

	cairo_close_path (cr);
}

void
moon_rounded_rectangle (cairo_t *cr, double x, double y, double w, double h, double radius_x, double radius_y)
{
	if (radius_x < 0.0)
		radius_x = -radius_x;
	if (radius_y < 0.0)
		radius_y = -radius_y;

	// test limits (without using multiplications)
	if (radius_x > w - radius_x)
		radius_x = w / 2;
	if (radius_y > h - radius_y)
		radius_y = h / 2;

	// approximate (quite close) the arc using a bezier curve
	double c1 = ARC_TO_BEZIER * radius_x;
	double c2 = ARC_TO_BEZIER * radius_y;

	// cairo_new_sub_path (cr); unrequired with the cairo_move_to call
	cairo_move_to (cr, x + radius_x, y);
	cairo_rel_line_to (cr, w - 2 * radius_x, 0.0);
	cairo_rel_curve_to (cr, c1, 0.0, radius_x, c2, radius_x, radius_y);
	cairo_rel_line_to (cr, 0, h - 2 * radius_y);
	cairo_rel_curve_to (cr, 0.0, c2, c1 - radius_x, radius_y, -radius_x, radius_y);
	cairo_rel_line_to (cr, -w + 2 * radius_x, 0);
	cairo_rel_curve_to (cr, -c1, 0, -radius_x, -c2, -radius_x, -radius_y);
	cairo_rel_line_to (cr, 0, -h + 2 * radius_y);
	cairo_rel_curve_to (cr, 0.0, -c2, radius_x - c1, -radius_y, radius_x, -radius_y);
	cairo_close_path (cr);
}

//
// Shape
//

DependencyProperty* Shape::FillProperty;
DependencyProperty* Shape::StretchProperty;
DependencyProperty* Shape::StrokeProperty;
DependencyProperty* Shape::StrokeDashArrayProperty;
DependencyProperty* Shape::StrokeDashCapProperty;
DependencyProperty* Shape::StrokeDashOffsetProperty;
DependencyProperty* Shape::StrokeEndLineCapProperty;
DependencyProperty* Shape::StrokeLineJoinProperty;
DependencyProperty* Shape::StrokeMiterLimitProperty;
DependencyProperty* Shape::StrokeStartLineCapProperty;
DependencyProperty* Shape::StrokeThicknessProperty;

Shape::Shape ()
{
	stroke = NULL;
	fill = NULL;
	path = NULL;
	SetShapeFlags (UIElement::SHAPE_NORMAL);
}

Shape::~Shape ()
{
	if (stroke != NULL) {
		stroke->Detach (NULL, this);
		stroke->unref ();
	}
	
	if (fill != NULL) {
		fill->Detach (NULL, this);
		fill->unref ();
	}
	if (path) {
		cairo_path_destroy (path);
	}
}

void
Shape::Draw (cairo_t *cr)
{
	if (path) {
		cairo_new_path (cr);
		cairo_append_path (cr, path);
	} else {
		BuildPath (cr);
	}
}

//
// This routine is useful for Shape derivatives: it can be used
// to either get the bounding box from cairo, or to paint it
//
void 
Shape::DoDraw (cairo_t *cr, bool do_op, bool consider_fill)
{
	cairo_set_matrix (cr, &absolute_xform);

	if (IsEmpty ()) {
		cairo_new_path (cr);
		return;
	}

// 	printf ("Draw, xform: %g %g %g %g %g %g\n", 
// 		absolute_xform.xy,
// 		absolute_xform.xx,
// 		absolute_xform.yx,
// 		absolute_xform.yy,
// 		absolute_xform.x0,
// 		absolute_xform.y0);

	bool drawn = false;

	// we need to use clipping to implement StretchUniformToFill
	if (shape_get_stretch (this) == StretchUniformToFill) {
		double w = framework_element_get_width (this);
		if (w > 0.0) {
			double h = framework_element_get_height (this);
			if (h > 0.0) {
				cairo_rectangle (cr, 0, 0, w, h);
				cairo_clip (cr);
				cairo_new_path (cr);
			}
		}
	}

	// getting bounds, using cairo_stroke_extents, doesn't requires us to fill (consider_fill)
	// unless there is no stroke brush assigned, which requires us to fill and use cairo_fill_extents
	// also not every shapes can be filled, e.g. polylines, CallFill
	if (!IsDegenerate () && (consider_fill || !stroke) && IsFilled ()) {
		if (fill) {
			Draw (cr);
			drawn = true;
			if (do_op) {
				fill->SetupBrush (cr, this);
				cairo_set_fill_rule (cr, convert_fill_rule (GetFillRule ()));
				cairo_fill_preserve (cr);
			}
		}
	}
	
	if (stroke) {
		double thickness = shape_get_stroke_thickness (this);
		if (thickness == 0) {
			if (drawn)
				cairo_new_path (cr);
			return;
		}

		if (IsDegenerate ())
			cairo_set_line_width (cr, 1.0);
		else
			cairo_set_line_width (cr, thickness);

		int count = 0;
		double *dashes = shape_get_stroke_dash_array (this, &count);
		if (dashes && (count > 0)) {
			/* FIXME: cairo doesn't support line cap for dashes */
			double offset = shape_get_stroke_dash_offset (this) * thickness;
			// special case or cairo stops drawing
			if ((count == 1) && (*dashes == 0.0)) {
				if (drawn)
					cairo_new_path (cr);
				return;
			}

			// multiply dashes length with thickness
			double *dmul = new double [count];
			for (int i=0; i < count; i++) {
				dmul [i] = dashes [i] * thickness;
			}
			cairo_set_dash (cr, dmul, count, offset);
			delete dmul;
		} else {
			cairo_set_dash (cr, NULL, 0, 0.0);
		}

		cairo_set_miter_limit (cr, shape_get_stroke_miter_limit (this));
		cairo_set_line_join (cr, convert_line_join (shape_get_stroke_line_join (this)));

		/* FIXME: cairo doesn't have separate line cap for the start and end */
		PenLineCap cap = shape_get_stroke_end_line_cap (this);
		if (cap == PenLineCapFlat) {
			cap = shape_get_stroke_start_line_cap (this);
		}
		cairo_set_line_cap (cr, convert_line_cap (cap));

		if (!drawn)
			Draw (cr);
		if (do_op) {
			stroke->SetupBrush (cr, this);
			if (IsDegenerate ())
				cairo_fill_preserve (cr);
			cairo_stroke (cr);
		}
	}
	else {
		if (drawn && do_op)
			cairo_new_path (cr);
	}
}

void
Shape::Render (cairo_t *cr, int x, int y, int width, int height)
{
	cairo_save (cr);
	DoDraw (cr, true, true);
	cairo_restore (cr);
}

void
Shape::ComputeBounds ()
{
	ComputeBoundsFast ();
}

void
Shape::ComputeBoundsFast ()
{
	bounds = Rect (0,0,0,0);

	Stretch stretch = shape_get_stretch (this);

	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);

	if ((w < 0.0) || (h < 0.0))
		return;

	switch (stretch) {
	case StretchUniform:
		w = h = (w < h) ? w : h;
		break;
	case StretchUniformToFill:
		w = h = (w > h) ? w : h;
		break;
	case StretchFill:
		/* nothing needed here.  the assignment of w/h above
		   is correct for this case. */
		break;
	case StretchNone:
		break;
	}

	if ((w == 0.0) && (h == 0.0))
		w = h = shape_get_stroke_thickness (this);

	if (w != 0.0 && h != 0.0) {

		bounds = bounding_rect_for_transformed_rect (&absolute_xform,
							     Rect (0,0,w,h));

		//printf ("%f,%f,%f,%f\n", bounds.x, bounds.y, bounds.w, bounds.h);
	}

	/* standard "grow the rectangle by enough to cover our
	   asses because of cairo's floating point rendering"
	   thing */
	bounds.GrowBy(1);
}

void
Shape::ComputeBoundsSlow ()
{
	double x1, y1, x2, y2;
	cairo_t* cr = measuring_context_create ();

	cairo_save (cr);
	// dont do the operation
	DoDraw (cr, false, true);

	// XXX this next call will hopefully become unnecessary in a
	// later version of cairo.
	cairo_identity_matrix (cr);
	if (stroke)
		cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	else
		cairo_fill_extents (cr, &x1, &y1, &x2, &y2);

	cairo_restore (cr);

	bounds = Rect (x1, y1, x2-x1, y2-y1);

	bounds.GrowBy (1);

	measuring_context_destroy (cr);
}

void
Shape::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	double x1, y1, x2, y2;
	
	cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	
	*height = fabs (y2 - y1);
	*width = fabs (x2 - x1);
}

bool
Shape::InsideObject (cairo_t *cr, double x, double y)
{
	bool ret = false;

	cairo_save (cr);
	// don't do the operation but do consider filling
	DoDraw (cr, false, true);
	double nx = x;
	double ny = y;

	cairo_matrix_t inverse = absolute_xform;
	cairo_matrix_invert (&inverse);

	cairo_matrix_transform_point (&inverse, &nx, &ny);

	if (cairo_in_stroke (cr, nx, ny) || (IsFilled () && cairo_in_fill (cr, nx, ny)))
		ret = TRUE;
	
	cairo_new_path (cr);

	cairo_restore (cr);
	return ret;
}

void
Shape::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::SHAPE) {
		if ((prop == FrameworkElement::HeightProperty) || (prop == FrameworkElement::WidthProperty))
			InvalidatePathCache ();

		FrameworkElement::OnPropertyChanged (prop);
		return;
	}

	if (prop == Shape::StretchProperty) {
		InvalidatePathCache ();
		UpdateBounds ();
	}
	else if (prop == Shape::StrokeProperty) {
		if (stroke != NULL) {
			stroke->Detach (NULL, this);
			stroke->unref ();
		}
		
		if ((stroke = shape_get_stroke (this)) != NULL) {
			stroke->Attach (NULL, this);
			stroke->ref ();
		}

		UpdateBounds ();
	} else if (prop == Shape::FillProperty) {
		if (fill != NULL) {
			fill->Detach (NULL, this);
			fill->unref ();
		}
		
		if ((fill = shape_get_fill (this)) != NULL) {
			fill->Attach (NULL, this);
			fill->ref ();
		}

		UpdateBounds ();
	} else if (prop == Shape::StrokeThicknessProperty) {
		InvalidatePathCache ();
		UpdateBounds ();
	} else if (prop == Shape::StrokeDashCapProperty
		   || prop == Shape::StrokeEndLineCapProperty
		   || prop == Shape::StrokeLineJoinProperty
		   || prop == Shape::StrokeMiterLimitProperty
		   || prop == Shape::StrokeStartLineCapProperty) {
		UpdateBounds ();
	}
	
	Invalidate ();

	NotifyAttacheesOfPropertyChange (prop);
}

void
Shape::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == Shape::FillProperty || prop == Shape::StrokeProperty) {
		Invalidate ();
	}
	else
		FrameworkElement::OnSubPropertyChanged (prop, subprop);
}

Point
Shape::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		framework_element_get_height (this) * user_xform_origin.y);
}


void
Shape::InvalidatePathCache ()
{
	SetShapeFlags (UIElement::SHAPE_NORMAL);
	if (path) {
		cairo_path_destroy (path);
		path = NULL;
	}
}

Brush *
shape_get_fill (Shape *shape)
{
	Value *value = shape->GetValue (Shape::FillProperty);
	return (value ? value->AsBrush() : NULL);
}

void 
shape_set_fill (Shape *shape, Brush *value)
{
	shape->SetValue (Shape::FillProperty, Value (value));
}

Brush *
shape_get_stroke (Shape *shape)
{
	Value *value = shape->GetValue (Shape::StrokeProperty);
	return (value ? value->AsBrush() : NULL);
}

void 
shape_set_stroke (Shape *shape, Brush *value)
{
	shape->SetValue (Shape::StrokeProperty, Value (value));
}

Stretch
shape_get_stretch (Shape *shape)
{
	return (Stretch) shape->GetValue (Shape::StretchProperty)->AsInt32();
}

void
shape_set_stretch (Shape *shape, Stretch value)
{
	shape->SetValue (Shape::StretchProperty, Value (value));
}

PenLineCap
shape_get_stroke_dash_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeDashCapProperty)->AsInt32();
}

void
shape_set_stroke_dash_cap (Shape *shape, PenLineCap value)
{
	shape->SetValue (Shape::StrokeDashCapProperty, Value (value));
}

PenLineCap
shape_get_stroke_start_line_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeStartLineCapProperty)->AsInt32();
}

void
shape_set_stroke_start_line_cap (Shape *shape, PenLineCap value)
{
	shape->SetValue (Shape::StrokeStartLineCapProperty, Value (value));
}

PenLineCap
shape_get_stroke_end_line_cap (Shape *shape)
{
	return (PenLineCap) shape->GetValue (Shape::StrokeEndLineCapProperty)->AsInt32();
}

void
shape_set_stroke_end_line_cap (Shape *shape, PenLineCap value)
{
	shape->SetValue (Shape::StrokeEndLineCapProperty, Value (value));
}

double
shape_get_stroke_dash_offset (Shape *shape)
{
	return shape->GetValue (Shape::StrokeDashOffsetProperty)->AsDouble();
}

void
shape_set_stroke_dash_offset (Shape *shape, double value)
{
	shape->SetValue (Shape::StrokeDashOffsetProperty, Value (value));
}

double
shape_get_stroke_miter_limit (Shape *shape)
{
	return shape->GetValue (Shape::StrokeMiterLimitProperty)->AsDouble();
}

void
shape_set_stroke_miter_limit (Shape *shape, double value)
{
	shape->SetValue (Shape::StrokeMiterLimitProperty, Value (value));
}

double
shape_get_stroke_thickness (Shape *shape)
{
	return shape->GetValue (Shape::StrokeThicknessProperty)->AsDouble();
}

void
shape_set_stroke_thickness (Shape *shape, double value)
{
	shape->SetValue (Shape::StrokeThicknessProperty, Value (value));
}

PenLineJoin
shape_get_stroke_line_join (Shape *shape)
{
	return (PenLineJoin) shape->GetValue (Shape::StrokeLineJoinProperty)->AsInt32();
}

void
shape_set_stroke_line_join (Shape *shape, PenLineJoin value)
{
	shape->SetValue (Shape::StrokeLineJoinProperty, Value (value));
}

/*
 * note: We return a reference, not a copy, of the dashes. Not a big issue as
 * Silverlight Shape.StrokeDashArray only has a setter (no getter), so it's 
 * use is only internal.
 */
double*
shape_get_stroke_dash_array (Shape *shape, int *count)
{
	Value *value = shape->GetValue (Shape::StrokeDashArrayProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	DoubleArray *da = value->AsDoubleArray();
	*count = da->basic.count;
	return da->values;
}

void
shape_set_stroke_dash_array (Shape *shape, double* dashes, int count)
{
	shape->SetValue (Shape::StrokeDashArrayProperty, Value (dashes, count));
}

//
// Ellipse
//

Ellipse::Ellipse ()
{
	SetValue (Shape::StretchProperty, Value (StretchFill));
}

void
Ellipse::BuildPath (cairo_t *cr)
{
	Stretch stretch = shape_get_stretch (this);
	if (stretch == StretchNone) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	double x = 0.0;
	double y = 0.0;
	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);

	if ((w < 0.0) || (h < 0.0)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	double t = shape_get_stroke_thickness (this);
	double t2 = t * 2.0;

	if ((t2 > w) || (t2 > h)) {
		if (w < t)
			w = t;
		if (h < t)
			h = t;
		SetShapeFlags (UIElement::SHAPE_DEGENERATE);
	} else {
		double half = t / 2;
		x = y = half;
		w -= t;
		h -= t;
		SetShapeFlags (UIElement::SHAPE_NORMAL);
	}

	switch (stretch) {
	case StretchUniform:
		w = h = (w < h) ? w : h;
		break;
	case StretchUniformToFill:
		// this gets an ellipse larger than it's dimension, relative
		// scaling is ok but we need Shape::Draw to clip to it's original size
		w = h = (w > h) ? w : h;
		break;
	case StretchFill:
		/* nothing needed here.  the assignment of w/h above
		   is correct for this case. */
		break;
	case StretchNone:
		/* not reached */
		break;
	}

	cairo_new_path (cr);
	if (IsDegenerate ()) {
		double radius = t / 2;
		moon_rounded_rectangle (cr, x, y, w, h, radius, radius);
	} else {
		moon_ellipse (cr, x, y, w, h);
	}
	// note: both moon_rounded_rectangle and moon_ellipse calls cairo_close_path
	path = cairo_copy_path (cr);
}

Ellipse *
ellipse_new (void)
{
	return new Ellipse ();
}

//
// Rectangle
//

DependencyProperty* Rectangle::RadiusXProperty;
DependencyProperty* Rectangle::RadiusYProperty;

Rectangle::Rectangle ()
{
	SetValue (Shape::StretchProperty, Value (StretchFill));
}

/*
 * rendering notes:
 * - a Width="0" or a Height="0" can be rendered differently from not specifying Width or Height
 * - if a rectangle has only a Width or only a Height it is NEVER rendered
 */
void
Rectangle::BuildPath (cairo_t *cr)
{
	Value *width = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *height = GetValueNoDefault (FrameworkElement::HeightProperty);

	// nothing is drawn if only the width or only the height is specified
	if ((!width && height) || (width && !height)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	Stretch stretch = shape_get_stretch (this);
	double t = IsStroked () ? shape_get_stroke_thickness (this) : 0.0;

	// nothing is drawn (nor filled) if no StrokeThickness="0"
	// unless both Width and Height are specified or when no streching is required
	if ((t == 0.0) && (!width || !height || (stretch == StretchNone))) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	double x = 0.5, y = 0.5;
	double w, h;
	double radius_x, radius_y;
	bool compute_origin = false;
	// degenerate cases are handled differently for round-corner rectangles
	bool round = GetRadius (&radius_x, &radius_y);

	cairo_new_path (cr);

	// if both width and height are missing then the width and height are equal (on screen) to the thickness
	if ((!width && !height) || (stretch == StretchNone)) {
		// don't make invisible points
		x = 0.5;
		if (t <= 1.0) {
			if (t > 0.0)
				y = 0.0;
			w = h = 0.5;
		} else {
			w = h = (t - 1.0);
		}

		SetShapeFlags (UIElement::SHAPE_DEGENERATE);
		goto shape;
	}

	w = width->AsDouble ();
	h = height->AsDouble ();

	if ((w < 0.0) || (h < 0.0)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	// there are two kinds of degenerations 
	// (a) the thickness is larger (or equal) to the width or height
	if ((t > w) || (t > h)) {
		// in this case we must adjust the values to make a (much) larger rectangle
		x -= t / 2.0;
		y -= t / 2.0 - 1.0;
		if (stretch == StretchUniform) {
			w = h = ((w < h) ? w : h) + t;
		} else {
			w += (t - 1.0);
			h += (t - 1.0);
			if (round)
				radius_x = radius_y = t / 2;
		}
		SetShapeFlags (UIElement::SHAPE_DEGENERATE);
		goto shape;
	} else {
		// (b) the thickness is larger (or equal) to half the width or half the height
		double t2 = t * 2.0;
		if ((t2 >= w) || (t2 >= h)) {
			x = 0.5;
			if (stretch == StretchUniform) {
				w = h = (w < h) ? w : h;
			} else {
				w -= 1.0;
				h -= 1.0;
				if (round) {
					radius_x = w;
					radius_y = h;
				}
			}
			SetShapeFlags (UIElement::SHAPE_DEGENERATE);
			goto shape;
		}
	}

	// both Width and Height are specified
	if (stretch != StretchNone) {
		compute_origin = true;
		if (t > w - t) {
			t = w / 2.0;
		}
		w -= t;
		if (t > h - t) {
			t = h / 2.0;
		}
		h -= t;
	}

	switch (stretch) {
	case StretchNone:
		break;
	case StretchUniform:
		w = h = (w < h) ? w : h;
		break;
	case StretchUniformToFill:
		// this gets an rectangle larger than it's dimension, relative
		// scaling is ok but we need Shape::Draw to clip to it's original size
		w = h = (w > h) ? w : h;
		break;
	case StretchFill:
		/* nothing needed here.  the assignment of w/h above
		   is correct for this case. */
		break;
	}

	if ((!IsStroked () || (t != 0.0)) && compute_origin) {
		x = t / 2.0;
		y = x - 0.5;
	}
	SetShapeFlags (UIElement::SHAPE_NORMAL);

shape:
	// rounded-corner rectangle ?
	if (round) {
		moon_rounded_rectangle (cr, x, y, w, h, radius_x, radius_y);
	} else {
		cairo_rectangle (cr, x, y, w, h);
	}

	// no need to call cairo_close_path (cr) since both mono_rounded_rectangle and cairo_rectangle calls it
	path = cairo_copy_path (cr);
}

void
Rectangle::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::RECTANGLE) {
		Shape::OnPropertyChanged (prop);
		return;
	}

	if ((prop == Rectangle::RadiusXProperty) || (prop == Rectangle::RadiusYProperty))
		InvalidatePathCache ();
	
	Invalidate ();

	NotifyAttacheesOfPropertyChange (prop);
}

bool
Rectangle::GetRadius (double *rx, double *ry)
{
	Value *value = GetValueNoDefault (Rectangle::RadiusXProperty);
	if (!value)
		return false;
	*rx = value->AsDouble ();

	value = GetValueNoDefault (Rectangle::RadiusYProperty);
	if (!value)
		return false;
	*ry = value->AsDouble ();

	return ((*rx != 0.0) && (*ry != 0.0));
}

double
rectangle_get_radius_x (Rectangle *rectangle)
{
	return rectangle->GetValue (Rectangle::RadiusXProperty)->AsDouble();
}

void
rectangle_set_radius_x (Rectangle *rectangle, double value)
{
	rectangle->SetValue (Rectangle::RadiusXProperty, Value (value));
}

double
rectangle_get_radius_y (Rectangle *rectangle)
{
	return rectangle->GetValue (Rectangle::RadiusYProperty)->AsDouble();
}

void
rectangle_set_radius_y (Rectangle *rectangle, double value)
{
	rectangle->SetValue (Rectangle::RadiusYProperty, Value (value));
}

Rectangle *
rectangle_new (void)
{
	return new Rectangle ();
}

//
// Line
//

DependencyProperty* Line::X1Property;
DependencyProperty* Line::Y1Property;
DependencyProperty* Line::X2Property;
DependencyProperty* Line::Y2Property;

void
Line::ComputeBounds ()
{
#if notyet
	double x1 = line_get_x1 (this);
	double y1 = line_get_y1 (this);
	double x2 = line_get_x2 (this);
	double y2 = line_get_y2 (this);

	bounds = bounding_rect_for_transformed_rect (&absolute_xform,
						     Rect (MIN(x1,x2), MIN(y1,y2),
							   fabs (x2-x1), fabs (y2-y1)));

	bounds.GrowBy (shape_get_stroke_thickness (this) + 1);
#else
	Shape::ComputeBounds ();
#endif
}

void
Line::Draw (cairo_t *cr)
{
	// Note: Shape::StretchProperty has no effect on lines
	// Note: SL 1.1 alpha considers X2,Y2 as the start point (when drawing end line caps)
	//	This doesn't affect us because Cairo doesn't support separate start/end line caps
	cairo_move_to (cr, line_get_x1 (this), line_get_y1 (this));
	cairo_line_to (cr, line_get_x2 (this), line_get_y2 (this));
}

void
Line::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::LINE) {
		Shape::OnPropertyChanged (prop);
		return;
	}

	FullInvalidate (false);

	NotifyAttacheesOfPropertyChange (prop);
}

double
line_get_x1 (Line *line)
{
	return line->GetValue (Line::X1Property)->AsDouble();
}

void
line_set_x1 (Line *line, double value)
{
	line->SetValue (Line::X1Property, Value (value));
}

double
line_get_y1 (Line *line)
{
	return line->GetValue (Line::Y1Property)->AsDouble();
}

void
line_set_y1 (Line *line, double value)
{
	line->SetValue (Line::Y1Property, Value (value));
}

double
line_get_x2 (Line *line)
{
	return line->GetValue (Line::X2Property)->AsDouble();
}

void
line_set_x2 (Line *line, double value)
{
	line->SetValue (Line::X2Property, Value (value));
}

double
line_get_y2 (Line *line)
{
	return line->GetValue (Line::Y2Property)->AsDouble();
}

void
line_set_y2 (Line *line, double value)
{
	line->SetValue (Line::Y2Property, Value (value));
}

Line *
line_new (void)
{
	return new Line ();
}

//
// Polygon
//

DependencyProperty* Polygon::FillRuleProperty;
DependencyProperty* Polygon::PointsProperty;

FillRule
Polygon::GetFillRule ()
{
	return polygon_get_fill_rule (this);
}

void
Polygon::ComputeBounds ()
{
	Shape::ComputeBoundsSlow ();
}

void
Polygon::BuildPath (cairo_t *cr)
{
	int i, count = 0;
	Point *points = polygon_get_points (this, &count);

	if (!points || (count < 1)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	SetShapeFlags (UIElement::SHAPE_NORMAL);
	cairo_new_path (cr);

	Stretch stretch = shape_get_stretch (this);
	switch (stretch) {
	case StretchNone:
		cairo_move_to (cr, points [0].x, points [0].y);
		for (i = 1; i < count; i++)
			cairo_line_to (cr, points [i].x, points [i].y);
		break;
	default:
		double x = points [0].x;
		double y = points [0].y;
		cairo_move_to (cr, 0, 0);
		for (i = 1; i < count; i++)
			cairo_line_to (cr, points [i].x - x, points [i].y - y);
		break;
	}

	cairo_close_path (cr);
	path = cairo_copy_path (cr);
}

void
Polygon::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::POLYGON) {
		Shape::OnPropertyChanged (prop);
		return;
	}

	if (prop == Polygon::PointsProperty) {
		InvalidatePathCache ();
		UpdateBounds (true /* force one here, even if the bounds don't change */);
	}

	Invalidate ();
	NotifyAttacheesOfPropertyChange (prop);
}

void
Polygon::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	UpdateBounds (true);
}

FillRule
polygon_get_fill_rule (Polygon *polygon)
{
	return (FillRule) polygon->GetValue (Polygon::FillRuleProperty)->AsInt32();
}

void
polygon_set_fill_rule (Polygon *polygon, FillRule value)
{
	polygon->SetValue (Polygon::FillRuleProperty, Value (value));
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight Polygon.Points only has a setter (no getter), so it's use is 
 * only internal.
 */
Point *
polygon_get_points (Polygon *polygon, int *count)
{
	Value *value = polygon->GetValue (Polygon::PointsProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	PointArray *pa = value->AsPointArray();
	*count = pa->basic.count;
	return pa->points;
}

void
polygon_set_points (Polygon *polygon, Point* points, int count)
{
	polygon->SetValue (Polygon::PointsProperty, Value (points, count));
}

Polygon *
polygon_new (void)
{
	return new Polygon ();
}

//
// Polyline
//

DependencyProperty* Polyline::FillRuleProperty;
DependencyProperty* Polyline::PointsProperty;

FillRule
Polyline::GetFillRule ()
{
	return polyline_get_fill_rule (this);
}

void
Polyline::ComputeBounds ()
{
	Shape::ComputeBoundsSlow ();
}

void
Polyline::BuildPath (cairo_t *cr)
{
	int i, count = 0;
	Point *points = polyline_get_points (this, &count);

	if (!points || (count < 1)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	SetShapeFlags (UIElement::SHAPE_NORMAL);
	cairo_new_path (cr);

	Stretch stretch = shape_get_stretch (this);
	switch (stretch) {
	case StretchNone:
		cairo_move_to (cr, points [0].x, points [0].y);
		for (i = 1; i < count; i++)
			cairo_line_to (cr, points [i].x, points [i].y);
		break;
	default:
		double x = points [0].x;
		double y = points [0].y;
		cairo_move_to (cr, 0, 0);
		for (i = 1; i < count; i++)
			cairo_line_to (cr, points [i].x - x, points [i].y - y);
		break;
	}
}

void
Polyline::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::POLYLINE) {
		Shape::OnPropertyChanged (prop);
		return;
	}

	if (prop == Polyline::PointsProperty) {
		InvalidatePathCache ();
		UpdateBounds (true /* force one here, even if the bounds don't change */);
	}

	Invalidate ();
	NotifyAttacheesOfPropertyChange (prop);
}

void
Polyline::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	UpdateBounds ();
	Invalidate ();
}

FillRule
polyline_get_fill_rule (Polyline *polyline)
{
	return (FillRule) polyline->GetValue (Polyline::FillRuleProperty)->AsInt32();
}

void
polyline_set_fill_rule (Polyline *polyline, FillRule value)
{
	polyline->SetValue (Polyline::FillRuleProperty, Value (value));
}

/*
 * note: We return a reference, not a copy, of the points. Not a big issue as
 * Silverlight Polyline.Points only has a setter (no getter), so it's use is 
 * only internal.
 */
Point *
polyline_get_points (Polyline *polyline, int *count)
{
	Value *value = polyline->GetValue (Polyline::PointsProperty);
	if (!value) {
		*count = 0;
		return NULL;
	}

	PointArray *pa = value->AsPointArray();
	*count = pa->basic.count;
	return pa->points;
}

void
polyline_set_points (Polyline *polyline, Point* points, int count)
{
	polyline->SetValue (Polyline::PointsProperty, Value (points, count));
}

Polyline *
polyline_new (void)
{
	return new Polyline ();
}

//
// Path
//

DependencyProperty* Path::DataProperty;

FillRule
Path::GetFillRule ()
{
	Geometry* geometry = path_get_data (this);
	return geometry ? geometry_get_fill_rule (geometry) : Shape::GetFillRule ();
}

void
Path::ComputeBounds()
{
	Shape::ComputeBoundsSlow ();
}

void
Path::BuildPath (cairo_t *cr)
{
	Geometry* geometry = path_get_data (this);
	if (!geometry) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);
	if ((w < 0.0) || (h < 0.0)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	SetShapeFlags (UIElement::SHAPE_NORMAL);
	geometry->Draw (this, cr);

	path = cairo_copy_path (cr);

	Stretch stretch = shape_get_stretch (this);
	if (stretch == StretchNone)
		return;

	/* NOTE: this looks complex but avoid a *lot* of changes in geometry 
	 * (resulting in something even more complex).
	 */
	double minx = G_MAXDOUBLE;
	double miny = G_MAXDOUBLE;
	double maxx = G_MINDOUBLE;
	double maxy = G_MINDOUBLE;

	// find origin (minimums) and actual width/height (maximums - minimums)
	for (int i=0; i < path->num_data; i+= path->data[i].header.length) {
		cairo_path_data_t *data = &path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_CURVE_TO:
			// minimum
			if (minx > data[3].point.x)
				minx = data[3].point.x;
			if (miny > data[3].point.y)
				miny = data[3].point.y;
			if (minx > data[2].point.x)
				minx = data[2].point.x;
			if (miny > data[2].point.y)
				miny = data[2].point.y;
			// maximum
			if (maxx < data[3].point.x)
				maxx = data[3].point.x;
			if (maxy < data[3].point.y)
				maxy = data[3].point.y;
			if (maxx < data[2].point.x)
				maxx = data[2].point.x;
			if (maxy < data[2].point.y)
				maxy = data[2].point.y;
			/* fallthru */
		case CAIRO_PATH_LINE_TO:
		case CAIRO_PATH_MOVE_TO:
			// minimum
			if (minx > data[1].point.x)
				minx = data[1].point.x;
			if (miny > data[1].point.y)
				miny = data[1].point.y;
			// maximum
			if (maxx < data[1].point.x)
				maxx = data[1].point.x;
			if (maxy < data[1].point.y)
				maxy = data[1].point.y;
			break;
		case CAIRO_PATH_CLOSE_PATH:
			break;
		}
	}

	double actual_height = maxy - miny;
	double actual_width = maxx - minx;

	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);

	double sh = vh ? (h / actual_height) : 1.0;
	double sw = vw ? (w / actual_width) : 1.0;
	switch (stretch) {
	case StretchFill:
		break;
	case StretchUniform:
		sw = sh = (sw < sh) ? sw : sh;
		break;
	case StretchUniformToFill:
		sw = sh = (sw > sh) ? sw : sh;
		break;
	case StretchNone:
		/* not reached */
		break;
	}

	bool stretch_horz = (vw || (sw != 1.0));
	bool stretch_vert = (vh || (sh != 1.0));

	// substract origin (min[x|y]) and scale to requested dimensions (if specified)
	for (int i=0; i < path->num_data; i+= path->data[i].header.length) {
		cairo_path_data_t *data = &path->data[i];
		switch (data->header.type) {
		case CAIRO_PATH_CURVE_TO:
			data[3].point.x -= minx;
			data[3].point.y -= miny;
			data[2].point.x -= minx;
			data[2].point.y -= miny;
			if (stretch_horz) {
				data[3].point.x *= sw;
				data[2].point.x *= sw;
			}
			if (stretch_vert) {
				data[3].point.y *= sh;
				data[2].point.y *= sh;
			}
			/* fallthru */
		case CAIRO_PATH_LINE_TO:
		case CAIRO_PATH_MOVE_TO:
			data[1].point.x -= minx;
			data[1].point.y -= miny;
			if (stretch_horz)
				data[1].point.x *= sw;
			if (stretch_vert)
				data[1].point.y *= sh;
			break;
		case CAIRO_PATH_CLOSE_PATH:
			break;
		}
	}

	// path was modified, replay...
	cairo_new_path (cr);
	cairo_append_path (cr, path);
}

bool
Path::IsFilled ()
{
	Geometry* data = path_get_data (this);
	return (data ? data->IsFilled () : false);
}

void
Path::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::PATH) {
		Shape::OnPropertyChanged (prop);
		return;
	}

	InvalidatePathCache ();
	FullInvalidate (false);

	NotifyAttacheesOfPropertyChange (prop);
}

void
Path::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == Path::DataProperty) {
		InvalidatePathCache ();
		FullInvalidate (false);
	}
	else
		Shape::OnSubPropertyChanged (prop, subprop);
}

Geometry *
path_get_data (Path *path)
{
	Value *value = path->GetValue (Path::DataProperty);
	return (value ? value->AsGeometry() : NULL);
}

void
path_set_data (Path *path, Geometry *value)
{
	path->SetValue (Path::DataProperty, Value (value));
}

Path *
path_new (void)
{
	return new Path ();
}


//
// 
//

void
shape_init (void)
{
	/* Shape fields */
	Shape::FillProperty = DependencyObject::Register (Type::SHAPE, "Fill", Type::BRUSH);
	Shape::StretchProperty = DependencyObject::Register (Type::SHAPE, "Stretch", new Value (StretchNone));
	Shape::StrokeProperty = DependencyObject::Register (Type::SHAPE, "Stroke", Type::BRUSH);
	Shape::StrokeDashArrayProperty = DependencyObject::Register (Type::SHAPE, "StrokeDashArray", Type::DOUBLE_ARRAY);
	Shape::StrokeDashCapProperty = DependencyObject::Register (Type::SHAPE, "StrokeDashCap", new Value (PenLineCapFlat));
	Shape::StrokeDashOffsetProperty = DependencyObject::Register (Type::SHAPE, "StrokeDashOffset", new Value (0.0));
	Shape::StrokeEndLineCapProperty = DependencyObject::Register (Type::SHAPE, "StrokeEndLineCap", new Value (PenLineCapFlat));
	Shape::StrokeLineJoinProperty = DependencyObject::Register (Type::SHAPE, "StrokeLineJoin", new Value (PenLineJoinMiter));
	Shape::StrokeMiterLimitProperty = DependencyObject::Register (Type::SHAPE, "StrokeMiterLimit", new Value (10.0));
	Shape::StrokeStartLineCapProperty = DependencyObject::Register (Type::SHAPE, "StrokeStartLineCap", new Value (PenLineCapFlat));
	Shape::StrokeThicknessProperty = DependencyObject::Register (Type::SHAPE, "StrokeThickness", new Value (1.0));

	/* Rectangle fields */
	Rectangle::RadiusXProperty = DependencyObject::Register (Type::RECTANGLE, "RadiusX", new Value (0.0));
	Rectangle::RadiusYProperty = DependencyObject::Register (Type::RECTANGLE, "RadiusY", new Value (0.0));

	/* Line fields */
	Line::X1Property = DependencyObject::Register (Type::LINE, "X1", new Value (0.0));
	Line::Y1Property = DependencyObject::Register (Type::LINE, "Y1", new Value (0.0));
	Line::X2Property = DependencyObject::Register (Type::LINE, "X2", new Value (0.0));
	Line::Y2Property = DependencyObject::Register (Type::LINE, "Y2", new Value (0.0));

	/* Polygon fields */
	Polygon::FillRuleProperty = DependencyObject::Register (Type::POLYGON, "FillRule", new Value (FillRuleEvenOdd));
	Polygon::PointsProperty = DependencyObject::Register (Type::POLYGON, "Points", Type::POINT_ARRAY);

	/* Polyline fields */
	Polyline::FillRuleProperty = DependencyObject::Register (Type::POLYLINE, "FillRule", new Value (FillRuleEvenOdd));
	Polyline::PointsProperty = DependencyObject::Register (Type::POLYLINE, "Points", Type::POINT_ARRAY);

	/* Path fields */
	Path::DataProperty = DependencyObject::Register (Type::PATH, "Data", Type::GEOMETRY);
}
