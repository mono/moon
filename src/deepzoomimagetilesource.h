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
#include "uri.h"

typedef void (*downloaded_cb) (const char* path);

/* @Version=2,Namespace=System.Windows.Media */
class DeepZoomImageTileSource : public MultiScaleTileSource {

	Downloader* downloader;

//	downloaded_cb callback;

	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	void DownloaderComplete ();	
	void download_uri (const char* url);
	bool downloaded;
	char *format;

	void Parse (const char* filename);

 protected:
	virtual ~DeepZoomImageTileSource ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	DeepZoomImageTileSource ();

	DeepZoomImageTileSource (const char *uri);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	//
	// Methods
	//
//	/* @GenerateCBinding */
//	void set_downloaded_cb (downloaded_cb callback);

	virtual void Download ();
	gpointer GetTileLayer (int level, int x, int y);

	//
	// Properties
	//

	/* @PropertyType=string,ManagedPropertyType=Uri,GenerateAccessors */
	static DependencyProperty *UriSourceProperty;

	void        SetUriSource (const char *value);
	const char* GetUriSource ();
		
};

#endif /* __DEEPZOOMIMAGETILESOURCE_H__ */
