/*
 * shape.cpp: This match the classes inside System.Windows.Shapes
 *
 * Authors:
 *   Miguel de Icaza (miguel@novell.com)
 *   Sebastien Pouliot  <sebastien@ximian.com>
 *   Stephane Delcroix  <sdelcroix@novell.com>
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
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
	default:
		/* note: invalid values should be trapped in SetValue (see bug #340799) */
		g_warning ("Invalid value (%d) specified for PenLineJoin, using default.", pen_line_join);
		/* at this stage we use the default value (Miter) for Shape */
	case PenLineJoinMiter:
		return CAIRO_LINE_JOIN_MITER;
	case PenLineJoinBevel:
		return CAIRO_LINE_JOIN_BEVEL;
	case PenLineJoinRound:
		return CAIRO_LINE_JOIN_ROUND;
	}
}

/* NOTE: Triangle doesn't exist in Cairo - unless you patched it using https://bugzilla.novell.com/show_bug.cgi?id=345892 */
#ifndef HAVE_CAIRO_LINE_CAP_TRIANGLE
	#define CAIRO_LINE_CAP_TRIANGLE		 CAIRO_LINE_CAP_ROUND
#endif

static cairo_line_cap_t
convert_line_cap (PenLineCap pen_line_cap)
{
	switch (pen_line_cap) {
	default:
		/* note: invalid values should be trapped in SetValue (see bug #340799) */
		g_warning ("Invalid value (%d) specified for PenLineCap, using default.", pen_line_cap);
		/* at this stage we use the default value (Flat) for Shape */
	case PenLineCapFlat:
		return CAIRO_LINE_CAP_BUTT;
	case PenLineCapSquare:
		return CAIRO_LINE_CAP_SQUARE;
	case PenLineCapRound:
		return CAIRO_LINE_CAP_ROUND;
	case PenLineCapTriangle: 		
		return CAIRO_LINE_CAP_TRIANGLE;
	}
}

cairo_fill_rule_t
convert_fill_rule (FillRule fill_rule)
{
	switch (fill_rule) {
	default:
		/* note: invalid values should be trapped in SetValue (see bug #340799) */
		g_warning ("Invalid value (%d) specified for FillRule, using default.", fill_rule);
		/* at this stage we use the default value (EvenOdd) for Geometry */
	case FillRuleEvenOdd:
		return CAIRO_FILL_RULE_EVEN_ODD;
	case FillRuleNonzero:
		return CAIRO_FILL_RULE_WINDING;
	}
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
	origin = Point (0, 0);
	cached_surface = NULL;
	SetShapeFlags (UIElement::SHAPE_NORMAL);
	cairo_matrix_init_identity (&stretch_transform);
}

Shape::~Shape ()
{
	// That also destroys the cached surface
	InvalidatePathCache (true);
}

bool
Shape::MixedHeightWidth (Value **height, Value **width)
{
	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);

	// nothing is drawn if only the width or only the height is specified
	if ((!vw && vh) || (vw && !vh)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return true;
	}

	if (width) *width = vw;
	if (height) *height = vh;
	return false;
}

void
Shape::Draw (cairo_t *cr)
{
	if (!path || (path->cairo.num_data == 0))
		BuildPath ();

	cairo_save (cr);
	cairo_transform (cr, &stretch_transform);

	cairo_new_path (cr);
	cairo_append_path (cr, &path->cairo);

	cairo_restore (cr);
}

// break up operations so we can exclude optional stuff, like:
// * StrokeStartLineCap & StrokeEndLineCap
// * StrokeLineJoin & StrokeMiterLimit
// * Fill

bool
Shape::SetupLine (cairo_t* cr)
{
	double thickness = shape_get_stroke_thickness (this);
	// check if something will be drawn or return 
	// note: override this method if cairo is used to compute bounds
	if (thickness == 0)
		return false;

	if (IsDegenerate ())
		cairo_set_line_width (cr, 1.0);
	else
		cairo_set_line_width (cr, thickness);

	return SetupDashes (cr, thickness);
}

bool
Shape::SetupDashes (cairo_t *cr, double thickness)
{
	return Shape::SetupDashes (cr, thickness, shape_get_stroke_dash_offset (this) * thickness);
}

bool
Shape::SetupDashes (cairo_t *cr, double thickness, double offset)
{
	int count = 0;
	double *dashes = shape_get_stroke_dash_array (this, &count);
	if (dashes && (count > 0)) {
		// NOTE: special case - if we continue cairo will stops drawing!
		if ((count == 1) && (*dashes == 0.0))
			return false;

		// multiply dashes length with thickness
		double *dmul = new double [count];
		for (int i=0; i < count; i++)
			dmul [i] = dashes [i] * thickness;

		cairo_set_dash (cr, dmul, count, offset);
		delete [] dmul;
	} else {
		cairo_set_dash (cr, NULL, 0, 0.0);
	}
	return true;
}

void
Shape::SetupLineCaps (cairo_t *cr)
{
	//Setting the cap to dash_cap. the endcaps (if different) are handled elsewhere
	PenLineCap cap = shape_get_stroke_dash_cap (this);
	cairo_set_line_cap (cr, convert_line_cap (cap));
}

void
Shape::SetupLineJoinMiter (cairo_t *cr)
{
	cairo_set_line_join (cr, convert_line_join (shape_get_stroke_line_join (this)));
	cairo_set_miter_limit (cr, shape_get_stroke_miter_limit (this));
}

// returns true if the path is set on the cairo, false if not
bool
Shape::Fill (cairo_t *cr, bool do_op)
{
	if (!fill)
		return false;

	Draw (cr);
	if (do_op) {
		fill->SetupBrush (cr, this);
		cairo_set_fill_rule (cr, convert_fill_rule (GetFillRule ()));
		cairo_fill_preserve (cr);
	}
	return true;
}

Rect
Shape::ComputeStretchBounds (Rect shape_bounds)
{
	Value *vh, *vw;

	if (Shape::MixedHeightWidth (&vh, &vw))
		return shape_bounds;

	double w = vw ? vw->AsDouble () : 0.0;
	double h = vh ? vh->AsDouble () : 0.0;
	
	if ((h < 0.0) || (w < 0.0)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return shape_bounds;
	}

	if (vh && (h <= 0.0) || vw && (w <= 0.0)) { 
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return shape_bounds;
	}

	h = (h == 0.0) ? shape_bounds.h : h;
	w = (w == 0.0) ? shape_bounds.w : w;

	if (h <= 0.0 || w <= 0.0 || shape_bounds.w <= 0.0 || shape_bounds.h <= 0.0) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return shape_bounds;
	}

	Stretch stretch = shape_get_stretch (this);
	if (stretch != StretchNone) {
		double sh = h / shape_bounds.h;
		double sw = w / shape_bounds.w;
		bool center = false;

		switch (stretch) {
		case StretchFill:
			break;
		case StretchUniform:
			sw = sh = (sw < sh) ? sw : sh;
			center = true;
			break;
		case StretchUniformToFill:
			sw = sh = (sw > sh) ? sw : sh;
			break;
		case StretchNone:
			/* not reached */
		break;
		}
		
		if (center)
			cairo_matrix_translate (&stretch_transform, w * 0.5, h * 0.5);
		cairo_matrix_scale (&stretch_transform, sw, sh);
		if (center)
			cairo_matrix_translate (&stretch_transform, -shape_bounds.w * 0.5, -shape_bounds.h * 0.5);
		
		if ((vh && vw) || !this->Is (Type::LINE))
			cairo_matrix_translate (&stretch_transform, -shape_bounds.x, -shape_bounds.y);

		// Double check our math
		cairo_matrix_t test = stretch_transform;
		if (cairo_matrix_invert (&test)) {
			g_warning ("Unable to compute stretch transform %f %f %f %f \n", sw, sh, shape_bounds.x, shape_bounds.y);
		}		
	}

	shape_bounds = shape_bounds.Transform (&stretch_transform);

	if (vh && vw) {
		shape_bounds.w = MIN (shape_bounds.w, vw->AsDouble () - shape_bounds.x);
		shape_bounds.h = MIN (shape_bounds.h, vh->AsDouble () - shape_bounds.y);
	}

	return shape_bounds;
}

void
Shape::Stroke (cairo_t *cr, bool do_op)
{
	if (do_op) {
		stroke->SetupBrush (cr, this);
		if (IsDegenerate ())
			cairo_fill_preserve (cr);
#if DONT_STROKE_DEGENERATES		
		else 
			cairo_stroke (cr);
#else
		cairo_stroke (cr);
#endif
	}
}

void
Shape::Clip (cairo_t *cr)
{
	// some shapes, like Line, Polyline, Polygon and Path, are clipped if both Height and Width properties are present
	if (ClipOnHeightAndWidth () || (!IsDegenerate () && (shape_get_stretch (this) == StretchUniformToFill))) {
		Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
		if (!vh)
			return;
		Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
		if (!vw)
			return;

		cairo_rectangle (cr, 0, 0, vw->AsDouble (), vh->AsDouble ());
		cairo_clip (cr);
		cairo_new_path (cr);
	}
}

int number = 0;

//
// Returns TRUE if surface is a good candidate for caching.
// Our current strategy is to cache big surfaces (likely backgrounds)
// that don't scale tranformations. We accept a little bit of scaling though.
//
bool
Shape::IsCandidateForCaching (void)
{
	if (IsEmpty ())
		return FALSE;

	if (! GetSurface ())
		return FALSE;

	if (bounds.w * bounds.h < 60000)
		return FALSE;

	// This is not 100% correct check -- the actual surface size might be
	// a tiny little bit larger. It's not a problem though if we go few
	// bytes above the cache limit.
	if (! GetSurface ()->VerifyWithCacheSizeCounter (bounds.w, bounds.h))
		return FALSE;

	// one last line of defense, lets not cache things 
	// much larger than the screen.
	if (bounds.w * bounds.h > 4000000)
		return FALSE;

	return TRUE;
}

//
// This routine is useful for Shape derivatives: it can be used
// to either get the bounding box from cairo, or to paint it
//
void
Shape::DoDraw (cairo_t *cr, bool do_op)
{
	bool ret = FALSE;

	// quick out if, when building the path, we detected a empty shape
	if (IsEmpty ())
		goto cleanpath;

	if (do_op && cached_surface == NULL && IsCandidateForCaching ()) {
		Rect cache_extents = bounds.RoundOut ();
		cairo_t *cached_cr = NULL;
		
		// g_warning ("bounds (%f, %f), extents (%f, %f), cache_extents (%f, %f)", 
		// bounds.w, bounds.h,
		// extents.w, extents.h,
		// cache_extents.w, cache_extents.h);
		
		cached_surface = image_brush_create_similar (cr, (int)cache_extents.w, (int)cache_extents.h);
		cairo_surface_set_device_offset (cached_surface, -cache_extents.x, -cache_extents.y);
		cached_cr = cairo_create (cached_surface);
		
		cairo_set_matrix (cached_cr, &absolute_xform);
		Clip (cached_cr);
	
		ret = DrawShape (cached_cr, do_op);
		
		cairo_destroy (cached_cr);
		
		// Increase our cache size
		cached_size = GetSurface ()->AddToCacheSizeCounter (cache_extents.w, cache_extents.h);
	}
	
	if (do_op && cached_surface) {
		cairo_pattern_t *cached_pattern = NULL;

		cached_pattern = cairo_pattern_create_for_surface (cached_surface);
		cairo_identity_matrix (cr);
		cairo_set_source (cr, cached_pattern);
		cairo_paint (cr);
		cairo_pattern_destroy (cached_pattern);
		
		if (ret)
			return;
	} else {
		cairo_set_matrix (cr, &absolute_xform);
		Clip (cr);
		
		if (DrawShape (cr, do_op))
			return;
	}

cleanpath:
	if (do_op)
		cairo_new_path (cr);
}

void
Shape::Render (cairo_t *cr, int x, int y, int width, int height)
{
	cairo_save (cr);
	DoDraw (cr, true);
	cairo_restore (cr);
}

void
Shape::ComputeBounds ()
{
	cairo_matrix_init_identity (&stretch_transform);
	InvalidateSurfaceCache ();
	
	extents = ComputeShapeBounds (false);
	extents = ComputeStretchBounds (extents);
	bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
	//printf ("%f,%f,%f,%f\n", bounds.x, bounds.y, bounds.w, bounds.h);
}

Rect
Shape::ComputeShapeBounds (bool logical)
{
	if (IsEmpty ())
		return Rect ();

	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);

	if ((w <= 0.0) || (h <= 0.0))
		return Rect ();
	

	//double t = shape_get_stroke_thickness (this) * .5;

	return Rect (0, 0, w, h);
}

Rect
Shape::ComputeLargestRectangleBounds ()
{
	Rect largest = ComputeLargestRectangle ();
	if (largest.IsEmpty ())
		return largest;

	return IntersectBoundsWithClipPath (largest, false).Transform (&absolute_xform);
}

Rect
Shape::ComputeLargestRectangle ()
{
	// by default the largest rectangle that fits into a shape is empty
	return Rect ();
}

void
Shape::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = extents.h;
	*width = extents.w;
}

bool
Shape::InsideObject (cairo_t *cr, double x, double y)
{
	cairo_save (cr);
	
	Value* clip_geometry = GetValue (UIElement::ClipProperty);
	if (clip_geometry) {
		Geometry* clip = clip_geometry->AsGeometry ();
		if (clip) {
			clip->Draw (NULL, cr);
			cairo_clip (cr);
		}
	}

	// don't do the operation but do consider filling
	DoDraw (cr, false);
	uielement_transform_point (this, &x ,&y);

	// don't check in_stroke without a stroke or in_fill without a fill (even if it can be filled)
	bool ret = ((stroke && cairo_in_stroke (cr, x, y)) || (fill && CanFill () && cairo_in_fill (cr, x, y)));

	cairo_new_path (cr);
	cairo_restore (cr);

	return ret;
}

void
Shape::CacheInvalidateHint (void)
{
	// Also kills the surface cache
	InvalidatePathCache ();
}

void
Shape::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::SHAPE) {
		if ((args->property == FrameworkElement::HeightProperty) || (args->property == FrameworkElement::WidthProperty))
			InvalidatePathCache ();

		if (args->property == UIElement::OpacityProperty) {
			if (IS_INVISIBLE (args->new_value->AsDouble ()))
				InvalidateSurfaceCache ();
		} else {
			if (args->property == UIElement::VisibilityProperty) {
				if (args->new_value->AsInt32() != VisibilityVisible)
					InvalidateSurfaceCache ();
			}
		}

		FrameworkElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == Shape::StretchProperty) {
		InvalidatePathCache ();
		UpdateBounds (true);
	}
	else if (args->property == Shape::StrokeProperty) {
		stroke = args->new_value ? args->new_value->AsBrush() : NULL;
		InvalidateSurfaceCache ();
		UpdateBounds ();
	} else if (args->property == Shape::FillProperty) {
		fill = args->new_value ? args->new_value->AsBrush() : NULL;
		InvalidateSurfaceCache ();
		UpdateBounds ();
	} else if (args->property == Shape::StrokeThicknessProperty) {
		InvalidatePathCache ();
		UpdateBounds ();
	} else if (args->property == Shape::StrokeDashCapProperty
		   || args->property == Shape::StrokeEndLineCapProperty
		   || args->property == Shape::StrokeLineJoinProperty
		   || args->property == Shape::StrokeMiterLimitProperty
		   || args->property == Shape::StrokeStartLineCapProperty) {
		UpdateBounds ();
		InvalidateSurfaceCache ();
	}
	
	Invalidate ();

	NotifyListenersOfPropertyChange (args);
}

void
Shape::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Shape::FillProperty || prop == Shape::StrokeProperty) {
		Invalidate ();
		InvalidateSurfaceCache ();
	}
	else
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

Point
Shape::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		      framework_element_get_height (this) * user_xform_origin.y);
}

void
Shape::InvalidatePathCache (bool free)
{
	SetShapeFlags (UIElement::SHAPE_NORMAL);
	if (path) {
		if (free) {
			moon_path_destroy (path);
			path = NULL;
		} else {
			moon_path_clear (path);
		}
	}

	InvalidateSurfaceCache ();
}

void
Shape::InvalidateSurfaceCache (void)
{
	if (cached_surface) {
		cairo_surface_destroy (cached_surface);
		if (GetSurface ())
			GetSurface ()->RemoveFromCacheSizeCounter (cached_size);
		cached_surface = NULL;
		cached_size = 0;
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
	if (!shape)
		return 0.0;

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

// The Ellipse shape can be drawn while ignoring properties:
// * Shape::StrokeStartLineCap
// * Shape::StrokeEndLineCap
// * Shape::StrokeLineJoin
// * Shape::StrokeMiterLimit
bool
Ellipse::DrawShape (cairo_t *cr, bool do_op)
{
	bool drawn = Fill (cr, do_op);

	if (!stroke)
		return drawn;
	if (!SetupLine (cr))
		return drawn;
	SetupLineCaps (cr);

	if (!drawn)
		Draw (cr);
	Stroke (cr, do_op);
	return true; 
}

void
Ellipse::BuildPath ()
{
	Value *height, *width;
	if (Shape::MixedHeightWidth (&height, &width))
		return;

	Stretch stretch = shape_get_stretch (this);
	double x = 0.0;
	double y = 0.0;
	double w = 0.0;
	double h = 0.0;
	double t = shape_get_stroke_thickness (this);
	double t2;

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

	t2 = t * 2.0;
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

shape:
	if (IsDegenerate ()) {
		double radius = t / 2;
		path = moon_path_renew (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH);
		moon_rounded_rectangle (path, x, y, w, h, radius, radius);
	} else {
		path = moon_path_renew (path, MOON_PATH_ELLIPSE_LENGTH);
		moon_ellipse (path, x, y, w, h);
	}
	// note: both moon_rounded_rectangle and moon_ellipse close the path
}

Rect
Ellipse::ComputeLargestRectangle ()
{
	double t = GetValue (Shape::StrokeThicknessProperty)->AsDouble ();
	double x = (GetValue (FrameworkElement::WidthProperty)->AsDouble () - t) * cos (M_PI_2);
	double y = (GetValue (FrameworkElement::HeightProperty)->AsDouble () - t) * sin (M_PI_2);
	return ComputeShapeBounds (false).GrowBy (-x, -y).RoundIn ();
}

void
Ellipse::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	DependencyProperty *prop = args->property;

	if ((prop == Shape::StrokeThicknessProperty) || (prop == Shape::StretchProperty) ||
		(prop == FrameworkElement::WidthProperty) || (prop == FrameworkElement::HeightProperty)) {
		BuildPath ();
		InvalidateSurfaceCache ();
	}

	// Ellipse has no property of it's own
	Shape::OnPropertyChanged (args);
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

// The Rectangle shape can be drawn while ignoring properties:
// * Shape::StrokeStartLineCap
// * Shape::StrokeEndLineCap
// * Shape::StrokeLineJoin	[for rounded-corner rectangles only]
// * Shape::StrokeMiterLimit	[for rounded-corner rectangles only]
bool
Rectangle::DrawShape (cairo_t *cr, bool do_op)
{
	bool drawn = Fill (cr, do_op);

	if (!stroke)
		return drawn;

	if (!SetupLine (cr))
		return drawn;

	SetupLineCaps (cr);

	if (!HasRadii ())
		SetupLineJoinMiter (cr);

	// Draw if the path wasn't drawn by the Fill call
	if (!drawn)
		Draw (cr);
	Stroke (cr, do_op);
	return true; 
}

/*
 * rendering notes:
 * - a Width="0" or a Height="0" can be rendered differently from not specifying Width or Height
 * - if a rectangle has only a Width or only a Height it is NEVER rendered
 */
void
Rectangle::BuildPath ()
{
	Value *height, *width;
	if (Shape::MixedHeightWidth (&height, &width))
		return;

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
	bool round = FALSE;

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
		// note: in this case the Radius[X|Y] properties are ignored
		goto shape;
	}

	w = width->AsDouble ();
	h = height->AsDouble ();

	if ((w < 0.0) || (h < 0.0)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	// degenerate cases are handled differently for round-corner rectangles
	round = GetRadius (&radius_x, &radius_y);

	// there are two kinds of degenerations 
	// (a) the thickness is larger (or equal) to the width or height
	if ((t > w) || (t > h)) {
		// in this case we must adjust the values to make a (much) larger rectangle
		x -= t / 2.0;
		y -= t / 2.0 - 1.0;
		switch (stretch) {
		case StretchUniform:
			w = h = ((w < h) ? w : h) + t;
			if (round)
				radius_x = radius_y = w / 3;	// FIXME - not quite correct
			break;
		case StretchUniformToFill:
			w = h = ((w > h) ? w : h) + t - 1;
			if (round)
				radius_x = radius_y = w / 3;	// FIXME - not quite correct
			break;
		default:
			w += (t - 1.0);
			h += (t - 1.0);
			if (round)
				radius_x = radius_y = t / 2;
			break;
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

	if (compute_origin && (!IsStroked () || (t != 1.0)))
		x = y = t / 2.0;

	SetShapeFlags (UIElement::SHAPE_NORMAL);

shape:
	// rounded-corner rectangle ?
	if (round) {
		AddShapeFlags (UIElement::SHAPE_RADII);
		path = moon_path_renew (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH);
		moon_rounded_rectangle (path, x, y, w, h, radius_x, radius_y);
	} else {
		path = moon_path_renew (path, MOON_PATH_RECTANGLE_LENGTH);
		moon_rectangle (path, x, y, w, h);
	}
}

void
Rectangle::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::RECTANGLE) {
		Shape::OnPropertyChanged (args);
		return;
	}

	if ((args->property == Rectangle::RadiusXProperty) || (args->property == Rectangle::RadiusYProperty)) {
		InvalidatePathCache ();
		// note: changing the X and/or Y radius doesn't affect the bounds
	}

	Invalidate ();
	NotifyListenersOfPropertyChange (args);
}

void
Rectangle::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	switch (shape_get_stretch (this)) {
	case StretchUniform:
		*width = *height = (extents.w < extents.h) ? extents.w : extents.h;
		break;
	case StretchUniformToFill:
		*width = *height = (extents.w > extents.h) ? extents.w : extents.h;
		break;
	default:
		return Shape::GetSizeForBrush (cr, width, height);
	}
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

Rect
Rectangle::ComputeLargestRectangle ()
{
	double x = GetValue (Shape::StrokeThicknessProperty)->AsDouble ();
	double y = x;
	if (HasRadii ()) {
		x += GetValue (Rectangle::RadiusXProperty)->AsDouble ();
		y += GetValue (Rectangle::RadiusYProperty)->AsDouble ();
	}
	return ComputeShapeBounds (false).GrowBy (-x, -y).RoundIn ();
}

double
rectangle_get_radius_x (Rectangle *rectangle)
{
	return rectangle->GetValue (Rectangle::RadiusXProperty)->AsDouble ();
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
// rules
// * the internal path must be rebuilt when
//	- Line::X1Property, Line::Y1Property, Line::X2Property or Line::Y2Property is changed
//
// * bounds calculation is based on
//	- Line::X1Property, Line::Y1Property, Line::X2Property and Line::Y2Property
//	- Shape::StrokeThickness
//

DependencyProperty* Line::X1Property;
DependencyProperty* Line::Y1Property;
DependencyProperty* Line::X2Property;
DependencyProperty* Line::Y2Property;

#define LINECAP_SMALL_OFFSET	0.1

//Draw the start cap. Shared with Polyline
static void
line_draw_cap (cairo_t *cr, Shape* shape, PenLineCap cap, double x1, double y1, double x2, double y2)
{
	double sx1, sy1;
	if (cap == PenLineCapFlat)
		return;

	if (cap == PenLineCapRound) {
		cairo_set_line_cap (cr, convert_line_cap (cap));
		cairo_move_to (cr, x1, y1);
		cairo_line_to (cr, x1, y1);
		shape->Stroke (cr, true);
		return;
	}

	if (x1 == x2) {
		// vertical line
		sx1 = x1;
		if (y1 > y2)
			sy1 = y1 + LINECAP_SMALL_OFFSET;
		else
			sy1 = y1 - LINECAP_SMALL_OFFSET;
	} else if (y1 == y2) {
		// horizontal line
		sy1 = y1;
		if (x1 > x2)
			sx1 = x1 + LINECAP_SMALL_OFFSET;
		else
			sx1 = x1 - LINECAP_SMALL_OFFSET;
	} else {
		double m = (y1 - y2) / (x1 - x2);
		if (x1 > x2) {
			sx1 = x1 + LINECAP_SMALL_OFFSET;
		} else {
			sx1 = x1 - LINECAP_SMALL_OFFSET;
		}
		sy1 = m * sx1 + y1 - (m * x1);
	}
	cairo_set_line_cap (cr, convert_line_cap (cap));
	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, sx1, sy1);
	shape->Stroke (cr, true);
}

// Draw the start and end line caps, if not flat. This doesn't draw the line itself
// note: function shared with single-segment Polyline
static void
line_draw_caps (cairo_t *cr, Shape* shape, double x1, double y1, PenLineCap start, double x2, double y2, PenLineCap end)
{
	if (start != PenLineCapFlat) 
		line_draw_cap (cr, shape, start, x1, y1, x2, y2);
	if (end != PenLineCapFlat)
		line_draw_cap (cr, shape, end, x2, y2, x1, y1);
}


// The Line shape can be drawn while ignoring properties:
// * Shape::StrokeLineJoin
// * Shape::StrokeMiterLimit
// * Shape::Fill
bool
Line::DrawShape (cairo_t *cr, bool do_op)
{
	// no need to clear path since none has been drawn to cairo
	if (!stroke)
		return false; 

	if (!SetupLine (cr))
		return false;

	// here we hack around #345888 where Cairo doesn't support different start and end linecaps
	PenLineCap start = shape_get_stroke_start_line_cap (this);
	PenLineCap end = shape_get_stroke_end_line_cap (this);
	PenLineCap dash = shape_get_stroke_dash_cap (this);
	if (do_op && !(start == end && start == dash)) {
		// draw start and end line caps
		if (start != PenLineCapFlat) 
			line_draw_cap (cr, this, start, line_get_x1 (this), line_get_y1 (this), line_get_x2 (this), line_get_y2 (this));
		if (end != PenLineCapFlat) {
			//don't draw the end cap if it's in an "off" segment
			double x1 = line_get_x1 (this);
			double y1 = line_get_y1 (this);
			double x2 = line_get_x2 (this);
			double y2 = line_get_y2 (this);
			double thickness = shape_get_stroke_thickness (this);
			SetupDashes (cr, thickness, sqrt ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)) + shape_get_stroke_dash_offset (this) * thickness);
			line_draw_cap (cr, this, end, x2, y2, x1, y1);
			SetupLine (cr);
		}

	}
	cairo_set_line_cap (cr, convert_line_cap (dash));

	Draw (cr);
	Stroke (cr, do_op);
	return true; 
}

void
calc_line_bounds (double x1, double x2, double y1, double y2, double thickness, Rect* bounds)
{
	if (x1 == x2) {
		bounds->x = x1 - thickness / 2.0;
		bounds->y = MIN (y1, y2);
		bounds->w = thickness;
		bounds->h = fabs (y2 - y1);
	} else 	if (y1 == y2) {
		bounds->x = MIN (x1, x2);
		bounds->y = y1 - thickness / 2.0;
		bounds->w = fabs (x2 - x1);
		bounds->h = thickness;
	} else {
		double m = fabs ((y1 - y2) / (x1 - x2));
#if EXACT_BOUNDS
		double dx = sin (atan (m)) * thickness;
		double dy = cos (atan (m)) * thickness;
#else
		double dx = (m > 1.0) ? thickness : thickness * m;
		double dy = (m < 1.0) ? thickness : thickness / m;
#endif
		bounds->x = MIN (x1, x2) - dx / 2.0;
		bounds->y = MIN (y1, y2) - dy / 2.0;
		bounds->w = fabs (x2 - x1) + dx;
		bounds->h = fabs (y2 - y1) + dy;
	}
}

void
Line::BuildPath ()
{
	if (Shape::MixedHeightWidth (NULL, NULL))
		return;

	SetShapeFlags (UIElement::SHAPE_NORMAL);

	path = moon_path_renew (path, MOON_PATH_MOVE_TO_LENGTH + MOON_PATH_LINE_TO_LENGTH);

	moon_move_to (path, line_get_x1 (this), line_get_y1 (this));
	moon_line_to (path, line_get_x2 (this), line_get_y2 (this));
}

Rect
Line::ComputeShapeBounds (bool logical)
{
	Rect shape_bounds = Rect ();

	if (Shape::MixedHeightWidth (NULL, NULL))
		return shape_bounds;

	double thickness;
	if (logical)
		thickness = 0.0;
	else
		thickness = shape_get_stroke_thickness (this);

	if (thickness <= 0.0 && ! logical)
		return shape_bounds;

	double x1 = line_get_x1 (this);
	double y1 = line_get_y1 (this);
	double x2 = line_get_x2 (this);
	double y2 = line_get_y2 (this);

	calc_line_bounds (x1, x2, y1, y2, thickness, &shape_bounds);
	origin.x = MIN (x1, x2);
	origin.y = MIN (y1, y2);

	return shape_bounds;
}

void
Line::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::LINE) {
		Shape::OnPropertyChanged (args);
		return;
	}

	if (args->property == Line::X1Property ||
	    args->property == Line::X2Property ||
	    args->property == Line::Y1Property ||
	    args->property == Line::Y2Property) {
		InvalidatePathCache ();
		UpdateBounds (true);
	}

	NotifyListenersOfPropertyChange (args);
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
// rules
// * the internal path must be rebuilt when
//	- Polygon::PointsProperty is changed
//	- Shape::StretchProperty is changed
//
// * bounds calculation is based on
//	- Polygon::PointsProperty
//	- Shape::StretchProperty
//	- Shape::StrokeThickness
//

DependencyProperty* Polygon::FillRuleProperty;
DependencyProperty* Polygon::PointsProperty;

FillRule
Polygon::GetFillRule ()
{
	return polygon_get_fill_rule (this);
}

// The Polygon shape can be drawn while ignoring properties:
// * Shape::StrokeStartLineCap
// * Shape::StrokeEndLineCap
bool
Polygon::DrawShape (cairo_t *cr, bool do_op)
{
	bool drawn = Fill (cr, do_op);

	if (!stroke)
		return drawn; 

	if (!SetupLine (cr))
		return drawn;
	SetupLineCaps (cr);
	SetupLineJoinMiter (cr);

	Draw (cr);
	Stroke (cr, do_op);
	return true;
}

// special case when a polygon has a single line in it (it's drawn longer than it should)
// e.g. <Polygon Fill="#000000" Stroke="#FF00FF" StrokeThickness="8" Points="260,80 300,40" />
static void
polygon_extend_line (double *x1, double *x2, double *y1, double *y2, double thickness)
{
	// not sure why it's a 5 ? afaik it's not related to the line length or any other property
	double t5 = thickness * 5.0;
	double dx = *x1 - *x2;
	double dy = *y1 - *y2;

	if (dy == 0.0) {
		t5 -= thickness / 2.0;
		if (dx > 0.0) {
			*x1 += t5;
			*x2 -= t5;
		} else {
			*x1 -= t5;
			*x2 += t5;
		}
	} else if (dx == 0.0) {
		t5 -= thickness / 2.0;
		if (dy > 0.0) {
			*y1 += t5;
			*y2 -= t5;
		} else {
			*y1 -= t5;
			*y2 += t5;
		}
	} else {
		double angle = atan (dy / dx);
		double ax = fabs (sin (angle) * t5);
		if (dx > 0.0) {
			*x1 += ax;
			*x2 -= ax;
		} else {
			*x1 -= ax;
			*x2 += ax;
		}
		double ay = fabs (sin ((M_PI / 2.0) - angle)) * t5;
		if (dy > 0.0) {
			*y1 += ay;
			*y2 -= ay;
		} else {
			*y1 -= ay;
			*y2 += ay;
		}
	}
}

static void
calc_offsets (double x1, double y1, double x2, double y2, double thickness, double *x, double *y)
{
	double dx = x1 - x2;
	double dy = y1 - y2;
	double t2 = thickness / 2.0;
	if (dx == 0.0) {
		*x = 0.0;
		*y = t2;
	} else if (dy == 0.0) {
		*x = t2;
		*y = 0.0;
	} else {
		double angle = atan (dy / dx);
		*x = t2 / sin (angle);
		*y = t2 / sin (M_PI / 2.0 - angle);
	}
}

static void
calc_line_bounds_with_joins (double x1, double y1, double x2, double y2, double x3, double y3, double thickness, Rect *bounds)
{
	double dx1, dy1;
	calc_offsets (x1, y1, x2, y2, thickness, &dx1, &dy1);

	double dx2, dy2;
	calc_offsets (x2, y2, x3, y3, thickness, &dx2, &dy2);

	double xi = x2;
	if (x1 < x2)
		xi += fabs (dx1);
	else
		xi -= fabs (dx1);
	if (x3 < x2)
		xi += fabs (dx2);
	else
		xi -= fabs (dx2);

	double yi = y2;
	if (y1 < y2)
		yi += fabs (dy1);
	else
		yi -= fabs (dy1);
	if (y3 < y2)
		yi += fabs (dy2);
	else
		yi -= fabs (dy2);

	if (bounds->x > xi) {
		bounds->w += (bounds->x - xi);
		bounds->x = xi;
	}
	double dx = bounds->x + bounds->w - xi;
	if (dx < 0.0) {
		bounds->w -= dx;
	}
	if (bounds->y > yi) {
		bounds->h += (bounds->y - yi);
		bounds->y = yi;
	}
	double dy = bounds->y + bounds->h - yi; 
	if (dy < 0.0) {
		bounds->h -= dy;
	}
}

Rect
Polygon::ComputeShapeBounds (bool logical)
{
	Rect shape_bounds = Rect ();

	if (Shape::MixedHeightWidth (NULL, NULL))
		return shape_bounds;

	int i, count = 0;
	Point *points = polygon_get_points (this, &count);

	// the first point is a move to, resulting in an empty shape
	if (!points || (count < 2))
		return shape_bounds;

	double thickness;
	if (logical)
		thickness = 0.0;
	else 
		thickness = shape_get_stroke_thickness (this);

	if (thickness == 0.0)
		thickness = 0.01; // avoid creating an empty rectangle (for union-ing)

	double x0 = origin.x = points [0].x;
	double y0 = origin.y = points [0].y;
	double x1, y1;

	if (count == 2) {
		x1 = points [1].x;
		y1 = points [1].y;
		origin.x = MIN (origin.x, points[1].x);
		origin.y = MIN (origin.y, points[1].y);

		polygon_extend_line (&x0, &x1, &y0, &y1, thickness);
		calc_line_bounds (x0, x1, y0, y1, thickness, &shape_bounds);
	} else {
		shape_bounds.x = x1 = x0;
		shape_bounds.y = y1 = y0;
		// FIXME: we're too big for large thickness and/or steep angle
		Rect line_bounds;
		double x2 = points [1].x;
		double y2 = points [1].y;
		double x3 = points [2].x;
		double y3 = points [2].y;
		origin.x = MIN (origin.x, MIN (x2, x3));
		origin.y = MIN (origin.y, MIN (y2, y3));

		calc_line_bounds_with_joins (x1, y1, x2, y2, x3, y3, thickness, &shape_bounds);
		for (i = 3; i < count; i++) {
			x1 = x2;
			y1 = y2;
			x2 = x3;
			y2 = y3;
			x3 = points [i].x;
			y3 = points [i].y;
			origin.x = MIN (origin.x, x3);
			origin.y = MIN (origin.y, y3);
			calc_line_bounds_with_joins (x1, y1, x2, y2, x3, y3, thickness, &shape_bounds);
		}
		// a polygon is a closed shape (unless it's a line)
		x1 = x2;
		y1 = y2;
		x2 = x3;
		y2 = y3;
		x3 = x0;
		y3 = y0;
		calc_line_bounds_with_joins (x1, y1, x2, y2, x3, y3, thickness, &shape_bounds);

		x1 = points [count-1].x;
		y1 = points [count-1].y;
		x2 = x0;
		y2 = y0;
		x3 = points [1].x;
		y3 = points [1].y;
		calc_line_bounds_with_joins (x1, y1, x2, y2, x3, y3, thickness, &shape_bounds);
	}

	return shape_bounds;
}

void
Polygon::BuildPath ()
{
	if (Shape::MixedHeightWidth (NULL, NULL))
		return;

	int i, count = 0;
	Point *points = polygon_get_points (this, &count);

	// the first point is a move to, resulting in an empty shape
	if (!points || (count < 2)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	SetShapeFlags (UIElement::SHAPE_NORMAL);

	// 2 data per [move|line]_to + 1 for close path
	path = moon_path_renew (path, count * 2 + 1);

	// special case, both the starting and ending points are 5 * thickness than the actual points
	if (count == 2) {
		double x1 = points [0].x;
		double y1 = points [0].y;
		double x2 = points [1].x;
		double y2 = points [1].y;

		polygon_extend_line (&x1, &x2, &y1, &y2, shape_get_stroke_thickness (this));

		moon_move_to (path, x1, y1);
		moon_line_to (path, x2, y2);
	} else {
		moon_move_to (path, points [0].x, points [0].y);
		for (i = 1; i < count; i++)
			moon_line_to (path, points [i].x, points [i].y);
	}
	moon_close_path (path);
}

void
Polygon::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::POLYGON) {
		Shape::OnPropertyChanged (args);
		return;
	}

	if (args->property == Polygon::PointsProperty) {
		InvalidatePathCache ();
		UpdateBounds (true /* force one here, even if the bounds don't change */);
	}

	Invalidate ();
	NotifyListenersOfPropertyChange (args);
}

void
Polygon::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	UpdateBounds (true);
	Invalidate ();
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
// rules
// * the internal path must be rebuilt when
//	- Polyline::PointsProperty is changed
//	- Shape::StretchProperty is changed
//
// * bounds calculation is based on
//	- Polyline::PointsProperty
//	- Shape::StretchProperty
//	- Shape::StrokeThickness

DependencyProperty* Polyline::FillRuleProperty;
DependencyProperty* Polyline::PointsProperty;

// The Polyline shape can be drawn while ignoring NO properties
bool
Polyline::DrawShape (cairo_t *cr, bool do_op)
{
	bool drawn = Fill (cr, do_op);

	if (!stroke)
		return drawn; 

	if (!SetupLine (cr))
		return drawn;
	SetupLineJoinMiter (cr);

	// here we hack around #345888 where Cairo doesn't support different start and end linecaps
	PenLineCap start = shape_get_stroke_start_line_cap (this);
	PenLineCap end = shape_get_stroke_end_line_cap (this);
	PenLineCap dash = shape_get_stroke_dash_cap (this);
	if (do_op && ! (start == end && start == dash)){
		// the previous fill, if needed, has preserved the path
		if (drawn)
			cairo_new_path (cr);

		// since Draw may not have been called (e.g. no Fill) we must ensure the path was built
		if (!drawn || !path || (path->cairo.num_data == 0))
			BuildPath ();

		cairo_path_data_t *data = path->cairo.data;
		int length = path->cairo.num_data;
		// single point polylines are not rendered
		if (length >= MOON_PATH_MOVE_TO_LENGTH + MOON_PATH_LINE_TO_LENGTH) {
			// draw line #1 with start cap
			if (start != PenLineCapFlat) {
				line_draw_cap (cr, this, start, data[1].point.x, data[1].point.y, data[3].point.x, data[3].point.y);
			}
			// draw last line with end cap
			if (end != PenLineCapFlat) {
				line_draw_cap (cr, this, end, data[length-1].point.x, data[length-1].point.y, data[length-3].point.x, data[length-3].point.y);
			}
		}
	}
	cairo_set_line_cap (cr, convert_line_cap (dash));

	Draw (cr);
	Stroke (cr, do_op);
	return true;
}

FillRule
Polyline::GetFillRule ()
{
	return polyline_get_fill_rule (this);
}

Rect
Polyline::ComputeShapeBounds (bool logical)
{
	Rect shape_bounds = Rect ();

	if (Shape::MixedHeightWidth (NULL, NULL))
		return shape_bounds;

	int i, count = 0;
	Point *points = polyline_get_points (this, &count);

	// the first point is a move to, resulting in an empty shape
	if (!points || (count < 2))
		return shape_bounds;

	double thickness;
	if (logical)
		thickness = 0.0;
	else
		thickness = shape_get_stroke_thickness (this);

	if (thickness == 0.0)
		thickness = 0.01; // avoid creating an empty rectangle (for union-ing)

	double x1 = origin.x = points [0].x;
	double y1 = origin.y = points [0].y;
	
	if (count == 2) {
		// this is a "simple" line (move to + line to)
		double x2 = points [1].x;
		double y2 = points [1].y;
		origin.x = MIN (origin.x, x2);
		origin.y = MIN (origin.y, y2);
		calc_line_bounds (x1, x2, y1, y2, thickness, &shape_bounds);
	} else {
		// FIXME: we're too big for large thickness and/or steep angle
		Rect line_bounds;
		double x2 = points [1].x;
		double y2 = points [1].y;
		origin.x = MIN (origin.x, x2);
		origin.y = MIN (origin.y, y2);
		calc_line_bounds (x1, x2, y1, y2, thickness, &shape_bounds);
		for (i = 2; i < count; i++) {
			double x3 = points [i].x;
			double y3 = points [i].y;
			origin.x = MIN (origin.x, x3);
			origin.y = MIN (origin.y, y3);
			calc_line_bounds_with_joins (x1, y1, x2, y2, x3, y3, thickness, &shape_bounds);
			x1 = x2;
			y1 = y2;
			x2 = x3;
			y2 = y3;
		}
		calc_line_bounds (x1, x2, y1, y2, thickness, &line_bounds);
		shape_bounds = shape_bounds.Union (line_bounds);
	}

	return shape_bounds;
}

void
Polyline::BuildPath ()
{
	if (Shape::MixedHeightWidth (NULL, NULL))
		return;

	int i, count = 0;
	Point *points = polyline_get_points (this, &count);

	// the first point is a move to, resulting in an empty shape
	if (!points || (count < 2)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	SetShapeFlags (UIElement::SHAPE_NORMAL);

	// 2 data per [move|line]_to
	path = moon_path_renew (path, count * 2);

	moon_move_to (path, points [0].x, points [0].y);
	for (i = 1; i < count; i++)
		moon_line_to (path, points [i].x, points [i].y);
}

void
Polyline::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::POLYLINE) {
		Shape::OnPropertyChanged (args);
		return;
	}

	if (args->property == Polyline::PointsProperty) {
		InvalidatePathCache ();
		UpdateBounds (true /* force one here, even if the bounds don't change */);
	}

	Invalidate ();
	NotifyListenersOfPropertyChange (args);
}

void
Polyline::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
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

bool
Path::SetupLine (cairo_t* cr)
{
	// we cannot use the thickness==0 optimization (like Shape::SetupLine provides)
	// since we'll be using cairo to compute the path's bounds later
	// see bug #352188 for an example of what this breaks
	double thickness = IsDegenerate () ? 1.0 : shape_get_stroke_thickness (this);
	cairo_set_line_width (cr, thickness);
	return SetupDashes (cr, thickness);
}

// The Polygon shape can be drawn while ignoring properties:
// * none 
// FIXME: actually it depends on the geometry, another level of optimization awaits ;-)
// e.g. close geometries don't need to setup line caps, 
//	line join/miter	don't applies to curve, like EllipseGeometry
bool
Path::DrawShape (cairo_t *cr, bool do_op)
{
	bool drawn = Shape::Fill (cr, do_op);

	if (stroke) {
		if (!SetupLine (cr))
			return drawn;	// return if we have a path in the cairo_t
		SetupLineCaps (cr);
		SetupLineJoinMiter (cr);

		if (!drawn)
			Draw (cr);
		Stroke (cr, do_op);
	}

	return true;
}

FillRule
Path::GetFillRule ()
{
	Geometry* geometry = path_get_data (this);
	return geometry ? geometry_get_fill_rule (geometry) : Shape::GetFillRule ();
}

Rect
Path::ComputeShapeBounds (bool logical)
{
	Rect shape_bounds = Rect ();

	Value *vh, *vw;
	if (Shape::MixedHeightWidth (&vh, &vw))
		return shape_bounds;

	Geometry* geometry = path_get_data (this);
	if (!geometry) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return shape_bounds;
	}

	double w = vw ? vw->AsDouble () : 0.0;
	double h = vh ? vh->AsDouble () : 0.0;
	
	if ((h < 0.0) || (w < 0.0)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return shape_bounds;
	}

	if (vh && (h <= 0.0) || vw && (w <= 0.0)) { 
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return shape_bounds;
	}

	shape_bounds = geometry->ComputeBounds (this, logical);

	if (! logical)
		origin = Point (shape_bounds.x, shape_bounds.y);

	return shape_bounds;
}

void
Path::Draw (cairo_t *cr)
{
	cairo_new_path (cr);

	Geometry* geometry = path_get_data (this);
	if (!geometry)
		return;

	cairo_save (cr);
	cairo_transform (cr, &stretch_transform);
	geometry->Draw (this, cr);

	cairo_restore (cr);
}

void
Path::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::PATH) {
		Shape::OnPropertyChanged (args);
		return;
	}

	InvalidatePathCache ();
	FullInvalidate (false);

	NotifyListenersOfPropertyChange (args);
}

void
Path::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Path::DataProperty) {
		InvalidatePathCache ();
		FullInvalidate (false);
	}
	else
		Shape::OnSubPropertyChanged (prop, obj, subobj_args);
}

/*
 * Right now implementing Path::ComputeLargestRectangle doesn't seems like a good idea. That would require
 * - checking the path for curves (and either flatten it or return an empty Rect)
 * - checking for polygon simplicity (finding intersections)
 * - checking for a convex polygon (if concave we can turn it into several convex or return an empty Rect)
 * - find the largest rectangle inside the (or each) convex polygon(s)
 * 	http://cgm.cs.mcgill.ca/~athens/cs507/Projects/2003/DanielSud/complete.html
 */

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
