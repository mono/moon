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
#include <string.h>

#include "playlist.h"
#include "downloader.h"
#include "xaml.h"

// PlaylistContent

PlaylistContent::PlaylistContent ()
{
	base = NULL;
	title = NULL;
	author = NULL;
	abstract = NULL;
}

PlaylistContent::~PlaylistContent ()
{
	g_free (base);
	g_free (title);
	g_free (author);
	g_free (abstract);
}

const char *
PlaylistContent::GetBase ()
{
	return base;
}

void
PlaylistContent::SetBase (char *base)
{
	this->base = base;
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

const char *
PlaylistContent::GetCopyright ()
{
	return copyright;
}

void
PlaylistContent::SetCopyright (char *copyright)
{
	this->copyright = copyright;
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

gint64
PlaylistEntry::GetDuration ()
{
	return duration;
}

void
PlaylistEntry::SetDuration (gint64 duration)
{
	this->duration = duration;
}

// Playlist

Playlist::Playlist (MediaElement *element, char *source_name)
	: MediaSource (element, source_name)
{
	entries = new List ();
}

Playlist::~Playlist ()
{
	entries->Clear (true);
	delete entries;
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

void
Playlist::AddEntry (PlaylistEntry *entry)
{
	entries->Append (entry);
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
	if (!parser->Parse (text, len))
		error = true;

	delete parser;
	g_free (text);

	return !error;
}

// PlaylistParser

PlaylistParser::PlaylistParser (Playlist *playlist)
{
	this->playlist = playlist;
	entry = NULL;

	parser = XML_ParserCreate (NULL);
	kind_stack = new List ();
	PushCurrentKind (Root);

	XML_SetUserData (parser, this);
	XML_SetElementHandler (parser, on_start_element, on_end_element);
	XML_SetCharacterDataHandler (parser, on_text);
}

PlaylistParser::~PlaylistParser ()
{
	kind_stack->Clear (true);
	delete kind_stack;
	XML_ParserFree (parser);
}

void
PlaylistParser::on_start_element (gpointer user_data, const char *name, const char **attrs)
{
	PlaylistParser *parser = reinterpret_cast<PlaylistParser *> (user_data);
	PlaylistNodeKind kind;
	//printf ("on_start_element, name: %s\n", name);
	if (!strcmp (name, "ABSTRACT")) {
		kind = Abstract;
	} else if (!strcmp (name, "ASX")) {
		kind = Asx;
	} else if (!strcmp (name, "AUTHOR")) {
		kind = Author;
	} else if (!strcmp (name, "BANNER")) {
		kind = Banner;
	} else if (!strcmp (name, "BASE")) {
		kind = Base;
	} else if (!strcmp (name, "COPYRIGHT")) {
		kind = Copyright;
	} else if (!strcmp (name, "DURATION")) {
		kind = Duration;
	} else if (!strcmp (name, "ENTRY")) {
		kind = Entry;
	} else if (!strcmp (name, "ENTRYREF")) {
		kind = EntryRef;
	} else if (!strcmp (name, "LOGURL")) {
		kind = LogUrl;
	} else if (!strcmp (name, "MOREINFO")) {
		kind = MoreInfo;
	} else if (!strcmp (name, "STARTTIME")) {
		kind = StartTime;
	} else if (!strcmp (name, "REF")) {
		kind = Ref;
		parser->GetCurrentEntry ()->SetSource (NULL); // get source from href attribute
	} else if (!strcmp (name, "TITLE")) {
		kind = Title;
	} else {
		kind = Unknown;
	}

	parser->PushCurrentKind (kind);
}

void
PlaylistParser::on_end_element (gpointer user_data, const char *name)
{
	PlaylistParser *parser = reinterpret_cast<PlaylistParser *> (user_data);
	switch (parser->GetCurrentKind ()) {
	case Entry:
		parser->EndEntry ();
	}
	parser->PopCurrentKind ();
}

void
PlaylistParser::on_text (gpointer user_data, const char *data, int len)
{
	PlaylistParser *parser = reinterpret_cast<PlaylistParser *> (user_data);
	//printf ("on_text, text: %s\n", g_strndup (data, len));
	switch (parser->GetCurrentKind ()) {
	case Abstract:
		parser->AssertParentKind (Asx | Entry);
		parser->GetCurrentContent ()->SetAbstract (g_strndup (data, len));
		break;
	case Author:
		parser->AssertParentKind (Asx | Entry);
		parser->GetCurrentContent ()->SetAuthor (g_strndup (data, len));
		break;
	case Base:
		parser->AssertParentKind (Asx | Entry);
		parser->GetCurrentContent ()->SetBase (g_strndup (data, len));
		break;
	case Copyright:
		parser->AssertParentKind (Asx | Entry);
		parser->GetCurrentContent ()->SetCopyright (g_strndup (data, len));
		break;
	case Duration:
		parser->AssertParentKind (Entry);
		parser->GetCurrentEntry ()->SetDuration (timespan_from_str (g_strndup (data, len)));
		break;
	case Entry:
		parser->AssertParentKind (Asx);
		parser->OnEntry ();
		break;
	case StartTime:
		parser->AssertParentKind (Entry);
		parser->GetCurrentEntry ()->SetStartTime (timespan_from_str (g_strndup (data, len)));
		break;
	case Title:
		parser->AssertParentKind (Asx | Entry);
		parser->GetCurrentContent ()->SetTitle (g_strndup (data, len));
		break;
	default:
		break;
	}
}

bool
PlaylistParser::Parse (const char *text, int len)
{
	if (XML_Parse (parser, text, len, TRUE))
		return true;

	g_warning ("parsing error: %s\n", XML_ErrorString (XML_GetErrorCode (parser)));
	return false;
}

PlaylistContent *
PlaylistParser::GetCurrentContent ()
{
	if (entry != NULL)
		return entry;

	return playlist;
}

PlaylistEntry *
PlaylistParser::GetCurrentEntry ()
{
	return entry;
}

void
PlaylistParser::OnEntry ()
{
	PlaylistEntry *entry = new PlaylistEntry ();
	playlist->AddEntry (entry);
	this->entry = entry;
}

void PlaylistParser::EndEntry ()
{
	this->entry = NULL;
}

void
PlaylistParser::PushCurrentKind (PlaylistParser::PlaylistNodeKind kind)
{
	kind_stack->Append (new KindNode (kind));
}

void
PlaylistParser::PopCurrentKind ()
{
	kind_stack->Remove (kind_stack->Last ());
}

PlaylistParser::PlaylistNodeKind
PlaylistParser::GetCurrentKind ()
{
	KindNode *node = dynamic_cast<KindNode *> (kind_stack->Last ());
	return node->kind;
}

PlaylistParser::PlaylistNodeKind
PlaylistParser::GetParentKind ()
{
	KindNode *node = dynamic_cast<KindNode *> (kind_stack->Last ()->prev);
	return node->kind;
}

void
PlaylistParser::AssertParentKind (int kind)
{
	printf ("AssertParentKind, current: %d kind %d\n", GetParentKind (), kind);
	g_assert (GetParentKind () & kind);
}
