/*
 * enums.c: various enumerated types + enum -> str helpers
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>
#include <glib.h>
#include "enums.h"

static GHashTable *enum_map = NULL;

typedef struct {
	const char *name;
	int value;
} enum_map_t;

static enum_map_t alignment_x_map [] = {
	{ "Left", 0 },
	{ "Center", 1 },
	{ "Right", 2 },
	{ NULL, 0 },
};

static enum_map_t alignment_y_map [] = {
	{ "Top", 0 },
	{ "Center", 1 },
	{ "Bottom", 2 },
	{ NULL, 0 },
};

static enum_map_t brush_mapping_mode_map [] = {
	{ "Absolute", 0 },
	{ "RelativeToBoundingBox", 1},
	{ NULL, 0 },
};

static enum_map_t color_interpolation_mode_map [] = {
	{ "ScRgbLinearInterpolation", 0 },
	{ "SRgbLinearInterpolation", 1 },
	{ NULL, 0 },
};

static enum_map_t cursors_map [] = {
	{ "Default", 0 },
	{ "Arrow", 1 },
	{ "Hand", 2 },
	{ "Wait", 3 },
	{ "IBeam", 4 },
	{ "Stylus", 5 },
	{ "Eraser", 6 },
	{ "None", 7 },
	{ NULL, 0 },
};

static enum_map_t error_type_map [] = {
	{ "NoError", 0 },
	{ "UnknownError", 1 },
	{ "InitializeError", 2 },
	{ "ParserError", 3 },
	{ "ObjectModelError", 4 },
	{ "RuntimeError", 5 },
	{ "DownloadError", 6 },
	{ "MediaError", 7 },
	{ "ImageError", 8 },
	{ NULL, 0 },
};

static enum_map_t fill_behavior_map [] = {
	{ "HoldEnd", 0 },
	{ "Stop", 1 },
	{ NULL, 0 },
};

static enum_map_t fill_rule_map [] = {
	{ "EvenOdd", 0 },
	{ "Nonzero", 1},
	{ NULL, 0 },
};

static enum_map_t font_stretches_map [] = {
	{ "UltraCondensed", 1 },
	{ "ExtraCondensed", 2 },
	{ "Condensed",      3 },
	{ "SemiCondensed",  4 },
	{ "Normal",         5 },
	{ "Medium",         5 },
	{ "SemiExpanded",   6 },
	{ "Expanded",       7 },
	{ "ExtraExpanded",  8 },
	{ "UltraExpanded",  9 },
	{ NULL, 0 },
};

static enum_map_t font_styles_map [] = {
	{ "Normal",  0 },
	{ "Oblique", 1 },
	{ "Italic",  2 },
	{ NULL, 0 },
};

static enum_map_t font_weights_map [] = {
	{ "Thin",       100 },
	{ "ExtraLight", 200 },
	{ "UltraLight", 200 },  // deprecated as of July 2007 
	{ "Light",      300 },
	{ "Normal",     400 },
	{ "Regular",    400 },  // deprecated as of July 2007 
	{ "Medium",     500 },
	{ "SemiBold",   600 },
	{ "DemiBold",   600 },  // deprecated as of July 2007 
	{ "Bold",       700 },
	{ "ExtraBold",  800 },
	{ "UltraBold",  800 },  // deprecated as of July 2007 
 	{ "Black",      900 },
	{ "Heavy",      900 },  // deprecated as of July 2007 
	{ "ExtraBlack", 950 },
	{ "UltraBlack", 950 },  // deprecated as of July 2007 
	{ NULL, 0 },
};

static enum_map_t style_simulations_map [] = {
	{ "BoldItalicSimulation", 0 },
	{ "BoldSimulation",       1 },
	{ "ItalicSimulation",     2 },
	{ "None",                 3 },
	{ NULL,                   0 },
};

static enum_map_t gradient_spread_method_map [] = {
	{ "Pad", 0 },
	{ "Reflect", 1 },
	{ "Repeat", 2 },
	{ NULL, 0 },
};

static enum_map_t pen_line_cap_map [] = {
	{ "Flat", 0 },
	{ "Square", 1 },
	{ "Round", 2 },
	{ "Triangle", 3 },
	{ NULL, 0 },
};

static enum_map_t pen_line_join_map [] = {
	{ "Miter", 0 },
	{ "Bevel", 1 },
	{ "Round", 2 },
	{ NULL, 0 },
};

static enum_map_t stretch_map [] = {
	{ "None", 0 },
	{ "Fill", 1 },
	{ "Uniform", 2 },
	{ "UniformToFill", 3 },
	{ NULL, 0 },
};

static enum_map_t sweep_direction_map [] = {
	{ "Counterclockwise", 0 },
	{ "Clockwise", 1 },
	{ NULL, 0 },
};

static enum_map_t tablet_device_type_map [] = {
	{ "Mouse", 0 },
	{ "Stylus", 1 },
	{ "Touch", 2 },
	{ NULL, 0 },
};

static enum_map_t text_decorations_map [] = {
	{ "None", 0 },
	{ "Underline", 1 },
	{ NULL, 0 },
};

static enum_map_t text_wrapping_map [] = {
	{ "Wrap", 0 },
	{ "NoWrap", 1 },
	{ "WrapWithOverflow", 2 },
	{ NULL, 0 },
};

static enum_map_t visibility_map [] = {
	{ "Visible", 0 },
	{ "Collapsed", 1 },
	{ NULL, 0 },
};

static void
initialize_enums (void)
{
	enum_map = g_hash_table_new (g_str_hash, g_str_equal);
	
	g_hash_table_insert (enum_map, (char *) "AlignmentX", alignment_x_map);
	g_hash_table_insert (enum_map, (char *) "AlignmentY", alignment_y_map);
	g_hash_table_insert (enum_map, (char *) "MappingMode", brush_mapping_mode_map);
	g_hash_table_insert (enum_map, (char *) "ColorInterpolationMode", color_interpolation_mode_map);
	g_hash_table_insert (enum_map, (char *) "Cursor", cursors_map);
	g_hash_table_insert (enum_map, (char *) "ErrorType", error_type_map);
	g_hash_table_insert (enum_map, (char *) "FillBehavior", fill_behavior_map);
	g_hash_table_insert (enum_map, (char *) "FillRule", fill_rule_map);
	g_hash_table_insert (enum_map, (char *) "FontStretch", font_stretches_map);
	g_hash_table_insert (enum_map, (char *) "FontStyle", font_styles_map);
	g_hash_table_insert (enum_map, (char *) "FontWeight", font_weights_map);
	g_hash_table_insert (enum_map, (char *) "SpreadMethod", gradient_spread_method_map);
	
	g_hash_table_insert (enum_map, (char *) "StrokeDashCap", pen_line_cap_map);
	g_hash_table_insert (enum_map, (char *) "StrokeStartLineCap", pen_line_cap_map);
	g_hash_table_insert (enum_map, (char *) "StrokeEndLineCap", pen_line_cap_map);
	
	g_hash_table_insert (enum_map, (char *) "StrokeLineJoin", pen_line_join_map);
	g_hash_table_insert (enum_map, (char *) "Stretch", stretch_map);
	g_hash_table_insert (enum_map, (char *) "StyleSimulations", style_simulations_map);
	g_hash_table_insert (enum_map, (char *) "SweepDirection", sweep_direction_map);
	g_hash_table_insert (enum_map, (char *) "DeviceType", tablet_device_type_map);
	g_hash_table_insert (enum_map, (char *) "TextDecorations", text_decorations_map);
	g_hash_table_insert (enum_map, (char *) "TextWrapping", text_wrapping_map);
	g_hash_table_insert (enum_map, (char *) "Visibility", visibility_map);
}

static int
enum_from_str (const enum_map_t *emu, const char *str)
{
	int i;
	for (i = 0; emu [i].name; i++) {
		if (!g_strcasecmp (emu [i].name, str))
			return emu [i].value;
	}

	return -1;
}

static const char*
str_from_enum (const enum_map_t *emu, int e)
{
	int i;
	for (i = 0; emu [i].name; i++) {
		if (emu [i].value == e)
			return emu [i].name;
	}

	return NULL;
}

//
// Converts a string representing an enum (ie. "FillEnd") to int.
// Returns -1 if no property or enum found.
//
int 
enums_str_to_int (const char *prop_name, const char *str)
{
	if (enum_map == NULL)
		initialize_enums ();

	enum_map_t *emu = (enum_map_t *) g_hash_table_lookup (enum_map, prop_name);
	if (! emu)
		return -1;

	return enum_from_str (emu, str);
}

//
// Converts an int enum to a string representation.
// Returns NULL if no match found.
//
const char *
enums_int_to_str (const char *prop_name, int e)
{
	if (enum_map == NULL)
		initialize_enums ();

	enum_map_t *emu = (enum_map_t *) g_hash_table_lookup (enum_map, prop_name);
	if (! emu)
		return NULL;

	return str_from_enum (emu, e);
}

