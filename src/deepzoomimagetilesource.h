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

#include "multiscalesubimage.h"
#include "multiscaleimage.h"
#include "tilesource.h"
#include "downloader.h"
#include "uri.h"

typedef void (*msi_cb) (void *userdata);

/* @Version=2,Namespace=System.Windows.Media */
class DeepZoomImageTileSource : public MultiScaleTileSource {


	Downloader* downloader;

	msi_cb parsed_callback;
	msi_cb failed_callback;
	msi_cb sourcechanged_callback;
	void *cb_userdata;

	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	void DownloaderComplete ();	
	void download_uri (const char* url);
	bool downloaded;
	bool parsed;
	char *format;
	bool nested;
	GList *display_rects;

	void Init ();

	void Parse (const char* filename);

	bool isCollection;
	int maxLevel;

 protected:
	virtual ~DeepZoomImageTileSource ();

 public:
	GList *subimages;
	bool IsCollection () { return isCollection;}
	int GetMaxLevel () { return maxLevel;}

	/* @GenerateCBinding,GeneratePInvoke */
	DeepZoomImageTileSource ();
	DeepZoomImageTileSource (Uri *uri, bool nested = false);

	/* @GenerateCBinding,GeneratePInvoke */
	void strip_and_set_uri (Uri *uri);

	void Download ();
	bool GetTileLayer (int level, int x, int y, Uri *uri);
	bool IsDownloaded () {return downloaded; }
	bool IsParsed () {return parsed; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	void set_callbacks (msi_cb parsed, msi_cb failed, msi_cb source_changed, void *userdata)
	{
		parsed_callback = parsed;
		failed_callback = failed;
		sourcechanged_callback = source_changed;
		cb_userdata = userdata;
	}


	//
	// Properties
	//
	/* @PropertyType=Uri,GenerateAccessors */
	const static int UriSourceProperty;

	void        SetUriSource (Uri* value);
	Uri*        GetUriSource ();

	void EndElement (void *info, const char* el);
		
};

#endif /* __DEEPZOOMIMAGETILESOURCE_H__ */
