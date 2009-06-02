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
		/* SMIL playlists */
		Smil		= 1 << 20,
		Switch		= 1 << 21,
		Media		= 1 << 22,
		Excl		= 1 << 23,
		Seq		= 1 << 24,
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
	Duration *repeat_duration;
	int repeat_count;
	char *role;

	PlaylistKind::Kind set_values;
	
	// Non ASX properties
	char *full_source_name;
	bool is_live;
	bool play_when_available;
	Playlist *parent;
	Media *media;

	void Init (Playlist *parent);

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
	void SetTitle (char *title);

	const char *GetAuthor ();
	void SetAuthor (char *author);

	const char *GetAbstract ();
	void SetAbstract (char *abstract);

	const char *GetCopyright ();
	void SetCopyright (char *copyright);

	Uri *GetSourceName ();
	void SetSourceName (Uri *source_name);

	TimeSpan GetStartTime ();
	void SetStartTime (TimeSpan start_time);

	Duration *GetDuration ();
	void SetDuration (Duration *duration);
	bool HasDuration () { return (set_values & PlaylistKind::Duration); }

	Duration *GetRepeatDuration ();
	void SetRepeatDuration (Duration *duration);
	// FIXME these are attributes from smil not nodes
	// bool HasRepeatDuration () { return (set_values & PlaylistNode::RepeatDuration); }

	int GetRepeatCount () { return repeat_count; }
	void SetRepeatCount (int count) { repeat_count = count; }
	// FIXME these are attributes from smil not nodes
	// bool HasRepeatCount () { return (set_values & PlaylistNode::RepeatDuration); }


	void SetRole (const char *value) { role = g_strdup (role); }
	char *GetRole (void) { return role; }

	const char *GetInfoTarget ();
	void SetInfoTarget (char *info_target);

	const char *GetInfoURL ();
	void SetInfoURL (char *info_url);

	bool GetClientSkip ();
	void SetClientSkip (bool value);

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

	virtual void OpenAsync ();
	virtual void PlayAsync ();
	virtual void PauseAsync ();
	virtual void StopAsync ();
	virtual void SeekAsync (guint64 pts);
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
	bool is_sequential;
	bool is_switch;
	bool waiting;
	bool opened;

	void Init ();

	bool HasMediaSource ();
	void OnMediaDownloaded ();

	void MergeWith (PlaylistEntry *entry);
	void PlayNext (bool fail);

protected:
	Playlist (Type::Kind kind);
	virtual ~Playlist () {}


public:
	Playlist (Playlist *parent, IMediaSource *source);
	
	virtual void Dispose ();

	virtual void OpenAsync ();
	virtual void PlayAsync ();
	virtual void PauseAsync ();
	virtual void StopAsync ();
	virtual void SeekAsync (guint64 to);
	virtual void PopulateMediaAttributes ();
	
	virtual void AddEntry (PlaylistEntry *entry);

	PlaylistEntry *GetCurrentEntry () { return current_node ? current_node->GetEntry () : NULL; }
	virtual PlaylistEntry *GetCurrentPlaylistEntry ();
	bool ReplaceCurrentEntry (Playlist *entry);

	virtual bool IsPlaylist () { return true; }
	virtual bool IsSingleFile () { return is_single_file; }
	void SetSequential (bool value) { is_sequential = value; }
	bool GetSequential (void) { return is_sequential; }
	void SetSwitch (bool value) { is_switch = value; }
	bool GetSwitch (void) { return is_switch; }
	void SetWaiting (bool value) { waiting = value; }
	bool GetWaiting (void) { return waiting; }

	bool IsCurrentEntryLastEntry ();
	void OnEntryEnded ();
	void OnEntryFailed ();

	void Print (int depth);
	
#if DEBUG
	virtual void DumpInternal (int tabs);
#endif
};

class PlaylistRoot : public Playlist {
private:
	MediaElement *element;
	MediaPlayer *mplayer;

	static void EmitStopEvent (EventObject *obj);
protected:
	virtual ~PlaylistRoot () {}
	
public:
	PlaylistRoot (MediaElement *element);
	virtual void Dispose (); // not thread-safe
	virtual MediaElement *GetElement ();
	
	virtual void StopAsync ();
	
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
	
	// Event handlers
	EVENTHANDLER (PlaylistRoot, MediaEnded, MediaPlayer, EventArgs);
	
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
	// For <ASX* files, this is 3 (or 0 if no version attribute was found).
	// for [Ref* files, this is 2.
	// The presence of a version does not guarantee that the playlist
	// was parsed correctly.
	int playlist_version;

	enum XmlType {
		XML_TYPE_NONE,
		XML_TYPE_ASX3,
		XML_TYPE_SMIL
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

	void GetSMILCommonAttrs (PlaylistEntry *entry, const char *name, const char **attrs);

	void OnSMILStartElement (const char *name, const char **attrs);
	void OnSMILEndElement (const char *name);

	static void on_asx_start_element (gpointer user_data, const char *name, const char **attrs);
	static void on_asx_end_element (gpointer user_data, const char *name);
	static void on_asx_text (gpointer user_data, const char *text, int len);

	static void on_smil_start_element (gpointer user_data, const char *name, const char **attrs);
	static void on_smil_end_element (gpointer user_data, const char *name);

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
	bool TryFixError (gint8 *buffer, int bytes_read);
public:

	PlaylistParser (PlaylistRoot *root, IMediaSource *source);
	~PlaylistParser ();

	Playlist *GetPlaylist () { return playlist; }

	MediaResult Parse ();
	bool ParseASX2 ();
	bool ParseASX3 ();
	bool ParseSMIL ();
	bool IsASX2 (IMediaSource *source);
	bool IsASX3 (IMediaSource *source);
	bool IsSMIL (IMediaSource *source);

	// This value determines if the data we parsed
	// actually was a playlist. It may be true even
	// if the playlist wasn't parsed correctly.
	bool WasPlaylist () { return was_playlist; }
	void ParsingError (ErrorEventArgs *args = NULL);

	static PlaylistKind::Kind StringToKind (const char *str);
	static const char *KindToString (PlaylistKind::Kind kind);
};

#endif /* __PLAYLIST_H__ */
