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

#if TRUE
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
#else
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
	cairo_set_line_width (cr, 2.0);

	moon_path *path = moon_path_new (10);

	// for each stroke in collection
	Collection::Node *cn = (Collection::Node *) strokes->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
		double thickness = 1.0;
		Color *color = NULL;
		Color *outline_color = NULL;

		float rx = 2.0;
		float ry = 2.0;

		Stroke *stroke = (Stroke *) cn->obj;
		// set drawing attributes
		value = stroke->GetValue (Stroke::DrawingAttributesProperty);
		if (value) {
			DrawingAttributes *da = value->AsDrawingAttributes ();
			if (da) {
// FIXME: we need a brush that will respect width|height and both colors
				rx += drawing_attributes_get_width (da);
				ry += drawing_attributes_get_height (da);
				color = drawing_attributes_get_color (da);
				outline_color = drawing_attributes_get_outline_color (da);
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

				double c1x = stylus_point_get_x (stylus_point);
				double c1y = stylus_point_get_y (stylus_point);

				// ellipse are drawn by specifying the upper-left corner, stylus points are at center
				double hrx = (rx / 2.0);
				double hry = (ry / 2.0);
				double ex = c1x - hrx;
				double ey = c1y - hry;

				if (!cnp->next) {
					// this is a single point stroke, replace by ellipse
					moon_ellipse (path, ex, ey, rx, ry);
				} else {
					moon_ellipse (path, ex, ey, rx, ry);

					for (cnp = (Collection::Node *) cnp->next ; cnp != NULL; cnp = (Collection::Node *) cnp->next) {
						StylusPoint *stylus_point = (StylusPoint*) cnp->obj;

						double c2x = stylus_point_get_x (stylus_point);
						double c2y = stylus_point_get_y (stylus_point);

						double x1, y1, x2, y2;
						if (c2x - c1x == 0) {
							x1 = c1x + hrx;
							y1 = c1y;
							x2 = c2x + hrx;							
							y2 = c2y;
						} else if (c2y - c1y == 0) {
							x1 = c1x;
							y1 = c1y + hry;
							x2 = c2x;							
							y2 = c2y + hry;
						} else {
							double m = (c2y - c1y) / (c2x - c1x);
							double angle = atan (1.0 / m);
							x1 = c1x + hrx * cos (angle);
							y1 = c1y - hry * sin (angle);
							x2 = c2x + hrx * cos (angle);
							y2 = c2y - hry * sin (angle);
						}
						moon_move_to (path, x1, y1);
						moon_line_to (path, x2, y2);

//						cairo_set_line_width (cr, ry * (1.0 + stylus_point_get_pressure_factor (stylus_point)));
						ex = c2x - hrx;
						ey = c2y - hry;
						moon_ellipse (path, ex, ey, rx, ry);
					}

					cnp = (Collection::Node *) spc->list->Last ();
					for ( ; cnp != NULL; cnp = (Collection::Node *) cnp->prev) {
						StylusPoint *stylus_point = (StylusPoint*) cnp->obj;

						double c2x = stylus_point_get_x (stylus_point);
						double c2y = stylus_point_get_y (stylus_point);

						double x1, y1, x2, y2;
						if (c2x - c1x == 0) {
							x1 = c1x - hrx;
							y1 = c1y;
							x2 = c2x - hrx;							
							y2 = c2y;
						} else if (c2y - c1y == 0) {
							x1 = c1x;
							y1 = c1y - hry;
							x2 = c2x;							
							y2 = c2y - hry;
						} else {
							double m = (c2y - c1y) / (c2x - c1x);
							double angle = atan (1.0 / m);
							x1 = c1x - hrx * cos (angle);
							y1 = c1y + hry * sin (angle);
							x2 = c2x - hrx * cos (angle);
							y2 = c2y + hry * sin (angle);
						}
						moon_move_to (path, x1, y1);
						moon_line_to (path, x2, y2);

//						cairo_set_line_width (cr, ry * (1.0 + stylus_point_get_pressure_factor (stylus_point)));
						ex = c2x - hrx;
						ey = c2y - hry;
						moon_ellipse (path, ex, ey, rx, ry);
					}
				}
			}

			cairo_new_path (cr);
			cairo_append_path (cr, &path->cairo);

			if (color) {
				cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
				cairo_fill_preserve (cr);
			}
			if (outline_color) {
				cairo_set_source_rgba (cr, outline_color->r, outline_color->g, outline_color->b, outline_color->a);
				cairo_stroke (cr);
			}
			moon_path_clear (path);
		}
	}
	moon_path_destroy (path);
}
#endif

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

