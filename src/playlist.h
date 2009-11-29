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

class PlaylistEntry;
class Playlist;
class PlaylistRoot;

#include <expat.h>

#include "value.h"
#include "error.h"
#include "dependencyobject.h"
#include "uri.h"
#include "pipeline.h"

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

class PlaylistNode : public List::Node {
private:
	PlaylistEntry *entry;

public:
	PlaylistNode (PlaylistEntry *entry);
	virtual ~PlaylistNode ();
	PlaylistEntry *GetEntry () { return entry; }
};

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
	TimeSpan start_time;
	Duration *duration;
	GHashTable *params;

	PlaylistKind::Kind set_values;
	
	// Non ASX properties
	char *full_source_name;
	bool is_live;
	bool play_when_available;
	Playlist *parent;
	Media *media;
	bool opened; // if OpenCompleted event has been received

	void Init (Playlist *parent);
	void OpenMediaPlayer ();
	
protected:
	PlaylistEntry (Type::Kind kind);
	PlaylistEntry (Type::Kind kind, Playlist *parent);
	virtual ~PlaylistEntry () {}

public:
	PlaylistEntry (Playlist *parent);

	void Initialize (Media *media);
	void InitializeWithUri (const char *uri);
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

	void AddParams (const char *name, const char *value);
	
	// non-ASX properties

	Playlist *GetParent () { return parent; }
	void SetParent (Playlist *value) { parent = value; }
	PlaylistRoot *GetRoot ();
	
	Media *GetMedia ();
	void ClearMedia ();

	virtual MediaElement *GetElement ();
	MediaPlayer *GetMediaPlayer ();

	const char *GetFullSourceName ();
	virtual bool IsPlaylist () { return false; }

	bool GetIsLive () { return is_live; }
	void SetIsLive (bool value) { is_live = value; }

	// Playback methods

	virtual void Play ();
	virtual void Pause ();
	virtual void Stop ();
	virtual void Seek (guint64 pts);
	virtual void Open ();
	virtual void PopulateMediaAttributes ();
	
	virtual PlaylistEntry *GetCurrentPlaylistEntry () { return this; }
	virtual bool IsSingleFile ();

	void Print (int depth);
	
	EVENTHANDLER (PlaylistEntry, Opening,             Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, OpenCompleted,       Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, Seeking,             Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, SeekCompleted,       Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, CurrentStateChanged, Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, MediaError,          Media, ErrorEventArgs);
	EVENTHANDLER (PlaylistEntry, DownloadProgressChanged, Media, EventArgs);
	EVENTHANDLER (PlaylistEntry, BufferingProgressChanged, Media, EventArgs);
	
#if DEBUG
	virtual void DumpInternal (int tabs);
#endif
};

class Playlist : public PlaylistEntry {
private:
	List *entries;
	PlaylistNode *current_node;
	IMediaSource *source;
	bool is_single_file;
	bool waiting;
	bool opened;

	void Init ();

	bool HasMediaSource ();
	void OnMediaDownloaded ();

	void MergeWith (PlaylistEntry *entry);
	bool PlayNext (); // returns false if nothing more to play

protected:
	Playlist (Type::Kind kind);
	virtual ~Playlist () {}


public:
	Playlist (Playlist *parent, IMediaSource *source);
	
	virtual void Dispose ();

	virtual void Play ();
	virtual void Pause ();
	virtual void Stop ();
	virtual void Seek (guint64 to);
	virtual void Open ();
	virtual void PopulateMediaAttributes ();
	
	virtual void AddEntry (PlaylistEntry *entry);

	PlaylistEntry *GetCurrentEntry () { return current_node ? current_node->GetEntry () : NULL; }
	virtual PlaylistEntry *GetCurrentPlaylistEntry ();
	bool ReplaceCurrentEntry (Playlist *entry);

	virtual bool IsPlaylist () { return true; }
	virtual bool IsSingleFile () { return is_single_file; }
	void SetWaiting (bool value) { waiting = value; }
	bool GetWaiting (void) { return waiting; }

	bool IsCurrentEntryLastEntry ();
	void OnEntryEnded ();
	void OnEntryFailed (ErrorEventArgs *args);

	void Print (int depth);
	
	gint32 GetCount () { return entries ? entries->Length () : 0; }

#if DEBUG
	virtual void DumpInternal (int tabs);
#endif
};

/* @Namespace=None,ManagedEvents=Manual */
class PlaylistRoot : public Playlist {
private:
	class PtsNode : public List::Node {
	public:
		guint64 pts;
		PtsNode (guint64 pts)
		{
			this->pts = pts;
		}
	};
	MediaElement *element;
	MediaPlayer *mplayer;

	List seeks; // the pts to seek to when SeekCallback is called. Main thread only.

	static void EmitBufferUnderflowEvent (EventObject *obj);
	static void StopCallback (EventObject *obj);
	static void PlayCallback (EventObject *obj);
	static void PauseCallback (EventObject *obj);
	static void OpenCallback (EventObject *obj);
	static void SeekCallback (EventObject *obj);
	
protected:
	virtual ~PlaylistRoot () {}
	
	virtual void Stop ();
	
public:
	PlaylistRoot (MediaElement *element);
	virtual void Dispose (); // not thread-safe
	virtual MediaElement *GetElement ();
	
	void StopAsync ();
	void OpenAsync ();
	void PlayAsync ();
	void PauseAsync ();
	void SeekAsync (guint64 pts);
	
	virtual bool IsSingleFile ();
	
	Media *GetCurrentMedia ();
	MediaPlayer *GetMediaPlayer ();
	
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
	EVENTHANDLER (PlaylistRoot, MediaEnded, MediaPlayer, EventArgs);
	EVENTHANDLER (PlaylistRoot, BufferUnderflow, MediaPlayer, EventArgs);
	
#if DEBUG
	void Dump ();
#endif
};

class PlaylistParserInternal {
public:
	XML_Parser parser;
	gint32 bytes_read;
	bool reparse;

	PlaylistParserInternal ();
	~PlaylistParserInternal ();
};

class PlaylistParser {
private:
	PlaylistRoot *root;
	Playlist *playlist;
	PlaylistEntry *current_entry;
	PlaylistParserInternal *internal;
	IMediaSource *source;
	bool was_playlist;
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

	void EndEntry ();
	PlaylistEntry *GetCurrentEntry ();

	PlaylistEntry *GetCurrentContent ();

	void PushCurrentKind (PlaylistKind::Kind kind);
	void PopCurrentKind ();
	PlaylistKind::Kind GetCurrentKind ();
	PlaylistKind::Kind GetParentKind ();
	bool AssertParentKind (int kind);

	void Setup (XmlType type);
	void Cleanup ();
	void SetSource (IMediaSource *source);
	bool TryFixError (gint8 *buffer, int bytes_read, int total_bytes_read);
public:

	PlaylistParser (PlaylistRoot *root, IMediaSource *source);
	~PlaylistParser ();

	Playlist *GetPlaylist () { return playlist; }

	MediaResult Parse ();
	bool ParseASX2 ();
	bool ParseASX3 ();
	static bool Is (IMediaSource *source, const char *header);
	static bool IsASX2 (IMediaSource *source);
	static bool IsASX3 (IMediaSource *source);

	// This value determines if the data we parsed
	// actually was a playlist. It may be true even
	// if the playlist wasn't parsed correctly.
	bool WasPlaylist () { return was_playlist; }
	void ParsingError (ErrorEventArgs *args = NULL);
	
	ErrorEventArgs *GetErrorEventArgs () { return error_args; }

	static PlaylistKind::Kind StringToKind (const char *str);
	static const char *KindToString (PlaylistKind::Kind kind);
};

#endif /* __PLAYLIST_H__ */
