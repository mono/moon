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

/* @Namespace=System.Windows.Controls */
class MultiScaleImage : public MediaBase {
	friend class MultiScaleImagePropertyValueProvider;

	GHashTable *cache;
	bool cache_contains (Uri* filename, bool check_empty_tile);
	MultiScaleTileSource *source;
	bool subimages_sorted;
	bool pending_motion_completed;

	cairo_user_data_key_t width_key;
	cairo_user_data_key_t height_key;
	cairo_user_data_key_t full_opacity_at_key;

	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);

	GList *bitmapimages;
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

	/* @PropertyType=double,DefaultValue=0.0,Version=2.0,GenerateManagedAccessors=false,ManagedFieldAccess=Private */
	const static int TileFadeProperty;
	/* @PropertyType=Point,DefaultValue=Point(0\,0),Version=2.0,GenerateGetter,GenerateManagedAccessors=false,ManagedFieldAccess=Private */
	const static int InternalViewportOriginProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateGetter,GenerateManagedAccessors=false,ManagedFieldAccess=Private */
	const static int InternalViewportWidthProperty;

	Point* GetInternalViewportOrigin ();
	void SetInternalViewportOrigin (Point* p);

	double GetInternalViewportWidth ();
	void SetInternalViewportWidth (double width);

	double zoom_target;
	Point pan_target;

	bool is_fading;
	bool is_zooming;
	bool is_panning;

 protected:
	virtual ~MultiScaleImage ();

 public:
	void EmitImageOpenSucceeded ();
	void EmitMotionFinished ();
	void EmitImageFailed ();
	void EmitImageOpenFailed ();
	void FadeFinished ();
	void ZoomFinished ();
	void PanFinished ();
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
	/* @PropertyType=MultiScaleSubImageCollection,AutoCreateValue,ReadOnly,Version=2.0,GenerateGetter,GenerateManagedAccessors=false */
	const static int SubImagesProperty;
	/* @PropertyType=bool,DefaultValue=true,Version=2.0,GenerateAccessors */
	const static int UseSpringsProperty;
	/* @PropertyType=Point,DefaultValue=Point(0\,0),Version=2.0,GenerateAccessors */
	const static int ViewportOriginProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateAccessors */
	const static int ViewportWidthProperty;


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
	void SetViewportOrigin (Point* p);

	double GetViewportWidth ();
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

	BitmapImageContext *GetFreeBitmapImageContext ();
	void DownloadTile (BitmapImageContext *ctx, Uri *tile, int subimage, int level, int x, int y);
};

/*
 * MultiScaleImagePropertyValueProvider
 */
 
class MultiScaleImagePropertyValueProvider : public FrameworkElementProvider {
 private:
	Value *viewport_origin;
	Value *viewport_width;

	Value *GetViewportOrigin ();
	Value *GetViewportWidth ();

 public:
	MultiScaleImagePropertyValueProvider (MultiScaleImage *obj, PropertyPrecedence precedence);
	virtual ~MultiScaleImagePropertyValueProvider ();
	virtual Value *GetPropertyValue (DependencyProperty *property);
};


#endif /* __MULTISCALIMAGE_H__ */
