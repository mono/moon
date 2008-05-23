/*
 * hhtp-downloader.h: Browser Http Downloader
 *
 * Author:
 *   Geoff Norton  (gnorton@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __HTTP_DOWNLOADER_H
#define __HTTP_DOWNLOADER_H

#include "moonlight.h"
#include "plugin-downloader.h"

class HttpDownloader : public BrowserDownloader {
 private:
	char *buffer;

	uint32_t offset;

 public:
	HttpDownloader (PluginDownloader *pdl) : BrowserDownloader (pdl)
	{
		this->buffer = NULL;
		this->offset = 0;
	}

	~HttpDownloader ()
	{
		g_free (buffer);
	}

	void Abort ();
	void Send ();
	void Started ();
	void Finished ();
	uint32_t Read (char *buffer, uint32_t length);
	
	void SetResponse (BrowserResponse *response) { this->response = response; }
};

#endif
