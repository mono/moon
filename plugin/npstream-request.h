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
	NPStream *stream;
	PluginInstance *instance;

 public:
	NPStreamRequest (const char *verb, const char *uri, PluginInstance *instance) : DownloaderRequest (verb, uri)
	{
		this->stream = NULL;
		this->instance = instance;
	}

	virtual ~NPStreamRequest ()
	{
	}

	virtual void Abort ();
	virtual bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context);
	virtual const bool IsAborted ();
	virtual void SetHttpHeader (const char *name, const char *value);
	virtual void SetBody (void *body, int size);
	
	void StreamDestroyed ();

	static void SetStreamData (Downloader *downloader, NPP npp, NPStream *stream);
};

#endif
