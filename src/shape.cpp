/*
 * shape.cpp: This match the classes inside System.Windows.Shapes
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <cairo.h>

#include <math.h>

#include "runtime.h"
#include "shape.h"
#include "brush.h"
#include "utils.h"
#include "ptr.h"

//
// SL-Cairo convertion and helper routines
//

#define EXACT_BOUNDS 1

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

Shape::Shape ()
{
	SetObjectType (Type::SHAPE);

	stroke = NULL;
	fill = NULL;
	path = NULL;
	cached_surface = NULL;
	SetShapeFlags (UIElement::SHAPE_NORMAL);
	cairo_matrix_init_identity (&stretch_transform);
	
	SetStrokeDashArray (DOPtr<DoubleCollection> (new DoubleCollection ()));
}

Shape::~Shape ()
{
	// That also destroys the cached surface
	InvalidatePathCache (true);
}

Point
Shape::GetTransformOrigin ()
{
	if (GetStretch () != StretchNone)
		return FrameworkElement::GetTransformOrigin ();

	return Point (0,0);
}

Transform *
Shape::GetGeometryTransform ()
{
	Matrix *matrix = new Matrix (&stretch_transform);
	
	MatrixTransform *transform = new MatrixTransform ();

	transform->SetValue (MatrixTransform::MatrixProperty, matrix);
	matrix->unref ();

	return transform;
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
Shape::SetupLine (cairo_t *cr)
{
	double thickness = GetStrokeThickness ();
	
	// check if something will be drawn or return 
	// note: override this method if cairo is used to compute bounds
	if (thickness == 0)
		return false;

	cairo_set_line_width (cr, thickness);

	return SetupDashes (cr, thickness);
}

bool
Shape::SetupDashes (cairo_t *cr, double thickness)
{
	return Shape::SetupDashes (cr, thickness, GetStrokeDashOffset () * thickness);
}

bool
Shape::SetupDashes (cairo_t *cr, double thickness, double offset)
{
	DoubleCollection *dashes = GetStrokeDashArray ();
	if (dashes && (dashes->GetCount() > 0)) {
		int count = dashes->GetCount();

		// NOTE: special case - if we continue cairo will stops drawing!
		if ((count == 1) && (dashes->GetValueAt(0)->AsDouble() == 0.0))
			return false;

		// multiply dashes length with thickness
		double *dmul = new double [count];
		for (int i=0; i < count; i++) {
			dmul [i] = dashes->GetValueAt(i)->AsDouble() * thickness;
		}

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
	// Setting the cap to dash_cap. the endcaps (if different) are handled elsewhere
	PenLineCap cap = GetStrokeDashCap ();
	
	cairo_set_line_cap (cr, convert_line_cap (cap));
}

void
Shape::SetupLineJoinMiter (cairo_t *cr)
{
	PenLineJoin join = GetStrokeLineJoin ();
	double limit = GetStrokeMiterLimit ();
	
	cairo_set_line_join (cr, convert_line_join (join));
	cairo_set_miter_limit (cr, limit);
}

// returns true if the path is set on the cairo, false if not
bool
Shape::Fill (cairo_t *cr, bool do_op)
{
	if (!fill)
		return false;

	Draw (cr);
	if (do_op) {
		fill->SetupBrush (cr, GetStretchExtents ());
		cairo_set_fill_rule (cr, convert_fill_rule (GetFillRule ()));
		fill->Fill (cr, true);
	}
	return true;
}

Rect
Shape::ComputeStretchBounds ()
{
	/*
	 * NOTE: this code is extremely fragile don't make a change here without
	 * checking the results of the test harness on with MOON_DRT_CATEGORIES=stretch
	 */

	bool autodim = isnan (GetWidth ());

	Stretch stretch = GetStretch ();
	Rect shape_bounds = GetNaturalBounds ();

	if (shape_bounds.width <= 0.0 || shape_bounds.height <= 0.0) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return Rect();
	}
	
	Size framework (GetActualWidth (), GetActualHeight ());
	Size specified (GetWidth (), GetHeight ());

	if (specified.width <= 0.0 || specified.height <= 0.0) { 
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return Rect ();
	}

	if (GetVisualParent () && GetVisualParent()->Is (Type::CANVAS)) {
		if (!isnan (specified.width))
			framework.width = specified.width;
		if (!isnan (specified.height))
			framework.height = specified.height;
	}

	framework.width = framework.width == 0.0 ? shape_bounds.width : framework.width;
	framework.height = framework.height == 0.0 ? shape_bounds.height : framework.height;

	if (stretch != StretchNone) {
		Rect logical_bounds = ComputeShapeBounds (true, NULL);

		bool adj_x = logical_bounds.width != 0.0;
		bool adj_y = logical_bounds.height != 0.0;
             
		double diff_x = shape_bounds.width - logical_bounds.width;
		double diff_y = shape_bounds.height - logical_bounds.height;
		double sw = adj_x ? (framework.width - diff_x) / logical_bounds.width : 1.0;
		double sh = adj_y ? (framework.height - diff_y) / logical_bounds.height : 1.0;

		bool center = false;

		switch (stretch) {
		case StretchFill:
			center = true;
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

		// trying to avoid the *VERY*SLOW* adjustments
		// e.g. apps like Silverlight World have a ratio up to 50 unneeded for 1 needed adjustment
		// so it all boilds down are we gonna change bounds anyway ?
		#define IS_SIGNIFICANT(dx,x)	(IS_ZERO(dx) && (fabs(dx) * x - x > 1.0))
		if ((adj_x && IS_SIGNIFICANT((sw - 1), shape_bounds.width)) || (adj_y && IS_SIGNIFICANT((sh - 1), shape_bounds.height))) {
			// FIXME: this IS still UBER slow
			// hereafter we're doing a second pass to refine the sw and sh we guessed
			// the first time. This usually gives pixel-recise stretches for Paths
			cairo_matrix_t temp;
			cairo_matrix_init_scale (&temp, adj_x ? sw : 1.0, adj_y ? sh : 1.0);
			Rect stretch_bounds = ComputeShapeBounds (false, &temp);
			if (stretch_bounds.width != shape_bounds.width && stretch_bounds.height != shape_bounds.height) {
				sw *= adj_x ? (framework.width - stretch_bounds.width + logical_bounds.width * sw) / (logical_bounds.width * sw): 1.0;
				sh *= adj_y ? (framework.height - stretch_bounds.height + logical_bounds.height * sh) / (logical_bounds.height * sh): 1.0;

				switch (stretch) {
				case StretchUniform:
					sw = sh = (sw < sh) ? sw : sh;
					break;
				case StretchUniformToFill:
					sw = sh = (sw > sh) ? sw : sh;
					break;
				default:
					break;
				}
			}
			// end of the 2nd pass code
		}

		double x = !autodim || adj_x ? shape_bounds.x : 0;
		double y = !autodim || adj_y ? shape_bounds.y : 0;

		if (center)
			cairo_matrix_translate (&stretch_transform, 
						adj_x ? framework.width * 0.5 : 0, 
						adj_y ? framework.height * 0.5 : 0);
		else //UniformToFill
			cairo_matrix_translate (&stretch_transform, 
						adj_x ? (logical_bounds.width * sw + diff_x) * .5 : 0,
						adj_y ? (logical_bounds.height * sh + diff_y) * .5: 0);
		
		cairo_matrix_scale (&stretch_transform, 
				    adj_x ? sw : 1.0, 
				    adj_y ? sh : 1.0);
		
		cairo_matrix_translate (&stretch_transform, 
					adj_x ? -shape_bounds.width * 0.5 : 0, 
					adj_y ? -shape_bounds.height * 0.5 : 0);

		if (!Is (Type::LINE) || !autodim)
			cairo_matrix_translate (&stretch_transform, -x, -y);
		
		// Double check our math
		cairo_matrix_t test = stretch_transform;
		if (cairo_matrix_invert (&test)) {
			g_warning ("Unable to compute stretch transform %f %f %f %f \n", sw, sh, shape_bounds.x, shape_bounds.y);
		}		
	}
	
	shape_bounds = shape_bounds.Transform (&stretch_transform);
	
	return shape_bounds;
}

void
Shape::Stroke (cairo_t *cr, bool do_op)
{
	if (do_op) {
		stroke->SetupBrush (cr, GetStretchExtents ());
		stroke->Stroke (cr);
	}
}

void
Shape::Clip (cairo_t *cr)
{
	Rect specified = Rect (0, 0, GetWidth (), GetHeight ());
	Rect paint = Rect (0, 0, GetActualWidth (), GetActualHeight ());
	UIElement *parent = GetVisualParent ();
	bool in_flow = parent && !parent->Is (Type::CANVAS);

	/* 
	 * NOTE the clumbsy rounding up to 1 here is based on tests like
	 * test-shape-path-stretch.xaml where it silverlight attempts
	 * to make sure something is always drawn a better mechanism
	 * is warranted
	 */
	if (!IsDegenerate ()) {
		bool clip_bounds = false;
		if (!isnan (specified.width) && specified.width >= 1) { // && paint.width > specified.width) {
			paint.width = specified.width;
			if (!in_flow)
				paint.height = isnan (specified.height) ? 0 : MAX (1, specified.height);
			clip_bounds = true;
		}

		if (!isnan (specified.height) && specified.height >= 1) { // && paint.height > specified.height) {
			paint.height = specified.height;

			if (!in_flow)
				paint.width = isnan (specified.width) ? 0 : MAX (1, specified.width);
			clip_bounds = true;
		}
	       
		if (clip_bounds) {
			paint.Draw (cr);
			cairo_clip (cr);
		}
	}
	RenderLayoutClip (cr);
}

//
// Returns TRUE if surface is a good candidate for caching.
// We accept a little bit of scaling.
//
bool
Shape::IsCandidateForCaching (void)
{
	if (IsEmpty ()) 
		return FALSE;

	if (! GetSurface ())
		return FALSE;

	/* 
	 * these fill and stroke short circuits are attempts be smart
	 * about determining when the cost of caching is greater than
	 * the cost of simply drawing all the choices here should really
	 * have best and worst case perf tests associated with them
	 * but they don't right now
	 */
	bool gradient_fill = false;
	/* XXX FIXME this should be a property on the shape */
	bool simple = Is (Type::RECTANGLE) || Is (Type::ELLIPSE);

	if (fill) {
		if (fill->IsAnimating ())
			return FALSE;

		gradient_fill |= fill->Is (Type::GRADIENTBRUSH);
	}


	if (stroke && stroke->IsAnimating ())
		return FALSE;
	
	if (simple && !gradient_fill)
		return FALSE;

	// This is not 100% correct check -- the actual surface size might be
	// a tiny little bit larger. It's not a problem though if we go few
	// bytes above the cache limit.
	if (!GetSurface ()->VerifyWithCacheSizeCounter ((int) bounds.width, (int) bounds.height))
		return FALSE;

	// one last line of defense, lets not cache things 
	// much larger than the screen.
	if (bounds.width * bounds.height > 4000000)
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

	// quick out if, when building the path, we detected an empty shape
	if (IsEmpty ())
		goto cleanpath;

	if (do_op && cached_surface == NULL && IsCandidateForCaching ()) {
		Rect cache_extents = bounds.RoundOut ();
		cairo_t *cached_cr = NULL;
		
		// g_warning ("bounds (%f, %f), extents (%f, %f), cache_extents (%f, %f)", 
		// bounds.width, bounds.height,
		// extents.width, extents.height,
		// cache_extents.width, cache_extents.height);
		
		cached_surface = image_brush_create_similar (cr, (int) cache_extents.width, (int) cache_extents.height);
		if (cairo_surface_status (cached_surface) == CAIRO_STATUS_SUCCESS) {
			cairo_surface_set_device_offset (cached_surface, -cache_extents.x, -cache_extents.y);
			cached_cr = cairo_create (cached_surface);
			
			cairo_set_matrix (cached_cr, &absolute_xform);
		
			ret = DrawShape (cached_cr, do_op);
			
			cairo_destroy (cached_cr);
			
			// Increase our cache size
			cached_size = GetSurface ()->AddToCacheSizeCounter ((int) cache_extents.width, (int) cache_extents.height);
		} else {
			cairo_surface_destroy (cached_surface);
			cached_surface = NULL;
		}
	}
	
	if (do_op && cached_surface) {
		cairo_pattern_t *cached_pattern = NULL;

		cached_pattern = cairo_pattern_create_for_surface (cached_surface);
		cairo_set_matrix (cr, &absolute_xform);
		if (do_op)
			Clip (cr);
		
		cairo_identity_matrix (cr);
		if (cairo_pattern_status (cached_pattern) == CAIRO_STATUS_SUCCESS)
			cairo_set_source (cr, cached_pattern);
		else
			cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);

		cairo_pattern_destroy (cached_pattern);
		cairo_paint (cr);
	} else {
		cairo_set_matrix (cr, &absolute_xform);
		if (do_op)
			Clip (cr);
		
		if (DrawShape (cr, do_op))
			return;
	}

cleanpath:
	if (do_op)
		cairo_new_path (cr);
}

void
Shape::Render (cairo_t *cr, Region *region, bool path_only)
{
	cairo_save (cr);
	DoDraw (cr, true && !path_only);
	cairo_restore (cr);
}

void
Shape::ShiftPosition (Point p)
{
	double dx = bounds.x - p.x;
	double dy = bounds.y - p.y;

	// FIXME this is much less than ideal but we must invalidate the surface cache
	// if the shift is not an integer otherwise we can potentially drow outside our
	// rounded out bounds.
       	if (cached_surface && (dx == trunc(dx)) && (dy == trunc(dy))) {
		cairo_surface_set_device_offset (cached_surface, trunc (-p.x), trunc (-p.y));
	} else {
		InvalidateSurfaceCache ();
	}

	FrameworkElement::ShiftPosition (p);
}


Size
Shape::ComputeActualSize ()
{
	Size desired = FrameworkElement::ComputeActualSize ();
	Rect shape_bounds = GetNaturalBounds ();
	double sx = 1.0;
	double sy = 1.0;
	UIElement *parent = GetVisualParent ();
	
	if (parent && !parent->Is (Type::CANVAS))
		if (LayoutInformation::GetPreviousConstraint (this) || LayoutInformation::GetLayoutSlot (this))
			return desired;

	if (!GetSurface ()) 
		return desired;

	if (shape_bounds.width <= 0 && shape_bounds.height <= 0)
		return desired;

	if (GetStretch () == StretchNone && shape_bounds.width > 0 && shape_bounds.height > 0)
		return Size (shape_bounds.width, shape_bounds.height);

	/* don't stretch to infinite size */
	if (isinf (desired.width))
		desired.width = shape_bounds.width;
	if (isinf (desired.height))
		desired.height = shape_bounds.height;
	
	/* compute the scaling */
	if (shape_bounds.width > 0)
		sx = desired.width / shape_bounds.width;
	if (shape_bounds.height > 0)
		sy = desired.height / shape_bounds.height;

	switch (GetStretch ()) {
	case StretchUniform:
               sx = sy = MIN (sx, sy);
               break;
	case StretchUniformToFill:
		sx = sy = MAX (sx, sy);
		break;
	default:
		break;
	}

	desired = desired.Min (shape_bounds.width * sx, shape_bounds.height * sy);

	return desired;
}

Size
Shape::MeasureOverride (Size availableSize)
{
	Size desired = availableSize;
	Rect shape_bounds = GetNaturalBounds ();
	double sx = 0.0;
	double sy = 0.0;

	if (Is (Type::RECTANGLE) || Is (Type::ELLIPSE)) {
		desired = Size (0,0);
	}

	if (GetStretch () == StretchNone)
		return Size (shape_bounds.x + shape_bounds.width, shape_bounds.y + shape_bounds.height);
	
	/* don't stretch to infinite size */
	if (isinf (availableSize.width))
		desired.width = shape_bounds.width;
	if (isinf (availableSize.height))
		desired.height = shape_bounds.height;

	/* compute the scaling */
	if (shape_bounds.width > 0)
		sx = desired.width / shape_bounds.width;
	if (shape_bounds.height > 0)
		sy = desired.height / shape_bounds.height;
	
	/* don't use infinite dimensions as constraints */
	if (isinf (availableSize.width))
		sx = sy;
	if (isinf (availableSize.height))
		sy = sx;
	
        switch (GetStretch ()) {
	case StretchUniform:
		sx = sy = MIN (sx, sy);
		break;
	case StretchUniformToFill:
		sx = sy = MAX (sx, sy);
		break;
	case StretchFill:		
		if (isinf (availableSize.width))
			sx = 1.0;
		if (isinf (availableSize.height))
			sy = 1.0;
		break;
	default:
		break;
	}

	desired = Size (shape_bounds.width * sx, shape_bounds.height * sy);

	return desired;
}

Size
Shape::ArrangeOverride (Size finalSize)
{
	Size arranged = finalSize;
	double sx = 1.0;
	double sy = 1.0;

	Rect shape_bounds = GetNaturalBounds ();
	
      	InvalidateStretch ();

	if (GetStretch () == StretchNone) 
		return arranged.Max (Size (shape_bounds.x + shape_bounds.width, shape_bounds.y + shape_bounds.height));

	/* compute the scaling */
	if (shape_bounds.width == 0)
		shape_bounds.width = arranged.width;
	if (shape_bounds.height == 0)
		shape_bounds.height = arranged.height;

	if (shape_bounds.width != arranged.width)
		sx = arranged.width / shape_bounds.width;
	if (shape_bounds.height != arranged.height)
		sy = arranged.height / shape_bounds.height;

	switch (GetStretch ()) {
	case StretchUniform:
		sx = sy = MIN (sx, sy);
		break;
	case StretchUniformToFill:
		sx = sy = MAX (sx, sy);
		break;
	default:
		break;
	}

	arranged = Size (shape_bounds.width * sx, shape_bounds.height * sy);

	return arranged;
}

void
Shape::TransformBounds (cairo_matrix_t *old, cairo_matrix_t *current)
{
	InvalidateSurfaceCache ();
	bounds_with_children = bounds = IntersectBoundsWithClipPath (GetStretchExtents (), false).Transform (current);
}

void
Shape::ComputeBounds ()
{
	bounds_with_children = bounds = IntersectBoundsWithClipPath (GetStretchExtents (), false).Transform (&absolute_xform);
	//printf ("%f,%f,%f,%f\n", bounds.x, bounds.y, bounds.width, bounds.height);
}

Rect
Shape::ComputeShapeBounds (bool logical, cairo_matrix_t *matrix)
{
	double thickness = (logical || !IsStroked ()) ? 0.0 : GetStrokeThickness ();
	if (Is (Type::RECTANGLE) || Is (Type::ELLIPSE))
		return logical ? Rect (0,0,1.0,1.0) : Rect ();

	if (!path || (path->cairo.num_data == 0))
		BuildPath ();

	if (IsEmpty ())
		return Rect ();

	cairo_t *cr = measuring_context_create ();
	if (matrix)
		cairo_set_matrix (cr, matrix);

	cairo_set_line_width (cr, thickness);

	if (thickness > 0.0) {
		//FIXME: still not 100% precise since it could be different from the end cap
		PenLineCap cap = GetStrokeStartLineCap ();
		if (cap == PenLineCapFlat)
			cap = GetStrokeEndLineCap ();
		cairo_set_line_cap (cr, convert_line_cap (cap));
	}

	cairo_append_path (cr, &path->cairo);
	
	cairo_identity_matrix (cr);

	double x1, y1, x2, y2;

	if (logical) {
		cairo_path_extents (cr, &x1, &y1, &x2, &y2);
	} else if (thickness > 0) {
		cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	} else {
		cairo_fill_extents (cr, &x1, &y1, &x2, &y2);
	}

	Rect bounds = Rect (MIN (x1, x2), MIN (y1, y2), fabs (x2 - x1), fabs (y2 - y1));

	measuring_context_destroy (cr);

	return bounds;
}

void
Shape::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = GetStretchExtents ().height;
	*width = GetStretchExtents ().width;
}

bool
Shape::InsideObject (cairo_t *cr, double x, double y)
{
	bool ret = false;

	if (!InsideLayoutClip (x, y))
		return false;

	if (!InsideClip (cr, x, y))
		return false;

	TransformPoint (&x, &y);  
	if (!GetStretchExtents ().PointInside (x, y))
		return false;

	cairo_save (cr);
	DoDraw (cr, false);

	// don't check in_stroke without a stroke or in_fill without a fill (even if it can be filled)
	if (fill && CanFill ())
		ret |= cairo_in_fill (cr, x, y);
	if (!ret && stroke)
		ret |= cairo_in_stroke (cr, x, y);

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
Shape::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::SHAPE) {
		if ((args->GetId () == FrameworkElement::HeightProperty) 
		    || (args->GetId () == FrameworkElement::WidthProperty))
			InvalidateStretch ();

		// CacheInvalidateHint should handle visibility changes in
		// DirtyRenderVisibility 

		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Shape::StretchProperty) {
		InvalidateMeasure ();
		InvalidateStretch ();
	}
	else if (args->GetId () == Shape::StrokeProperty) {
		Brush *new_stroke = args->GetNewValue() ? args->GetNewValue()->AsBrush () : NULL;

		if (!stroke || !new_stroke) {
			// If the stroke changes from null to
			// <something> or <something> to null, then
			// some shapes need to reclaculate the offset
			// (based on stroke thickness) to start
			// painting.
			InvalidateStrokeBounds ();
               } else
			InvalidateSurfaceCache ();
		
		stroke = new_stroke;
	} else if (args->GetId () == Shape::FillProperty) {
		Brush *new_fill = args->GetNewValue() ? args->GetNewValue()->AsBrush () : NULL;

		if (!fill || !new_fill) {
			InvalidateFillBounds ();
		} else
			InvalidateSurfaceCache ();
			
		fill = args->GetNewValue() ? args->GetNewValue()->AsBrush() : NULL;
	} else if (args->GetId () == Shape::StrokeThicknessProperty) {
		// do we invalidate the path here for Type::RECT and Type::ELLIPSE 
		// in case they degenerate?  Or do we need it for line caps too
		InvalidateStrokeBounds ();
	} else if (args->GetId () == Shape::StrokeDashCapProperty
		   || args->GetId () == Shape::StrokeDashArrayProperty
		   || args->GetId () == Shape::StrokeEndLineCapProperty
		   || args->GetId () == Shape::StrokeLineJoinProperty
		   || args->GetId () == Shape::StrokeMiterLimitProperty
		   || args->GetId () == Shape::StrokeStartLineCapProperty) {
		InvalidateStrokeBounds ();
	}
	
	Invalidate ();

	NotifyListenersOfPropertyChange (args, error);
}

void
Shape::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && (prop->GetId () == Shape::FillProperty || prop->GetId () == Shape::StrokeProperty)) {
		Invalidate ();
		InvalidateSurfaceCache ();
	}
	else
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Shape::InvalidateStretch ()
{
	extents = Rect (0, 0, -INFINITY, -INFINITY);
	cairo_matrix_init_identity (&stretch_transform);
	InvalidatePathCache ();
}

Rect 
Shape::GetStretchExtents ()
{
	if (extents.IsEmpty ())
		extents = ComputeStretchBounds ();
	
	return extents;
}

void
Shape::InvalidateStrokeBounds ()
{
	InvalidateNaturalBounds ();
}

void
Shape::InvalidateFillBounds ()
{
	InvalidateNaturalBounds ();
}

void
Shape::InvalidateNaturalBounds ()
{
	natural_bounds = Rect (0, 0, -INFINITY, -INFINITY);
	InvalidateStretch ();
}

Rect
Shape::GetNaturalBounds ()
{
	if (natural_bounds.IsEmpty ())
		natural_bounds = ComputeShapeBounds (false, NULL);
	
	return natural_bounds;
}

void
Shape::InvalidatePathCache (bool free)
{
	//SetShapeFlags (UIElement::SHAPE_NORMAL);
	if (path) {
		if (free) {
			moon_path_destroy (path);
			path = NULL;
		} else {
			moon_path_clear (path);
		}
	}
	
	// we always pass true here because in some cases
	// while the bounds may not have change the rendering
	// still may have
	UpdateBounds (true);
	//InvalidateMeasure ();
	//InvalidateArrange ();
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

Value *
Shape::CreateDefaultStretch (DependencyObject *instance, DependencyProperty *property)
{
	if (instance->Is (Type::RECTANGLE) || instance->Is (Type::ELLIPSE))
		return new Value (StretchFill);
	else
		return new Value (StretchNone);
}

//
// Ellipse
//

Ellipse::Ellipse ()
{
	SetObjectType (Type::ELLIPSE);
}

/*
 * Ellipses (like Rectangles) are special and they don't need to participate
 * in the other stretch logic
 */
Rect
Ellipse::ComputeStretchBounds ()
{
       return ComputeShapeBounds (false);
}

Rect
Ellipse::ComputeShapeBounds (bool logical)
{
	Rect rect = Rect (0, 0, GetActualWidth (), GetActualHeight ());
	SetShapeFlags (UIElement::SHAPE_NORMAL);
	double t = GetStrokeThickness ();

	if (rect.width < 0.0 || rect.height < 0.0 || GetWidth () <= 0.0 || GetHeight () <= 0.0) { 
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return Rect ();
	}
	
	if (GetVisualParent () && GetVisualParent ()->Is (Type::CANVAS)) {
		if (isnan (GetWidth ()) != isnan (GetHeight ())) {
			SetShapeFlags (UIElement::SHAPE_EMPTY);
			return Rect ();
		}
	}

	switch (GetStretch ()) {
	case StretchNone:
		rect.width = rect.height = 0.0;
		break;
	case StretchUniform:
		rect.width = rect.height = (rect.width < rect.height) ? rect.width : rect.height;
		break;
	case StretchUniformToFill:
		rect.width = rect.height = (rect.width > rect.height) ? rect.width : rect.height;
		break;
	case StretchFill:
		/* nothing needed here.  the assignment of w/h above
		   is correct for this case. */
		break;
	}

	if (rect.width <= t || rect.height <= t){
		rect.width = MAX (rect.width, t + t * 0.001);
		rect.height = MAX (rect.height, t + t * 0.001);
		SetShapeFlags (UIElement::SHAPE_DEGENERATE);
	} else
		SetShapeFlags (UIElement::SHAPE_NORMAL);

	return rect;
}

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
	Stretch stretch = GetStretch ();
	double t = IsStroked () ? GetStrokeThickness () : 0.0;
	Rect rect = Rect (0.0, 0.0, GetActualWidth (), GetActualHeight ());

	if (rect.width < 0.0 || rect.height < 0.0 || GetWidth () <= 0.0 || GetHeight () <= 0.0) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);		
		return;
	}


	SetShapeFlags (UIElement::SHAPE_NORMAL);

	switch (stretch) {
	case StretchNone:
		rect.width = rect.height = 0.0;
		break;
	case StretchUniform:
		rect.width = rect.height = (rect.width < rect.height) ? rect.width : rect.height;
		break;
	case StretchUniformToFill:
		rect.width = rect.height = (rect.width > rect.height) ? rect.width : rect.height;
		break;
	case StretchFill:
		/* nothing needed here.  the assignment of w/h above
		   is correct for this case. */
		break;
	}

	if (rect.width <= t || rect.height <= t){
		rect.width = MAX (rect.width, t + t * 0.001);
		rect.height = MAX (rect.height, t + t * 0.001);
		SetShapeFlags (UIElement::SHAPE_DEGENERATE);
	} else
		SetShapeFlags (UIElement::SHAPE_NORMAL);

	rect = rect.GrowBy ( -t/2, -t/2);

	path = moon_path_renew (path, MOON_PATH_ELLIPSE_LENGTH);
	moon_ellipse (path, rect.x, rect.y, rect.width, rect.height);
}

void
Ellipse::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	/*
	DependencyProperty *prop = args->GetProperty ();

	if ((prop->GetId () == Shape::StrokeThicknessProperty) || (prop->GetId () == Shape::StretchProperty) ||
		(prop->GetId () == FrameworkElement::WidthProperty) || (prop->GetId () == FrameworkElement::HeightProperty)) {
		// FIXME why are we building the path here?
		BuildPath ();
		InvalidateSurfaceCache ();
	}
	*/

	// Ellipse has no property of it's own
	Shape::OnPropertyChanged (args, error);
}

//
// Rectangle
//

Rectangle::Rectangle ()
{
	SetObjectType (Type::RECTANGLE);
}

/*
 * Rectangles (like Ellipses) are special and they don't need to participate
 * in the other stretch logic
 */
Rect
Rectangle::ComputeStretchBounds ()
{
	Rect shape_bounds = ComputeShapeBounds (false);
	return ComputeShapeBounds (false);
}

Rect
Rectangle::ComputeShapeBounds (bool logical)
{
	Rect rect = Rect (0, 0, GetActualWidth (), GetActualHeight ());
	SetShapeFlags (UIElement::SHAPE_NORMAL);

	if (rect.width < 0.0 || rect.height < 0.0 || GetWidth () <= 0.0 || GetHeight () <= 0.0) { 
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return Rect ();
	}

	if (GetVisualParent () && GetVisualParent ()->Is (Type::CANVAS)) {
		if (isnan (GetWidth ()) != isnan (GetHeight ())) {
			SetShapeFlags (UIElement::SHAPE_EMPTY);
			return Rect ();
		}
	}

	double t = IsStroked () ? GetStrokeThickness () : 0.0;
	switch (GetStretch ()) {
	case StretchNone:
		rect.width = rect.height = 0.0;
		break;
	case StretchUniform:
		rect.width = rect.height = MIN (rect.width, rect.height);
		break;
	case StretchUniformToFill:
		// this gets an rectangle larger than it's dimension, relative
		// scaling is ok but we need Shape::Draw to clip to it's original size
		rect.width = rect.height = MAX (rect.width, rect.height);
		break;
	case StretchFill:
		/* nothing needed here.  the assignment of w/h above
		   is correct for this case. */
		break;
	}
	
	if (rect.width == 0)
		rect.x = t *.5;
	if (rect.height == 0)
		rect.y = t *.5;

	if (t >= rect.width || t >= rect.height) {
		SetShapeFlags (UIElement::SHAPE_DEGENERATE);
		rect = rect.GrowBy (t * .5005, t * .5005);
	} else {
		SetShapeFlags (UIElement::SHAPE_NORMAL);
	}

	return rect;
}

Rect 
Rectangle::GetCoverageBounds ()
{
	Brush *fill = GetFill ();
	
	if (fill != NULL && fill->IsOpaque()) {
		/* make it a little easier - only consider the rectangle inside the corner radii.
		   we're also a little more conservative than we need to be, regarding stroke
		   thickness. */
		double xr = (GetRadiusX () + GetStrokeThickness () / 2);
		double yr = (GetRadiusY () + GetStrokeThickness () / 2);
		
		return bounds.GrowBy (-xr, -yr).RoundIn ();
	}
	
	return Rect ();
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
	Stretch stretch = GetStretch ();
	double t = IsStroked () ? GetStrokeThickness () : 0.0;
	
	// nothing is drawn (nor filled) if no StrokeThickness="0"
	// unless both Width and Height are specified or when no streching is required
	Rect rect = Rect (0, 0, GetActualWidth (), GetActualHeight ());

	double radius_x = GetRadiusX ();
	double radius_y = GetRadiusY ();

	switch (stretch) {
	case StretchNone:
		rect.width = rect.height = 0;
		break;
	case StretchUniform:
		rect.width = rect.height = MIN (rect.width, rect.height);
		break;
	case StretchUniformToFill:
		// this gets an rectangle larger than it's dimension, relative
		// scaling is ok but we need Shape::Draw to clip to it's original size
		rect.width = rect.height = MAX (rect.width, rect.height);
		break;
	case StretchFill:
		/* nothing needed here.  the assignment of w/h above
		   is correct for this case. */
		break;
	}
	
	if (rect.width == 0)
		rect.x = t *.5;
	if (rect.height == 0)
		rect.y = t *.5;

	if (t >= rect.width || t >= rect.height) {
		rect = rect.GrowBy (t * 0.001, t * 0.001);
		SetShapeFlags (UIElement::SHAPE_DEGENERATE);
	} else {
		rect = rect.GrowBy (-t * 0.5, -t * 0.5);
		SetShapeFlags (UIElement::SHAPE_NORMAL);
	}

	path = moon_path_renew (path, MOON_PATH_ROUNDED_RECTANGLE_LENGTH);
	moon_rounded_rectangle (path, rect.x, rect.y, rect.width, rect.height, radius_x, radius_y);
}

void
Rectangle::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::RECTANGLE) {
		Shape::OnPropertyChanged (args, error);
		return;
	}

	if ((args->GetId () == Rectangle::RadiusXProperty) || (args->GetId () == Rectangle::RadiusYProperty)) {
		InvalidateMeasure ();
		InvalidatePathCache ();
	}

	Invalidate ();
	NotifyListenersOfPropertyChange (args, error);
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

#define LINECAP_SMALL_OFFSET	0.1

//Draw the start cap. Shared with Polyline
static void
line_draw_cap (cairo_t *cr, Shape* shape, PenLineCap cap, double x1, double y1, double x2, double y2)
{
	double sx1, sy1;
	if (cap == PenLineCapFlat)
		return;

	cairo_save (cr);
	cairo_transform (cr, &(shape->stretch_transform));
	if (cap == PenLineCapRound) {
		cairo_move_to (cr, x1, y1);
		cairo_line_to (cr, x1, y1);
		cairo_restore (cr);
		cairo_set_line_cap (cr, convert_line_cap (cap));
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
	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, sx1, sy1);
	cairo_restore (cr);
	cairo_set_line_cap (cr, convert_line_cap (cap));
	shape->Stroke (cr, true);
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
	PenLineCap start = GetStrokeStartLineCap ();
	PenLineCap end = GetStrokeEndLineCap ();
	PenLineCap dash = GetStrokeDashCap ();
	bool dashed = false;
	DoubleCollection *dashes = GetStrokeDashArray ();
	
	if (dashes && (dashes->GetCount() > 0))
		dashed = true;

	
	//if (do_op && !(start == end && start == dash)) {
	if (do_op && (start != end || (dashed && !(start == end && start == dash)))) {
		double x1 = GetX1 ();
		double y1 = GetY1 ();
		double x2 = GetX2 ();
		double y2 = GetY2 ();
		
		// draw start and end line caps
		if (start != PenLineCapFlat) 
			line_draw_cap (cr, this, start, x1, y1, x2, y2);
		
		if (end != PenLineCapFlat) {
			//don't draw the end cap if it's in an "off" segment
			double thickness = GetStrokeThickness ();
			double offset = GetStrokeDashOffset ();
			
			SetupDashes (cr, thickness, sqrt ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)) + offset * thickness);
			line_draw_cap (cr, this, end, x2, y2, x1, y1);
			SetupLine (cr);
		}

		cairo_set_line_cap (cr, convert_line_cap (dash));
	} else 
		cairo_set_line_cap (cr, convert_line_cap (start));

	Draw (cr);
	Stroke (cr, do_op);
	return true;
}

void
calc_line_bounds (double x1, double x2, double y1, double y2, double thickness, PenLineCap start_cap, PenLineCap end_cap, Rect* bounds)
{
	if (x1 == x2) {
		bounds->x = x1 - thickness / 2.0;
		bounds->y = MIN (y1, y2) - (y1 < y2 && start_cap != PenLineCapFlat ? thickness / 2.0 : 0.0) - (y1 >= y2 && end_cap != PenLineCapFlat ? thickness / 2.0 : 0.0);
		bounds->width = thickness;
		bounds->height = fabs (y2 - y1) + (start_cap != PenLineCapFlat ? thickness / 2.0 : 0.0) + (end_cap != PenLineCapFlat ? thickness / 2.0 : 0.0);
	} else 	if (y1 == y2) {
		bounds->x = MIN (x1, x2) - (x1 < x2 && start_cap != PenLineCapFlat ? thickness / 2.0 : 0.0) - (x1 >= x2 && end_cap != PenLineCapFlat ? thickness / 2.0 : 0.0);
		bounds->y = y1 - thickness / 2.0;
		bounds->width = fabs (x2 - x1) + (start_cap != PenLineCapFlat ? thickness / 2.0 : 0.0) + (end_cap != PenLineCapFlat ? thickness / 2.0 : 0.0);
		bounds->height = thickness;
	} else {
		double m = fabs ((y1 - y2) / (x1 - x2));
#if EXACT_BOUNDS
		double dx = sin (atan (m)) * thickness;
		double dy = cos (atan (m)) * thickness;
#else
		double dx = (m > 1.0) ? thickness : thickness * m;
		double dy = (m < 1.0) ? thickness : thickness / m;
#endif
		if (x1 < x2)
			switch (start_cap) {
			case PenLineCapSquare:
				bounds->x = MIN (x1, x2) - (dx + dy) / 2.0;
				break;
			case PenLineCapTriangle: //FIXME, reverting to Round for now
			case PenLineCapRound:
				bounds->x = MIN (x1, x2) - thickness / 2.0;
				break;
			default: //PenLineCapFlat
				bounds->x = MIN (x1, x2) - dx / 2.0;
			}	
		else 
			switch (end_cap) {
			case PenLineCapSquare:
				bounds->x = MIN (x1, x2) - (dx + dy) / 2.0;
				break;
			case PenLineCapTriangle: //FIXME, reverting to Round for now
			case PenLineCapRound:
				bounds->x = MIN (x1, x2) - thickness / 2.0;
				break;
			default: //PenLineCapFlat
				bounds->x = MIN (x1, x2) - dx / 2.0;
			}		
		if (y1 < y2)
			switch (start_cap) {
			case PenLineCapSquare:
				bounds->y = MIN (y1, y2) - (dx + dy) / 2.0;
				break;
			case PenLineCapTriangle: //FIXME, reverting to Round for now
			case PenLineCapRound:
				bounds->y = MIN (y1, y2) - thickness / 2.0;
				break;
			default: //PenLineCapFlat
				bounds->y = MIN (y1, y2) - dy / 2.0;
			}	
		else
			switch (end_cap) {
			case PenLineCapSquare:
				bounds->y = MIN (y1, y2) - (dx + dy) / 2.0;
				break;
			case PenLineCapTriangle: //FIXME, reverting to Round for now
			case PenLineCapRound:
				bounds->y = MIN (y1, y2) - thickness / 2.0;
				break;
			default: //PenLineCapFlat
				bounds->y = MIN (y1, y2) - dy / 2.0;
			}	
		bounds->width = fabs (x2 - x1);
		bounds->height = fabs (y2 - y1);
		switch (start_cap) {
		case PenLineCapSquare:
			bounds->width += (dx + dy) / 2.0;
			bounds->height += (dx + dy) / 2.0;
			break;
		case PenLineCapTriangle: //FIXME, reverting to Round for now
		case PenLineCapRound:
			bounds->width += thickness / 2.0;
			bounds->height += thickness / 2.0;
			break;
		default: //PenLineCapFlat
			bounds->width += dx/2.0;
			bounds->height += dy/2.0;
		}
		switch (end_cap) {
		case PenLineCapSquare:
			bounds->width += (dx + dy) / 2.0;
			bounds->height += (dx + dy) / 2.0;
			break;
		case PenLineCapTriangle: //FIXME, reverting to Round for now
		case PenLineCapRound:
			bounds->width += thickness / 2.0;
			bounds->height += thickness / 2.0;
			break;
		default: //PenLineCapFlat
			bounds->width += dx/2.0;
			bounds->height += dy/2.0;	
		}
	}
}

void
Line::BuildPath ()
{
	SetShapeFlags (UIElement::SHAPE_NORMAL);

	path = moon_path_renew (path, MOON_PATH_MOVE_TO_LENGTH + MOON_PATH_LINE_TO_LENGTH);
	
	double x1 = GetX1 ();
	double y1 = GetY1 ();
	double x2 = GetX2 ();
	double y2 = GetY2 ();
	
	moon_move_to (path, x1, y1);
	moon_line_to (path, x2, y2);
}

Rect
Line::ComputeShapeBounds (bool logical)
{
	Rect shape_bounds = Rect ();
	double thickness;

	if (!logical)
		thickness = GetStrokeThickness ();
	else
		thickness = 0.0;

	PenLineCap start_cap, end_cap;
	if (!logical) {
		start_cap = GetStrokeStartLineCap ();
		end_cap = GetStrokeEndLineCap ();
	} else 
		start_cap = end_cap = PenLineCapFlat;
	
	if (thickness <= 0.0 && !logical)
		return shape_bounds;
	
	double x1 = GetX1 ();
	double y1 = GetY1 ();
	double x2 = GetX2 ();
	double y2 = GetY2 ();
	
	calc_line_bounds (x1, x2, y1, y2, thickness, start_cap, end_cap, &shape_bounds);

	return shape_bounds;
}

void
Line::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::LINE) {
		Shape::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Line::X1Property ||
	    args->GetId () == Line::X2Property ||
	    args->GetId () == Line::Y1Property ||
	    args->GetId () == Line::Y2Property) {
		InvalidateNaturalBounds ();
	}

	NotifyListenersOfPropertyChange (args, error);
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

Polygon::Polygon ()
{
	SetObjectType (Type::POLYGON);
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

void
Polygon::BuildPath ()
{
	PointCollection *col = GetPoints ();
	
	// the first point is a move to, resulting in an empty shape
	if (!col || (col->GetCount() < 2)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	int i, count = col->GetCount();
	GPtrArray* points = col->Array();

	SetShapeFlags (UIElement::SHAPE_NORMAL);

	// 2 data per [move|line]_to + 1 for close path
	path = moon_path_renew (path, count * 2 + 1);

	// special case, both the starting and ending points are 5 * thickness than the actual points
	if (count == 2) {
		double thickness = GetStrokeThickness ();
		double x1 = ((Value*)g_ptr_array_index(points, 0))->AsPoint()->x;
		double y1 = ((Value*)g_ptr_array_index(points, 0))->AsPoint()->y;
		double x2 = ((Value*)g_ptr_array_index(points, 1))->AsPoint()->x;
		double y2 = ((Value*)g_ptr_array_index(points, 1))->AsPoint()->y;
		
		polygon_extend_line (&x1, &x2, &y1, &y2, thickness);

		moon_move_to (path, x1, y1);
		moon_line_to (path, x2, y2);
	} else {
		moon_move_to (path,
			      ((Value*)g_ptr_array_index(points, 0))->AsPoint()->x,
			      ((Value*)g_ptr_array_index(points, 0))->AsPoint()->y);
		for (i = 1; i < count; i++)
			moon_line_to (path,
				      ((Value*)g_ptr_array_index(points, i))->AsPoint()->x,
				      ((Value*)g_ptr_array_index(points, i))->AsPoint()->y);
	}
	moon_close_path (path);
}

void
Polygon::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::POLYGON) {
		Shape::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Polygon::PointsProperty) {
		InvalidateNaturalBounds ();
	}

	Invalidate ();
	NotifyListenersOfPropertyChange (args, error);
}

void
Polygon::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	Shape::OnCollectionChanged (col, args);

	InvalidateNaturalBounds ();
}

void
Polygon::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	Shape::OnCollectionItemChanged (col, obj, args);
	
	InvalidateNaturalBounds ();
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

Polyline::Polyline ()
{
	SetObjectType (Type::POLYLINE);
}

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
	PenLineCap start = GetStrokeStartLineCap ();
	PenLineCap end = GetStrokeEndLineCap ();
	PenLineCap dash = GetStrokeDashCap ();
	
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

void
Polyline::BuildPath ()
{
	PointCollection *col = GetPoints ();
	
	// the first point is a move to, resulting in an empty shape
	if (!col || (col->GetCount() < 2)) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return;
	}

	int i, count = col->GetCount();
	GPtrArray *points = col->Array();

	SetShapeFlags (UIElement::SHAPE_NORMAL);

	// 2 data per [move|line]_to
	path = moon_path_renew (path, count * 2);

	moon_move_to (path,
		      ((Value*)g_ptr_array_index(points, 0))->AsPoint()->x,
		      ((Value*)g_ptr_array_index(points, 0))->AsPoint()->y);

	for (i = 1; i < count; i++)
		moon_line_to (path,
			      ((Value*)g_ptr_array_index(points, i))->AsPoint()->x,
			      ((Value*)g_ptr_array_index(points, i))->AsPoint()->y);
}

void
Polyline::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::POLYLINE) {
		Shape::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Polyline::PointsProperty) {
		InvalidateNaturalBounds ();
	}

	Invalidate ();
	NotifyListenersOfPropertyChange (args, error);
}

void
Polyline::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col != GetPoints ()) {
		Shape::OnCollectionChanged (col, args);
		return;
	}
	
	InvalidateNaturalBounds ();
}

void
Polyline::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	Shape::OnCollectionItemChanged (col, obj, args);
	
	InvalidateNaturalBounds ();
}

//
// Path
//
bool
Path::SetupLine (cairo_t* cr)
{
	// we cannot use the thickness==0 optimization (like Shape::SetupLine provides)
	// since we'll be using cairo to compute the path's bounds later
	// see bug #352188 for an example of what this breaks
	double thickness = IsDegenerate () ? 1.0 : GetStrokeThickness ();
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
	Geometry *geometry;
	
	if (!(geometry = GetData ()))
		return Shape::GetFillRule ();
	
	return geometry->GetFillRule ();
}

Rect
Path::ComputeShapeBounds (bool logical, cairo_matrix_t *matrix)
{
	Rect shape_bounds = Rect ();
	Geometry *geometry;
	
	if (!(geometry = GetData ())) {
		SetShapeFlags (UIElement::SHAPE_EMPTY);
		return shape_bounds;
	}
	
	if (logical)
		return geometry->GetBounds ();
		
	double thickness = !IsStroked () ? 0.0 : GetStrokeThickness ();
	
	cairo_t *cr = measuring_context_create ();
	cairo_set_line_width (cr, thickness);

	if (thickness > 0.0) {
		//FIXME: still not 100% precise since it could be different from the end cap
		PenLineCap cap = GetStrokeStartLineCap ();
		if (cap == PenLineCapFlat)
			cap = GetStrokeEndLineCap ();
		cairo_set_line_cap (cr, convert_line_cap (cap));
	}

	if (matrix)
		cairo_set_matrix (cr, matrix);
	geometry->Draw (cr);

	cairo_identity_matrix (cr);

	double x1, y1, x2, y2;

	if (thickness > 0) {
		cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	} else {
		cairo_fill_extents (cr, &x1, &y1, &x2, &y2);
	}

        shape_bounds = Rect (MIN (x1, x2), MIN (y1, y2), fabs (x2 - x1), fabs (y2 - y1));
	
	measuring_context_destroy (cr);

	return shape_bounds;
}

void
Path::Draw (cairo_t *cr)
{
	cairo_new_path (cr);
	
	Geometry *geometry;
	
	if (!(geometry = GetData ()))
		return;
	
	cairo_save (cr);
	cairo_transform (cr, &stretch_transform);
	geometry->Draw (cr);
	cairo_restore (cr);
}

void
Path::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::PATH) {
		Shape::OnPropertyChanged (args, error);
		return;
	}

	InvalidateNaturalBounds ();

	NotifyListenersOfPropertyChange (args, error);
}

void
Path::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && prop->GetId () == Path::DataProperty) {
		InvalidateNaturalBounds ();
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
