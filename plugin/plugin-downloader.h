/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-downloader.h: Plugin downloader
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __PLUGIN_DOWNLOADER_H__
#define __PLUGIN_DOWNLOADER_H__

class PluginDownloader;

#include "moonlight.h"
#include "runtime.h"
#include "downloader.h"
#include "plugin.h"
#include "plugin-class.h"


G_BEGIN_DECLS

guint32 plugin_downloader_started (DownloaderResponse *response, gpointer state);
guint32 plugin_downloader_available (DownloaderResponse *response, gpointer state, char *buffer, guint32 length);
guint32 plugin_downloader_finished (DownloaderResponse *response, gpointer state, gpointer data);

G_END_DECLS

class PluginDownloader {
 private:
	DownloaderResponse *response;
	DownloaderRequest *request;
	DownloaderResponseHeaderCallback response_header_callback;
	gpointer response_header_context;
	uint64_t offset;
	bool finished;
	bool aborted;
	
 protected:
	char *uri;
	char *verb;

 public:
	PluginDownloader (Downloader *dl);
	virtual ~PluginDownloader ();

	void Abort ();
	void Open (const char *verb, const char *uri, bool custom_header_support, bool disable_cache);
	void Send ();

	guint32 Read (char *buffer, guint32 length);
	void Started ();
	void Finished (bool success, gpointer data, const char *uri);

	void SetHttpHeader (const char *header, const char *value);
	void SetBody (void *body, guint32 length);
	
	void SetResponseHeaderCallback (DownloaderResponseHeaderCallback callback, gpointer context);

	PluginInstance *GetPlugin ();

	void setResponse (DownloaderResponse *response);
	DownloaderResponse *getResponse () { return response; }
	DownloaderRequest *getRequest ();
	bool IsAborted () { return aborted; }

	Downloader *dl;
};

void downloader_initialize (void);
void downloader_destroy (void);

#endif // __PLUGIN_DOWNLOADER_H__
