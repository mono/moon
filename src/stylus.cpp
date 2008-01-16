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

#include "stylus.h"
#include "collection.h"
#include "color.h"
#include "moon-path.h"

StylusInfo*
stylus_info_new ()
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
stylus_point_new ()
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

	int count = collection_count (stylusPointCollection);
	for (int i=0; i < count; i++) {
		collection_add (this, collection_get_value_at (stylusPointCollection, i));
	}
	return collection_count (this) - 1;
}


Stroke::Stroke ()
{
	this->SetValue (Stroke::StylusPointsProperty, Value::CreateUnref (new StylusPointCollection ()));
	this->SetValue (Stroke::DrawingAttributesProperty, Value::CreateUnref (new DrawingAttributes ()));
}

Rect
Stroke::GetBounds ()
{
	DrawingAttributes *da = stroke_get_drawing_attributes (this);
	StylusPointCollection *spc = stroke_get_stylus_points (this);

	if (da)
		return da->ComputeBounds (spc);
	else
		return DrawingAttributes::ComputeBoundsWithoutDrawingAttributes (spc);
}

bool
Stroke::HitTest (StylusPointCollection *stylusPoints)
{
	Collection::Node *n;

	Rect bounds = GetBounds();

	for (n = (Collection::Node *) stylusPoints->list->First (); n; n = (Collection::Node *) n->next) {
		StylusPoint *sp = (StylusPoint*)n->obj;
	}

	return false;
}

Stroke*
stroke_new ()
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

	if (stylusPoints->list->Length () == 0)
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
drawing_attributes_new ()
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
	drawing_attributes_quick_render (cr, height + 2.0, outline, collection);
	drawing_attributes_quick_render (cr, height - 2.0, color, collection);
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

Rect
DrawingAttributes::ComputeBounds (StylusPointCollection *stylusPointCollection)
{
	// XXX
	return Rect (0,0,0,0);
}

Rect
DrawingAttributes::ComputeBoundsWithoutDrawingAttributes (StylusPointCollection *stylusPointCollection)
{
	// XXX
	return Rect (0,0,0,0);
}

InkPresenter::InkPresenter ()
{
	this->SetValue (InkPresenter::StrokesProperty, Value::CreateUnref (new StrokeCollection ()));
}

void
InkPresenter::RenderChildren (cairo_t *cr, Region *region)
{
	// Canvas elements are supported inside the InkPresenter
	Panel::RenderChildren (cr, region);

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
}

bool
InkPresenter::OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child)
{
	if (prop == StrokesProperty)
		return true;

	return Canvas::OnChildPropertyChanged (prop, child);
}

InkPresenter*
ink_presenter_new ()
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

