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

#include "downloader.h"
#include "media.h"

class Playlist : public MediaSource {
	List *items;
public:
	Playlist (MediaElement *element, char *source_name);
	virtual ~Playlist ();
	
	virtual bool Open ();

	static bool IsPlaylistFile (const char *file_name);
private:
	bool Parse ();

	static void parser_on_start_element (GMarkupParseContext *context, const gchar *element_name,
										  const gchar **attributes_names, const gchar **attributes_values,
										  gpointer user_data, GError **error);

	static void parser_on_end_element (GMarkupParseContext *context, const gchar *element_name,
										gpointer user_data, GError **error);

	static void parser_on_text (GMarkupParseContext *context, const gchar *text, gsize text_len,
								 gpointer user_data, GError **error);

	static void parser_on_error (GMarkupParseContext *context, GError *error, gpointer user_data);
};

G_END_DECLS

#endif /* __PLAYLIST_H__ */
