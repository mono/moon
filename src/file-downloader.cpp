/*
 * file-downloader.cpp: File Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "file-downloader.h"

//TODO: Move all the zip related semantics in here to clean up downloader.cpp

FileDownloader::FileDownloader (Downloader *dl) : InternalDownloader (dl)
{
}

FileDownloader::~FileDownloader ()
{
}

void
FileDownloader::Open (const char *verb, const char *uri)
{
	dl->InternalOpen (verb, uri, false);
}

void
FileDownloader::Write (void *buf, int32_t off, int32_t n)
{
	dl->InternalWrite (buf, off, n);
}
