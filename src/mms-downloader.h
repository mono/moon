/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mms-downloader.h: MMS Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlist-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MMS_DOWNLOADER_H__
#define __MMS_DOWNLOADER_H__

#include <glib.h>

#include "internal-downloader.h"
#include "mutex.h"

class MmsDownloader : public InternalDownloader {
private:
	Mutex mutex;
	MmsSource *source;

 protected:
	virtual ~MmsDownloader ();

 public:
	MmsDownloader (Downloader *dl, MmsSource *srcs);
	void Dispose ();

	virtual void Open (const char *verb, const char *uri);
	virtual void Write (void *buf, gint32 offset, gint32 n);
	virtual char *GetDownloadedFilename (const char *partname);
	virtual char *GetResponseText (const char *partname, gint64 *size);
	virtual void SetFilename (const char *fname) { /* we don't need this */ }

	void ClearSource ();
};

#endif /* __MMS_DOWNLOADER_H__ */
