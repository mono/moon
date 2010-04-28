/*
 * mms-downloader.cpp: MMS Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "mms-downloader.h"
#include "debug.h"
#include "pipeline-asf.h"

/*
 * MmsDownloader
 */
MmsDownloader::MmsDownloader (Downloader *dl, MmsSource *source)
	: InternalDownloader (dl, Type::MMSDOWNLOADER)
{
	LOG_MMS ("MmsDownloader::MmsDownloader ()\n");

	this->source = source;
	this->source->ref ();
}

MmsDownloader::~MmsDownloader ()
{
	LOG_MMS ("MmsDownloader::~MmsDownloader ()\n");

	ClearSource ();
}

void
MmsDownloader::Dispose ()
{
	ClearSource ();
	InternalDownloader::Dispose ();
}

void
MmsDownloader::Open (const char *verb, const char *uri)
{
	LOG_MMS ("MmsDownloader::Open (%s, %s)\n", verb, uri);
}

void
MmsDownloader::ClearSource ()
{
	MmsSource *source;

	mutex.Lock ();
	source = this->source;
	this->source = NULL;
	mutex.Unlock ();

	if (source != NULL)
		source->unref ();
}

void
MmsDownloader::Write (void *buf, gint32 off, gint32 n)
{
	LOG_MMS_EX ("MmsDownloader::Write (%p, %i, %i) this: %p id: %i\n", buf, off, n, this, GET_OBJ_ID (this));
	g_return_if_fail (source != NULL);
	mutex.Lock ();
	if (source != NULL)
		source->Write (buf, off, n);
	mutex.Unlock ();
}

char *
MmsDownloader::GetDownloadedFilename (const char *partname)
{
	LOG_MMS ("MmsDownloader::GetDownloadedFilename ('%s')\n", partname);
	return NULL;
}

char *
MmsDownloader::GetResponseText (const char *partname, gint64 *size)
{
	LOG_MMS ("MmsDownloader::GetResponseText ('%s', %p)\n", partname, size);
	return NULL;
}
