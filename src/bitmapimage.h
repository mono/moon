/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bitmapimage.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __BITMAPIMAGE_H__
#define __BITMAPIMAGE_H__

#include <gdk/gdkpixbuf.h>

#include "utils.h"
#include "dependencyobject.h"
#include "downloader.h"
#include "bitmapsource.h"

/* @Namespace=System.Windows.Media.Imaging */
class BitmapImage : public BitmapSource {
 private:
	Downloader *downloader;
	GdkPixbufLoader *loader;
	GError *error;
	char *part_name;
	Cancellable *get_res_aborter;

 protected:
	virtual ~BitmapImage ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	BitmapImage ();
	virtual void Dispose ();

	/* @PropertyType=Uri,AlwaysChange,GenerateAccessors,DefaultValue=Uri() */
	const static int UriSourceProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int ProgressProperty;

	void SetUriSource (Uri* value);
	Uri* GetUriSource ();
	
	void SetProgress (double progress);
	double GetProgress ();
	
	void CleanupLoader ();
	void CreateLoader (unsigned char *buffer);
	/* @GenerateCBinding,GeneratePInvoke */
	void PixbufWrite (gpointer buffer, gint32 offset, gint32 n);
	/* @GenerateCBinding,GeneratePInvoke */
	void PixmapComplete ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	const static int DownloadProgressEvent;
	const static int ImageFailedEvent;
	const static int ImageOpenedEvent;

	void SetDownloader (Downloader *downloader, Uri *uri, const char *part_name);
	void CleanupDownloader ();
	void DownloaderProgressChanged ();
	void DownloaderComplete ();
	void DownloaderFailed ();
	void UriSourceChanged ();

	static void uri_source_changed_callback (EventObject *user_data);
	static void downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void pixbuf_write (void *buffer, gint32 offset, gint32 n, gpointer data);
};

#endif /* __BITMAPIMAGE_H__ */
