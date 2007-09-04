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
#include <expat.h>

#include "playlist.h"
#include "downloader.h"

// PlaylistContent

PlaylistContent::PlaylistContent ()
{
	title = NULL;
	author = NULL;
	abstract = NULL;
}

PlaylistContent::~PlaylistContent ()
{
	g_free (title);
	g_free (author);
	g_free (abstract);
}

const char *
PlaylistContent::GetTitle ()
{
	return title;
}

void
PlaylistContent::SetTitle (char *title)
{
	this->title = title;
}

const char *
PlaylistContent::GetAuthor ()
{
	return author;
}

void
PlaylistContent::SetAuthor (char *author)
{
	this->author = author;
}

const char *
PlaylistContent::GetAbstract ()
{
	return abstract;
}

void
PlaylistContent::SetAbstract (char *abstract)
{
	this->abstract = abstract;
}

// PlaylistEntry

PlaylistEntry::PlaylistEntry ()
{
	source = NULL;
	start_time = 0;
}

PlaylistEntry::~PlaylistEntry ()
{
	g_free (source);
}

const char *
PlaylistEntry::GetSource ()
{
	return source;
}

void
PlaylistEntry::SetSource (char *source)
{
	this->source = source;
}

gint64
PlaylistEntry::GetStartTime ()
{
	return start_time;
}

void
PlaylistEntry::SetStartTime (gint64 start_time)
{
	this->start_time = start_time;
}

bool
playlist_entry_free (List::Node *node, gpointer data)
{
	PlaylistEntry *entry = dynamic_cast<PlaylistEntry *> (node);
	delete entry;

	return true;
}

// Playlist

Playlist::Playlist (MediaElement *element, char *source_name)
	: MediaSource (element, source_name)
{
	items = new List ();
}

Playlist::~Playlist ()
{
	items->ForEach (playlist_entry_free, NULL);

	delete items;
}

bool
Playlist::Open ()
{
	if (!Parse ())
		return false;

	g_warning ("playlists are not implemented\n");
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

bool
Playlist::Parse ()
{
	gchar *text;
	gsize len;
	bool error = false;

	if (!g_file_get_contents (source_name, &text, &len, NULL))
		return false;

	PlaylistParser *parser = new PlaylistParser (this);
	parser->Parse (text, len);

	delete parser;
	g_free (text);

	return !error;
}

// PlaylistParser

PlaylistParser::PlaylistParser (Playlist *playlist)
{
	this->playlist = playlist;

	parser = XML_ParserCreate (NULL);

	XML_SetElementHandler (parser, on_start_element, on_end_element);
	XML_SetCharacterDataHandler (parser, on_text);
}

PlaylistParser::~PlaylistParser ()
{
	XML_ParserFree (parser);

	g_free (base);
}

void
PlaylistParser::on_start_element (gpointer user_data, const char *name, const char **attrs)
{
//	PlaylistParser *parser = reinterpret_cast<PlaylistParser *> (user_data);
}

void
PlaylistParser::on_end_element (gpointer user_data, const char *name)
{
//	PlaylistParser *parser = reinterpret_cast<PlaylistParser *> (user_data);
}

void
PlaylistParser::on_text (gpointer user_data, const char *text, int len)
{
//	PlaylistParser *parser = reinterpret_cast<PlaylistParser *> (user_data);
}

void
PlaylistParser::Parse (const char *text, int len)
{
	if (XML_Parse (parser, text, len, TRUE))
		return;

	g_warning ("parsing error: %s\n", XML_ErrorString (XML_GetErrorCode (parser)));
}
