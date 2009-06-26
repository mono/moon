/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fonts.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "timesource.h"
#include "moon-path.h"
#include "debug.h"
#include "fonts.h"

//
// TextFont
//

TextFont::TextFont (FontFace **faces, int n_faces, double size)
{
	FontFaceExtents extents;
	int i;
	
	faces[0]->GetExtents (size, &this->extents);
	for (i = 1; i < n_faces; i++) {
		faces[i]->GetExtents (size, &extents);
		//this->extents.underline_position = MAX (this->extents.underline_position, extents.underline_position);
		this->extents.descent = MIN (this->extents.descent, extents.descent);
		this->extents.ascent = MAX (this->extents.ascent, extents.ascent);
		this->extents.height = MAX (this->extents.height, extents.height);
	}
	
	this->simulate = StyleSimulationsNone;
	this->n_faces = n_faces;
	this->faces = faces;
	this->n_glyphs = 0;
	this->size = size;
	this->desc = NULL;
}

TextFont::~TextFont ()
{
	ClearGlyphCache ();
	
	for (int i = 0; i < n_faces; i++)
		faces[i]->unref ();
	g_free (faces);
}

void
TextFont::ClearGlyphCache ()
{
	for (int i = 0; i < n_glyphs; i++) {
		if (glyphs[i].path)
			moon_path_destroy (glyphs[i].path);
	}
	
	n_glyphs = 0;
}

TextFont *
TextFont::Load (const char *resource, int index, double size, StyleSimulations simulate)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	FontFace **faces;
	TextFont *font;
	
	faces = g_new (FontFace *, 1);
	if (!(faces[0] = manager->OpenFont (resource, index))) {
		g_free (faces);
		return NULL;
	}
	
	font = new TextFont (faces, 1, size);
	font->simulate = simulate;
	
	return font;
}

#define lowercase(x) (((x) >= 'A' && (x) <= 'Z') ? (x) - 'A' + 'a' : (x))

static int
strcase_equal (gconstpointer v, gconstpointer v2)
{
	return g_ascii_strcasecmp ((const char *) v, (const char *) v2) == 0;
}


static guint
strcase_hash (gconstpointer key)
{
	const char *p = (const char *) key;
	guint h = 0;
	
	while (*p != '\0') {
		h = (h << 5) - h + lowercase (*p);
		p++;
	}
	
	return h;
}

static struct {
	const char *lang;
	const char *families[6];
} default_fonts[] = {
	{ "",   { "Lucida Sans Unicode", "Liberation Sans", "Bitstream Vera Sans", "DejaVu Sans", "Luxi Sans", NULL } },
	{ "zh", { "SimSun", "SimHei", "Microsoft YaHei", "Arial Unicode MS", NULL, NULL } },
	{ "ja", { "Meiryo", "MS PMincho", "MS PGothic", "MS UI Gothic", NULL, NULL } },
	{ "ko", { "Malgun Gothic", "Dotum", "Arial Unicode MS", "Batang", NULL, NULL } },
};

static void
LoadPortableUserInterface (FontManager *manager, GHashTable *loaded, GPtrArray *faces, FontStretches stretch, FontWeights weight, FontStyles style)
{
	const char **families;
	FontFace *face;
	guint i, j;
	
	for (i = 0; i < G_N_ELEMENTS (default_fonts); i++) {
		families = default_fonts[i].families;
		
		for (j = 0; families[j]; j++) {
			if (g_hash_table_lookup (loaded, families[j]))
				continue;
			
			if ((face = manager->OpenFont (families[j], stretch, weight, style))) {
				g_hash_table_insert (loaded, (void *) families[j], GINT_TO_POINTER (true));
				g_ptr_array_add (faces, face);
				break;
			}
		}
	}
}

TextFont *
TextFont::Load (const TextFontDescription *desc)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	FontStretches stretch = desc->GetStretch ();
	FontWeights weight = desc->GetWeight ();
	const char *source = desc->GetSource ();
	char **families = desc->GetFamilies ();
	FontStyles style = desc->GetStyle ();
	GHashTable *loaded;
	GPtrArray *faces;
	FontFace *face;
	TextFont *font;
	char *name;
	int i;
	
	loaded = g_hash_table_new (strcase_hash, strcase_equal);
	
	faces = g_ptr_array_new ();
	
	if (families) {
		for (i = 0; families[i]; i++) {
			if (g_hash_table_lookup (loaded, families[i]))
				continue;
			
			if (!g_ascii_strcasecmp (families[i], "Portable User Interface")) {
				LoadPortableUserInterface (manager, loaded, faces, stretch, weight, style);
			} else {
				face = NULL;
				
				if (source && !strchr (families[i], '#')) {
					// if there is a font source, try loading from the font source first
					name = g_strdup_printf ("%s#%s", source, families[i]);
					face = manager->OpenFont (name, stretch, weight, style);
					g_free (name);
				}
				
				if (face == NULL)
					face = manager->OpenFont (families[i], stretch, weight, style);
				
				if (face != NULL)
					g_ptr_array_add (faces, face);
			}
			
			g_hash_table_insert (loaded, families[i], GINT_TO_POINTER (true));
		}
		
		g_strfreev (families);
	} else if (source) {
		if ((face = manager->OpenFont (source, 0)))
			g_ptr_array_add (faces, face);
	}
	
	if (faces->len == 0 && !g_hash_table_lookup (loaded, "Portable User Interface"))
		LoadPortableUserInterface (manager, loaded, faces, stretch, weight, style);
	
	g_hash_table_destroy (loaded);
	
	if (faces->len == 0) {
		g_ptr_array_free (faces, true);
		return NULL;
	}
	
	font = new TextFont ((FontFace **) faces->pdata, faces->len, desc->GetSize ());
	g_ptr_array_free (faces, false);
	font->desc = desc;
	
	return font;
}

bool
TextFont::SetSize (double size)
{
	if (this->size == size)
		return false;
	
	ClearGlyphCache ();
	
	this->size = size;
	
	return true;
}

double
TextFont::GetSize ()
{
	return size;
}

bool
TextFont::SetStyleSimulations (StyleSimulations simulate)
{
	if (this->simulate == simulate)
		return false;
	
	ClearGlyphCache ();
	
	this->simulate = simulate;
	
	return true;
}

StyleSimulations
TextFont::GetStyleSimulations ()
{
	return simulate;
}

double
TextFont::Kerning (guint32 left, guint32 right)
{
	return 0.0;
}

double
TextFont::Descender ()
{
	return extents.descent;
}

double
TextFont::Ascender ()
{
	return extents.ascent;
}

double
TextFont::Height ()
{
	return extents.height;
}

static int
glyphsort (const void *v1, const void *v2)
{
	GlyphInfo *g1 = (GlyphInfo *) v1;
	GlyphInfo *g2 = (GlyphInfo *) v2;
	gint64 cmp = g2->atime - g1->atime;
	
	if ((cmp = g2->atime - g1->atime) < 0)
		return -1;
	
	return cmp > 0 ? 1 : 0;
}

GlyphInfo *
TextFont::GetGlyphInfo (FontFace *face, gunichar unichar, guint32 index)
{
	gint64 now = get_now ();
	GlyphInfo glyph, *slot;
	int i;
	
	for (i = 0; i < n_glyphs; i++) {
		if (glyphs[i].unichar == unichar) {
			slot = &glyphs[i];
			slot->atime = now;
			return slot;
		}
	}
	
	glyph.unichar = unichar;
	glyph.index = index;
	glyph.atime = now;
	glyph.path = NULL;
	
	if (desc != NULL) {
		// figure out what to similate
		simulate = StyleSimulationsNone;
		if (FontWeightIsBold (desc->GetWeight ()) && !face->IsBold ())
			simulate = (StyleSimulations) (simulate | StyleSimulationsBold);
		if (desc->GetStyle () == FontStylesItalic && !face->IsItalic ())
			simulate = (StyleSimulations) (simulate | StyleSimulationsItalic);
	}
	
	if (!face->LoadGlyph (size, &glyph, simulate))
		return NULL;
	
	if (n_glyphs == GLYPH_CACHE_SIZE) {
		// need to expire the least recently requested glyph (which will be the last element in the array after sorting)
		qsort (glyphs, n_glyphs, sizeof (GlyphInfo), glyphsort);
		
		for (i = 0; i < n_glyphs; i++)
			fprintf (stderr, "glyphs[%d].atime = %lld\n", i, glyphs[i].atime);
		
		slot = &glyphs[n_glyphs - 1];
		
		if (slot->path)
			moon_path_destroy (slot->path);
	} else {
		slot = &glyphs[n_glyphs++];
	}
	
	memcpy (slot, &glyph, sizeof (GlyphInfo));
	
	return slot;
}

//static GlyphInfo ZeroWidthNoBreakSpace = {
//	0xFEFF, 0, { 0.0, 0.0, 0.0, 0.0, 0.0 }, NULL, 0, 0
//};

GlyphInfo *
TextFont::GetGlyphInfo (gunichar unichar)
{
	FontFace *face = NULL;
	guint32 index;
	int i;
	
	//if (unichar == 0xFEFF)
	//	return &ZeroWidthNoBreakSpace;
	
	// find the face that contains this character
	for (i = 0; i < n_faces; i++) {
		if ((index = faces[i]->GetCharIndex (unichar)) != 0) {
			face = faces[i];
			break;
		}
	}
	
	if (face == NULL) {
		// draw the empty glyph from the primary face
		face = faces[0];
		index = 0;
	}
	
	return GetGlyphInfo (face, unichar, index);
}

GlyphInfo *
TextFont::GetGlyphInfoByIndex (guint32 index)
{
	gunichar unichar;
	
	unichar = faces[0]->GetCharFromIndex (index);
	
	return GetGlyphInfo (faces[0], unichar, index);
}

double
TextFont::UnderlinePosition ()
{
	return extents.underline_position;
}

double
TextFont::UnderlineThickness ()
{
	return extents.underline_thickness;
}

void
TextFont::Path (cairo_t *cr, GlyphInfo *glyph, double x, double y)
{
	if (!glyph->path || !(&glyph->path->cairo)->data)
		return;
	
	cairo_translate (cr, x, y);
	cairo_append_path (cr, &glyph->path->cairo);
	cairo_translate (cr, -x, -y);
}

void
TextFont::Path (cairo_t *cr, gunichar unichar, double x, double y)
{
	GlyphInfo *glyph;
	
	if (!(glyph = GetGlyphInfo (unichar)))
		return;
	
	Path (cr, glyph, x, y);
}

static void
moon_append_path_with_origin (moon_path *mpath, cairo_path_t *path, double x, double y)
{
	cairo_path_data_t *data;
	
	moon_move_to (mpath, x, y);
	
	for (int i = 0; i < path->num_data; i += path->data[i].header.length) {
		data = &path->data[i];
		
		switch (data->header.type) {
		case CAIRO_PATH_MOVE_TO:
			moon_move_to (mpath, data[1].point.x + x, data[1].point.y + y);
			break;
		case CAIRO_PATH_LINE_TO:
			moon_line_to (mpath, data[1].point.x + x, data[1].point.y + y);
			break;
		case CAIRO_PATH_CURVE_TO:
			moon_curve_to (mpath, data[1].point.x + x, data[1].point.y + y,
				       data[2].point.x + x, data[2].point.y + y,
				       data[3].point.x + x, data[3].point.y + y);
			break;
		case CAIRO_PATH_CLOSE_PATH:
			break;
		}
	}
}

void
TextFont::AppendPath (moon_path *path, GlyphInfo *glyph, double x, double y)
{
	if (!glyph->path || !(&glyph->path->cairo)->data)
		return;
	
	moon_append_path_with_origin (path, &glyph->path->cairo, x, y);
}

void
TextFont::AppendPath (moon_path *path, gunichar unichar, double x, double y)
{
	GlyphInfo *glyph;
	
	if (!(glyph = GetGlyphInfo (unichar)))
		return;
	
	AppendPath (path, glyph, x, y);
}


TextFontDescription::TextFontDescription ()
{
	changed = true;
	font = NULL;
	
	family = NULL;
	source = NULL;
	
	style = FontStylesNormal;
	weight = FontWeightsNormal;
	stretch = FontStretchesNormal;
	size = 14.666666984558105;
}

TextFontDescription::~TextFontDescription ()
{
	g_free (family);
	delete font;
}

void
TextFontDescription::Reload ()
{
	// load the new font first - trick to hit the cache for old FontFaces
	TextFont *ttf = TextFont::Load (this);
	changed = false;
	delete font;
	font = ttf;
}

TextFont *
TextFontDescription::GetFont ()
{
	if (changed)
		Reload ();
	
	return font;
}

const char *
TextFontDescription::GetSource () const
{
	return source;
}

bool
TextFontDescription::SetSource (const char *source)
{
	bool changed;
	
	if (source) {
		if (!this->source || g_ascii_strcasecmp (this->source, source) != 0) {
			g_free (this->source);
			this->source = g_strdup (source);
			this->changed = true;
			changed = true;
		} else {
			changed = false;
		}
	} else {
		if (this->source) {
			g_free (this->source);
			this->source = NULL;
			this->changed = true;
			changed = true;
		} else {
			changed = false;
		}
	}
	
	return changed;
}

char **
TextFontDescription::GetFamilies () const
{
	char **families;
	
	if (!family)
		return NULL;
	
	if ((families = g_strsplit (family, ",", -1))) {
		for (int i = 0; families[i]; i++)
			g_strstrip (families[i]);
	}
	
	return families;
}

const char *
TextFontDescription::GetFamily () const
{
	return family;
}

bool
TextFontDescription::SetFamily (const char *family)
{
	bool changed;
	
	if (family) {
		if (!this->family || g_ascii_strcasecmp (this->family, family) != 0) {
			g_free (this->family);
			this->family = g_strdup (family);
			this->changed = true;
			changed = true;
		} else {
			changed = false;
		}
	} else {
		if (this->family) {
			g_free (this->family);
			this->family = NULL;
			this->changed = true;
			changed = true;
		} else {
			changed = false;
		}
	}
	
	return changed;
}

FontStyles
TextFontDescription::GetStyle () const
{
	return style;
}

bool
TextFontDescription::SetStyle (FontStyles style)
{
	bool changed = this->style != style;
	
	if (changed) {
		this->style = style;
		this->changed = true;
	}
	
	return changed;
}

FontWeights
TextFontDescription::GetWeight () const
{
	return weight;
}

bool
TextFontDescription::SetWeight (FontWeights weight)
{
	bool changed = this->weight != weight;
	
	if (changed) {
		this->weight = weight;
		this->changed = true;
	}
	
	return changed;
}

FontStretches
TextFontDescription::GetStretch () const
{
	return stretch;
}

bool
TextFontDescription::SetStretch (FontStretches stretch)
{
	bool changed = this->stretch != stretch;
	
	if (changed) {
		this->stretch = stretch;
		this->changed = true;
	}
	
	return changed;
}

double
TextFontDescription::GetSize () const
{
	return size;
}

bool
TextFontDescription::SetSize (double size)
{
	bool changed = this->size != size;
	
	if (font)
		font->SetSize (size);
	
	this->size = size;
	
	return changed;
}
