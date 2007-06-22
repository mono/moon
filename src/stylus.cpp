/*
 * stylus.cpp
 *
 * Author:
 *   Rolf Bjarne Kvinge  (RKvinge@novell.com)
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

Stroke*
stroke_new ()
{
	return new Stroke ();
}

DrawingAttributes*
drawing_attributes_new ()
{
	return new DrawingAttributes ();
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
	StylusInfo::DeviceTypeProperty = DependencyObject::Register (Type::STYLUSINFO, "DeviceType", Type::INT32);
	StylusInfo::IsInvertedProperty = DependencyObject::Register (Type::STYLUSINFO, "IsInverted", Type::BOOL);

	StylusPoint::PressureFactorProperty = DependencyObject::Register (Type::STYLUSPOINT, "PressureFactor", Type::DOUBLE);
	StylusPoint::XProperty = DependencyObject::Register (Type::STYLUSPOINT, "X", Type::DOUBLE);
	StylusPoint::YProperty = DependencyObject::Register (Type::STYLUSPOINT, "Y", Type::DOUBLE);

	Stroke::DrawingAttributesProperty = DependencyObject::Register (Type::STROKE, "DrawingAttributes", Type::DRAWINGATTRIBUTES);
	Stroke::StylusPointsProperty = DependencyObject::Register (Type::STROKE, "StylusPoints", Type::STYLUSPOINT_COLLECTION);

	DrawingAttributes::ColorProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "Color", Type::COLOR);
	DrawingAttributes::OutlineColorProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "OutlineColor", Type::COLOR);
	DrawingAttributes::HeightProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "Height", Type::DOUBLE);
	DrawingAttributes::WidthProperty = DependencyObject::Register (Type::DRAWINGATTRIBUTES, "Width", Type::DOUBLE);

	InkPresenter::StrokesProperty = DependencyObject::Register (Type::INKPRESENTER, "Strokes", Type::STROKE_COLLECTION);
}

