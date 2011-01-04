/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * playlist.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>
#include <expat.h>
#include <string.h>
#include <math.h>
#include <glib.h>

#include "playlist.h"
#include "clock.h"
#include "mediaelement.h"
#include "debug.h"
#include "media.h"
#include "mediaplayer.h"
#include "factory.h"

namespace Moonlight {

/*
 * PlaylistParserInternal
 */

PlaylistParserInternal::PlaylistParserInternal ()
{
	parser = XML_ParserCreate (NULL);
	asxparser = new AsxParser ();
}

PlaylistParserInternal::~PlaylistParserInternal ()
{
	XML_ParserFree (parser);
	parser = NULL;

	delete asxparser;
}

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

PlaylistEntry::PlaylistEntry (Playlist *parent)
	: EventObject (Type::PLAYLISTENTRY, false)
{
	LOG_PLAYLIST ("PlaylistEntry::PlaylistEntry (%p)\n", parent);

	Init (parent);
	g_return_if_fail (parent != NULL); // should ge a g_warn..., but glib 2.10 doesn't have g_warn.
}

PlaylistEntry::PlaylistEntry (Type::Kind kind, Playlist *parent)
	: EventObject (kind, false)
{
	LOG_PLAYLIST ("PlaylistEntry::PlaylistEntry (%p)\n", parent);

	Init (parent);
	g_return_if_fail (parent != NULL); // should ge a g_warn..., but glib 2.10 doesn't have g_warn.
}


PlaylistEntry::PlaylistEntry (Type::Kind kind)
	: EventObject (kind, false)
{
	LOG_PLAYLIST ("PlaylistEntry::PlaylistEntry ()\n");

	Init (NULL);
}

void
PlaylistEntry::Dispose ()
{
	LOG_PLAYLIST ("PlaylistEntry::Dispose () id: %i media: %i\n", GET_OBJ_ID (this), GET_OBJ_ID (media));
	
	if (media) {
		Media *tmp = media;
		media = NULL;
		tmp->RemoveSafeHandlers (this);
		tmp->DisposeObject (tmp);
		tmp->unref ();
	}
	
	delete source_name;
	source_name = NULL;
	delete full_source_name;
	full_source_name = NULL;

	delete base;
	base = NULL;
	g_free (title);
	title = NULL;
	g_free (author);
	author = NULL;
	g_free (abstract);
	abstract = NULL;
	g_free (copyright);
	copyright = NULL;
	g_free (info_target);
	info_target = NULL;
	g_free (info_url);
	info_url = NULL;
	
	parent = NULL;
	if (params != NULL) {
		g_hash_table_destroy (params);
		params = NULL;
	}
	
	EventObject::Dispose ();
}

void
PlaylistEntry::Init (Playlist *parent)
{
	// Parent might be null
	this->parent = parent;
	this->media = NULL;
	source_name = NULL;
	full_source_name = NULL;
	start_time = 0;
	duration = NULL;
	play_when_available = false;
	base = NULL;
	title = NULL;
	author = NULL;
	abstract = NULL;
	copyright = NULL;
	info_target = NULL;
	info_url = NULL;
	client_skip = true;
	is_live = false;
	set_values = (PlaylistKind::Kind) 0;
	opened = false;
	params = NULL;
	is_entry_ref = false;
}

void
PlaylistEntry::Initialize (Media *media)
{
	g_return_if_fail (media != NULL);
	g_return_if_fail (this->media == NULL);
	
	media->AddSafeHandler (Media::OpenCompletedEvent, OpenCompletedCallback, this);
	media->AddSafeHandler (Media::OpeningEvent, OpeningCallback, this);
	media->AddSafeHandler (Media::SeekingEvent, SeekingCallback, this);
	media->AddSafeHandler (Media::SeekCompletedEvent, SeekCompletedCallback, this);
	media->AddSafeHandler (Media::CurrentStateChangedEvent, CurrentStateChangedCallback, this);
	media->AddSafeHandler (Media::DownloadProgressChangedEvent, DownloadProgressChangedCallback, this);
	media->AddSafeHandler (Media::BufferingProgressChangedEvent, BufferingProgressChangedCallback, this);
	media->AddSafeHandler (Media::MediaErrorEvent, MediaErrorCallback, this);

	if (!media->IsInitialized ()) {
		/* For nested asx playlists we'll end up here with an initialized media. */
		media->SetStartTime (start_time);
		if (HasInheritedDuration ()) {
			Duration *duration = GetInheritedDuration ();
			if (duration->HasTimeSpan ()) {
				media->SetDuration (duration->GetTimeSpan ());
			}
		}
	}

	this->media = media;
	this->media->ref ();
}

void
PlaylistEntry::InitializeWithStream (ManagedStreamCallbacks *callbacks)
{
	Media *media;
	ManagedStreamSource *source;
	PlaylistRoot *root = GetRoot ();
	
	g_return_if_fail (callbacks != NULL);
	g_return_if_fail (root != NULL);
	
	media = new Media (root);
	Initialize (media);
	
	source = new ManagedStreamSource (media, callbacks);
	media->Initialize (source);
	if (!media->HasReportedError ())
		media->OpenAsync ();
	media->unref ();
	source->unref ();
}

void
PlaylistEntry::InitializeWithSource (IMediaSource *source)
{
	Media *media;
	PlaylistRoot *root = GetRoot ();
	
	g_return_if_fail (source != NULL);
	g_return_if_fail (root != NULL);
	
	media = source->GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	Initialize (media);
	
	media->Initialize (source);
	if (!media->HasReportedError ())
		media->OpenAsync ();
	media->unref ();
}

void
PlaylistEntry::InitializeWithUri (const Uri *resource_base, const Uri *uri)
{
	Media *media;
	PlaylistRoot *root = GetRoot ();
	
	g_return_if_fail (uri != NULL);
	g_return_if_fail (root != NULL);
	
	media = new Media (root);
	Initialize (media);
	media->Initialize (resource_base, uri);
	if (!media->HasReportedError ())
		media->OpenAsync ();
	media->unref ();
}

void
PlaylistEntry::InitializeWithDownloader (Downloader *dl, const char *PartName)
{
	Media *media;
	PlaylistRoot *root = GetRoot ();
	
	g_return_if_fail (dl != NULL);
	g_return_if_fail (root != NULL);
	
	media = new Media (root);
	Initialize (media);
	media->Initialize (dl, PartName);
	if (!media->HasReportedError ())
		media->OpenAsync ();
	media->unref ();
}

void
PlaylistEntry::InitializeWithDemuxer (IMediaDemuxer *demuxer)
{
	Media *media;
	PlaylistRoot *root = GetRoot ();
	
	g_return_if_fail (demuxer != NULL);
	g_return_if_fail (root != NULL);
	
	media = demuxer->GetMediaReffed ();
	
	g_return_if_fail (media != NULL);
	
	Initialize (media);
	media->Initialize (demuxer);
	if (!media->HasReportedError ())
		media->OpenAsync ();
	media->unref ();
}

void
PlaylistEntry::OpeningHandler (Media *media, EventArgs *args)
{
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::OpeningHandler (%p, %p) current entry: %i\n", media, args, root->GetCurrentPlaylistEntry () == this);
	
	g_return_if_fail (root != NULL);
	
	if (root->GetCurrentPlaylistEntry () == this)
		root->Emit (PlaylistRoot::OpeningEvent, args);
}

void
PlaylistEntry::OpenMediaPlayer ()
{
	PlaylistRoot *root = GetRoot ();
	MediaPlayer *mplayer;
	
	g_return_if_fail (opened == true);
	g_return_if_fail (root != NULL);
	
	mplayer = GetMediaPlayer ();
	g_return_if_fail (mplayer != NULL);

	mplayer->Open (media, this);
	
	root->Emit (PlaylistRoot::OpenCompletedEvent, NULL);
}

void
PlaylistEntry::OpenCompletedHandler (Media *media, EventArgs *args)
{
	PlaylistRoot *root = GetRoot ();
	IMediaDemuxer *demuxer = NULL;
	Playlist *playlist;
		
	LOG_PLAYLIST ("PlaylistEntry::OpenCompletedHandler (%p, %p)\n", media, args);
	opened = true;
	
	g_return_if_fail (media != NULL);
	g_return_if_fail (root != NULL);
	g_return_if_fail (parent != NULL);
	
	demuxer = media->GetDemuxerReffed ();
	
	g_return_if_fail (demuxer != NULL);
	
	LOG_PLAYLIST ("PlaylistEntry::OpenCompletedHandler (%p, %p) demuxer: %i %s\n", media, args, GET_OBJ_ID (demuxer), demuxer->GetTypeName ());
		
	if (demuxer->IsPlaylist ()) {
		playlist = demuxer->GetPlaylist ();
		
		if (playlist == NULL || parent == NULL) {
			goto cleanup;
		}
		
		parent->ReplaceCurrentEntry (playlist);
		playlist->Open ();
	} else {
		if (parent->GetCurrentEntry () == this) {
			OpenMediaPlayer ();
		} else {
			LOG_PLAYLIST ("PlaylistEntry::OpenCompletedHandler (%p, %p): opened entry in advance, waiting for current entry to finish.\n", media, args);
		}
	}
	
cleanup:
	if (demuxer)
		demuxer->unref ();
}

void
PlaylistEntry::SeekingHandler (Media *media, EventArgs *args)
{
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::SeekingHandler (%p, %p)\n", media, args);
	
	g_return_if_fail (root != NULL);
	
	if (args)
		args->ref ();
	root->Emit (PlaylistRoot::SeekingEvent, args);
}

void
PlaylistEntry::SeekCompletedHandler (Media *media, EventArgs *args)
{
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::SeekCompletedHandler (%p, %p)\n", media, args);
	
	g_return_if_fail (root != NULL);
	
	if (args)
		args->ref ();
	root->Emit (PlaylistRoot::SeekCompletedEvent, args);
}

void
PlaylistEntry::CurrentStateChangedHandler (Media *media, EventArgs *args)
{
	LOG_PLAYLIST ("PlaylistEntry::CurrentStateChangedHandler (%p, %p)\n", media, args);
}

void
PlaylistEntry::MediaErrorHandler (Media *media, ErrorEventArgs *args)
{
	LOG_PLAYLIST ("PlaylistEntry::MediaErrorHandler (%p, %p): %s '%s'\n", media, args, GetFullSourceName () != NULL ? GetFullSourceName ()->GetOriginalString () : NULL, args ? args->GetErrorMessage() : "?");
	
	if (parent == NULL) {
#if DEBUG
		if (args) {
			printf ("Moonlight: there was an error in the media pipeline which couldn't be delivered to the plugin (most likely because MediaElement's source has changed): %i %s %s\n",
				args->GetErrorCode (), args->GetErrorMessage (), args->GetExtendedMessage ());
		}
#endif
		return;
	}
	
	parent->OnEntryFailed (args);
}

void
PlaylistEntry::DownloadProgressChangedHandler (Media *media, EventArgs *args)
{
	PlaylistRoot *root;
	
	LOG_PLAYLIST ("PlaylistEntry::DownloadProgressChanged (%p, %p %.2f). Disposed: %i\n", media, args, args ? ((ProgressEventArgs *) args)->progress : -1.0, IsDisposed ());
	
	if (IsDisposed ())
		return;
	
	root = GetRoot ();
	
	g_return_if_fail (root != NULL);
	
	if (root->GetCurrentPlaylistEntry () != this) {
		/* FIXME: we should stop any running downloads when we advance from one playlistentry
		 * to the next. Currently this will fail because we won't ever resume downloads if the
		 * user decides to stop&replay the media */
		LOG_PLAYLIST ("PlaylistEntry::DownloadProgressChangedHandler (): Got download progress changed handler (%.2f) for an inactive media\n", args ? ((ProgressEventArgs *) args)->progress : -1.0);
		return;
	}

	if (args)
		args->ref ();
	root->Emit (PlaylistRoot::DownloadProgressChangedEvent, args);
}

void
PlaylistEntry::BufferingProgressChangedHandler (Media *media, EventArgs *args)
{
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::BufferingProgressChanged (%p, %p) %.2f\n", media, args, args ? ((ProgressEventArgs *) args)->progress : -1.0);
	
	if (root == NULL)
		return; // this might happen if the media is still buffering and we're in the process of getting cleaned up
	
	if (args)
		args->ref ();
	root->Emit (PlaylistRoot::BufferingProgressChangedEvent, args);
}

void
PlaylistEntry::Seek (guint64 pts)
{	
	LOG_CUSTOM (RUNTIME_DEBUG_SEEK | RUNTIME_DEBUG_PLAYLIST, "PlaylistEntry::Seek (%" G_GUINT64_FORMAT ")\n", pts);
	
	g_return_if_fail (media != NULL);
	
	media->SeekAsync (pts);
}

void
PlaylistEntry::AddParams (const char *name, const char *value)
{
	char *uppername = g_ascii_strup (name, strlen (name));
	if (!strcmp (uppername, "AUTHOR")) {
		SetAuthor (value);
	} else if (!strcmp (uppername, "ABSTRACT")) {
		SetAbstract (value);
	} else if (!strcmp (uppername, "TITLE")) {
		SetTitle (value);
	} else if (!strcmp (uppername, "COPYRIGHT")) {
		SetCopyright (value);
	} else if (!strcmp (uppername, "INFOTARGET")) {
		SetInfoTarget (value);
	} else if (!strcmp (uppername, "INFOURL")) {
		SetInfoURL (value);
	} else {
		if (params == NULL)
			params = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
		
		if (g_hash_table_lookup  (params, uppername) == NULL) {
			g_hash_table_insert (params, uppername, g_strdup (value));
			uppername = NULL;
		} 
	}
	g_free (uppername);
}

Uri *
PlaylistEntry::GetBase ()
{
	return base;
}

Uri *
PlaylistEntry::GetBaseInherited ()
{
	if (base != NULL)
		return base;
	if (parent != NULL)
		return parent->GetBaseInherited ();
	return NULL;
}

void 
PlaylistEntry::SetBase (Uri *base)
{
	// TODO: Haven't been able to make BASE work with SL,
	// which means that I haven't been able to confirm any behaviour.
	if (!(set_values & PlaylistKind::Base)) {
		this->base = base;
		set_values = (PlaylistKind::Kind) (set_values | PlaylistKind::Base);
	} else {
		delete base;
	}
}

const char *
PlaylistEntry::GetTitle ()
{
	return title;
}

void 
PlaylistEntry::SetTitle (const char *title)
{
	if (!(set_values & PlaylistKind::Title)) {
		this->title = g_strdup (title);
		set_values = (PlaylistKind::Kind) (set_values | PlaylistKind::Title);
	}
}

const char *
PlaylistEntry::GetAuthor ()
{
	return author;
}

void PlaylistEntry::SetAuthor (const char *author)
{
	if (!(set_values & PlaylistKind::Author)) {
		this->author = g_strdup (author);
		set_values = (PlaylistKind::Kind) (set_values | PlaylistKind::Author);
	}
}

const char *
PlaylistEntry::GetAbstract ()
{
	return abstract;
}

void 
PlaylistEntry::SetAbstract (const char *abstract)
{
	if (!(set_values & PlaylistKind::Abstract)) {
		this->abstract = g_strdup (abstract);
		set_values = (PlaylistKind::Kind) (set_values | PlaylistKind::Abstract);
	}
}

const char *
PlaylistEntry::GetCopyright ()
{
	return copyright;
}

void 
PlaylistEntry::SetCopyright (const char *copyright)
{
	if (!(set_values & PlaylistKind::Copyright)) {
		this->copyright = g_strdup (copyright);
		set_values = (PlaylistKind::Kind) (set_values | PlaylistKind::Copyright);
	}
}

Uri *
PlaylistEntry::GetSourceName ()
{
	return source_name;
}

void 
PlaylistEntry::SetSourceName (Uri *source_name)
{
	if (this->source_name)
		delete this->source_name;
	this->source_name = source_name;
}

TimeSpan 
PlaylistEntry::GetStartTime ()
{
	return start_time;
}

void 
PlaylistEntry::SetStartTime (TimeSpan start_time)
{
	if (!(set_values & PlaylistKind::StartTime)) {
		this->start_time = start_time;
		set_values = (PlaylistKind::Kind) (set_values | PlaylistKind::StartTime);
	}
}

Duration*
PlaylistEntry::GetDuration ()
{
	return duration;
}

Duration *
PlaylistEntry::GetInheritedDuration ()
{
	if (HasDuration ()) {
		return GetDuration ();
	} else if (parent != NULL) {
		return parent->GetInheritedDuration ();
	} else {
		return NULL;
	}
}

bool
PlaylistEntry::HasInheritedDuration ()
{
	if (HasDuration ()) {
		return true;
	} else if (parent != NULL) {
		return parent->HasInheritedDuration ();
	} else {
		return false;
	}
}

void 
PlaylistEntry::SetDuration (Duration *duration)
{
	if (!(set_values & PlaylistKind::Duration)) {
		this->duration = duration;
		set_values = (PlaylistKind::Kind) (set_values | PlaylistKind::Duration);
	}
}

const char *
PlaylistEntry::GetInfoTarget ()
{
	return info_target;
}

void
PlaylistEntry::SetInfoTarget (const char *info_target)
{
	g_free (this->info_target);
	this->info_target = g_strdup (info_target);
}

const char *
PlaylistEntry::GetInfoURL ()
{
	return info_url;
}

void
PlaylistEntry::SetInfoURL (const char *info_url)
{
	g_free (this->info_url);
	this->info_url = g_strdup (info_url);
}

bool
PlaylistEntry::GetClientSkip ()
{
	return client_skip;
}

void
PlaylistEntry::SetClientSkip (bool value)
{
	client_skip = value;
}

MediaElement *
PlaylistEntry::GetElement ()
{
	g_return_val_if_fail (parent != NULL, NULL);
	
	return parent->GetElement ();
}

void
PlaylistEntry::ClearMedia ()
{
	g_return_if_fail (media != NULL);
	media->unref ();
	media = NULL;
}

MediaPlayer *
PlaylistEntry::GetMediaPlayer ()
{
	PlaylistRoot *root = GetRoot ();
	
	g_return_val_if_fail (root != NULL, NULL);
	
	return root->GetMediaPlayer ();
}

static void
add_attribute (MediaAttributeCollection *attributes, const char *name, const char *attr)
{
	if (!attr)
		return;

	MediaAttribute *attribute = new MediaAttribute ();
	attribute->SetValue (attr);
	attribute->SetName (name);
	
	attributes->Add (attribute);
	attribute->unref ();
}

static void
add_attribute_glib (const char *name, const char *value, MediaAttributeCollection *attributes)
{
	add_attribute (attributes, name, value);
}

void
PlaylistEntry::PopulateMediaAttributes ()
{
	LOG_PLAYLIST ("PlaylistEntry::PopulateMediaAttributes ()\n");

	const char *abstract = NULL;
	const char *author = NULL;
	const char *copyright = NULL;
	const char *title = NULL;
	const char *infotarget = NULL;
	const char *infourl = NULL;
	const char *baseurl = NULL;

	MediaElement *element = GetElement ();
	PlaylistEntry *current = this;
	MediaAttributeCollection *attributes;
	
	g_return_if_fail (element != NULL);
	
	if (!(attributes = element->GetAttributes ())) {
		attributes = new MediaAttributeCollection ();
		element->SetAttributes (attributes);
	} else {
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
		if (infotarget == NULL)
			infotarget = current->GetInfoTarget ();
		if (infourl == NULL)
			infourl = current->GetInfoURL ();
		if (baseurl == NULL && current->GetBase () != NULL)
			baseurl = current->GetBase ()->GetOriginalString ();

		current = current->GetParent ();
	}

	add_attribute (attributes, "ABSTRACT", abstract);
	add_attribute (attributes, "AUTHOR", author);
	add_attribute (attributes, "BaseURL", baseurl);
	add_attribute (attributes, "COPYRIGHT", copyright);
	add_attribute (attributes, "InfoTarget", infotarget);
	add_attribute (attributes, "InfoURL", infourl);
	add_attribute (attributes, "TITLE", title);
	
	current = this;
	while (current != NULL && !current->GetIsEntryRef ()) {
		if (current->params != NULL)
			g_hash_table_foreach (current->params, (GHFunc) add_attribute_glib, attributes);
		current = current->GetParent ();
	}
}

const Uri *
PlaylistEntry::GetFullSourceName ()
{
	if (full_source_name == NULL) {
		Uri *base = GetBaseInherited ();
		Uri *current = GetSourceName ();
		
		//printf ("PlaylistEntry::GetFullSourceName (), base: %s, current: %s\n", base ? base->ToString () : "NULL", current ? current->ToString () : "NULL");
		
		if (current == NULL) {
			full_source_name = NULL;
		} else if (current->IsAbsolute ()) {
			full_source_name = Uri::Clone (current);
		} else if (base != NULL) {
			full_source_name = Uri::Create (base, current);
		} else {
			full_source_name = Uri::Clone (current);
		}
	}
	return full_source_name;
}

void
PlaylistEntry::Open ()
{
	LOG_PLAYLIST ("PlaylistEntry::Open (), media = %p, FullSourceName = %s\n", media, GetFullSourceName () != NULL ? GetFullSourceName ()->GetOriginalString () : NULL);

	if (!media) {
		g_return_if_fail (GetFullSourceName () != NULL);
		InitializeWithUri (GetElement ()->GetResourceBase (), GetFullSourceName ());
	} else if (opened) {
		OpenMediaPlayer ();
	} else {
		media->OpenAsync ();
	}
}

void
PlaylistEntry::Play ()
{
	MediaPlayer *mplayer = GetMediaPlayer ();
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::Play (), play_when_available: %s, media: %p, source name: %s\n", play_when_available ? "true" : "false", media, source_name ? source_name->ToString () : "NULL");

	g_return_if_fail (media != NULL);
	g_return_if_fail (mplayer != NULL);
	g_return_if_fail (root != NULL);

	media->PlayAsync ();
	mplayer->Play ();
	
	root->Emit (PlaylistRoot::PlayEvent);
}

void
PlaylistEntry::Pause ()
{
	MediaPlayer *mplayer = GetMediaPlayer ();
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::Pause ()\n");

	g_return_if_fail (media != NULL);
	g_return_if_fail (mplayer != NULL);
	g_return_if_fail (root != NULL);
	
	play_when_available = false;
	media->PauseAsync ();
	mplayer->Pause ();
	
	root->Emit (PlaylistRoot::PauseEvent);
}

void
PlaylistEntry::Stop ()
{
	LOG_PLAYLIST ("PlaylistEntry::Stop ()\n");

	play_when_available = false;
	if (media != NULL)
		media->StopAsync ();
}

Media *
PlaylistEntry::GetMedia ()
{
	return media;
}

bool
PlaylistEntry::IsSingleFile ()
{
	return parent ? parent->IsSingleFile () : false;
}

PlaylistRoot *
PlaylistEntry::GetRoot ()
{
	Playlist *pl;
	
	if (IsDisposed ())
		return NULL;
	
	if (parent == NULL) {
		g_return_val_if_fail (GetObjectType () == Type::PLAYLISTROOT, NULL);
		return (PlaylistRoot *) this;
	}
	
	pl = parent;
	
	while (pl->parent != NULL)
		pl = pl->parent;
	
	g_return_val_if_fail (pl->GetObjectType () == Type::PLAYLISTROOT, NULL);
	
	return (PlaylistRoot *) pl;
}

/*
 * Playlist
 */

Playlist::Playlist (Playlist *parent)
	: PlaylistEntry (Type::PLAYLIST, parent)
{
	is_single_file = false;
	Init ();
}

Playlist::Playlist (Type::Kind kind)
	: PlaylistEntry (kind)
{
	LOG_PLAYLIST ("Playlist::Playlist ()\n");
	is_single_file = true;
	Init ();

	AddEntry (new PlaylistEntry (this));
}

void
Playlist::Init ()
{
	LOG_PLAYLIST ("Playlist::Init ()\n");

	waiting = false;
	opened = false;
	entries = new List ();
	current_node = NULL;
}

void
Playlist::Dispose ()
{
	PlaylistNode *node;
	PlaylistEntry *entry;

	LOG_PLAYLIST ("Playlist::Dispose () id: %i\n", GET_OBJ_ID (this));
	
	current_node = NULL;
	
	if (entries != NULL) {
		node = (PlaylistNode *) entries->First ();
		while (node != NULL) {
			entry = node->GetEntry ();
			if (entry != NULL)
				entry->Dispose ();
			node = (PlaylistNode *) node->next;
		}
		delete entries;
		entries = NULL;
	}
	PlaylistEntry::Dispose ();
}


bool
Playlist::IsCurrentEntryLastEntry ()
{
	PlaylistEntry *entry;
	Playlist *pl;
	
	if (entries->Last () == NULL)
		return false;
		
	if (current_node != entries->Last ())
		return false;
		
	entry = GetCurrentEntry ();
	
	if (!entry->IsPlaylist ())
		return true;
		
	pl = (Playlist *) entry;
	
	return pl->IsCurrentEntryLastEntry ();
}

void
Playlist::Open ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("Playlist::Open ()\n");
	
	current_node = (PlaylistNode *) entries->First ();

	current_entry = GetCurrentEntry ();	
	
	while (current_entry && current_entry->HasDuration () && current_entry->GetDuration ()->HasTimeSpan() &&
	       current_entry->GetDuration ()->GetTimeSpan () == 0) {
		LOG_PLAYLIST ("Playlist::Open (), current entry (%s) has zero duration, skipping it.\n", current_entry->GetSourceName ()->ToString ());
		current_node = (PlaylistNode *) current_node->next;
		current_entry = GetCurrentEntry ();
	}
	
	if (current_entry)
		current_entry->Open ();

	opened = true;

	LOG_PLAYLIST ("Playlist::Open (): current node: %p, current entry: %p\n", current_entry, GetCurrentEntry ());
}

bool
Playlist::PlayNext ()
{
	PlaylistEntry *current_entry;
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("Playlist::PlayNext () current_node: %p\n", current_node);
	g_return_val_if_fail (root != NULL, false);
	VERIFY_MAIN_THREAD;
	
	if (!current_node)
		return false;

	SetWaiting (false);

	current_entry = GetCurrentEntry ();

	if (current_entry->HasDuration() && current_entry->GetDuration()->IsForever ()) {
		GetElement ()->SetPlayRequested ();
		current_entry->Play ();
		return true;
	}

	if (current_entry->IsPlaylist ()) {
		LOG_PLAYLIST ("Playlist::PlayNext (): calling nested playlist.\n");
		Playlist *current_playlist = (Playlist *) current_entry;
		if (current_playlist->PlayNext ())
			return true;
	}
	
	if (current_node->next) {
		current_node = (PlaylistNode *) current_node->next;
	
		current_entry = GetCurrentEntry ();
		if (current_entry) {
			LOG_PLAYLIST ("Playlist::PlayNext () playing entry: %p %s\n", current_entry, current_entry->GetFullSourceName () != NULL ? current_entry->GetFullSourceName ()->GetOriginalString () : NULL);
			GetElement ()->SetPlayRequested ();
			root->Emit (PlaylistRoot::EntryChangedEvent);
			current_entry->Open ();
			return true;
		} else {
			LOG_PLAYLIST ("Playlist::PlayNext (): no next entry.\n");
		}
	}
	
	LOG_PLAYLIST ("Playlist::PlayNext () current_node: %p, nothing to play (is root: %i)\n", current_node, GetObjectType () == Type::PLAYLISTROOT);
	
	if (GetObjectType () == Type::PLAYLISTROOT) {
		PlaylistRoot *root = (PlaylistRoot *) this;
		root->EmitMediaEnded ();
	}
	
	return false;
}

void
Playlist::OnEntryEnded ()
{
	LOG_PLAYLIST ("Playlist::OnEntryEnded ()\n");
	PlayNext ();
}

void
Playlist::OnEntryFailed (ErrorEventArgs *args)
{	
	bool fatal = true;
	PlaylistRoot *root = GetRoot ();
	
	LOG_PLAYLIST ("Playlist::OnEntryFailed () extended_code: %i is_single_file: %i\n", args ? args->GetExtendedCode() : 0, is_single_file);
	
	g_return_if_fail (root != NULL);
	
	// media or playlist 404: fatal
	// invalid playlist (playlist parsing failed): fatal
	// invalid media (gif, swf): play next
	if (args == NULL) {
		fatal = true;
	} else {
		// check if we're in a playlist
		IMediaDemuxer *demuxer = NULL;
		if (GetMedia () != NULL)
			demuxer = GetMedia ()->GetDemuxerReffed ();
			
		if (demuxer != NULL && demuxer->GetObjectType () == Type::ASXDEMUXER) {
			// we're a playlist
			if (args->GetExtendedCode() == MEDIA_UNKNOWN_CODEC) {
				fatal = false;
			} else {
				fatal = true;
			}
		} else {
			// we're not a playlist
			fatal = true;
		}

		if (demuxer)
			demuxer->unref ();
	}
	
	if (fatal) {
		if (args)
			args->ref ();
		root->Emit (PlaylistRoot::MediaErrorEvent, args);
	} else {
		root->PlayNext ();
	}
}

void
Playlist::Seek (guint64 pts)
{
	PlaylistEntry *current_entry;
	
	LOG_CUSTOM (RUNTIME_DEBUG_SEEK | RUNTIME_DEBUG_PLAYLIST, "Playlist::Seek (%" G_GUINT64_FORMAT ")\n", pts);
	
	current_entry = GetCurrentEntry ();
	
	g_return_if_fail (current_entry != NULL);
	
	current_entry->Seek (pts);
}

void
Playlist::Play ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("Playlist::Play ()\n");
	
 	current_entry = GetCurrentEntry ();
 	
 	g_return_if_fail (current_entry != NULL);
 	
	if (current_entry && current_entry->HasDuration () && current_entry->GetDuration () == 0) {
		LOG_PLAYLIST ("Playlist::Open (), current entry (%s) has zero duration, skipping it.\n", current_entry->GetSourceName ()->ToString ());
		OnEntryEnded ();
	} else {
		if (current_entry)
			current_entry->Play ();
	}
}

void
Playlist::Pause ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("Playlist::Pause ()\n");

	current_entry = GetCurrentEntry ();
	
	g_return_if_fail (current_entry != NULL);

	current_entry->Pause ();
}

void
Playlist::Stop ()
{
	PlaylistNode *node;

	LOG_PLAYLIST ("Playlist::Stop ()\n");

	node = (PlaylistNode *) entries->First ();
	current_node = node;  // reset to first node
	while (node != NULL) {
		node->GetEntry ()->Stop ();
		node = (PlaylistNode *) node->next;
	}
}

void
Playlist::PopulateMediaAttributes ()
{
	PlaylistEntry *current_entry = GetCurrentEntry ();

	LOG_PLAYLIST ("Playlist::PopulateMediaAttributes ()\n");

	if (!current_entry)
		return;

	current_entry->PopulateMediaAttributes ();
}

void
Playlist::AddEntry (PlaylistEntry *entry)
{
	PlaylistNode *node;
	
	LOG_PLAYLIST ("Playlist::AddEntry (%p) Count: %i\n", entry, entries->Length ());

	node = new PlaylistNode (entry);
	entries->Append (node);
	entry->unref ();
	
	if (entries->Length () == 1) {
		g_return_if_fail (current_node == NULL);
		current_node = node;
	}
}

bool
Playlist::ReplaceCurrentEntry (Playlist *pl)
{
	bool result;
	
	PlaylistEntry *current_entry = GetCurrentEntry ();

	LOG_PLAYLIST ("Playlist::ReplaceCurrentEntry (%p)\n", pl);

	// check for too nested playlist
	int counter = 0;
	PlaylistEntry *e = this;
	while (e != NULL && e->IsPlaylist ()) {
		IMediaDemuxer *demuxer = NULL;

		if (e->GetMedia () != NULL)
			demuxer = e->GetMedia ()->GetDemuxerReffed ();

		if (e->GetObjectType () != Type::PLAYLISTROOT && demuxer != NULL && demuxer->GetObjectType () == Type::ASXDEMUXER)
			counter++;

		if (demuxer)
			demuxer->unref ();

		e = e->GetParent ();
		
		if (counter > 5) {
			ErrorEventArgs *args = new ErrorEventArgs (MediaError,
								   MoonError (MoonError::EXCEPTION, 4001, "AG_E_NETWORK_ERROR"));
			OnEntryFailed (args);
			args->unref ();
			return false;
		}
	}
	
	if (current_entry->IsPlaylist ()) {
		result = ((Playlist *) current_entry)->ReplaceCurrentEntry (pl);
	} else {
		PlaylistNode *pln = new PlaylistNode (pl);
		pl->MergeWith (current_entry);
		entries->InsertBefore (pln, current_node);
		entries->Remove (current_node);
		pl->SetParent (this);
		current_node = pln;
		result = true;
	}
	
	LOG_PLAYLIST ("Playlist::ReplaceCurrentEntrY (%p) [DONE]\n", pl);
	
	return result;
}

void
Playlist::MergeWith (PlaylistEntry *entry)
{
	LOG_PLAYLIST ("Playlist::MergeWith (%p)\n", entry);

	SetBase (entry->GetBase () ? new Uri (*entry->GetBase ()) : NULL);
	SetTitle (entry->GetTitle ());
	SetAuthor (entry->GetAuthor ());
	SetAbstract (entry->GetAbstract ());
	SetCopyright (entry->GetCopyright ());

	SetSourceName (entry->GetSourceName () ? new Uri (*entry->GetSourceName ()) : NULL);
	if (entry->HasDuration ()) 
		SetDuration (entry->GetDuration ());
	if (entry->HasStartTime ())
		SetStartTime (entry->GetStartTime ());
	SetIsEntryRef (entry->GetIsEntryRef ());
	Initialize (entry->GetMedia ());
	entry->ClearMedia ();
}

PlaylistEntry *
Playlist::GetCurrentPlaylistEntry () 
{
	PlaylistEntry *result = NULL;
	
	if (current_node)
		result = current_node->GetEntry () ->GetCurrentPlaylistEntry ();
	
	return result;
}
/*
 * PlaylistRoot
 */

PlaylistRoot::PlaylistRoot (MediaElement *element)
	: Playlist (Type::PLAYLISTROOT)
{
	dynamic = false;
	dynamic_ended = false;
	dynamic_waiting = false;
	this->element = element;
	
	mplayer = element->GetMediaPlayer ();
	mplayer->AddHandler (MediaPlayer::MediaEndedEvent, MediaEndedCallback, this);
	mplayer->AddHandler (MediaPlayer::BufferUnderflowEvent, BufferUnderflowCallback, this);
	mplayer->ref ();
}

void
PlaylistRoot::Dispose ()
{
	if (mplayer != NULL) {
		mplayer->RemoveAllHandlers (this);
		mplayer->unref ();
		mplayer = NULL;
	}
		
	Playlist::Dispose ();
}

void
PlaylistRoot::SetHasDynamicEndedCallback (EventObject *obj)
{
	((PlaylistRoot *) obj)->SetHasDynamicEnded ();
}

void
PlaylistRoot::SetHasDynamicEnded ()
{
	LOG_PLAYLIST ("PlaylistRoot::SetHasDynamicEnded () InMainThread: %i\n", Surface::InMainThread ());

	if (!Surface::InMainThread ()) {
		AddTickCall (SetHasDynamicEndedCallback);
		return;
	}

	dynamic_ended = true;
	if (dynamic_waiting) {
		dynamic_waiting = false;
		EmitMediaEnded ();
	}
}

bool
PlaylistRoot::IsSingleFile ()
{
	PlaylistEntry *entry;
	
	if (GetCount () != 1)
		return false;
	
	entry = GetCurrentEntry ();
	if (entry == NULL)
		return false;
	
	if (entry->GetObjectType () == Type::PLAYLISTENTRY)
		return true;
	
	return entry->IsSingleFile ();
}

#if DEBUG
void
PlaylistEntry::DumpInternal (int tabs)
{
	printf ("%*s%s %i %s\n", tabs, "", GetTypeName (), GET_OBJ_ID (this), GetIsEntryRef () ? "EntryRef" : "Entry");
	tabs++;
	printf ("%*sParent: %p %s\n", tabs, "", parent, parent ? parent->GetTypeName () : NULL);
	printf ("%*sFullSourceName: %s\n", tabs, "", GetFullSourceName () != NULL ? GetFullSourceName ()->GetOriginalString () : NULL);
	printf ("%*sDuration: %s %.2f seconds StartTime: %" G_GUINT64_FORMAT " ms\n", tabs, "", HasDuration () ? "yes" : "no", HasDuration () ? GetDuration ()->ToSecondsFloat () : 0.0, MilliSeconds_FromPts (GetStartTime ()));
	printf ("%*sMedia: %i %s\n", tabs, "", GET_OBJ_ID (media), media ? "" : "(null)");
	if (media) {
		IMediaDemuxer *demuxer = media->GetDemuxerReffed ();
		printf ("%*sUri: %s\n", tabs, "", media->GetUri () ? media->GetUri ()->GetOriginalString () : NULL);
		printf ("%*sDemuxer: %i %s\n", tabs, "", GET_OBJ_ID (demuxer), demuxer ? demuxer->GetTypeName () : "N/A");
		printf ("%*sSource:  %i %s\n", tabs, "", GET_OBJ_ID (media->GetSource ()), media->GetSource () ? media->GetSource ()->GetTypeName () : "N/A");
		if (demuxer)
			demuxer->unref ();
	}
	
}

void 
Playlist::DumpInternal (int tabs)
{
	PlaylistNode *node;
	
	PlaylistEntry::DumpInternal (tabs);
	printf ("%*s %i entries:\n", tabs, "", entries->Length ());
	node = (PlaylistNode *) entries->First ();
	while (node != NULL) {
		if (node == current_node)
			printf ("*%*s * CURRENT NODE *\n", tabs, "");
		node->GetEntry ()->DumpInternal (tabs + 2);
		node = (PlaylistNode *) node->next;
	}
}
void
PlaylistRoot::Dump ()
{
	printf ("\n\nDUMP OF PLAYLIST\n\n");
	DumpInternal (0);
	printf ("\n\nDUMP OF PLAYLIST DONE\n\n");
}
#endif

void
PlaylistRoot::SeekCallback (EventObject *obj)
{
	PlaylistRoot *playlist = (PlaylistRoot *) obj;
	PtsNode *pts_node;
	
	LOG_CUSTOM (RUNTIME_DEBUG_SEEK | RUNTIME_DEBUG_PLAYLIST, "PlaylistRoot::SeekCallback ()\n");

	if (playlist->IsDisposed ())
		return;

	pts_node = (PtsNode *) playlist->seeks.First ();
	if (pts_node != NULL) {
		playlist->seeks.Unlink (pts_node);
		playlist->Seek (pts_node->pts);
		delete pts_node;
	}
}

void
PlaylistRoot::SeekAsync (guint64 pts)
{
	LOG_CUSTOM (RUNTIME_DEBUG_SEEK | RUNTIME_DEBUG_PLAYLIST, "PlaylistRoot::SeekAsync (%" G_GUINT64_FORMAT ")\n", pts);
	seeks.Append (new PtsNode (pts));
	AddTickCall (SeekCallback);
}

void
PlaylistRoot::PlayCallback (EventObject *obj)
{
	LOG_PLAYLIST ("PlaylistRoot::PlayCallback ()\n");

	PlaylistRoot *root = (PlaylistRoot *) obj;
	if (root->IsDisposed ())
		return;
	root->Play ();
}

void
PlaylistRoot::PlayAsync ()
{
	LOG_PLAYLIST ("PlaylistRoot::PlayAsync ()\n");
	AddTickCall (PlayCallback);
}

void
PlaylistRoot::PauseCallback (EventObject *obj)
{
	LOG_PLAYLIST ("PlaylistRoot::PauseCallback ()\n");

	PlaylistRoot *root = (PlaylistRoot *) obj;
	if (root->IsDisposed ())
		return;
	root->Pause ();
}

void
PlaylistRoot::PauseAsync ()
{
	LOG_PLAYLIST ("PlaylistRoot::PauseAsync ()\n");
	AddTickCall (PauseCallback);
}

void
PlaylistRoot::OpenCallback (EventObject *obj)
{
	LOG_PLAYLIST ("PlaylistRoot::OpenCallback ()\n");

	PlaylistRoot *root = (PlaylistRoot *) obj;
	if (root->IsDisposed ())
		return;
	root->Open ();
}

void
PlaylistRoot::OpenAsync ()
{
	LOG_PLAYLIST ("PlaylistRoot::OpenAsync ()\n");
	AddTickCall (OpenCallback);
}

void
PlaylistRoot::StopCallback (EventObject *obj)
{
	LOG_PLAYLIST ("PlaylistRoot::StopCallback ()\n");

	PlaylistRoot *root = (PlaylistRoot *) obj;
	if (root->IsDisposed ())
		return;
	root->Stop ();
}

void
PlaylistRoot::StopAsync ()
{
	LOG_PLAYLIST ("PlaylistRoot::StopAsync ()\n");
	AddTickCall (StopCallback);
}

void
PlaylistRoot::Stop ()
{
	MediaPlayer *mplayer;
	
	LOG_PLAYLIST ("PlaylistRoot::Stop ()\n");
	
	mplayer = GetMediaPlayer ();
	
	Playlist::Stop ();
	if (mplayer != NULL)
		mplayer->Stop ();
	// Stop is called async, and if we now emit Open async, we'd possibly not get events in the right order 
	// example with user code: 
	//   Stop ();
	//   Play ();
	// would end up like: 
	//  StopAsync (); -> enqueue Stop
	//  PlayAsync (); -> enqueue Play
	//  Stop is called, enqueue Open
	Open ();
	Emit (StopEvent); // we emit the event after enqueuing the Open request, do avoid funky side-effects of event emission.
}

void
PlaylistRoot::EmitMediaEnded ()
{
	/* We only emit MediaEnded if we're a static playlist, or a dynamic whose end has been signalled */
	LOG_PLAYLIST ("PlaylistRoot::EmitMediaEnded () dynamic: %i dynamic_ended: %i\n", dynamic, dynamic_ended);

	if (!(dynamic && !dynamic_ended)) {
		Emit (MediaEndedEvent);
	} else {
		dynamic_waiting = true;
		LOG_PLAYLIST ("PlaylistRoot::EmitMediaEnded (): dynamic playlist hasn't ended yet.\n");
	}
}

void
PlaylistRoot::EmitBufferUnderflowEvent (EventObject *obj)
{
	PlaylistRoot *root = (PlaylistRoot *) obj;
	root->Emit (BufferUnderflowEvent);
}

MediaPlayer *
PlaylistRoot::GetMediaPlayer ()
{
	return mplayer;
}

Media *
PlaylistRoot::GetCurrentMedia ()
{
	PlaylistEntry *entry = GetCurrentEntry ();
	
	if (entry == NULL)
		return NULL;
	
	return entry->GetMedia ();
}

MediaElement *
PlaylistRoot::GetElement ()
{
	return element;
}

void
PlaylistRoot::MediaEndedHandler (MediaPlayer *mplayer, EventArgs *args)
{
	LOG_PLAYLIST ("PlaylistRoot::MediaEndedHandler (%p, %p)\n", mplayer, args);
	
	OnEntryEnded ();
}

void
PlaylistRoot::BufferUnderflowHandler (MediaPlayer *mplayer, EventArgs *args)
{
	LOG_PLAYLIST ("PlaylistRoot::BufferUnderflowHandler (%p, %p)\n", mplayer, args);
	
	if (Surface::InMainThread ()) {
		EmitBufferUnderflowEvent (this);
	} else {
		AddTickCall (EmitBufferUnderflowEvent);
	}
}

/*
 * PlaylistParser
 */

PlaylistParser::PlaylistParser (PlaylistRoot *root, MemoryBuffer *source)
{
	this->root = root;
	this->source = source;
	this->internal = NULL;
	this->kind_stack = NULL;
	this->playlist = NULL;
	this->current_entry = NULL;
	this->current_text = NULL;
	this->error_args = NULL;

	this->use_internal_asxparser = getenv ("MOONLIGHT_USE_EXPAT_ASXPARSER") == NULL;
}

void
PlaylistParser::Setup (XmlType type)
{
	playlist = NULL;
	current_entry = NULL;
	current_text = NULL;

	was_playlist = false;

	internal = new PlaylistParserInternal ();
	kind_stack = new List ();
	PushCurrentKind (PlaylistKind::Root);

	if (type == XML_TYPE_ASX3) {
		if (use_internal_asxparser) {
			internal->asxparser->SetUserData (this);
			internal->asxparser->SetElementStartHandler (on_start_element_internal_asxparser);
			internal->asxparser->SetElementEndHandler (on_end_element_internal_asxparser);
			internal->asxparser->SetTextHandler (on_text_internal_asxparser);
		} else {
			XML_SetUserData (internal->parser, this);
			XML_SetElementHandler (internal->parser, on_asx_start_element, on_asx_end_element);
			XML_SetCharacterDataHandler (internal->parser, on_asx_text);
		}
	}
	
}

void
PlaylistParser::Cleanup ()
{
	if (kind_stack) {
		kind_stack->Clear (true);
		delete kind_stack;
		kind_stack = NULL;
	}
	delete internal;
	internal = NULL;
	if (playlist) {
		playlist->unref ();
		playlist = NULL;
	}
	if (error_args) {
		error_args->unref ();
		error_args = NULL;
	}
}

PlaylistParser::~PlaylistParser ()
{
	Cleanup ();
}

static bool
str_match (const char *candidate, const char *tag)
{
	return g_ascii_strcasecmp (candidate, tag) == 0;
}


void
PlaylistParser::on_start_element_internal_asxparser (AsxParser *parser, const char *name, GHashTable *atts)
{
	PlaylistParser *pp = (PlaylistParser *) parser->GetUserData ();

	/*
	  For now, because we are runtime configurable, just make the internal asxparser look like
	  the expat parser by wrapping the attributes into an array.  Once the internal parser
	  is better tested we can remove this and change OnASXStartElement code to use the hashtable.
	*/

	GList *keys = g_hash_table_get_keys (atts);
	int kl = g_hash_table_size (atts);
	const char **attr_list = (const char **) g_malloc (sizeof (char *) * (kl + 1) * 2);

	int i = 0;
	while (keys) {
		attr_list [i++] = (const char *) keys->data;
		attr_list [i++] = (const char *) g_hash_table_lookup (atts, keys->data);
		keys = keys->next;
	}
	attr_list [i] = attr_list [i+1] = NULL;

	pp->OnASXStartElement (name, attr_list);

	g_list_free (keys);
	g_free (attr_list);
}

void
PlaylistParser::on_end_element_internal_asxparser (AsxParser *parser, const char *name)
{
	PlaylistParser *pp = (PlaylistParser *) parser->GetUserData ();
	pp->OnASXEndElement (name);
}

void
PlaylistParser::on_text_internal_asxparser (AsxParser *parser, const char *text)
{
	PlaylistParser *pp = (PlaylistParser *) parser->GetUserData ();
	pp->OnASXText (text, strlen (text));
}

void
PlaylistParser::on_asx_start_element (gpointer user_data, const char *name, const char **attrs)
{
	((PlaylistParser *) user_data)->OnASXStartElement (name, attrs);
}

void
PlaylistParser::on_asx_end_element (gpointer user_data, const char *name)
{
	((PlaylistParser *) user_data)->OnASXEndElement (name);
}

void
PlaylistParser::on_asx_text (gpointer user_data, const char *data, int len)
{
	((PlaylistParser *) user_data)->OnASXText (data, len);
}

static bool
is_all_whitespace (const char *str)
{
	if (str == NULL)
		return true;

	for (int i = 0; str [i] != 0; i++) {
		switch (str [i]) {
		case 10:
		case 13:
		case ' ':
		case '\t':
			break;
		default:
			return false;
		}
	}
	return true;
}

// 
// To make matters more interesting, the format of the VALUE attribute in the STARTTIME tag isn't
// exactly the same as xaml's or javascript's TimeSpan format.
// 
// The time index, in hours, minutes, seconds, and hundredths of seconds.
// [[hh]:mm]:ss.fract
// 
// The parser seems to stop if it finds a second dot, returnning whatever it had parsed
// up till then
//
// At most 4 digits of fract is read, the rest is ignored (even if it's not numbers).
// 
static bool
parse_int (const char **pp, const char *end, int *result)
{
	const char *p = *pp;
	int res = 0;
	bool success = false;

	while (p <= end && g_ascii_isdigit (*p)) {
		res = res * 10 + *p - '0';
		p++;
	}

	success = *pp != p;
	
	*pp = p;
	*result = res;

	return success;
}

static bool
duration_from_asx_str (PlaylistParser *parser, const char *str, Duration **res)
{
	const char *end = str + strlen (str);
	const char *p;

	int values [] = {0, 0, 0};
	int counter = 0;
	int hh = 0, mm = 0, ss = 0;
	int milliseconds = 0;
	int digits = 2;

	p = str;

	if (!g_ascii_isdigit (*p)) {
		parser->ParsingError (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 2210, "AG_E_INVALID_ARGUMENT")));
		return false;
	}

	for (int i = 0; i < 3; i++) {
		if (!parse_int (&p, end, &values [i])) {
			parser->ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 2210, "AG_E_INVALID_ARGUMENT")));
			return false;
		}
		counter++;
		if (*p != ':') 
			break;
		p++;
	}
	
	if (*p == '.') {
		p++;
		while (digits >= 0 && g_ascii_isdigit (*p)) {
			milliseconds += pow (10.0f, digits) * (*p - '0');
			p++;
			digits--;
		}
		if (counter == 3 && *p != 0 && !g_ascii_isdigit (*p)) {
			parser->ParsingError (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 2210, "AG_E_INVALID_ARGUMENT")));
			return false;
		}
	}
	
	switch (counter) {
	case 1:
		ss = values [0];
		break;
	case 2:
		ss = values [1];
		mm = values [0];
		break;
	case 3:
		ss = values [2];
		mm = values [1];
		hh = values [0];
		break;
	default:
		parser->ParsingError (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 2210, "AG_E_INVALID_ARGUMENT")));
		return false;		
	}

	gint64 ms = ((hh * 3600) + (mm * 60) + ss) * 1000 + milliseconds;
	TimeSpan result = TimeSpan_FromPts (MilliSeconds_ToPts (ms));
	Duration *duration = new Duration (result);

	*res = duration;

	return true;
}

void
PlaylistParser::OnASXStartElement (const char *name, const char **attrs)
{
	PlaylistKind::Kind kind = StringToKind (name);
	Uri *uri = NULL;
	bool failed;

	LOG_PLAYLIST ("PlaylistParser::OnStartElement (%s, %p), kind = %d\n", name, attrs, kind);

	g_free (current_text);
	current_text = NULL;

	PushCurrentKind (kind);

	switch (kind) {
	case PlaylistKind::Abstract:
		if (attrs != NULL && attrs [0] != NULL)
			ParsingError (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
		break;
	case PlaylistKind::Asx:
		// Here the kind stack should be: Root+Asx
		if (kind_stack->Length () != 2 || !AssertParentKind (PlaylistKind::Root)) {
			ParsingError (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
			return;
		}

		playlist = new Playlist (root);

		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "VERSION")) {
				if (str_match (attrs [i+1], "3")) {
					playlist_version = 3;
				} else if (str_match (attrs [i+1], "3.0")) {
					playlist_version = 3;
				} else {
					ParsingError (new ErrorEventArgs (MediaError,
									  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
				}
			} else if (str_match (attrs [i], "BANNERBAR")) {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3007, "Unsupported ASX attribute")));
			} else if (str_match (attrs [i], "PREVIEWMODE")) {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3007, "Unsupported ASX attribute")));
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
			}
		}
		break;
	case PlaylistKind::Author:
		if (attrs != NULL && attrs [0] != NULL)
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
		break;
	case PlaylistKind::Banner:
		if (attrs != NULL && attrs [0] != NULL)
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
		break;
	case PlaylistKind::Base:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "HREF")) {
				// TODO: What do we do with this value?
				if (GetCurrentContent () != NULL) {
					failed = false;
					uri = Uri::Create (attrs [i+1]);
					if (uri == NULL) {
						failed = true;
					} else if (uri->GetScheme() == NULL) {
						failed = true;
					} else if (uri->IsScheme ("http") && 
						   uri->IsScheme ("https") && 
						   uri->IsScheme ("mms") &&
						   uri->IsScheme ("rtsp") && 
						   uri->IsScheme ("rstpt")) {
						failed = true;
					}

					if (!failed) {
						GetCurrentContent ()->SetBase (uri);
					} else {
						delete uri;
						ParsingError (new ErrorEventArgs (MediaError,
										  MoonError (MoonError::EXCEPTION, 4001, "AG_E_NETWORK_ERROR")));
					}
					uri = NULL;
				}
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}
		break;
	case PlaylistKind::Copyright:
		if (attrs != NULL && attrs [0] != NULL)
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
		break;
	case PlaylistKind::Duration: {
		Duration *dur;
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "VALUE")) {
				if (duration_from_asx_str (this, attrs [i+1], &dur)) {
					if (GetCurrentEntry () != NULL && GetParentKind () != PlaylistKind::Ref) {
						LOG_PLAYLIST ("PlaylistParser::OnStartElement (%s, %p), found VALUE/timespan = %f s\n", name, attrs, TimeSpan_ToSecondsFloat (dur->GetTimeSpan()));
						GetCurrentEntry ()->SetDuration (dur);
					}
				}
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}
		break;
	}
	case PlaylistKind::Entry: {
		bool client_skip = true;
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "CLIENTSKIP")) {
				// TODO: What do we do with this value?
				if (str_match (attrs [i+1], "YES")) {
					client_skip = true;
				} else if (str_match (attrs [i+1], "NO")) {
					client_skip = false;
				} else {
					ParsingError (new ErrorEventArgs (MediaError,
									  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
					break;
				}
			} else if (str_match (attrs [i], "SKIPIFREF")) {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3007, "Unsupported ASX attribute")));
				break;
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}
		PlaylistEntry *entry = new PlaylistEntry (playlist);
		entry->SetClientSkip (client_skip);
		playlist->AddEntry (entry);
		current_entry = entry;
		break;
	}
	case PlaylistKind::EntryRef: {
		char *href = NULL;
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "HREF")) {
				if (href == NULL)
					href = g_strdup (attrs [i+1]);
			// Docs says this attribute isn't unsupported, but an error is emitted.
			//} else if (str_match (attrs [i], "CLIENTBIND")) {
			//	// TODO: What do we do with this value?
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}

		if (href) {
			uri = Uri::Create (href);
			if (uri == NULL) {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 1001, "AG_E_UNKNOWN_ERROR")));
			}
		}

		PlaylistEntry *entry = new PlaylistEntry (playlist);
		entry->SetIsEntryRef (true);
		if (uri)
			entry->SetSourceName (uri);
		uri = NULL;
		playlist->AddEntry (entry);
		current_entry = entry;
		break;
	}
	case PlaylistKind::LogUrl:
		if (attrs != NULL && attrs [0] != NULL)
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
		break;
	case PlaylistKind::MoreInfo:
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "HREF")) {
				if (GetCurrentEntry () != NULL)
					GetCurrentEntry ()->SetInfoURL (attrs [i+1]);
			} else if (str_match (attrs [i], "TARGET")) {
				if (GetCurrentEntry () != NULL)
					GetCurrentEntry ()->SetInfoTarget (attrs [i+1]);
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}
		break;
	case PlaylistKind::StartTime: {
		Duration *dur;
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "VALUE")) {
				if (duration_from_asx_str (this, attrs [i+1], &dur) && dur->HasTimeSpan ()) {
					if (GetCurrentEntry () != NULL && GetParentKind () != PlaylistKind::Ref) {
						LOG_PLAYLIST ("PlaylistParser::OnStartElement (%s, %p), found VALUE/timespan = %f s\n", name, attrs, TimeSpan_ToSecondsFloat (dur->GetTimeSpan()));
						GetCurrentEntry ()->SetStartTime (dur->GetTimeSpan ());
					}
				}
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}

		break;
	}
	case PlaylistKind::Ref: {
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "HREF")) {
				if (GetCurrentEntry () != NULL && GetCurrentEntry ()->GetSourceName () == NULL) {
					uri = Uri::Create (attrs [i+1]);
					if (uri != NULL) {
						GetCurrentEntry ()->SetSourceName (uri);
					} else {
						ParsingError (new ErrorEventArgs (MediaError,
										  MoonError (MoonError::EXCEPTION, 1001, "AG_E_UNKNOWN_ERROR")));
					}
					uri = NULL;
				}
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}
		break;
	}
	case PlaylistKind::Param: {
		const char *name = NULL;
		const char *value = NULL;
		
		for (int i = 0; attrs [i] != NULL; i+= 2) {
			if (str_match (attrs [i], "name")) {
				name = attrs [i + 1];				
			} else if (str_match (attrs [i], "value")) {
				value = attrs [i + 1];
			} else {
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
				break;
			}
		}
		if (value != NULL && value [0] != 0 && name != NULL && name [0] != 0) {
			PlaylistEntry *entry = GetCurrentEntry ();
			if (entry == NULL)
				entry = playlist;
			if (entry == NULL)
				entry = root;
			entry->AddParams (name, value);
		} else {
			// TODO: add test
		}
		break;
	}
	case PlaylistKind::Title:
		if (attrs != NULL && attrs [0] != NULL)
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3005, "Invalid ASX attribute")));
		break;
	case PlaylistKind::StartMarker:
	case PlaylistKind::EndMarker:
	case PlaylistKind::Repeat:
	case PlaylistKind::Event:
		ParsingError (new ErrorEventArgs (MediaError,
						  MoonError (MoonError::EXCEPTION, 3006, "Unsupported ASX element")));
		break;
	case PlaylistKind::Root:
	case PlaylistKind::Unknown:
	default:
		LOG_PLAYLIST ("PlaylistParser::OnStartElement ('%s', %p): Unknown kind: %d\n", name, attrs, kind);
		ParsingError (new ErrorEventArgs (MediaError,
						  MoonError (MoonError::EXCEPTION, 3004, "Invalid ASX element")));
		break;
	}
}

void
PlaylistParser::OnASXEndElement (const char *name)
{
	PlaylistKind::Kind kind = GetCurrentKind ();
	Duration *dur; 

	LOG_PLAYLIST ("PlaylistParser::OnEndElement (%s), GetCurrentKind (): %d, GetCurrentKind () to string: %s\n", name, kind, KindToString (kind));

	switch (kind) {
	case PlaylistKind::Abstract:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetAbstract (current_text);
		break;
	case PlaylistKind::Author:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetAuthor (current_text);
		break;
	case PlaylistKind::Base:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		break;
	case PlaylistKind::Copyright:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetCopyright (current_text);
		break;
	case PlaylistKind::Duration:
		if (!AssertParentKind (PlaylistKind::Entry | PlaylistKind::Ref))
			break;
		if (current_text == NULL)
			break;
		duration_from_asx_str (this, current_text, &dur);
		if (GetCurrentEntry () != NULL)
			GetCurrentEntry ()->SetDuration (dur);
		break;
	case PlaylistKind::Entry:
		if (!AssertParentKind (PlaylistKind::Asx))
			break;
		if (!is_all_whitespace (current_text)) {
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
		}
		break;
	case PlaylistKind::EntryRef:
		if (!AssertParentKind (PlaylistKind::Asx))
			break;
		break;
	case PlaylistKind::StartTime:
		if (!AssertParentKind (PlaylistKind::Entry | PlaylistKind::Ref))
			break;
		if (!is_all_whitespace (current_text)) {
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
		}
		break;
	case PlaylistKind::Title:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		if (GetCurrentContent () != NULL)
			GetCurrentContent ()->SetTitle (current_text);
		break;
	case PlaylistKind::Asx:
		if (playlist_version == 3)
			was_playlist = true;
		if (!AssertParentKind (PlaylistKind::Root))
			break;
		break;
	case PlaylistKind::Ref:
		if (!AssertParentKind (PlaylistKind::Entry))
			break;
		if (!is_all_whitespace (current_text)) {
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
		}
		break;
	case PlaylistKind::MoreInfo:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		if (!is_all_whitespace (current_text)) {
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
		}
		break;
	case PlaylistKind::Param:
		if (!AssertParentKind (PlaylistKind::Asx | PlaylistKind::Entry))
			break;
		if (!is_all_whitespace (current_text)) {
			ParsingError (new ErrorEventArgs (MediaError,
							  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
		}
		break;
	default:
		LOG_PLAYLIST ("PlaylistParser::OnEndElement ('%s'): Unknown kind %d.\n", name, kind);
		ParsingError (new ErrorEventArgs (MediaError,
						  MoonError (MoonError::EXCEPTION, 3004, "Invalid ASX element")));
		break;
	}
	
	if (current_text != NULL) {	
		g_free (current_text);
		current_text = NULL;
	}

	switch (GetCurrentKind ()) {
	case PlaylistKind::Entry:
		EndEntry ();
		break;
	default:
		break;
	}
	PopCurrentKind ();
}

void
PlaylistParser::OnASXText (const char *text, int len)
{
	char *a = g_strndup (text, len);

#if DEBUG
	char *p = g_strndup (text, len);
	for (int i = 0; p [i] != 0; i++)
		if (p [i] == 10 || p [i] == 13)
			p [i] = ' ';

	LOG_PLAYLIST ("PlaylistParser::OnText (%s, %d)\n", p, len);
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
PlaylistParser::Is (MemoryBuffer *source, const char *asx_header)
{
	int asx_header_length = strlen (asx_header);

	do {
		if (source->GetRemainingSize () < asx_header_length)
			return false;

		unsigned char a = source->PeekByte (0);
		switch (a) {
		case ' ':
		case '\t':
		case 10:
		case 13:
			/* Skip whitespace */
			source->SeekOffset (1);
			continue;
		case 0xef: {
			unsigned char b = source->PeekByte (1);
			unsigned char c = source->PeekByte (2);
			if (b == 0xbb && c == 0xbf) {
				/* UTF-8 BOM: EF BB BF. Skip it */
				source->SeekOffset (3);
				continue;
			}
			// TODO: there might be other BOMs we should handle too
			// fall through
		}
		default:
			bool result = !g_ascii_strncasecmp ((const char *) source->GetCurrentPtr (), asx_header, asx_header_length);
			return result;
		}
	} while (true);
}

bool
PlaylistParser::IsASX3 (MemoryBuffer *source)
{
	return Is (source, "<ASX");
}

bool
PlaylistParser::IsASX2 (MemoryBuffer *source)
{
	return Is (source, "[Reference]");
}

bool
PlaylistParser::ParseASX2 ()
{
	int bytes_read;
	char *buffer;
	char *ref;
	char *mms_uri;
	GKeyFile *key_file;
	Uri *uri;
	
	playlist_version = 2;

	buffer = (char *) source->GetCurrentPtr ();
	bytes_read = source->GetRemainingSize ();
	if (bytes_read <= 0) {
		LOG_PLAYLIST ("Could not read asx2 document for parsing.\n");
		return false;
	}

	key_file = g_key_file_new ();
	if (!g_key_file_load_from_data (key_file, buffer, bytes_read,
					G_KEY_FILE_NONE, NULL)) {
		LOG_PLAYLIST ("Invalid asx2 document.\n");
		g_key_file_free (key_file);
		return false;
	}

	ref = g_key_file_get_value (key_file, "Reference", "Ref1", NULL);
	if (ref == NULL) {
		LOG_PLAYLIST ("Could not find Ref1 entry in asx2 document.\n");
		g_key_file_free (key_file);
		return false;
	}

	if (!g_str_has_prefix (ref, "http://")) {
		LOG_PLAYLIST ("Could not find a valid uri within Ref1 entry in asx2 document.\n");
		g_free (ref);
		g_key_file_free (key_file);
		return false;
	}

	mms_uri = g_strdup_printf ("mms://%s", strstr (ref, "http://") + strlen ("http://"));
	g_free (ref);
	g_key_file_free (key_file);


	playlist = new Playlist (root);

	PlaylistEntry *entry = new PlaylistEntry (playlist);
	uri = Uri::Create (mms_uri);
	if (uri != NULL) {
		entry->SetSourceName (uri);
	}
	playlist->AddEntry (entry);
	current_entry = entry;

	return true;
}

bool
PlaylistParser::TryFixError (gint8 **buffer, guint32 *buffer_size, bool *free_buffer)
{
	gint8 *current_buffer = *buffer;
	
	if (XML_GetErrorCode (internal->parser) != XML_ERROR_INVALID_TOKEN)
		return false;

	guint32 index = XML_GetErrorByteIndex (internal->parser);

	// check that the index is within the buffer
	if (index >= *buffer_size)
		return false;

	LOG_PLAYLIST ("Attempting to fix invalid token error  index: %d\n", index);

	// OK, so we are going to guess that we are in an attribute here and walk back
	// until we hit a control char that should be escaped.
	const char * escape = NULL;
	while (index >= 0) {
		switch (current_buffer [index]) {
		case '&':
			escape = "&amp;";
			break;
		case '<':
			escape = "&lt;";
			break;
		case '>':
			escape = "&gt;";
			break;
		case '\"':
			break;
		}
		if (escape)
			break;
		index--;
	}

	if (!escape) {
		LOG_PLAYLIST ("Unable to find an invalid escape character to fix in ASX: %s.\n", current_buffer);
		return false;
	}

	int escape_len = strlen (escape);
	int new_size = *buffer_size + escape_len - 1;
	gint8 * new_buffer = (gint8 *) g_malloc (new_size);

	memcpy (new_buffer, current_buffer, index);
	memcpy (new_buffer + index, escape, escape_len);
	memcpy (new_buffer + index + escape_len, current_buffer + index + 1, *buffer_size - index - 1); 

	*buffer = new_buffer;
	*free_buffer = true;
	*buffer_size = new_size;

	if (error_args) {
		// Clear out errors in the old buffer
		error_args->unref ();
		error_args = NULL;
	}

	return true;
}

MediaResult
PlaylistParser::Parse ()
{
	bool result;

	LOG_PLAYLIST ("PlaylistParser::Parse ()\n");

	if (this->IsASX2 (source)) {
		/* Parse as a asx2 mms file */
		Setup (XML_TYPE_NONE);
		result = this->ParseASX2 ();
	} else if (this->IsASX3 (source)) {
		result = use_internal_asxparser ? this->ParseASX3Internal () : this->ParseASX3 ();
	} else {
		result = false;
	}

	return result ? MEDIA_SUCCESS : MEDIA_FAIL;
}

bool
PlaylistParser::ParseASX3 ()
{
	guint32 buffer_size;
	gint8 *buffer;
	bool result = true;
	bool reparse = false;
	bool free_buffer = false;

	/* We have the entire asx document in memory here */
	buffer = (gint8 *) source->GetCurrentPtr ();
	buffer_size = source->GetRemainingSize ();

	if (buffer_size <= 0) {
		fprintf (stderr, "Moonlight: ASX playlist is empty.\n");
		return false;
	}

	do {
		reparse = false;
		Setup (XML_TYPE_ASX3);
		if (!XML_Parse (internal->parser, (const char *) buffer, buffer_size, true)) {
			if (error_args != NULL) {
				result = false;
				break;
			}
			
			switch (XML_GetErrorCode (internal->parser)) {
			case XML_ERROR_NO_ELEMENTS:
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 7000, "unexpected end of input")));
				result = false;
				break;
			case XML_ERROR_DUPLICATE_ATTRIBUTE:
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 7031, "wfc: unique attribute spec")));
				result = false;
				break;
			case XML_ERROR_INVALID_TOKEN:
				// save error args in case the error fixing fails (in which case we want this error, not the error the error fixing caused)
				error_args = new ErrorEventArgs (MediaError,
								 MoonError (MoonError::EXCEPTION, 7007, "quote expected"));
				if (TryFixError (&buffer, &buffer_size, &free_buffer)) {
					reparse = true;
					break;
				}
				// fall through
			default:
				char *msg = g_strdup_printf ("%s %d (%d, %d)", 
					XML_ErrorString (XML_GetErrorCode (internal->parser)), (int) XML_GetErrorCode (internal->parser),
					(int) XML_GetCurrentLineNumber (internal->parser), (int) XML_GetCurrentColumnNumber (internal->parser));
				ParsingError (new ErrorEventArgs (MediaError,
								  MoonError (MoonError::EXCEPTION, 3000, msg)));
				g_free (msg);
				result = false;
				break;
			}
		}
	} while (reparse);

	if (free_buffer)
		g_free (buffer);

	return result && playlist != NULL;
}

bool
PlaylistParser::ParseASX3Internal ()
{
	Setup (XML_TYPE_ASX3);

	bool result = internal->asxparser->ParseBuffer (source);

	if (result)
		return true;

	switch (internal->asxparser->GetErrorCode ()) {
	case ASXPARSER_ERROR_NO_ELEMENTS:
		ParsingError (new ErrorEventArgs (MediaError,
				MoonError (MoonError::EXCEPTION, 7000, "unexpected end of input")));
		result = false;
		break;
	case ASXPARSER_ERROR_DUPLICATE_ATTRIBUTE:
		ParsingError (new ErrorEventArgs (MediaError,
				MoonError (MoonError::EXCEPTION, 7031, "wfc: unique attribute spec")));
		result = false;
		break;
	case ASXPARSER_ERROR_QUOTE_EXPECTED:
		ParsingError (new ErrorEventArgs (MediaError,
				MoonError (MoonError::EXCEPTION, 7007, "quote expected")));
		result = false;
		break;
	default:
		char *msg = g_strdup_printf ("%s %d (%d, %d)", 
				internal->asxparser->GetErrorMessage (),
				internal->asxparser->GetErrorCode (),
				internal->asxparser->GetCurrentLineNumber (),
				internal->asxparser->GetCurrentColumnNumber ());

		ParsingError (new ErrorEventArgs (MediaError,
				MoonError (MoonError::EXCEPTION, 3000, msg)));
		g_free (msg);
		result = false;
		break;
	}

	return result;
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
PlaylistParser::EndEntry ()
{
	this->current_entry = NULL;
}

void
PlaylistParser::PushCurrentKind (PlaylistKind::Kind kind)
{
	kind_stack->Append (new KindNode (kind));
	LOG_PLAYLIST ("PlaylistParser::Push (%d)\n", kind);
}

void
PlaylistParser::PopCurrentKind ()
{
	LOG_PLAYLIST ("PlaylistParser::PopCurrentKind (), current: %d\n", ((KindNode *)kind_stack->Last ())->kind);
	kind_stack->Remove (kind_stack->Last ());
}

PlaylistKind::Kind
PlaylistParser::GetCurrentKind ()
{
	KindNode *node = (KindNode *) kind_stack->Last ();
	return node->kind;
}

PlaylistKind::Kind
PlaylistParser::GetParentKind ()
{
	KindNode *node = (KindNode *) kind_stack->Last ()->prev;
	return node->kind;
}

bool
PlaylistParser::AssertParentKind (int kind)
{
	LOG_PLAYLIST ("PlaylistParser::AssertParentKind (%d), GetParentKind: %d, result: %d\n", kind, GetParentKind (), GetParentKind () & kind);

	if (GetParentKind () & kind)
		return true;

	ParsingError (new ErrorEventArgs (MediaError,
					  MoonError (MoonError::EXCEPTION, 3008, "ASX parse error")));
	
	return false;
}

void
PlaylistParser::ParsingError (ErrorEventArgs *args)
{
	LOG_PLAYLIST ("PlaylistParser::ParsingError (%s)\n", args->GetErrorMessage());

	if (use_internal_asxparser)
		internal->asxparser->Stop ();
	else
		XML_StopParser (internal->parser, false);

	if (error_args) {
		if (args)
			args->unref ();
		return; // don't overwrite any previous errors.
	}
	error_args = args; // don't ref, this method is called like this: ParsingError (new ErrorEventArgs (...));, so the caller gives us the ref he has
}


PlaylistKind PlaylistParser::playlist_kinds [] = {
	/* ASX3 */
	PlaylistKind ("ABSTRACT", PlaylistKind::Abstract), 
	PlaylistKind ("ASX", PlaylistKind::Asx),
	PlaylistKind ("ROOT", PlaylistKind::Root),
	PlaylistKind ("AUTHOR", PlaylistKind::Author),
	PlaylistKind ("BANNER", PlaylistKind::Banner),
	PlaylistKind ("BASE", PlaylistKind::Base),
	PlaylistKind ("COPYRIGHT", PlaylistKind::Copyright),
	PlaylistKind ("DURATION", PlaylistKind::Duration),
	PlaylistKind ("ENTRY", PlaylistKind::Entry),
	PlaylistKind ("ENTRYREF", PlaylistKind::EntryRef),
	PlaylistKind ("LOGURL", PlaylistKind::LogUrl),
	PlaylistKind ("MOREINFO", PlaylistKind::MoreInfo),
	PlaylistKind ("REF", PlaylistKind::Ref),
	PlaylistKind ("STARTTIME", PlaylistKind::StartTime),
	PlaylistKind ("TITLE", PlaylistKind::Title),
	PlaylistKind ("STARTMARKER", PlaylistKind::StartMarker),
	PlaylistKind ("REPEAT", PlaylistKind::Repeat),
	PlaylistKind ("ENDMARKER", PlaylistKind::EndMarker),
	PlaylistKind ("PARAM", PlaylistKind::Param),
	PlaylistKind ("EVENT", PlaylistKind::Event),

	PlaylistKind (NULL, PlaylistKind::Unknown)
};

PlaylistKind::Kind
PlaylistParser::StringToKind (const char *str)
{
	PlaylistKind::Kind kind = PlaylistKind::Unknown;

	for (int i = 0; playlist_kinds [i].str != NULL; i++) {
		if (str_match (str, playlist_kinds [i].str)) {
			kind = playlist_kinds [i].kind;
			break;
		}
	}

	LOG_PLAYLIST ("PlaylistParser::StringToKind ('%s') = %d\n", str, kind);

	return kind;
}

const char *
PlaylistParser::KindToString (PlaylistKind::Kind kind)
{
	const char *result = NULL;

	for (int i = 0; playlist_kinds [i].str != NULL; i++) {
		if (playlist_kinds [i].kind == kind) {
			result = playlist_kinds [i].str;
			break;
		}
	}

	LOG_PLAYLIST ("PlaylistParser::KindToString (%d) = '%s'\n", kind, result);

	return result;
}

};
