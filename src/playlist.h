/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * playlist.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

#include "value.h"
#include "error.h"
#include "dependencyobject.h"
#include "uri.h"
#include "pipeline.h"
#include "asxparser.h"

namespace Moonlight {

class PlaylistKind {
public:
	enum Kind {
		/* ASX3 playlists */
		Unknown		= 0,
		Root		= 1 << 0,
		Abstract	= 1 << 1,
		Asx			= 1 << 2,
		Author		= 1 << 3,
		Banner		= 1 << 4,
		Base		= 1 << 5,
		Copyright	= 1 << 6,
		Duration	= 1 << 7,
		Entry		= 1 << 8,
		EntryRef	= 1 << 9,
		LogUrl		= 1 << 10,
		MoreInfo	= 1 << 11,
		Ref			= 1 << 12,
		StartTime	= 1 << 13,
		Title		= 1 << 14,
		StartMarker	= 1 << 15,
		Repeat		= 1 << 16,
		EndMarker	= 1 << 17,
		Param		= 1 << 18,
		Event		= 1 << 19,
	};
	
public:
	const char *str;
	Kind kind;
	PlaylistKind (const char *str, Kind kind)
	{
		this->str = str;
		this->kind = kind;
	}
};

/*
 * PlaylistNode
 */
class PlaylistNode : public EventObjectNode<PlaylistEntry> {
public:
	PlaylistNode (PlaylistEntry *entry) : EventObjectNode<PlaylistEntry> (entry) {}
};

/*
 * PlaylistEntry
 */

class PlaylistEntry : public EventObject {
private:
	// ASX Properties 
	Uri *base;
	char *title;
	char *author;
	char *abstract;
	char *copyright;
	Uri *source_name;
	char *info_target;
	char *info_url;
	bool client_skip;
	bool is_entry_ref;
	TimeSpan start_time;
	Duration *duration;
	GHashTable *params;

	PlaylistKind::Kind set_values;
	
	// Non ASX properties
	Uri *full_source_name;
	bool is_live;
	bool play_when_available;
	PlaylistEntry *parent;
	Playlist *root;
	Media *media;
	bool opened; // if OpenCompleted event has been received

	List entries; // list of child elements
	PlaylistNode *current_node; // current node

	bool dynamic; /* If the playlist can change during playback. This is true for mms/server-side playlists. */
	bool dynamic_ended; /* If a dynamic playlist has ended transmission (it might still be playing due to buffers, etc). Note that a dynamic playlist which has ended can't modify the playlist anymore. */
	bool dynamic_waiting; /* if we finished playing what we have in the playlist, and are waiting for more playlist entries */

	void OpenMediaPlayer ();

	void EmitMediaEnded ();
	bool HasMediaSource ();
	void OnMediaDownloaded ();
	static void SetHasDynamicEndedCallback (EventObject *obj);

protected:
	/* @SkipFactories */
	PlaylistEntry ();
	virtual ~PlaylistEntry () {}

public:
	/* @SkipFactories */
	PlaylistEntry (Playlist *root, PlaylistEntry *parent);

	void Initialize (Media *media);
	void InitializeWithUri (const Uri *resource_base, const Uri *uri);
	void InitializeWithDownloader (Downloader *dl, const char *PartName);
	void InitializeWithDemuxer (IMediaDemuxer *demuxer);
	void InitializeWithStream (ManagedStreamCallbacks *callbacks);
	void InitializeWithSource (IMediaSource *source);
	
	Media * CreateMedia ();
	
	virtual void Dispose ();
	
	// ASX properties

	Uri *GetBase ();
	Uri *GetBaseInherited ();
	void SetBase (Uri *base);

	const char *GetTitle ();
	void SetTitle (const char *title);

	const char *GetAuthor ();
	void SetAuthor (const char *author);

	const char *GetAbstract ();
	void SetAbstract (const char *abstract);

	const char *GetCopyright ();
	void SetCopyright (const char *copyright);

	Uri *GetSourceName ();
	void SetSourceName (Uri *source_name);

	TimeSpan GetStartTime ();
	void SetStartTime (TimeSpan start_time);
	bool HasStartTime () { return (set_values & PlaylistKind::StartTime); }

	Duration *GetDuration ();
	Duration *GetInheritedDuration ();
	void SetDuration (Duration *duration);
	bool HasDuration () { return (set_values & PlaylistKind::Duration); }
	bool HasInheritedDuration ();
	
	const char *GetInfoTarget ();
	void SetInfoTarget (const char *info_target);

	const char *GetInfoURL ();
	void SetInfoURL (const char *info_url);

	bool GetClientSkip ();
	void SetClientSkip (bool value);

	bool GetIsEntryRef () { return is_entry_ref; }
	void SetIsEntryRef (bool value) { is_entry_ref = value; }

	void AddParams (const char *name, const char *value);
	
	// non-ASX properties

	PlaylistEntry *GetParent () { return parent; }
	Playlist *GetRoot ();
	
	Media *GetMedia ();
	void ClearMedia ();

	virtual MediaElement *GetElement ();
	MediaPlayer *GetMediaPlayer ();

	const Uri *GetFullSourceName ();
	const Uri *GetLocation ();

	bool IsPlaylist () { return entries.Length () > 0; }

	bool GetIsLive () { return is_live; }
	void SetIsLive (bool value) { is_live = value; }

	// Playback methods

	void Play ();
	void Pause ();
	void Stop ();
	void Seek (guint64 pts);
	void Open ();
	void PopulateMediaAttributes ();
	
	EVENTHANDLER (PlaylistEntry, Opening,             Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, OpenCompleted,       Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, Seeking,             Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, SeekCompleted,       Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, CurrentStateChanged, Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, MediaError,          Media, ErrorEventArgs);
	EVENTHANDLER (PlaylistEntry, DownloadProgressChanged, Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, BufferingProgressChanged, Media, EventArgs);
	void AddEntry (PlaylistEntry *entry);

	bool PlayNext (); // returns false if nothing more to play

	/* non recursive */
	PlaylistEntry *GetCurrentEntry () { return current_node ? current_node->GetElement () : NULL; }
	/* recursive */
	PlaylistEntry *GetCurrentEntryLeaf ();
	PlaylistNode *GetFirstNode () { return (PlaylistNode *) entries.First (); }

	void OnEntryEnded ();
	void OnEntryFailed (ErrorEventArgs *args);

	gint32 GetCount () { return entries.Length (); }

	void SetIsDynamicWaiting (bool value) { dynamic_waiting = value; }
	bool GetIsDynamicWaiting () { return dynamic_waiting; } // TODO: this should be recursive
	/* This method should only be called during opening, not after playback has started
	 * - in which case it's thread-safe. (write from media thread during opening, read
	 * from main thread after media has been opened) */
	void SetIsDynamic () { dynamic = true; }
	bool GetIsDynamic () { return dynamic; } // TODO: this should be recursive
	/* Thread-safe (calls are marshalled to the main thread) */
	void SetHasDynamicEnded ();

	bool IsASXDemuxer ();
	bool IsCurrent ();

#if DEBUG
	void Dump (int tabs, bool is_current, GString *fmt, bool html);
#endif
};

/* @Namespace=None,ManagedEvents=Manual */
class Playlist : public EventObject {
private:
	MediaElement *element;
	MediaPlayer *mplayer;
	PlaylistEntry *entry;
	List seeks; // the pts to seek to when SeekCallback is called. Main thread only.

	static void EmitBufferUnderflowEvent (EventObject *obj);
	static void StopCallback (EventObject *obj);
	static void PlayCallback (EventObject *obj);
	static void PauseCallback (EventObject *obj);
	static void OpenCallback (EventObject *obj);
	static void SeekCallback (EventObject *obj);

	void Stop ();

public:
	/* @SkipFactories */
	Playlist (MediaElement *element);
	virtual void Dispose (); // not thread-safe

	MediaElement *GetElement ();

	void StopAsync ();
	void OpenAsync ();
	void PlayAsync ();
	void PauseAsync ();
	void SeekAsync (guint64 pts);
	
	bool IsSingleFile ();
	
	MediaPlayer *GetMediaPlayer ();

	/* recursive */
	PlaylistEntry *GetCurrentEntryLeaf () { return entry->GetCurrentEntryLeaf (); }
	PlaylistEntry *GetFirstEntry () { return entry; }

	// Events
	const static int OpeningEvent;
	const static int OpenCompletedEvent;
	const static int SeekingEvent;
	const static int SeekCompletedEvent;
	const static int CurrentStateChangedEvent;
	const static int PlayEvent;
	const static int PauseEvent;
	const static int StopEvent;
	const static int MediaErrorEvent;
	const static int MediaEndedEvent;
	const static int DownloadProgressChangedEvent;
	const static int BufferingProgressChangedEvent;
	const static int BufferUnderflowEvent;
	const static int EntryChangedEvent;
	
	// Event handlers
	EVENTHANDLER (Playlist, MediaEnded, MediaPlayer, EventArgs);
	EVENTHANDLER (Playlist, BufferUnderflow, MediaPlayer, EventArgs);

	bool Emit (int event_id, EventArgs *calldata = NULL);

#if DEBUG
	void Dump ();
	void Dump (GString *fmt, bool html);
#endif
};

/*
 * PlaylistParser
 */
class PlaylistParser {
private:
	AsxParser *asxparser;
	PlaylistEntry *parent;
	PlaylistEntry *current_entry;
	MemoryBuffer *source;
	ErrorEventArgs *error_args;
	// For <ASX* files, this is 3 (or 0 if no version attribute was found).
	// for [Ref* files, this is 2.
	// The presence of a version does not guarantee that the playlist
	// was parsed correctly.
	int playlist_version;

	enum XmlType {
		XML_TYPE_NONE,
		XML_TYPE_ASX3,
	};

	char *current_text;

	class KindNode : public List::Node {
	public:
		PlaylistKind::Kind kind;

		KindNode (PlaylistKind::Kind kind)
		{
			this->kind = kind;
		}
	};

	static PlaylistKind playlist_kinds [];
	List *kind_stack;

	void OnASXStartElement (const char *name, const char **attrs);
	void OnASXEndElement (const char *name);
	void OnASXText (const char *text, int len);

	static void on_asx_start_element (gpointer user_data, const char *name, const char **attrs);
	static void on_asx_end_element (gpointer user_data, const char *name);
	static void on_asx_text (gpointer user_data, const char *text, int len);

	static void on_start_element_internal_asxparser (AsxParser *parser, const char *name, GHashTable *atts);
	static void on_end_element_internal_asxparser (AsxParser *parser, const char *name);
	static void on_text_internal_asxparser (AsxParser *parser, const char *name);

	void EndEntry ();
	PlaylistEntry *GetCurrentEntry ();

	PlaylistEntry *GetCurrentContent ();

	void PushCurrentKind (PlaylistKind::Kind kind);
	void PopCurrentKind ();
	PlaylistKind::Kind GetCurrentKind ();
	PlaylistKind::Kind GetParentKind ();
	bool AssertParentKind (int kind);

	void Setup (XmlType type);

public:
	PlaylistParser (PlaylistEntry *parent, MemoryBuffer *source);
	~PlaylistParser ();

	MediaResult Parse ();
	bool ParseASX2 ();
	bool ParseASX3 ();
	static bool Is (MemoryBuffer *source, const char *header);
	static bool IsASX2 (MemoryBuffer *source);
	static bool IsASX3 (MemoryBuffer *source);

	void ParsingError (ErrorEventArgs *args = NULL);
	
	ErrorEventArgs *GetErrorEventArgs () { return error_args; }

	static PlaylistKind::Kind StringToKind (const char *str);
	static const char *KindToString (PlaylistKind::Kind kind);
};

};
#endif /* __PLAYLIST_H__ */
