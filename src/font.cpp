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
#include <glib/gstdio.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#include "zip/unzip.h"
#include "moon-path.h"
#include "utils.h"
#include "list.h"
#include "font.h"

#include FT_OUTLINE_H

#if FONT_DEBUG
#define d(x) x
#else
#define d(x)
#endif


static const FT_Matrix invert_y = {
        65535, 0,
        0, -65535,
};

#define LOAD_FLAGS (FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL)


struct GlyphBitmap {
	cairo_surface_t *surface;
	unsigned char *buffer;
	int height;
	int width;
	int left;
	int top;
};

struct FontStyleInfo {
	int weight;
	int width;
	int slant;
};

struct FontFile : public List::Node {
	GPtrArray *faces;
	char *path;
	
	FontFile (const char *path);
	~FontFile ();
};

struct FontFileFace {
	FontStyleInfo style;
	char *family_name;
	FontFile *file;
	int index;
	
	FontFileFace (FontFile *file, FT_Face face, int index);
	
	~FontFileFace ()
	{
		g_free (family_name);
	}
};

struct FontDir {
	List *fonts;
	char *key;
	char *dir;
	
	FontDir (const char *key)
	{
		this->key = g_strdup (key);
		fonts = new List ();
		dir = NULL;
	}
	
	~FontDir ()
	{
		delete fonts;
		g_free (dir);
		g_free (key);
	}
	
	void CacheFileInfo (const char *filename, FT_Face face);
};

static GHashTable *font_cache = NULL;
static GHashTable *fontdirs = NULL;
static bool initialized = false;
static FT_Library libft2;
static double dpi;


static void
delete_fontdir (FontDir *dir)
{
	delete dir;
}

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
	
	fontdirs = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify) delete_fontdir);
	
	pattern = FcPatternBuild (NULL, FC_FAMILY, FcTypeString, "Sans",
				  FC_SIZE, FcTypeDouble, 10.0, NULL);
	
	if (FcPatternGetDouble (pattern, FC_DPI, 0, &dpi) != FcResultMatch)
		dpi = 72.0;
	
	FcPatternDestroy (pattern);
	
	initialized = true;
}

void
font_shutdown (void)
{
	if (!initialized)
		return;
	
	g_hash_table_destroy (font_cache);
	g_hash_table_destroy (fontdirs);
	
	FT_Done_FreeType (libft2);
	
	initialized = false;
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


static struct {
	const char *name;
	int value;
} style_weights[] = {
	// FIXME: are these strings all correct (e.g. spaces vs no spaces)?
	{ "Ultra Light", FC_WEIGHT_ULTRALIGHT },
	{ "Light",       FC_WEIGHT_LIGHT      },
	{ "Semi Bold",   FC_WEIGHT_DEMIBOLD   },
	{ "Ultra Bold",  FC_WEIGHT_ULTRABOLD  },
	{ "Medium",      FC_WEIGHT_MEDIUM     },
	{ "Bold",        FC_WEIGHT_BOLD       },
	{ "Black",       FC_WEIGHT_BLACK      }
};

static int
style_weight_parse (const char *style)
{
	unsigned int i;
	
	if (style) {
		for (i = 0; i < G_N_ELEMENTS (style_weights); i++) {
			if (strstr (style, style_weights[i].name))
				return style_weights[i].value;
		}
	}
	
	return FC_WEIGHT_NORMAL;
}

static struct {
	const char *name;
	int value;
} style_widths[] = {
	// FIXME: are these strings all correct (e.g. spaces vs no spaces, Cond vs Condensed)?
	{ "Ultra Condensed", FC_WIDTH_ULTRACONDENSED },
	{ "Extra Condensed", FC_WIDTH_EXTRACONDENSED },
	{ "Semi Condensed",  FC_WIDTH_SEMICONDENSED  },
	{ "Condensed",       FC_WIDTH_CONDENSED      },
	{ "Ultra Expanded",  FC_WIDTH_ULTRAEXPANDED  },
	{ "Extra Expanded",  FC_WIDTH_EXTRAEXPANDED  },
	{ "Semi Expanded",   FC_WIDTH_SEMIEXPANDED   },
	{ "Expanded",        FC_WIDTH_EXPANDED       }
};

static int
style_width_parse (const char *style)
{
	unsigned int i;
	
	if (style) {
		for (i = 0; i < G_N_ELEMENTS (style_widths); i++) {
			if (strstr (style, style_widths[i].name))
				return style_widths[i].value;
		}
	}
	
	return FC_WIDTH_NORMAL;
}

static struct {
	const char *name;
	int value;
} style_slants[] = {
	{ "Oblique", FC_SLANT_OBLIQUE },
	{ "Italic",  FC_SLANT_ITALIC  }
};

static int
style_slant_parse (const char *style)
{
	unsigned int i;
	
	if (style) {
		for (i = 0; i < G_N_ELEMENTS (style_slants); i++) {
			if (strstr (style, style_slants[i].name))
				return style_slants[i].value;
		}
	}
	
	return FC_SLANT_ROMAN;
}

static void
style_info_parse (const char *style, FontStyleInfo *info)
{
	info->weight = style_weight_parse (style);
	info->width = style_width_parse (style);
	info->slant = style_slant_parse (style);
}

#if FONT_DEBUG
static const char *
style_name (FontStyleInfo *style, char *namebuf)
{
	char *p;
	uint i;
	
	strcpy (namebuf, "Regular");
	p = namebuf;
	
	for (i = 0; i < G_N_ELEMENTS (style_widths); i++) {
		if (style_widths[i].value == style->width) {
			p = g_stpcpy (p, style_widths[i].name);
			break;
		}
	}
	
	for (i = 0; i < G_N_ELEMENTS (style_weights); i++) {
		if (style_weights[i].value == style->weight) {
			if (p != namebuf)
				*p++ = ' ';
			
			p = g_stpcpy (p, style_weights[i].name);
			break;
		}
	}
	
	for (i = 0; i < G_N_ELEMENTS (style_slants); i++) {
		if (style_slants[i].value == style->slant) {
			if (p != namebuf)
				*p++ = ' ';
			
			p = g_stpcpy (p, style_slants[i].name);
			break;
		}
	}
	
	return namebuf;
}
#endif

FontFileFace::FontFileFace (FontFile *file, FT_Face face, int index)
{
	d(fprintf (stderr, "\t\t\t* index=%d: family=\"%s\"; style=\"%s\"\n",
		   index, face->family_name, face->style_name));
	
	style_info_parse (face->style_name, &style);
	family_name = g_strdup (face->family_name);
	this->index = index;
	this->file = file;
}

FontFile::FontFile (const char *path)
{
	this->path = g_strdup (path);
	faces = NULL;
}

FontFile::~FontFile ()
{
	if (faces != NULL) {
		FontFileFace *face;
		
		for (uint i = 0; i < faces->len; i++) {
			face = (FontFileFace *) faces->pdata[i];
			delete face;
		}
		
		g_ptr_array_free (faces, true);
	}
}

void
FontDir::CacheFileInfo (const char *filename, FT_Face face)
{
	int i = 0, nfaces = face->num_faces;
	FontFileFace *fface;
	FontFile *file;
	
	file = new FontFile (filename);
	file->faces = g_ptr_array_new ();
	
	do {
		if (i > 0 && FT_New_Face (libft2, filename, i, &face) != 0)
			break;
		
		fface = new FontFileFace (file, face, i);
		g_ptr_array_add (file->faces, fface);
		
		FT_Done_Face (face);
		i++;
	} while (i < nfaces);
	
	fonts->Append (file);
}

bool
DecodeObfuscatedFontGUID (const char *in, char *key)
{
	const char *inptr = in;
	int i = 16;
	
	while (i > 0 && *inptr && *inptr != '.') {
		if (*inptr == '-')
			inptr++;
		
		i--;
		
		if (*inptr >= '0' && *inptr <= '9')
			key[i] = (*inptr - '0') * 16;
		else if (*inptr >= 'a' && *inptr <= 'f')
			key[i] = ((*inptr - 'a') + 10) * 16;
		else if (*inptr >= 'A' && *inptr <= 'F')
			key[i] = ((*inptr - 'A') + 10) * 16;
		else
			return false;
		
		inptr++;
		
		if (*inptr >= '0' && *inptr <= '9')
			key[i] += (*inptr - '0');
		else if (*inptr >= 'a' && *inptr <= 'f')
			key[i] += ((*inptr - 'a') + 10);
		else if (*inptr >= 'A' && *inptr <= 'F')
			key[i] += ((*inptr - 'A') + 10);
		else
			return false;
		
		inptr++;
	}
	
	if (i > 0)
		return false;
	
	return true;
}

bool
DeobfuscateFontFileWithGUID (const char *filename, const char *guid, FT_Face *pFace)
{
	char deobfuscated[32], buf[32];
	FT_Face face = NULL;
	size_t nread;
	FILE *fp;
	int i;
	
	if (!(fp = fopen (filename, "r+")))
		return false;
	
	// read the first 32 bytes of the obfuscated font file
	if ((nread = fread (buf, 1, 32, fp)) < 32)
		goto exception;
	
	// XOR the guid with the first 32 bytes of the obfuscated font file
	for (i = 0; i < 32; i++)
		deobfuscated[i] = buf[i] ^ guid[i % 16];
	
	if (fseek (fp, 0, SEEK_SET) != 0)
		goto exception;
	
	if ((nread = fwrite (deobfuscated, 1, 32, fp)) != 32)
		goto exception;
	
	fflush (fp);
	
	if (FT_New_Face (libft2, filename, 0, &face) != 0)
		goto undo;
	
	if (!pFace)
		FT_Done_Face (face);
	else
		*pFace = face;
	
	fclose (fp);
	
	return true;
	
undo:
	
	if (fseek (fp, 0, SEEK_SET) == 0) {
		fwrite (buf, 1, 32, fp);
		fflush (fp);
	}
	
exception:
	
	fclose (fp);
	
	return false;
}

static bool
IndexFontSubdirectory (const char *toplevel, GString *path, FontDir **out)
{
	FontDir *fontdir = *out;
	struct dirent *dent;
	struct stat st;
	FT_Face face;
	size_t len;
	DIR *dir;
	
	if (!(dir = opendir (path->str)))
		return fontdir != NULL;
	
	g_string_append_c (path, G_DIR_SEPARATOR);
	len = path->len;
	
	while ((dent = readdir (dir))) {
		if (!strcmp (dent->d_name, "..") ||
		    !strcmp (dent->d_name, "."))
			continue;
		
		g_string_append (path, dent->d_name);
		
		if (stat (path->str, &st) == -1)
			goto next;
		
		if (S_ISDIR (st.st_mode)) {
			IndexFontSubdirectory (toplevel, path, &fontdir);
			goto next;
		}
		
		if (FT_New_Face (libft2, path->str, 0, &face) != 0) {
			// not a valid font file... is it maybe an obfuscated font?
			char guid[16];
			
			if (!DecodeObfuscatedFontGUID (dent->d_name, guid))
				goto next;
			
			if (!DeobfuscateFontFileWithGUID (path->str, guid, &face))
				goto next;
		}
		
		if (fontdir == NULL)
			fontdir = new FontDir (toplevel);
		
		// cache font info
		fontdir->CacheFileInfo (path->str, face);
		
	next:
		g_string_truncate (path, len);
	}
	
	closedir (dir);
	
	*out = fontdir;
	
	return fontdir != NULL;
}

static FontDir *
IndexFontDirectory (const char *dirname)
{
	FontDir *fontdir = NULL;
	GString *path;
	size_t len;
	
	path = g_string_new (dirname);
	len = path->len;
	
	if (!IndexFontSubdirectory (dirname, path, &fontdir)) {
		g_string_free (path, true);
		return NULL;
	}
	
	g_string_truncate (path, len);
	fontdir->dir = path->str;
	
	g_string_free (path, false);
	
	return fontdir;
}


struct FontFamilyInfo {
	char *name;
	int weight;
	int width;
	int slant;
};

struct Token {
	Token *next;
	const char *value;
	size_t len;
};

enum StyleType {
	Weight,
	Width,
	Slant
};

static struct {
	const char *name;
	StyleType type;
	int value;
} style_values[] = {
	// FIXME: what other names can be specified?
	{ "Light",    Weight, FC_WEIGHT_LIGHT        },
	{ "SemiBold", Weight, FC_WEIGHT_DEMIBOLD     },
	{ "Bold",     Weight, FC_WEIGHT_BOLD         },
	{ "Black",    Weight, FC_WEIGHT_BLACK        },
	
	{ "SemiCond", Width,  FC_WIDTH_SEMICONDENSED },
	{ "Cond",     Width,  FC_WIDTH_CONDENSED     },
	
	{ "Oblique",  Slant,  FC_SLANT_OBLIQUE       },
	{ "Italic",   Slant,  FC_SLANT_ITALIC        }
};


static FontFamilyInfo *
parse_font_family (const char *in)
{
	register const char *inptr = in;
	Token *tokens, *token, *tail;
	const char *start, *end;
	FontFamilyInfo *family;
	bool style_info;
	uint i;
	
	// tokenize the font name so we can check for stretch/weight/style info in the name
	tail = tokens = token = new Token ();
	token->value = start = inptr;
	
	while (*inptr) {
		if (*inptr == ' ') {
			token->len = (inptr - token->value);
			while (*inptr == ' ')
				inptr++;
			
			if (*inptr) {
				token = new Token ();
				token->value = inptr;
				tail->next = token;
				tail = token;
			}
		} else {
			inptr++;
		}
	}
	
	token->len = (inptr - token->value);
	token->next = NULL;
	
	family = new FontFamilyInfo ();
	family->weight = FC_WEIGHT_NORMAL;
	family->width = FC_WIDTH_NORMAL;
	family->slant = FC_SLANT_ROMAN;
	
	// assume at least the first token is part of the face name
	end = tokens->value + tokens->len;
	token = tokens->next;
	style_info = false;
	delete tokens;
	
	while (token != NULL) {
		for (i = 0; i < G_N_ELEMENTS (style_values); i++) {
			if (!strncmp (style_values[i].name, token->value, token->len)) {
				switch (style_values[i].type) {
				case Weight:
					family->weight = style_values[i].value;
					break;
				case Width:
					family->width = style_values[i].value;
					break;
				case Slant:
					family->slant = style_values[i].value;
					break;
				}
				
				style_info = true;
				break;
			}
		}
		
		if (!style_info)
			end = token->value + token->len;
		
		tail = token->next;
		delete token;
		token = tail;
	}
	
	family->name = g_strndup (start, (end - start));
	
	return family;
}


struct FontFaceSimilarity {
	FontFileFace *face;
	int weight;
	int width;
	int slant;
};


bool
TextFont::OpenFontDirectory (FcPattern *pattern, const char *path, const char **families)
{
#if FONT_DEBUG
	char stylebuf1[256], stylebuf2[256];
#endif
	FontFaceSimilarity similar;
	FontFamilyInfo *family;
	FontStyleInfo style;
	FontFileFace *fface;
	GPtrArray *array;
	FontFile *file;
	FontDir *dir;
	uint i, j;
	
	if (FcPatternGetInteger (pattern, FC_WEIGHT, 0, &style.weight) != FcResultMatch)
		return false;
	
	if (FcPatternGetInteger (pattern, FC_WIDTH, 0, &style.width) != FcResultMatch)
		return false;
	
	if (FcPatternGetInteger (pattern, FC_SLANT, 0, &style.slant) != FcResultMatch)
		return false;
	
	if (!(dir = (FontDir *) g_hash_table_lookup (fontdirs, path))) {
		d(fprintf (stderr, "\t* indexing font directory...\n"));
		if (!(dir = IndexFontDirectory (path)))
			return false;
		
		g_hash_table_insert (fontdirs, dir->key, dir);
	} else {
		d(fprintf (stderr, "\t* reusing an extracted zip archive...\n"));
	}
	
	array = g_ptr_array_new ();
	for (i = 0; families[i]; i++) {
		// parse the family name to extract any additional style info
		family = parse_font_family ((const char *) families[i]);
		g_ptr_array_add (array, family);
		
		// FIXME: which style info should take prioriy?
		if (family->weight == FC_WEIGHT_NORMAL)
			family->weight = style.weight;
		if (family->width == FC_WIDTH_NORMAL)
			family->width = style.width;
		if (family->slant == FC_SLANT_ROMAN)
			family->slant = style.slant;
	}
	
	similar.weight = INT_MAX;
	similar.width = INT_MAX;
	similar.slant = INT_MAX;
	similar.face = NULL;
	
	file = (FontFile *) dir->fonts->First ();
	while (file != NULL) {
		for (i = 0; i < file->faces->len; i++) {
			fface = (FontFileFace *) file->faces->pdata[i];
			if (!fface->family_name)
				continue;
			
			for (j = 0; j < array->len; j++) {
				family = (FontFamilyInfo *) array->pdata[i];
				style.weight = family->weight;
				style.width = family->width;
				style.slant = family->slant;
				
				d(fprintf (stderr, "\t\t* checking if '%s' matches '%s'... ",
					   fface->family_name, family->name));
				
				// compare against parsed family name (don't want to include style info)
				if (strcmp (family->name, fface->family_name) != 0) {
					d(fprintf (stderr, "no\n"));
					continue;
				}
				
				d(fprintf (stderr, "yes\n\t\t\t* checking if '%s' matches '%s'... ",
					   style_name (&fface->style, stylebuf1), style_name (&style, stylebuf2)));
				
				if (fface->style.weight == style.weight &&
				    fface->style.width == style.width &&
				    fface->style.slant == style.slant) {
					// found an exact match
					d(fprintf (stderr, "yes\n"));
					goto found;
				}
				
				// not an exact match, but similar...
				if (abs (fface->style.weight - style.weight) <= similar.weight &&
				    abs (fface->style.width - style.width) <= similar.width &&
				    abs (fface->style.slant - style.slant) <= similar.slant) {
					d(fprintf (stderr, "no, but closest match\n"));
					similar.weight = abs (fface->style.weight - style.weight);
					similar.width = abs (fface->style.width - style.width);
					similar.slant = abs (fface->style.slant - style.slant);
					similar.face = fface;
				} else {
					d(fprintf (stderr, "no\n"));
				}
			}
		}
		
		file = (FontFile *) file->next;
	}
	
	// we were unable to find an exact match...
	
	if (similar.face == NULL) {
		// no similar fonts, I guess we just fail?
		for (i = 0; i < array->len; i++)
			delete (FontFamilyInfo *) array->pdata[i];
		g_ptr_array_free (array, true);
		return false;
	}
	
	// use the closest match...
	file = similar.face->file;
	fface = similar.face;
	
found:
	
	d(fprintf (stderr, "\t\t* using font '%s, %s'\n", fface->family_name, style_name (&fface->style, stylebuf1)));
	
	for (i = 0; i < array->len; i++)
		delete (FontFamilyInfo *) array->pdata[i];
	g_ptr_array_free (array, true);
	
	return FT_New_Face (libft2, file->path, fface->index, &face) == 0;
}

TextFont::TextFont (FcPattern *pattern, const char *family_name, const char *debug_name)
{
	FcPattern *matched = NULL, *fallback = NULL;
	FT_Long position, thickness;
	FcChar8 *filename = NULL;
	bool try_nofile = false;
	char **families = NULL;
	FcResult result;
	FT_Error err;
	double size;
	int id, i;
	
	d(fprintf (stderr, "\nTextFont %p: Attempting to load %s\n", this, debug_name));
	
	FcPatternGetDouble (pattern, FC_PIXEL_SIZE, 0, &size);
	FcPatternGetDouble (pattern, FC_SCALE, 0, &scale);
	
	// FIXME: would be nice to simply get this from the original
	// pattern... then we'd have fewer args to pass in.
	if (family_name) {
		families = g_strsplit (family_name, ",", -1);
		for (i = 0; families[i]; i++)
			families[i] = g_strstrip (families[i]);
	}
	
	if (FcPatternGetString (pattern, FC_FILE, 0, &filename) == FcResultMatch) {
		struct stat st;
		int rv;
		
		try_nofile = true;
		
		if ((rv = stat ((const char *) filename, &st)) == -1 || S_ISDIR (st.st_mode)) {
			if (rv != -1 && OpenFontDirectory (pattern, (const char *) filename, (const char **) families)) {
				// we found the font in the directory...
				goto loaded;
			}
			
			if (family_name)
				goto try_nofile;
		}
	} else {
		// original pattern does not reference a file or directory path,
		// need to query FontConfig to find us a match...
		matched = FcFontMatch (NULL, pattern, &result);
	}
	
	if (matched == NULL) {
		FcPatternReference (pattern);
		matched = pattern;
	}
	
	do {
		if (FcPatternGetString (matched, FC_FILE, 0, &filename) != FcResultMatch)
			goto fail;
		
		if (FcPatternGetInteger (matched, FC_INDEX, 0, &id) != FcResultMatch)
			goto fail;
		
		d(fprintf (stderr, "\t* loading font from `%s' (index=%d)... ", filename, id));
		if ((err = FT_New_Face (libft2, (const char *) filename, id, &face)) == 0) {
			if (!family_name || !face->family_name) {
				d(fprintf (stderr, "success!\n"));
				break;
			}
			
			// make sure the font family name matches what was requested...
			for (i = 0; families[i]; i++) {
				if (!g_ascii_strcasecmp (face->family_name, families[i]))
					break;
			}
			
			if (families[i]) {
				d(fprintf (stderr, "success!\n"));
				break;
			}
			
			d(fprintf (stderr, "no; incorrect family: '%s' does not match '%s'\n",
				   face->family_name, family_name));
			
			FT_Done_Face (face);
			face = NULL;
		} else {
			d(fprintf (stderr, "failed :(\n"));
		}
		
	fail:
		
		if (try_nofile && family_name) {
		try_nofile:
			// We couldn't find a matching font in the font directory, so let's try
			// removing the filename from the pattern and see if that gets us what
			// we are looking for.
			d(fprintf (stderr, "\t* falling back to specified family, '%s'...\n", family_name));
			fallback = FcPatternDuplicate (pattern);
			FcPatternDel (fallback, FC_FILE);
			
			if (matched != NULL)
				FcPatternDestroy (matched);
			
			matched = FcFontMatch (NULL, fallback, &result);
			FcPatternDestroy (fallback);
			fallback = NULL;
			filename = NULL;
			
			try_nofile = false;
			continue;
		}
		
		if (fallback != NULL) {
			face = NULL;
			break;
		}
		
		d(fprintf (stderr, "\t* falling back to default font\n"));
		
		fallback = FcPatternCreate ();
		FcPatternAddString (fallback, FC_FAMILY, (FcChar8 *) "Lucida Sans Unicode");
		FcPatternAddString (fallback, FC_FAMILY, (FcChar8 *) "Lucida Sans");
		FcPatternAddString (fallback, FC_FAMILY, (FcChar8 *) "Sans");
		FcPatternAddDouble (fallback, FC_PIXEL_SIZE, size);
		FcPatternAddDouble (fallback, FC_SCALE, scale);
		FcPatternAddDouble (fallback, FC_DPI, dpi);
		
		FcPatternDestroy (matched);
		matched = FcFontMatch (NULL, fallback, &result);
		FcPatternDestroy (fallback);
		family_name = NULL;
		filename = NULL;
		face = NULL;
	} while (true);
	
	FcPatternDestroy (matched);
	
loaded:
	
	if (families)
		g_strfreev (families);
	
	if (face != NULL) {
		FT_Set_Pixel_Sizes (face, 0, (int) size);
		
		// calculate underline thickness
		thickness = FT_MulFix (face->underline_thickness, face->size->metrics.y_scale);
		underline_thickness = ((double) thickness) / (scale * 64);
		
		// calculate underline position
		position = FT_MulFix (-face->underline_position, face->size->metrics.y_scale);
		underline_position = ((double) position) / (scale * 64) + ((underline_thickness + 1) / 2.0);
		
		if (underline_thickness < 1.0)
			underline_thickness = 1.0;
	} else {
		underline_thickness = 1.0;
		underline_position = 0.0;
	}
	
	g_hash_table_insert (font_cache, pattern, this);
	FcPatternReference (pattern);
	this->pattern = pattern;
	ref_count = 1;
	nglyphs = 0;
}

TextFont::~TextFont ()
{
	int i;
	
	for (i = 0; i < nglyphs; i++) {
		if (glyphs[i].path)
			moon_path_destroy (glyphs[i].path);
		
		if (glyphs[i].bitmap) {
			if (glyphs[i].bitmap->surface)
				cairo_surface_destroy (glyphs[i].bitmap->surface);
			
			g_free (glyphs[i].bitmap->buffer);
			g_free (glyphs[i].bitmap);
		}
	}
	
	if (face)
		FT_Done_Face (face);
	
	g_hash_table_remove (font_cache, pattern);
	FcPatternDestroy (pattern);
}

TextFont *
TextFont::Load (FcPattern *pattern, const char *family_name, const char *debug_name)
{
	TextFont *font;
	
	if ((font = (TextFont *) g_hash_table_lookup (font_cache, pattern))) {
		font->ref ();
		return font;
	}
	
	return new TextFont (pattern, family_name, debug_name);
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
	if (!face)
		return false;
	
	return FT_IS_SCALABLE (face);
}

double
TextFont::Kerning (gunichar left, gunichar right)
{
	FT_Vector kerning;
	
	if (!face || !FT_HAS_KERNING (face) || left == 0 || right == 0)
		return 0.0;
	
	FT_Get_Kerning (face, left, right, FT_KERNING_DEFAULT, &kerning);
	
	return (double) kerning.x / (scale * 64);
}

double
TextFont::Descender ()
{
	if (!face)
		return 0.0;
	
	return (double) face->size->metrics.descender / (scale * 64);
}

double
TextFont::Ascender ()
{
	if (!face)
		return 0.0;
	
	return (double) face->size->metrics.ascender / (scale * 64);
}

double
TextFont::Height ()
{
	if (!face)
		return 0.0;
	
	return (double) face->size->metrics.height / (scale * 64);
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

#define BITSWAP8(c) ((((c) * 0x0802LU & 0x22110LU) | ((c) * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16)

static void
prepare_bitmap (GlyphInfo *glyph, FT_Bitmap *bitmap)
{
	int width, height, stride;
	unsigned char *buffer, *d;
	cairo_format_t format;
	size_t size;
	int count;
	
	height = bitmap->rows;
	width = bitmap->width;
	
	size = bitmap->rows * bitmap->pitch;
	buffer = glyph->bitmap->buffer = (unsigned char *) g_malloc (size);
	memcpy (buffer, bitmap->buffer, size);
	
	switch (bitmap->pixel_mode) {
	case FT_PIXEL_MODE_MONO:
		//printf ("pixel_mode is FT_PIXEL_MODE_MONO\n");
		stride = (((width + 31) & ~31) >> 3);
		format = CAIRO_FORMAT_A1;
		
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
		count = stride * height;
		d = buffer;
		
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
		//printf ("unknown pixel format\n");
		return;
	}
	
	glyph->bitmap->surface = cairo_image_surface_create_for_data (buffer, format, width, height, stride);
	glyph->bitmap->height = height;
	glyph->bitmap->width = width;
}

static int
glyphsort (const void *v1, const void *v2)
{
	GlyphInfo *g1 = (GlyphInfo *) v1;
	GlyphInfo *g2 = (GlyphInfo *) v2;
	
	return g2->requested - g1->requested;
}

GlyphInfo *
TextFont::GetGlyphInfo (gunichar unichar, uint32_t index)
{
	double scale = 1.0 / (64.0 * this->scale);
	GlyphInfo *glyph;
	int i;
	
	if (!face)
		return NULL;
	
	for (i = 0; i < nglyphs; i++) {
		if (glyphs[i].index == index) {
			glyph = &glyphs[i];
			glyph->requested++;
			return glyph;
		}
	}
	
	if (FT_Load_Glyph (face, index, LOAD_FLAGS) != 0)
		return NULL;
	
	if (FT_Render_Glyph (face->glyph, FT_RENDER_MODE_NORMAL) != 0)
		return NULL;
	
	if (nglyphs == 256) {
		// need to expire the least requested glyph (which will be the last element in the array after sorting)
		qsort (glyphs, nglyphs, sizeof (GlyphInfo), glyphsort);
		glyph = &glyphs[nglyphs - 1];
		
		if (glyph->bitmap) {
			if (glyph->bitmap->surface) {
				cairo_surface_destroy (glyph->bitmap->surface);
				glyph->bitmap->surface = NULL;
			}
			
			g_free (glyph->bitmap->buffer);
			glyph->bitmap->buffer = NULL;
		}
		
		if (glyph->path)
			moon_path_destroy (glyph->path);
	} else {
		glyph = &glyphs[nglyphs++];
		glyph->bitmap = NULL;
	}
	
	glyph->unichar = unichar;
	glyph->index = index;
	glyph->requested = 1;
	glyph->path = NULL;
	
	if (FT_IS_SCALABLE (face)) {
		FT_Matrix matrix;
		
		// FIXME: can the scale ever overflow the 16.16 Fixed type?
		matrix.xx = (FT_Fixed) (65535 / this->scale);
		matrix.xy = 0;
		matrix.yy = (FT_Fixed) (-65535 / this->scale);
		matrix.yx = 0;
		
		glyph->path = moon_path_new (8);
		FT_Outline_Transform (&face->glyph->outline, &matrix);
		FT_Outline_Decompose (&face->glyph->outline, &outline_funcs, glyph->path);
	} else {
		if (glyph->bitmap == NULL)
			glyph->bitmap = g_new (GlyphBitmap, 1);
		
		glyph->bitmap->left = face->glyph->bitmap_left;
		glyph->bitmap->top = face->glyph->bitmap_top;
		prepare_bitmap (glyph, &face->glyph->bitmap);
	}
	
	glyph->metrics.horiBearingX = face->glyph->metrics.horiBearingX * scale;
	glyph->metrics.horiBearingY = face->glyph->metrics.horiBearingY * scale;
	glyph->metrics.horiAdvance = face->glyph->metrics.horiAdvance * scale;
	//glyph->metrics.vertBearingX = face->glyph->metrics.vertBearingX * scale;
	//glyph->metrics.vertBearingY = face->glyph->metrics.vertBearingY * scale;
	//glyph->metrics.vertAdvance = face->glyph->metrics.vertAdvance * scale;
	glyph->metrics.height = face->glyph->metrics.height * scale;
	glyph->metrics.width = face->glyph->metrics.width * scale;
	
#if 0
	// FIXME: Seems like MS Gothic, GulimChe, DotumChe, BatangChe and GungsuhChe
	// are all fixed-width fonts, except that MS renders the ascii subset with
	// 1/2 the horiAdvance of the East Asian glyphs. This is a really gross hack
	// to mimic their behavior until I find a better way.
	// Note: Pango seems to handle these fonts ok, but I can't figure out how...
	if (FT_IS_FIXED_WIDTH (face) && unichar > 0 && face->num_glyphs > 4096) {
		if ((face->glyph->metrics.width * 1.86) < (double) face->glyph->metrics.horiAdvance)
			glyph->metrics.horiAdvance /= 2.0;
	}
#endif
	
	return glyph;
}

GlyphInfo *
TextFont::GetGlyphInfo (gunichar unichar)
{
	uint32_t index;
	
	if (!face)
		return NULL;
	
	index = FT_Get_Char_Index (face, unichar);
	
	return GetGlyphInfo (unichar, index);
}

GlyphInfo *
TextFont::GetGlyphInfoByIndex (uint32_t index)
{
	gunichar unichar;
	uint32_t idx;
	
	if (!face)
		return NULL;
	
	if (index != 0) {
		unichar = FT_Get_First_Char (face, &idx);
		while (idx != index && idx != 0)
			unichar = FT_Get_Next_Char (face, unichar, &idx);
		
		if (idx == 0)
			unichar = 0;
	} else {
		unichar = 0;
	}
	
	return GetGlyphInfo (unichar, index);
}

bool
TextFont::HasGlyph (gunichar unichar)
{
	if (!face)
		return false;
	
	return FcFreeTypeCharIndex (face, unichar) != 0;
}

double
TextFont::UnderlinePosition ()
{
	return underline_position;
}

double
TextFont::UnderlineThickness ()
{
	return underline_thickness;
}

void
TextFont::RenderGlyphBitmap (cairo_t *cr, GlyphInfo *glyph, double x, double y)
{
	// take horiBearingX into consideration
	x += glyph->metrics.horiBearingX;
	
	// Bitmap glyphs are rendered from the top down
	y -= glyph->metrics.horiBearingY;
	
	cairo_save (cr);
	
	cairo_mask_surface (cr, glyph->bitmap->surface, x, y);
	cairo_set_operator (cr, CAIRO_OPERATOR_SATURATE);
	
	cairo_new_path (cr);
	cairo_rectangle (cr, x, y, (double) glyph->bitmap->width, (double) glyph->bitmap->height);
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
TextFont::Render (cairo_t *cr, gunichar unichar, double x, double y)
{
	GlyphInfo *glyph;
	
	if (!(glyph = GetGlyphInfo (unichar)))
		return;
	
	Render (cr, glyph, x, y);
}

void
TextFont::Path (cairo_t *cr, GlyphInfo *glyph, double x, double y)
{
	if (!glyph->path || !(&glyph->path->cairo)->data)
		return;
	
	cairo_save (cr);
	cairo_translate (cr, x, y);
	cairo_append_path (cr, &glyph->path->cairo);
	cairo_restore (cr);
}

void
TextFont::Path (cairo_t *cr, gunichar unichar, double x, double y)
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
	size = 14.666666984558105;
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

#define MIN_FONT_SIZE 41.0f

FcPattern *
TextFontDescription::CreatePattern ()
{
	FcPattern *pattern;
	char **families;
	int i;
	
	pattern = FcPatternCreate ();
	FcPatternAddDouble (pattern, FC_DPI, dpi);
	
	if (set & FontMaskFilename) {
		FcPatternAddString (pattern, FC_FILE, (FcChar8 *) filename);
		FcPatternAddInteger (pattern, FC_INDEX, index);
	}
	
	if (!(set & FontMaskFilename) || (set & FontMaskFamily)) {
		families = g_strsplit (GetFamily (), ",", -1);
		for (i = 0; families[i]; i++)
			FcPatternAddString (pattern, FC_FAMILY, (FcChar8 *) g_strstrip (families[i]));
		g_strfreev (families);
	}
	
	FcPatternAddInteger (pattern, FC_SLANT, fc_style (style));
	FcPatternAddInteger (pattern, FC_WEIGHT, fc_weight (weight));
	FcPatternAddInteger (pattern, FC_WIDTH, fc_stretch (stretch));
	if (size < MIN_FONT_SIZE) {
		FcPatternAddDouble (pattern, FC_PIXEL_SIZE, MIN_FONT_SIZE);
		FcPatternAddDouble (pattern, FC_SCALE, MIN_FONT_SIZE / size);
	} else {
		FcPatternAddDouble (pattern, FC_PIXEL_SIZE, size);
		FcPatternAddDouble (pattern, FC_SCALE, 1.0);
	}
	
	FcDefaultSubstitute (pattern);
	
	return pattern;
}

TextFont *
TextFontDescription::GetFont ()
{
	char *debug_name = NULL;
	FcPattern *pattern;
	
	if (font == NULL) {
		d(debug_name = ToString ());
		pattern = CreatePattern ();
		font = TextFont::Load (pattern, family, debug_name);
		FcPatternDestroy (pattern);
		d(g_free (debug_name));
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
	
	if (mask & FontMaskStretch)
		stretch = FontStretchesNormal;
	
	if (mask & FontMaskWeight)
		weight = FontWeightsNormal;
	
	if (mask & FontMaskStyle)
		style = FontStylesNormal;
	
	if (mask & FontMaskSize)
		size = 14.666f;
	
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
	bool changed;
	
	if (filename) {
		if (!this->filename || strcmp (this->filename, filename) != 0) {
			g_free (this->filename);
			this->filename = g_strdup (filename);
			set |= FontMaskFilename;
			changed = true;
		} else {
			changed = false;
		}
	} else {
		changed = this->filename != NULL;
		set &= ~FontMaskFilename;
		g_free (this->filename);
		this->filename = NULL;
	}
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
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
	bool changed = this->index != index;
	
	this->index = index;
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
	}
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
	bool changed;
	
	if (family) {
		if (!g_ascii_strcasecmp (family, "Portable User Interface"))
			family = "Lucida Sans Unicode, Lucida Sans";
		
		if (!this->family || g_ascii_strcasecmp (this->family, family) != 0) {
			g_free (this->family);
			this->family = g_strdup (family);
			set |= FontMaskFamily;
			changed = true;
		} else {
			changed = false;
		}
	} else {
		changed = this->family != NULL;
		set &= ~FontMaskFamily;
		g_free (this->family);
		this->family = NULL;
	}
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
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
	bool changed = this->style != style;
	
	this->style = style;
	set |= FontMaskStyle;
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
	}
}

FontWeights
TextFontDescription::GetWeight ()
{
	return weight;
}

void
TextFontDescription::SetWeight (FontWeights weight)
{
	bool changed = this->weight != weight;
	
	this->weight = weight;
	set |= FontMaskWeight;
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
	}
}

FontStretches
TextFontDescription::GetStretch ()
{
	return stretch;
}

void
TextFontDescription::SetStretch (FontStretches stretch)
{
	bool changed = this->stretch != stretch;
	
	this->stretch = stretch;
	set |= FontMaskStretch;
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
	}
}

double
TextFontDescription::GetSize ()
{
	return size;
}

void
TextFontDescription::SetSize (double size)
{
	bool changed = this->size != size;
	
	this->size = size;
	set |= FontMaskSize;
	
	if (changed && font != NULL) {
		font->unref ();
		font = NULL;
	}
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
		
		if (set & FontMaskFamily)
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
	
	g_string_append_printf (str, " %.3fpx", size);
	
	return g_string_free (str, false);
}

#if 0
TextRun::TextRun (const char *utf8, int len, TextDecorations deco, TextFontDescription *font, Brush **fg)
{
	gunichar *s, *d;
	
	d = this->text = g_utf8_to_ucs4_fast (utf8, len, NULL);
	
	// convert all ascii lwsp into a SPACE, conserving only \n's
	for (s = this->text; *s; s++) {
		if (g_unichar_isspace (*s)) {
			if (*s == '\n')
				*d++ = *s;
			else if (*s < 128)
				*d++ = ' ';
			else
				*d++ = *s;
		} else {
			*d++ = *s;
		}
	}
	
	*d = '\0';
	
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
	double uwidth;
	double width;
	
	TextSegment (TextRun *run, int start);
	~TextSegment ();
};

TextSegment::TextSegment (TextRun *run, int start)
{
	this->uwidth = -1.0;
	this->width = -1.0;
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

#define BBOX_MARGIN 1.0
#define BBOX_PADDING 2.0

static int
IndexOfLastWord (gunichar *text)
{
	gunichar *inptr = text;
	gunichar *word = NULL;
	
	while (*inptr == ' ' || *inptr == '\n')
		inptr++;
	
	while (*inptr) {
		// skip over the word
		word = inptr++;
		while (*inptr && *inptr != ' ' && *inptr != '\n')
			inptr++;
		
		if (*inptr == '\n')
			word = NULL;
		
		// skip over lwsp
		while (*inptr == ' ' || *inptr == '\n')
			inptr++;
	}
	
	if (word != NULL)
		return (word - text);
	
	return -1;
}

void
TextLayout::Layout ()
{
	double nslw, lw, lh, sx, dy;
	bool first_char = true;
	bool first_run = true;
	TextSegment *segment;
	bool clipped = false;
	int last_word = -1;
	uint32_t prev = 0;
	GlyphInfo *glyph;
	TextLine *line;
	double advance;
	double descend;
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
	nslw = 0.0;
	dy = 0.0;
	sx = 0.0;
	
	if (!runs || runs->IsEmpty () || max_width == 0 || max_height == 0)
		return;
	
	spc.width = 0.0;
	spc.index = -1;
	
	line = new TextLine ();
	for (run = (TextRun *) runs->First (); run; run = (TextRun *) run->next) {
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
			dy += lh;
			
			if (nslw > 0.0)
				height = dy;
			
			width = MAX (width, nslw);
			
			if (run->next)
				line = new TextLine ();
			else
				line = 0;
			
			first_char = true;
			first_run = true;
			clipped = false;
			spc.width = 0.0;
			spc.index = -1;
			lw = lh = 0.0;
			descend = 0.0;
			//ascend = 0.0;
			nslw = 0.0;
			continue;
		}
		
		if (!run->text[0])
			continue;
		
		if (!run->next)
			last_word = IndexOfLastWord (run->text);
		
		descend = MIN (descend, run->font->Descender ());
		//ascend = MAX (ascend, run->font->Ascender ());
		lh = MAX (lh, run->font->Height ());
		
		sx = lw;
		spc.index = -1;
		spc.width = nslw;
		segment = new TextSegment (run, 0);
		for (i = 0, prev = 0; run->text[i]; i++) {
			if (run->text[i] == '\n') {
				if (segment->start == i) {
					delete segment;
					segment = NULL;
				} else {
					segment->end = i;
				}
				
				wrap = true;
				goto wrap;
			}
			
			if (!(glyph = run->font->GetGlyphInfo (run->text[i])))
				continue;
			
			advance = glyph->metrics.horiAdvance;
			
			if (prev != 0)
				advance += run->font->Kerning (prev, glyph->index);
			else if (glyph->metrics.horiBearingX < 0)
				advance -= glyph->metrics.horiBearingX;
			
			if ((is_space = isSpace (run->text[i]))) {
				spc.width = nslw;
				spc.index = i;
			}
			
			if (!is_space && max_width >= 0.0 && (lw + advance + 1.0) > max_width) {
				double overflow = (lw + advance) - max_width;
				
				switch (wrapping) {
				case TextWrappingWrapWithOverflow:
					// We can stretch the width as wide as we have to
					// in order to fit the current word.
					if (spc.index != -1) {
						// wrap at the Space
						segment->end = spc.index + 1;
						nslw = spc.width;
						i = spc.index;
						wrap = true;
					} else {
						// In the middle of a word, can't wrap here.
						wrap = false;
					}
					break;
				case TextWrappingWrap:
					// Wrap at word boundaries if at all possible, failing that break the word.
					if (last_word != -1 && i >= last_word && spc.index == -1 && first_run) {
						// Silverlight has a bug where it will never break the last word
						wrap = false;
					} else if (overflow <= 1.0 && (run->text[i + 1] == 0 || isSpace (run->text[i + 1]))) {
						// Force-fit this last char
						wrap = false;
					} else if (spc.index != -1) {
						// Wrap at the Space
						segment->end = spc.index + 1;
						nslw = spc.width;
						i = spc.index;
						wrap = true;
					} else if (!first_char) {
						if (!first_run) {
							// Not the first Run on the line,
							// wrap from the beginning of the
							// Run
							nslw = spc.width;
							i = -1;
						} else {
							// Wrap before this char
							segment->end = i;
							i--;
						}
						
						wrap = true;
					} else {
						// Wrap after this char
						segment->end = i + 1;
						lw += advance;
						if (!is_space)
							nslw = lw;
						wrap = true;
					}
					break;
				case TextWrappingNoWrap:
				default:
					// Never wrap.
					wrap = false;
					break;
				}
			} else if (is_space) {
				if (max_width >= 0.0 && (lw + advance + 1.0) > max_width &&
				    wrapping == TextWrappingNoWrap && !clipped) {
					segment->end = i;
					bbox_width = lw;
					clipped = true;
				}
				
				first_char = false;
				wrap = false;
			} else {
				first_char = false;
				wrap = false;
			}
			
			if (wrap) {
			wrap:
				// end this segment and line if it has content
				if (!segment || segment->end > segment->start) {
					if (segment) {
						line->segments->Append (segment);
						segment->uwidth = nslw - sx;
						segment->width = lw - sx;
					}
					
					segment = new TextSegment (run, i + 1);
				}
				
				width = MAX (width, nslw);
				lines->Append (line);
				line->descend = descend;
				//line->ascend = ascend;
				line->height = lh;
				dy += lh;
				
				if (nslw > 0.0)
					height = dy;
				
				// create a new line
				descend = run->font->Descender ();
				//ascend = run->font->Ascender ();
				lh = run->font->Height ();
				line = new TextLine ();
				first_char = true;
				first_run = true;
				spc.width = 0.0;
				spc.index = -1;
				sx = lw = 0.0;
				nslw = 0.0;
				
				prev = 0;
			} else {
				// add this glyph to the current line
				prev = glyph->index;
				lw += advance;
				if (!is_space)
					nslw = lw;
			}
		}
		
		if (!clipped)
			segment->end = i;
		
		line->segments->Append (segment);
		segment->uwidth = nslw - sx;
		segment->width = lw - sx;
		
		width = MAX (width, nslw);
		
		first_run = false;
	}
	
	if (line) {
		width = MAX (width, lw);
		lines->Append (line);
		line->descend = descend;
		//line->ascend = ascend;
		line->height = lh;
		dy += lh;
		
		if (nslw > 0.0)
			height = dy;
	}
	
	//height += 1.0;
	//width += 2.0;
	
	// height is never clipped
	bbox_height = height;
	
	// width is only clipped if TextWrapping is set to NoWrap
	if (bbox_width == -1.0)
		bbox_width = width;
	
	//printf ("layout extents are %.3f, %.3f, bounding box extents are %.3f, %.3f\n", width, height, bbox_width, bbox_height);
}

static inline void
RenderLine (cairo_t *cr, UIElement *element, TextLine *line, Brush *default_fg, double x, double y)
{
	TextFont *font = NULL;
	TextDecorations deco;
	TextSegment *segment;
	const gunichar *text;
	gunichar prev = 0;
	GlyphInfo *glyph;
	double x1, y1;
	double x0, y0;
	Brush *fg;
	int i;
	
	// set y0 to the line's baseline (descend is a negative value)
	y0 = y + line->height + line->descend;
	
	x0 = x;
	
	segment = (TextSegment *) line->segments->First ();
	
	while (segment) {
		text = segment->run->text;
		deco = segment->run->deco;
		font = segment->run->font;
		
		cairo_save (cr);
		cairo_translate (cr, x0, y0 - font->Ascender ());
		
		// set y1 to the baseline relative to the translation matrix
		y1 = font->Ascender ();
		x1 = 0.0;
		
		if (segment->run->fg && *segment->run->fg)
			fg = *segment->run->fg;
		else
			fg = default_fg;
		
		fg->SetupBrush (cr, element, segment->width, font->Height ());
		
		if (!segment->path) {
			if (font->IsScalable ())
				cairo_new_path (cr);
			
			for (i = segment->start, prev = 0; i < segment->end; i++) {
				if (!(glyph = font->GetGlyphInfo (text[i])))
					continue;
				
				if (prev != 0)
					x1 += font->Kerning (prev, glyph->index);
				else if (glyph->metrics.horiBearingX < 0)
					x1 -= glyph->metrics.horiBearingX;
				
				prev = glyph->index;
				
				if (!font->IsScalable ())
					font->Render (cr, glyph, x1, y1);
				else
					font->Path (cr, glyph, x1, y1);
				
				x1 += glyph->metrics.horiAdvance;
			}
			
			if (font->IsScalable ()) {
				cairo_close_path (cr);
				segment->path = cairo_copy_path (cr);
				cairo_fill (cr);
			}
		} else {
			// it is an error to append a path with no data
			if (segment->path->data)
				cairo_append_path (cr, segment->path);
			
			x1 = segment->width;
			cairo_fill (cr);
		}
		
		if (deco == TextDecorationsUnderline && segment->uwidth > 0.0) {
			cairo_antialias_t aa = cairo_get_antialias (cr);
			double thickness = font->UnderlineThickness ();
			double pos = y1 + font->UnderlinePosition ();
			
			cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
			cairo_set_line_width (cr, thickness);
			
			cairo_new_path (cr);
			cairo_move_to (cr, 0.0, pos);
			cairo_line_to (cr, segment->uwidth, pos);
			cairo_stroke (cr);
			
			cairo_set_antialias (cr, aa);
		}
		
		segment = (TextSegment *) segment->next;
		cairo_restore (cr);
		x0 += x1;
	}
}

void
TextLayout::Render (cairo_t *cr, UIElement *element, Brush *default_fg, double x, double y)
{
	TextLine *line;
	double y1 = y;
	
	Layout ();
	
	line = (TextLine *) lines->First ();
	
	while (line) {
		RenderLine (cr, element, line, default_fg, x, y1);
		y1 += (double) line->height;
		
		line = (TextLine *) line->next;
	}
}
#endif

