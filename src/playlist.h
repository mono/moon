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
	char *title;
	char *author;
	char *abstract;
public:
	PlaylistContent ();
	~PlaylistContent ();

	const char *GetTitle ();
	void SetTitle (char *title);

	const char *GetAuthor ();
	void SetAuthor (char *author);

	const char *GetAbstract ();
	void SetAbstract (char *abstract);
};

class PlaylistEntry : public List::Node, public PlaylistContent {
private:
	char *source;
	gint64 start_time;
public:
	PlaylistEntry ();
	~PlaylistEntry ();

	const char *GetSource ();
	void SetSource (char *file);

	gint64 GetStartTime ();
	void SetStartTime (gint64 start_time);
};

class Playlist : public MediaSource, public PlaylistContent {
	List *items; // list of PlaylistEntries
public:
	Playlist (MediaElement *element, char *source_name);
	virtual ~Playlist ();

	virtual bool Open ();

	static bool IsPlaylistFile (const char *file_name);
private:
	bool Parse ();
};

class PlaylistParser {
private:
	Playlist *playlist;
	XML_Parser parser;
	char *base;

	static void on_start_element (gpointer user_data, const char *name, const char **attrs);
	static void on_end_element (gpointer user_data, const char *name);
	static void on_text (gpointer user_data, const char *text, int len);
public:
	PlaylistParser (Playlist *list);
	~PlaylistParser ();

	void Parse (const char *text, int len);
};

G_END_DECLS

#endif /* __PLAYLIST_H__ */
