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
 * PlaylistEntry
 */

PlaylistEntry::PlaylistEntry (Playlist *root, PlaylistEntry *parent)
	: EventObject (Type::PLAYLISTENTRY, false)
{
	LOG_PLAYLIST ("PlaylistEntry::PlaylistEntry (root: %p = %i, parent: %p = %i) id: %i\n", root, GET_OBJ_ID (root), parent, GET_OBJ_ID (parent), GET_OBJ_ID (this));

	base = NULL;
	title = NULL;
	author = NULL;
	abstract = NULL;
	copyright = NULL;
	source_name = NULL;
	info_target = NULL;
	info_url = NULL;
	client_skip = true;
	is_entry_ref = false;
	start_time = 0;
	duration = NULL;
	params = NULL;

	set_values = (PlaylistKind::Kind) 0;

	full_source_name = NULL;
	is_live = false;
	play_when_available = false;
	this->parent = parent; // might be null if we're the root entry
	this->root = root;
	media = NULL;
	opened = false;

	current_node = NULL;

	dynamic = false;
	dynamic_ended = false;
	dynamic_waiting = false;
}

void
PlaylistEntry::Dispose ()
{
	PlaylistNode *node;
	PlaylistEntry *entry;

	LOG_PLAYLIST ("PlaylistEntry::Dispose () id: %i media: %i\n", GET_OBJ_ID (this), GET_OBJ_ID (media));

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
	delete source_name;
	source_name = NULL;
	g_free (info_target);
	info_target = NULL;
	g_free (info_url);
	info_url = NULL;
	delete duration;
	duration = NULL;

	if (params != NULL) {
		g_hash_table_destroy (params);
		params = NULL;
	}

	delete full_source_name;
	full_source_name = NULL;

	parent = NULL;
	root = NULL;

	if (media) {
		Media *tmp = media;
		media = NULL;
		tmp->RemoveSafeHandlers (this);
		tmp->DisposeObject (tmp);
		tmp->unref ();
	}

	current_node = NULL;


	LOG_PLAYLIST ("Playlist::Dispose () id: %i\n", GET_OBJ_ID (this));
	
	current_node = NULL;

	node = (PlaylistNode *) entries.First ();
	while (node != NULL) {
		entry = node->GetElement ();
		if (entry != NULL)
			entry->Dispose ();
		node = (PlaylistNode *) node->next;
	}

	EventObject::Dispose ();
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

	g_return_if_fail (callbacks != NULL);

	media = new Media (this);
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

	g_return_if_fail (source != NULL);

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

	g_return_if_fail (uri != NULL);

	media = new Media (this);
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
	
	g_return_if_fail (dl != NULL);

	media = new Media (this);
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
	
	g_return_if_fail (demuxer != NULL);

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
	Playlist *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::OpeningHandler (%p, %p) current entry: %i\n", media, args, root->GetCurrentEntryLeaf () == this);
	
	g_return_if_fail (root != NULL);
	
	if (root->GetCurrentEntryLeaf () == this)
		root->Emit (Playlist::OpeningEvent, args);
}

void
PlaylistEntry::OpenMediaPlayer ()
{
	Playlist *root = GetRoot ();
	MediaPlayer *mplayer;
	
	g_return_if_fail (opened == true);
	g_return_if_fail (root != NULL);
	
	mplayer = GetMediaPlayer ();
	g_return_if_fail (mplayer != NULL);

	mplayer->Open (media, this);
	
	root->Emit (Playlist::OpenCompletedEvent, NULL);
}

void
PlaylistEntry::OpenCompletedHandler (Media *media, EventArgs *args)
{
	IMediaDemuxer *demuxer;
	bool is_mms_demuxer;

	LOG_PLAYLIST ("PlaylistEntry::OpenCompletedHandler (%p = %i, %p) id: %i entries: %i opened: %i\n", media, GET_OBJ_ID (media), args, GET_OBJ_ID (this), entries.Length (), opened);

	if (entries.Length () == 0) {
		if (parent == NULL && !root->IsSingleFile ()) {
			/* ASX file with nothing in it. our playlist test #656 (empty.asx) */
			return;
		}

		demuxer = media->GetDemuxerReffed ();
		if (demuxer) {
			is_mms_demuxer = demuxer->Is (Type::MMSDEMUXER);
			demuxer->unref ();
			if (is_mms_demuxer) {
				/* Mms demuxers should not propagate the open event, its entries will also raise open events */
				return;
			}
		}

		opened = true;

		if (parent == NULL || parent->GetCurrentEntry () == this) {
			OpenMediaPlayer ();
		} else {
			LOG_PLAYLIST ("PlaylistEntry::OpenCompletedHandler (%p = %i, %p): id: %i opened entry in advance, waiting for current entry to finish.\n", media, GET_OBJ_ID (media), args, GET_OBJ_ID (this));
		}
	} else {
		// check for too many nested playlists
		int counter = 0;
		PlaylistEntry *e = this;
		while (e != NULL && !e->GetIsDynamic ()) {
			counter++;

			if (counter > 6) {
				ErrorEventArgs *args = new ErrorEventArgs (MediaError,
									   MoonError (MoonError::EXCEPTION, 4001, "AG_E_NETWORK_ERROR"));
				OnEntryFailed (args);
				args->unref ();
				return;
			}

			e = e->GetParent ();
		}

		demuxer = media->GetDemuxerReffed ();
		if (demuxer) {
			is_mms_demuxer = demuxer->Is (Type::MMSDEMUXER);
			demuxer->unref ();
			if (is_mms_demuxer) {
				/* Mms demuxers should not propagate the open event, its entries will also raise open events */
				return;
			}
		}

		opened = true;
		Open (); // open any nested elements
	}
}

void
PlaylistEntry::SeekingHandler (Media *media, EventArgs *args)
{
	Playlist *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::SeekingHandler (%p, %p)\n", media, args);

	if (root == NULL)
		return;
	
	if (args)
		args->ref ();
	root->Emit (Playlist::SeekingEvent, args);
}

void
PlaylistEntry::SeekCompletedHandler (Media *media, EventArgs *args)
{
	Playlist *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::SeekCompletedHandler (%p, %p)\n", media, args);

	if (root == NULL)
		return;

	if (args)
		args->ref ();
	root->Emit (Playlist::SeekCompletedEvent, args);
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

	OnEntryFailed (args);
}

void
PlaylistEntry::DownloadProgressChangedHandler (Media *media, EventArgs *args)
{
	Playlist *root;
	
	LOG_PLAYLIST ("PlaylistEntry::DownloadProgressChanged (%p, %p %.2f). Disposed: %i\n", media, args, args ? ((ProgressEventArgs *) args)->progress : -1.0, IsDisposed ());
	
	if (IsDisposed ())
		return;
	
	root = GetRoot ();
	
	g_return_if_fail (root != NULL);
	
	if (root->GetCurrentEntryLeaf () != this) {
		/* FIXME: we should stop any running downloads when we advance from one playlistentry
		 * to the next. Currently this will fail because we won't ever resume downloads if the
		 * user decides to stop&replay the media */
		LOG_PLAYLIST ("PlaylistEntry::DownloadProgressChangedHandler (): Got download progress changed handler (%.2f) for an inactive media\n", args ? ((ProgressEventArgs *) args)->progress : -1.0);
		return;
	}

	if (args)
		args->ref ();
	root->Emit (Playlist::DownloadProgressChangedEvent, args);
}

void
PlaylistEntry::BufferingProgressChangedHandler (Media *media, EventArgs *args)
{
	Playlist *root = GetRoot ();
	
	LOG_PLAYLIST ("PlaylistEntry::BufferingProgressChanged (%p, %p) %.2f\n", media, args, args ? ((ProgressEventArgs *) args)->progress : -1.0);
	
	if (root == NULL)
		return; // this might happen if the media is still buffering and we're in the process of getting cleaned up
	
	if (args)
		args->ref ();
	root->Emit (Playlist::BufferingProgressChangedEvent, args);
}

void
PlaylistEntry::AddParams (const char *name, const char *value)
{
#if PLUMB_ME
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
#endif
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
		delete this->duration;
		this->duration = new Duration (*duration);
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
	if (root == NULL)
		return NULL;

	return root->GetElement ();
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
	g_return_val_if_fail (root != NULL, NULL);

	return root->GetMediaPlayer ();
}

static void
add_attribute (MediaAttributeCollection *attributes, const char *name, const char *attr)
{
	MediaAttribute *attribute;

	if (!attr)
		return;

	for (int i = 0; i < attributes->GetCount (); i++) {
		attribute = attributes->GetValueAt (i)->AsMediaAttribute ();
		if (strcmp (attribute->GetName (), name) == 0)
			return;
	}

	LOG_PLAYLIST ("PlaylistEntry::AddAttribute () added '%s'='%s'\n", name, attr);

	attribute = new MediaAttribute ();
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

const Uri *
PlaylistEntry::GetLocation ()
{
	if (media != NULL)
		return media->GetFinalUri ();
	if (parent != NULL)
		return parent->GetLocation ();
	return NULL;
}

const Uri *
PlaylistEntry::GetFullSourceName ()
{
	if (full_source_name == NULL) {
		Uri *base = GetBaseInherited ();
		Uri *current = GetSourceName ();
		const Uri *location = GetLocation ();
		
		LOG_PLAYLIST ("PlaylistEntry::GetFullSourceName (), base: %s, current: %s location: %s\n", base ? base->ToString () : "NULL", current ? current->ToString () : "NULL", location ? location->ToString () : NULL);
		
		if (current == NULL) {
			full_source_name = NULL;
		} else if (current->IsAbsolute ()) {
			full_source_name = Uri::Clone (current);
		} else if (base != NULL) {
			full_source_name = Uri::Create (base, current);
		} else if (location != NULL) {
			full_source_name = Uri::Create (location, current);
		} else {
			full_source_name = Uri::Clone (current);
		}
	}
	return full_source_name;
}

void
PlaylistEntry::Open ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("PlaylistEntry::Open () id: %i media = %p entries: %i FullSourceName = %s\n", GET_OBJ_ID (this), media, entries.Length (), GetFullSourceName () != NULL ? GetFullSourceName ()->GetOriginalString () : NULL);

	if (entries.Length () == 0) {
		if (!media) {
			if (GetFullSourceName () == NULL) {
				fprintf (stderr, "Moonlight: An entry in the playlist didn't specify a url.\n");
				return;
			}
			InitializeWithUri (GetElement ()->GetResourceBase (), GetFullSourceName ());
		} else if (opened) {
			OpenMediaPlayer ();
		} else {
			media->OpenAsync ();
		}
	} else {
		current_node = (PlaylistNode *) entries.First ();
		current_entry = current_node->GetElement ();

		/* Skip elements with a zero duration */
		while (current_entry && current_entry->HasDuration () && current_entry->GetDuration ()->HasTimeSpan() && current_entry->GetDuration ()->GetTimeSpan () == 0) {
			LOG_PLAYLIST ("PlaylistEntry::Open (), current entry (%s) has zero duration, skipping it.\n", current_entry->GetSourceName ()->ToString ());
			current_node = (PlaylistNode *) current_node->next;
			current_entry = current_node ? current_node->GetElement () : NULL;
		}

		if (current_entry)
			current_entry->Open ();
		opened = true;
	}

	LOG_PLAYLIST ("Playlist::Open (): current node: %p, current entry: %p\n", current_entry, GetCurrentEntry ());
}

Media *
PlaylistEntry::GetMedia ()
{
	return media;
}

Playlist *
PlaylistEntry::GetRoot ()
{
	return root;
}

bool
PlaylistEntry::PlayNext ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("PlaylistEntry::PlayNext () %i current_node: %p %i length: %i\n", GET_OBJ_ID (this), current_node, current_node ? GET_OBJ_ID (current_node->GetElement ()) : 0, entries.Length ());
	VERIFY_MAIN_THREAD;

	if (root == NULL)
		return false;

	if (entries.Length () == 0) {
		LOG_PLAYLIST ("PlaylistEntry::PlayNext () %i no entries and we're the root element, opening myself.\n", GET_OBJ_ID (this));
		GetElement ()->SetPlayRequested ();
		root->Emit (Playlist::EntryChangedEvent);
		Open ();
		return true;
	}

	current_entry = GetCurrentEntry ();

	if (!current_entry) {
		/* We've already played beyond the end */
		LOG_PLAYLIST ("PlaylistEntry::PlayNext () %i we've already played beyond the end.\n", GET_OBJ_ID (this));
		return false;
	}

	if (current_entry->IsPlaylist () && current_entry->PlayNext ()) {
		/* We've already played beyond the end */
		LOG_PLAYLIST ("PlaylistEntry::PlayNext () %i nested element/playlist.\n", GET_OBJ_ID (this));
		return true;
	}

	/* Go to next element */
	current_node = (PlaylistNode *) current_node->next;
	current_entry = current_node ? current_node->GetElement () : NULL;

	/* Skip elements with a zero duration */
	while (current_entry && current_entry->HasDuration () && current_entry->GetDuration ()->HasTimeSpan() && current_entry->GetDuration ()->GetTimeSpan () == 0) {
		LOG_PLAYLIST ("PlaylistEntry::PlayNext (), current entry (%s) has zero duration, skipping it.\n", current_entry->GetSourceName ()->ToString ());
		current_node = (PlaylistNode *) current_node->next;
		current_entry = current_node ? current_node->GetElement () : NULL;
	}

	if (!current_node) {
		LOG_PLAYLIST ("PlaylistEntry::PlayNext () %i nothing to play. is root element: %i\n", GET_OBJ_ID (this), parent == NULL);
		if (parent == NULL)
			EmitMediaEnded ();
		return false;
	}

	if (!current_entry->IsPlaylist ()) {
		LOG_PLAYLIST ("PlaylistEntry::PlayNext () %i playing next entry: %i\n", GET_OBJ_ID (this), GET_OBJ_ID (current_entry));
		GetElement ()->SetPlayRequested ();
		root->Emit (Playlist::EntryChangedEvent);
		current_entry->Open ();
		return true;
	}

	LOG_PLAYLIST ("PlaylistEntry::PlayNext () %i entering next entry, which is a playlist: %i.\n", GET_OBJ_ID (this), GET_OBJ_ID (current_entry));

	return current_entry->PlayNext ();
}

void
PlaylistEntry::OnEntryEnded ()
{
	LOG_PLAYLIST ("PlaylistEntry::OnEntryEnded () %i\n", GET_OBJ_ID (this));

	if (!root)
		return;

	if (root->GetFirstEntry () == this) {
		EmitMediaEnded ();
	} else {
		root->GetFirstEntry ()->PlayNext ();
	}
}

void
PlaylistEntry::OnEntryFailed (ErrorEventArgs *args)
{	
	bool fatal = true;
	
	LOG_PLAYLIST ("PlaylistEntry::OnEntryFailed () extended_code: %i\n", args ? args->GetExtendedCode() : 0);

	if (root == NULL) {
#if DEBUG
		if (args) {
			printf ("Moonlight: there was an error in the media pipeline which couldn't be delivered to the plugin (most likely because MediaElement's source has changed): %i %s %s\n",
				args->GetErrorCode (), args->GetErrorMessage (), args->GetExtendedMessage ());
		}
#endif
		return;
	}

	// media or playlist 404: fatal
	// invalid playlist (playlist parsing failed): fatal
	// invalid media (gif, swf): play next (#78)
	if (args == NULL) {
		fatal = true;
	} else {
		// check if we're in a playlist
		if (parent != NULL && parent->IsASXDemuxer ()) {
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
	}
	
	if (fatal) {
		if (args)
			args->ref ();
		root->Emit (Playlist::MediaErrorEvent, args);
	} else {
		root->GetFirstEntry ()->PlayNext ();
	}
}

void
PlaylistEntry::Seek (guint64 pts)
{
	PlaylistEntry *current_entry;
	
	LOG_CUSTOM (RUNTIME_DEBUG_SEEK | RUNTIME_DEBUG_PLAYLIST, "Playlist::Seek (%" G_GUINT64_FORMAT ") id: %i entries: %i\n", pts, GET_OBJ_ID (this), entries.Length ());

	if (entries.Length () == 0) {
		g_return_if_fail (media != NULL);
		media->SeekAsync (pts);
	} else {
		current_entry = GetCurrentEntry ();
		g_return_if_fail (current_entry != NULL);
		current_entry->Seek (pts);
	}
}

void
PlaylistEntry::Play ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("PlaylistEntry::Play () id: %i entries: %i\n", GET_OBJ_ID (this), entries.Length ());

	if (entries.Length () == 0) {
		MediaPlayer *mplayer = GetMediaPlayer ();

		LOG_PLAYLIST ("PlaylistEntry::Play (), play_when_available: %s, media: %p, source name: %s\n", play_when_available ? "true" : "false", media, source_name ? source_name->ToString () : "NULL");

		g_return_if_fail (media != NULL);
		g_return_if_fail (mplayer != NULL);
		g_return_if_fail (root != NULL);

		media->PlayAsync ();
		mplayer->Play ();

		root->Emit (Playlist::PlayEvent);
	} else {
	 	current_entry = GetCurrentEntry ();

	 	g_return_if_fail (current_entry != NULL);

		if (current_entry && current_entry->HasDuration () && current_entry->GetDuration () == 0) {
			LOG_PLAYLIST ("PlaylistEntry::Open (), current entry (%s) has zero duration, skipping it.\n", current_entry->GetSourceName ()->ToString ());
			OnEntryEnded ();
		} else {
			current_entry->Play ();
		}
	}
}

void
PlaylistEntry::Pause ()
{
	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("Playlist::Pause () id: %i entries: %i\n", GET_OBJ_ID (this), entries.Length ());

	if (entries.Length () == 0) {
		MediaPlayer *mplayer = GetMediaPlayer ();

		g_return_if_fail (media != NULL);
		g_return_if_fail (mplayer != NULL);
		g_return_if_fail (root != NULL);
		
		play_when_available = false;
		media->PauseAsync ();
		mplayer->Pause ();
		
		root->Emit (Playlist::PauseEvent);
	} else {
		current_entry = GetCurrentEntry ();

		g_return_if_fail (current_entry != NULL);

		current_entry->Pause ();
	}
}

void
PlaylistEntry::Stop ()
{
	PlaylistNode *node;

	LOG_PLAYLIST ("Playlist::Stop () id: %i entries: %i\n", GET_OBJ_ID (this), entries.Length ());

	if (entries.Length () == 0) {
		play_when_available = false;
		if (media != NULL)
			media->StopAsync ();
	} else {
		node = (PlaylistNode *) entries.First ();
		current_node = node;  // reset to first node
		while (node != NULL) {
			node->GetElement ()->Stop ();
			node = (PlaylistNode *) node->next;
		}
	}
}

void
PlaylistEntry::PopulateMediaAttributes ()
{
	MediaElement *element;
	PlaylistEntry *current;
	MediaAttributeCollection *attributes;

	const char *abstract = NULL;
	const char *author = NULL;
	const char *copyright = NULL;
	const char *title = NULL;
	const char *infotarget = NULL;
	const char *infourl = NULL;
	const char *baseurl = NULL;

	PlaylistEntry *current_entry;

	LOG_PLAYLIST ("Playlist::PopulateMediaAttributes () id: %i entries: %i\n", GET_OBJ_ID (this), entries.Length ());

	if (entries.Length () != 0) {
		current_entry = GetCurrentEntry ();

		if (!current_entry)
			return;

		current_entry->PopulateMediaAttributes ();
		return;
	}

	element = GetElement ();
	current = this;

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
	while (current != NULL) {
		if (current->params != NULL)
			g_hash_table_foreach (current->params, (GHFunc) add_attribute_glib, attributes);
		if (current->GetIsEntryRef () && current->IsASXDemuxer ())
			break;
		current = current->GetParent ();
	}
}

bool
PlaylistEntry::IsASXDemuxer ()
{
	bool result = false;
	IMediaDemuxer *demuxer;

	if (media == NULL)
		return false;

	demuxer = media->GetDemuxerReffed ();
	if (demuxer != NULL) {
		result = demuxer->Is (Type::ASXDEMUXER);
		demuxer->unref ();
	}

	return result;
}

void
PlaylistEntry::AddEntry (PlaylistEntry *entry)
{
	PlaylistNode *node;
	
	LOG_PLAYLIST ("PlaylistEntry::AddEntry (%p) Count: %i\n", entry, entries.Length ());

	node = new PlaylistNode (entry);
	entries.Append (node);
	
	if (entries.Length () == 1) {
		current_node = node;
	}
}

PlaylistEntry *
PlaylistEntry::GetCurrentEntryLeaf ()
{
	if (current_node)
		return current_node->GetElement ()->GetCurrentEntryLeaf ();
	
	return this;
}

void
PlaylistEntry::SetHasDynamicEndedCallback (EventObject *obj)
{
	((PlaylistEntry *) obj)->SetHasDynamicEnded ();
}

void
PlaylistEntry::SetHasDynamicEnded ()
{
	LOG_PLAYLIST ("PlaylistEntry::SetHasDynamicEnded () InMainThread: %i\n", Surface::InMainThread ());

	if (!Surface::InMainThread ()) {
		AddTickCall (SetHasDynamicEndedCallback);
		return;
	}

	dynamic_ended = true;
	if (dynamic_waiting) {
		dynamic_waiting = false;

		if (root)
			EmitMediaEnded ();
	}
}

void
PlaylistEntry::EmitMediaEnded ()
{
	/* We only emit MediaEnded if we're a static playlist, or a dynamic whose end has been signalled */
	LOG_PLAYLIST ("PlaylistEntry::EmitMediaEnded () %i dynamic: %i dynamic_ended: %i parent: %i\n", GET_OBJ_ID (this), dynamic, dynamic_ended, GET_OBJ_ID (parent));

	if (root == NULL)
		return;

	if (dynamic) {
		if (!dynamic_ended) {
			/* dynamic playlist, wait for more */
			dynamic_waiting = true;
			LOG_PLAYLIST ("PlaylistRoot::EmitMediaEnded (): dynamic playlist hasn't ended yet.\n");
		} else if (parent == NULL) {
			/* we're the root entry of a (ended) dynamic playlist, raise MediaEnded */
			root->Emit (Playlist::MediaEndedEvent);
		} else {
			/* let the parent decide */
			parent->EmitMediaEnded ();
		}
	} else {
		if (parent == NULL) {
			/* we're the root entry, raise MediaEnded */
			root->Emit (Playlist::MediaEndedEvent);
		} else {
			/* let the parent decide */
			parent->EmitMediaEnded ();
		}
	}
}

bool
PlaylistEntry::IsCurrent ()
{
	if (parent == NULL)
		return true; /* Only 1 top-level entry */

	if (parent->GetCurrentEntry () != this)
		return false;

	return parent->IsCurrent ();
}


/*
 * Playlist
 */

Playlist::Playlist (MediaElement *element)
	: EventObject (Type::PLAYLIST)
{
	this->element = element;
	
	mplayer = element->GetMediaPlayer ();
	mplayer->AddHandler (MediaPlayer::MediaEndedEvent, MediaEndedCallback, this);
	mplayer->AddHandler (MediaPlayer::BufferUnderflowEvent, BufferUnderflowCallback, this);
	mplayer->ref ();

	entry = new PlaylistEntry (this, NULL);
}

void
Playlist::Dispose ()
{
	element = NULL;

	if (mplayer != NULL) {
		mplayer->RemoveAllHandlers (this);
		mplayer->unref ();
		mplayer = NULL;
	}

	if (entry != NULL) {
		entry->Dispose ();
		entry->unref ();
		entry = NULL;
	}

	EventObject::Dispose ();
}


bool
Playlist::Emit (int event_id, EventArgs *calldata)
{
	return EventObject::Emit (event_id, calldata);
}

bool
Playlist::IsSingleFile ()
{
	return !entry->IsASXDemuxer ();
}

#if DEBUG
void
PlaylistEntry::Dump (int tabs, bool is_current, GString *fmt, bool html)
{
	int ps = html ? 4 : 4;
	char pad [tabs * ps + 1];
	char pad2 [(tabs + 1) * ps + 1];
	for (int i = 0; i < tabs; i++)
		memcpy (pad + i * ps, !html ? "     " : "    ", ps);
	for (int i = 0; i < tabs + 1; i++)
		memcpy (pad2 + i * ps, !html ? "     " : "    ", ps);
	pad [tabs * ps] = 0;
	pad2 [(tabs + 1) * ps] = 0;

	if (is_current) {
		g_string_append (fmt, html ? "<span foreground='blue'>" : "\033[34;49m");
	}
	g_string_append_printf (fmt, "%s%s %i Dynamic: %s [CURRENT NODE]", pad, GetIsEntryRef () ? "EntryRef" : "Entry", GET_OBJ_ID (this), GetIsDynamic () ? "yes" : "no");
	if (is_current) {
		g_string_append (fmt, html ? "</span>" : "\033[39;49m");
	}
	g_string_append_printf (fmt, "\n");
	tabs++;
	g_string_append_printf (fmt, "%sParent: %p %s %i\n", pad2, parent, parent ? parent->GetTypeName () : NULL, GET_OBJ_ID (parent));
	g_string_append_printf (fmt, "%sFullSourceName: %s\n", pad2, GetFullSourceName () != NULL ? GetFullSourceName ()->GetOriginalString () : NULL);
	g_string_append_printf (fmt, "%sDuration: %s %.2f seconds StartTime: %" G_GUINT64_FORMAT " ms\n", pad2, HasDuration () ? "yes" : "no", HasDuration () ? GetDuration ()->ToSecondsFloat () : 0.0, MilliSeconds_FromPts (GetStartTime ()));
	g_string_append_printf (fmt, "%sMedia: %i %s\n", pad2, GET_OBJ_ID (media), media ? "" : "(null)");
	if (media) {
		IMediaDemuxer *demuxer = media->GetDemuxerReffed ();
		g_string_append_printf (fmt, "%sUri: %s\n", pad2, media->GetUri () ? media->GetUri ()->GetOriginalString () : NULL);
		g_string_append_printf (fmt, "%sDemuxer: %i %s Pending stream: %s\n", pad2, GET_OBJ_ID (demuxer), demuxer ? demuxer->GetTypeName () : "N/A", demuxer && demuxer->GetPendingStream () ? demuxer->GetPendingStream ()->GetTypeName () : "None");
		g_string_append_printf (fmt, "%sSource:  %i %s\n", pad2, GET_OBJ_ID (media->GetSource ()), media->GetSource () ? media->GetSource ()->GetTypeName () : "N/A");
		if (demuxer)
			demuxer->unref ();
	}

	if (entries.Length () > 0) {
		PlaylistNode *node;

		g_string_append_printf (fmt, "%s %i entries:\n", pad2, entries.Length ());
		node = (PlaylistNode *) entries.First ();
		while (node != NULL) {
			node->GetElement ()->Dump (tabs + 1, node == current_node && is_current, fmt, html);
			node = (PlaylistNode *) node->next;
		}
	}
}

void
Playlist::Dump ()
{
	GString *fmt = g_string_new ("");
	Dump (fmt, false);
	printf (fmt->str);
	g_string_free (fmt, true);
}

void
Playlist::Dump (GString *fmt, bool html)
{
	int flags = debug_flags;
	debug_flags &= ~RUNTIME_DEBUG_PLAYLIST;
	entry->Dump (0, true, fmt, html);
	debug_flags = flags;
}
#endif

void
Playlist::SeekCallback (EventObject *obj)
{
	Playlist *playlist = (Playlist *) obj;
	List::GenericNode<guint64> *pts_node;
	
	LOG_CUSTOM (RUNTIME_DEBUG_SEEK | RUNTIME_DEBUG_PLAYLIST, "Playlist::SeekCallback ()\n");

	if (playlist->IsDisposed ())
		return;

	pts_node = (List::GenericNode<guint64> *) playlist->seeks.First ();
	if (pts_node != NULL) {
		playlist->seeks.Unlink (pts_node);
		playlist->entry->Seek (pts_node->GetElement ());
		delete pts_node;
	}
}

void
Playlist::SeekAsync (guint64 pts)
{
	LOG_CUSTOM (RUNTIME_DEBUG_SEEK | RUNTIME_DEBUG_PLAYLIST, "Playlist::SeekAsync (%" G_GUINT64_FORMAT ")\n", pts);
	seeks.Append (new List::GenericNode<guint64> (pts));
	AddTickCall (SeekCallback);
}

void
Playlist::PlayCallback (EventObject *obj)
{
	LOG_PLAYLIST ("Playlist::PlayCallback ()\n");

	Playlist *root = (Playlist *) obj;
	if (root->IsDisposed ())
		return;
	root->entry->Play ();
}

void
Playlist::PlayAsync ()
{
	LOG_PLAYLIST ("Playlist::PlayAsync ()\n");
	AddTickCall (PlayCallback);
}

void
Playlist::PauseCallback (EventObject *obj)
{
	LOG_PLAYLIST ("Playlist::PauseCallback ()\n");

	Playlist *root = (Playlist *) obj;
	if (root->IsDisposed ())
		return;
	root->entry->Pause ();
}

void
Playlist::PauseAsync ()
{
	LOG_PLAYLIST ("Playlist::PauseAsync ()\n");
	AddTickCall (PauseCallback);
}

void
Playlist::OpenCallback (EventObject *obj)
{
	LOG_PLAYLIST ("Playlist::OpenCallback ()\n");

	Playlist *root = (Playlist *) obj;
	if (root->IsDisposed ())
		return;
	root->entry->Open ();
}

void
Playlist::OpenAsync ()
{
	LOG_PLAYLIST ("Playlist::OpenAsync ()\n");
	AddTickCall (OpenCallback);
}

void
Playlist::StopCallback (EventObject *obj)
{
	LOG_PLAYLIST ("Playlist::StopCallback ()\n");

	Playlist *root = (Playlist *) obj;
	if (root->IsDisposed ())
		return;
	root->Stop ();
}

void
Playlist::StopAsync ()
{
	LOG_PLAYLIST ("PlaylistRoot::StopAsync ()\n");
	AddTickCall (StopCallback);
}

void
Playlist::Stop ()
{
	MediaPlayer *mplayer;
	
	LOG_PLAYLIST ("PlaylistRoot::Stop ()\n");
	
	mplayer = GetMediaPlayer ();
	
	entry->Stop ();
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
	entry->Open ();
	Emit (StopEvent); // we emit the event after enqueuing the Open request, do avoid funky side-effects of event emission.
}

void
Playlist::EmitBufferUnderflowEvent (EventObject *obj)
{
	Playlist *root = (Playlist *) obj;
	root->Emit (BufferUnderflowEvent);
}

MediaPlayer *
Playlist::GetMediaPlayer ()
{
	return mplayer;
}

MediaElement *
Playlist::GetElement ()
{
	return element;
}

void
Playlist::MediaEndedHandler (MediaPlayer *mplayer, EventArgs *args)
{
	LOG_PLAYLIST ("Playlist::MediaEndedHandler (%p, %p)\n", mplayer, args);
	
	GetCurrentEntryLeaf ()->OnEntryEnded ();
}

void
Playlist::BufferUnderflowHandler (MediaPlayer *mplayer, EventArgs *args)
{
	LOG_PLAYLIST ("Playlist::BufferUnderflowHandler (%p, %p)\n", mplayer, args);
	
	if (Surface::InMainThread ()) {
		EmitBufferUnderflowEvent (this);
	} else {
		AddTickCall (EmitBufferUnderflowEvent);
	}
}

/*
 * PlaylistParser
 */

PlaylistParser::PlaylistParser (PlaylistEntry *parent, MemoryBuffer *source)
{
	this->parent = parent;
	this->source = source;
	this->kind_stack = NULL;
	this->current_entry = NULL;
	this->current_text = NULL;
	this->error_args = NULL;
	this->asxparser = new AsxParser ();
}

void
PlaylistParser::Setup (XmlType type)
{
	current_entry = NULL;
	current_text = NULL;

	kind_stack = new List ();
	PushCurrentKind (PlaylistKind::Root);

	if (type == XML_TYPE_ASX3) {
		asxparser->SetUserData (this);
		asxparser->SetElementStartHandler (on_start_element_internal_asxparser);
		asxparser->SetElementEndHandler (on_end_element_internal_asxparser);
		asxparser->SetTextHandler (on_text_internal_asxparser);
	}
}

PlaylistParser::~PlaylistParser ()
{
	if (kind_stack) {
		delete kind_stack;
		kind_stack = NULL;
	}
	delete asxparser;
	asxparser = NULL;
	if (error_args) {
		error_args->unref ();
		error_args = NULL;
	}
}

static bool
str_match (const char *candidate, const char *tag)
{
	return g_ascii_strcasecmp (candidate, tag) == 0;
}


void
PlaylistParser::on_start_element_internal_asxparser (AsxParser *parser, const char *name, GHashTable *atts)
{
#if PLUMB_ME
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
#endif
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

#if PLUMB_ME
	while (p <= end && g_ascii_isdigit (*p)) {
		res = res * 10 + *p - '0';
		p++;
	}
#endif

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

#if PLUMB_ME
	if (!g_ascii_isdigit (*p)) {
		parser->ParsingError (new ErrorEventArgs (MediaError, MoonError (MoonError::EXCEPTION, 2210, "AG_E_INVALID_ARGUMENT")));
		return false;
	}
#endif

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
	
#if PLUMB_ME
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
#endif
	
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
		Duration *dur = NULL;
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "VALUE")) {
				if (duration_from_asx_str (this, attrs [i+1], &dur)) {
					if (GetCurrentEntry () != NULL && GetParentKind () != PlaylistKind::Ref) {
						LOG_PLAYLIST ("PlaylistParser::OnStartElement (%s, %p), found VALUE/timespan = %f s\n", name, attrs, TimeSpan_ToSecondsFloat (dur->GetTimeSpan()));
						GetCurrentEntry ()->SetDuration (dur);
					}
					delete dur;
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
		PlaylistEntry *entry = new PlaylistEntry (parent->GetRoot (), GetCurrentContent ());
		entry->SetClientSkip (client_skip);
		GetCurrentContent ()->AddEntry (entry);
		current_entry = entry;
		entry->unref ();
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

		PlaylistEntry *entry = new PlaylistEntry (parent->GetRoot (), GetCurrentContent ());
		entry->SetIsEntryRef (true);
		if (uri)
			entry->SetSourceName (uri);
		uri = NULL;
		GetCurrentContent ()->AddEntry (entry);
		current_entry = entry;
		entry->unref ();
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
		Duration *dur = NULL;
		for (int i = 0; attrs [i] != NULL; i += 2) {
			if (str_match (attrs [i], "VALUE")) {
				if (duration_from_asx_str (this, attrs [i+1], &dur)) {
					if (dur->HasTimeSpan ()) {
						if (GetCurrentEntry () != NULL && GetParentKind () != PlaylistKind::Ref) {
							LOG_PLAYLIST ("PlaylistParser::OnStartElement (%s, %p), found VALUE/timespan = %f s\n", name, attrs, TimeSpan_ToSecondsFloat (dur->GetTimeSpan()));
							GetCurrentEntry ()->SetStartTime (dur->GetTimeSpan ());
						}
					}
					delete dur;
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
			GetCurrentContent ()->AddParams (name, value);
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
		delete dur;
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
	case PlaylistKind::EntryRef:
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
#if PLUMB_ME
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

	PlaylistEntry *entry = new PlaylistEntry (parent->GetRoot (), parent);
	uri = Uri::Create (mms_uri);
	if (uri != NULL) {
		entry->SetSourceName (uri);
	}
	parent->AddEntry (entry);
	current_entry = entry;
	entry->unref ();
#endif

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
		result = this->ParseASX3 ();
	} else {
		result = false;
	}

	return result ? MEDIA_SUCCESS : MEDIA_FAIL;
}

bool
PlaylistParser::ParseASX3 ()
{
	Setup (XML_TYPE_ASX3);

	bool result = asxparser->ParseBuffer (source);

	if (result)
		return true;

	switch (asxparser->GetErrorCode ()) {
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
				asxparser->GetErrorMessage (),
				asxparser->GetErrorCode (),
				asxparser->GetCurrentLineNumber (),
				asxparser->GetCurrentColumnNumber ());

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

	return parent;
}

PlaylistEntry *
PlaylistParser::GetCurrentEntry ()
{
	return current_entry;
}

void 
PlaylistParser::EndEntry ()
{
	current_entry = current_entry->GetParent ();
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

	asxparser->Stop ();

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
