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

#include "multiscalesubimage.h"
#include "tilesource.h"
#include "eventargs.h"
#include "control.h"
#include "media.h"
#include "animation.h"

void multi_scale_image_handle_parsed (void *userdata);
struct BitmapImageContext;

/* @Namespace=System.Windows.Controls,ManagedDependencyProperties=Manual */
class MultiScaleImage : public MediaBase {
	GHashTable *cache;
	bool cache_contains (Uri* filename, bool check_empty_tile);
	MultiScaleTileSource *source;
	bool subimages_sorted;

	cairo_user_data_key_t width_key;
	cairo_user_data_key_t height_key;
	cairo_user_data_key_t full_opacity_at_key;

	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);

	GList *bitmapimages;
	BitmapImageContext *GetFreeBitmapImageContext ();
	void DownloadTile (BitmapImageContext *ctx, Uri *tile, int subimage, int level, int x, int y);
	BitmapImageContext *GetBitmapImageContext (BitmapImage *bitmapimage);

	void RenderSingle (cairo_t *cr, Region *region);
	void RenderCollection (cairo_t *cr, Region *region);

	Storyboard *zoom_sb;
	Storyboard *pan_sb;
	Storyboard *fadein_sb;
	DoubleAnimationUsingKeyFrames *zoom_animation;
	PointAnimationUsingKeyFrames *pan_animation;
	DoubleAnimation *fadein_animation;

	void SetIsDownloading (bool value);
	void SetIsIdle (bool value);

 protected:
	virtual ~MultiScaleImage ();

 public:
	void EmitImageOpenSucceeded ();
	void EmitMotionFinished ();
	void TileOpened (BitmapImage *bitmapImage);
	void TileFailed (BitmapImage *bitmapImage);

	/* @PropertyType=bool,DefaultValue=true,Version=3.0,GenerateAccessors */
	const static int AllowDownloadingProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateGetter */
	const static int AspectRatioProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=3.0,GenerateAccessors */
	const static int BlurFactorProperty;
	/* @PropertyType=bool,DefaultValue=false,Version=3.0,GenerateGetter */
	const static int IsDownloadingProperty;
	/* @PropertyType=bool,DefaultValue=true,Version=3.0,GenerateGetter */
	const static int IsIdleProperty;
	/* @PropertyType=MultiScaleTileSource,Version=2.0,GenerateAccessors */
	const static int SourceProperty;
	/* @PropertyType=MultiScaleSubImageCollection,AutoCreateValue,ReadOnly,Version=2.0,GenerateGetter */
	const static int SubImagesProperty;
	/* @PropertyType=bool,DefaultValue=true,Version=2.0,GenerateAccessors */
	const static int UseSpringsProperty;
	/* @PropertyType=Point,DefaultValue=Point(0\,0),Version=2.0,GenerateGetter */
	const static int ViewportOriginProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateGetter */
	const static int ViewportWidthProperty;

	/* @PropertyType=double,DefaultValue=0.0,Version=2.0 */
	const static int TileFadeProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	MultiScaleImage ();

	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);

	//
	// Methods
	//
	/* @GenerateCBinding,GeneratePInvoke,GenerateJSBinding */
	void ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY);
	/* @GenerateCBinding,GeneratePInvoke */
	Point ElementToLogicalPoint (Point elementPoint);
	/* @GenerateCBinding,GeneratePInvoke */
	Point LogicalToElementPoint (Point logicalPoint);

	/* @GenerateJSBinding */
	MultiScaleSubImage *GetIthSubImage (int index);
	// There is no documentation in MSDN for this method, it only shows up in a few tests.
	/* @GenerateJSBinding */
	int LogicalToElementX (int x, int y);
	// There is no documentation in MSDN for this method, it only shows up in a few tests.
	/* @GenerateJSBinding */
	int LogicalToElementY (int x, int y);
	// There is no documentation in MSDN for this method, it only shows up in a few tests.
	/* @GenerateJSBinding */
	int GetSubImageCount ();
	
	//
	// Property Accessors
	//
	double GetAspectRatio ();

	bool GetAllowDownloading ();
	void SetAllowDownloading (bool value);

	double GetBlurFactor ();
	void SetBlurFactor (double value);

	bool GetIsIdle ();

	bool GetIsDownloading ();

	MultiScaleTileSource* GetSource ();
	void SetSource (MultiScaleTileSource* source);

	bool GetUseSprings ();
	void SetUseSprings (bool spring);

	Point* GetViewportOrigin ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetViewportOrigin (Point p);

	double GetViewportWidth ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetViewportWidth (double width);

	MultiScaleSubImageCollection *GetSubImages ();

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
