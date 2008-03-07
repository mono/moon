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

class PlaylistContent;
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
};

class PlaylistContent : public EventObject {
private:
	char *base;
	char *title;
	char *author;
	char *abstract;
	char *copyright;

protected:
	virtual ~PlaylistContent ();

public:
	PlaylistContent ();

	const char *GetBase () { return base; }
	const char *GetTitle () { return title; }
	const char *GetAuthor () { 	return author; }
	const char *GetAbstract () { return abstract; }
	const char *GetCopyright () { return copyright; }
	void SetBase (char *base) { this->base = base; }
	void SetTitle (char *title) { this->title = title; }
	void SetAuthor (char *author) { this->author = author; }
	void SetAbstract (char *abstract) { this->abstract = abstract; }
	void SetCopyright (char *copyright) { this->copyright = copyright; }
};

class PlaylistEntry : public PlaylistContent {
private:
	char *source_name;
	char *full_source_name;

	TimeSpan start_time;
	TimeSpan duration;
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

	const char *GetSourceName () { return source_name; }
	void SetSourceName (char *source_name) { this->source_name = source_name; }

	TimeSpan GetStartTime () { return start_time; }
	void SetStartTime (TimeSpan start_time) { this->start_time = start_time; }

	TimeSpan GetDuration () { return duration; }
	void SetDuration (TimeSpan duration) { this->duration = duration; }

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
	
};

class Playlist : public PlaylistEntry {
private:
	List *entries;
	PlaylistNode *current_node;
	MediaElement *element;
	IMediaSource *source;

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
	void ReplaceCurrentEntry (Playlist *entry);

	virtual bool IsPlaylist () { return true; }
};

class PlaylistParser {
private:
	Playlist *playlist;
	PlaylistEntry *current_entry;
	XML_Parser parser;
	IMediaSource *source;
	MediaElement *element;

	char *current_text;

	enum PlaylistNodeKind {
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
	};

	struct PlaylistKind {
		const char *str;
		PlaylistNodeKind kind;
		PlaylistKind (const char *str, PlaylistNodeKind kind)
		{
			this->str = str;
			this->kind = kind;
		}
	};

	class KindNode : public List::Node {
	public:
		PlaylistNodeKind kind;

		KindNode (PlaylistNodeKind kind)
		{
			this->kind = kind;
		}
	};

	static PlaylistParser::PlaylistKind playlist_kinds [];
	List *kind_stack;

	void OnStartElement (const char *name, const char **attrs);
	void OnEndElement (const char *name);
	void OnText (const char *text, int len);
	char *GetHrefAttribute (const char **attrs);

	static void on_start_element (gpointer user_data, const char *name, const char **attrs);
	static void on_end_element (gpointer user_data, const char *name);
	static void on_text (gpointer user_data, const char *text, int len);

	void OnEntry ();
	void EndEntry ();
	PlaylistEntry *GetCurrentEntry ();

	PlaylistContent *GetCurrentContent ();

	void PushCurrentKind (PlaylistNodeKind kind);
	void PopCurrentKind ();
	PlaylistNodeKind GetCurrentKind ();
	PlaylistNodeKind GetParentKind ();
	bool AssertParentKind (int kind);

	void ParsingError (ErrorEventArgs *args = NULL);
public:
	PlaylistParser (MediaElement *element, IMediaSource *source);
	~PlaylistParser ();

	Playlist *GetPlaylist () { return playlist; }

	bool Parse ();
	bool ParseASX2();
	bool IsASX2 (IMediaSource *source);
	bool IsPlaylistFile (IMediaSource *source);

	static PlaylistNodeKind StringToKind (const char *str);
	static const char *KindToString (PlaylistNodeKind kind);
};

#endif /* __PLAYLIST_H__ */
