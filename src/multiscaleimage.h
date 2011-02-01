/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * multiscaleimage.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
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
#include "ptr.h"

struct BitmapImageContext;

/* @Namespace=System.Windows.Controls */
class MOON_API MultiScaleImage : public MediaBase {
	friend class MultiScaleImagePropertyValueProvider;

	GHashTable *cache;
	bool cache_contains (Uri* filename, bool check_empty_tile);
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

	DOPtr<Storyboard> zoom_sb;
	DOPtr<Storyboard> pan_sb;
	DOPtr<Storyboard> fadein_sb;
	DOPtr<DoubleAnimationUsingKeyFrames> zoom_animation;
	DOPtr<PointAnimationUsingKeyFrames> pan_animation;
	DOPtr<DoubleAnimation> fadein_animation;

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

	double *GetZoomAnimationEndPoint ();
	void SetZoomAnimationEndPoint (double endpoint);
	Point *GetPanAnimationEndPoint ();
	void SetPanAnimationEndPoint (Point endpoint);

	double zoom_target;
	Point pan_target;

	bool is_fading;
	bool is_zooming;
	bool is_panning;

 protected:
	virtual ~MultiScaleImage ();

 public:
	void EmitImageOpenSucceeded ();
	/* @GenerateCBinding */
	void EmitMotionFinished ();
	/* @GenerateCBinding */
	void EmitImageFailed ();
	/* @GenerateCBinding */
	void EmitImageOpenFailed ();
	void FadeFinished ();
	void ZoomFinished ();
	void PanFinished ();
	void TileOpened (BitmapImage *bitmapImage);
	void TileFailed (BitmapImage *bitmapImage);

	/* @PropertyType=bool,DefaultValue=true,Version=3.0,GenerateAccessors */
	const static int AllowDownloadingProperty;
	/* @PropertyType=double,ReadOnly,DefaultValue=1.0,Version=2.0,GenerateGetter */
	const static int AspectRatioProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=3.0,GenerateAccessors */
	const static int BlurFactorProperty;
	/* @PropertyType=bool,ReadOnly,DefaultValue=false,Version=3.0,GenerateGetter */
	const static int IsDownloadingProperty;
	/* @PropertyType=bool,ReadOnly,DefaultValue=true,Version=3.0,GenerateGetter */
	const static int IsIdleProperty;
	/* @PropertyType=MultiScaleTileSource,Version=2.0,GenerateAccessors */
	const static int SourceProperty;
	/* @PropertyType=MultiScaleSubImageCollection,AutoCreateValue,ReadOnly,Version=2.0,GenerateGetter,GenerateManagedAccessors=false */
	const static int SubImagesProperty;
	/* @PropertyType=bool,DefaultValue=true,Version=2.0,GenerateAccessors */
	const static int UseSpringsProperty;
	/* @PropertyType=Point,AlwaysChange,DefaultValue=Point(0\,0),Version=2.0,GenerateAccessors */
	const static int ViewportOriginProperty;
	/* @PropertyType=double,AlwaysChange,DefaultValue=1.0,Version=2.0,GenerateAccessors */
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
	/* @GenerateCBinding,GeneratePInvoke */
	void ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY);
	/* @GenerateCBinding,GeneratePInvoke */
	Point ElementToLogicalPoint (Point elementPoint);
	/* @GenerateCBinding,GeneratePInvoke */
	Point LogicalToElementPoint (Point logicalPoint);

	MultiScaleSubImage *GetIthSubImage (int index);
	// There is no documentation in MSDN for this method, it only shows up in a few tests.
	int LogicalToElementX (int x, int y);
	// There is no documentation in MSDN for this method, it only shows up in a few tests.
	int LogicalToElementY (int x, int y);
	// There is no documentation in MSDN for this method, it only shows up in a few tests.
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
	/* @DelegateType=RoutedEventHandler */
	const static int ImageFailedEvent;
	/* @DelegateType=EventHandler<ExceptionRoutedEventArgs> */
	const static int ImageOpenFailedEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int ImageOpenSucceededEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int MotionFinishedEvent;
	/* @DelegateType=RoutedEventHandler */
	const static int ViewportChangedEvent;

	BitmapImageContext *GetFreeBitmapImageContext ();
	void StopDownloading ();
	void DownloadTile (BitmapImageContext *ctx, Uri *tile, void *user_data);
	/* @GenerateCBinding */
	void HandleDzParsed ();

	/* @GenerateCBinding */
	void OnSourcePropertyChanged ();

	/* @GenerateCBinding */
	void InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer);
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
