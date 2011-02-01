/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * deepzoomimagetilesource.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __DEEPZOOMIMAGETILESOURCE_H__
#define __DEEPZOOMIMAGETILESOURCE_H__

#include <expat.h>
#include "multiscalesubimage.h"
#include "multiscaleimage.h"
#include "tilesource.h"
#include "downloader.h"
#include "uri.h"

typedef void (*msi_cb) (MultiScaleImage *msi);

/* @Version=2,Namespace=System.Windows.Media */
class MOON_API DeepZoomImageTileSource : public MultiScaleTileSource {

	msi_cb parsed_callback;
	msi_cb failed_callback;
	msi_cb sourcechanged_callback;
	MultiScaleImage *cb_userdata;

	bool downloaded;
	bool parsed;
	char *format;
	bool nested;
	GList *display_rects;
	XML_Parser parser;

	void Init ();

	void UriSourceChanged ();	
	void Abort ();

	bool isCollection;
	int maxLevel;
	Cancellable *get_resource_aborter;

 protected:
	virtual ~DeepZoomImageTileSource ();

 public:
	GList *subimages;
	bool IsCollection () { return isCollection;}
	int GetMaxLevel () { return maxLevel;}

	/* @GenerateCBinding,GeneratePInvoke */
	DeepZoomImageTileSource ();
	DeepZoomImageTileSource (Uri *uri, bool nested = false);

	void Download ();
	void DownloaderComplete ();	
	void DownloaderFailed ();
	bool GetTileLayer (int level, int x, int y, Uri *uri);
	bool IsDownloaded () {return downloaded; }
	bool IsParsed () {return parsed; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	void XmlWrite (char* buffer, gint32 offset, gint32 n);

	void set_callbacks (msi_cb parsed, msi_cb failed, msi_cb source_changed, MultiScaleImage *userdata)
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
