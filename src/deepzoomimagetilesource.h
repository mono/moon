/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * deepzoomimagetilesource.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __DEEPZOOMIMAGETILESOURCE_H__
#define __DEEPZOOMIMAGETILESOURCE_H__

#include "tilesource.h"
#include "downloader.h"

typedef void (*downloaded_cb) (const char* path);

/* @Version=2,Namespace=System.Windows.Media */
class DeepZoomImageTileSource : public MultiScaleTileSource {

	Downloader* downloader;

	downloaded_cb callback;

	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	void DownloaderComplete ();	

 protected:
	virtual ~DeepZoomImageTileSource ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	DeepZoomImageTileSource ();

	virtual Type::Kind GetObjectType () { return Type::DEEPZOOMIMAGETILESOURCE; }	

	//
	// Methods
	//
	/* @GenerateCBinding */
	void download_urisource (const char* uri, downloaded_cb callback); 
};

#endif /* __DEEPZOOMIMAGETILESOURCE_H__ */
