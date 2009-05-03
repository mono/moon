/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * stylus.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>
#include <math.h>

#include "stylus.h"
#include "collection.h"
#include "color.h"
#include "moon-path.h"
#include "error.h"

#define DEBUG_HITTEST 0


//
// StylusPointCollection
//

bool
StylusPointCollection::CanAdd (Value *value)
{
	// We skip DependencyObjectCollection::CanAdd because that one
	// mandates 1 parent per DO, which StylusPoints violate.
	return Collection::CanAdd (value) && !Contains (value);
}

double
StylusPointCollection::AddStylusPoints (StylusPointCollection *points)
{
	if (!points)
		return 1.0; // documented as such, needs testing
	
	for (int i = 0; i < points->GetCount (); i++)
		Add (points->GetValueAt (i)->AsDependencyObject ());
	
	return array->len - 1;
}

Rect
StylusPointCollection::GetBounds ()
{
	if (array->len == 0)
		return Rect (0, 0, 0, 0);
	
	StylusPoint *point = GetValueAt (0)->AsStylusPoint ();
	Rect r = Rect (point->GetX (), point->GetY (), 0, 0);
	
	for (guint i = 1; i < array->len; i++) {
		point = GetValueAt (i)->AsStylusPoint ();
		r = r.ExtendTo (point->GetX (), point->GetY ());
	}
	
	return r;
}


//
// Stroke
//

Stroke::Stroke ()
{
	SetObjectType (Type::STROKE);
	old_bounds = Rect ();
	bounds = Rect ();
	dirty = Rect ();
}

bool
Stroke::HitTestEndcapSegment (Point c,
			      double w, double h,
			      Point p1, Point p2)
{
	Point op1 = p1;
	Point op2 = p2;

#if DEBUG_HITTEST
	fprintf (stderr, "HitTestEndcapSegment: (%g,%g / %g, %g) hits segment (%g,%g  - %g,%g)?\n",
		c.x, c.y,
		w, h,
		p1.x, p1.y,
		p2.x, p2.y);
#endif

	// handle dx == 0
	if (p2.x == p1.x) {
#if DEBUG_HITTEST
		fprintf (stderr, "dx == 0, returning %d\n", p1.x >= (c.x - w/2) && p1.x <= (c.x + w/2));
#endif
 		if (p1.x >= (c.x - w/2) && p1.x <= (c.x + w/2)) {
			if (p1.y < (c.y - h/2) && p2.y < (c.y - h/2))
				return false;
			if (p1.y > (c.y + h/2) && p2.y > (c.y + h/2))
				return false;
			return true;
		}
		return false;
	}

  	p1 = p1 - c;
  	p2 = p2 - c;

	// this body of code basically uses the following form of the line:
	//
	// y = mx + b_
	//
	// and the equation for an ellipse:
	//
	// x^2    y^2
	// ---    ---  - 1 = 0
	// a^2    b^2
	//
	// where a > b (as leave off the center point of the ellipse
	// because we've subtracted it out above).
	//
	// we substitute the line equation in for y in the ellipse
	// equation, and use the quadratic formula to find our roots
	// (if there are any).

	double m = (p2.y - p1.y)/(p2.x - p1.x);
	double b_ = p1.y - m * p1.x;

	double a, b;
	if (w > h) {
		a = w / 2;
		b = h / 2;
	}
	else {
		a = h / 2;
		b = w / 2;
	}

	if (b == 0 || a == 0) {
		return false;
	}

	double aq = (m*m) / (b*b) + 1 / (a*a);
	double bq = (2 * m * b_) / (b * b);
	double cq = (b_ * b_) / (b * b) - 1;

	double discr =  bq * bq - 4 * aq * cq;

#if DEBUG_HITTEST
	fprintf (stderr, "HitTestEndCapSegment: discr = %g\n", discr);
#endif	

	// if we have roots we need to check if they occur on the line
	// segment (using the parametric form of the line).
	if (discr < 0)
		return false;
	else {
		double sqrt_discr = discr > 0 ? sqrt(discr) : 0;

		double root_1 = ((- bq) - sqrt_discr) / (2 * aq);
		if (root_1 > p1.x && (root_1 - p1.x) < (p2.x - p1.x))
			return true;

		double root_2 = (- bq + sqrt_discr) / (2 * aq);
		return (root_2 > p1.x && (root_2 - p1.x) < (p2.x - p1.x));
	}
}

static bool
intersect_line_2d (Point p1, Point p2, Point p3, Point p4)
{
	// taken from http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
	// line segments are p1 - p2 and p3 - p4

	double denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);

	if (denom == 0) // they're parallel
		return false;

	double ua = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
	ua /= denom;

	double ub = (p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x);
	ub /= denom;
	if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1)
		return true;

	return false;
}

// given the line segment between @p1 and @p2, with an ellipse with
// width = @w and height = @h centered at point @p, return the left
// (lesser x coordinate if the x's are different, and lesser y
// coordinate if they're the same) and right (greater x coordinate, or
// greater y coordinate) intersection points of the line through the
// same point @p but perpendicular to line @p2 - @p1.
static void
calc_perpendicular_intersection_points (Point p1, Point p2,
					Point p,
					double w, double h,
					Point *left_point, Point *right_point)
{
	if (p2.y == p1.y) { // horizontal line
		*left_point = Point (p.x, p.y - h / 2);
		*right_point = Point (p.x, p.y + h / 2);
	}
	else if (p2.x == p1.x) { // vertical line
		*left_point = Point (p.x - w / 2, p.y);
		*right_point = Point (p.x + w / 2, p.y);
	}
	else {
		// slope of the perpendicular line
		double m = -(p2.x - p1.x)/p2.y - p1.y;

		double a, b;
		if (w > h) {
			a = w / 2;
			b = h / 2;
		}
		else {
			a = h / 2;
			b = w / 2;
		}

		double aq = (m*m) / (b*b) + 1 / (a*a);

		double discr =  4 * aq;

		if (discr <= 0) {
			g_warning ("should never happen, there should always be two roots");
			*left_point = p;
			*right_point = p;
		}
		else {
			double sqrt_discr = sqrt (discr);

			double root = (sqrt_discr) / (2 * aq);

			*left_point = Point (-root + p.x, (-root * m) + p.y);
			*right_point = Point (root + p.x, (root * m) + p.y);
		}

	}
}

bool
Stroke::HitTestSegmentSegment (Point stroke_p1, Point stroke_p2,
			       double w, double h,
			       Point p1, Point p2)
{
#if DEBUG_HITTEST
	fprintf (stderr, "HitTestSegmentSegment: (%g,%g - %g,%g / %g, %g) hits segment (%g,%g - %g,%g) ?\n",
		stroke_p1.x, stroke_p1.y,
		stroke_p2.x, stroke_p2.y,
		w, h,
		p1.x, p1.y,
		p2.x, p2.y);
#endif
	Point left_stroke_p1, right_stroke_p1;
	Point left_stroke_p2, right_stroke_p2;

	calc_perpendicular_intersection_points (stroke_p1, stroke_p2, stroke_p1, w, h, &left_stroke_p1, &right_stroke_p1);
	calc_perpendicular_intersection_points (stroke_p1, stroke_p2, stroke_p2, w, h, &left_stroke_p2, &right_stroke_p2);

	if (intersect_line_2d (left_stroke_p1, left_stroke_p2, p1, p2))
		return true;
	if (intersect_line_2d (right_stroke_p1, right_stroke_p2, p1, p2))
		return true;

	return false;
}

bool
Stroke::HitTestEndcapPoint (Point c,
			    double w, double h,
			    Point p)
{
#if DEBUG_HITTEST
	fprintf (stderr, "HitTestEndcapPoint: (%g,%g / %g, %g) hits point %g,%g ?\n",
		c.x, c.y,
		w, h,
		p.x, p.y);
#endif

	Point dp = p - c;
	double a, b;

	a = w / 2;
	b = h / 2;

	bool rv = ((dp.x * dp.x) / (a * a) + (dp.y * dp.y) / (b * b)) < 1;
#if DEBUG_HITTEST
	fprintf (stderr, " + %s\n", rv ? "TRUE" : "FALSE");
#endif
	return rv;
}

static bool
point_gte_line (Point p,
		Point p1, Point p2)
{
	// return true if the point is to the right of or beneath the line segment

	if (p1.y == p2.y) {
		return p.y > p1.y;
	}
	else if (p1.x == p2.x)
		return p.x > p1.x;
	else {
		double m = (p2.y - p1.y) / (p2.x - p1.x);

	  	if (m > 0)
			return p.y < (p1.y + m * p.x);
		else
			return p.y > (p1.y + m * p.x);
	}
}

static bool
point_lte_line (Point p,
		Point p1, Point p2)
{
	// return true if the point is to the right of or beneath the line segment

	if (p1.y == p2.y)
		return p.y < p1.y;
	else if (p1.x == p2.x)
		return p.x < p1.x;
	else {
		double m = (p2.y - p1.y) / (p2.x - p1.x);

	  	if (m > 0)
			return p.y > (p1.y + m * p.x);
		else
			return p.y < (p1.y + m * p.x);
	}
}

bool
Stroke::HitTestSegmentPoint (Point stroke_p1, Point stroke_p2,
			     double w, double h,
			     Point p)
{
#if DEBUG_HITTEST
	fprintf (stderr, "HitTestSegment: (%g,%g - %g,%g / %g, %g) hits point (%g,%g) ?\n",
		stroke_p1.x, stroke_p1.y,
		stroke_p2.x, stroke_p2.y,
		w, h,
		p.x, p.y);
#endif

	Point left_stroke_p1, right_stroke_p1;
	Point left_stroke_p2, right_stroke_p2;

	calc_perpendicular_intersection_points (stroke_p1, stroke_p2, stroke_p1, w, h, &left_stroke_p1, &right_stroke_p1);
	calc_perpendicular_intersection_points (stroke_p1, stroke_p2, stroke_p2, w, h, &left_stroke_p2, &right_stroke_p2);

	return point_gte_line (p, left_stroke_p1, left_stroke_p2) && point_lte_line (p, right_stroke_p1, right_stroke_p2);
}
			      

bool
Stroke::HitTestSegment (Point p1, Point p2, double w, double h, StylusPointCollection *stylusPoints)
{
	StylusPoint *sp;
	
	if (HitTestEndcap (p1, w, h, stylusPoints))
		return true;
	
	if (HitTestEndcap (p2, w, h, stylusPoints))
		return true;
	
	for (int i = 0; i < stylusPoints->GetCount (); i++) {
		sp = stylusPoints->GetValueAt (i)->AsStylusPoint ();
		
		if (i + 1 == stylusPoints->GetCount ()) {
			Point p (sp->GetX (), sp->GetY ());
			
			if (!bounds.PointInside (p))
				continue;

			if (HitTestSegmentPoint (p1, p2,
						 w, h,
						 p))
				return true;
		}
		else  {
			StylusPoint *next_sp = stylusPoints->GetValueAt (i + 1)->AsStylusPoint ();
			i++;
			
			Point p (sp->GetX (), sp->GetY ());
			Point next_p (next_sp->GetX (), next_sp->GetY ());
			
			if (HitTestSegmentSegment (p1, p2,
						   w, h,
						   p, next_p))
				return true;
		}
	}
	
	return false;
}

bool
Stroke::HitTestEndcap (Point p, double w, double h, StylusPointCollection *stylusPoints)
{
	StylusPoint *sp = stylusPoints->GetValueAt (0)->AsStylusPoint ();
	Point cur, next;
	
	cur.x = sp->GetX ();
	cur.y = sp->GetY ();
	
	if (stylusPoints->GetCount () < 2) {
		// singleton input point to match against
		if (bounds.PointInside (cur)) {
			if (HitTestEndcapPoint (p, w, h, cur))
				return true;
			
#if DEBUG_HITTEST
			fprintf (stderr, "\t(%f, %f) EndcapPoint failed\n",
				 cur.x, cur.y);
#endif
		} else {
#if DEBUG_HITTEST
			fprintf (stderr, "\t(%f, %f) is not within bounds\n",
				 cur.x, cur.y);
#endif
		}
	}
	
	for (int i = 1; i < stylusPoints->GetCount (); i++) {
		sp = stylusPoints->GetValueAt (i)->AsStylusPoint ();
		next.x = sp->GetX ();
		next.y = sp->GetY ();
		
		if (HitTestEndcapSegment (p, w, h, cur, next))
			return true;
		
#if DEBUG_HITTEST
		fprintf (stderr, "\t(%f, %f) (%f, %f) EndcapSegment failed\n",
			 cur.x, cur.y, next.x, next.y);
#endif
		
		cur.x = next.x;
		cur.y = next.y;
	}
	
	return false;
}

bool
Stroke::HitTest (StylusPointCollection *stylusPoints)
{
	StylusPointCollection *myStylusPoints = GetStylusPoints ();
	
	if (myStylusPoints->GetCount () == 0) {
#if DEBUG_HITTEST
		fprintf (stderr, "no points in the collection, returning false!\n");
#endif
		return false;
	}
	
	DrawingAttributes *da = GetDrawingAttributes ();
	StylusPoint *sp;
	
	double height, width;

	if (da) {
		height = da->GetHeight ();
		width = da->GetWidth ();

		Color *col = da->GetOutlineColor ();
		if (col->a != 0x00) {
			height += 4.0;
			width += 4.0;
		}
	}
	else {
		height = width = 6.0;

	}
	
#if DEBUG_HITTEST
	fprintf (stderr, "Stroke::HitTest()\n");
	fprintf (stderr, "\tInput points:\n");
	
	for (int i = 0; i < stylusPoints->GetCount (); i++) {
		sp = stylusPoints->GetValueAt (i)->AsStylusPoint ();
		
		fprintf (stderr, "\t\tPoint: (%f, %f)\n", sp->GetX (), sp->GetY ());
	}
	
	fprintf (stderr, "\tStroke points:\n");
	
	for (int i = 0; i < myStylusPoints->GetCount (); i++) {
		sp = myStylusPoints->GetValueAt (i)->AsStylusPoint ();
		
		fprintf (stderr, "\t\tPoint: (%f, %f)\n", sp->GetX (), sp->GetY ());
	}
#endif	
	if (!GetBounds ().IntersectsWith (stylusPoints->GetBounds ()))
		return false;

	/* test the beginning endcap */
	sp = myStylusPoints->GetValueAt (0)->AsStylusPoint ();
	
	if (HitTestEndcap (Point (sp->GetX (), sp->GetY ()),
			   width, height, stylusPoints)) {
#if DEBUG_HITTEST
		fprintf (stderr, "\tA point matched the beginning endcap\n");
#endif
		return true;
	}
	
	/* test all the interior line segments */
	StylusPoint *prev_point = sp;
	for (int i = 1; i < myStylusPoints->GetCount (); i++) {
		sp = myStylusPoints->GetValueAt (i)->AsStylusPoint ();
		
		if (HitTestSegment (Point (prev_point->GetX (), prev_point->GetY ()),
				    Point (sp->GetX (), sp->GetY ()),
				    width, height, stylusPoints)) {
#if DEBUG_HITTEST
			fprintf (stderr, "\tA point matched an interior line segment\n");
#endif
			return true;
		}
	}

	/* the the ending endcap */
	if (myStylusPoints->GetCount () > 1) {
		sp = myStylusPoints->GetValueAt (myStylusPoints->GetCount () - 1)->AsStylusPoint ();
		
		if (HitTestEndcap (Point (sp->GetX (), sp->GetY ()),
				   width, height, stylusPoints)) {
#if DEBUG_HITTEST
			fprintf (stderr, "\tA point matched the ending endcap\n");
#endif
			return true;
		}
	}
	
#if DEBUG_HITTEST
	fprintf (stderr, "\tso sad, no points intersected...\n");
#endif
	
	return false;
}

Rect
Stroke::AddStylusPointToBounds (StylusPoint *stylus_point, const Rect &bounds)
{
	DrawingAttributes *da = GetDrawingAttributes ();
	double height, width;

	if (da) {
		height = da->GetHeight ();
		width = da->GetWidth ();
		
		Color *col = da->GetOutlineColor ();
		if (col->a != 0x00) {
			height += 4.0;
			width += 4.0;
		}
	} else {
		height = width = 6.0;
	}
	
	return bounds.Union (Rect (stylus_point->GetX () - width / 2,
				   stylus_point->GetY () - height / 2,
				   width, height));
}

void
Stroke::ComputeBounds ()
{
	bounds = Rect ();
	
	StylusPointCollection *spc = GetStylusPoints ();
	if (!spc)
		return;
	
	for (int i = 0; i < spc->GetCount (); i++)
		bounds = AddStylusPointToBounds (spc->GetValueAt (i)->AsStylusPoint (), bounds);
}

void
Stroke::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	Rect point;
	
	if (col != GetStylusPoints ()) {
		DependencyObject::OnCollectionChanged (col, args);
		return;
	}
	
	old_bounds = bounds;
	
	switch (args->GetChangedAction ()) {
	case CollectionChangedActionAdd:
		// add previous point to dirty
		if (args->GetIndex() > 0)
			dirty = AddStylusPointToBounds (col->GetValueAt (args->GetIndex() - 1)->AsStylusPoint (), dirty);
		
		// add new point to dirty
		dirty = AddStylusPointToBounds (args->GetNewItem()->AsStylusPoint (), dirty);
		
		// add next point to dirty
		if (args->GetIndex() + 1 < col->GetCount ())
			dirty = AddStylusPointToBounds (col->GetValueAt (args->GetIndex() + 1)->AsStylusPoint (), dirty);
		
		// update official bounds
		bounds = bounds.Union (dirty);
		break;
	case CollectionChangedActionRemove:
	case CollectionChangedActionReplace:
	case CollectionChangedActionCleared:
		ComputeBounds ();
		dirty = dirty.Union (old_bounds.Union (bounds));
		break;
	case CollectionChangedActionClearing:
		// nothing needed here.
		break;
	}
	
	NotifyListenersOfPropertyChange (Stroke::StylusPointsProperty, NULL);
}

void
Stroke::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col != GetStylusPoints ()) {
		DependencyObject::OnCollectionItemChanged (col, obj, args);
		return;
	}

	old_bounds = bounds;

	ComputeBounds ();
	
	dirty = old_bounds.Union (bounds);
	
	NotifyListenersOfPropertyChange (Stroke::StylusPointsProperty, NULL);
}

void
Stroke::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::STROKE) {
		DependencyObject::OnPropertyChanged (args, error);
	}

	if (args->GetId () == Stroke::DrawingAttributesProperty) {
		ComputeBounds ();
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
Stroke::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop->GetId () == Stroke::DrawingAttributesProperty) {
		if (subobj_args->GetId () == DrawingAttributes::WidthProperty ||
		    subobj_args->GetId () == DrawingAttributes::HeightProperty ||
		    subobj_args->GetId () == DrawingAttributes::OutlineColorProperty) {
			ComputeBounds ();
		}
	}

	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
}


//
// StrokeCollection
//

bool
StrokeCollection::CanAdd (Value *value)
{
	// We skip DependencyObjectCollection::CanAdd because that one
	// mandates 1 parent per DO, which strokes violate.
	return Collection::CanAdd (value) && !Contains (value);
}

bool
StrokeCollection::AddedToCollection (Value *value, MoonError *error)
{
	DependencyObject *obj = value->AsDependencyObject ();
	
	obj->SetSurface (GetSurface ());
	obj->SetParent (this, error);
	obj->AddPropertyChangeListener (this);
	
	// Bypass DependencyObjectCollection::AddedToCollection(), we
	// are handling everything it would normally do. Also Clear()
	// the MoonError because we ignore any errors from
	// SetLogicalParent() since we'll only get an error if the
	// stroke already has a logical parent (which is OK for
	// strokes).
	
	error->Clear ();
	
	return Collection::AddedToCollection (value, error);
}

Rect
StrokeCollection::GetBounds ()
{
	Rect r = Rect (0, 0, 0, 0);
	
	for (guint i = 0; i < array->len; i++)
		r = r.Union (((Value *) array->pdata[i])->AsStroke ()->GetBounds ());
	
	return r;
}

StrokeCollection *
StrokeCollection::HitTest (StylusPointCollection *stylusPoints)
{
	StrokeCollection *result = new StrokeCollection ();
	
	if (stylusPoints->GetCount () == 0)
		return result;
	
	for (guint i = 0; i < array->len; i++) {
		Stroke *s = ((Value *) array->pdata[i])->AsStroke ();
		
		if (s->HitTest(stylusPoints))
			result->Add (s);
	}
	
	return result;
}

static void
drawing_attributes_quick_render (cairo_t *cr, double thickness, Color *color, StylusPointCollection *collection)
{
	StylusPoint *sp;
	double x, y;
	
	if (collection->GetCount () == 0)
		return;
	
	sp = collection->GetValueAt (0)->AsStylusPoint ();
	x = sp->GetX ();
	y = sp->GetY ();
	
	cairo_move_to (cr, x, y);
	
	if (collection->GetCount () > 1) {
		for (int i = 1; i < collection->GetCount (); i++) {
			sp = collection->GetValueAt (i)->AsStylusPoint ();
			x = sp->GetX ();
			y = sp->GetY ();
			
			cairo_line_to (cr, x, y);
		}
	} else {
		cairo_line_to (cr, x, y);
	}
	
	if (color)
		cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
	else
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);

	cairo_set_line_width (cr, thickness);
	cairo_stroke (cr);
}

static void
drawing_attributes_normal_render (cairo_t *cr, double width, double height, Color *color, Color *outline, StylusPointCollection *collection)
{
	// FIXME: use cairo_stroke_to_path once available
	// until then draw bigger with the outline color and smaller with the inner color
	drawing_attributes_quick_render (cr, height + 4.0, outline, collection);
	drawing_attributes_quick_render (cr, (height > 4.0) ? height - 2.0 : 2.0, color, collection);
}

void
DrawingAttributes::Render (cairo_t *cr, StylusPointCollection *collection)
{
	if (!collection)
		return;

	double height = GetHeight ();
	double width = GetWidth ();
	Color *color = GetColor ();
	Color *outline = GetOutlineColor ();
	
	// we can render very quickly if the pen is round, i.e. Width==Height (circle)
	// and when no OutlineColor are specified (e.g. NULL, transparent)
	if ((!outline || outline->a == 0x00) && (height == width)) {
		drawing_attributes_quick_render (cr, height, color, collection);
		// TODO - we could add another fast-path in the case where height!=width and without an outline
		// in this case we would need a scaling transform (for the pen) and adjust the coordinates
	} else {
		drawing_attributes_normal_render (cr, width, height, color, outline, collection);
	}
}

void
DrawingAttributes::RenderWithoutDrawingAttributes (cairo_t *cr, StylusPointCollection *collection)
{
	// default values that (seems to) match the output when no DrawingAttributes are specified
	drawing_attributes_quick_render (cr, 2.0, NULL, collection);
}


//
// InkPresenter
//

InkPresenter::InkPresenter ()
{
	SetObjectType (Type::INKPRESENTER);
}

void
InkPresenter::PostRender (cairo_t *cr, Region *region, bool front_to_back)
{
	// render our chidren if not in front to back mode
	if (!front_to_back) {
		VisualTreeWalker walker = VisualTreeWalker (this, ZForward);
		while (UIElement *child = walker.Step ())
			child->DoRender (cr, region);
	}
	
	cairo_set_matrix (cr, &absolute_xform);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

	StrokeCollection *strokes = GetStrokes ();
	// for each stroke in collection
	for (int i = 0; i < strokes->GetCount (); i++) {
		Stroke *stroke = strokes->GetValueAt (i)->AsStroke ();
		DrawingAttributes *da = stroke->GetDrawingAttributes ();
		StylusPointCollection *spc = stroke->GetStylusPoints ();
			
		if (da) {
			da->Render (cr, spc);
		} else {
			DrawingAttributes::RenderWithoutDrawingAttributes (cr, spc);
		}
		
		stroke->ResetDirty ();
	}

	// Chain up in front_to_back mode since we've alread rendered content
	UIElement::PostRender (cr, region, true);
}

	
void
InkPresenter::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::INKPRESENTER) {
		Canvas::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == InkPresenter::StrokesProperty) {
		// be smart about invalidating only the union of the
		// old stroke bounds and the new stroke bounds

		if (args->GetOldValue()) {
			StrokeCollection *strokes = args->GetOldValue()->AsStrokeCollection();
			if (strokes)
				Invalidate (strokes->GetBounds().Transform (&absolute_xform));
			//XXX else ?
		}

		if (args->GetNewValue()) {
			StrokeCollection *strokes = args->GetNewValue()->AsStrokeCollection();
			if (strokes)
				Invalidate (strokes->GetBounds().Transform (&absolute_xform));
			//XXX else ?
		}

		UpdateBounds ();
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
InkPresenter::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	Stroke *stroke;
	
	if (col != GetStrokes ()) {
		Canvas::OnCollectionChanged (col, args);
		return;
	}
	
	switch (args->GetChangedAction()) {
	case CollectionChangedActionAdd:
		stroke = args->GetNewItem()->AsStroke ();
		Invalidate (stroke->GetBounds().Transform (&absolute_xform));
		UpdateBounds ();
		break;
	case CollectionChangedActionRemove:
		stroke = args->GetOldItem()->AsStroke ();
		Invalidate (stroke->GetOldBounds ().Transform (&absolute_xform));
		Invalidate (stroke->GetBounds ().Transform (&absolute_xform));
		UpdateBounds ();
		break;
	case CollectionChangedActionReplace:
		stroke = args->GetOldItem()->AsStroke ();
		Invalidate (stroke->GetOldBounds ().Transform (&absolute_xform));
		stroke = args->GetNewItem()->AsStroke ();
		Invalidate (stroke->GetBounds().Transform (&absolute_xform));
		UpdateBounds ();
		break;
	case CollectionChangedActionCleared:
		Invalidate (render_bounds);
		Invalidate (((StrokeCollection*)col)->GetBounds().Transform (&absolute_xform));
		UpdateBounds ();
		break;
	case CollectionChangedActionClearing:
		// nothing needed here.
		break;
	}
}

void
InkPresenter::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	Stroke *stroke = (Stroke *) obj;
	
	if (col != GetStrokes ()) {
		Canvas::OnCollectionItemChanged (col, obj, args);
		return;
	}
	
	Invalidate (stroke->GetDirty ().Transform (&absolute_xform));
	UpdateBounds ();
}

void
InkPresenter::ComputeBounds ()
{
	Canvas::ComputeBounds ();

	render_bounds = bounds;
	
	StrokeCollection *strokes = GetStrokes ();
	if (!strokes)
		return;

	Rect stroke_bounds = strokes->GetBounds ();
	stroke_bounds = stroke_bounds.Transform (&absolute_xform);
	bounds_with_children = bounds_with_children.Union (stroke_bounds);

	render_bounds = render_bounds.Union (stroke_bounds);
}

Rect
InkPresenter::GetRenderBounds ()
{
	return render_bounds;
}

void
InkPresenter::ShiftPosition (Point p)
{
	double dx = p.x - bounds.x;
	double dy = p.y - bounds.y;

	// need to do this after computing the delta
	Canvas::ShiftPosition (p);

	render_bounds.x += dx;
	render_bounds.y += dy;
}

void
stroke_get_bounds (Stroke *stroke, Rect *bounds)
{
	*bounds = stroke->GetBounds ();
}

void
stroke_collection_get_bounds (StrokeCollection *collection, Rect *bounds)
{
	*bounds = collection->GetBounds ();
}
