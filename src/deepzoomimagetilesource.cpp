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
	SetObjectType (Type::DEEPZOOMIMAGETILESOURCE);

	callback = NULL;
	downloader = NULL;
	downloaded = false;
	get_tile_func = NULL;
}

DeepZoomImageTileSource::DeepZoomImageTileSource (const char *uri)
{
	callback = NULL;
	downloader = NULL;
	downloaded = false;
	SetValue (DeepZoomImageTileSource::UriSourceProperty, Value (uri));
	get_tile_func = NULL;
}

DeepZoomImageTileSource::~DeepZoomImageTileSource ()
{
	if (downloader) {
		downloader_abort (downloader);
		downloader->unref ();
	}		
}

void
DeepZoomImageTileSource::set_downloaded_cb (downloaded_cb cb)
{
	callback = cb;
}

void
DeepZoomImageTileSource::Download ()
{
	if (downloaded)
		return;
	char *stringuri;
	if (stringuri = GetValue (DeepZoomImageTileSource::UriSourceProperty)->AsString ()) {
		downloaded = true;
		download_uri (GetValue (DeepZoomImageTileSource::UriSourceProperty)->AsString ());
	}
}

void 
DeepZoomImageTileSource::download_uri (const char* url)
{
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

	if (callback != NULL)
		callback (filename);
}

void
DeepZoomImageTileSource::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((DeepZoomImageTileSource *) closure)->DownloaderComplete ();
}

void
DeepZoomImageTileSource::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == DeepZoomImageTileSource::UriSourceProperty) {
		downloaded = false;
	}

	if (args->property->GetOwnerType () != Type::DEEPZOOMIMAGETILESOURCE) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}


