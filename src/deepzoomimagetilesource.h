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
#include "utils.h"

namespace Moonlight {

/* @Version=2,Namespace=System.Windows.Media */
class DeepZoomImageTileSource : public MultiScaleTileSource {
	Cancellable *get_resource_aborter;
	bool is_collection;
	bool downloaded;
	bool parsed;
	bool nested;
	
	char *format;
	char *server_format;
	GPtrArray *display_rects;
	GPtrArray *subimages;
	XML_Parser parser;
	int max_level;
	
	void Init ();
	
	void UriSourceChanged ();	
	void Abort ();
	
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	DeepZoomImageTileSource ();

	virtual ~DeepZoomImageTileSource ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=Uri,IsConstPropertyType,GenerateAccessors */
	const static int UriSourceProperty;

	/* @SkipFactories */
	DeepZoomImageTileSource (Uri *uri, bool nested = false);
	
	bool IsCollection () { return is_collection; }
	int GetMaxLevel () { return max_level; }
	
	MultiScaleSubImage *GetSubImage (guint index);
	guint GetSubImageCount ();
	
	void Download ();
	void DownloaderComplete ();
	void DownloaderFailed ();
	bool GetTileLayer (int level, int x, int y, Uri **uri);
	bool IsDownloaded () { return downloaded; }
	bool IsParsed () { return parsed; }
	char *GetServerFormat () { return server_format; }

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	void XmlWrite (char *buffer, gint32 offset, gint32 n);
	void EndElement (void *info, const char *el);
	
	//
	// Property Accessors
	//
	void SetUriSource (const Uri *value);
	const Uri *GetUriSource ();
	
	//
	// Events
	//
	const static int DownloaderCompletedEvent;
	const static int DownloaderFailedEvent;
	const static int UriSourceChangedEvent;
};

};
#endif /* __DEEPZOOMIMAGETILESOURCE_H__ */
