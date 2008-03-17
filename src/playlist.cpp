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
#include "runtime.h"

#define LOG_PLAYLISTS(...) printf (__VA_ARGS__);

/*
 * PlaylistNode
 */

PlaylistNode::PlaylistNode (PlaylistEntry *entry) : List::Node () 
{
	if (entry)
		entry->ref ();
	this->entry = entry;
}

PlaylistNode::~PlaylistNode ()
{
	if (entry) {
		entry->unref ();
		entry = NULL;
	}
}

/*
 * PlaylistEntry
 */

PlaylistEntry::PlaylistEntry (MediaElement *element, Playlist *parent, Media *media)
{
	LOG_PLAYLISTS ("PlaylistEntry::PlaylistEntry (%p, %p, %p)\n", element, parent, media);

	this->parent = parent;
	this->element = element;
	this->media = media;
	if (this->media)
		this->media->ref ();
	source_name = NULL;
	full_source_name = NULL;
	start_time = 0;
	duration = 0;
	play_when_available = true;
	base = NULL;
	title = NULL;
	author = NULL;
	abstract = NULL;
	copyright = NULL;
}


PlaylistEntry::~PlaylistEntry ()
{
	LOG_PLAYLISTS ("PlaylistEntry::~PlaylistEntry ()\n");

	g_free (source_name);
	g_free (full_source_name);

	if (media)
		media->unref ();

	g_free (base);
	g_free (title);
	g_free (author);
	g_free (abstract);
	g_free (copyright);
}

static void
add_attribute (MediaAttributeCollection *attributes, const char *name, const char *attr)
{
	if (!attr)
		return;

	MediaAttribute *attribute = new MediaAttribute ();
	dependency_object_set_name (attribute, name);
	media_attribute_set_value (attribute, g_strdup (attr));

	attributes->Add (attribute);
	attribute->unref ();
}

void
PlaylistEntry::PopulateMediaAttributes ()
{
	LOG_PLAYLISTS ("PlaylistEntry::PopulateMediaAttributes ()\n");

	const char *abstract = NULL;
	const char *author = NULL;
	const char *copyright = NULL;
	const char *title = NULL;
	PlaylistEntry *current = this;
	MediaAttributeCollection *attributes;

	Value *value = element->GetValue (MediaElement::AttributesProperty);
	if (!value) {
		attributes = new MediaAttributeCollection ();
		element->SetValue (MediaElement::AttributesProperty, Value (attributes));
	} else {
		attributes = value->AsMediaAttributeCollection ();
		attributes->Clear ();
	}

	while (current != NULL) {
		if (abstract == NULL)
			abstract = current->GetAbstract ();
		if (author == NULL)
			author = current->GetAuthor ();
		if (copyright == NULL)
			copyright = current->GetCopyright ();
		if (title == NULL)
			title = current->GetTitle ();		

		current = current->GetParent ();
	}

	add_attribute (attributes, "Abstract", abstract);
	add_attribute (attributes, "Author", author);
	add_attribute (attributes, "Copyright", copyright);
	add_attribute (attributes, "Title", title);
}

const char *
PlaylistEntry::GetFullSourceName ()
{
	if (full_source_name == NULL) {
		const char *base = GetBase ();
		GString *uri = g_string_sized_new (strlen (source_name) + 25);

		if (base != NULL && *base != 0) {
			g_string_append (uri, base);
			if (!g_str_has_suffix (uri->str, "/"))
				g_string_append (uri, "/");
		}
		g_string_append (uri, source_name);
		full_source_name = g_strdup (uri->str);
		g_string_free (uri, true);
	}
	return full_source_name;
}

void
PlaylistEntry::Open ()
{
	LOG_PLAYLISTS ("PlaylistEntry::Open (), media = %p, FullSourceName = %s\n", media, GetFullSourceName ());

	if (media != NULL)
		return;

	Downloader *dl = element->GetSurface ()->CreateDownloader ();
	dl->Open ("GET", GetFullSourceName ());
	element->SetSourceInternal (dl, NULL);
	dl->unref ();
}

bool
PlaylistEntry::Play ()
{
	LOG_PLAYLISTS ("PlaylistEntry::Play (), play_when_available: %s, media: %p, source name: %s\n", play_when_available ? "true" : "false", media, source_name);

	if (media == NULL) {
		play_when_available = true;
		Open ();
		return false;
	}

	element->SetMedia (media);
	element->PlayInternal ();

	play_when_available = false;

	return true;
}

bool
PlaylistEntry::Pause ()
{
	LOG_PLAYLISTS ("PlaylistEntry::Pause ()\n");
	
	play_when_available = false;
	element->GetMediaPlayer ()->Pause ();
	return true;
}

void
PlaylistEntry::Stop ()
{
	LOG_PLAYLISTS ("PlaylistEntry::Stop ()\n");

	play_when_available = false;
	element->GetMediaPlayer ()->Stop ();
}

Media *
PlaylistEntry::GetMedia ()
{
	return media;
}

void 	
PlaylistEntry::SetMedia (Media *media)
{
	LOG_PLAYLISTS ("PlaylistEntry::SetMedia (%p), previous media: %p\n", media, this->media);

	if (this->media)
		this->media->unref ();

	this->media = media;
	this->media->ref ();

	if (play_when_available && element->GetState () != MediaElement::Buffering)
		Play ();
}

/*
 * Playlist
 */

Playlist::Playlist (MediaElement *element, IMediaSource *source)
	: PlaylistEntry (element, NULL)
{
	is_single_file = false;
	Init (element);
	this->source = source;
}

Playlist::Playlist (MediaElement *element, Media *media)
	: PlaylistEntry (element, NULL, media)
{
	is_single_file = true;
	Init (element);

	AddEntry (new PlaylistEntry (element, this, media));
	current_node = (PlaylistNode *) entries->First ();
}

Playlist::~Playlist ()
{
	LOG_PLAYLISTS ("Playlist::~Playlist ()\n");

	delete entries;
	element->RemoveHandler (element->MediaEndedEvent, on_media_ended, this);
}

void
Playlist::Init (MediaElement *element)
{	
	LOG_PLAYLISTS ("Playlist::Init (%p)\n", element);

	this->element = element;
	entries = new List ();
	current_node = NULL;
	source = NULL;
	element->AddHandler(element->MediaEndedEvent, on_media_ended, this);
}

void
Playlist::Open ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLISTS ("Playlist::Open ()\n");
	
	current_node = (PlaylistNode *) entries->First ();

	current_entry = GetCurrentEntry ();	
	if (current_entry)
		current_entry->Open ();

	LOG_PLAYLISTS ("Playlist::Open (): current node: %p, current entry: %p\n", current_entry, GetCurrentEntry ());
}

void
Playlist::OnMediaEnded ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLISTS ("Playlist::OnMediaEnded () current_node: %p, source: %s\n", current_node, ((PlaylistNode*) current_node)->GetEntry ()->GetSourceName () );

	if (!current_node)
		return;

	current_node = (PlaylistNode *) current_node->next;

	current_entry = GetCurrentEntry ();
	if (current_entry)
		current_entry->Play ();

	LOG_PLAYLISTS ("Playlist::OnMediaEnded () current_node: %p [Done]\n", current_node);
}

void
Playlist::on_media_ended (EventObject *sender, EventArgs *calldata, gpointer userdata)
{
	Playlist *playlist = (Playlist *) userdata;
	playlist->OnMediaEnded ();
}

bool
Playlist::Play ()
{
	if (current_node == NULL)
		current_node = (PlaylistNode *) entries->First ();	

	PlaylistEntry *current_entry = GetCurrentEntry ();

	LOG_PLAYLISTS ("Playlist::Play (), current entry: %p\n", current_entry);

	if (current_entry)
		return current_entry->Play ();

	return false;
}

bool
Playlist::Pause ()
{
	PlaylistEntry *current_entry = GetCurrentEntry ();

	LOG_PLAYLISTS ("Playlist::Pause ()\n");

	if (!current_entry)
		return false;

	return current_entry->Pause ();
}

void
Playlist::Stop ()
{
	PlaylistEntry *current_entry = GetCurrentEntry ();

	LOG_PLAYLISTS ("Playlist::Stop ()\n");

	current_node = (PlaylistNode *) entries->First ();

	if (!current_entry)
		return;

	current_entry->Stop ();
}

void
Playlist::PopulateMediaAttributes ()
{
	PlaylistEntry *current_entry = GetCurrentEntry ();

	LOG_PLAYLISTS ("Playlist::PopulateMediaAttributes ()\n");

	if (!current_entry)
		return;

	current_entry->PopulateMediaAttributes ();
}

void
Playlist::AddEntry (PlaylistEntry *entry)
{
	LOG_PLAYLISTS ("Playlist::AddEntry (%p)\n", entry);

	entries->Append (new PlaylistNode (entry));
	entry->unref ();
}

void
Playlist::ReplaceCurrentEntry (Playlist *pl)
{
	PlaylistEntry *current_entry = GetCurrentEntry ();

	LOG_PLAYLISTS ("Playlist::ReplaceCurrentEntry (%p)\n", pl);

	if (current_entry->IsPlaylist ()) {
		((Playlist *) current_entry)->ReplaceCurrentEntry (pl);
	} else {
		PlaylistNode *pln = new PlaylistNode (pl);
		pl->MergeWith (current_entry);
		entries->InsertBefore (pln, current_node);
		entries->Remove (current_node);
		current_node = pln;
	}
}

void
Playlist::MergeWith (PlaylistEntry *entry)
{
	LOG_PLAYLISTS ("Playlist::MergeWith (%p)\n", entry);

	SetBase (g_strdup (entry->GetBase ()));
	SetTitle (g_strdup (entry->GetTitle ()));
	SetAuthor (g_strdup (entry->GetAuthor ()));
	SetAbstract (g_strdup (entry->GetAbstract ()));
	SetCopyright (g_strdup (entry->GetCopyright ()));

	SetSourceName (g_strdup (entry->GetSourceName ()));
	SetStartTime (entry->GetStartTime ());
	SetDuration (entry->GetDuration ());
	
	element = entry->GetElement ();
}

/*
 * PlaylistParser
 */

PlaylistParser::PlaylistParser (MediaElement *element, IMediaSource *source)
{
	this->element = element;
	this->source = source;

	playlist = NULL;
	current_entry = NULL;
	current_text = NULL;

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
	if (playlist)
		playlist->unref ();
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
	((PlaylistParser *) user_data)->OnStartElement (name, attrs);
}

void
PlaylistParser::on_end_element (gpointer user_data, const char *name)
{
	((PlaylistParser *) user_data)->OnEndElement (name);
}

void
PlaylistParser::on_text (gpointer user_data, const char *data, int len)
{
	((PlaylistParser *) user_data)->OnText (data, len);
}

char *
PlaylistParser::GetHrefAttribute (const char **attrs)
{
	if (!has_href (attrs)) {
		ParsingError ();
		return NULL;
	}

	return get_href (attrs);	
}

void
PlaylistParser::OnStartElement (const char *name, const char **attrs)
{
	PlaylistNodeKind kind = StringToKind (name);

	LOG_PLAYLISTS ("PlaylistParser::OnStartElement (%s, %p), kind = %i\n", name, attrs, kind);

	g_free (current_text);
	current_text = NULL;

	PushCurrentKind (kind);

	switch (kind) {
	case Abstract:
		break;
	case Asx:
		// Here the kind stack should be: Root+Asx
		if (kind_stack->Length () != 2 || !AssertParentKind (Root)) {
			ParsingError (new ErrorEventArgs (MediaError, 3008, "ASX parse error"));
			return;
		}

		playlist = new Playlist (element, source);

		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "VERSION")) {
				if (str_match (attrs [i+1], "3")) {
					// OK
				} else if (str_match (attrs [i+1], "3.0")) {
					// OK
				} else {
					ParsingError (new ErrorEventArgs (MediaError, 3008, "ASX parse error"));
				}
			} else if (str_match (attrs [i], "BANNERBAR")) {
				ParsingError (new ErrorEventArgs (MediaError, 3007, "Unsupported ASX attribute"));
			} else {
				ParsingError (new ErrorEventArgs (MediaError, 3005, "Invalid ASX attribute"));
			}
		}
		break;
	case Author:
		break;
	case Banner:
		break;
	case Base:
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetBase (GetHrefAttribute (attrs));
		break;
	case Copyright:
		break;
	case Duration:
		break;
	case Entry:
		OnEntry ();
		break;
	case EntryRef:
		break;
	case LogUrl:
		break;
	case MoreInfo:
		break;
	case StartTime:
		break;
	case Ref:
		if (GetCurrentEntry () != NULL)
			GetCurrentEntry ()->SetSourceName (GetHrefAttribute (attrs));
		break;
	case Title:
		break;
	case Unknown:
		break;
	case Root:
	default:
		LOG_PLAYLISTS ("PlaylistParser::OnStartElement ('%s', %p): Unknown kind: %i\n", name, attrs, kind);
		ParsingError (new ErrorEventArgs (MediaError, 3008, "ASX parse error"));
		break;
	}
}

void
PlaylistParser::OnEndElement (const char *name)
{
	PlaylistNodeKind kind = GetCurrentKind ();
	TimeSpan ts; 

	LOG_PLAYLISTS ("PlaylistParser::OnEndElement (%s), GetCurrentKind (): %i, GetCurrentKind () to string: %s\n", name, kind, KindToString (kind));

	switch (kind) {
	case Abstract:
		if (!AssertParentKind (Asx | Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetAbstract (current_text);
		current_text = NULL;
		break;
	case Author:
		if (!AssertParentKind (Asx | Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetAuthor (current_text);
		current_text = NULL;
		break;
	case Base:
		if (!AssertParentKind (Asx | Entry))
			break;
		break;
	case Copyright:
		if (!AssertParentKind (Asx | Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetCopyright (current_text);
		current_text = NULL;
		break;
	case Duration:
		if (!AssertParentKind (Entry))
			break;
		if (current_text == NULL)
			break;
		time_span_from_str (current_text, &ts);
		if (GetCurrentEntry () != NULL)
			GetCurrentEntry ()->SetDuration (ts);
		break;
	case Entry:
		if (!AssertParentKind (Asx))
			break;
		break;
	case StartTime:
		if (!AssertParentKind (Entry))
			break;
		time_span_from_str (current_text, &ts);
		current_text = NULL;
		if (GetCurrentEntry () != NULL)
			GetCurrentEntry ()->SetStartTime (ts);
		break;
	case Title:
		if (!AssertParentKind (Asx | Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetTitle (current_text);
		current_text = NULL;
		break;
	case Asx:
		if (!AssertParentKind (Root))
			break;
		break;
	case Ref:
		break;
	default:
		printf ("PlaylistParser::OnEndElement ('%s'): Unknown kind %i.\n", name, kind);
		ParsingError (new ErrorEventArgs (MediaError, 3008, "ASX parse error"));
		break;
	}
	
	if (current_text != NULL) {	
		g_free (current_text);
		current_text = NULL;
	}

	switch (GetCurrentKind ()) {
	case Entry:
		EndEntry ();
		break;
	default:
		break;
	}
	PopCurrentKind ();
}

void
PlaylistParser::OnText (const char *text, int len)
{
	char *a = g_strndup (text, len);

#if DEBUG
	char *p = g_strndup (text, len);
	for (int i = 0; p [i] != NULL; i++)
		if (p [i] == 10 || p [i] == 13)
			p [i] = ' ';

	LOG_PLAYLISTS ("PlaylistParser::OnText (%s, %i)\n", p, len);
	g_free (p);
#endif

	if (current_text == NULL) {
		current_text = a;
	} else {
		char *b = g_strconcat (current_text, a, NULL);
		g_free (current_text);
		current_text = b;
	}
}

bool
PlaylistParser::IsPlaylistFile (IMediaSource *source)
{
	static const char *asx_header = "<ASX";
	int asx_header_length = strlen (asx_header);
	char buffer [20];
	
	if (!source->Peek (buffer, asx_header_length))
		return false;
		
	return strncmp (asx_header, buffer, asx_header_length) == 0;
}

bool
PlaylistParser::IsASX2 (IMediaSource *source)
{
	static const char *asx2_header = "[Reference]";
	int asx2_header_length = strlen (asx2_header);
	char buffer [20];
	
	if (!source->Peek (buffer, asx2_header_length))
		return false;
		
	return strncmp (asx2_header, buffer, asx2_header_length) == 0;
}



bool
PlaylistParser::ParseASX2 ()
{
#define BUFFER_SIZE 1024
	int bytes_read;
	char buffer[BUFFER_SIZE];
	char *mms_uri;
	char *end;

	bytes_read = source->Read (buffer, BUFFER_SIZE);
	if (bytes_read < 0) {
		fprintf (stderr, "Could not read asx document for parsing.\n");
		return false;
	}
	if (!g_str_has_prefix (buffer, "[Reference]\r\nRef1=http://") ||
	    !strstr (buffer, "?MSWMExt=.asf")) {
		return false;
	}

	end = strstr (buffer, "?MSWMExt=.asf");
	*end = '\0';

	mms_uri = g_strdup_printf ("mms://%s", buffer + strlen ("[Reference]\r\nRef1=http://"));

	playlist = new Playlist (element, source);

	PlaylistEntry *entry = new PlaylistEntry (element, playlist);
	entry->SetSourceName (mms_uri);
        playlist->AddEntry (entry);
        current_entry = entry;

	return true;
}

bool
PlaylistParser::Parse ()
{
	LOG_PLAYLISTS ("PlaylistParser::Parse ()\n");

	int bytes_read;
	void *buffer;

	if (!this->IsPlaylistFile (source) && this->IsASX2 (source)) {
		/* Parse as a asx1 mms file */
		return this->ParseASX2 ();
	}

// asx documents don't tend to be very big, so there's no need for a big buffer
#define BUFFER_SIZE 1024

	for (;;) {
		buffer = XML_GetBuffer(parser, BUFFER_SIZE);
		if (buffer == NULL) {
			fprintf (stderr, "Could not allocate memory for asx document parsing.\n");
			return false;
		}
		
		bytes_read = source->Read (buffer, BUFFER_SIZE);
		if (bytes_read < 0) {
			fprintf (stderr, "Could not read asx document for parsing.\n");
			return false;
		}
		
		if (!XML_ParseBuffer (parser, bytes_read, bytes_read == 0)) {
			fprintf (stderr, "Failed to parse asx document: %s (%i)\n", XML_ErrorString (XML_GetErrorCode (parser)), XML_GetErrorCode (parser));
			return false;
		}
		
		if (bytes_read == 0)
			break;
	}

	return playlist != NULL;
}

PlaylistEntry *
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
	PlaylistEntry *entry = new PlaylistEntry (element, playlist);
	playlist->AddEntry (entry);
	current_entry = entry;
}

void 
PlaylistParser::EndEntry ()
{
	this->current_entry = NULL;
}

void
PlaylistParser::PushCurrentKind (PlaylistParser::PlaylistNodeKind kind)
{
	kind_stack->Append (new KindNode (kind));
	LOG_PLAYLISTS ("PlaylistParser::Push (%i)\n", kind);
}

void
PlaylistParser::PopCurrentKind ()
{
	LOG_PLAYLISTS ("PlaylistParser::PopCurrentKind (), current: %i\n", ((KindNode *)kind_stack->Last ())->kind);
	kind_stack->Remove (kind_stack->Last ());
}

PlaylistParser::PlaylistNodeKind
PlaylistParser::GetCurrentKind ()
{
	KindNode *node = (KindNode *) kind_stack->Last ();
	return node->kind;
}

PlaylistParser::PlaylistNodeKind
PlaylistParser::GetParentKind ()
{
	KindNode *node = (KindNode *) kind_stack->Last ()->prev;
	return node->kind;
}

bool
PlaylistParser::AssertParentKind (int kind)
{
	LOG_PLAYLISTS ("PlaylistParser::AssertParentKind (%i), GetParentKind: %i, result: %i\n", kind, GetParentKind (), GetParentKind () & kind);

	if (GetParentKind () & kind)
		return true;

	ParsingError (new ErrorEventArgs (MediaError, 3008, "ASX parse error"));
	
	return false;
}

void
PlaylistParser::ParsingError (ErrorEventArgs *args)
{
	LOG_PLAYLISTS ("PlaylistParser::ParsingError (%p)\n", args);
	
	XML_StopParser (parser, false);
	element->MediaFailed (args);
}


PlaylistParser::PlaylistKind PlaylistParser::playlist_kinds [] = {
	PlaylistParser::PlaylistKind ("ABSTRACT", PlaylistParser::Abstract), 
	PlaylistParser::PlaylistKind ("ASX", PlaylistParser::Asx),
	PlaylistParser::PlaylistKind ("ROOT", PlaylistParser::Root),
	PlaylistParser::PlaylistKind ("AUTHOR", PlaylistParser::Author),
	PlaylistParser::PlaylistKind ("BANNER", PlaylistParser::Banner),
	PlaylistParser::PlaylistKind ("BASE", PlaylistParser::Base),
	PlaylistParser::PlaylistKind ("COPYRIGHT", PlaylistParser::Copyright),
	PlaylistParser::PlaylistKind ("DURATION", PlaylistParser::Duration),
	PlaylistParser::PlaylistKind ("ENTRY", PlaylistParser::Entry),
	PlaylistParser::PlaylistKind ("ENTRYREF", PlaylistParser::EntryRef),
	PlaylistParser::PlaylistKind ("LOGURL", PlaylistParser::LogUrl),
	PlaylistParser::PlaylistKind ("MOREINFO", PlaylistParser::MoreInfo),
	PlaylistParser::PlaylistKind ("REF", PlaylistParser::Ref),
	PlaylistParser::PlaylistKind ("STARTTIME", PlaylistParser::StartTime),
	PlaylistParser::PlaylistKind ("TITLE", PlaylistParser::Title),
	PlaylistParser::PlaylistKind (NULL, PlaylistParser::Unknown)
};

PlaylistParser::PlaylistNodeKind
PlaylistParser::StringToKind (const char *str)
{
	PlaylistNodeKind kind = Unknown;

	for (int i = 0; playlist_kinds [i].str != NULL; i++) {
		if (str_match (str, playlist_kinds [i].str)) {
			kind = playlist_kinds [i].kind;
			break;
		}
	}

	LOG_PLAYLISTS ("PlaylistParser::StringToKind ('%s') = %i\n", str, kind);

	return kind;
}

const char *
PlaylistParser::KindToString (PlaylistNodeKind kind)
{
	const char *result = NULL;

	for (int i = 0; playlist_kinds [i].str != NULL; i++) {
		if (playlist_kinds [i].kind == kind) {
			result = playlist_kinds [i].str;
			break;
		}
	}

	LOG_PLAYLISTS ("PlaylistParser::KindToString (%i) = '%s'\n", kind, result);

	return result;
}
