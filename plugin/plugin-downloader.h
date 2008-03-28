/*
 * plugin-downloader.h: Plugin downloader
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __PLUGIN_DOWNLOADER_H__
#define __PLUGIN_DOWNLOADER_H__

#include "moonlight.h"
#include "runtime.h"
#include "downloader.h"
#include "plugin.h"
#include "plugin-class.h"

class PluginDownloader {
public:
	PluginDownloader (Downloader *dl)
	{
		this->dl = dl;
		this->verb = NULL;
		this->uri = NULL;

		this->mmsh = false;

		// these are set after the stream is created, and used for destroying the stream
		this->npp = NULL;
		this->stream = NULL;

		ignore_non_data = false;

		// The Downloader will call destroy_state from it's destructor,
		// but if we ref the Downloader, its destructor will never get called.
		// No need to keep a ref, since this instance will never live longer
		// than its Downloader.
		// base_ref (dl);
	}

	~PluginDownloader ()
	{
		g_free (verb);
		verb = NULL;
		g_free (uri);
		uri = NULL;
		// base_unref (dl);
		dl = NULL;
	}
	void StreamDestroyed ()
	{
		stream = NULL;
	}
	
	Downloader *dl;
	char *uri;
	char *verb;
	bool mmsh;
	NPStream *stream;
	NPP npp;
	bool ignore_non_data;
	int header_size;
};

void downloader_set_stream_data (Downloader *downloader, NPP npp, NPStream *stream);
void downloader_initialize (void);
void downloader_destroy (void);

#endif // __PLUGIN_DOWNLOADER_H__
