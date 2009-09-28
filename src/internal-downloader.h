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

class InternalDownloader;

#include "dependencyobject.h"

class Downloader;

class InternalDownloader : public EventObject {
 protected:
	Downloader *dl;

	virtual ~InternalDownloader () {}

 public:
	InternalDownloader (Downloader *dl, Type::Kind type)
		: EventObject (type)
	{
		this->dl = dl;
	}

	virtual void Open (const char *verb, const char *uri) = 0;
	virtual void Write (void *buf, gint32 offset, gint32 n) = 0;
	virtual char *GetResponseText (const char *partname, gint64 *size) = 0; 
	virtual char *GetDownloadedFilename (const char *partname) = 0;
	virtual void SetFilename (const char *fname) = 0;
};

#endif /* __INTERNAL_DOWNLOADER_H__ */
