/*
 * playlist.cpp: 
 *
 * Author: Jb Evain <jbevain@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <glib.h>
#include <gtk/gtk.h>

#include "playlist.h"
#include "downloader.h"

// Playlist

Playlist::Playlist (MediaElement *element, char *source_name)
	: MediaSource (element, source_name)
{
	items = new List ();
}

Playlist::~Playlist ()
{
	delete items;
}

bool
Playlist::Open ()
{
	if (!Parse ())
		return false;

	g_warning ("playlists are not implemented");
	return false;
}

bool
Playlist::IsPlaylistFile (const char * file_name)
{
	static const char *exts [] = {".asx", ".wax", ".wvx", ".wmx"};

	for (int i = 0; i < 4; i++)
		if (g_str_has_suffix (file_name, exts [i]))
			return true;

	return false;
}

void
Playlist::parser_on_start_element (GMarkupParseContext *context, const gchar *element_name,
								   const gchar **attributes_names, const gchar **attributes_values,
								   gpointer user_data, GError **error)
{
}

void
Playlist::parser_on_end_element (GMarkupParseContext *context, const gchar *element_name,
								 gpointer user_data, GError **error)
{
}

void
Playlist::parser_on_text (GMarkupParseContext *context, const gchar *text, gsize text_len,
						 gpointer user_data, GError **error)
{
}

void
Playlist::parser_on_error (GMarkupParseContext *context, GError *error, gpointer user_data)
{
}

bool
Playlist::Parse ()
{
	static const GMarkupParser PlaylistParser = {
		parser_on_start_element,
		parser_on_end_element,
		parser_on_text,
		NULL, // passthrough
		parser_on_error,
	};

	GMarkupParseContext *context;
	gchar *text;
	gsize len;
	bool error = false;

	if (!g_file_get_contents (source_name, &text, &len, NULL))
		return false;

	context = g_markup_parse_context_new (&PlaylistParser, (GMarkupParseFlags) 0, this, NULL);
	if (!g_markup_parse_context_parse (context, text, len, NULL)) {
		error = true;
		g_markup_parse_context_end_parse (context, NULL);
	}

	g_markup_parse_context_free (context);
	g_free (text);

	return !error;
}
