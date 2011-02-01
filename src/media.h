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
#include "bitmapsource.h"


/* @Namespace=System.Windows.Media */
class MOON_API MediaAttribute : public DependencyObject {
 protected:
	virtual ~MediaAttribute () {}

 public:
 	/* @PropertyType=string,GenerateAccessors */
	const static int ValueProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	MediaAttribute () { SetObjectType (Type::MEDIAATTRIBUTE); }
	
	//
	// Property Accessors
	//
	const char *GetValue();
	void SetValue (const char *value);
};


/* @Namespace=System.Windows.Media */
class MOON_API MediaAttributeCollection : public DependencyObjectCollection {
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
/* @ManagedDependencyProperties=None */
/* @ManagedEvents=Manual */
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
	
	int allow_downloads:1;
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
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
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
class MOON_API Image : public MediaBase {
 private:
	void DownloadProgress ();
	void ImageOpened (RoutedEventArgs *args);
	void ImageFailed (ImageErrorEventArgs *args);
	void SourcePixelDataChanged ();

	static void download_progress (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void image_opened (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void image_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void source_pixel_data_changed (EventObject *sender, EventArgs *calldata, gpointer closure);

 protected:
	virtual ~Image ();
	
 public:
 	/* @PropertyType=ImageSource,AutoCreator=Image::CreateDefaultImageSource,GenerateAccessors */
	const static int SourceProperty;

	/* @DelegateType=EventHandler<ExceptionRoutedEventArgs> */
	const static int ImageFailedEvent;

	/* @DelegateType=EventHandler<RoutedEventArgs> */
	const static int ImageOpenedEvent;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	Image ();
	
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	
	virtual void SetSourceInternal (Downloader *downloader, char *PartName);
	virtual void SetSource (Downloader *downloader, const char *PartName);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

	int GetImageHeight () { return GetSource () ? GetSource ()->GetPixelHeight () : 0; };
	int GetImageWidth  () { return GetSource () ? GetSource ()->GetPixelWidth () : 0; };
	
	virtual Rect GetCoverageBounds ();

	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);
	virtual Size ComputeActualSize ();
	virtual bool CanFindElement () { return true; }

	virtual bool InsideObject (cairo_t *cr, double x, double y);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSource (ImageSource *source);
	ImageSource *GetSource ();

	static Value *CreateDefaultImageSource (DependencyObject *instance, DependencyProperty *property);
};

#endif /* __MEDIA_H__ */
