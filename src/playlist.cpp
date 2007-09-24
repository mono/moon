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

// Playlist

Playlist::Playlist (MediaElement *element, const char *source_name, const char *file_name)
	: MediaSource (element, source_name, file_name)
{
	entries = new List ();
	current_entry = NULL;
	downloader = new Downloader ();
	downloader->ref ();
	downloader->SetWriteFunc (on_downloader_data_write, on_downloader_size_notify, this);
	downloader->AddHandler (downloader->CompletedEvent, on_downloader_complete, this);
	element->AddHandler(element->MediaEndedEvent, on_media_ended, this);
}

Playlist::~Playlist ()
{
	entries->Clear (true);
	downloader->unref ();
	delete entries;
	delete downloader;
}

bool
Playlist::Parse ()
{
	gchar *text;
	gsize len;
	bool error;

	if (!g_file_get_contents (file_name, &text, &len, NULL))
		return false;

	PlaylistParser *parser = new PlaylistParser (this);
	error = !parser->Parse (text, len);

	delete parser;
	g_free (text);

	return !error;
}

void
Playlist::OnMediaEnded ()
{
	if (!current_entry || !current_entry->next)
		return;

	OpenEntry (dynamic_cast<PlaylistEntry *> (current_entry->next));
	if (HasMediaSource ())
		current_entry->GetSource ()->Play ();
}

void
Playlist::OnMediaDownloaded ()
{
	if (current_entry == NULL)
		return;

	char *file_name = downloader->GetResponseFile ("");
	if (file_name == NULL)
		return;

	printf ("OnMediaDownloaded, file: %s\n", file_name);

	MediaSource *source = MediaSource::CreateSource (element, current_entry->GetSourceName (), file_name);
	current_entry->SetSource (source);
	if (source->Open ())
		Play ();
}

bool
Playlist::IsPlaylistFile (const char * file_name)
{
	static const char *exts [] = {".asx", ".wax", ".wvx", ".wmx"};

	if (!file_name)
		return false;

	for (int i = 0; i < 4; i++)
		if (g_str_has_suffix (file_name, exts [i]))
			return true;

	return false;
}

void
Playlist::on_media_ended (EventObject *sender, gpointer calldata, gpointer userdata)
{
	Playlist *playlist = reinterpret_cast<Playlist *> (userdata);
	playlist->OnMediaEnded ();
}

void
Playlist::on_downloader_complete (EventObject *sender, gpointer calldata, gpointer userdata)
{
	Playlist *playlist = reinterpret_cast<Playlist *> (userdata);
	playlist->OnMediaDownloaded ();
}

void
Playlist::on_downloader_data_write (guchar *buf, gsize offset, gsize count, gpointer data)
{
}

void
Playlist::on_downloader_size_notify (int64_t size, gpointer data)
{
}

static bool
check_base (GString *uri, PlaylistContent *content)
{
	const char *base;

	base = content->GetBase ();

	if (base == NULL || !strlen (base))
		return false;

	g_string_append (uri, base);
	g_string_append_c (uri, '/');
	return true;
}

bool
Playlist::OpenEntry (PlaylistEntry *entry)
{
	GString *uri;
	MediaSource *source;

	current_entry = entry;

	source = current_entry->GetSource ();

	// if the source if already downloaded, no need to go further
	if (source)
		return source->Open ();

	media_element_set_current_state (element, "Buffering");

	uri = g_string_new ("");
	if (!check_base (uri, entry))
		check_base (uri, this);

	g_string_append (uri, entry->GetSourceName ());

	printf ("open entry at uri: %s\n", uri->str);

	downloader->Open ("GET", uri->str);
	downloader->Send ();

	g_string_free (uri, true);

	return true;
}

bool
Playlist::OpenSource ()
{
	if (!Parse ())
		return false;

	if (entries->Length () == 0)
		return false;

	return OpenEntry (dynamic_cast<PlaylistEntry *> (entries->First ()));
}

bool
Playlist::HasMediaSource ()
{
	if (current_entry == NULL)
		return false;

	return current_entry->GetSource () != NULL;
}

void
Playlist::Play ()
{
	if (!HasMediaSource ())
		return;

	current_entry->GetSource ()->Play ();
}

void
Playlist::Pause ()
{
	if (!HasMediaSource ())
		return;

	current_entry->GetSource ()->Pause ();
}

void
Playlist::Stop ()
{
	if (!HasMediaSource ())
		return;

	current_entry->GetSource ()->Stop ();

	OpenEntry (dynamic_cast<PlaylistEntry *> (entries->First ()));
}

void
Playlist::Close ()
{
	if (!HasMediaSource ())
		return;

	current_entry->GetSource ()->Close ();
}

void
Playlist::AddEntry (PlaylistEntry *entry)
{
	entries->Append (entry);
}

// PlaylistParser

PlaylistParser::PlaylistParser (Playlist *playlist)
{
	this->playlist = playlist;
	current_entry = NULL;

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

static bool
str_match (const char *candidate, const char *tag)
{
	return g_strcasecmp (candidate, tag) == 0;
}

static bool
has_href (const char **attrs)
{
	return attrs && str_match (*attrs, "HREF");
}

static char *
get_href (const char **attrs)
{
	return g_strdup (*(attrs + 1));
}

void
PlaylistParser::on_start_element (gpointer user_data, const char *name, const char **attrs)
{
	PlaylistNodeKind kind;
	PlaylistParser *parser = reinterpret_cast<PlaylistParser *> (user_data);

	//printf ("on_start_element, name: %s\n", name);
	if (str_match (name, "ABSTRACT")) {
		kind = Abstract;
	} else if (str_match (name, "ASX")) {
		kind = Asx;
	} else if (str_match (name, "AUTHOR")) {
		kind = Author;
	} else if (str_match (name, "BANNER")) {
		kind = Banner;
	} else if (str_match (name, "BASE")) {
		kind = Base;
		if (has_href (attrs))
			parser->GetCurrentContent ()->SetBase (get_href (attrs));
	} else if (str_match (name, "COPYRIGHT")) {
		kind = Copyright;
	} else if (str_match (name, "DURATION")) {
		kind = Duration;
	} else if (str_match (name, "ENTRY")) {
		kind = Entry;
		parser->OnEntry ();
	} else if (str_match (name, "ENTRYREF")) {
		kind = EntryRef;
	} else if (str_match (name, "LOGURL")) {
		kind = LogUrl;
	} else if (str_match (name, "MOREINFO")) {
		kind = MoreInfo;
	} else if (str_match (name, "STARTTIME")) {
		kind = StartTime;
	} else if (str_match (name, "REF")) {
		kind = Ref;
		if (has_href (attrs))
			parser->GetCurrentEntry ()->SetSourceName (get_href (attrs));
	} else if (str_match (name, "TITLE")) {
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
		break;
	default:
		break;
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
	if (current_entry != NULL)
		return current_entry;

	return playlist;
}

PlaylistEntry *
PlaylistParser::GetCurrentEntry ()
{
	return current_entry;
}

void
PlaylistParser::OnEntry ()
{
	PlaylistEntry *entry = new PlaylistEntry ();
	playlist->AddEntry (entry);
	current_entry = entry;
}

void PlaylistParser::EndEntry ()
{
	this->current_entry = NULL;
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
	if (GetParentKind () & kind)
		return;

	XML_StopParser (parser, false);
	printf ("AssertParentKind, current: %d kind %d\n", GetParentKind (), kind);
}
