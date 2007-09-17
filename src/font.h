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
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>

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
	FontMaskVariant  = 1 << 2,  /* unused */
	FontMaskWeight   = 1 << 3,
	FontMaskStretch  = 1 << 4,
	FontMaskSize     = 1 << 5,
	FontMaskGravity  = 1 << 6,  /* unused */
	FontMaskFilename = 1 << 7
};


void font_init (void);


struct GlyphInfo;

class Font {
	int ref_count;
	
	FcPattern *pattern;
	FT_Face face;
	
	GlyphInfo *glyphs;
	
	Font (FcPattern *pattern, double size);
	
public:
	
	~Font ();
	
	void ref ();
	void unref ();
	
	static Font *Load (FcPattern *pattern, double size);
	
	const GlyphInfo *GetGlyphInfo (uint32_t unichar);
	
	int EmSize ();
	int Height ();
	
	void Render (cairo_t *cr, const GlyphInfo *glyph);
	void Render (cairo_t *cr, uint32_t unichar);
};


class FontDescription {
	Font *font;
	
	// bitmask of set attributes
	uint8_t set;
	
	// font attributes
	char *family;
	char *filename;
	FontStyles style;
	FontWeights weight;
	FontStretches stretch;
	double size;
	
	FcPattern *CreatePattern ();
	
public:
	FontDescription ();
	FontDescription (const char *str);
	~FontDescription ();
	
	const Font *GetFont ();
	
	uint8_t GetFields ();
	
	void UnsetFields (uint8_t mask);
	
	void Merge (FontDescription *font, bool replace);
	
	const char *GetFilename ();
	void SetFilename (const char *filename);
	
	const char *GetFamily ();
	void SetFamily (const char *family);
	
	FontStyles GetStyle ();
	void SetStyle (FontStyles style);
	
	FontWeights GetWeight ();
	void SetWeight (FontWeights weight);
	
	FontStretchs GetStretch ();
	void SetStretch (FontStretchs stretch);
	
	double GetSize ();
	void SetSize (double size);
	
	char *ToString ();
};


enum TextRunType {
	LineBreak,
	Run,
};

class TextRun : public List::Node {
public:
	TextRunType type;
	
	TextDecorations deco;
	uint32_t *text;
	Font *font;
	Brush *fg;
	
	int height;
	
	TextRun (const char *uft8, int len, TextDecorations deco, Font *font, Brush *fg);
	TextRun (TextDecorations deco, Font *font, Brush *fg);
	~TextRun ();
};


class TextLayout {
	// User-set data;
	TextWrapping wrapping;
	int max_height;
	int max_width;
	List *runs;
	
	// Internal representation
	List *lines;
	
	// cached info
	int height;
	int width;
	
public:
	
	TextLayout ();
	~TextLayout ();
	
	int GetMaxWidth ();
	void SetMaxWidth (int width);
	
	int GetMaxHeight ();
	void SetMaxHeight (int height);
	
	TextWrapping GetWrapping ();
	void SetWrapping (TextWrapping wrapping);
	
	List *GetTextRuns ();
	void SetTextRuns (List *runs);
	
	void Layout ();
	void GetPixelSize (int *w, int *h);
	void Render (cairo_t *cr);
};

#endif /* __FONT_H__ */
