/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fonts-gtk.cpp: different types of collections
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>

#include "pal-android.h"
#include "font-utils.h"

namespace Moonlight {

struct AndroidFont {
	FontStyleInfo style;
	char *path;
	int index;
	
	AndroidFont (FT_Face face, const char *path, int index)
	{
		font_style_info_init (&style, NULL);
		
		// extract whatever little style info we can from the family name
		font_style_info_parse (&style, face->family_name, true);
		
		// style info parsed from style_name overrides anything we got from family_name
		font_style_info_parse (&style, face->style_name, false);
		
		this->path = g_strdup (path);
		this->index = index;
	}
	
	~AndroidFont ()
	{
		g_free (style.family_name);
		g_free (path);
	}
};

MoonFontServiceAndroid::MoonFontServiceAndroid ()
{
	AndroidFont *font;
	const char *name;
	GString *path;
	FT_Long i, n;
	FT_Face face;
	size_t len;
        GDir *dir;
	
	system_fonts = g_ptr_array_new ();
	
	if (!(dir = g_dir_open ("/system/fonts", 0, NULL)))
		return;
	
	path = g_string_new ("/system/fonts/");
	len = path->len;
	
	// scan all system fonts
	while ((name = g_dir_read_name (dir))) {
		if (!strcmp (name, ".") || !strcmp (name, ".."))
			continue;
		
		g_string_truncate (path, len);
		g_string_append (path, name);
		
		if (FT_New_Face (libft2, path->str, 0, &face) != 0)
			continue;
		
		font = new AndroidFont (face, path->str, 0);
		g_ptr_array_add (system_fonts, font);
		
		n = face->num_faces;
		FT_Done_Face (face);
		
		for (i = 1; i < n; i++) {
			if (FT_New_Face (libft2, path->str, i, &face) != 0)
				continue;
			
			font = new AndroidFont (face, path->str, 0);
			g_ptr_array_add (system_fonts, font);
			
			FT_Done_Face (face);
		}
	}
	
	g_string_free (path, true);
	
	g_dir_close (dir);
}

MoonFontServiceAndroid::~MoonFontServiceAndroid ()
{
	for (guint i = 0; i < system_fonts->len; i++)
		delete (AndroidFont *) system_fonts->pdata[i];
	
	g_ptr_array_free (system_fonts, true);
}

MoonFont *
MoonFontServiceAndroid::FindFont (const FontStyleInfo *desired)
{
	AndroidFont *font, *best = NULL;
	int diff, closest = G_MAXINT;
	const char *family_name;
	
	if (!g_ascii_strcasecmp (desired->family_name, "Sans"))
		family_name = "Droid Sans Fallback";
	else
		family_name = desired->family_name;
	
	for (guint i = 0; i < system_fonts->len; i++) {
		font = (AndroidFont *) system_fonts->pdata[i];
		
		if (!g_ascii_strcasecmp (font->style.family_name, family_name)) {
			diff = font_style_info_diff (&font->style, desired);
			if (diff < closest) {
				closest = diff;
				best = font;
			}
		}
	}
	
	if (best != NULL)
		return new MoonFont (best->path, best->index);
	
	return NULL;
}

void
MoonFontServiceAndroid::ForeachFont (MoonForeachFontCallback foreach, gpointer user_data)
{
	AndroidFont *font;
	
	for (guint i = 0; i < system_fonts->len; i++) {
		font = (AndroidFont *) system_fonts->pdata[i];
		
		if (!foreach (font->path, font->index, user_data))
			break;
	}
}

};

