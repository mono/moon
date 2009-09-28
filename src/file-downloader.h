/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * file-downloader.h: File Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __FILE_DOWNLOADER_H__
#define __FILE_DOWNLOADER_H__

#include <glib.h>

#include "internal-downloader.h"
#include "downloader.h"

class FileDownloader : public InternalDownloader {
 private:
	char *filename;
	char *unzipdir;
	char *uri;
	
	bool unlinkit;
	bool unzipped;
	
	bool DownloadedFileIsZipped ();
	void CleanupUnzipDir ();

	virtual ~FileDownloader ();

 public:
	FileDownloader (Downloader *dl);
	
	virtual void Open (const char *verb, const char *uri);
	virtual void Write (void *buf, gint32 offset, gint32 n);
	virtual char *GetDownloadedFilename (const char *partname);
	virtual char *GetResponseText (const char *partname, gint64 *size);

	const char *GetDownloadedFile ();
	
	const char *GetUnzippedPath ();

	virtual void SetFilename (const char *fname) { g_free (filename); filename = g_strdup (fname); }
	void SetUnlink (bool value) { unlinkit = value; }
};

#endif /* __FILE_DOWNLOADER_H__ */
