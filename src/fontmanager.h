/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fontmanager.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_MANAGER_H__
#define __FONT_MANAGER_H__

#include <glib.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "moon-path.h"
#include "collection.h"
#include "enums.h"

namespace Moonlight {

struct ManagedStreamCallbacks;
class FontManager;
class FontFace;

bool FontWeightIsBold (FontWeights weight);

struct GlyphMetrics {
	double horiBearingX;
	//double horiBearingY;
	double horiAdvance;
	//double height;
	//double width;
};

struct GlyphInfo {
	GlyphMetrics metrics;
	gunichar unichar;
	guint32 index;
	moon_path *path;
	FontFace *face;
	gint64 atime;
};

struct FontFaceExtents {
	double underline_thickness;
	double underline_position;
	double descent;
	double ascent;
	double height;
};


class FontFace {
	FontManager *manager;
	double cur_size;
	int ref_count;
	FT_Face face;
	char *key;
	
 public:
	FontFace (FontManager *manager, FT_Face face, char *key);
	~FontFace ();
	
	void ref ();
	void unref ();
	
	const char *GetFamilyName ();
	const char *GetStyleName ();
	
	bool GetVersion (int *major, int *minor);
	
	bool IsScalable ();
	bool IsItalic ();
	bool IsBold ();
	
	gunichar GetCharFromIndex (guint32 index);
	guint32 GetCharIndex (gunichar unichar);
	bool HasChar (gunichar unichar);
	
	void GetExtents (double size, bool gapless, FontFaceExtents *extents);
	double Kerning (double size, guint32 left, guint32 right);
	bool LoadGlyph (double size, GlyphInfo *glyph, StyleSimulations simulate = StyleSimulationsNone);
};

/* @IncludeInKinds */
/* @Namespace=None */
class GlyphTypeface {
	char *family_name;
	FontStretches stretch;
	FontWeights weight;
	FontStyles style;
	char *resource;
	int ver_major;
	int ver_minor;
	
 public:
	GlyphTypeface (const char *path, int index, FontFace *face);
	GlyphTypeface (const GlyphTypeface *typeface);
	~GlyphTypeface ();
	
	//
	// Accessors meant for FontManager's use
	//
	const char *GetFontResource () const { return resource; }
	const char *GetFamilyName () const { return family_name; }
	FontStretches GetFontStretch () const { return stretch; }
	FontWeights GetFontWeight () const { return weight; }
	FontStyles GetFontStyle () const { return style; }
	
	//
	// Public Accessors
	//
	int GetMajorVersion () const { return ver_major; }
	int GetMinorVersion () const { return ver_minor; }
	/* @GenerateCBinding,GeneratePInvoke */
	double GetVersion () const;
	
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetFontUri () const;
	
	bool operator== (const GlyphTypeface &v) const;
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media */
class GlyphTypefaceCollection : public Collection {
 protected:
	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value, bool is_value_safe);

	virtual ~GlyphTypefaceCollection () { }
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	GlyphTypefaceCollection ();

	virtual void OnIsAttachedChanged (bool attached);

	virtual Type::Kind GetElementType () { return Type::GLYPHTYPEFACE; }
};

enum FontResourceType {
	FontResourceTypeResourceId,
	FontResourceTypeGlyphTypeface,
};

class FontResource {
	FontResourceType type;
	union {
		GlyphTypeface *typeface;
		char *id;
	} resource;
	
 public:
	FontResource (const FontResource *resource);
	FontResource (const GlyphTypeface *typeface);
	FontResource (const char *id);
	~FontResource ();
	
	FontResourceType GetType () const { return type; }
	
	const GlyphTypeface *GetGlyphTypeface () const { return resource.typeface; }
	const char *GetId () const { return resource.id; }
	
	bool operator== (const FontResource &v) const;
};

class FontManager {
	friend class FontFace;
	
	GlyphTypefaceCollection *typefaces;
	GHashTable *system_faces;
	GHashTable *resources;
	GHashTable *faces;
	FT_Library libft2;
	char *root;
	double dpi;
	
	FontFace *OpenFontResource (const char *resource, const char *family, int index, FontStretches stretch, FontWeights weight, FontStyles style);
	FontFace *OpenSystemFont (const char *family, FontStretches stretch, FontWeights weight, FontStyles style);
	FontFace *OpenFontFace (const char *filename, const char *guid, int index);
	
 public:
	FontManager ();
	~FontManager ();
	
	void AddResource (const char *resource_id, const char *path);
	FontResource *AddResource (ManagedStreamCallbacks *stream);
	
	FontFace *OpenFont (const char *name, FontStretches stretch, FontWeights weight, FontStyles style);
	FontFace *OpenFont (const char *name, int index);
	FontFace *OpenFont (const GlyphTypeface *typeface);
	
	/* @GenerateCBinding,GeneratePInvoke */
	GlyphTypefaceCollection *GetSystemGlyphTypefaces ();
};

};
#endif /* __FONT_MANAGER_H__ */
