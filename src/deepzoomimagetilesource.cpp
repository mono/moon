/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * deepzoomimagetilesource.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>

#include "deepzoomimagetilesource.h"
#include "uri.h"
#include "runtime.h"
#include "file-downloader.h"

DeepZoomImageTileSource::DeepZoomImageTileSource ()
{
	downloader = NULL;
}

DeepZoomImageTileSource::~DeepZoomImageTileSource ()
{
	if (downloader) {
		downloader_abort (downloader);
		downloader->unref ();
	}		
}

void
DeepZoomImageTileSource::download_urisource (const char* url, downloaded_cb cb)
{
	callback = cb;

	Uri *uri = new Uri ();

	Surface* surface = GetSurface ();
	if (!surface)
		return;
	
	if (!(uri->Parse (url)))
		return;
	
	if (!downloader)
		downloader = surface->CreateDownloader ();
	
	if (!downloader)
		return;

	downloader->Open ("GET", uri->ToString (), NoPolicy);
	
	downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);

	downloader->Send ();

	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete ();
	} else 
		downloader->Send ();
	delete uri;
}

void
DeepZoomImageTileSource::DownloaderComplete ()
{
	const char *filename;

	if (!(filename = downloader->getFileDownloader ()->GetDownloadedFile ()))
		return;

	callback (filename);
}


void
DeepZoomImageTileSource::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((DeepZoomImageTileSource *) closure)->DownloaderComplete ();
}
