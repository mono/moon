/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * lunar-downloader.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __LUNAR_DOWNLOADER_H__
#define __LUNAR_DOWNLOADER_H__

#include "browser-bridge.h"
#include "downloader.h"

class LunarDownloader {
	static BrowserBridge *bridge;
	
	DownloaderResponseHeaderCallback response_header_callback;
	gpointer response_header_context;
	DownloaderResponse *response;
	DownloaderRequest *request;
	Downloader *downloader;
	uint64_t offset;
	bool finished;
	bool aborted;
	char *verb;
	char *uri;
	
 public:
	static void SetBridge (BrowserBridge *bb) { bridge = bb; }
	static BrowserBridge *GetBridge () { return bridge; }
	
	LunarDownloader (Downloader *dl);
	~LunarDownloader ();
	
	void Open (const char *verb, const char *uri, bool streaming, bool disable_cache);
	void Abort ();
	void Send ();
	
	bool IsAborted () { return aborted; }
	
	guint32 Read (char *buffer, guint32 length);
	void Started ();
	void Finished (bool success, gpointer data, const char *uri);
	
	void SetHttpHeader (const char *header, const char *value, bool disable_folding);
	void SetBody (void *body, guint32 length);
	
	void SetResponseHeaderCallback (DownloaderResponseHeaderCallback callback, gpointer context);
	
	void SetResponse (DownloaderResponse *response);
	
	DownloaderResponse *GetResponse () { return response; }
	DownloaderRequest *GetRequest () { return request; }
};

G_BEGIN_DECLS

void lunar_downloader_init (void);
void lunar_downloader_shutdown (void);

G_END_DECLS

#endif /* __LUNAR_DOWNLOADER_H__ */
