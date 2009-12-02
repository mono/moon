/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fontmanager.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <glib/gstdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "fontmanager.h"
#include "zip/unzip.h"
#include "debug.h"
#include "utils.h"
#include "list.h"

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include FT_TRUETYPE_TABLES_H
#include FT_OUTLINE_H
#include FT_SYSTEM_H
#include FT_GLYPH_H


//
// OpenType's OS/2 fsSelection Table:
//
// http://www.microsoft.com/typography/otspec/os2.htm#fss
//
enum fsSelection {
	fsSelectionItalic         = (1 << 0),
	fsSelectionUnderscore     = (1 << 1),
	fsSelectionNegative       = (1 << 2),
	fsSelectionOutlined       = (1 << 3),
	fsSelectionStrikeout      = (1 << 4),
	fsSelectionBold           = (1 << 5),
	fsSelectionRegular        = (1 << 6),
	fsSelectionUseTypoMetrics = (1 << 7),
	fsSelectionWWS            = (1 << 8),
	fsSelectionOblique        = (1 << 9),
};

#define FONT_FACE_SIZE 41.0

#define DOUBLE_TO_26_6(d) ((FT_F26Dot6)((d) * 64.0))
#define DOUBLE_FROM_26_6(t) ((double)(t) / 64.0)
#define DOUBLE_TO_16_16(d) ((FT_Fixed)((d) * 65536.0))
#define DOUBLE_FROM_16_16(t) ((double)(t) / 65536.0)

#define EMBOLDEN_STRENGTH 0.75
#define EMBOLDEN_STRENGTH_26_6 DOUBLE_TO_26_6 (EMBOLDEN_STRENGTH)
#define EMBOLDEN_STRENGTH_16_16 DOUBLE_TO_16_16 (EMBOLDEN_STRENGTH)

#define ITALIC_SLANT -17.5
#define ITALIC_SLANT_RADIANS (ITALIC_SLANT * M_PI / 180.0)

static const FT_Matrix italicize = {
	DOUBLE_TO_16_16 (1.0), DOUBLE_TO_16_16 (tan (ITALIC_SLANT_RADIANS)),
	DOUBLE_TO_16_16 (0.0), DOUBLE_TO_16_16 (1.0)
};

#define LOAD_FLAGS (FT_LOAD_NO_BITMAP | /*FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT |*/ FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH | FT_LOAD_TARGET_NORMAL)



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

bool
FontWeightIsBold (FontWeights weight)
{
	if (weight > FONT_LOWER_BOLD_LIMIT)
		return weight < 0 || (weight >= FontWeightsSemiBold && weight < FONT_UPPER_BOLD_LIMIT);
	
	return false;
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


//
// Font-style parser utils
//

#define Width  (1 << 0)
#define Weight (1 << 1)
#define Slant  (1 << 2)

struct FontStyleInfo {
	char *family_name;
	FontStretches width;
	FontWeights weight;
	FontStyles slant;
	int set;
};

static struct {
	const char *name;
	size_t len;
	int type;
	int value;
} style_hints[] = {
	// widths
	{ "Ultra-Condensed", 15, Width,  FontStretchesUltraCondensed },
	{ "Extra-Condensed", 15, Width,  FontStretchesExtraCondensed },
	{ "Semi-Condensed",  14, Width,  FontStretchesSemiCondensed  },
	{ "UltraCondensed",  14, Width,  FontStretchesUltraCondensed },
	{ "ExtraCondensed",  14, Width,  FontStretchesExtraCondensed },
	{ "SemiCondensed",   13, Width,  FontStretchesSemiCondensed  },
	{ "Condensed",        9, Width,  FontStretchesCondensed      },
	{ "Cond",             4, Width,  FontStretchesCondensed      },
	{ "Ultra-Expanded",  14, Width,  FontStretchesUltraExpanded  },
	{ "Extra-Expanded",  14, Width,  FontStretchesExtraExpanded  },
	{ "Semi-Expanded",   13, Width,  FontStretchesSemiExpanded   },
	{ "UltraExpanded",   13, Width,  FontStretchesUltraExpanded  },
	{ "ExtraExpanded",   13, Width,  FontStretchesExtraExpanded  },
	{ "SemiExpanded",    12, Width,  FontStretchesSemiExpanded   },
	{ "Expanded",         8, Width,  FontStretchesExpanded       },
	
	// weights
	{ "Thin",             4, Weight, FontWeightsThin             },
	{ "Ultra-Light",     11, Weight, FontWeightsExtraLight       },
	{ "Extra-Light",     11, Weight, FontWeightsExtraLight       },
	{ "UltraLight",      10, Weight, FontWeightsExtraLight       },
	{ "ExtraLight",      10, Weight, FontWeightsExtraLight       },
	{ "Light",            5, Weight, FontWeightsLight            },
	{ "Book",             4, Weight, FontWeightsNormal           },
	{ "Medium",           6, Weight, FontWeightsMedium           },
	{ "Demi-Bold",        9, Weight, FontWeightsSemiBold         },
	{ "Semi-Bold",        9, Weight, FontWeightsSemiBold         },
	{ "DemiBold",         8, Weight, FontWeightsSemiBold         },
	{ "SemiBold",         8, Weight, FontWeightsSemiBold         },
	{ "Bold",             4, Weight, FontWeightsBold             },
	{ "Extra-Bold",      10, Weight, FontWeightsExtraBold        },
	{ "Ultra-Bold",      10, Weight, FontWeightsExtraBold        },
	{ "ExtraBold",        9, Weight, FontWeightsExtraBold        },
	{ "UltraBold",        9, Weight, FontWeightsExtraBold        },
	{ "Black",            5, Weight, FontWeightsBlack            },
	{ "Heavy",            5, Weight, FontWeightsBlack            },
	{ "Extra-Black",     11, Weight, FontWeightsExtraBlack       },
	{ "Ultra-Black",     11, Weight, FontWeightsExtraBlack       },
	{ "ExtraBlack",      10, Weight, FontWeightsExtraBlack       },
	{ "UltraBlack",      10, Weight, FontWeightsExtraBlack       },
	
	// slants
	{ "Oblique",          7, Slant,  FontStylesOblique           },
	{ "Italic",           6, Slant,  FontStylesItalic            },
	{ "Kursiv",           6, Slant,  FontStylesItalic            },
	
	// changes nothing
	{ "Regular",          7, 0,      0                           },
	{ "W3",               2, 0,      0                           },  // as in Hiragino Mincho Pro W3
};

static void
style_info_parse (const char *style, FontStyleInfo *info, bool family)
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
				case Width:
					info->width = (FontStretches) style_hints[i].value;
					info->set |= Width;
					break;
				case Weight:
					info->weight = (FontWeights) style_hints[i].value;
					info->set |= Weight;
					break;
				case Slant:
					info->slant = (FontStyles) style_hints[i].value;
					info->set |= Slant;
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
			info->width = FontStretchesNormal;
			info->weight = FontWeightsNormal;
			info->slant = FontStylesNormal;
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

#ifdef LOGGING
static const char *
style_info_to_string (FontStretches stretch, FontWeights weight, FontStyles style)
{
	static char namebuf[256];
	guint i = 0;
	char *p;
	
	namebuf[0] = '\0';
	p = namebuf;
	
	if (stretch != FontStretchesNormal) {
		while (style_hints[i].type == Width) {
			if (style_hints[i].value == stretch) {
				p = g_stpcpy (p, style_hints[i].name);
				break;
			}
			
			i++;
		}
	}
	
	if (weight != FontWeightsNormal) {
		while (style_hints[i].type != Weight)
			i++;
		
		while (style_hints[i].type == Weight) {
			if (style_hints[i].value == weight) {
				if (p != namebuf)
					*p++ = ' ';
				
				p = g_stpcpy (p, style_hints[i].name);
				break;
			}
			
			i++;
		}
	}
	
	if (style != FontStylesNormal) {
		while (style_hints[i].type != Slant)
			i++;
		
		while (i < G_N_ELEMENTS (style_hints)) {
			if (style_hints[i].value == style) {
				if (p != namebuf)
					*p++ = ' ';
				
				p = g_stpcpy (p, style_hints[i].name);
				break;
			}
			
			i++;
		}
	}
	
	return namebuf;
}
#endif

static bool
is_odttf (const char *name)
{
	size_t len = strlen (name);
	
	if (len > 6 && !g_ascii_strcasecmp (name + len - 6, ".odttf"))
		return true;
	
	return false;
}


//
// FontStream
//

struct FontStream {
	bool obfuscated;
	char guid[16];
	FILE *fp;
};

static const char *
path_get_basename (const char *path)
{
	const char *name;
	
	if (!(name = strrchr (path, '/')))
		return path;
	
	return name + 1;
}

static bool
decode_guid (const char *in, char *guid)
{
	const char *inptr = in;
	int i = 16;
	
	while (i > 0 && *inptr && *inptr != '.') {
		if (*inptr == '-')
			inptr++;
		
		i--;
		
		if (*inptr >= '0' && *inptr <= '9')
			guid[i] = (*inptr - '0') * 16;
		else if (*inptr >= 'a' && *inptr <= 'f')
			guid[i] = ((*inptr - 'a') + 10) * 16;
		else if (*inptr >= 'A' && *inptr <= 'F')
			guid[i] = ((*inptr - 'A') + 10) * 16;
		else
			return false;
		
		inptr++;
		
		if (*inptr >= '0' && *inptr <= '9')
			guid[i] += (*inptr - '0');
		else if (*inptr >= 'a' && *inptr <= 'f')
			guid[i] += ((*inptr - 'a') + 10);
		else if (*inptr >= 'A' && *inptr <= 'F')
			guid[i] += ((*inptr - 'A') + 10);
		else
			return false;
		
		inptr++;
	}
	
	if (i > 0)
		return false;
	
	return true;
}

static bool
font_stream_set_guid (FT_Stream stream, const char *guid)
{
	FontStream *fs = (FontStream *) stream->descriptor.pointer;
	
	if (guid) {
		fs->obfuscated = decode_guid (guid, fs->guid);
	} else {
		fs->obfuscated = false;
	}
	
	return fs->obfuscated;
}

static unsigned long
font_stream_read (FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	FontStream *fs = (FontStream *) stream->descriptor.pointer;
	size_t nread;
	
	if (fseek (fs->fp, (long) offset, SEEK_SET) == -1)
		return 0;
	
	if (count == 0 || buffer == NULL)
		return 0;
	
	nread = fread (buffer, 1, count, fs->fp);
	
	if (fs->obfuscated && offset < 32 && nread > 0) {
		/* obfuscated font... need to deobfuscate */
		unsigned long i = offset;
		unsigned long j = 0;
		
		for ( ; i < 32 && j < nread; i++, j++)
			buffer[j] = buffer[j] ^ fs->guid[i % 16];
	}
	
	return nread;
}

static void
font_stream_reset (FT_Stream stream)
{
	FontStream *fs = (FontStream *) stream->descriptor.pointer;
	
	rewind (fs->fp);
	stream->pos = 0;
}

static void
font_stream_close (FT_Stream stream)
{
	// no-op
}

static FT_Stream
font_stream_new (const char *filename, const char *guid)
{
	FT_Stream stream;
	FontStream *fs;
	FILE *fp;
	
	if (!(fp = fopen (filename, "r")))
		return NULL;
	
	fs = (FontStream *) g_malloc (sizeof (FontStream));
	fs->obfuscated = false;
	fs->fp = fp;
	
	stream = (FT_Stream) g_malloc0 (sizeof (FT_StreamRec));
	stream->close = font_stream_close;
	stream->read = font_stream_read;
	stream->descriptor.pointer = fs;
	
	fseek (fp, 0, SEEK_END);
	stream->size = ftell (fp);
	fseek (fp, 0, SEEK_SET);
	
	font_stream_set_guid (stream, guid);
	
	return stream;
}

static void
font_stream_destroy (FT_Stream stream)
{
	FontStream *fs = (FontStream *) stream->descriptor.pointer;
	
	g_free (stream);
	fclose (fs->fp);
	g_free (fs);
}


//
// FaceInfo
//

struct FontFile : public List::Node {
	GPtrArray *faces;
	char *path, *guid;
	
	FontFile (const char *path, const char *guid);
	~FontFile ();
};

struct FaceInfo {
	FontStyleInfo style;
	char *family_name;
	FontFile *file;
	int index;
	
	FaceInfo (FontFile *file, FT_Face face, int index);
	~FaceInfo ();
};

FaceInfo::FaceInfo (FontFile *file, FT_Face face, int index)
{
	LOG_FONT (stderr, "      * indexing %s[%d]: family=\"%s\"; style=\"%s\"\n",
		  path_get_basename (file->path), index, face->family_name, face->style_name);
	
	style.width = FontStretchesNormal;
	style.weight = FontWeightsNormal;
	style.slant = FontStylesNormal;
	style.family_name = NULL;
	style.set = 0;
	
	// extract whatever little style info we can from the family name
	style_info_parse (face->family_name, &style, true);
	
	// style info parsed from style_name overrides anything we got from family_name
	style_info_parse (face->style_name, &style, false);
	
	family_name = style.family_name;
	
	LOG_FONT (stderr, "        * indexed as %s; %s\n", family_name,
		  style_info_to_string (style.width, style.weight, style.slant));
	
	this->index = index;
	this->file = file;
}

FaceInfo::~FaceInfo ()
{
	g_free (family_name);
}


//
// FontFile
//

FontFile::FontFile (const char *path, const char *guid)
{
	this->path = g_strdup (path);
	this->guid = g_strdup (guid);
	faces = NULL;
}

FontFile::~FontFile ()
{
	if (faces != NULL) {
		FaceInfo *face;
		
		for (guint i = 0; i < faces->len; i++) {
			face = (FaceInfo *) faces->pdata[i];
			delete face;
		}
		
		g_ptr_array_free (faces, true);
	}
	
	g_free (path);
	g_free (guid);
}


//
// FontIndex
//

struct FontIndex {
	List *fonts;
	char *name;
	char *path;
	
	FontIndex (const char *name);
	~FontIndex ();
	
	void CacheFontInfo (FT_Library libft2, const char *filename, FT_Stream stream, FT_Face face, const char *guid);
};

FontIndex::FontIndex (const char *name)
{
	this->name = g_strdup (name);
	fonts = new List ();
	path = NULL;
}

FontIndex::~FontIndex ()
{
	g_free (name);
	g_free (path);
	delete fonts;
}

void
FontIndex::CacheFontInfo (FT_Library libft2, const char *filename, FT_Stream stream, FT_Face face, const char *guid)
{
	int i = 0, nfaces = face->num_faces;
	FT_Open_Args args;
	FontFile *file;
	FaceInfo *fi;
	
	LOG_FONT (stderr, "    * caching font info for `%s'...\n", filename);
	
	file = new FontFile (filename, guid);
	file->faces = g_ptr_array_new ();
	
	do {
		args.flags = FT_OPEN_STREAM;
		args.stream = stream;
		
		if (i > 0 && FT_Open_Face (libft2, &args, i, &face) != 0)
			break;
		
		fi = new FaceInfo (file, face, i);
		g_ptr_array_add (file->faces, fi);
		
		FT_Done_Face (face);
		
		font_stream_reset (stream);
		
		i++;
	} while (i < nfaces);
	
	fonts->Append (file);
}

static void
font_index_destroy (gpointer user_data)
{
	delete ((FontIndex *) user_data);
}


//
// FontFace
//

FontFace::FontFace (FontManager *manager, FT_Face face, char *key)
{
	FT_Set_Pixel_Sizes (face, 0, (int) FONT_FACE_SIZE);
	this->cur_size = FONT_FACE_SIZE;
	this->manager = manager;
	this->ref_count = 1;
	this->face = face;
	this->key = key;
	
	g_hash_table_insert (manager->faces, key, this);
}

FontFace::~FontFace ()
{
	FT_Stream stream;
	
	g_hash_table_steal (manager->faces, key);
	
	stream = face->stream;
	FT_Done_Face (face);
	font_stream_destroy (stream);
	g_free (key);
}

void
FontFace::ref ()
{
	ref_count++;
}

void
FontFace::unref ()
{
	ref_count--;
	
	if (ref_count == 0)
		delete this;
}

const char *
FontFace::GetFamilyName ()
{
	return face->family_name;
}

const char *
FontFace::GetStyleName ()
{
	return face->style_name;
}

bool
FontFace::IsScalable ()
{
	return FT_IS_SCALABLE (face);
}

bool
FontFace::IsItalic ()
{
	return (face->style_flags & FT_STYLE_FLAG_ITALIC);
}

bool
FontFace::IsBold ()
{
	return (face->style_flags & FT_STYLE_FLAG_BOLD);
}

gunichar
FontFace::GetCharFromIndex (guint32 index)
{
	gunichar unichar;
	guint32 idx;
	
	if (index == 0)
		return 0;
	
	unichar = FT_Get_First_Char (face, &idx);
	while (idx != index && idx != 0)
		unichar = FT_Get_Next_Char (face, unichar, &idx);
	
	if (idx == 0)
		unichar = 0;
	
	return unichar;
}

guint32
FontFace::GetCharIndex (gunichar unichar)
{
	return FcFreeTypeCharIndex (face, unichar);
}

bool
FontFace::HasChar (gunichar unichar)
{
	return FcFreeTypeCharIndex (face, unichar) != 0;
}

void
FontFace::GetExtents (double size, bool gapless, FontFaceExtents *extents)
{
	double scale = size / face->units_per_EM;
	
	if (FT_IS_SFNT (face)) {
		TT_HoriHeader *hhea = (TT_HoriHeader *) FT_Get_Sfnt_Table (face, ft_sfnt_hhea);
		TT_OS2 *os2 = (TT_OS2 *) FT_Get_Sfnt_Table (face, ft_sfnt_os2);
		int height, ascender, descender;
		
		if (os2 && (os2->fsSelection & fsSelectionUseTypoMetrics)) {
			// Use the typographic Ascender, Descender, and LineGap values for everything.
			height = os2->sTypoAscender - os2->sTypoDescender;
			if (!gapless)
				height += os2->sTypoLineGap;
			
			descender = -os2->sTypoDescender;
			ascender = os2->sTypoAscender;
		} else {
			// Calculate the LineSpacing for both the hhea table and the OS/2 table.
			int hhea_height = hhea->Ascender + abs (hhea->Descender) + hhea->Line_Gap;
			int os2_height = os2 ? (os2->usWinAscent + os2->usWinDescent) : 0;
			
			// The LineSpacing is the maximum of the two sumations.
			height = MAX (hhea_height, os2_height);
			
			if (gapless && os2) {
				// Subtract the OS/2 typographic LineGap (not the hhea LineGap)
				height -= os2->sTypoLineGap;
			}
			
			// If the OS/2 table exists, use usWinAscent as the
			// ascender. Otherwise use hhea's Ascender value.
			ascender = os2 ? os2->usWinAscent : hhea->Ascender;
			
			// The Descender becomes the difference between the
			// LineSpacing and the Ascender.
			descender = height - ascender;
		}
		
		extents->descent = -descender * scale;
		extents->ascent = ascender * scale;
		extents->height = height * scale;
	} else {
		// Fall back to the default FreeType2 values.
		extents->descent = face->descender * scale;
		extents->ascent = face->ascender * scale;
		extents->height = face->height * scale;
	}
	
	extents->underline_thickness = face->underline_thickness * scale;
	extents->underline_position = -face->underline_position * scale;
	extents->underline_position += ((extents->underline_thickness + 1) / 2.0);
	
	if (extents->underline_thickness < 1.0)
		extents->underline_thickness = 1.0;
}

double
FontFace::Kerning (double size, guint32 left, guint32 right)
{
	FT_Vector kerning;
	
	if (!FT_HAS_KERNING (face) || left == 0 || right == 0)
		return 0.0;
	
	if (size <= FONT_FACE_SIZE) {
		if (cur_size != FONT_FACE_SIZE) {
			FT_Set_Pixel_Sizes (face, 0, (int) FONT_FACE_SIZE);
			cur_size = FONT_FACE_SIZE;
		}
		
		FT_Get_Kerning (face, left, right, FT_KERNING_DEFAULT, &kerning);
		
		return (kerning.x * size) / (FONT_FACE_SIZE * 64.0);
	} else {
		if (cur_size != size) {
			FT_Set_Pixel_Sizes (face, 0, (int) size);
			cur_size = size;
		}
		
		FT_Get_Kerning (face, left, right, FT_KERNING_DEFAULT, &kerning);
		
		return kerning.x / 64.0;
	}
}

static int
font_move_to (FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x, y;
	
	x = DOUBLE_FROM_26_6 (to->x);
	y = DOUBLE_FROM_26_6 (to->y);
	
	moon_move_to (path, x, y);
	
	return 0;
}

static int
font_line_to (FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x, y;
	
	x = DOUBLE_FROM_26_6 (to->x);
	y = DOUBLE_FROM_26_6 (to->y);
	
	moon_line_to (path, x, y);
	
	return 0;
}

static int
font_conic_to (FT_Vector *control, FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x3, y3;
	double x, y;
	
	x = DOUBLE_FROM_26_6 (control->x);
	y = DOUBLE_FROM_26_6 (control->y);
	
	x3 = DOUBLE_FROM_26_6 (to->x);
	y3 = DOUBLE_FROM_26_6 (to->y);
	
	moon_quad_curve_to (path, x, y, x3, y3);
	
	return 0;
}

static int
font_cubic_to (FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *user_data)
{
	moon_path *path = (moon_path *) user_data;
	double x0, y0;
	double x1, y1;
	double x2, y2;
	
	x0 = DOUBLE_FROM_26_6 (control1->x);
	y0 = DOUBLE_FROM_26_6 (control1->y);
	
	x1 = DOUBLE_FROM_26_6 (control2->x);
	y1 = DOUBLE_FROM_26_6 (control2->y);
	
	x2 = DOUBLE_FROM_26_6 (to->x);
	y2 = DOUBLE_FROM_26_6 (to->y);
	
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

bool
FontFace::LoadGlyph (double size, GlyphInfo *glyph, StyleSimulations simulate)
{
	FT_Glyph_Metrics *metrics;
	FT_Fixed hori_adj = 0;
	FT_Pos bbox_adj = 0;
	FT_Matrix matrix;
	double scale;
	
	if (!face)
		return false;
	
	if (size <= FONT_FACE_SIZE) {
		if (cur_size != FONT_FACE_SIZE) {
			FT_Set_Pixel_Sizes (face, 0, (int) FONT_FACE_SIZE);
			cur_size = FONT_FACE_SIZE;
		}
		
		scale = size / FONT_FACE_SIZE;
	} else {
		if (cur_size != size) {
			FT_Set_Pixel_Sizes (face, 0, (int) size);
			cur_size = size;
		}
		
		scale = 1.0;
	}
	
	if (FT_Load_Glyph (face, glyph->index, LOAD_FLAGS) != 0)
		return false;
	
	if (FT_Render_Glyph (face->glyph, FT_RENDER_MODE_NORMAL) != 0)
		return false;
	
	// invert the glyph over the y-axis and scale
	matrix.xx = DOUBLE_TO_16_16 (scale);
	matrix.xy = 0;
	matrix.yx = 0;
	matrix.yy = -DOUBLE_TO_16_16 (scale);
	
	if ((simulate & StyleSimulationsBold) != 0) {
		FT_Outline_Embolden (&face->glyph->outline, EMBOLDEN_STRENGTH_26_6);
		hori_adj = EMBOLDEN_STRENGTH_16_16;
		bbox_adj = EMBOLDEN_STRENGTH_26_6;
	}
	
	if ((simulate & StyleSimulationsItalic) != 0)
		FT_Matrix_Multiply (&italicize, &matrix);
	
	glyph->path = moon_path_new (8);
	FT_Outline_Transform (&face->glyph->outline, &matrix);
	FT_Outline_Decompose (&face->glyph->outline, &outline_funcs, glyph->path);
	
	metrics = &face->glyph->metrics;
	
	glyph->metrics.horiBearingX = DOUBLE_FROM_26_6 (metrics->horiBearingX) * scale;
	//glyph->metrics.horiBearingY = DOUBLE_FROM_26_6 (metrics->horiBearingY) * scale;
	// always prefer linearHoriAdvance over horiAdvance since the later is rounded to an integer
	glyph->metrics.horiAdvance = DOUBLE_FROM_16_16 (face->glyph->linearHoriAdvance + hori_adj) * scale;
	//glyph->metrics.height = DOUBLE_FROM_26_6 (metrics->height + bbox_adj) * scale;
	//glyph->metrics.width = DOUBLE_FROM_26_6 (metrics->width + bbox_adj) * scale;
	
	return true;
}

static void
font_face_destroy (gpointer data)
{
	FontFace *face = (FontFace *) data;
	
	delete face;
}


//
// FontManager
//

FontManager::FontManager ()
{
	FcPattern *pattern;
	
	resources = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, font_index_destroy);
	faces = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, font_face_destroy);
	system_faces = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	
	FT_Init_FreeType (&libft2);
	
	pattern = FcPatternBuild (NULL, FC_FAMILY, FcTypeString, "Sans",
				  FC_SIZE, FcTypeDouble, 10.0, NULL);
	
	if (FcPatternGetDouble (pattern, FC_DPI, 0, &dpi) != FcResultMatch)
		dpi = 72.0;
	
	FcPatternDestroy (pattern);
	root = NULL;
}

FontManager::~FontManager ()
{
	g_hash_table_destroy (system_faces);
	g_hash_table_destroy (resources);
	g_hash_table_destroy (faces);
	FT_Done_FreeType (libft2);
	
	if (root) {
		RemoveDir (root);
		g_free (root);
	}
}

static bool
IndexFontSubdirectory (FT_Library libft2, const char *name, GString *path, FontIndex **out)
{
	FontIndex *fontdir = *out;
	const gchar *dirname;
	FT_Open_Args args;
	FT_Stream stream;
	bool obfuscated;
	struct stat st;
	FT_Face face;
	size_t len;
	GDir *dir;
	
	if (!(dir = g_dir_open (path->str, 0, NULL)))
		return fontdir != NULL;
	
	LOG_FONT (stderr, "  * indexing font directory `%s'...\n", path->str);
	
	g_string_append_c (path, G_DIR_SEPARATOR);
	len = path->len;
	
	while ((dirname = g_dir_read_name (dir))) {
		if (!strcmp (dirname, "..") ||
		    !strcmp (dirname, "."))
			continue;
		
		g_string_append (path, dirname);
		
		if (g_stat (path->str, &st) == -1)
			goto next;
		
		if (S_ISDIR (st.st_mode)) {
			IndexFontSubdirectory (libft2, name, path, &fontdir);
			goto next;
		}
		
		if (!(stream = font_stream_new (path->str, NULL)))
			goto next;
		
		args.flags = FT_OPEN_STREAM;
		args.stream = stream;
		
		obfuscated = false;
		
		if (FT_Open_Face (libft2, &args, 0, &face) != 0) {
			// not a valid font file... is it maybe an obfuscated font?
			if (!is_odttf (dirname) || !font_stream_set_guid (stream, dirname)) {
				font_stream_destroy (stream);
				goto next;
			}
			
			font_stream_reset (stream);
			
			args.flags = FT_OPEN_STREAM;
			args.stream = stream;
			
			if (FT_Open_Face (libft2, &args, 0, &face) != 0) {
				font_stream_destroy (stream);
				goto next;
			}
			
			obfuscated = true;
		}
		
		if (fontdir == NULL)
			fontdir = new FontIndex (name);
		
		// cache font info
		fontdir->CacheFontInfo (libft2, path->str, stream, face, obfuscated ? dirname : NULL);
		
		font_stream_destroy (stream);
		
	 next:
		g_string_truncate (path, len);
	}
	
	g_dir_close (dir);
	
	*out = fontdir;
	
	return fontdir != NULL;
}

static FontIndex *
IndexFontDirectory (FT_Library libft2, const char *name, const char *dirname)
{
	FontIndex *fontdir = NULL;
	GString *path;
	size_t len;
	
	path = g_string_new (dirname);
	len = path->len;
	
	if (!IndexFontSubdirectory (libft2, name, path, &fontdir)) {
		g_string_free (path, true);
		return NULL;
	}
	
	g_string_truncate (path, len);
	fontdir->path = path->str;
	
	g_string_free (path, false);
	
	return fontdir;
}

static FontIndex *
IndexFontFile (FT_Library libft2, const char *name, const char *path)
{
	const char *filename = path_get_basename (name);
	FontIndex *index = NULL;
	FT_Open_Args args;
	FT_Stream stream;
	bool obfuscated;
	FT_Face face;
	
	LOG_FONT (stderr, "  * indexing font file `%s'...\n", path);
	
	if (!(stream = font_stream_new (path, NULL)))
		return NULL;
	
	args.flags = FT_OPEN_STREAM;
	args.stream = stream;
	
	obfuscated = false;
	
	if (FT_Open_Face (libft2, &args, 0, &face) != 0) {
		// not a valid font file... is it maybe an obfuscated font?
		if (!is_odttf (filename) || !font_stream_set_guid (stream, filename)) {
			font_stream_destroy (stream);
			return NULL;
		}
		
		font_stream_reset (stream);
		
		args.flags = FT_OPEN_STREAM;
		args.stream = stream;
		
		if (FT_Open_Face (libft2, &args, 0, &face) != 0) {
			font_stream_destroy (stream);
			return NULL;
		}
		
		obfuscated = true;
	}
	
	index = new FontIndex (name);
	index->path = g_strdup (path);
	
	// cache font info
	index->CacheFontInfo (libft2, path, stream, face, obfuscated ? filename : NULL);
	
	font_stream_destroy (stream);
	
	return index;
}

void
FontManager::AddResource (const char *resource, const char *path)
{
	FontIndex *index;
	struct stat st;
	
	LOG_FONT (stderr, "Adding font resource '%s' at %s\n", resource, path);
	
	if ((index = (FontIndex *) g_hash_table_lookup (resources, resource)))
		return;
	
	if (stat (path, &st) == -1)
		return;
	
	if (S_ISDIR (st.st_mode))
		index = IndexFontDirectory (libft2, resource, path);
	else if (S_ISREG (st.st_mode))
		index = IndexFontFile (libft2, resource, path);
	else
		return;
	
	if (index)
		g_hash_table_insert (resources, index->name, index);
}

char *
FontManager::AddResource (ManagedStreamCallbacks *stream)
{
	char buf[4096], *resource, *dirname, *path;
	unzFile zipfile;
	int nread, fd;
	gint64 pos;
	
	if (!stream->CanRead (stream->handle))
		return NULL;
	
	if (!root && !(root = CreateTempDir ("moonlight-fonts")))
		return NULL;
	
	// check if we've already added this resource
	resource = g_strdup_printf ("font-source://%p", stream->handle);
	if (g_hash_table_lookup (resources, resource) != NULL)
		return resource;
	
	snprintf (buf, sizeof (buf), "%p", stream->handle);
	path = g_build_filename (root, buf, NULL);
	
	if ((fd = g_open (path, O_CREAT | O_EXCL | O_WRONLY, 0600)) == -1) {
		g_free (resource);
		g_free (path);
		return NULL;
	}
	
	// write the managed stream to disk
	pos = stream->Position (stream->handle);

	if (stream->CanSeek (stream->handle))
		stream->Seek (stream->handle, 0, SEEK_SET);

	while ((nread = stream->Read (stream->handle, buf, 0, sizeof (buf))) > 0) {
		if (write_all (fd, buf, (size_t) nread) == -1) {
			g_free (resource);
			close (fd);
			g_unlink (path);
			g_free (path);
			return NULL;
		}
	}
	
	// reset the stream to the original state
	if (stream->CanSeek (stream->handle) && pos != -1)
		stream->Seek (stream->handle, pos, SEEK_SET);
	
	close (fd);
	
	// check to see if the resource is zipped
	if ((zipfile = unzOpen (path))) {
		snprintf (buf, sizeof (buf), "%p.zip", stream->handle);
		dirname = g_build_filename (root, buf, NULL);
		
		// create a directory to contain our unzipped content
		if (g_mkdir (dirname, 0700) == -1) {
			unzClose (zipfile);
			g_free (resource);
			g_free (dirname);
			g_unlink (path);
			g_free (path);
			return NULL;
		}
		
		// unzip the contents
		if (!ExtractAll (zipfile, dirname, CanonModeNone)) {
			RemoveDir (dirname);
			unzClose (zipfile);
			g_free (resource);
			g_free (dirname);
			g_unlink (path);
			g_free (path);
			return NULL;
		}
		
		unzClose (zipfile);
		g_unlink (path);
		g_free (path);
		
		path = dirname;
	}
	
	AddResource (resource, path);
	
	g_free (path);
	
	return resource;
}

static int
style_diff (FontStyleInfo *actual, FontStyleInfo *desired)
{
#if 0
	// we convert to FontConfig for 2 reasons:
	// 1. negative values and values > 1023
	// 2. smaller ranges
	int weight = abs (fc_weight (actual->weight) - fc_weight (desired->weight));
	
	if (actual->slant == desired->slant)
		return weight;
	
	if (actual->slant == FontStylesNormal) {
		// we can emulate italic/oblique, but we would still prefer the real
		// italic font if we can find it so apply a slight penalty
		return 1000 + weight;
	}
	
	// ouch, apply a huge penalty
	return 1000000 + weight;
#else
	// convert to FontConfig values so that each style property fits within 8 bits
	int weight = abs (fc_weight (actual->weight) - fc_weight (desired->weight));
	int width = abs (fc_width (actual->width) - fc_width (desired->width));
	int slant = abs (fc_slant (actual->slant) - fc_slant (desired->slant));
	
	// weight has the highest priority, followed by weight and then slant
	return ((width & 0xff) << 16) | ((weight & 0xff) << 8) | (slant & 0xff);
#endif
}

static void
canon_font_family_and_style (FontStyleInfo *desired, const char *family, FontStretches stretch, FontWeights weight, FontStyles style)
{
	desired->width = FontStretchesNormal;
	desired->weight = FontWeightsNormal;
	desired->slant = FontStylesNormal;
	desired->family_name = NULL;
	desired->set = 0;
	
	// extract whatever little style info we can from the family name
	style_info_parse (family, desired, true);
	
	// override style with user-specified attributes
	if (!(desired->set & Width))
		desired->width = stretch;
	if (!(desired->set & Weight))
		desired->weight = weight;
	if (!(desired->set & Slant))
		desired->slant = style;
}

static FaceInfo *
IndexMatchFace (FontIndex *index, const char *family, FontStretches stretch, FontWeights weight, FontStyles style)
{
	FontFile *file = (FontFile *) index->fonts->First ();
	FaceInfo *face, *best = NULL;
	FontStyleInfo desired;
	int closest = G_MAXINT;
	int diff;
	guint i;
	
	LOG_FONT (stderr, "  * searching index for %s; %s\n", family, style_info_to_string (stretch, weight, style));
	
	canon_font_family_and_style (&desired, family, stretch, weight, style);
	
	LOG_FONT (stderr, "    * canonicalized family/style: %s; %s\n", desired.family_name,
		  style_info_to_string (desired.width, desired.weight, desired.slant));
	
	while (file != NULL) {
		for (i = 0; i < file->faces->len; i++) {
			face = (FaceInfo *) file->faces->pdata[i];
			
			if (!g_ascii_strcasecmp (face->family_name, desired.family_name)) {
				diff = style_diff (&face->style, &desired);
				if (diff < closest) {
					closest = diff;
					best = face;
				}
			}
		}
		
		file = (FontFile *) file->next;
	}
	
	g_free (desired.family_name);
	
	return best;
}

FontFace *
FontManager::OpenFontFace (const char *filename, const char *guid, int index)
{
	FT_Open_Args args;
	FT_Stream stream;
	FontFace *ff;
	FT_Face face;
	char *key;
	
	key = g_strdup_printf ("%s#%d", filename, index);
	if ((ff = (FontFace *) g_hash_table_lookup (faces, key))) {
		g_free (key);
		ff->ref ();
		return ff;
	}
	
	if (!(stream = font_stream_new (filename, guid))) {
		g_free (key);
		return NULL;
	}
	
	args.flags = FT_OPEN_STREAM;
	args.stream = stream;
	
	if (FT_Open_Face (libft2, &args, index, &face) != 0) {
		font_stream_destroy (stream);
		g_free (key);
		return NULL;
	}
	
	if (!FT_IS_SCALABLE (face)) {
		FT_Done_Face (face);
		font_stream_destroy (stream);
		g_free (key);
		return NULL;
	}
	
	return new FontFace (this, face, key);
}

FontFace *
FontManager::OpenFontResource (const char *resource, const char *family, int idx, FontStretches stretch, FontWeights weight, FontStyles style)
{
	FontIndex *index;
	FontFile *file;
	FontFace *face;
	FaceInfo *fi;
	
	LOG_FONT (stderr, "OpenFontResource (\"%s\", \"%s\", %d, %s)\n", resource ? resource : "(null)",
		  family ? family : "(null)", idx, style_info_to_string (stretch, weight, style));
	
	if (!(index = (FontIndex *) g_hash_table_lookup (resources, resource))) {
		LOG_FONT (stderr, "  * error: no such resource\n");
		return NULL;
	}
	
	if (family != NULL) {
		// open by family
		if (!(fi = IndexMatchFace (index, family, stretch, weight, style))) {
			LOG_FONT (stderr, "  * error: resource does not contain requested font\n");
			return NULL;
		}
	} else if (idx >= 0) {
		// open by index
		if (!(file = (FontFile *) index->fonts->First ()) || file->next != NULL)
			return NULL;
		
		if ((int) file->faces->len <= idx)
			return NULL;
		
		fi = (FaceInfo *) file->faces->pdata[idx];
	} else {
		// no family or index specified... error?
		return NULL;
	}
	
	if (!(face = OpenFontFace (fi->file->path, fi->file->guid, fi->index)))
		return NULL;
	
	LOG_FONT (stderr, "  * opened %s; %s\n", face->GetFamilyName (), face->GetStyleName ());
	
	return face;
}

FontFace *
FontManager::OpenSystemFont (const char *family, FontStretches stretch, FontWeights weight, FontStyles style)
{
	FcPattern *pattern, *matched;
	FontStyleInfo desired;
	FcChar8 *filename;
	FcResult result;
	FontFace *face;
	int index;
	char *key;
	
	key = g_strdup_printf ("%s:%d:%d:%d", family, stretch, weight, style);
	LOG_FONT (stderr, "Attempting to open system font: %s %s ... ", family, style_info_to_string (stretch, weight, style));
	if (g_hash_table_lookup_extended (system_faces, key, NULL, (gpointer *) &face)) {
		LOG_FONT (stderr, "found!\n");
		g_free (key);
		if (face)
			face->ref ();
		return face;
	}
	LOG_FONT (stderr, "not found in cache.\n");
	
	for (int attempt = 0; attempt < 2; attempt++) {
		if (attempt == 0) {
			desired.family_name = g_strdup (family);
			desired.width = stretch;
			desired.weight = weight;
			desired.slant = style;
		} else {
			g_free (desired.family_name);
			canon_font_family_and_style (&desired, family, stretch, weight, style);
		}
		
		LOG_FONT (stderr, "Attempting to load installed font: %s %s... ", desired.family_name,
			  style_info_to_string (desired.width, desired.weight, desired.slant));
		
		pattern = FcPatternCreate ();
		FcPatternAddDouble (pattern, FC_DPI, dpi);
		FcPatternAddString (pattern, FC_FAMILY, (const FcChar8 *) desired.family_name);
		FcPatternAddInteger (pattern, FC_WIDTH, fc_width (desired.width));
		FcPatternAddInteger (pattern, FC_WEIGHT, fc_weight (desired.weight));
		FcPatternAddInteger (pattern, FC_SLANT, fc_slant (desired.slant));
		FcDefaultSubstitute (pattern);
		
		if (!(matched = FcFontMatch (NULL, pattern, &result))) {
			LOG_FONT (stderr, "no matches\n");
			FcPatternDestroy (pattern);
			continue;
		}
		
		FcPatternDestroy (pattern);
		
		if (FcPatternGetString (matched, FC_FILE, 0, &filename) != FcResultMatch) {
			LOG_FONT (stderr, "no filename\n");
			FcPatternDestroy (matched);
			continue;
		}
		
		if (FcPatternGetInteger (matched, FC_INDEX, 0, &index) != FcResultMatch) {
			LOG_FONT (stderr, "no index\n");
			FcPatternDestroy (matched);
			continue;
		}
		
		if ((face = OpenFontFace ((const char *) filename, NULL, index))) {
			if (!g_ascii_strcasecmp (face->GetFamilyName (), desired.family_name)) {
				LOG_FONT (stderr, "got %s %s\n", face->GetFamilyName (), face->GetStyleName ());
				face->ref ();
				g_hash_table_insert (system_faces, key, face); // the key is freed when the hash table is destroyed
				g_free (desired.family_name);
				FcPatternDestroy (matched);
				return face;
			}
			
			LOG_FONT (stderr, "family mismatch\n");
			face->unref ();
		} else {
			LOG_FONT (stderr, "family not found\n");
		}
		
		FcPatternDestroy (matched);
	}
	
	g_hash_table_insert (system_faces, key, NULL); // the key is freed when the hash table is destroyed
	g_free (desired.family_name);
	
	return NULL;
}

FontFace *
FontManager::OpenFont (const char *name, FontStretches stretch, FontWeights weight, FontStyles style)
{
	const char *family;
	FontFace *face;
	//char *end;
	//int index;
	
	if ((family = strchr (name, '#'))) {
		char *resource = g_strndup (name, family - name);
		
		family++;
		
		//if ((index = strtol (family, &end, 10)) >= 0 && index < G_MAXINT && *end == '\0')
		//	face = OpenFontResource (resource, NULL, index, stretch, weight, style);
		//else
		face = OpenFontResource (resource, family, -1, stretch, weight, style);
		
		g_free (resource);
	} else {
		face = OpenSystemFont (name, stretch, weight, style);
	}
	
	return face;
}

FontFace *
FontManager::OpenFont (const char *name, int index)
{
	return OpenFontResource (name, NULL, index, FontStretchesNormal, FontWeightsNormal, FontStylesNormal);
}
