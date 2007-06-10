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
#include "runtime.h"
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
}
