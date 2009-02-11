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
gpointer get_tile_layer (int level, int x, int y, void *user_data);

/* @Version=2,Namespace=System.Windows.Media */
class DeepZoomImageTileSource : public MultiScaleTileSource {
	friend class MultiScaleImage;
	friend gpointer get_tile_layer (int level, int x, int y, void *user_data);
	friend void multi_scale_image_handle_parsed (void *userdata);
	friend void end_element (void *data, const char *el);


	void Download ();
	gpointer GetTileLayer (int level, int x, int y);

	Downloader* downloader;

	parsed_cb parsed_callback;
	void *cb_userdata;

	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	void DownloaderComplete ();	
	void download_uri (const char* url);
	bool downloaded;
	char *format;
	bool nested;
	GList *display_rects;
	GList *subimages;

	void Init ();

	void Parse (const char* filename);

	void set_parsed_cb (parsed_cb callback, void *userdata)
	{
		parsed_callback = callback;
		cb_userdata = userdata;
	}

	bool isCollection;
	int maxLevel;

 protected:
	virtual ~DeepZoomImageTileSource ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	DeepZoomImageTileSource ();

	DeepZoomImageTileSource (const char *uri, bool nested = false);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	//
	// Properties
	//
	/* @PropertyType=string,ManagedPropertyType=Uri,GenerateAccessors */
	const static int UriSourceProperty;

	void        SetUriSource (const char *value);
	const char* GetUriSource ();
		
};

#endif /* __DEEPZOOMIMAGETILESOURCE_H__ */
