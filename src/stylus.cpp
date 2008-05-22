/*
 * stylus.cpp
 *
 * Author:
 *   Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *   Sebastien Pouliot  <sebastien@ximian.com>
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

#define DEBUG_HITTEST 0

StylusInfo*
stylus_info_new (void)
{
	return new StylusInfo ();
}

TabletDeviceType
stylus_info_get_device_type (StylusInfo* stylus_info)
{
	return (TabletDeviceType) stylus_info->GetValue (StylusInfo::DeviceTypeProperty)->AsInt32 ();
}

void
stylus_info_set_device_type (StylusInfo* stylus_info, TabletDeviceType type)
{
	stylus_info->SetValue (StylusInfo::DeviceTypeProperty, Value (type));
}

bool
stylus_info_get_inverted (StylusInfo* stylus_info)
{
	return stylus_info->GetValue (StylusInfo::IsInvertedProperty)->AsBool ();
}

void
stylus_info_set_inverted (StylusInfo* stylus_info, bool inverted)
{
	stylus_info->SetValue (StylusInfo::IsInvertedProperty, Value (inverted));
}

StylusPoint*
stylus_point_new (void)
{
	return new StylusPoint ();
}

double
stylus_point_get_x (StylusPoint *stylus_point)
{
	return stylus_point->GetValue (StylusPoint::XProperty)->AsDouble();
}

void
stylus_point_set_x (StylusPoint *stylus_point, double x)
{
	stylus_point->SetValue (StylusPoint::XProperty, Value (x));
}

double
stylus_point_get_y (StylusPoint *stylus_point)
{
	return stylus_point->GetValue (StylusPoint::YProperty)->AsDouble();
}

void
stylus_point_set_y (StylusPoint *stylus_point, double y)
{
	stylus_point->SetValue (StylusPoint::YProperty, Value (y));
}

double
stylus_point_get_pressure_factor (StylusPoint *stylus_point)
{
	return stylus_point->GetValue (StylusPoint::PressureFactorProperty)->AsDouble();
}

void
stylus_point_set_pressure_factor (StylusPoint *stylus_point, double pressure)
{
	stylus_point->SetValue (StylusPoint::PressureFactorProperty, Value (pressure));
}


StylusPointCollection *
stylus_point_collection_new (void)
{
	return new StylusPointCollection ();
}

double
stylus_point_collection_add_stylus_points (StylusPointCollection *col, StylusPointCollection *stylusPointCollection)
{
	return col->AddStylusPoints (stylusPointCollection);
}

double
StylusPointCollection::AddStylusPoints (StylusPointCollection *stylusPointCollection)
{
	if (!stylusPointCollection)
		return 1.0; // documented as such, needs testing

	for (Collection::Node *n = (Collection::Node *) stylusPointCollection->list->First (); n; n = (Collection::Node *) n->next)
		Add (n->obj);

	return list->Length () - 1;
}


Stroke::Stroke ()
{
	this->SetValue (Stroke::StylusPointsProperty, Value::CreateUnref (new StylusPointCollection ()));
	this->SetValue (Stroke::DrawingAttributesProperty, Value::CreateUnref (new DrawingAttributes ()));

	bounds = Rect (0,0,0,0);
	old_bounds = Rect (0,0,0,0);
}

Rect
Stroke::GetBounds ()
{
	return bounds;
}

Rect
Stroke::GetOldBounds ()
{
	return old_bounds;
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
	if (ua >= 0 && ua <= 1)
		return true;

	double ub = (p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x);
	ub /= denom;
	if (ub >= 0 && ub <= 1)
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
	Collection::Node *n;
	StylusPoint *sp;

	if (HitTestEndcap (p1, w, h, stylusPoints))
		return true;

	if (HitTestEndcap (p2, w, h, stylusPoints))
		return true;

	for (n = (Collection::Node *) stylusPoints->list->First (); n; n = (Collection::Node *) n->next) {
		sp = (StylusPoint*)n->obj;

		if (n->next == NULL) {
			Point p (stylus_point_get_x (sp),
				 stylus_point_get_y (sp));

			if (!bounds.PointInside (p))
				continue;

			if (HitTestSegmentPoint (p1, p2,
						 w, h,
						 p))
				return true;
		}
		else  {
			n = (Collection::Node *) n->next;
			StylusPoint *next_sp = (StylusPoint*)n->obj;

			Point p (stylus_point_get_x (sp),
				 stylus_point_get_y (sp));
			Point next_p (stylus_point_get_x (next_sp),
				      stylus_point_get_y (next_sp));

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
	Collection::Node *n = (Collection::Node *) stylusPoints->list->First ();
	StylusPoint *sp = (StylusPoint *) n->obj;
	Point cur, next;
	
	cur.x = stylus_point_get_x (sp);
	cur.y = stylus_point_get_y (sp);
	
	if (!n->next) {
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
	
	for (n = (Collection::Node *) n->next; n; n = (Collection::Node *) n->next) {
		sp = (StylusPoint *) n->obj;
		next.x = stylus_point_get_x (sp);
		next.y = stylus_point_get_y (sp);
		
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
	StylusPointCollection *myStylusPoints = stroke_get_stylus_points (this);

	if (myStylusPoints->list->IsEmpty ()) {
#if DEBUG_HITTEST
		fprintf (stderr, "no points in the collection, returning false!\n");
#endif
		return false;
	}

	DrawingAttributes *da = stroke_get_drawing_attributes (this);
	Collection::Node *n;
	StylusPoint *sp;

	double height, width;

	if (da) {
		height = drawing_attributes_get_height (da);
		width = drawing_attributes_get_width (da);

		Color *col = drawing_attributes_get_outline_color (da);
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
	n = (Collection::Node *) stylusPoints->list->First ();
	while (n != NULL) {
		sp = (StylusPoint *) n->obj;
		
		fprintf (stderr, "\t\tPoint: (%f, %f)\n", stylus_point_get_x (sp),
			stylus_point_get_y (sp));
		
		n = (Collection::Node *) n->next;
	}
	fprintf (stderr, "\tStroke points:\n");
	n = (Collection::Node *) myStylusPoints->list->First ();
	while (n != NULL) {
		sp = (StylusPoint *) n->obj;
		
		fprintf (stderr, "\t\tPoint: (%f, %f)\n", stylus_point_get_x (sp),
			stylus_point_get_y (sp));
		
		n = (Collection::Node *) n->next;
	}
#endif	

	/* test the beginning endcap */
	n = (Collection::Node *) myStylusPoints->list->First ();
	sp = (StylusPoint*)n->obj;
	if (HitTestEndcap (Point (stylus_point_get_x (sp),
				  stylus_point_get_y (sp)),
			   width, height, stylusPoints)) {
#if DEBUG_HITTEST
		fprintf (stderr, "\tA point matched the beginning endcap\n");
#endif
		return true;
	}

	/* test all the interior line segments */
	StylusPoint *prev_point = sp;
	for (n = (Collection::Node *)n->next; n; n = (Collection::Node *) n->next) {
		sp = (StylusPoint*)n->obj;
		if (HitTestSegment (Point (stylus_point_get_x (prev_point),
					   stylus_point_get_y (prev_point)),
				    Point (stylus_point_get_x (sp),
					   stylus_point_get_y (sp)),
				    width, height, stylusPoints)) {
#if DEBUG_HITTEST
			fprintf (stderr, "\tA point matched an interior line segment\n");
#endif
			return true;
		}
	}

	/* the the ending endcap */
	if (myStylusPoints->list->Length() > 1) {
		n = (Collection::Node *) myStylusPoints->list->Last ();
		sp = (StylusPoint*)n->obj;
		if (HitTestEndcap (Point (stylus_point_get_x (sp),
					  stylus_point_get_y (sp)),
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

void
Stroke::AddStylusPointToBounds (StylusPoint *stylus_point)
{
	DrawingAttributes *da = stroke_get_drawing_attributes (this);
	double height, width;

	if (da) {
		height = drawing_attributes_get_height (da);
		width = drawing_attributes_get_width (da);

		Color *col = drawing_attributes_get_outline_color (da);
		if (col->a != 0x00) {
			height += 4.0;
			width += 4.0;
		}
	}
	else {
		height = width = 6.0;

	}
	bounds = bounds.Union (Rect (stylus_point_get_x (stylus_point) - width / 2,
				     stylus_point_get_y (stylus_point) - height / 2,
				     width, height));
}

void
Stroke::ComputeBounds ()
{
	bounds = Rect (0,0,0,0);

	StylusPointCollection *spc = stroke_get_stylus_points (this);
	if (!spc)
		return;

	Collection::Node *cnp;
	for (cnp = (Collection::Node *) spc->list->First (); cnp != NULL; cnp = (Collection::Node *) cnp->next)
		AddStylusPointToBounds ((StylusPoint*)cnp->obj);
}

void
Stroke::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	old_bounds = bounds;

	switch (type) {
	case CollectionChangeTypeChanging:
		break;
	case CollectionChangeTypeItemAdded:
		AddStylusPointToBounds ((StylusPoint*)obj);
		break;
	case CollectionChangeTypeItemRemoved:
	case CollectionChangeTypeItemChanged:
	case CollectionChangeTypeChanged:
		ComputeBounds ();
		break;
	}

	NotifyListenersOfPropertyChange (Stroke::StylusPointsProperty);
}

void
Stroke::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::STROKE) {
		DependencyObject::OnPropertyChanged (args);
	}

	if (args->property == Stroke::DrawingAttributesProperty) {
		ComputeBounds ();
	}

	NotifyListenersOfPropertyChange (args);
}

void
Stroke::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Stroke::DrawingAttributesProperty) {
		if (subobj_args->property == DrawingAttributes::WidthProperty ||
		    subobj_args->property == DrawingAttributes::HeightProperty ||
		    subobj_args->property == DrawingAttributes::OutlineColorProperty) {
			ComputeBounds ();
		}
	}

	DependencyObject::OnSubPropertyChanged (prop, obj, subobj_args);
}

Stroke*
stroke_new (void)
{
	return new Stroke ();
}

DrawingAttributes*
stroke_get_drawing_attributes (Stroke *stroke)
{
	Value *value = stroke->GetValue (Stroke::DrawingAttributesProperty);
	return value ? value->AsDrawingAttributes () : NULL;
}

void
stroke_set_drawing_attributes (Stroke *stroke, DrawingAttributes *attributes)
{
	stroke->SetValue (Stroke::DrawingAttributesProperty, Value (attributes));
}

StylusPointCollection*
stroke_get_stylus_points (Stroke *stroke)
{
	Value *value = stroke->GetValue (Stroke::StylusPointsProperty);
	return (value ? value->AsStylusPointCollection () : NULL);
}

void
stroke_set_stylus_points (Stroke *stroke, StylusPointCollection* collection)
{
	stroke->SetValue (Stroke::StylusPointsProperty, Value (collection));
}

void
stroke_get_bounds (Stroke *stroke, Rect *bounds)
{
	*bounds = stroke->GetBounds ();
}

bool
stroke_hit_test (Stroke *stroke, StylusPointCollection *stylusPointCollection)
{
	return stroke->HitTest (stylusPointCollection);
}



StrokeCollection *
stroke_collection_new (void)
{
	return new StrokeCollection ();
}

Rect
StrokeCollection::GetBounds ()
{
	Rect r = Rect (0, 0, 0, 0);
	
	Collection::Node *n;
	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next)
		r = r.Union (((Stroke*)n->obj)->GetBounds());
	
	return r;
}

StrokeCollection*
StrokeCollection::HitTest (StylusPointCollection *stylusPoints)
{
	Collection::Node *n;

	StrokeCollection *result = new StrokeCollection ();

	if (stylusPoints->list->IsEmpty ())
		return result;

	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next) {
		Stroke *s = (Stroke*)n->obj;

		if (s->HitTest(stylusPoints))
			result->Add (s);
	}

	return result;
}

void
stroke_collection_get_bounds (StrokeCollection *col, Rect *bounds)
{
	*bounds = col->GetBounds();
}

StrokeCollection* 
stroke_collection_hit_test (StrokeCollection* col, StylusPointCollection* stylusPointCollection)
{
	return col->HitTest (stylusPointCollection);
}




DrawingAttributes*
drawing_attributes_new (void)
{
	return new DrawingAttributes ();
}

Color*
drawing_attributes_get_color (DrawingAttributes* da)
{
	return da->GetValue (DrawingAttributes::ColorProperty)->AsColor();
}

void
drawing_attributes_set_color (DrawingAttributes* da, Color *color)
{
	da->SetValue (DrawingAttributes::ColorProperty, Value (*color));
}

Color*
drawing_attributes_get_outline_color (DrawingAttributes* da)
{
	return da->GetValue (DrawingAttributes::OutlineColorProperty)->AsColor();
}

void
drawing_attributes_set_outline_color (DrawingAttributes* da, Color *color)
{
	da->SetValue (DrawingAttributes::OutlineColorProperty, Value (*color));
}

double
drawing_attributes_get_height (DrawingAttributes* da)
{
	return da->GetValue (DrawingAttributes::HeightProperty)->AsDouble();
}

void
drawing_attributes_set_height (DrawingAttributes* da, double height)
{
	da->SetValue (DrawingAttributes::HeightProperty, Value (height));
}

double
drawing_attributes_get_width (DrawingAttributes* da)
{
	return da->GetValue (DrawingAttributes::WidthProperty)->AsDouble();
}

void
drawing_attributes_set_width (DrawingAttributes* da, double width)
{
	da->SetValue (DrawingAttributes::WidthProperty, Value (width));
}

static void
drawing_attributes_quick_render (cairo_t *cr, double thickness, Color *color, StylusPointCollection *collection)
{
	Collection::Node *cnp = (Collection::Node *) collection->list->First ();
	if (!cnp)
		return;

	StylusPoint *stylus_point = (StylusPoint*) cnp->obj;
	double x = stylus_point_get_x (stylus_point);
	double y = stylus_point_get_y (stylus_point);
	cairo_move_to (cr, x, y);

	if (!cnp->next) {
		cairo_line_to (cr, x, y);
	} else {
		for (cnp = (Collection::Node *) cnp->next; cnp != NULL; cnp = (Collection::Node *) cnp->next) {
			StylusPoint *stylus_point = (StylusPoint*) cnp->obj;
			cairo_line_to (cr, stylus_point_get_x (stylus_point), stylus_point_get_y (stylus_point));
		}
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
DrawingAttributes::Render (cairo_t *cr, StylusPointCollection* collection)
{
	if (!collection)
		return;

	double height = drawing_attributes_get_height (this);
	double width = drawing_attributes_get_width (this);
	Color *color = drawing_attributes_get_color (this);
	Color *outline = drawing_attributes_get_outline_color (this);

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

InkPresenter::InkPresenter ()
{
	this->SetValue (InkPresenter::StrokesProperty, Value::CreateUnref (new StrokeCollection ()));
}

void
InkPresenter::PostRender (cairo_t *cr, Region *region, bool front_to_back)
{
	// if we didn't render front to back, then render the children here
	if (!front_to_back || !UseBackToFront ()) {
		RenderChildren (cr, region);
	}

	Value* value = GetValue (InkPresenter::StrokesProperty);
	if (!value)
		return;

	StrokeCollection *strokes = value->AsStrokeCollection ();
	if (!strokes)
		return;

	cairo_set_matrix (cr, &absolute_xform);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

	// for each stroke in collection
	Collection::Node *cn = (Collection::Node *) strokes->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
		Stroke *stroke = (Stroke *) cn->obj;

		value = stroke->GetValue (Stroke::DrawingAttributesProperty);
		DrawingAttributes *da = value ? value->AsDrawingAttributes () : NULL;

		value = stroke->GetValue (Stroke::StylusPointsProperty);
		StylusPointCollection *spc = value ? value->AsStylusPointCollection () : NULL;

		if (da) {
			da->Render (cr, spc);
		} else {
			DrawingAttributes::RenderWithoutDrawingAttributes (cr, spc);
		}
	}

	UIElement::PostRender (cr, region, front_to_back);
}

void
InkPresenter::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::INKPRESENTER) {
		Canvas::OnPropertyChanged (args);
		return;
	}

	if (args->property == InkPresenter::StrokesProperty) {
		// be smart about invalidating only the union of the
		// old stroke bounds and the new stroke bounds

		if (args->old_value) {
			StrokeCollection *strokes = args->old_value->AsStrokeCollection();
			Invalidate (strokes->GetBounds().Transform (&absolute_xform));
		}

		if (args->new_value) {
			StrokeCollection *strokes = args->new_value->AsStrokeCollection();
			Invalidate (strokes->GetBounds().Transform (&absolute_xform));
		}

		UpdateBounds ();
	}

	NotifyListenersOfPropertyChange (args);
}

void
InkPresenter::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_Args)
{
	if (col == GetValue (InkPresenter::StrokesProperty)->AsCollection()) {
		switch (type) {
		case CollectionChangeTypeChanging:
			break;
		case CollectionChangeTypeItemAdded:
		case CollectionChangeTypeItemRemoved:
		case CollectionChangeTypeItemChanged: {
			Stroke *stroke = (Stroke*)obj;
			Invalidate (stroke->GetBounds().Transform (&absolute_xform));
			if (type != CollectionChangeTypeItemAdded)
				Invalidate (stroke->GetOldBounds().Transform (&absolute_xform));
			UpdateBounds ();
			break;
		}
		case CollectionChangeTypeChanged:
			Invalidate (render_bounds);
			Invalidate (((StrokeCollection*)col)->GetBounds().Transform (&absolute_xform));
			UpdateBounds ();
			break;
		}
	} else {
		Canvas::OnCollectionChanged (col, type, obj, element_Args);
	}
}

void
InkPresenter::ComputeBounds ()
{
	Canvas::ComputeBounds ();

	render_bounds = bounds;

	Value* value = GetValue (InkPresenter::StrokesProperty);
	if (!value)
		return;

	StrokeCollection *strokes = value->AsStrokeCollection ();
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

InkPresenter*
ink_presenter_new (void)
{
	return new InkPresenter ();
}

StrokeCollection*
ink_presenter_get_strokes (InkPresenter *ink_presenter)
{
	Value *value = ink_presenter->GetValue (InkPresenter::StrokesProperty);
	return (value ? value->AsStrokeCollection () : NULL);
}

void
ink_presenter_set_strokes (InkPresenter *ink_presenter, StrokeCollection* collection)
{
	ink_presenter->SetValue (InkPresenter::StrokesProperty, Value (collection));
}


DependencyProperty* StylusInfo::DeviceTypeProperty;
DependencyProperty* StylusInfo::IsInvertedProperty;

DependencyProperty* StylusPoint::PressureFactorProperty;
DependencyProperty* StylusPoint::XProperty;
DependencyProperty* StylusPoint::YProperty;

DependencyProperty* Stroke::DrawingAttributesProperty;
DependencyProperty* Stroke::StylusPointsProperty;

DependencyProperty* DrawingAttributes::ColorProperty;
DependencyProperty* DrawingAttributes::OutlineColorProperty;
DependencyProperty* DrawingAttributes::HeightProperty;
DependencyProperty* DrawingAttributes::WidthProperty;

DependencyProperty* InkPresenter::StrokesProperty;

void stylus_init ()
{
	StylusInfo::DeviceTypeProperty = DependencyObject::Register (Type::STYLUSINFO, "DeviceType", new Value (TabletDeviceTypeMouse));
	StylusInfo::IsInvertedProperty = DependencyObject::Register (Type::STYLUSINFO, "IsInverted", new Value (false));

	StylusPoint::PressureFactorProperty = DependencyObject::Register (Type::STYLUSPOINT, "PressureFactor", new Value (0.5));
	StylusPoint::XProperty = DependencyObject::Register (Type::STYLUSPOINT, "X", new Value (0.0));
	StylusPoint::YProperty = DependencyObject::Register (Type::STYLUSPOINT, "Y", new Value (0.0));

	Stroke::DrawingAttributesProperty = DependencyObject::Register (Type::STROKE, "DrawingAttributes", Type::DRAWINGATTRIBUTES);
	Stroke::StylusPointsProperty = DependencyObject::Register (Type::STROKE, "StylusPoints", Type::STYLUSPOINT_COLLECTION);

	DrawingAttributes::ColorProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "Color", new Value (Color (0xFF000000)));
	DrawingAttributes::OutlineColorProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "OutlineColor", new Value (Color (0x00000000)));
	DrawingAttributes::HeightProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "Height", new Value (3.0));
	DrawingAttributes::WidthProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "Width", new Value (3.0));

	InkPresenter::StrokesProperty = DependencyObject::Register (Type::INKPRESENTER, "Strokes", Type::STROKE_COLLECTION);
}

