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

G_BEGIN_DECLS

#include <expat.h>

#include "downloader.h"
#include "media.h"

class PlaylistContent {
private:
	char *base;
	char *title;
	char *author;
	char *abstract;
	char *copyright;
public:
	PlaylistContent ();
	~PlaylistContent ();

	const char *GetBase ();
	void SetBase (char *base);

	const char *GetTitle ();
	void SetTitle (char *title);

	const char *GetAuthor ();
	void SetAuthor (char *author);

	const char *GetAbstract ();
	void SetAbstract (char *abstract);

	const char *GetCopyright ();
	void SetCopyright (char *copyright);
};

class PlaylistEntry : public List::Node, public PlaylistContent {
private:
	char *source;
	gint64 start_time;
	gint64 duration;
public:
	PlaylistEntry ();
	~PlaylistEntry ();

	const char *GetSource ();
	void SetSource (char *file);

	gint64 GetStartTime ();
	void SetStartTime (gint64 start_time);

	gint64 GetDuration ();
	void SetDuration (gint64 duration);
};

class Playlist : public MediaSource, public PlaylistContent {
private:
	List *entries;
	Downloader *downloader;

	bool Parse ();
public:
	Playlist (MediaElement *element, char *source_name);
	virtual ~Playlist ();

	virtual bool Open ();
	virtual guint Play ();

	void AddEntry (PlaylistEntry *entry);

	static bool IsPlaylistFile (const char *file_name);
};

class PlaylistParser {
private:
	Playlist *playlist;
	PlaylistEntry *entry;
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

	void OnEntry ();
	void EndEntry ();
	PlaylistEntry *GetCurrentEntry ();

	PlaylistContent *GetCurrentContent ();

	void PushCurrentKind (PlaylistNodeKind kind);
	void PopCurrentKind ();
	PlaylistNodeKind GetCurrentKind ();
	PlaylistNodeKind GetParentKind ();
	void AssertParentKind (int kind);

public:
	PlaylistParser (Playlist *list);
	~PlaylistParser ();

	bool Parse (const char *text, int len);
};

G_END_DECLS

#endif /* __PLAYLIST_H__ */
