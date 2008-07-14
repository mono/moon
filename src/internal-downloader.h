/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * downloader.h: Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __INTERNAL_DOWNLOADER_H__
#define __INTERNAL_DOWNLOADER_H__

#include <stdint.h>

class Downloader;

class InternalDownloader {
 public:
 	enum DownloaderType {
	 	MmsDownloader,
	 	FileDownloader,
 	};
 protected:
	Downloader *dl;

 public:
	InternalDownloader (Downloader *dl)
	{
		this->dl = dl;
	}

	~InternalDownloader ()
	{
	}

	virtual void Open (const char *verb, const char *uri) = 0;
	virtual void Write (void *buf, gint32 offset, gint32 n) = 0;
	virtual char *GetResponseText (const char *partname, guint64 *size) = 0; 
	virtual char *GetDownloadedFilename (const char *partname) = 0;
	virtual DownloaderType GetType () = 0;
};

#endif
