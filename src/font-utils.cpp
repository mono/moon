/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * font-utils.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include "font-utils.h"

// Silverlight accepts negative values ]0,-475[ as bold and everything over 1023 as normal
#define FONT_LOWER_BOLD_LIMIT	-475
#define FONT_UPPER_BOLD_LIMIT	1024

namespace Moonlight {

bool
FontWeightIsBold (FontWeights weight)
{
	if (weight > FONT_LOWER_BOLD_LIMIT)
		return weight < 0 || (weight >= FontWeightsSemiBold && weight < FONT_UPPER_BOLD_LIMIT);
	
	return false;
}

static struct {
	const char *name;
	size_t len;
	int type;
	int value;
} style_hints[] = {
	// widths
	{ "Ultra-Condensed", 15, FontPropertyStretch,  FontStretchesUltraCondensed },
	{ "Extra-Condensed", 15, FontPropertyStretch,  FontStretchesExtraCondensed },
	{ "Semi-Condensed",  14, FontPropertyStretch,  FontStretchesSemiCondensed  },
	{ "UltraCondensed",  14, FontPropertyStretch,  FontStretchesUltraCondensed },
	{ "ExtraCondensed",  14, FontPropertyStretch,  FontStretchesExtraCondensed },
	{ "SemiCondensed",   13, FontPropertyStretch,  FontStretchesSemiCondensed  },
	{ "Condensed",        9, FontPropertyStretch,  FontStretchesCondensed      },
	{ "Cond",             4, FontPropertyStretch,  FontStretchesCondensed      },
	{ "Ultra-Expanded",  14, FontPropertyStretch,  FontStretchesUltraExpanded  },
	{ "Extra-Expanded",  14, FontPropertyStretch,  FontStretchesExtraExpanded  },
	{ "Semi-Expanded",   13, FontPropertyStretch,  FontStretchesSemiExpanded   },
	{ "UltraExpanded",   13, FontPropertyStretch,  FontStretchesUltraExpanded  },
	{ "ExtraExpanded",   13, FontPropertyStretch,  FontStretchesExtraExpanded  },
	{ "SemiExpanded",    12, FontPropertyStretch,  FontStretchesSemiExpanded   },
	{ "Expanded",         8, FontPropertyStretch,  FontStretchesExpanded       },
	
	// weights
	{ "Thin",             4, FontPropertyWeight,   FontWeightsThin             },
	{ "Ultra-Light",     11, FontPropertyWeight,   FontWeightsExtraLight       },
	{ "Extra-Light",     11, FontPropertyWeight,   FontWeightsExtraLight       },
	{ "UltraLight",      10, FontPropertyWeight,   FontWeightsExtraLight       },
	{ "ExtraLight",      10, FontPropertyWeight,   FontWeightsExtraLight       },
	{ "Light",            5, FontPropertyWeight,   FontWeightsLight            },
	{ "Book",             4, FontPropertyWeight,   FontWeightsNormal           },
	{ "Medium",           6, FontPropertyWeight,   FontWeightsMedium           },
	{ "Demi-Bold",        9, FontPropertyWeight,   FontWeightsSemiBold         },
	{ "Semi-Bold",        9, FontPropertyWeight,   FontWeightsSemiBold         },
	{ "DemiBold",         8, FontPropertyWeight,   FontWeightsSemiBold         },
	{ "SemiBold",         8, FontPropertyWeight,   FontWeightsSemiBold         },
	{ "Bold",             4, FontPropertyWeight,   FontWeightsBold             },
	{ "Extra-Bold",      10, FontPropertyWeight,   FontWeightsExtraBold        },
	{ "Ultra-Bold",      10, FontPropertyWeight,   FontWeightsExtraBold        },
	{ "ExtraBold",        9, FontPropertyWeight,   FontWeightsExtraBold        },
	{ "UltraBold",        9, FontPropertyWeight,   FontWeightsExtraBold        },
	{ "Black",            5, FontPropertyWeight,   FontWeightsBlack            },
	{ "Heavy",            5, FontPropertyWeight,   FontWeightsBlack            },
	{ "Extra-Black",     11, FontPropertyWeight,   FontWeightsExtraBlack       },
	{ "Ultra-Black",     11, FontPropertyWeight,   FontWeightsExtraBlack       },
	{ "ExtraBlack",      10, FontPropertyWeight,   FontWeightsExtraBlack       },
	{ "UltraBlack",      10, FontPropertyWeight,   FontWeightsExtraBlack       },
	
	// slants
	{ "Oblique",          7, FontPropertyStyle,    FontStylesOblique           },
	{ "Italic",           6, FontPropertyStyle,    FontStylesItalic            },
	{ "Kursiv",           6, FontPropertyStyle,    FontStylesItalic            },
	
	// changes nothing
	{ "Regular",          7, 0,                    0                           },
	{ "W3",               2, 0,                    0                           },  // as in Hiragino Mincho Pro W3
};

void
font_style_info_parse (FontStyleInfo *info, const char *style, bool family)
{
	register const char *inptr = style;
	const char *first_hint = NULL;
	const char *token;
	guint tokens = 0;
	size_t len;
	guint i;
	
	if (!style)
		return;
	
	while (*inptr) {
		while (*inptr && isspace ((int) ((unsigned char) *inptr)))
			inptr++;
		
		if (*inptr == '\0')
			break;
		
		token = inptr;
		while (*inptr && !isspace ((int) ((unsigned char) *inptr)))
			inptr++;
		
		tokens++;
		
		if (family && tokens == 1) {
			// if parsing the family_name, first token must not be interpreted as a style hint
			continue;
		}
		
		len = (size_t) (inptr - token);
		for (i = 0; i < G_N_ELEMENTS (style_hints); i++) {
			if (style_hints[i].len == len && !strncmp (style_hints[i].name, token, len)) {
				switch (style_hints[i].type) {
				case FontPropertyStretch:
					info->stretch = (FontStretches) style_hints[i].value;
					info->set |= FontPropertyStretch;
					break;
				case FontPropertyWeight:
					info->weight = (FontWeights) style_hints[i].value;
					info->set |= FontPropertyWeight;
					break;
				case FontPropertyStyle:
					info->style = (FontStyles) style_hints[i].value;
					info->set |= FontPropertyStyle;
					break;
				}
				
				if (!first_hint)
					first_hint = token;
				break;
			}
		}
		
		if (family && i == G_N_ELEMENTS (style_hints)) {
			// if we come across an unknown style hint when
			// parsing the family_name, assume that any previously
			// found style hints were not actually style hints,
			// but instead just part of the family name.
			info->stretch = FontStretchesNormal;
			info->weight = FontWeightsNormal;
			info->style = FontStylesNormal;
			info->set = 0;
			
			first_hint = NULL;
		}
	}
	
	if (family) {
		if (first_hint)
			info->family_name = g_strndup (style, first_hint - style);
		else
			info->family_name = g_strdup (style);
		
		g_strstrip (info->family_name);
	}
}

const char *
font_style_info_to_string (FontStretches stretch, FontWeights weight, FontStyles style)
{
	static char namebuf[256];
	guint i = 0;
	char *p;
	
	namebuf[0] = '\0';
	p = namebuf;
	
	if (stretch != FontStretchesNormal) {
		while (style_hints[i].type == FontPropertyStretch) {
			if (style_hints[i].value == stretch) {
#if PLUMB_ME
				p = g_stpcpy (p, style_hints[i].name);
#endif
				break;
			}
			
			i++;
		}
	}
	
	if (weight != FontWeightsNormal) {
		while (style_hints[i].type != FontPropertyWeight)
			i++;
		
		while (style_hints[i].type == FontPropertyWeight) {
			if (style_hints[i].value == weight) {
				if (p != namebuf)
					*p++ = ' ';
				
#if PLUMB_ME
				p = g_stpcpy (p, style_hints[i].name);
#endif
				break;
			}
			
			i++;
		}
	}
	
	if (style != FontStylesNormal) {
		while (style_hints[i].type != FontPropertyStyle)
			i++;
		
		while (i < G_N_ELEMENTS (style_hints)) {
			if (style_hints[i].value == style) {
				if (p != namebuf)
					*p++ = ' ';

#if PLUMB_ME
				p = g_stpcpy (p, style_hints[i].name);
#endif
				break;
			}
			
			i++;
		}
	}
	
	return namebuf;
}

void
font_style_info_init (FontStyleInfo *info, const char *family)
{
	info->family_name = family ? g_strdup (family) : NULL;
	info->stretch = FontStretchesNormal;
	info->weight = FontWeightsNormal;
	info->style = FontStylesNormal;
	info->set = 0;
}

void
font_style_info_hydrate (FontStyleInfo *info, const char *family, FontStretches stretch, FontWeights weight, FontStyles style)
{
	font_style_info_init (info, NULL);
	
	// extract whatever little style info we can from the family name
	font_style_info_parse (info, family, true);
	
	// override style with user-specified attributes
	if (!(info->set & FontPropertyStretch))
		info->stretch = stretch;
	if (!(info->set & FontPropertyWeight))
		info->weight = weight;
	if (!(info->set & FontPropertyStyle))
		info->style = style;
}

static int
normalize_weight (int weight)
{
	if (weight <= FONT_LOWER_BOLD_LIMIT || weight >= FONT_UPPER_BOLD_LIMIT)
		return (int) FontWeightsNormal;
	
	return weight;
}

int
font_style_info_diff (const FontStyleInfo *actual, const FontStyleInfo *desired)
{
#if 0
	int weight = abs (normalize_weight (actual->weight) - normalize_weight (desired->weight));
	
	if (actual->style == desired->style)
		return weight;
	
	if (actual->style == FontStylesNormal) {
		// we can emulate italic/oblique, but we would still prefer the real
		// italic font if we can find it so apply a slight penalty
		return 1000 + weight;
	}
	
	// ouch, apply a huge penalty
	return 1000000 + weight;
#else
	int weight = abs (normalize_weight (actual->weight) - normalize_weight (desired->weight));
	int width = abs (actual->stretch - desired->stretch);
	int slant = abs (actual->style - desired->style);
	
	// weight needs ~12 bits, width needs ~4 bits, and slant needs ~2 bits
	
	// width has the highest priority, followed by weight and then slant
	return ((width & 0x000f) << 14) | ((weight & 0x0fff) << 2) | (slant & 0x0003);
#endif
}

};
