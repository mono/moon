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

#include "font.h"


static GHashTable *font_hash = NULL;
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
	
	font_hash = g_hash_table_new ((GHashFunc) FcPatternHash, (GEqualFunc) FcPatternEqual);
	
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
	case FontStretchesMedium:
		return FC_WIDTH_NORMAL;
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


struct GlyphInfo {
	FT_Glyph_Metrics metrics;
	uint32_t unichar;
	uint32_t index;
	FT_Bitmap bitmap;
	int bitmap_left;
	int bitmap_top;
};


Font::Font (FcPattern *pattern, double size)
{
	const char *filename = NULL;
	FcPattern *matched, *sans;
	bool retried = false;
	FcResult result;
	//double size;
	int flags;
	int id;
	
	FcPatternReference (pattern);
	matched = pattern;
	
	flags = FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL;
	
	//FcPatternGetDouble (matched, FC_PIXEL_SIZE, 0, &size);
	
retry:
	
	if (FcPatternGetString (matched, FC_FILE, 0, &filename) != FcResultMatch)
		goto fail;
	
	if (FcPatternGetInteger (matched, FC_INDEX, 0, &id) != FcResultMatch)
		goto fail;
	
	if (FT_New_Face (libft2, filename, id, &face) != 0) {
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
	
	FT_Set_Pixel_Sizes (face, 0, size);
	
	glyphs = g_new0 (GlyphInfo, 256);
	glyphs[0].unichar = 1; /* invalidate */
	
	g_hash_table_insert (font_cache, pattern, this);
	FcPatternReference (pattern);
	this->pattern = pattern;
	ref_count = 1;
}

Font::~Font ()
{
	int i;
	
	for (i = 0; i < 256; i++)
		g_free (glyphs[i].bitmap.buffer);
	g_free (glyphs);
	
	FT_Done_Face (face);
	g_hash_table_remove (font_cache, pattern);
	FcPatternDestroy (pattern);
}

Font *
Font::Load (FcPattern *pattern, double size)
{
	Font *font;
	
	if ((font = (Font *) g_hash_table_lookup (font_cache, pattern))) {
		font->ref ();
		return font;
	}
	
	return new Font (pattern, size);
}

void
Font::ref ()
{
	ref_count++;
}

void
Font::unref ()
{
	ref_count--;
	
	if (ref_count == 0)
		delete this;
}

#define LOAD_FLAGS (FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL)

const GlyphInfo *
Font::GetGlyphInfo (uint32_t unichar)
{
	GlyphInfo *glyph;
	uint32_t index;
	
	if (unichar == 0)
		return NULL;
	
	index = unichar & 0xff;
	glyph = &glyphs[index];
	
	if (glyph->unichar != unichar) {
		glyph->index = FcFreeTypeCharIndex (face, unichar);
		glyph->unichar = unichar;
		
		g_free (glyph->bitmap.buffer);
		
		if (FT_Load_Glyph (face, glyph->index, LOAD_FLAGS) == 0) {
			if (FT_Render_Glyph (face->glyph, FT_RENDER_MODE_NORMAL) != 0)
				goto unavail;
			
			memcpy (&glyph->metrics, &face->glyph->metrics, sizeof (&glyph->metrics));
			
			glyph->bitmap = face->glyph->bitmap;
			glyph->bitmap.buffer = g_memdup (face->glyph->bitmap.buffer,
							 face->glyph->bitmap.rows * face->glyph->bitmap.pitch);
			glyph->bitmap_left = face->glyph->bitmap_left;
			glyph->bitmap_top = face->glyph->bitmap_top;
		} else {
		unavail:
			memset (&glyph->metrics, 0, sizeof (&glyph->metrics));
			memset (&glyph->bitmap, 0, sizeof (&glyph->bitmap));
			glyph->bitmap_left = 0;
			glyph->bitmap_top = 0;
		}
	}
	
	if (glyph->bitmap.buffer)
		return glyph;
	
	return NULL;
}

void
Font::Render (cairo_t *cr, const GlyphInfo *glyph)
{
	// FIXME: render the glyph
}

void
Font::Render (cairo_t *cr, uint32_t unichar)
{
	const GlyphInfo *glyph;
	
	if (!(glyph = GetGlyphInfo (unichar)))
		return;
	
	Render (cr, glyph);
}


FontDescription::FontDescription ()
{
	font = NULL;
	
	set = 0;
	family = NULL;
	filename = NULL;
	style = FontStylesNormal;
	weight = FontWeightsNormal;
	stretch = FontStretchesNormal;
	size = 14.666f;
}

FontDescription::FontDescription (const char *str)
{
	// FIXME: implement me
}

FontDescription::~FontDescription ()
{
	if (font != NULL)
		font->unref ();
	
	g_free (filename);
	g_free (family);
}

FcPattern *
FontDescription::CreatePattern ()
{
	FcPattern *pattern;
	char **families;
	int i;
	
	pattern = FcPatternCreate ();
	FcPatternAddDouble (pattern, FC_DPI, dpi);
	
	if (set & FontMaskFilename)
		FcPatternAddString (pattern, FC_FILE, filename);
	
	families = g_strsplit (GetFamily (), ',', -1);
	for (i = 0; families[i]; i++)
		FcPatternAddString (pattern, FC_FAMILY, families[i]);
	g_strfreev (families);
	
	FcPatternAddInteger (pattern, FC_SLANT, fc_style (style));
	FcPatternAddInteger (pattern, FC_WEIGHT, fc_weight (weight));
	FcPatternAddInteger (pattern, FC_WIDTH, fc_stretch (stretch));
	FcPatternAddDouble (pattern, FC_PIXEL_SIZE, size);
	
	return pattern;
}

const Font *
FontDescription::GetFont ()
{
	FcPattern *pattern;
	
	if (font == NULL) {
		pattern = CreatePattern ();
		font = Font::Load (pattern, size);
		FcPatternDestroy (pattern);
	}
	
	return font;
}

uint8_t
FontDescription::GetFields ()
{
	return set;
}

void
FontDescription::UnsetFields (uint8_t mask)
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
	}
	
	if (mask & FontMaskFamily) {
		g_free (family);
		family = NULL;
	}
	
	set &= ~mask;
}

void
FontDescription::Merge (FontDescription *font, bool replace)
{
	bool changed = false;
	
	if ((font->set & FontMaskFilename) && (!(set & FontMaskFilename) || replace)) {
		if (strcmp (filename, font->filename) != 0) {
			g_free (filename);
			filename = g_strdup (font->filename);
			changed = true;
		}
		
		set |= FontMaskFilename;
	}
	
	if ((font->set & FontMaskFamily) && (!(set & FontMaskFamily) || replace)) {
		if (strcmp (family, font->family) != 0) {
			g_free (family);
			family = g_strdup (font->family);
			changed = true;
		}
		
		set |= FontMaskFamily;
	}
	
	if ((font->set & FontMaskStyle) && (!(set & FontMaskStyle) || replace)) {
		if (style != font->style) {
			style = font->style;
			changed = true;
		}
		
		set |= FontMaskStyle;
	}
	
	if ((font->set & FontMaskWeight) && (!(set & FontMaskWeight) || replace)) {
		if (weight != font->weight) {
			weight = font->weight;
			changed = true;
		}
		
		set |= FontMaskWeight;
	}
	
	if ((font->set & FontMaskStretch) && (!(set & FontMaskStretch) || replace)) {
		if (stretch != font->stretch) {
			stretch = font->stretch;
			changed = true;
		}
		
		set |= FontMaskStretch;
	}
	
	if ((font->set & FontMaskSize) && (!(set & FontMaskSize) || replace)) {
		if (size != font->size) {
			size = font->size;
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
FontDescription::GetFilename ()
{
	if (set & FontMaskFilename)
		return filename;
	
	return NULL;
}

void
FontDescription::SetFilename (const char *filename)
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

const char *
FontDescription::GetFamily ()
{
	if (set & FontMaskFamily)
		return family;
	
	return "Lucida Sans";
}

void
FontDescription::SetFamily (const char *family)
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
FontDescription::GetStyle ()
{
	return style;
}

void
FontDescription::SetStyle (FontStyles style)
{
	this->style = style;
	set |= FontMaskStyle;
}

FontWeights
FontDescription::GetWeight ()
{
	return weight;
}

void
FontDescription::SetWeight (FontWeights weight)
{
	this->weight = weight;
	set |= FontMaskWeight;
}

FontStretchs
FontDescription::GetStretch ()
{
	return stretch;
}

void
FontDescription::SetStretch (FontStretchs stretch)
{
	this->stretch = stretch;
	set |= FontMaskStretch;
}

double
FontDescription::GetSize ()
{
	return size;
}

void
FontDescription::SetSize (double size)
{
	this->size = size;
	set |= FontMaskSize;
}

char *
FontDescription::ToString ()
{
	bool attrs = false;
	GString *str;
	
	if (set == 0)
		return NULL;
	
	str = g_string_new ("");
	
	if (set & FontMaskFilename) {
		g_string_append (str, "font:");
		g_string_append (str, filename);
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
		g_string_append (str, "Lucida Sans");
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
			g_string_append_printf (str, "%d", (int) wieght);
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
		case FontStretchesMedium:
			g_string_append (str, "Medium");
			break;
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


TextAttr::TextAttr (Brush *fg, int start, int end)
{
	type = TextAttrForeground;
	attr.fg = fg;
	fg.ref ();
	
	start = start;
	end = end;
}

TextAttr::TextAttr (Font *font, int start, int end)
{
	type = TextAttrFont;
	attr.font = font;
	font.ref ();
	
	start = start;
	end = end;
}

TextAttr::TextAttr (bool uline, int start, int end)
{
	type = TextAttrUnderline;
	attr.uline = uline;
	
	start = start;
	end = end;
}

TextAttr::~TextAttr ()
{
	switch (type) {
	case TextAttrForeground:
		attr.fg->unref ();
		break;
	case TextAttrFont:
		attr.font->unref ();
		break;
	default:
		break;
	}
}


//
// Notes: a TextSegment should represent a substring of text that
// shares identical font/uline/fg attrs. A new TextSegment should be
// created anytime a LineBreak is needed (either because we encounter
// a '\n' or because we need to satisfy the TextWrapping rules). I'm
// thinking that we might also want to create new segments for '\t' as
// well?
//
// For LineBreaks, we'll want to add some "MoveTo"-type info - e.g.,
// 'x' would be 0 and we'd want a 'dy' to represent the offset to the
// next baseline.
//
// For '\t', if we go that route, we'd want to have a 'dx'.
//

enum TextSegmentType {
	LineBreak,
	Text,
	Tab,
};

class TextSegment : public List::Node {
public:
	TextSegmentType type;
	
	Font *font;
	bool uline;
	Brush *fg;
	
	int start;
	int end;
	
	// post-rendered width/height of this segment (if Text)
	int height;
	int width;
	
	// For MoveTo-type segments
	int x, dx, dy;
};



TextLayout::TextLayout ()
{
	wrapping = TextWrappingNoWrap;
	max_height = -1;
	max_width = -1;
	
	attrs = NULL;
	text = NULL;
	
	utext = NULL;
	segments = new List ();
	
	dirty = true;
	height = -1;
	width = -1;
}

TextLayout::~TextLayout ()
{
	if (attrs) {
		attrs->Clear (true);
		delete attrs;
	}
	
	segments->Clear (true);
	delete segments;
	
	g_free (utext);
	g_free (text);
}

int
TextLayout::GetMaxWidth ()
{
	return max_width;
}

void
TextLayout::SetMaxWidth (int width)
{
	if (max_width == width)
		return;
	
	max_width = width;
	dirty = true;
}

int
TextLayout::GetMaxHeight ()
{
	return max_height;
}

void
TextLayout::SetMaxHeight (int height)
{
	if (max_height == height)
		return;
	
	max_height = height;
	dirty = true;
}

const char *
TextLayout::GetText ()
{
	return text;
}

void
TextLayout::SetText (const char *text)
{
	if ((this->text && text && !strcmp (this->text, text))
	    || (!this->text && !text)) {
		// text is identical, no-op
		return;
	}
	
	g_free (this->utext);
	g_free (this->text);
	
	if (text) {
		this->utext = g_utf8_to_ucs4_fast (text, -1, NULL);
		this->text = g_strdup (text);
	} else {
		this->utext = NULL;
		this->text = NULL;
	}
	
	dirty = true;
}

const List *
TextLayout::GetAttributes ()
{
	return attrs;
}

void
TextLayout::SetAttributes (List *attrs)
{
	if (!this->attrs && !attrs)
		return;
	
	if (this->attrs) {
		this->attrs->Clear (true);
		delete this->attrs;
	}
	
	this->attrs = attrs;
	dirty = true;
}

void
TextLayout::GetPixelSize (int *w, int *h)
{
	
}
