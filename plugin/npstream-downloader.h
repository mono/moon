 /*
 * npstream-downloader.h: NPStream Browser Downloader
 *
 * Author:
 *   Geoff Norton  (gnorton@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __NPSTREAM_DOWNLOADER_H
#define __NPSTREAM_DOWNLOADER_H

#include "moonlight.h"
#include "plugin-downloader.h"

class NPStreamDownloader : public BrowserDownloader {
 private:
	NPP npp;
	NPStream *stream;
	char *buffer;

	uint32_t offset;

 public:
	NPStreamDownloader (PluginDownloader *pdl) : BrowserDownloader (pdl)
	{
		this->npp = NULL;
		this->stream = NULL;
		this->buffer = NULL;
		this->offset = 0;
	}

	virtual ~NPStreamDownloader ()
	{
		g_free (buffer);
	}

	void Abort ();
	void Send ();
	void Started ();
	void Finished ();
	uint32_t Read (char *buffer, uint32_t length);
	
	void SetResponse (BrowserResponse *response) { this->response = response; }
	void SetNPP (NPP npp) { this->npp = npp; }
	void SetStream (NPStream *stream) { this->stream = stream; }

	void StreamDestroyed () { stream = NULL; }
};

#endif
