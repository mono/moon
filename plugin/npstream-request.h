 /*
 * npstream-downloader.h: NPStream Browser Request
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __NPSTREAM_DOWNLOADER_H
#define __NPSTREAM_DOWNLOADER_H

#include "moonlight.h"
#include "browser-bridge.h"
#include "plugin-downloader.h"

class NPStreamRequest : public DownloaderRequest {
 private:
	NPP npp;
	NPStream *stream;
	char *buffer;
	PluginInstance *instance;

	uint32_t offset;

 public:
	NPStreamRequest (const char *verb, const char *uri, PluginInstance *instance) : DownloaderRequest (verb, uri)
	{
		this->npp = NULL;
		this->stream = NULL;
		this->buffer = NULL;
		this->offset = 0;
		this->instance = instance;
	}

	virtual ~NPStreamRequest ()
	{
		g_free (buffer);
	}

	void Abort ();
	bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context);
	const bool IsAborted () { return this->aborted; }
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (void *body, int size);
	
	void SetNPP (NPP npp) { this->npp = npp; }
	void SetStream (NPStream *stream) { this->stream = stream; }
	void StreamDestroyed () { stream = NULL; }
};

#endif
