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

uint32_t plugin_downloader_started (DownloaderResponse *response, gpointer state);
uint32_t plugin_downloader_available (DownloaderResponse *response, gpointer state, char *buffer, uint32_t length);
uint32_t plugin_downloader_finished (DownloaderResponse *response, gpointer state, gpointer data);

G_END_DECLS

class PluginDownloader {
 private:
	DownloaderResponse *response;
	DownloaderRequest *request;
	uint64_t offset;
	bool finished;
	
 protected:
	char *uri;
	char *verb;

 public:
	PluginDownloader (Downloader *dl);
	virtual ~PluginDownloader ();

	void Abort ();
	void Open (const char *verb, const char *uri, bool streaming);
	void Send ();

	uint32_t Read (char *buffer, uint32_t length);
	void Started ();
	void Finished (bool success, gpointer data);

	void SetHttpHeader (const char *header, const char *value);
	void SetBody (void *body, uint32_t length);
	
	PluginInstance *GetPlugin ();

	void setResponse (DownloaderResponse *response) { this->response = response; }
	DownloaderRequest *getRequest () { return this->request; }
	
	Downloader *dl;
};

void npstream_request_set_stream_data (Downloader *downloader, NPP npp, NPStream *stream);
void downloader_initialize (void);
void downloader_destroy (void);

#endif // __PLUGIN_DOWNLOADER_H__
