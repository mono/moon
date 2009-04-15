/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * font.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_H__
#define __FONT_H__

#include <glib.h>
#include <cairo.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>

#include "moon-path.h"
#include "uielement.h"
#include "brush.h"
#include "enums.h"
#include "list.h"

class TextFontDescription;

// Silverlight accepts negative values ]0,-475[ as bold and everything over 1023 as normal
#define FONT_LOWER_BOLD_LIMIT	-475
#define FONT_UPPER_BOLD_LIMIT	1024

enum FontMask {
	FontMaskFamily   = (1 << 0),
	FontMaskStyle    = (1 << 1),
	FontMaskWeight   = (1 << 2),
	FontMaskStretch  = (1 << 3),
	FontMaskSize     = (1 << 4),
	FontMaskFilename = (1 << 5),
	FontMaskIndex    = (1 << 6),
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
	double height;
	double width;
};

struct GlyphInfo {
	gunichar unichar;
	guint32 index;
	GlyphMetrics metrics;
	moon_path *path;
	int requested;
};

struct FontFaceExtents {
	double underline_thickness;
	double underline_position;
	double descent;
	double ascent;
	double height;
};

class FontFace {
	//static FT_Face default_face;
	static GHashTable *cache;
	
	int ref_count;
	
	FcPattern *pattern;
	double cur_size;
	bool own_face;
	FT_Face face;
	
	static bool OpenFontDirectory (FT_Face *face, FcPattern *pattern, const char *path, const char **families);
	static bool LoadFontFace (FT_Face *face, FcPattern *pattern, const char **families);
	
	static FontFace *GetDefault (FcPattern *pattern);
	static void LoadDefaultFace ();
	
	
	FontFace (FT_Face face, FcPattern *pattern, bool own);
	~FontFace ();
	
 public:
	static void Init ();
	static void Shutdown ();
	
	void ref ();
	void unref ();
	
	static FontFace *Load (const TextFontDescription *desc);
	
	gunichar GetCharFromIndex (guint32 index);
	guint32 GetCharIndex (gunichar unichar);
	bool HasChar (gunichar unichar);
	
	bool IsScalable ();
	
	double Kerning (double size, gunichar left, gunichar right);
	void GetExtents (double size, FontFaceExtents *extents);
	bool LoadGlyph (double size, GlyphInfo *glyph);
};


class TextFont {
	static GHashTable *cache;
	
	int ref_count;
	
	FcPattern *pattern;
	
	FontFaceExtents extents;
	FontFace *face;
	double size;
	
	GlyphInfo glyphs[256];
	int nglyphs;
	
	TextFont (FontFace *face, FcPattern *pattern);
	~TextFont ();
	
 public:
	static void Init ();
	static void Shutdown ();
	
	void ref ();
	void unref ();
	
	static TextFont *Load (const TextFontDescription *desc);
	
	GlyphInfo *GetGlyphInfo (gunichar unichar);
	GlyphInfo *GetGlyphInfoByIndex (guint32 index);
	GlyphInfo *GetGlyphInfo (gunichar unichar, guint32 index);
	
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
	
	void AppendPath (moon_path *path, GlyphInfo *glyph, double x, double y);
	void AppendPath (moon_path *path, gunichar unichar, double x, double y);
};


class TextFontDescription {
	TextFont *font;
	
	// bitmask of set attributes
	guint8 set;
	
	// font attributes
	char *family;
	char *filename;
	char *guid;
	
	FontStyles style;
	FontWeights weight;
	FontStretches stretch;
	double size;
	int index;
	
 public:
	TextFontDescription ();
	TextFontDescription (const char *str);
	~TextFontDescription ();
	
	FcPattern *CreatePattern (bool sized) const;
	
	TextFont *GetFont ();
	
	guint8 GetFields () const;
	
	void UnsetFields (guint8 mask);
	
	void Merge (TextFontDescription *desc, bool replace);
	
	bool IsDefault () const;
	
	const char *GetGUID () const;
	
	const char *GetFilename () const;
	bool SetFilename (const char *filename, const char *guid = NULL);
	
	int GetIndex () const;
	bool SetIndex (int index);
	
	char **GetFamilies () const;
	const char *GetFamily () const;
	bool SetFamily (const char *family);
	
	FontStyles GetStyle () const;
	bool SetStyle (FontStyles style);
	
	FontWeights GetWeight () const;
	bool SetWeight (FontWeights weight);
	
	FontStretches GetStretch () const;
	bool SetStretch (FontStretches stretch);
	
	double GetSize () const;
	bool SetSize (double size);
	
	char *ToString () const;
};

#endif /* __FONT_H__ */
