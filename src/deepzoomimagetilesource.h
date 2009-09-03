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

typedef void (*parsed_cb) (void *userdata);

/* @Version=2,Namespace=System.Windows.Media */
class DeepZoomImageTileSource : public MultiScaleTileSource {


	Downloader* downloader;

	parsed_cb parsed_callback;
	parsed_cb failed_callback;
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
	void set_parsed_cb (parsed_cb callback, parsed_cb f_callback, void *userdata)
	{
		parsed_callback = callback;
		failed_callback = f_callback;
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
