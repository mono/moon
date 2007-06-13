/*
 * brush.cpp: Brushes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#define __STDC_CONSTANT_MACROS
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "brush.h"
#include "transform.h"

//
// Brush
//

DependencyProperty* Brush::OpacityProperty;
DependencyProperty* Brush::RelativeTransformProperty;
DependencyProperty* Brush::TransformProperty;

double
brush_get_opacity (Brush *brush)
{
	return brush->GetValue (Brush::OpacityProperty)->AsDouble();
}

void
brush_set_opacity (Brush *brush, double opacity)
{
	brush->SetValue (Brush::OpacityProperty, Value (opacity));
}

TransformGroup*
brush_get_relative_transform (Brush *brush)
{
	Value *value = brush->GetValue (Brush::RelativeTransformProperty);
	return value ? value->AsTransformGroup() : NULL;
}

void
brush_set_relative_transform (Brush *brush, TransformGroup* transform_group)
{
	brush->SetValue (Brush::RelativeTransformProperty, Value (transform_group));
}

TransformGroup*
brush_get_transform (Brush *brush)
{
	Value *value = brush->GetValue (Brush::TransformProperty);
	return value ? value->AsTransformGroup() : NULL;
}

void
brush_set_transform (Brush *brush, TransformGroup* transform_group)
{
	brush->SetValue (Brush::TransformProperty, Value (transform_group));
}

//
// SolidColorBrush
//

DependencyProperty* SolidColorBrush::ColorProperty;

void
SolidColorBrush::SetupBrush (cairo_t *target, UIElement *uielement)
{
	Color *color = solid_color_brush_get_color (this);

	double alpha = color->a;
	// apply UIElement opacity and Brush opacity on color's alpha
	if (uielement) {
		// this is recursive to parents
		while (uielement) {
			double uielement_opacity = uielement_get_opacity (uielement);
			if (uielement_opacity < 1.0)
				alpha *= uielement_opacity;

			// FIXME: we should be calling FrameworkElement::Parent
			uielement = uielement->parent;
		}
	}

	double brush_opacity = brush_get_opacity (this);
	if (brush_opacity < 1.0)
		alpha *= brush_opacity;
	
	cairo_set_source_rgba (target, color->r, color->g, color->b, alpha);
}

Color*
solid_color_brush_get_color (SolidColorBrush *solid_color_brush)
{
	return solid_color_brush->GetValue (SolidColorBrush::ColorProperty)->AsColor();
}

void
solid_color_brush_set_color (SolidColorBrush *solid_color_brush, Color *color)
{
	solid_color_brush->SetValue (SolidColorBrush::ColorProperty, Value (*color));
}

SolidColorBrush*
solid_color_brush_new ()
{
	return new SolidColorBrush ();
}

// match System.Windows.Media.Colors properties
typedef struct {
	const char *name;
	const unsigned int color;
} named_colors_t;

named_colors_t named_colors [] = {

	{ "black",		0xFF000000 },
	{ "blue",		0xFF0000FF },
	{ "brown",		0xFFA52A2A },
	{ "cyan",		0xFF00FFFF },
	{ "darkgray",		0xFFA9A9A9 },
	{ "gray",		0xFF808080 },
	{ "green",		0xFF008000 },
	{ "lightgray",		0xFFD3D3D3 },
	{ "magenta",		0xFFFF00FF },
	{ "orange",		0xFFFFA500 },
	{ "purple",		0xFF800080 },
	{ "red",		0xFFFF0000 },
	{ "transparent",	0x00FFFFFF },
	{ "white",		0xFFFFFFFF },
	{ "yellow",		0xFFFFFF00 },
	{ NULL, 0 }
};

/**
 * see: http://msdn2.microsoft.com/en-us/library/system.windows.media.solidcolorbrush.aspx
 *
 * If no color is found, Color.Transparent is returned.
 */
Color*
color_from_str (const char *name)
{
	if (!name)
		return new Color (0x00FFFFFF);

	if (name [0] == '#') {
		char a [3] = "FF";
		char r [3] = "FF";
		char g [3] = "FF";
		char b [3] = "FF";

		switch (strlen (name + 1)) {
		case 3:
			// rgb
			r [1] = '0'; r [0] = name [1];
			g [1] = '0'; g [0] = name [2];
			b [1] = '0'; b [0] = name [3];
			break;
		case 4:
			// argb
			a [1] = '0'; a [0] = name [1];
			r [1] = '0'; r [0] = name [2];
			g [1] = '0'; g [0] = name [3];
			b [1] = '0'; b [0] = name [4];
			break;
		case 6:
			// rrggbb
			r [0] = name [1]; r [1] = name [2];
			g [0] = name [3]; g [1] = name [4];
			b [0] = name [5]; b [1] = name [6];
			break;
		case 8:
			// aarrggbb
			a [0] = name [1]; a [1] = name [2];
			r [0] = name [3]; r [1] = name [4];
			g [0] = name [5]; g [1] = name [6];
			b [0] = name [7]; b [1] = name [8];
			break;			
		}

		return new Color (strtol (r, NULL, 16) / 255.0F, strtol (g, NULL, 16) / 255.0F,
				strtol (b, NULL, 16) / 255.0F, strtol (a, NULL, 16) / 255.0F);
	}

	if (name [0] == 's' && name [1] == 'c' && name [2] == '#') {
		/* TODO */
	}

	for (int i = 0; named_colors [i].name; i++) {
		if (!g_strcasecmp (named_colors [i].name, name)) {
			return new Color (named_colors [i].color);
		}
	}

	return new Color (0x00FFFFFF);
}

//
// GradientBrush
//

DependencyProperty* GradientBrush::ColorInterpolationModeProperty;
DependencyProperty* GradientBrush::GradientStopsProperty;
DependencyProperty* GradientBrush::MappingModeProperty;
DependencyProperty* GradientBrush::SpreadProperty;

ColorInterpolationMode
gradient_brush_get_color_interpolation_mode (GradientBrush *brush)
{
	return (ColorInterpolationMode) brush->GetValue (GradientBrush::ColorInterpolationModeProperty)->AsInt32();
}

void
gradient_brush_set_color_interpolation_mode (GradientBrush *brush, ColorInterpolationMode mode)
{
	brush->SetValue (GradientBrush::ColorInterpolationModeProperty, Value (mode));
}

GradientStopCollection*
gradient_brush_get_gradient_stops (GradientBrush *brush)
{
	Value *value = brush->GetValue (GradientBrush::GradientStopsProperty);
	return (GradientStopCollection*) (value ? value->AsGradientStopCollection() : NULL);
}

void
gradient_brush_set_gradient_stops (GradientBrush *brush, GradientStopCollection* collection)
{
	brush->SetValue (GradientBrush::GradientStopsProperty, Value (collection));
}

BrushMappingMode
gradient_brush_get_mapping_mode (GradientBrush *brush)
{
	return (BrushMappingMode) brush->GetValue (GradientBrush::MappingModeProperty)->AsInt32();
}

void
gradient_brush_set_mapping_mode (GradientBrush *brush, BrushMappingMode mode)
{
	brush->SetValue (GradientBrush::MappingModeProperty, Value (mode));
}

GradientSpreadMethod 
gradient_brush_get_spread (GradientBrush *brush)
{
	return (GradientSpreadMethod) brush->GetValue (GradientBrush::SpreadProperty)->AsInt32();
}

void
gradient_brush_set_spread (GradientBrush *brush, GradientSpreadMethod method)
{
	brush->SetValue (GradientBrush::SpreadProperty, Value (method));
}

GradientBrush::GradientBrush ()
{
	children = NULL;
	GradientStopCollection *c = new GradientStopCollection ();

	this->SetValue (GradientBrush::GradientStopsProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == children);
}

void
GradientBrush::OnPropertyChanged (DependencyProperty *prop)
{
	Brush::OnPropertyChanged (prop);

	if (prop == GradientStopsProperty){
		// The new value has already been set, so unref the old collection

		GradientStopCollection *newcol = GetValue (prop)->AsGradientStopCollection();

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

//
// RadialGradientBrush
//

DependencyProperty* RadialGradientBrush::CenterProperty;
DependencyProperty* RadialGradientBrush::GradientOriginProperty;
DependencyProperty* RadialGradientBrush::RadiusXProperty;
DependencyProperty* RadialGradientBrush::RadiusYProperty;

RadialGradientBrush*
radial_gradient_brush_new ()
{
	return new RadialGradientBrush ();
}

Point*
radial_gradient_brush_get_center (RadialGradientBrush *brush)
{
	Value *value = brush->GetValue (RadialGradientBrush::CenterProperty);
	return (value ? value->AsPoint() : NULL);
}

void
radial_gradient_brush_set_center (RadialGradientBrush *brush, Point *center)
{
	brush->SetValue (RadialGradientBrush::CenterProperty, Value (*center));
}

Point*
radial_gradient_brush_get_gradientorigin (RadialGradientBrush *brush)
{
	Value *value = brush->GetValue (RadialGradientBrush::GradientOriginProperty);
	return (value ? value->AsPoint() : NULL);
}

void
radial_gradient_brush_set_gradientorigin (RadialGradientBrush *brush, Point *origin)
{
	brush->SetValue (RadialGradientBrush::GradientOriginProperty, Value (*origin));
}

double
radial_gradient_brush_get_radius_x (RadialGradientBrush *brush)
{
	return brush->GetValue (RadialGradientBrush::RadiusXProperty)->AsDouble();
}

void
radial_gradient_brush_set_radius_x (RadialGradientBrush *brush, double radiusX)
{
	brush->SetValue (RadialGradientBrush::RadiusXProperty, Value (radiusX));
}

double
radial_gradient_brush_get_radius_y (RadialGradientBrush *brush)
{
	return brush->GetValue (RadialGradientBrush::RadiusYProperty)->AsDouble();
}

void
radial_gradient_brush_set_radius_y (RadialGradientBrush *brush, double radiusY)
{
	brush->SetValue (RadialGradientBrush::RadiusYProperty, Value (radiusY));
}

void
RadialGradientBrush::SetupBrush (cairo_t *cairo, UIElement *uielement)
{
}

//
// GradientStopCollection
//

void
GradientStopCollection::Add (void *data)
{
	Value *value = (Value*) data;
	GradientStop *gradient_stop = value->AsGradientStop ();
	Collection::Add (gradient_stop);
}

void GradientStopCollection::Remove (void *data)
{
	Value *value = (Value*) data;
	GradientStop *gradient_stop = value->AsGradientStop ();
	Collection::Remove (gradient_stop);
}

GradientStopCollection*
gradient_stop_collection_new ()
{
	return new GradientStopCollection ();
}

//
// GradientStop
//

DependencyProperty* GradientStop::ColorProperty;
DependencyProperty* GradientStop::OffsetProperty;

GradientStop*
gradient_stop_new ()
{
	return new GradientStop ();
}

Color*
gradient_stop_get_color (GradientStop *stop)
{
	return stop->GetValue (GradientStop::ColorProperty)->AsColor();
}

void
gradient_stop_set_color (GradientStop *stop, Color *color)
{
	stop->SetValue (GradientStop::ColorProperty, Value (*color));
}

double
gradient_stop_get_offset (GradientStop *stop)
{
	return stop->GetValue (GradientStop::OffsetProperty)->AsDouble();
}

void
gradient_stop_set_offset (GradientStop *stop, double offset)
{
	stop->SetValue (GradientStop::OffsetProperty, Value (offset));
}

//
//
//

void
brush_init ()
{
	/* Brush fields */
	Brush::OpacityProperty = DependencyObject::Register (Value::BRUSH, "Opacity", new Value (1.0));
	Brush::RelativeTransformProperty = DependencyObject::Register (Value::BRUSH, "RelativeTransform", new Value ());
	Brush::TransformProperty = DependencyObject::Register (Value::BRUSH, "Transform", new Value ());

	/* SolidColorBrush fields */
	SolidColorBrush::ColorProperty = DependencyObject::Register (Value::SOLIDCOLORBRUSH, "Color", new Value (Color (0x00FFFFFF)));

	/* GradientBrush fields */
	GradientBrush::ColorInterpolationModeProperty = DependencyObject::Register (Value::GRADIENTBRUSH, "ColorInterpolationMode",  new Value (0));
	GradientBrush::GradientStopsProperty = DependencyObject::Register (Value::RADIALGRADIENTBRUSH, "GradientStops", Value::GRADIENTSTOP_COLLECTION);
	GradientBrush::MappingModeProperty = DependencyObject::Register (Value::GRADIENTBRUSH, "MappingMode",  new Value (0));
	GradientBrush::SpreadProperty = DependencyObject::Register (Value::GRADIENTBRUSH, "Spread",  new Value (0));

	/* RadialGradientBrush fields */
	RadialGradientBrush::CenterProperty = DependencyObject::Register (Value::RADIALGRADIENTBRUSH, "Center", Value::POINT);
	RadialGradientBrush::GradientOriginProperty = DependencyObject::Register (Value::RADIALGRADIENTBRUSH, "GradientOrigin", Value::POINT);
	RadialGradientBrush::RadiusXProperty = DependencyObject::Register (Value::RADIALGRADIENTBRUSH, "RadiusX",  new Value (0.0));
	RadialGradientBrush::RadiusYProperty = DependencyObject::Register (Value::RADIALGRADIENTBRUSH, "RadiusY",  new Value (0.0));

	/* GradientStop */
	GradientStop::ColorProperty = DependencyObject::Register (Value::GRADIENTSTOP, "Color", new Value (Color (0x00FFFFFF)));
	GradientStop::OffsetProperty = DependencyObject::Register (Value::GRADIENTSTOP, "Offset", new Value (0.0));
}
