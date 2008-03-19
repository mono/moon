/*
 * font.h: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

#include <glib.h>
#include <cairo.h>
#include <ft2build.h>
#include <gtk/gtk.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>

#include "uielement.h"
#include "brush.h"
#include "list.h"

enum FontStretches {
	FontStretchesUltraCondensed = 1,
	FontStretchesExtraCondensed = 2,
	FontStretchesCondensed      = 3,
	FontStretchesSemiCondensed  = 4,
	FontStretchesNormal         = 5,
	FontStretchesMedium         = 5,
	FontStretchesSemiExpanded   = 6,
	FontStretchesExpanded       = 7,
	FontStretchesExtraExpanded  = 8,
	FontStretchesUltraExpanded  = 9
};

enum FontStyles {
	FontStylesNormal,
	FontStylesOblique,
	FontStylesItalic
};

enum FontWeights {
	FontWeightsThin       = 100,
	FontWeightsExtraLight = 200,
	FontWeightsLight      = 300,
	FontWeightsNormal     = 400,
	FontWeightsMedium     = 500,
	FontWeightsSemiBold   = 600,
	FontWeightsBold       = 700,
	FontWeightsExtraBold  = 800,
	FontWeightsBlack      = 900,
	FontWeightsExtraBlack = 950,
};

enum StyleSimulations {
	StyleSimulationsNone
};

enum TextDecorations {
	TextDecorationsNone,
	TextDecorationsUnderline
};

enum TextWrapping {
	TextWrappingWrap,
	TextWrappingNoWrap,
	TextWrappingWrapWithOverflow
};


enum FontMask {
	FontMaskFamily   = 1 << 0,
	FontMaskStyle    = 1 << 1,
	FontMaskWeight   = 1 << 2,
	FontMaskStretch  = 1 << 3,
	FontMaskSize     = 1 << 4,
	FontMaskFilename = 1 << 5,
};


G_BEGIN_DECLS
void font_init (void);
void font_shutdown (void);
G_END_DECLS

struct GlyphBitmap;

struct GlyphMetrics {
	double horiBearingX;
	double horiBearingY;
	double horiAdvance;
	//double vertBearingX;
	//double vertBearingY;
	//double vertAdvance;
	double height;
	double width;
};

struct GlyphInfo {
	gunichar unichar;
	uint32_t index;
	GlyphMetrics metrics;
	GlyphBitmap *bitmap;
	moon_path *path;
	int requested;
};

class TextFont {
	int ref_count;
	
	FcPattern *pattern;
	FT_Face face;
	
	double underline_thickness;
	double underline_position;
	double scale;
	
	GlyphInfo glyphs[256];
	int nglyphs;
	
	TextFont (FcPattern *pattern, const char *family_name, const char *debug_name);
	
	bool OpenZipArchiveFont (FcPattern *pattern, const char *path, const char **families);
	
	void RenderGlyphPath (cairo_t *cr, GlyphInfo *glyph, double x, double y);
	void RenderGlyphBitmap (cairo_t *cr, GlyphInfo *glyph, double x, double y);
	
public:
	
	~TextFont ();
	
	void ref ();
	void unref ();
	
	static TextFont *Load (FcPattern *pattern, const char *family_name, const char *debug_name);
	
	GlyphInfo *GetGlyphInfo (gunichar unichar);
	GlyphInfo *GetGlyphInfoByIndex (uint32_t index);
	GlyphInfo *GetGlyphInfo (gunichar unichar, uint32_t index);
	
	bool HasGlyph (gunichar unichar);
	
	bool IsScalable ();
	
	double Kerning (gunichar left, gunichar right);
	double Descender ();
        double Ascender ();
	double Height ();
	
	double UnderlinePosition ();
	double UnderlineThickness ();
	
	void Path (cairo_t *cr, GlyphInfo *glyph, double x, double y);
	void Path (cairo_t *cr, gunichar unichar, double x, double y);
	
	void Render (cairo_t *cr, GlyphInfo *glyph, double x, double y);
	void Render (cairo_t *cr, gunichar unichar, double x, double y);
};


class TextFontDescription {
	TextFont *font;
	
	// bitmask of set attributes
	uint8_t set;
	
	// font attributes
	char *family;
	char *filename;
	FontStyles style;
	FontWeights weight;
	FontStretches stretch;
	double size;
	int index;
	
	FcPattern *CreatePattern ();
	
public:
	TextFontDescription ();
	TextFontDescription (const char *str);
	~TextFontDescription ();
	
	TextFont *GetFont ();
	
	uint8_t GetFields ();
	
	void UnsetFields (uint8_t mask);
	
	void Merge (TextFontDescription *desc, bool replace);
	
	const char *GetFilename ();
	void SetFilename (const char *filename);
	
	int GetIndex ();
	void SetIndex (int index);
	
	const char *GetFamily ();
	void SetFamily (const char *family);
	
	FontStyles GetStyle ();
	void SetStyle (FontStyles style);
	
	FontWeights GetWeight ();
	void SetWeight (FontWeights weight);
	
	FontStretches GetStretch ();
	void SetStretch (FontStretches stretch);
	
	double GetSize ();
	void SetSize (double size);
	
	char *ToString ();
};

#if 0
class TextRun : public List::Node {
public:
	TextDecorations deco;
	gunichar *text;
	TextFont *font;
	Brush **fg;
	
	TextRun (const char *utf8, int len, TextDecorations deco, TextFontDescription *font, Brush **fg);
	TextRun (TextFontDescription *font);
	~TextRun ();
};


class TextLayout {
	// User-set data;
	TextWrapping wrapping;
	double max_height;
	double max_width;
	List *runs;
	
	// Internal representation
	List *lines;
	
	// cached info
	double bbox_height;
	double bbox_width;
	double height;
	double width;
	
public:
	
	TextLayout ();
	~TextLayout ();
	
	double GetMaxWidth ();
	void SetMaxWidth (double width);
	
	double GetMaxHeight ();
	void SetMaxHeight (double height);
	
	TextWrapping GetWrapping ();
	void SetWrapping (TextWrapping wrapping);
	
	List *GetTextRuns ();
	void SetTextRuns (List *runs);
	
	void Layout ();
	void GetActualExtents (double *width, double *height);
	void GetLayoutExtents (double *width, double *height);
	void Render (cairo_t *cr, UIElement *element, Brush *default_fg, double x, double y);
};
#endif

#endif /* __FONT_H__ */
