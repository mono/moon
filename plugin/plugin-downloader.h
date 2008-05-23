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

class PluginDownloader;

#include "moonlight.h"
#include "runtime.h"
#include "downloader.h"
#include "plugin.h"
#include "plugin-class.h"

class BrowserResponse;

G_BEGIN_DECLS

uint32_t browser_downloader_started (BrowserResponse *response, gpointer state);
uint32_t browser_downloader_available (BrowserResponse *response, gpointer state, char *buffer, uint32_t length);
uint32_t browser_downloader_finished (BrowserResponse *response, gpointer state);

G_END_DECLS

class BrowserDownloader {
 protected:
	PluginDownloader *pd;
	BrowserResponse *response;

 public:
	BrowserDownloader (PluginDownloader *pd)
	{
		this->pd = pd;
		this->response = NULL;
	}

	~BrowserDownloader ()
	{
		this->pd = NULL;
	}

	virtual void Abort () = 0;
	virtual uint32_t Read (char *buffer, uint32_t length) = 0;
	virtual void Send () = 0;
	virtual void Started () = 0;
	virtual void Finished () = 0;

	PluginInstance *GetPlugin ();
	void SetResponse (BrowserResponse *response) { this->response = response; }
};

class PluginDownloader {
 private:
	BrowserDownloader *bdl;

 protected:
	char *uri;
	char *verb;

 public:
	PluginDownloader (Downloader *dl)
	{
		this->dl = dl;
		this->uri = NULL;
		this->verb = NULL;
	}

	~PluginDownloader ()
	{
		g_free (verb);
		g_free (uri);
		dl = NULL;
	}

	void Open (const char *verb, const char *uri);

	void Abort () { bdl->Abort(); }
	void Send () { bdl->Send(); }
	char *GetUri () { return uri; }
	PluginInstance *GetPlugin ();

	Downloader *dl;

	BrowserDownloader *getBrowserDownloader () { return this->bdl; }
};

void npstream_downloader_set_stream_data (Downloader *downloader, NPP npp, NPStream *stream);
void downloader_initialize (void);
void downloader_destroy (void);

#endif // __PLUGIN_DOWNLOADER_H__
