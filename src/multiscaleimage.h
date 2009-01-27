/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * multiscaleimage.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MULTISCALEIMAGE_H__
#define __MULTISCALEIMAGE_H__

#include <glib.h>
#include <cairo.h>

#include "tilesource.h"
#include "eventargs.h"
#include "control.h"
#include "media.h"

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class MultiScaleImage : public MediaBase {
	void DownloaderAbort ();
	void DownloadUri (const char* url);
	int layer_to_render;
	MultiScaleTileSource *source;
	char *filename;
	bool continue_rendering;

	void DownloaderComplete ();
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	double GetViewportHeight ();

	int layers;
	Downloader *downloader;

 protected:
	virtual ~MultiScaleImage ();

 public:
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateGetter */
	static DependencyProperty *AspectRatioProperty;
	/* @PropertyType=MultiScaleTileSource,Version=2.0,GenerateAccessors */
	static DependencyProperty *SourceProperty;
//	/* @PropertyType=ReadOnlyCollection<MultiScaleSubImage>,Version=2.0,GenerateGetter */
// FIXME: When this DP is reinstated - uncomment the validator in the MultiScaleImage constructor
//	static DependencyProperty *SubImagesProperty;
	/* @PropertyType=bool,DefaultValue=true,Version=2.0,GenerateAccessors */
	static DependencyProperty *UseSpringsProperty;
	/* @PropertyType=Point,DefaultValue=Point(0\,0),Version=2.0,GenerateAccessors */
	static DependencyProperty *ViewportOriginProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateAccessors */
	static DependencyProperty *ViewportWidthProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	MultiScaleImage ();

	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, Region *region);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	//
	// Methods
	//
	/* @GenerateCBinding,GeneratePInvoke */
	void ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY);
	/* @GenerateCBinding,GeneratePInvoke */
	Point ElementToLogicalPoint (Point elementPoint);

	//
	// Property Accessors
	//
	double GetAspectRatio ();

	MultiScaleTileSource* GetSource ();
	void SetSource (MultiScaleTileSource* source);

	bool GetUseSprings ();
	void SetUseSprings (bool spring);

	Point* GetViewportOrigin ();
	void SetViewportOrigin (Point* p);

	double GetViewportWidth ();
	void SetViewportWidth (double width);

	//
	// Events
	//
	const static int ImageFailedEvent;
	const static int ImageOpenFailedEvent;
	const static int ImageOpenSucceededEvent;
	const static int MotionFinishedEvent;
	const static int ViewportChangedEvent;
};

#endif /* __MULTISCALIMAGE_H__ */
