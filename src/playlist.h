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

#include <expat.h>

#include "downloader.h"
#include "media.h"
#include "pipeline.h"

class PlaylistContent {
private:
	char *base;
	char *title;
	char *author;
	char *abstract;
	char *copyright;
public:
	PlaylistContent () : base (NULL), title (NULL), author (NULL), abstract (NULL), copyright (NULL)
	{
	}

	~PlaylistContent ()
	{
		g_free (base);
		g_free (title);
		g_free (author);
		g_free (abstract);
	}

	const char *GetBase ()
	{
		return base;
	}

	void SetBase (char *base)
	{
		this->base = base;
	}

	const char *GetTitle ()
	{
		return title;
	}

	void SetTitle (char *title)
	{
		this->title = title;
	}

	const char *GetAuthor ()
	{
		return author;
	}

	void SetAuthor (char *author)
	{
		this->author = author;
	}

	const char *GetAbstract ()
	{
		return abstract;
	}

	void SetAbstract (char *abstract)
	{
		this->abstract = abstract;
	}

	const char *GetCopyright ()
	{
		return copyright;
	}

	void SetCopyright (char *copyright)
	{
		this->copyright = copyright;
	}
};

class PlaylistEntry : public List::Node, public PlaylistContent {
private:
	char *source_name;
	gint64 start_time;
	gint64 duration;
	MediaSource *source;
	bool play_when_available;
public:
	PlaylistEntry () : source_name (NULL), start_time (0), duration (0), source (NULL), play_when_available (true)
	{
	}

	~PlaylistEntry ()
	{
		delete source;
		g_free (source_name);
	}

	const char *GetSourceName ()
	{
		return source_name;
	}

	void SetSourceName (char *source_name)
	{
		this->source_name = source_name;
	}

	gint64 GetStartTime ()
	{
		return start_time;
	}

	void SetStartTime (gint64 start_time)
	{
		this->start_time = start_time;
	}

	gint64 GetDuration ()
	{
		return duration;
	}

	void SetDuration (gint64 duration)
	{
		this->duration = duration;
	}

	MediaSource *GetSource ()
	{
		return source;
	}

	void SetSource (MediaSource *source)
	{
		this->source = source;
	}

	bool PlayWhenAvailable ()
	{
		return play_when_available;
	}

	void PlayWhenAvailable (bool play_when_available)
	{
		this->play_when_available = play_when_available;
	}
};

class Playlist : public MediaSource, public PlaylistContent {
private:
	List *entries;
	PlaylistEntry *current_entry;
	Downloader *downloader;

	bool Parse ();
	bool HasMediaSource ();
	bool OpenEntry (PlaylistEntry *entry);
	bool OpenCurrentSource ();
	void OnMediaEnded ();
	void OnMediaDownloaded ();
	FileSource *GetFileSource () { return (FileSource*) GetSource (); }

	void PopulateMediaAttributes ();

	static void on_media_ended (EventObject *sender, EventArgs *calldata, gpointer userdata);
	static void on_downloader_complete (EventObject *sender, EventArgs *calldata, gpointer userdata);
	static void on_downloader_data_write (void *buf, int32_t offset, int32_t n, gpointer data);
	static void on_downloader_size_notify (int64_t size, gpointer data);
	
protected:
	virtual bool OpenInternal ();
	
public:
	Playlist (MediaElement *element, const char *source_name, FileSource *source);
	virtual ~Playlist ();

	virtual void Play ();
	virtual void Pause ();
	virtual void Stop (bool media_ended);
	virtual void Close ();

	void AddEntry (PlaylistEntry *entry);

	static bool IsPlaylistFile (IMediaSource *source);
};

class PlaylistParser {
private:
	Playlist *playlist;
	PlaylistEntry *current_entry;
	XML_Parser parser;

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

	class KindNode : public List::Node {
	public:
		PlaylistNodeKind kind;

		KindNode (PlaylistNodeKind kind)
		{
			this->kind = kind;
		}
	};

	List *kind_stack;

	static void on_start_element (gpointer user_data, const char *name, const char **attrs);
	static void on_end_element (gpointer user_data, const char *name);
	static void on_text (gpointer user_data, const char *text, int len);
	static char *get_href_attribute (gpointer user_data, const char **attrs);

	void OnEntry ();
	void EndEntry ();
	PlaylistEntry *GetCurrentEntry ();

	PlaylistContent *GetCurrentContent ();

	void PushCurrentKind (PlaylistNodeKind kind);
	void PopCurrentKind ();
	PlaylistNodeKind GetCurrentKind ();
	PlaylistNodeKind GetParentKind ();
	void AssertParentKind (int kind);

	void ParsingError ();
public:
	PlaylistParser (Playlist *list);
	~PlaylistParser ();

	bool Parse (const char *text, int len);
};

#endif /* __PLAYLIST_H__ */
