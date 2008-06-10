/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * file-downloader.h: File Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlist-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __FILE_DOWNLOADER_H__
#define __FILE_DOWNLOADER_H__

#include <glib.h>

G_BEGIN_DECLS

#include "internal-downloader.h"
#include "downloader.h"

class FileDownloader : public InternalDownloader {
 private:
	char *uri;
	char *filename;
	char *unzipdir;
	
	bool deobfuscated;
	bool unlinkit;
	bool unzipped;

	InternalDownloader *internal_dl;
	
	bool DownloadedFileIsZipped ();
	void CleanupUnzipDir ();

 public:
	FileDownloader (Downloader *dl);
	~FileDownloader ();

	void Open (const char *verb, const char *uri);
	void Write (void *buf, gint32 offset, gint32 n);
	char *GetDownloadedFilename (const char *partname);
	char *GetResponseText (const char *partname, guint64 *size);
	
	const char *GetDownloadedFile ();

	bool IsDeobfuscated ();
	void SetDeobfuscated (bool val);
	void SetDeobfuscatedFile (const char *filename);

	const char *GetUnzippedPath ();

	void setFilename (char *fname) { if (filename) g_free (filename); filename = g_strdup (fname); }
};

G_END_DECLS

#endif
