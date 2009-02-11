/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * media.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <glib.h>
#include <gdk/gdkpixbuf.h>

#include "value.h"
#include "brush.h"
#include "frameworkelement.h"
#include "error.h"
#include "downloader.h"
#include "pipeline.h"


/* NOTE: both of these formats are 32bits wide per pixel */
#if USE_OPT_RGB24
#define MOON_FORMAT_RGB CAIRO_FORMAT_RGB24
#else
#define MOON_FORMAT_RGB CAIRO_FORMAT_ARGB32
#endif
#define MOON_FORMAT_ARGB CAIRO_FORMAT_ARGB32

/* @Namespace=None */
/* @ManagedDependencyProperties=None */
class MediaAttribute : public DependencyObject {
 protected:
	virtual ~MediaAttribute () {}

 public:
 	/* @PropertyType=string,GenerateAccessors */
	const static int ValueProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	MediaAttribute () { SetObjectType (Type::MEDIAATTRIBUTE); }
	
	// property accessors

	const char *GetValue();
	void SetValue (const char *value);
};


/* @Namespace=None */
class MediaAttributeCollection : public DependencyObjectCollection {
 protected:
	virtual ~MediaAttributeCollection () {}

 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	MediaAttributeCollection () { SetObjectType (Type::MEDIAATTRIBUTE_COLLECTION); }
	
	virtual Type::Kind GetElementType () { return Type::MEDIAATTRIBUTE; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	MediaAttribute *GetItemByName (const char *name);
};


/*
 * This collection is always sorted by the time value of the markers.
 * We override AddToList to add the node where it's supposed to be, keeping the
 * collection sorted at all times.
 * We also override Insert to ignore the index and behave just like Add.
 */
/* @Namespace=System.Windows.Media */
class TimelineMarkerCollection : public DependencyObjectCollection {
 protected:
	virtual ~TimelineMarkerCollection () {}
	
 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	TimelineMarkerCollection () { SetObjectType (Type::TIMELINEMARKER_COLLECTION); }
	
	virtual Type::Kind GetElementType () { return Type::TIMELINEMARKER; }
	
	virtual int AddWithError (Value *value, MoonError *error);
	virtual bool InsertWithError (int index, Value *value, MoonError *error);
};


/* @Namespace=None */
class MarkerReachedEventArgs : public EventArgs {
	TimelineMarker *marker;
	
 protected:
	virtual ~MarkerReachedEventArgs ();
	
 public:
	MarkerReachedEventArgs (TimelineMarker *marker);

	TimelineMarker *GetMarker () { return marker; }
};


/* @Namespace=None */
/* @ManagedDependencyProperties=None */
class MediaBase : public FrameworkElement {
 private:
	static void set_source_async (EventObject *user_data);
	void SetSourceAsyncCallback ();

 protected:
	struct {
		Downloader *downloader;
		char *part_name;
		bool queued;
	} source;
	
	Downloader *downloader;
	char *part_name;
	
	int updating_size_from_media:1;
	int allow_downloads:1;
	int use_media_height:1;
	int use_media_width:1;
	int source_changed:1;
	
	virtual ~MediaBase ();
	
	virtual void DownloaderFailed (EventArgs *args);
	virtual void DownloaderComplete ();
	void DownloaderAbort ();
	
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	virtual void OnEmptySource () { }
	
	void SetDownloadProgress (double progress);
	
	virtual DownloaderAccessPolicy GetDownloaderPolicy (const char *uri) { return MediaPolicy; }

 public:
 	/* @PropertyType=string,AlwaysChange,GenerateAccessors */
	const static int SourceProperty;
 	/* @PropertyType=Stretch,DefaultValue=StretchUniform,GenerateAccessors */
	const static int StretchProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int DownloadProgressProperty;
	
	const static int DownloadProgressChangedEvent;
	
	/* @GenerateCBinding,GeneratePInvoke */
	MediaBase ();

	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSource (const char *uri);
	
	void SetAllowDownloads (bool allow);
	bool AllowDownloads () { return allow_downloads; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	virtual void OnLoaded ();
	
	//
	// Property Accessors
	//
	double GetDownloadProgress ();
	
	const char *GetSource ();
	
	void SetStretch (Stretch stretch);
	Stretch GetStretch ();
};


/* @Namespace=System.Windows.Controls */
class Image : public MediaBase {
	int create_xlib_surface:1;

	bool CreateSurface (const char *filename);
	bool IsSurfaceCached ();
	void CleanupSurface ();
	void CleanupPattern ();

	// downloader callbacks
	void PixbufWrite (void *buf, gint32 offset, gint32 n);
	virtual void DownloaderFailed (EventArgs *args);
	virtual void DownloaderComplete ();
	void UpdateProgress ();
	void UpdateSize ();
	
	static void pixbuf_write (void *buf, gint32 offset, gint32 n, gpointer data);
	static void size_notify (gint64 size, gpointer data);
	
	// pattern caching
	cairo_pattern_t *pattern;

	// pixbuf loading
	GdkPixbufLoader *loader;
	GError *loader_err;

 protected:
	virtual ~Image ();
	
	virtual void OnEmptySource () { CleanupSurface (); }
	
 public:
 	/* @PropertyType=BitmapImage,ManagedPropertyType=ImageSource,GenerateAccessors */
	const static int SourceProperty;

	static GHashTable *surface_cache;
	
	const static int ImageFailedEvent;
	
	struct CachedSurface {
		int xlib_surface_created:1;
		int ref_count:30;
		int has_alpha:1;
		
		GdkPixbuf *backing_pixbuf;
		cairo_surface_t *cairo;
		guchar *backing_data;
		char *filename;
		
		int height;
		int width;
	};
	
	CachedSurface *surface;
	ImageBrush *brush;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	Image ();
	
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	
	cairo_surface_t *GetCairoSurface ();
	
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	int GetImageHeight () { return surface ? surface->height : 0; };
	int GetImageWidth  () { return surface ? surface->width : 0; };
	
	virtual Rect GetCoverageBounds ();
	
	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);

	virtual bool InsideObject (cairo_t *cr, double x, double y);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSource (BitmapImage *source);
	BitmapImage *GetSource ();
};

#endif /* __MEDIA_H__ */
