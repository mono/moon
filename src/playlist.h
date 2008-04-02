/*
 * playlist.h:
 *
 * Author: Jb Evain <jbevain@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

class PlaylistEntry;
class Playlist;
class MediaSource;
class SingleMedia;

#include <expat.h>

#include "downloader.h"
#include "media.h"
#include "pipeline.h"
#include "error.h"
#include "dependencyobject.h"

class PlaylistNode : public List::Node {
private:
	PlaylistEntry *entry;

public:
	PlaylistNode (PlaylistEntry *entry);
	virtual ~PlaylistNode ();
	PlaylistEntry *GetEntry () { return entry; }

	enum Kind {
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
};

class PlaylistEntry : public EventObject {
private:
	// ASX Properties 
	char *base;
	char *title;
	char *author;
	char *abstract;
	char *copyright;
	char *source_name;
	char *info_target;
	char *info_url;
	TimeSpan start_time;
	TimeSpan duration;

	PlaylistNode::Kind set_values;
	
	// Non ASX properties
	char *full_source_name;
	bool play_when_available;
	Playlist *parent;
	MediaElement *element;
	Media *media;

	static MediaResult playlist_entry_open_callback (MediaClosure *closure);
	void Init (Playlist *parent);

protected:
	virtual ~PlaylistEntry ();

public:
	PlaylistEntry (MediaElement *element, Playlist *parent, Media *media = NULL);

	// ASX properties

	const char *GetBase ();
	const char *GetBaseInherited ();
	void SetBase (char *base);

	const char *GetTitle ();
	void SetTitle (char *title);

	const char *GetAuthor ();
	void SetAuthor (char *author);

	const char *GetAbstract ();
	void SetAbstract (char *abstract);

	const char *GetCopyright ();
	void SetCopyright (char *copyright);

	const char *GetSourceName ();
	void SetSourceName (char *source_name);

	TimeSpan GetStartTime ();
	void SetStartTime (TimeSpan start_time);

	TimeSpan GetDuration ();
	void SetDuration (TimeSpan duration);

	const char *GetInfoTarget ();
	void SetInfoTarget (char *info_target);

	const char *GetInfoURL ();
	void SetInfoURL (char *info_url);

	// non-ASX properties

	Playlist *GetParent () { return parent; }

	MediaElement *GetElement () { return element; }
	void SetElement (MediaElement *element) { this->element = element; }

	Media *GetMedia ();
	void SetMedia (Media *media);

	const char *GetFullSourceName ();
	virtual bool IsPlaylist () { return false; }

	// Playback methods

	virtual void Open ();
	virtual bool Play ();
	virtual bool Pause ();
	virtual void Stop ();
	virtual void PopulateMediaAttributes ();
	
	virtual PlaylistEntry *GetCurrentPlaylistEntry () { return this; }
};

class Playlist : public PlaylistEntry {
private:
	List *entries;
	PlaylistNode *current_node;
	MediaElement *element;
	IMediaSource *source;
	bool is_single_file;

	void Init (MediaElement *element);

	bool HasMediaSource ();
	void OnMediaEnded ();
	void OnMediaDownloaded ();

	static void on_media_ended (EventObject *sender, EventArgs *calldata, gpointer userdata);
	void MergeWith (PlaylistEntry *entry);

protected:
	virtual ~Playlist ();

public:
	Playlist (MediaElement *element, IMediaSource *source);
	Playlist (MediaElement *element, Media *media);

	virtual void Open ();
	virtual bool Play ();
	virtual bool Pause ();
	virtual void Stop ();
	virtual void PopulateMediaAttributes ();
	
	virtual void AddEntry (PlaylistEntry *entry);

	virtual MediaElement *GetElement () { return element; }
	PlaylistEntry *GetCurrentEntry () { return current_node ? current_node->GetEntry () : NULL; }
	virtual PlaylistEntry *GetCurrentPlaylistEntry () { return current_node ? current_node->GetEntry ()->GetCurrentPlaylistEntry () : NULL; }
	bool ReplaceCurrentEntry (Playlist *entry);

	virtual bool IsPlaylist () { return true; }
	bool IsSingleFile () { return is_single_file; }
};

class PlaylistParser {
private:
	Playlist *playlist;
	PlaylistEntry *current_entry;
	XML_Parser parser;
	IMediaSource *source;
	MediaElement *element;
	bool was_playlist;
	// For <ASX* files, this is 3 (or 0 if no version attribute was found).
	// for [Ref* files, this is 2.
	// The presence of a version does not guarantee that the playlist
	// was parsed correctly.
	int playlist_version;

	char *current_text;

	struct PlaylistKind {
		const char *str;
		PlaylistNode::Kind kind;
		PlaylistKind (const char *str, PlaylistNode::Kind kind)
		{
			this->str = str;
			this->kind = kind;
		}
	};

	class KindNode : public List::Node {
	public:
		PlaylistNode::Kind kind;

		KindNode (PlaylistNode::Kind kind)
		{
			this->kind = kind;
		}
	};

	static PlaylistParser::PlaylistKind playlist_kinds [];
	List *kind_stack;

	void OnStartElement (const char *name, const char **attrs);
	void OnEndElement (const char *name);
	void OnText (const char *text, int len);

	static void on_start_element (gpointer user_data, const char *name, const char **attrs);
	static void on_end_element (gpointer user_data, const char *name);
	static void on_text (gpointer user_data, const char *text, int len);

	void EndEntry ();
	PlaylistEntry *GetCurrentEntry ();

	PlaylistEntry *GetCurrentContent ();

	void PushCurrentKind (PlaylistNode::Kind kind);
	void PopCurrentKind ();
	PlaylistNode::Kind GetCurrentKind ();
	PlaylistNode::Kind GetParentKind ();
	bool AssertParentKind (int kind);

public:

	PlaylistParser (MediaElement *element, IMediaSource *source);
	~PlaylistParser ();

	Playlist *GetPlaylist () { return playlist; }

	bool Parse ();
	bool ParseASX2 ();
	bool ParseASX3 ();
	bool IsASX2 (IMediaSource *source);
	bool IsASX3 (IMediaSource *source);

	// This value determines if the data we parsed
	// actually was a playlist. It may be true even
	// if the playlist wasn't parsed correctly.
	bool WasPlaylist () { return was_playlist; }
	void ParsingError (ErrorEventArgs *args = NULL);

	static PlaylistNode::Kind StringToKind (const char *str);
	static const char *KindToString (PlaylistNode::Kind kind);
};

#endif /* __PLAYLIST_H__ */
