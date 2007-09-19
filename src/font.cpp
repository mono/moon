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


#define LOAD_FLAGS (FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL)

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

int
Font::EmSize ()
{
	return face->units_per_EM;
}

int
Font::Height ()
{
	return face->height;
}

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


TextRun::TextRun (const char *uft8, int len, TextDecorations deco, Font *font, Brush *fg)
{
	type = Run;
	
	text = g_utf8_to_ucs4_fast (utf8, len, NULL);
	
	this->height = font->Height ();
	this->deco = deco;
	this->font = font;
	this->fg = fg;
}

TextRun::TextRun (TextDecorations deco, Font *font, Brush *fg)
{
	type = LineBreak;
	
	this->height = font->Height ();
	this->text = NULL;
	this->deco = deco;
	this->font = font;
	this->fg = fg;
}

TextRun::~TextRun ()
{
	g_free (text);
}



class TextSegment : public List::Node {
public:
	TextDecorations deco;
	const uint32_t *text;
	int start, end;
	Font *font;
	Brush *fg;
	
	int height;
	
	TextSegment (TextRun *run, int start);
};

TextSegment::TextSegment (TextRun *run, int start)
{
	deco = run->deco;
	text = run->text;
	font = run->font;
	fg = run->fg;
	
	this->start = start;
	this->end = -1;
	
	height = -1;
}

class TextLine : public List::Node {
public:
	List *segments;
	int height;
	
	TextLine ();
	~TextLine ();
};

TextLine::TextLine ()
{
	segments = new List ();
	height = -1;
}

TextLine::~TextLine ()
{
	segments->Clear ();
	delete segments;
}

TextLayout::TextLayout ()
{
	wrapping = TextWrappingNoWrap;
	max_height = -1;
	max_width = -1;
	
	runs = NULL;
	
	lines = new List ();
	
	height = -1;
	width = -1;
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

int
TextLayout::GetMaxWidth ()
{
	return max_width;
}

void
TextLayout::SetMaxWidth (int max)
{
	if (max_width == max)
		return;
	
	max_width = width;
	height = -1;
	width = -1;
}

int
TextLayout::GetMaxHeight ()
{
	return max_height;
}

void
TextLayout::SetMaxHeight (int max)
{
	if (max_height == max)
		return;
	
	max_height = max;
	height = -1;
	width = -1;
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
	height = -1;
	width = -1;
}

List *
TextLayout::GetTextRuns ()
{
	return runs;
}

void
TextLayout::SetTextRuns (List *runs)
{
	this->runs->Clear (true);
	delete this->runs;
	
	this->runs = runs;
	
	height = -1;
	width = -1;
}

void
TextLayout::GetPixelSize (int *w, int *h)
{
	if (w)
		*w = width;
	
	if (h)
		*h = height;
}

struct Space {
	int index;
	int width;
};

void
TextLayout::Layout ()
{
	TextSegment *segment;
	GlyphInfo *glyph;
	TextLine *line;
	TextRun *run;
	int lw, lh;
	Space spc;
	int i;
	
	if (width != -1 && height != -1)
		return;
	
	lines->Clear (true);
	lh = height = 0;
	lw = width = 0;
	
	if (!runs || runs->IsEmpty ())
		return;
	
	line = new TextLine ();
	for (run = (TextRun *) runs->First (); run; run = (TextRun *) run->next) {
		lh = MAX (lh, run->font->Height ());
		
		if (run->type == LineBreak) {
			lines->Append (line);
			line->height = lh;
			
			width = MAX (width, lw);
			
			if (run->next)
				line = new TextLine ();
			else
				line = 0;
			
			lw = lh = 0;
			continue;
		}
		
		if (!text[0])
			continue;
		
		spc.index = -1;
		segment = new TextSegment (run, 0);
		for (i = 0; run->text[i]; i++) {
			if (!(glyph = run->font->GetGlyphInfo (run->text[i])))
				continue;
			
			if (g_unichar_isspace (run->text[i])) {
				spc.index = i;
				spc.width = lw;
			}
			
			if ((lw + glyph->metrics.horiAdvance) <= max_width) {
				// this glyph fits nicely on this line
				lw += glyph->metrics.horiAdvance;
				continue;
			}
			
			// need to wrap
			if (spc.index != -1) {
				segment->end = spc.index;
				lw = spc->width;
				i = spc.index;
			} else {
				segment->end = i;
				i--;
			}
			
			// end this line
			if (segment->end > segment->start) {
				line->segments->Append (segment);
				segment = new TextSegment (run, i + 1);
			}
			
			lines->Append (line);
			line->height = lh;
			
			width = MAX (width, lw);
			height += lh;
			
			// create a new line
			line = new TextLine ();
			spc.index = -1;
			lw = lh = 0;
		}
		
		segment->end = i;
		line->segments->Append (segment);
	}
	
	if (line)
		lines->Append (line);
}

void
TextLayout::Render (cairo_t *cr, UIElement *element, double x, double y)
{
	cairo_image_surface_t *surface;
	int width, height, stride;
	TextSegment *segment;
	TextDecorations deco;
	FT_Bitmap *bitmap;
	Font *font = NULL;
	Brush *fg = NULL;
	GlyphInfo *glyph;
	TextLine *line;
	double dx, dy;
	int i;
	
	Layout ();
	
	if ((line = (TextLine *) lines->First ()))
		dy = (double) line->height;
	dx = 0.0f;
	
	while (line) {
		segment = (TextSegment *) line->segments->First ();
		while (segment) {
			deco = segment->deco;
			font = segment->font;
			
			if (segment->fg != fg) {
				fg = segment->fg;
				fg->SetupBrush (cr, element);
			}
			
			for (i = segment->start; i < segment->end; i++) {
				if (!(glyph = font->GetGlyphInfo (segment->text[i])))
					continue;
				
				bitmap = &glyph->bitmap;
				
				height = bitmap->rows;
				width = bitmap->width;
				stride = bitmap->pitch;
				
				cairo_move_to (cr, x + dx, y + dy);
				
				// Render the glyph
				surface = cairo_image_surface_create_for_data (bitmap->buffer,
									       CAIRO_FORMAT_A8,
									       width, height,
									       stride);
				
				cairo_set_source_surface (cr, surface, glyph->bitmap_left, glyph->bitmap_top);
				
				cairo_surface_destroy (surface);
				
				dx += (double) glyph->metrics->horiAdvance;
			}
			
			segment = (TextSegment *) segment->next;
		}
		
		if ((line = (TextLine *) line->next))
			dy += (double) line->height;
		dx = 0.0f;
	}
}







#if 0
void
TextLayout::Layout ()
{
	bool new_segment = true;
	TextAttr *uline = NULL;
	TextAttr *font = NULL;
	TextAttr *fg = NULL;
	TextSegment *segment;
	GlyphInfo *glyph;
	TextAttr *attr;
	int i = 0;
	int w, h;
	
	if (!dirty)
		return;
	
	segments->Clear (true);
	h = height = 0;
	w = width = 0;
	
	if (!text || !text[0])
		return;
	
	attr = (TextAttr *) attrs->First ();
	
	while (utext[i]) {
		if (uline && uline->end <= i)
			uline = NULL;
		if (font && font->end <= i)
			font = NULL;
		if (fg && fg->end <= i)
			fg = NULL;
		
		while (attr) {
			if (attr->start > i)
				break;
			
			switch (attr->type) {
			case TextAttrForeground:
				new_segment = true;
				fg = attr;
				break;
			case TextAttrUnderline:
				new_segment = true;
				uline = attr;
				break;
			case TextAttrFont:
				new_segment = true;
				font = attr;
				break;
			}
			
			attr = (TextAttr *) attr->next;
		}
		
		// we always need a font and an fg
		g_assert (font != NULL);
		g_assert (fg != NULL);
		
		if (new_segment) {
			if (segment)
				segment->end = i;
			
			segment = new TextSegment (Text, i);
			segment->uline = uline ? true : false;
			segment->font = font->font;
			segment->fg = fg->fg;
			
			segments->Append (segment);
			
			new_segment = false;
		}
		
		if (utext[i] == '\n') {
			// insert a LineBreak segment
			if (segment->start < i) {
				segment = new TextSegment (LineBreak, i);
				segment->uline = uline ? true : false;
				segment->font = font->font;
				segment->fg = fg->fg;
				
				segments->Append (segment);
			} else {
				// current Text segment had no text
				segment->type = LineBreak;
			}
			
			width = MAX (width, w);
			segment->end = ++i;
			w = 0;
			
			segment = new TextSegment (Text, i);
			segment->uline = uline ? true : false;
			segment->font = font->font;
			segment->fg = fg->fg;
			
			segments->Append (segment);
			
			continue;
		}
		
		glyph = font->font->GlyphInfo (utext[i]);
		i++;
	}
}
#endif
