/*
 * font.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <string.h>
#include <sys/types.h>

#include "moon-path.h"
#include "font.h"

#include FT_OUTLINE_H


static GHashTable *font_cache = NULL;
static bool initialized = false;
static FT_Library libft2;
static double dpi;


void
font_init (void)
{
	FcPattern *pattern;
	
	if (initialized)
		return;
	
	if (FT_Init_FreeType (&libft2) != 0) {
		g_warning ("could not init libfreetype2");
		return;
	}
	
	font_cache = g_hash_table_new ((GHashFunc) FcPatternHash, (GEqualFunc) FcPatternEqual);
	
	pattern = FcPatternBuild (NULL, FC_FAMILY, FcTypeString, "Sans",
				  FC_SIZE, FcTypeDouble, 10.0, NULL);
	
	if (FcPatternGetDouble (pattern, FC_DPI, 0, &dpi) != FcResultMatch)
		dpi = 72.0;
	
	FcPatternDestroy (pattern);
	
	initialized = true;
}

static int
fc_weight (FontWeights weight)
{
	if (weight < (FontWeightsThin + FontWeightsLight) / 2)
		return FC_WEIGHT_ULTRALIGHT;
	else if (weight < (FontWeightsLight + FontWeightsNormal) / 2)
		return FC_WEIGHT_LIGHT;
	else if (weight < (FontWeightsNormal + FontWeightsMedium) / 2)
		return FC_WEIGHT_NORMAL;
	else if (weight < (FontWeightsMedium + FontWeightsSemiBold) / 2)
		return FC_WEIGHT_MEDIUM;
	else if (weight < (FontWeightsSemiBold + FontWeightsBold) / 2)
		return FC_WEIGHT_DEMIBOLD;
	else if (weight < (FontWeightsBold + FontWeightsExtraBold) / 2)
		return FC_WEIGHT_BOLD;
	else if (weight < (FontWeightsExtraBold + FontWeightsBlack) / 2)
		return FC_WEIGHT_ULTRABOLD;
	else
		return FC_WEIGHT_BLACK;
}

static int
fc_style (FontStyles style)
{
	switch (style) {
	case FontStylesNormal:
		return FC_SLANT_ROMAN;
	case FontStylesOblique:
		return FC_SLANT_OBLIQUE;
	case FontStylesItalic:
		return FC_SLANT_ITALIC;
	default:
		return FC_SLANT_ROMAN;
	}
}

static int
fc_stretch (FontStretches stretch)
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


struct GlyphMetrics {
	double horiBearingX;
	double horiBearingY;
	double horiAdvance;
	double vertBearingX;
	double vertBearingY;
	double vertAdvance;
	double height;
	double width;
};

struct GlyphInfo {
	uint32_t unichar;
	uint32_t index;
	moon_path *path;
	GlyphMetrics metrics;
	cairo_surface_t *surface;
	FT_Bitmap bitmap;
	int bitmap_left;
	int bitmap_top;
};

static const FT_Matrix invert_y = {
        65535, 0,
        0, -65535,
};


#define LOAD_FLAGS (FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL)

TextFont::TextFont (FcPattern *pattern)
{
	FcChar8 *filename = NULL;
	FcPattern *matched, *sans;
	bool retried = false;
	FcResult result;
	double size;
	int id;
	
	FcPatternGetDouble (pattern, FC_PIXEL_SIZE, 0, &size);
	FcPatternReference (pattern);
	matched = pattern;
	
retry:
	
	if (FcPatternGetString (matched, FC_FILE, 0, &filename) != FcResultMatch)
		goto fail;
	
	printf ("loading font from `%s'\n", filename);
	
	if (FcPatternGetInteger (matched, FC_INDEX, 0, &id) != FcResultMatch)
		goto fail;
	
	if (FT_New_Face (libft2, (const char *) filename, id, &face) != 0) {
	fail:
		if (retried)
			exit (1);
		
		sans = FcPatternBuild (NULL, FC_FAMILY, FcTypeString, "sans",
				       FC_PIXEL_SIZE, FcTypeDouble, size,
				       FC_DPI, FcTypeDouble, dpi, NULL);
		
		FcPatternDestroy (matched);
		matched = FcFontMatch (NULL, sans, &result);
		FcPatternDestroy (sans);
		filename = NULL;
		retried = true;
		goto retry;
	}
	
	FcPatternDestroy (matched);
	
	FT_Set_Pixel_Sizes (face, 0, (int) size);
	
	glyphs = g_new0 (GlyphInfo, 256);
	glyphs[0].unichar = 1; /* invalidate */
	
	g_hash_table_insert (font_cache, pattern, this);
	FcPatternReference (pattern);
	this->pattern = pattern;
	ref_count = 1;
}

TextFont::~TextFont ()
{
	int i;
	
	for (i = 0; i < 256; i++)
		g_free (glyphs[i].bitmap.buffer);
	g_free (glyphs);
	
	FT_Done_Face (face);
	
	g_hash_table_remove (font_cache, pattern);
	FcPatternDestroy (pattern);
}

TextFont *
TextFont::Load (FcPattern *pattern)
{
	TextFont *font;
	
	if ((font = (TextFont *) g_hash_table_lookup (font_cache, pattern))) {
		font->ref ();
		return font;
	}
	
	return new TextFont (pattern);
}

void
TextFont::ref ()
{
	ref_count++;
}

void
TextFont::unref ()
{
	ref_count--;
	
	if (ref_count == 0)
		delete this;
}

bool
TextFont::IsScalable ()
{
	return (face->face_flags & FT_FACE_FLAG_SCALABLE);
}

int
TextFont::EmSize ()
{
	return face->units_per_EM;
}

double
TextFont::Kerning (uint32_t left, uint32_t right)
{
	FT_Vector kerning;
	
	if (!FT_HAS_KERNING (face) || left == 0 || right == 0)
		return 0.0;
	
	FT_Get_Kerning (face, left, right, FT_KERNING_DEFAULT, &kerning);
	
	return (double) kerning.x / 64;
}

double
TextFont::Descender ()
{
	return (double) face->size->metrics.descender / 64;
}

double
TextFont::Ascender ()
{
	return (double) face->size->metrics.ascender / 64;
}

double
TextFont::Height ()
{
	return (double) face->size->metrics.height / 64;
}

static int
font_move_to (FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x, y;
	
	x = to->x / 64.0;
	y = to->y / 64.0;
	
	moon_move_to (path, x, y);
	
	return 0;
}

static int
font_line_to (FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x, y;
	
	x = to->x / 64.0;
	y = to->y / 64.0;
	
	moon_line_to (path, x, y);
	
	return 0;
}

static int
font_conic_to (FT_Vector *control, FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x0, y0;
	double x1, y1;
	double x2, y2;
	double x3, y3;
	double x, y;
	
	moon_get_current_point (path, &x0, &y0);
	
	x = control->x / 64.0;
	y = control->y / 64.0;
	
	x3 = to->x / 64.0;
	y3 = to->y / 64.0;
	
	x1 = x0 + 2.0/3.0 * (x - x0);
	y1 = y0 + 2.0/3.0 * (y - y0);
	
	x2 = x3 + 2.0/3.0 * (x - x3);
	y2 = y3 + 2.0/3.0 * (y - y3);
	
	moon_curve_to (path, x1, y1, x2, y2, x3, y3);
	
	return 0;
}

static int
font_cubic_to (FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x0, y0;
	double x1, y1;
	double x2, y2;
	
	x0 = control1->x / 64.0;
	y0 = control1->y / 64.0;
	
	x1 = control2->x / 64.0;
	y1 = control2->y / 64.0;
	
	x2 = to->x / 64.0;
	y2 = to->y / 64.0;
	
	moon_curve_to (path, x0, y0, x1, y1, x2, y2);
	
	return 0;
}

static const FT_Outline_Funcs outline_funcs = {
        (FT_Outline_MoveToFunc) font_move_to,
        (FT_Outline_LineToFunc) font_line_to,
        (FT_Outline_ConicToFunc) font_conic_to,
        (FT_Outline_CubicToFunc) font_cubic_to,
        0, /* shift */
        0, /* delta */
};

GlyphInfo *
TextFont::GetGlyphInfo (uint32_t unichar)
{
	GlyphInfo *glyph;
	uint32_t index;
	size_t size;
	
	if (unichar == 0)
		return NULL;
	
	index = unichar & 0xff;
	glyph = &glyphs[index];
	
	if (glyph->unichar != unichar) {
		glyph->index = FcFreeTypeCharIndex (face, unichar);
		glyph->unichar = unichar;
		
		if (glyph->surface) {
			cairo_surface_destroy (glyph->surface);
			glyph->surface = NULL;
		}
		
		if (glyph->path)
			moon_path_destroy (glyph->path);
		
		g_free (glyph->bitmap.buffer);
		
		if (glyph->index > 0 && FT_Load_Glyph (face, glyph->index, LOAD_FLAGS) == 0) {
			if (FT_Render_Glyph (face->glyph, FT_RENDER_MODE_NORMAL) != 0)
				goto unavail;
			
			if (face->face_flags & FT_FACE_FLAG_SCALABLE) {
				glyph->path = moon_path_new (8);
				FT_Outline_Transform (&face->glyph->outline, &invert_y);
				FT_Outline_Decompose (&face->glyph->outline, &outline_funcs, glyph->path);
				memset (&glyph->bitmap, 0, sizeof (&glyph->bitmap));
				glyph->bitmap_left = 0;
				glyph->bitmap_top = 0;
			} else {
				glyph->path = NULL;
				glyph->bitmap = face->glyph->bitmap;
				
				size = face->glyph->bitmap.rows * face->glyph->bitmap.pitch;
				glyph->bitmap.buffer = (unsigned char *) g_malloc (size);
				memcpy (glyph->bitmap.buffer, face->glyph->bitmap.buffer, size);
				glyph->bitmap_left = face->glyph->bitmap_left;
				glyph->bitmap_top = face->glyph->bitmap_top;
			}
			
			glyph->metrics.horiBearingX = face->glyph->metrics.horiBearingX / 64.0;
			glyph->metrics.horiBearingY = face->glyph->metrics.horiBearingY / 64.0;
			glyph->metrics.vertBearingX = face->glyph->metrics.vertBearingX / 64.0;
			glyph->metrics.vertBearingY = face->glyph->metrics.vertBearingY / 64.0;
			glyph->metrics.horiAdvance = face->glyph->metrics.horiAdvance / 64.0;
			glyph->metrics.vertAdvance = face->glyph->metrics.vertAdvance / 64.0;
			glyph->metrics.height = face->glyph->metrics.height / 64.0;
			glyph->metrics.width = face->glyph->metrics.width / 64.0;
		} else if (glyph->index == 0 && (unichar == 0x20 || unichar == 0x09)) {
			glyph->metrics.horiBearingX = 0.0;
			glyph->metrics.horiBearingY = 0.0;
			glyph->metrics.vertBearingX = 0.0;
			glyph->metrics.vertBearingY = 0.0;
			
			glyph->path = NULL;
			memset (&glyph->bitmap, 0, sizeof (&glyph->bitmap));
			glyph->bitmap_left = 0;
			glyph->bitmap_top = 0;
			
			if (unichar == 0x20) {
				// Space
				glyph->metrics.horiAdvance = face->max_advance_width / 64.0;
				glyph->metrics.vertAdvance = face->max_advance_height / 64.0;
				glyph->metrics.height = face->max_advance_height / 64.0;
				glyph->metrics.width = face->max_advance_width / 64.0;
			} else if (unichar == 0x09) {
				// Tab
				glyph->metrics.horiAdvance = face->max_advance_width / 8.0;
				glyph->metrics.vertAdvance = face->max_advance_height / 64.0;
				glyph->metrics.height = face->max_advance_height / 64.0;
				glyph->metrics.width = face->max_advance_width / 8.0;
			}
		} else {
		unavail:
			glyph->metrics.horiBearingX = 0.0;
			glyph->metrics.horiBearingY = 0.0;
			glyph->metrics.vertBearingX = 0.0;
			glyph->metrics.vertBearingY = 0.0;
			glyph->metrics.horiAdvance = 0.0;
			glyph->metrics.vertAdvance = 0.0;
			glyph->metrics.height = 0.0;
			glyph->metrics.width = 0.0;
			memset (&glyph->bitmap, 0, sizeof (&glyph->bitmap));
			glyph->bitmap_left = 0;
			glyph->bitmap_top = 0;
			glyph->path = NULL;
		}
	}
	
	if (glyph->metrics.horiAdvance > 0)
		return glyph;
	
	return NULL;
}

double
TextFont::UnderlinePosition ()
{
	return (double) -face->underline_position / 64.0;
}

double
TextFont::UnderlineThickness ()
{
	// Note: integer math seems to match more closely with Silverlight here.
	return (double) (face->underline_thickness / 64);
}

#define BITSWAP8(c) ((((c) * 0x0802LU & 0x22110LU) | ((c) * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16)

void
TextFont::RenderGlyphBitmap (cairo_t *cr, GlyphInfo *glyph, double x, double y)
{
	int width, height, stride;
	cairo_format_t format;
	FT_Bitmap *bitmap;
	unsigned char *d;
	int count;
	
	bitmap = &glyph->bitmap;
	
	height = bitmap->rows;
	width = bitmap->width;
	
	if (!glyph->surface) {
		switch (bitmap->pixel_mode) {
		case FT_PIXEL_MODE_MONO:
			//printf ("pixel_mode is FT_PIXEL_MODE_MONO\n");
			stride = (((width + 31) & ~31) >> 3);
			format = CAIRO_FORMAT_A1;
			
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			count = stride * height;
			d = bitmap->buffer;
			
			while (count--) {
				*d = BITSWAP8 (*d);
				d++;
			}
#endif
			break;
		case FT_PIXEL_MODE_LCD:
		case FT_PIXEL_MODE_LCD_V:
		case FT_PIXEL_MODE_GRAY:
			//printf ("pixel_mode is FT_PIXEL_MODE_GRAY\n");
			stride = bitmap->pitch;
			format = CAIRO_FORMAT_A8;
			break;
		default:
			printf ("unknown pixel format\n");
			return;
		}
		
		glyph->surface = cairo_image_surface_create_for_data (bitmap->buffer, format,
								      width, height, stride);
	}
	
	// take horiBearingX into consideration
	x += glyph->metrics.horiBearingX;
	
	// Bitmap glyphs are rendered from the top down
	y -= glyph->metrics.horiBearingY;
	
	cairo_save (cr);
	
	cairo_mask_surface (cr, glyph->surface, x, y);
	cairo_set_operator (cr, CAIRO_OPERATOR_SATURATE);
	
	cairo_new_path (cr);
	cairo_rectangle (cr, x, y, (double) width, (double) height);
	cairo_close_path (cr);
	cairo_fill (cr);
	
	cairo_restore (cr);
}

void
TextFont::RenderGlyphPath (cairo_t *cr, GlyphInfo *glyph, double x, double y)
{
	cairo_new_path (cr);
	Path (cr, glyph, x, y);
	cairo_close_path (cr);
	cairo_fill (cr);
}

void
TextFont::Render (cairo_t *cr, GlyphInfo *glyph, double x, double y)
{
	if (glyph->path)
		RenderGlyphPath (cr, glyph, x, y);
	else
		RenderGlyphBitmap (cr, glyph, x, y);
}

void
TextFont::Render (cairo_t *cr, uint32_t unichar, double x, double y)
{
	GlyphInfo *glyph;
	
	if (!(glyph = GetGlyphInfo (unichar)))
		return;
	
	Render (cr, glyph, x, y);
}

void
TextFont::Path (cairo_t *cr, GlyphInfo *glyph, double x, double y)
{
	if (!glyph->path)
		return;
	
	cairo_save (cr);
	
	cairo_translate (cr, x, y);
	cairo_append_path (cr, &glyph->path->cairo);
	
	cairo_restore (cr);
}

void
TextFont::Path (cairo_t *cr, uint32_t unichar, double x, double y)
{
	GlyphInfo *glyph;
	
	if (!(glyph = GetGlyphInfo (unichar)))
		return;
	
	Path (cr, glyph, x, y);
}


TextFontDescription::TextFontDescription ()
{
	font = NULL;
	
	set = 0;
	family = NULL;
	filename = NULL;
	style = FontStylesNormal;
	weight = FontWeightsNormal;
	stretch = FontStretchesNormal;
	size = 14.666f;
	index = 0;
}

TextFontDescription::TextFontDescription (const char *str)
{
	// FIXME: implement me
}

TextFontDescription::~TextFontDescription ()
{
	if (font != NULL)
		font->unref ();
	
	g_free (filename);
	g_free (family);
}

FcPattern *
TextFontDescription::CreatePattern ()
{
	FcPattern *pattern, *matched;
	char **families;
	FcResult result;
	int i;
	
	pattern = FcPatternCreate ();
	FcPatternAddDouble (pattern, FC_DPI, dpi);
	
	if (set & FontMaskFilename) {
		FcPatternAddString (pattern, FC_FILE, (FcChar8 *) filename);
		FcPatternAddInteger (pattern, FC_INDEX, index);
	}
	
	families = g_strsplit (GetFamily (), ",", -1);
	for (i = 0; families[i]; i++)
		FcPatternAddString (pattern, FC_FAMILY, (FcChar8 *) g_strstrip (families[i]));
	g_strfreev (families);
	
	FcPatternAddInteger (pattern, FC_SLANT, fc_style (style));
	FcPatternAddInteger (pattern, FC_WEIGHT, fc_weight (weight));
	FcPatternAddInteger (pattern, FC_WIDTH, fc_stretch (stretch));
	FcPatternAddDouble (pattern, FC_PIXEL_SIZE, size);
	
	if (!(matched = FcFontMatch (NULL, pattern, &result)))
		return pattern;
	
	FcPatternDestroy (pattern);
	
	return matched;
}

TextFont *
TextFontDescription::GetFont ()
{
	FcPattern *pattern;
	
	if (font == NULL) {
		//char *str = ToString ();
		//printf ("requested font: %s\n", str);
		//g_free (str);
		
		pattern = CreatePattern ();
		font = TextFont::Load (pattern);
		FcPatternDestroy (pattern);
	}
	
	if (font)
		font->ref ();
	
	return font;
}

uint8_t
TextFontDescription::GetFields ()
{
	return set;
}

void
TextFontDescription::UnsetFields (uint8_t mask)
{
	if (!(set & mask))
		return;
	
	if (font != NULL) {
		font->unref ();
		font = NULL;
	}
	
	if (mask & FontMaskFilename) {
		g_free (filename);
		filename = NULL;
		index = 0;
	}
	
	if (mask & FontMaskFamily) {
		g_free (family);
		family = NULL;
	}
	
	set &= ~mask;
}

void
TextFontDescription::Merge (TextFontDescription *desc, bool replace)
{
	bool changed = false;
	
	if ((desc->set & FontMaskFilename) && (!(set & FontMaskFilename) || replace)) {
		if (!filename || strcmp (filename, desc->filename) != 0) {
			g_free (filename);
			filename = g_strdup (desc->filename);
			changed = true;
		}
		
		index = desc->index;
		
		set |= FontMaskFilename;
	}
	
	if ((desc->set & FontMaskFamily) && (!(set & FontMaskFamily) || replace)) {
		if (!family || strcmp (family, desc->family) != 0) {
			g_free (family);
			family = g_strdup (desc->family);
			changed = true;
		}
		
		set |= FontMaskFamily;
	}
	
	if ((desc->set & FontMaskStyle) && (!(set & FontMaskStyle) || replace)) {
		if (style != desc->style) {
			style = desc->style;
			changed = true;
		}
		
		set |= FontMaskStyle;
	}
	
	if ((desc->set & FontMaskWeight) && (!(set & FontMaskWeight) || replace)) {
		if (weight != desc->weight) {
			weight = desc->weight;
			changed = true;
		}
		
		set |= FontMaskWeight;
	}
	
	if ((desc->set & FontMaskStretch) && (!(set & FontMaskStretch) || replace)) {
		if (stretch != desc->stretch) {
			stretch = desc->stretch;
			changed = true;
		}
		
		set |= FontMaskStretch;
	}
	
	if ((desc->set & FontMaskSize) && (!(set & FontMaskSize) || replace)) {
		if (size != desc->size) {
			size = desc->size;
			changed = true;
		}
		
		set |= FontMaskSize;
	}
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
	}
}

const char *
TextFontDescription::GetFilename ()
{
	if (set & FontMaskFilename)
		return filename;
	
	return NULL;
}

void
TextFontDescription::SetFilename (const char *filename)
{
	g_free (this->filename);
	
	if (filename) {
		this->filename = g_strdup (filename);
		set |= FontMaskFilename;
	} else {
		this->filename = NULL;
		set &= ~FontMaskFilename;
	}
}

int
TextFontDescription::GetIndex ()
{
	return index;
}

void
TextFontDescription::SetIndex (int index)
{
	this->index = index;
}

const char *
TextFontDescription::GetFamily ()
{
	if (set & FontMaskFamily)
		return family;
	
	return "Lucida Sans Unicode, Lucida Sans";
}

void
TextFontDescription::SetFamily (const char *family)
{
	g_free (this->family);
	
	if (family) {
		this->family = g_strdup (family);
		set |= FontMaskFamily;
	} else {
		this->family = NULL;
		set &= ~FontMaskFamily;
	}
}

FontStyles
TextFontDescription::GetStyle ()
{
	return style;
}

void
TextFontDescription::SetStyle (FontStyles style)
{
	this->style = style;
	set |= FontMaskStyle;
}

FontWeights
TextFontDescription::GetWeight ()
{
	return weight;
}

void
TextFontDescription::SetWeight (FontWeights weight)
{
	this->weight = weight;
	set |= FontMaskWeight;
}

FontStretches
TextFontDescription::GetStretch ()
{
	return stretch;
}

void
TextFontDescription::SetStretch (FontStretches stretch)
{
	this->stretch = stretch;
	set |= FontMaskStretch;
}

double
TextFontDescription::GetSize ()
{
	return size;
}

void
TextFontDescription::SetSize (double size)
{
	this->size = size;
	set |= FontMaskSize;
}

char *
TextFontDescription::ToString ()
{
	bool attrs = false;
	GString *str;
	
	if (set == 0)
		return NULL;
	
	str = g_string_new ("");
	
	if (set & FontMaskFilename) {
		g_string_append (str, "font:");
		g_string_append (str, filename);
		g_string_append_printf (str, "?index=%d", index);
		g_string_append (str, "?family=");
	}
	
	if (set & FontMaskFamily) {
		if (strchr (family, ',')) {
			g_string_append_c (str, '"');
			g_string_append (str, family);
			g_string_append_c (str, '"');
		} else {
			g_string_append (str, family);
		}
	} else if (!(set & FontMaskFilename)) {
		g_string_append (str, "\"Lucida Sans Unicode, Lucida Sans\"");
	}
	
	if ((set & FontMaskStyle) && style != FontStylesNormal) {
		if (!attrs) {
			g_string_append_c (str, ',');
			attrs = true;
		}
		
		g_string_append_c (str, ' ');
		switch (style) {
		case FontStylesNormal:
			break;
		case FontStylesOblique:
			g_string_append (str, "Oblique");
			break;
		case FontStylesItalic:
			g_string_append (str, "Italic");
			break;
		}
	}
	
	if ((set & FontMaskWeight) && weight != FontWeightsNormal) {
		if (!attrs) {
			g_string_append_c (str, ',');
			attrs = true;
		}
		
		g_string_append_c (str, ' ');
		switch (weight) {
		case FontWeightsThin:
			g_string_append (str, "Thin");
			break;
		case FontWeightsExtraLight:
			g_string_append (str, "ExtraLight");
			break;
		case FontWeightsLight:
			g_string_append (str, "Light");
			break;
		case FontWeightsNormal:
			break;
		case FontWeightsSemiBold:
			g_string_append (str, "SemiBold");
			break;
		case FontWeightsBold:
			g_string_append (str, "Bold");
			break;
		case FontWeightsExtraBold:
			g_string_append (str, "ExtraBold");
			break;
		case FontWeightsBlack:
			g_string_append (str, "Black");
			break;
		case FontWeightsExtraBlack:
			g_string_append (str, "ExtraBlack");
			break;
		default:
			g_string_append_printf (str, "%d", (int) weight);
			break;
		}
	}
	
	if ((set & FontMaskStretch) && stretch != FontStretchesNormal) {
		if (!attrs) {
			g_string_append_c (str, ',');
			attrs = true;
		}
		
		g_string_append_c (str, ' ');
		switch (stretch) {
		case FontStretchesUltraCondensed:
			g_string_append (str, "UltraCondensed");
			break;
		case FontStretchesExtraCondensed:
			g_string_append (str, "ExtraCondensed");
			break;
		case FontStretchesCondensed:
			g_string_append (str, "Condensed");
			break;
		case FontStretchesSemiCondensed:
			g_string_append (str, "SemiCondensed");
			break;
		case FontStretchesNormal:
			break;
#if 0
		case FontStretchesMedium:
			g_string_append (str, "Medium");
			break;
#endif
		case FontStretchesSemiExpanded:
			g_string_append (str, "SemiExpanded");
			break;
		case FontStretchesExpanded:
			g_string_append (str, "Expanded");
			break;
		case FontStretchesExtraExpanded:
			g_string_append (str, "ExtraExpanded");
			break;
		case FontStretchesUltraExpanded:
			g_string_append (str, "UltraExpanded");
			break;
		}
	}
	
	g_string_append_printf (str, " %.3fpx", size);
	
	return g_string_free (str, false);
}


TextRun::TextRun (const char *utf8, int len, TextDecorations deco, TextFontDescription *font, Brush *fg)
{
	this->text = g_utf8_to_ucs4_fast (utf8, len, NULL);
	this->font = font->GetFont ();
	this->deco = deco;
	this->fg = fg;
}

TextRun::TextRun (TextFontDescription *font)
{
	// This TextRun will represent a LineBreak
	this->deco = TextDecorationsNone;
	this->font = font->GetFont ();
	this->text = NULL;
	this->fg = NULL;
}

TextRun::~TextRun ()
{
	font->unref ();
	g_free (text);
}



class TextSegment : public List::Node {
public:
	cairo_path_t *path;
	int start, end;
	TextRun *run;
	double width;
	
	TextSegment (TextRun *run, int start);
	~TextSegment ();
};

TextSegment::TextSegment (TextRun *run, int start)
{
	this->width = 0.0;
	this->path = NULL;
	this->run = run;
	this->start = start;
	this->end = start;
}

TextSegment::~TextSegment ()
{
	if (path)
		cairo_path_destroy (path);
}

class TextLine : public List::Node {
public:
	List *segments;
	double descend;
	//double ascend;
	double height;
	
	TextLine ();
	~TextLine ();
};

TextLine::TextLine ()
{
	segments = new List ();
	descend = 0.0;
	//ascend = -1.0;
	height = -1.0;
}

TextLine::~TextLine ()
{
	segments->Clear (true);
	delete segments;
}

TextLayout::TextLayout ()
{
	wrapping = TextWrappingNoWrap;
	max_height = -1.0;
	max_width = -1.0;
	
	runs = NULL;
	
	lines = new List ();
	
	bbox_height = -1.0;
	bbox_width = -1.0;
	height = -1.0;
	width = -1.0;
}

TextLayout::~TextLayout ()
{
	if (runs) {
		runs->Clear (true);
		delete runs;
	}
	
	lines->Clear (true);
	delete lines;
}

double
TextLayout::GetMaxWidth ()
{
	return max_width;
}

void
TextLayout::SetMaxWidth (double max)
{
	if (max_width == max)
		return;
	
	max_width = max;
	
	bbox_height = -1.0;
	bbox_width = -1.0;
	height = -1.0;
	width = -1.0;
}

double
TextLayout::GetMaxHeight ()
{
	return max_height;
}

void
TextLayout::SetMaxHeight (double max)
{
	if (max_height == max)
		return;
	
	max_height = max;
	
	bbox_height = -1.0;
	bbox_width = -1.0;
	height = -1.0;
	width = -1.0;
}

TextWrapping
TextLayout::GetWrapping ()
{
	return wrapping;
}

void
TextLayout::SetWrapping (TextWrapping wrapping)
{
	if (this->wrapping == wrapping)
		return;
	
	this->wrapping = wrapping;
	
	bbox_height = -1.0;
	bbox_width = -1.0;
	height = -1.0;
	width = -1.0;
}

List *
TextLayout::GetTextRuns ()
{
	return runs;
}

void
TextLayout::SetTextRuns (List *runs)
{
	if (this->runs) {
		this->runs->Clear (true);
		delete this->runs;
	}
	
	this->runs = runs;
	
	bbox_height = -1.0;
	bbox_width = -1.0;
	height = -1.0;
	width = -1.0;
}

/**
 * TextLayout::GetActualExtents:
 * @width:
 * @height:
 *
 * Gets the actual width and height extents required for rendering the
 * full text.
 **/
void
TextLayout::GetActualExtents (double *width, double *height)
{
	*height = this->height;
	*width = this->width;
}

/**
 * TextLayout::GetLayoutExtents:
 * @width:
 * @height:
 *
 * Gets the width and height extents suitable for rendering the text
 * w/ the current wrapping model.
 **/
void
TextLayout::GetLayoutExtents (double *width, double *height)
{
	*height = this->bbox_height;
	*width = this->bbox_width;
}

struct Space {
	double width;
	int index;
};

#define BBOX_MARGIN 1
#define BBOX_PADDING 2

void
TextLayout::Layout ()
{
	TextSegment *segment;
	bool clipped = false;
	uint32_t prev = 0;
	GlyphInfo *glyph;
	TextLine *line;
	double advance;
	double descend;
	double lw, lh;
	bool is_space;
	TextRun *run;
	Space spc;
	bool wrap;
	int i;
	
	if (width != -1.0 && height != -1.0)
		return;
	
	lines->Clear (true);
	lh = height = 0.0;
	lw = width = 0.0;
	descend = 0.0;
	
	if (!runs || runs->IsEmpty () || max_width == 0 || max_height == 0)
		return;
	
	line = new TextLine ();
	for (run = (TextRun *) runs->First (); run; run = (TextRun *) run->next) {
#if 0
		if (max_height >= 0.0 && height > max_height) {
			// Stop calculating layout at this point, any
			// text beyond this point won't be rendered
			// anyway.
			break;
		}
#endif
		
		if (run->text == NULL /* LineBreak */) {
			if (lh == 0.0) {
				descend = run->font->Descender ();
				//ascend = run->font->Ascender ();
				lh = run->font->Height ();
			}
			
			lines->Append (line);
			line->descend = descend;
			//line->ascend = ascend;
			line->height = lh;
			height += lh;
			
			width = MAX (width, lw);
			
			if (run->next)
				line = new TextLine ();
			else
				line = 0;
			
			clipped = false;
			lw = lh = 0.0;
			descend = 0.0;
			//ascend = 0.0;
			continue;
		}
		
		if (!run->text[0])
			continue;
		
		descend = MIN (descend, run->font->Descender ());
		//ascend = MAX (ascend, run->font->Ascender ());
		lh = MAX (lh, run->font->Height ());
		
		spc.index = -1;
		spc.width = -1.0;
		segment = new TextSegment (run, 0);
		for (i = 0, prev = 0; run->text[i]; i++) {
			if (!(glyph = run->font->GetGlyphInfo (run->text[i])))
				continue;
			
			advance = glyph->metrics.horiAdvance;
			advance += run->font->Kerning (prev, glyph->index);
			
			if ((is_space = g_unichar_isspace (run->text[i]))) {
				spc.width = lw + advance;
				spc.index = i;
			}
			
			if (max_width >= 0.0 && (lw + advance + 1.0) > max_width) {
				switch (wrapping) {
				case TextWrappingWrapWithOverflow:
					// We can stretch the width as wide as we have to
					// in order to fit the current word.
					if (spc.index != -1 && (i - spc.index) <= 2) {
						// We've only got max 2 chars from the word...
						// wrap at the previous char (which is a space).
						segment->end = spc.index + 1;
						lw = spc.width;
						i = spc.index;
						wrap = true;
					} else {
						// In the middle of a word, can't wrap here.
						wrap = false;
					}
					break;
				case TextWrappingWrap:
					// We cannot allow line width > max_width.
					if (spc.index != -1 && (i - spc.index) <= 2) {
						// We've only got 1 char from the word...
						// wrap at the previous char (which is a space).
						segment->end = spc.index + 1;
						lw = spc.width;
						i = spc.index;
					} else {
						// Force wrap in the middle of the word.
						segment->end = i;
						i--;
					}
					
					wrap = true;
					break;
				case TextWrappingNoWrap:
				default:
					// Never wrap.
					
					if (is_space && !clipped) {
						lw += advance;
						if (lw > bbox_width)
							bbox_width = lw;
						
						segment->end = i + 1;
						clipped = true;
					}
					
					wrap = false;
					break;
				}
			} else {
				wrap = false;
			}
			
			if (wrap) {
				// end this segment and line
				line->segments->Append (segment);
				segment = new TextSegment (run, i + 1);
				
				width = MAX (width, lw);
				lines->Append (line);
				line->descend = descend;
				//line->ascend = ascend;
				line->height = lh;
				height += lh;
				
				// create a new line
				descend = run->font->Descender ();
				//ascend = run->font->Ascender ();
				lh = run->font->Height ();
				line = new TextLine ();
				spc.index = -1;
				lw = 0.0;
				
				prev = 0;
			} else {
				// add this glyph to the current line
				prev = glyph->index;
				lw += advance;
			}
		}
		
		if (!clipped)
			segment->end = i;
		
		line->segments->Append (segment);
		
		width = MAX (width, lw);
	}
	
	if (line) {
		width = MAX (width, lw);
		lines->Append (line);
		line->descend = descend;
		//line->ascend = ascend;
		line->height = lh;
		height += lh;
	}
	
	height += 1.0;
	width += 1.0;
	
	// height is never clipped
	bbox_height = height;
	
	// width is only clipped if TextWrapping is set to NoWrap
	if (bbox_width == -1.0)
		bbox_width = width;
	
	//printf ("layout extents are %.3f, %.3f, bounding box extents are %.3f, %.3f\n", width, height, bbox_width, bbox_height);
}

void
TextLayout::Render (cairo_t *cr, UIElement *element, double x, double y)
{
	TextSegment *segment;
	TextDecorations deco;
	TextFont *font = NULL;
	const uint32_t *text;
	uint32_t prev = 0;
	GlyphInfo *glyph;
	Brush *fg = NULL;
	TextLine *line;
	double height;
	double x1, y1;
	double x0;
	int i;
	
	Layout ();
	
	x1 = x;
	
	line = (TextLine *) lines->First ();
	
	while (line) {
		segment = (TextSegment *) line->segments->First ();
		while (segment) {
			text = segment->run->text;
			deco = segment->run->deco;
			font = segment->run->font;
			x0 = x1;
			
			height = font->Height ();
			
			if (segment->run->fg != fg) {
				fg = segment->run->fg;
				fg->SetupBrush (cr, element);
			}
			
			if (!segment->path) {
				if (font->IsScalable ())
					cairo_new_path (cr);
				
				for (i = segment->start, prev = 0; i < segment->end; i++) {
					if (!(glyph = font->GetGlyphInfo (text[i])))
						continue;
					
					if (glyph->index > 0) {
						x1 += font->Kerning (prev, glyph->index);
						prev = glyph->index;
						
						// set y1 to the baseline (descend is a negative value)
						y1 = y + line->height + line->descend;
						
						if (!font->IsScalable ())
							font->Render (cr, glyph, x1, y1);
						else
							font->Path (cr, glyph, x1, y1);
					}
					
					x1 += glyph->metrics.horiAdvance;
				}
				
				if (font->IsScalable ()) {
					cairo_close_path (cr);

					segment->path = cairo_copy_path (cr);
					segment->width = x1 - x0;
					
					cairo_fill (cr);
				}
			} else {
				cairo_append_path (cr, segment->path);
				x1 = x0 + segment->width;
				cairo_fill (cr);
			}
			
			if (deco == TextDecorationsUnderline) {
				cairo_antialias_t aa = cairo_get_antialias (cr);
				double thickness = font->UnderlineThickness ();
				double pos = font->UnderlinePosition ();
				
				pos += y + line->height + line->descend;
				
				cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
				cairo_set_line_width (cr, thickness);
				
				cairo_new_path (cr);
				cairo_move_to (cr, x0, pos);
				cairo_line_to (cr, x1, pos);
				cairo_stroke (cr);
				
				cairo_set_antialias (cr, aa);
			}
			
			segment = (TextSegment *) segment->next;
		}
		
		y += (double) line->height;
		
		line = (TextLine *) line->next;
		x1 = x;
	}
}
