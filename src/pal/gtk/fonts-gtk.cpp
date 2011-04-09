/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fonts-gtk.cpp: different types of collections
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H
#include FT_SFNT_NAMES_H
#include FT_OUTLINE_H
#include FT_SYSTEM_H
#include FT_GLYPH_H

#include "pal-gtk.h"

namespace Moonlight {

//
// Silverlight -> FontConfig enumeration conversion utilities
//

#ifndef FC_WEIGHT_EXTRABLACK
#define FC_WEIGHT_EXTRABLACK 215
#endif
#ifndef FC_WEIGHT_ULTRABLACK
#define FC_WEIGHT_ULTRABLACK FC_WEIGHT_EXTRABLACK
#endif

// Silverlight accepts negative values ]0,-475[ as bold and everything over 1023 as normal
#define FONT_LOWER_BOLD_LIMIT	-475
#define FONT_UPPER_BOLD_LIMIT	1024


MoonFontServiceGtk::MoonFontServiceGtk ()
{
	FcPattern *pattern;
	
	pattern = FcPatternBuild (NULL, FC_FAMILY, FcTypeString, "Sans",
				  FC_SIZE, FcTypeDouble, 10.0, NULL);
	
	if (FcPatternGetDouble (pattern, FC_DPI, 0, &dpi) != FcResultMatch)
		dpi = 72.0;
	
	FcPatternDestroy (pattern);
}

MoonFontServiceGtk::~MoonFontServiceGtk ()
{
	// nothing to do...
}

static int
fc_weight (FontWeights weight)
{
	if ((weight < 0) && (weight > FONT_LOWER_BOLD_LIMIT))
		return FC_WEIGHT_BLACK;
	else if (weight < (FontWeightsThin + FontWeightsLight) / 2)
		return FC_WEIGHT_ULTRALIGHT;
	else if (weight < (FontWeightsLight + FontWeightsNormal) / 2)
		return FC_WEIGHT_LIGHT;
	else if (weight < (FontWeightsNormal + FontWeightsMedium) / 2)
		return FC_WEIGHT_NORMAL;
	else if (weight < (FontWeightsMedium + FontWeightsSemiBold) / 2)
		return FC_WEIGHT_MEDIUM;
	else if (weight < (FontWeightsSemiBold + FontWeightsBold) / 2)
		return FC_WEIGHT_SEMIBOLD;
	else if (weight < (FontWeightsBold + FontWeightsExtraBold) / 2)
		return FC_WEIGHT_BOLD;
	else if (weight < (FontWeightsExtraBold + FontWeightsBlack) / 2)
		return FC_WEIGHT_ULTRABOLD;
	else if (weight < FONT_UPPER_BOLD_LIMIT)
		return FC_WEIGHT_BLACK;
	else
		return FC_WEIGHT_NORMAL;
}

static int
fc_width (FontStretches stretch)
{
	switch (stretch) {
	case FontStretchesUltraCondensed:
		return FC_WIDTH_ULTRACONDENSED;
	case FontStretchesExtraCondensed:
		return FC_WIDTH_EXTRACONDENSED;
	case FontStretchesCondensed:
		return FC_WIDTH_CONDENSED;
	case FontStretchesSemiCondensed:
		return FC_WIDTH_SEMICONDENSED;
	case FontStretchesNormal:
		return FC_WIDTH_NORMAL;
#if 0
	case FontStretchesMedium:
		return FC_WIDTH_NORMAL;
#endif
	case FontStretchesSemiExpanded:
		return FC_WIDTH_SEMIEXPANDED;
	case FontStretchesExpanded:
		return FC_WIDTH_EXPANDED;
	case FontStretchesExtraExpanded:
		return FC_WIDTH_EXTRAEXPANDED;
	case FontStretchesUltraExpanded:
		return FC_WIDTH_ULTRAEXPANDED;
	default:
		return FC_WIDTH_NORMAL;
	}
}

static int
fc_slant (FontStyles style)
{
	switch (style) {
	case FontStylesNormal:
		return FC_SLANT_ROMAN;
	// technically Olbique does not exists in SL 1.0 or 2.0 (it's in WPF) but the parser allows it
	case FontStylesOblique:
		return FC_SLANT_OBLIQUE;
	case FontStylesItalic:
	// Silverlight defaults bad values to Italic
	default:
		return FC_SLANT_ITALIC;
	}
}

MoonFont *
MoonFontServiceGtk::FindFont (const FontStyleInfo *desired)
{
	FcPattern *pattern, *matched;
	FcChar8 *filename;
	FcResult result;
	MoonFont *font;
	int index;
	
	pattern = FcPatternCreate ();
	FcPatternAddDouble (pattern, FC_DPI, dpi);
	FcPatternAddString (pattern, FC_FAMILY, (const FcChar8 *) desired->family_name);
	FcPatternAddInteger (pattern, FC_WIDTH, fc_width (desired->stretch));
	FcPatternAddInteger (pattern, FC_WEIGHT, fc_weight (desired->weight));
	FcPatternAddInteger (pattern, FC_SLANT, fc_slant (desired->style));
	FcDefaultSubstitute (pattern);
	
	if (!(matched = FcFontMatch (NULL, pattern, &result))) {
		FcPatternDestroy (pattern);
		return NULL;
	}
	
	FcPatternDestroy (pattern);
	
	if (FcPatternGetString (matched, FC_FILE, 0, &filename) != FcResultMatch) {
		FcPatternDestroy (matched);
		return NULL;
	}
	
	if (FcPatternGetInteger (matched, FC_INDEX, 0, &index) != FcResultMatch) {
		FcPatternDestroy (matched);
		return NULL;
	}
	
	font = new MoonFont ((const char *) filename, index);
	FcPatternDestroy (matched);
	
	return font;
}

void
MoonFontServiceGtk::ForeachFont (MoonForeachFontCallback foreach, gpointer user_data)
{
	FcObjectSet *objects;
	FcPattern *pattern;
	FcFontSet *fonts;
	const char *path;
	int index;
	
	objects = FcObjectSetBuild (FC_FILE, FC_INDEX, NULL);
	pattern = FcPatternCreate ();
	
	fonts = FcFontList (NULL, pattern, objects);
	FcObjectSetDestroy (objects);
	FcPatternDestroy (pattern);
	
	for (int i = 0; i < fonts->nfont; i++) {
		if (FcPatternGetString (fonts->fonts[i], FC_FILE, 0, (FcChar8 **) &path) != FcResultMatch)
			continue;
		
		if (FcPatternGetInteger (fonts->fonts[i], FC_INDEX, 0, &index) != FcResultMatch)
			continue;
		
		if (!foreach (path, index, user_data))
			break;
	}
	
	FcFontSetDestroy (fonts);
}

guint32
MoonFontServiceGtk::GetCharIndex (FT_Face face, gunichar unichar)
{
	return FcFreeTypeCharIndex (face, unichar);
}

};
