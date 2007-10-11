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
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <math.h>

#include "stylus.h"
#include "collection.h"
#include "color.h"


StylusInfo*
stylus_info_new ()
{
	return new StylusInfo ();
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


void
InkPresenter::RenderChildren (cairo_t *cr, int x, int y, int width, int height)
{
	// Canvas elements are supported inside the InkPresenter
	Panel::RenderChildren (cr, x, y, width, height);

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
		double thickness = 1.0;
		Stroke *stroke = (Stroke *) cn->obj;
		// set drawing attributes
		value = stroke->GetValue (Stroke::DrawingAttributesProperty);
		if (value) {
			DrawingAttributes *da = value->AsDrawingAttributes ();
			if (da) {
// FIXME: we need a brush that will respect width|height and both colors
				thickness = drawing_attributes_get_height (da);
				Color *color = drawing_attributes_get_color (da);
				cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
			}
		}
		// for each stylus point in stroke
		value = stroke->GetValue (Stroke::StylusPointsProperty);
		if (value) {
			StylusPointCollection *spc = value->AsStylusPointCollection ();
			if (spc) {
				Collection::Node *cnp = (Collection::Node *) spc->list->First ();
				if (!cnp)
					continue;

				StylusPoint *stylus_point = (StylusPoint*) cnp->obj;
				cairo_move_to (cr, stylus_point_get_x (stylus_point), stylus_point_get_y (stylus_point));
				cnp = (Collection::Node *) cnp->next;

				for ( ; cnp != NULL; cnp = (Collection::Node *) cnp->next) {
					StylusPoint *stylus_point = (StylusPoint*) cnp->obj;

					cairo_set_line_width (cr, thickness * (1.0 + stylus_point_get_pressure_factor (stylus_point)));
					cairo_line_to (cr, stylus_point_get_x (stylus_point), stylus_point_get_y (stylus_point));
				}

				cairo_stroke (cr);
			}
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

