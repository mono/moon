/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * multiscaleimage.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

// TODO
//
// - only invalidate regions
// - only render changed regions
// - render all images to a single surface to cache, and do scaling
//   on that rather than scaling each individual image and piecing
//   them together.
//

#include <config.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#include "cbinding.h"
#include "multiscaleimage.h"
#include "tilesource.h"
#include "deepzoomimagetilesource.h"
#include "multiscalesubimage.h"
#include "bitmapimage.h"
#include "ptr.h"
#include "factory.h"

namespace Moonlight {

#if LOGGING
#include "clock.h"
#define MSI_STARTTIMER(id)    if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MSI)) TimeSpan id##_t_start = get_now()
#define MSI_ENDTIMER(id,str)  if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MSI)) TimeSpan id##_t_end = get_now(); printf ("timing of '%s' took %f ms\n", str, id##_t_end, (double)(id##_t_end - id##_t_start) / 10000)
#else
#define MSI_STARTTIMER(id)
#define MSI_ENDTIMER(id,str)
#endif

#define MAX_DOWNLOADERS 6

static inline guint64
pow2 (int pow)
{
	return ((guint64) 1 << pow);
}

#define blur_factor_is_valid(x) (!isnan (x) && !isinf (x) && (x) > 0.0)

static inline int
blur_factor_get_offset (double factor)
{
	int offset;
	
	if (frexp (factor, &offset) == 0.5)
		offset--;
	
	return -offset;
}

/*
 * Q(uad)Tree.
 */

struct QTreeTile {
	cairo_surface_t *image;
	double opacity;
};

struct QTree {
	QTreeTile *tile;
	QTree *parent;
	QTree *l0; //N-E
	QTree *l1; //N-W
	QTree *l2; //S-E
	QTree *l3; //S-W
};

static QTree *
qtree_new (void)
{
	return g_slice_new0 (QTree);
}

static QTree *
qtree_insert (QTree *root, int level, guint64 x, guint64 y)
{
	guint64 level2 = pow2 (level);
	QTree *node = root;
	
	if (x >= level2 || y >= level2)
		return NULL;
	
	if (!root)
		return NULL;
	
	while (node && level-- > 0) {
		level2 = pow2 (level);
		
		if (y < level2) {
			if (x < level2) {
				if (!node->l0) {
					node->l0 = qtree_new ();
					node->l0->parent = node;
				}
				node = node->l0;
			} else {
				if (!node->l1) {
					node->l1 = qtree_new ();
					node->l1->parent = node;
				}
				node = node->l1;
				x -= level2;
			}
		} else {
			if (x < level2) {
				if (!node->l2) {
					node->l2 = qtree_new ();
					node->l2->parent = node;
				}
				node = node->l2;
				y -= level2;
			} else {
				if (!node->l3) {
					node->l3 = qtree_new ();
					node->l3->parent = node;
				}
				node = node->l3;
				x -= level2;
				y -= level2;
			}
		}
	}
	
	return node;
}

static void
qtree_set_tile (QTree *node, cairo_surface_t *image, double opacity)
{
	if (node->tile) {
		if (node->tile->image)
			cairo_surface_destroy (node->tile->image);
	} else
		node->tile = g_slice_new (QTreeTile);
	
	node->tile->opacity = opacity;
	node->tile->image = image;
}

static QTree *
qtree_lookup (QTree *root, int level, guint64 x, guint64 y)
{
	guint64 level2 = pow2 (level);
	QTree *node = root;
	
	if (x >= level2 || y >= level2)
		return NULL;
	
	while (node && level-- > 0) {
		level2 = pow2 (level);
		
		if (y < level2) {
			if (x < level2) {
				node = node->l0;
			} else {
				node = node->l1;
				x -= level2;
			}
		} else {
			if (x < level2) {
				node = node->l2;
				y -= level2;
			} else {
				node = node->l3;
				x -= level2;
				y -= level2;
			}
		}
	}
	
	return node;
}

static QTreeTile *
qtree_lookup_tile (QTree *root, int level, guint64 x, guint64 y)
{
	QTree *node = qtree_lookup (root, level, x, y);
	
	return node ? node->tile : NULL;
}

static void
qtree_remove (QTree *node, int depth)
{
	if (!node)
		return;
	
	if (node->tile) {
		if (node->tile->image)
			cairo_surface_destroy (node->tile->image);
		g_slice_free (QTreeTile, node->tile);
		node->tile = NULL;
	}
	
	if (depth <= 0)
		return;
	
	qtree_remove (node->l0, depth - 1);
	qtree_remove (node->l1, depth - 1);
	qtree_remove (node->l2, depth - 1);
	qtree_remove (node->l3, depth - 1);
}

static void
qtree_remove_at (QTree *root, int level, guint64 x, guint64 y, int depth)
{
	QTree *node = qtree_lookup (root, level, x, y);
	
	qtree_remove (node, depth);
}

static inline bool
qtree_has_tile (QTree *node)
{
	return node->tile != NULL;
}

static void
qtree_destroy (QTree *root)
{
	if (!root)
		return;
	
	if (root->tile) {
		if (root->tile->image)
			cairo_surface_destroy (root->tile->image);
		g_slice_free (QTreeTile, root->tile);
	}
	
	qtree_destroy (root->l0);
	qtree_destroy (root->l1);
	qtree_destroy (root->l2);
	qtree_destroy (root->l3);
	g_slice_free (QTree, root);
}

/*
 * BitmapImageContext
 */

struct BitmapImageContext {
	MultiScaleImage *msi;
	BitmapImage *image;
	QTree *node;
	bool avail;
	int retry;
};

/*
 * Morton layout
 */

#if 0
static void
morton (int n, int *x, int *y) {
	n = (n & 0x99999999) + ((n & 0x22222222) << 1) + ((n & 0x44444444) >> 1);
	n = (n & 0xc3c3c3c3) + ((n & 0x0c0c0c0c) << 2) + ((n & 0x30303030) >> 2);
	n = (n & 0xf00ff00f) + ((n & 0x00f000f0) << 4) + ((n & 0x0f000f00) >> 4);
	n = (n & 0xff0000ff) + ((n & 0x0000ff00) << 8) + ((n & 0x00ff0000) >> 8);
	*x = n & 0x0000ffff;
	*y = n >> 16;
}
#endif

static inline int
morton_x (int n)
{
	n = (n & 0x11111111) + ((n & 0x44444444) >> 1);
	n = (n & 0x03030303) + ((n & 0x30303030) >> 2);
	n = (n & 0x000f000f) + ((n & 0x0f000f00) >> 4);
	return  (n & 0x000000ff) + ((n & 0x00ff0000) >> 8);
}

static inline int
morton_y (int n)
{
	n = (n & 0x88888888) + ((n & 0x22222222) << 1);
	n = (n & 0xc0c0c0c0) + ((n & 0x0c0c0c0c) << 2);
	n = (n & 0xf000f000) + ((n & 0x00f000f0) << 4);
	n = (n & 0xff000000) + ((n & 0x0000ff00) << 8);

	return n >> 16;
}

static void
int_free (gpointer user_data)
{
	delete (int *) user_data;
}


#define MOTION_IS_FADING   (1 << 0)
#define MOTION_IS_PANNING  (1 << 1)
#define MOTION_IS_ZOOMING  (1 << 2)
#define MOTION_IS_FINISHED (1 << 3)

#define IS_IN_MOTION(m) (((m) & (MOTION_IS_FADING | MOTION_IS_PANNING | MOTION_IS_ZOOMING)) != 0)

/*
 * MultiScaleImage
 */

MultiScaleImage::MultiScaleImage ()
{
	SetObjectType (Type::MULTISCALEIMAGE);
	
	// Note: cairo_user_data_key_t's do not need to be initialized
	
	cache = g_hash_table_new_full (g_int_hash, g_int_equal, int_free, (GDestroyNotify) qtree_destroy);
	downloaders = g_ptr_array_new ();
	subimages_sorted = false;
	pan_target = Point (0, 0);
	zoom_target = 1.0;
	n_downloading = 0;
	motion = 0;
}

MultiScaleImage::~MultiScaleImage ()
{
	MultiScaleTileSource *source = GetSource ();
	
	StopDownloading ();
	
	if (source)
		DisconnectSourceEvents (source);
	
	g_ptr_array_free (downloaders, true);
	g_hash_table_destroy (cache);
}

void
MultiScaleImage::ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY)
{
	LOG_MSI ("\nzoomabout logical %f  (%f, %f)\n", zoomIncrementFactor, zoomCenterLogicalX, zoomCenterLogicalY);

	if (zoom_sb)
		zoom_sb->PauseWithError (NULL);
	if (pan_sb)
		pan_sb->PauseWithError (NULL);

	double viewport_width;
	Point viewport_origin;

	if (GetUseSprings () && zoom_sb && pan_sb) {
		viewport_width = zoom_target;
		viewport_origin = pan_target;
	} else {
		viewport_width = GetAnimatedViewportWidth ();
		viewport_origin = *GetAnimatedViewportOrigin ();
	}

	double width = viewport_width / zoomIncrementFactor;
	SetViewportWidth (width);
	if (!isnan(zoomCenterLogicalX) && !isnan(zoomCenterLogicalY)) {
		SetViewportOrigin (new Point (zoomCenterLogicalX - (zoomCenterLogicalX - viewport_origin.x) / zoomIncrementFactor,
					      zoomCenterLogicalY - (zoomCenterLogicalY - viewport_origin.y) / zoomIncrementFactor));
	}
}

Point
MultiScaleImage::ElementToLogicalPoint (Point elementPoint)
{
	Point *vp_origin = GetAnimatedViewportOrigin ();
	double vp_width = GetAnimatedViewportWidth ();
	double actual_width = GetActualWidth ();
	return Point (vp_origin->x + (double)elementPoint.x * vp_width / actual_width,
		      vp_origin->y + (double)elementPoint.y * vp_width / actual_width);
}

Point
MultiScaleImage::LogicalToElementPoint (Point logicalPoint)
{
	Point *vp_origin = GetAnimatedViewportOrigin ();
	double vp_width = GetAnimatedViewportWidth ();
	double actual_width = GetActualWidth ();
	return Point ((logicalPoint.x - vp_origin->x) * actual_width / vp_width,
		      (logicalPoint.y - vp_origin->y) * actual_width / vp_width);
}

bool
MultiScaleImage::CanDownloadMoreTiles ()
{
	return n_downloading < MAX_DOWNLOADERS;
}

void
MultiScaleImage::DownloadTile (Uri *tile, void *user_data)
{
	BitmapImageContext *ctx, *avail = NULL;
	
	// Check that we aren't already downloading this tile and find an
	// available BitmapImageContext
	for (guint i = 0; i < downloaders->len; i++) {
		ctx = (BitmapImageContext *) downloaders->pdata[i];
		
		if (ctx->avail) {
			if (!avail)
				avail = ctx;
			
			continue;
		}
		
		if (ctx->image->GetUriSource ()->operator==(*tile)) {
			//LOG_MSI ("Tile %s is already being downloaded\n", tile->ToString ());
			return;
		}
	}
	
	//LOG_MSI ("downloading tile %s\n", tile->ToString ());
	
	if (!avail) {
		ctx = new BitmapImageContext ();
		ctx->image = MoonUnmanagedFactory::CreateBitmapImage ();
		ctx->msi = this;
		
		ctx->image->AddHandler (ctx->image->ImageOpenedEvent, tile_opened, ctx);
		ctx->image->AddHandler (ctx->image->ImageFailedEvent, tile_failed, ctx);
		g_ptr_array_add (downloaders, ctx);
	} else
		ctx = avail;
	
	ctx->node = (QTree *) user_data;
	ctx->avail = false;
	ctx->retry = 0;
	
	SetIsDownloading (true);
	ctx->image->SetDownloadPolicy (MsiPolicy);
	ctx->image->SetUriSource (tile);
	SetIsIdle (false);
	
	n_downloading++;
}

// Only used for DeepZoom sources
void
MultiScaleImage::HandleDzParsed ()
{
	// if the source is a collection, fill the subimages list
	MultiScaleTileSource *source = GetSource ();
	MultiScaleSubImageCollection *subs = GetSubImages ();
	DeepZoomImageTileSource *dzits = NULL;
	
	if (source->GetImageWidth () >= 0 && source->GetImageHeight () >= 0) {
		SetValue (MultiScaleImage::AspectRatioProperty, Value (source->GetImageWidth () / source->GetImageHeight ()));
	} else {
		SetValue (MultiScaleImage::AspectRatioProperty, Value (1.0));
	}
	
	if (source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
		MultiScaleSubImage *si;
		
		dzits = (DeepZoomImageTileSource *) source;
		
		for (guint i = 0; i < dzits->GetSubImageCount (); i++) {
			si = dzits->GetSubImage (i);
			subs->Add (si);
		}
	}
	
	Invalidate ();
	
	// Get the first tiles
	int shared_index = -1;
	QTree *shared_cache = (QTree *) g_hash_table_lookup (cache, &shared_index);
	if (!shared_cache)
		g_hash_table_insert (cache, new int(shared_index), (shared_cache = qtree_new ()));
	
	int layer = 0;
	while (CanDownloadMoreTiles ()) {
		Uri *tile = NULL;
		
		if (source->get_tile_func (layer, 0, 0, &tile, source) && tile != NULL) {
			QTree *node;
			
			if ((node = qtree_insert (shared_cache, layer, 0, 0)))
				DownloadTile (tile, node);
		}
		
		delete tile;
		layer++;
	}
	
	EmitImageOpenSucceeded ();
}

void
MultiScaleImage::fade_finished (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->FadeFinished ();
}

void
MultiScaleImage::FadeFinished ()
{
	motion = (motion & ~MOTION_IS_FADING) | MOTION_IS_FINISHED;
	if (!IS_IN_MOTION (motion)) {
		//printf ("FadeFinished emitting MotionFinished\n");
		MotionFinished ();
	}
}

void
MultiScaleImage::zoom_finished (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->ZoomFinished ();
}

void
MultiScaleImage::ZoomFinished ()
{
	motion = (motion & ~MOTION_IS_ZOOMING) | MOTION_IS_FINISHED;
	if (!IS_IN_MOTION (motion)) {
		//printf ("ZoomFinished emitting MotionFinished\n");
		MotionFinished ();
	}
}

void
MultiScaleImage::pan_finished (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->PanFinished ();
}

void
MultiScaleImage::PanFinished ()
{
	motion = (motion & ~MOTION_IS_PANNING) | MOTION_IS_FINISHED;
	if (!IS_IN_MOTION (motion)) {
		//printf ("PanFinished emitting MotionFinished\n");
		MotionFinished ();
	}
}

void
MultiScaleImage::tile_opened (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	BitmapImageContext *ctx = (BitmapImageContext *) closure;
	
	ctx->msi->TileOpened (ctx);
}

void
MultiScaleImage::TileOpened (BitmapImageContext *ctx)
{
	ProcessTile (ctx);
	
	ctx->image->SetUriSource (NULL);
	ctx->avail = true;
	n_downloading--;
	Invalidate ();
}

void
MultiScaleImage::tile_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	BitmapImageContext *ctx = (BitmapImageContext *) closure;
	
	ctx->msi->TileFailed (ctx);
}

void
MultiScaleImage::TileFailed (BitmapImageContext *ctx)
{
	if (ctx->retry < 5) {
		ctx->image->SetUriSource (ctx->image->GetUriSource ());
		ctx->retry++;
	} else {
		LOG_MSI ("caching a NULL for %s\n", ctx->image->GetUriSource()->ToString ());
		qtree_set_tile (ctx->node, NULL, 0.0);
		ctx->avail = true;
		n_downloading--;
		Invalidate ();
	}
	
	EmitImageFailed ();
}

void
MultiScaleImage::StopDownloading ()
{
	BitmapImageContext *ctx;
	
	for (guint i = 0; i < downloaders->len; i++) {
		ctx = (BitmapImageContext *) downloaders->pdata[i];
		ctx->image->Abort ();
		ctx->image->Dispose ();
		ctx->image->unref ();
		delete ctx;
	}
	
	g_ptr_array_set_size (downloaders, 0);
	n_downloading = 0;
}

void
MultiScaleImage::tile_layer_invalidated (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	TileLayerInvalidatedEventArgs *args = (TileLayerInvalidatedEventArgs *) calldata;
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	
	msi->InvalidateTileLayer (args->GetLevel (), args->GetTilePositionX (), args->GetTilePositionY (), args->GetTileLayer ());
}

void
MultiScaleImage::downloader_completed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((MultiScaleImage *) closure)->HandleDzParsed ();
}

void
MultiScaleImage::downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((MultiScaleImage *) closure)->EmitImageOpenFailed ();
}

void
MultiScaleImage::DisconnectSourceEvents (MultiScaleTileSource *source)
{
	if (!source)
		return;
	
	source->RemoveHandler (MultiScaleTileSource::TileLayerInvalidatedEvent, tile_layer_invalidated, this);
	
	if (source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
		DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *) source;
		dzits->RemoveHandler (DeepZoomImageTileSource::DownloaderCompletedEvent, downloader_completed, this);
		dzits->RemoveHandler (DeepZoomImageTileSource::DownloaderFailedEvent, downloader_failed, this);
		dzits->RemoveHandler (DeepZoomImageTileSource::UriSourceChangedEvent, uri_source_changed, this);
	}
}

void
MultiScaleImage::ConnectSourceEvents (MultiScaleTileSource *source)
{
	if (!source)
		return;
	
	source->AddHandler (MultiScaleTileSource::TileLayerInvalidatedEvent, tile_layer_invalidated, this);
	
	if (source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
		DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *) source;
		dzits->AddHandler (DeepZoomImageTileSource::DownloaderCompletedEvent, downloader_completed, this);
		dzits->AddHandler (DeepZoomImageTileSource::DownloaderFailedEvent, downloader_failed, this);
		dzits->AddHandler (DeepZoomImageTileSource::UriSourceChangedEvent, uri_source_changed, this);
	}
}

void
MultiScaleImage::subdownloader_completed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((UIElement *) closure)->Invalidate ();
}

void
MultiScaleImage::subdownloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((MultiScaleImage *) closure)->EmitImageFailed ();
}

void
MultiScaleImage::DisconnectSubImageEvents (MultiScaleSubImage *subimage)
{
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *) subimage->source;
	
	dzits->RemoveHandler (DeepZoomImageTileSource::DownloaderCompletedEvent, subdownloader_completed, this);
	dzits->RemoveHandler (DeepZoomImageTileSource::DownloaderFailedEvent, subdownloader_failed, this);
}

void
MultiScaleImage::ConnectSubImageEvents (MultiScaleSubImage *subimage)
{
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *) subimage->source;
	
	dzits->AddHandler (DeepZoomImageTileSource::DownloaderCompletedEvent, subdownloader_completed, this);
	dzits->AddHandler (DeepZoomImageTileSource::DownloaderFailedEvent, subdownloader_failed, this);
}

Size
MultiScaleImage::ComputeActualSize ()
{
	Size result = MediaBase::ComputeActualSize ();
	UIElement *parent = GetVisualParent ();
	Size available;
	
	if (parent && !parent->Is (Type::CANVAS))
		if (ReadLocalValue (LayoutInformation::LayoutSlotProperty))
			return result;
	
	available = Size (INFINITY, INFINITY);
	available = ApplySizeConstraints (available);
	
	// FIXME: this keeps the moon-unit DefaultValue tests passing.
	if (isinf (available.width) && isinf (available.height))
		return Size (0, 0);
	
	result = MeasureOverrideWithError (available, NULL);
	result = ApplySizeConstraints (result);
	
	return result;
}

Size
MultiScaleImage::MeasureOverrideWithError (Size availableSize, MoonError *error)
{
	double vp_w = GetAnimatedViewportWidth ();
	double vp_ar = GetAspectRatio ();
	Size desired = availableSize;
	
	if (isinf (desired.width)) {
		if (isinf (desired.height)) {
			desired.height = vp_w / vp_ar;
			desired.width = vp_w;
		} else {
			desired.width = desired.height * vp_ar;
		}
	} else if (isinf (desired.height)) {
		desired.height = desired.width / vp_ar;
	}
	
	return desired;
}

void
MultiScaleImage::ProcessTile (BitmapImageContext *ctx)
{
	cairo_surface_t *surface;
	double tile_fade;
	
	if (!(surface = cairo_surface_reference (ctx->image->GetSurface (NULL)))) {
		LOG_MSI ("caching NULL for %s\n", ctx->image->GetUriSource ()->ToString ());
		qtree_set_tile (ctx->node, NULL, 0.0);
		return;
	}
	
	if (!fadein_sb) {
		fadein_sb = MoonUnmanagedFactory::CreateStoryboard ();
		fadein_sb->SetManualTargetWithError (this, NULL);
		fadein_sb->SetTargetProperty (fadein_sb, new PropertyPath ("(MultiScaleImage.TileFade)"));
		fadein_animation = MoonUnmanagedFactory::CreateDoubleAnimation ();
		fadein_animation->SetDuration (Duration (GetSource ()->GetTileBlendTime ()));
		TimelineCollection *tlc = new TimelineCollection ();
		tlc->Add (static_cast<DoubleAnimation*> (fadein_animation));
		fadein_sb->SetChildren (tlc);
		tlc->unref ();
		fadein_sb->AddHandler (Storyboard::CompletedEvent, fade_finished, this);
#if DEBUG
		fadein_sb->SetName ("Multiscale Fade-In");
#endif
	} else {
		fadein_sb->PauseWithError (NULL);
		motion &= ~MOTION_IS_FADING;
	}
	
	tile_fade = GetTileFade ();
	//LOG_MSI ("animating Fade from %f to %f\n\n", tile_fade, tile_fade + 0.9);
	fadein_animation->SetFrom (tile_fade);
	fadein_animation->SetTo (tile_fade + 0.9);
	
	if (fadein_sb->BeginWithError (NULL))
		motion |= MOTION_IS_FADING;
	
	UpdateIdleStatus ();
	
	LOG_MSI ("caching %s\n", ctx->image->GetUriSource ()->ToString ());
	qtree_set_tile (ctx->node, surface, tile_fade + 0.9);
}

void
MultiScaleImage::Render (cairo_t *cr, Region *region, bool path_only)
{
	MultiScaleTileSource *source = GetSource ();
	DeepZoomImageTileSource *dzits;
	
	LOG_MSI ("MSI::Render\n");
	
	if (!source) {
		LOG_MSI ("no sources set, nothing to render\n");
		UpdateIdleStatus ();
		return;
	}
	
	if (source->Is (Type::DEEPZOOMIMAGETILESOURCE))
		dzits = (DeepZoomImageTileSource *) source;
	else
		dzits = NULL;
	
	bool is_collection = dzits && dzits->IsCollection () && GetSubImages ();
	
	if (source->GetImageWidth () < 0 && !is_collection) {
		LOG_MSI ("nothing to render so far...\n");
		if (dzits != NULL)
			dzits->Download ();
		
		UpdateIdleStatus ();
		return;
	}
	
#if DEBUG
	if (!source->get_tile_func) {
		g_warning ("no get_tile_func set\n");
		return;
	}
#endif
	
	if (is_collection)
		RenderCollection (cr, region);
	else
		RenderSingle (cr, region);
	
	UpdateIdleStatus ();
}

void
MultiScaleImage::RenderCollection (cairo_t *cr, Region *region)
{
	MultiScaleTileSource *source = GetSource ();
	DeepZoomImageTileSource *dzits;
	double msi_w = GetActualWidth ();
	double msi_h = GetActualHeight ();
	double msi_ar = GetAspectRatio();
	double msivp_ox = GetAnimatedViewportOrigin()->x;
	double msivp_oy = GetAnimatedViewportOrigin()->y;
	double msivp_w = GetAnimatedViewportWidth();
	double blur_factor = GetBlurFactor ();
	double fade = GetTileFade ();
	int blur_offset;
	int max_level;
	
	LOG_MSI ("\nMSI::RenderCollection\n");
	
	if (!source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
		g_warning ("RenderCollection called for a non deepzoom tile source. this should not happen");
		return;
	}
	
	dzits = (DeepZoomImageTileSource *) source;
	
	if (!dzits->IsParsed ())
		return;
	
	max_level = dzits->GetMaxLevel ();
	
	if (msi_w <= 0.0 || msi_h <= 0.0 || !blur_factor_is_valid (blur_factor))
		return; // invisible widget, nothing to render
	
	blur_offset = blur_factor_get_offset (blur_factor);
	
	Rect viewport = Rect (msivp_ox, msivp_oy, msivp_w, msivp_w/msi_ar);

	MultiScaleSubImageCollection *subs = GetSubImages ();
	if (!subimages_sorted) {
		subs->ResortByZIndex ();
		subimages_sorted = true;
	}

	// using the "-1" index for the shared cache
	int shared_index = -1;
	QTree *shared_cache = (QTree *) g_hash_table_lookup (cache, &shared_index);
	if (!shared_cache)
		g_hash_table_insert (cache, new int(shared_index), (shared_cache = qtree_new ()));

	int subs_count = subs->GetCount ();
	
	for (int i = 0; i < subs_count; i++) {
		MultiScaleSubImage *sub_image = (MultiScaleSubImage *) subs->z_sorted->pdata[i];
		DeepZoomImageTileSource *sub_dzits = (DeepZoomImageTileSource *) sub_image->source;

		int index = sub_image->GetId();
		QTree *subimage_cache = (QTree *) g_hash_table_lookup (cache, &index);
		if (!subimage_cache)
			g_hash_table_insert (cache, new int(index), (subimage_cache = qtree_new ()));
		
		double subvp_ox = sub_image->GetViewportOrigin()->x;
		double subvp_oy = sub_image->GetViewportOrigin()->y;
		double subvp_w = sub_image->GetViewportWidth();
		double sub_w = sub_image->source->GetImageWidth ();
		double sub_h = sub_image->source->GetImageHeight ();
		double sub_ar = sub_image->GetAspectRatio();
		
		// expressing the subimage viewport in main viewport coordinates.
		Rect sub_vp = Rect (-subvp_ox / subvp_w, -subvp_oy / subvp_w, 1.0/subvp_w, 1.0/(sub_ar * subvp_w));
		
		// render only if the subimage viewport intersects with this viewport
		if (!sub_vp.IntersectsWith (viewport))
			continue;
		
		LOG_MSI ("Intersects with main viewport...rendering\n");
		
		int layers;
		if (frexp (MAX (sub_w, sub_h), &layers) == 0.5)
			layers--;
		
		int optimal_layer;
		
		frexp (msi_w / (subvp_w * msivp_w * MIN (1.0, sub_ar)), &optimal_layer);
		LOG_MSI ("number of layers: %d; optimal layer: %d; BlurFactor: %.2f; adjustment: %d\n",
			 layers, optimal_layer, blur_factor, blur_offset);
		
		optimal_layer = MIN (optimal_layer + blur_offset, layers);
		
		int to_layer = -1;
		int from_layer = optimal_layer;
		while (from_layer >= 0) {
			bool parsed = (from_layer > max_level && sub_dzits->IsParsed ());
			int tile_width = parsed ? sub_image->source->GetTileWidth () : dzits->GetTileWidth ();
			int tile_height = parsed ? sub_image->source->GetTileHeight () : dzits->GetTileHeight ();
			guint64 from_layer2 = pow2 (from_layer);
			guint64 layers2 = pow2 (layers - from_layer);
			double v_scale = (double) layers2 * sub_vp.width / sub_w;
			double v_tile_w = tile_width * v_scale;
			double v_tile_h = tile_height * v_scale;
			double minx = (MAX (msivp_ox, sub_vp.x) - sub_vp.x) / v_tile_w;
			double maxx = MIN (msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;
			double miny = (MAX (msivp_oy, sub_vp.y) - sub_vp.y) / v_tile_h;
			double maxy = MIN (msivp_oy + msivp_w / msi_ar, sub_vp.y + sub_vp.width / sub_ar) - sub_vp.y;
			bool blending = false;
			int count = 0;
			int found = 0;
			
			//LOG_MSI ("virtual tile size at layer %d; %fx%f\n", from_layer, v_tile_w, v_tile_h);
			
			for (guint64 i = (guint64) minx; i < from_layer2 && i * v_tile_w < maxx; i++) {
				for (guint64 j = (guint64) miny; j < from_layer2 && j * v_tile_h < maxy; j++) {
					QTreeTile *tile;
					
					count++;
					
					if (from_layer > max_level)
						tile = qtree_lookup_tile (subimage_cache, from_layer, i, j);
					else
						tile = qtree_lookup_tile (shared_cache, from_layer,
									  morton_x (sub_image->n) * from_layer2 / tile_width,
									  morton_y (sub_image->n) * from_layer2 / tile_height);
					
					if (tile && tile->image) {
						if (tile->opacity > GetTileFade ())
							blending = true;
						
						found++;
					}
				}
			}
			
			if (found > 0 && to_layer < from_layer)
				to_layer = from_layer;
			
			if (found == count && (!blending || from_layer == 0))
				break;
			
			from_layer--;
		}
		
		// render loop
		LOG_MSI ("rendering layers from %d to %d\n", from_layer, to_layer);
		
		if (from_layer >= 0) {
			cairo_save (cr);
			cairo_rectangle (cr, 0, 0, msi_w, msi_h);
			cairo_clip (cr);
			cairo_scale (cr, msi_w / msivp_w, msi_w / msivp_w); //scale to widget

			cairo_translate (cr, 
					 -msivp_ox + sub_vp.x,
					 -msivp_oy + sub_vp.y);

			cairo_scale (cr, 
				     sub_vp.width/sub_w, 
				     sub_vp.width/sub_w);

			cairo_rectangle (cr, 0, 0, sub_w, sub_h);
			cairo_clip (cr);

			if (IS_TRANSLUCENT (sub_image->GetOpacity ()))
				cairo_push_group (cr);

			int layer_to_render = from_layer;
			while (layer_to_render <= to_layer) {
				bool parsed = (layer_to_render > max_level && sub_dzits->IsParsed ());
				int tile_width = parsed ? sub_image->source->GetTileWidth () : dzits->GetTileWidth ();
				int tile_height = parsed ? sub_image->source->GetTileHeight () : dzits->GetTileHeight ();
				guint64 render_layer2 = pow2 (layer_to_render);
				guint64 layers2 = pow2 (layers - layer_to_render);
				double v_scale = (double) layers2 * sub_vp.width / sub_w;
				double v_tile_w = tile_width * v_scale;
				double v_tile_h = tile_height * v_scale;
				double minx = (MAX (msivp_ox, sub_vp.x) - sub_vp.x) / v_tile_w;
				double maxx = MIN (msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;
				double miny = (MAX (msivp_oy, sub_vp.y) - sub_vp.y) / v_tile_h;
				double maxy = MIN (msivp_oy + msivp_w / msi_ar, sub_vp.y + sub_vp.width / sub_ar) - sub_vp.y;
				
				for (guint64 i = (guint64) minx; i < render_layer2 && i * v_tile_w < maxx; i++) {
					for (guint64 j = (guint64) miny; j < render_layer2 && j * v_tile_h < maxy; j++) {
						bool shared_tile = false;
						QTreeTile *tile;
						
						if (layer_to_render > max_level)
							tile = qtree_lookup_tile (subimage_cache, layer_to_render, i, j);
						else {
							// Check in the shared levels
							shared_tile = true;
							
							tile = qtree_lookup_tile (shared_cache, layer_to_render,
										  morton_x (sub_image->n) * render_layer2 / tile_width,
										  morton_y (sub_image->n) * render_layer2 / tile_height);
						}
						
						if (!tile || !tile->image)
							continue;
						
						LOG_MSI ("rendering subimage %d %d %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT "\n", sub_image->id, layer_to_render, i, j);
						cairo_save (cr);
						
						cairo_scale (cr, layers2, layers2);
						
						cairo_translate (cr, i * tile_width, j * tile_height);
						
						if (shared_tile) {
							cairo_translate (cr,
									 (int)(-morton_x(sub_image->n) * render_layer2) % tile_width,
									 (int)(-morton_y(sub_image->n) * render_layer2) % tile_height);
						}
						
						cairo_set_source_surface (cr, tile->image, 0, 0);
						
						double combined = 1.0;
						
						if (tile->opacity > fade) 
							combined = MIN (1.0 - tile->opacity + fade, 1.0);
						
						if (IS_TRANSLUCENT (combined))
							cairo_paint_with_alpha (cr, combined);
						else
							cairo_paint (cr);
						    
						cairo_restore (cr);
					}
				}
				
				layer_to_render++;
			}
			
			if (IS_TRANSLUCENT (sub_image->GetOpacity ())) {
				cairo_pattern_t *data = cairo_pop_group (cr);
				if (cairo_pattern_status (data) == CAIRO_STATUS_SUCCESS) {
					cairo_set_source (cr, data);
					cairo_paint_with_alpha (cr, sub_image->GetOpacity ());
				}
				cairo_pattern_destroy (data);
			}
			
			cairo_restore (cr);
		}
		
		if (!GetAllowDownloading () || !CanDownloadMoreTiles ())
			continue;
		
		// Download the next set of tiles..
		while (from_layer < optimal_layer) {
			from_layer ++;
			
			// if the subimage is unparsed, trigger the download
			if (from_layer > max_level && !sub_dzits->IsDownloaded ()) {
				sub_dzits->Download ();
				break;
			}
			
			bool parsed = (from_layer > max_level && sub_dzits->IsParsed ());
			int tile_width = parsed ? sub_image->source->GetTileWidth () : dzits->GetTileWidth ();
			int tile_height = parsed ? sub_image->source->GetTileHeight () : dzits->GetTileHeight ();
			guint64 layers2 = pow2 (layers - from_layer);
			double v_scale = (double) layers2 * sub_vp.width / sub_w;
			double v_tile_w = tile_width * v_scale;
			double v_tile_h = tile_height * v_scale;
			double minx = (MAX (msivp_ox, sub_vp.x) - sub_vp.x) / v_tile_w;
			double maxx = MIN (msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;
			double miny = (MAX (msivp_oy, sub_vp.y) - sub_vp.y) / v_tile_h;
			double maxy = MIN (msivp_oy + msivp_w / msi_ar, sub_vp.y + sub_vp.width / sub_ar) - sub_vp.y;
			MultiScaleTileSource *tile_source;
			QTree *tile_cache, *node;
			guint64 x, y;
			Uri *tile;
			
			for (int i = (int) minx; i * v_tile_w < maxx; i++) {
				if (!CanDownloadMoreTiles ())
					break;
				
				for (int j = (int) miny; j * v_tile_h < maxy; j++) {
					if (!CanDownloadMoreTiles ())
						break;
					
					if (from_layer <= max_level) {
						guint64 from_layer2 = pow2 (from_layer);
						x = morton_x (sub_image->n) * from_layer2 / tile_width;
						y = morton_y (sub_image->n) * from_layer2 / tile_height;
						tile_cache = shared_cache;
						tile_source = dzits;
					} else {
						tile_source = sub_image->source;
						tile_cache = subimage_cache;
						x = i;
						y = j;
					}
					
					if (!(node = qtree_insert (tile_cache, from_layer, x, y)))
						continue;
					
					if (!qtree_has_tile (node)) {
						tile = NULL;
						if (dzits->get_tile_func (from_layer, x, y, &tile, tile_source))
							DownloadTile (tile, node);
						delete tile;
					}
				}
			}
		}
	}
}

void
MultiScaleImage::RenderSingle (cairo_t *cr, Region *region)
{
	MultiScaleTileSource *source = GetSource ();
	double msi_w = GetActualWidth ();
	double msi_h = GetActualHeight ();
	double msi_ar = GetAspectRatio ();
	double im_w = source->GetImageWidth ();
	double im_h = source->GetImageHeight ();
	int tile_width = source->GetTileWidth ();
	int tile_height = source->GetTileHeight ();
	double vp_ox = GetAnimatedViewportOrigin()->x;
	double vp_oy = GetAnimatedViewportOrigin()->y;
	double vp_w = GetAnimatedViewportWidth ();
	double blur_factor = GetBlurFactor ();
	double fade = GetTileFade ();
	int optimal_layer;
	int blur_offset;
	int layers;
	
	if (msi_w <= 0.0 || msi_h <= 0.0 || !blur_factor_is_valid (blur_factor))
		return; // invisible widget, nothing to render
	
	blur_offset = blur_factor_get_offset (blur_factor);
	
	// number of layers in the MSI, aka the lowest powerof2 that's bigger than width and height
	if (frexp (MAX (im_w, im_h), &layers) == 0.5)
		layers --;
	
	// optimal layer for this... aka "best viewed at"
	double fr;
	
	if ((fr = frexp (msi_w / (vp_w * MIN (1.0, msi_ar)), &optimal_layer)) == 0.5)
		optimal_layer--;
	
	LOG_MSI ("number of layers: %d; optimal layer: %d; BlurFactor: %.2f; adjustment: %d; fr = %.4f\n",
		 layers, optimal_layer, blur_factor, blur_offset, fr);
	
	optimal_layer = MIN (optimal_layer + blur_offset, layers);
	
	// We have to figure all the layers that we'll have to render:
	// - from_layer is the highest COMPLETE layer that we can display (all tiles are
	//   there and blended (except for level 0, where it might not be blended yet))
	// - to_layer is the highest PARTIAL layer that we can display (contains at least
	//   1 tiles partially blended)

	int to_layer = -1;
	int from_layer = optimal_layer;
	
	// using the "-1" index for the single image case
	int index = -1;
	QTree *subimage_cache = (QTree *) g_hash_table_lookup (cache, &index);
	if (!subimage_cache)
		g_hash_table_insert (cache, new int(index), (subimage_cache = qtree_new ()));

	while (from_layer >= 0) {
		guint64 layers2 = pow2 (layers - from_layer);
		guint64 from_layer2 = pow2 (from_layer);
		double v_scale = (double) layers2 / im_w;
		double v_tile_w = tile_width * v_scale;
		double v_tile_h = tile_height * v_scale;
		double minx = MAX (0, (vp_ox / v_tile_w));
		double maxx = MIN (vp_ox + vp_w, 1.0);
		double miny = MAX (0, (vp_oy / v_tile_h));
		double maxy = MIN (vp_oy + vp_w / msi_w * msi_h, 1.0 / msi_ar);
		bool blending = false;
		int count = 0;
		int found = 0;
		
		// This double loop iterate over the displayed part of the image and find all (i,j) being top-left corners of tiles
		for (guint64 i = (guint64) minx; i < from_layer2 && i * v_tile_w < maxx; i++) {
			for (guint64 j = (guint64) miny; j < from_layer2 && j * v_tile_h < maxy; j++) {
				QTreeTile *tile = qtree_lookup_tile (subimage_cache, from_layer, i, j);
				
				count++;
				
				if (tile && tile->image) {
					if (tile->opacity > fade)
						blending = true;
					
					found++;
				}
			}
		}
		
		if (found > 0 && to_layer < from_layer)
			to_layer = from_layer;
		
		if (found == count && (!blending || from_layer == 0))
			break;
		
		from_layer --;
	}

	//render here
	//cairo_push_group (cr);
	
	cairo_save (cr);
	cairo_matrix_t render_xform;
	cairo_matrix_init_identity (&render_xform);
	cairo_matrix_scale (&render_xform, msi_w / vp_w, msi_w / vp_w);
	cairo_matrix_translate (&render_xform, -vp_ox, -vp_oy);
	cairo_matrix_scale (&render_xform, 1.0 / im_w, 1.0 / im_w);

	/*
	 * here we want to clip to the bounds of the image to ensure we don't
	 * have scaling artifacts on the edges but the image potentially has bounds
	 * larger that cairo can handle right now so we transform the image
	 * bounds to the viewport coordinate space and do intersection and clipping
	 * there to work around the cairo coordinate limitations.
	 */
	Rect vp_bounds (0, 0, msi_w, msi_h);
	Rect im_bounds (0, 0, im_w, im_h);
	im_bounds = im_bounds.Transform (&render_xform);
	Rect render_region = vp_bounds.Intersection (im_bounds);
	render_region.Draw (cr);
	cairo_clip (cr);

	cairo_transform (cr, &render_xform);

	LOG_MSI ("rendering layers from %d to %d\n", from_layer, to_layer);
	
	int layer_to_render = MAX (0, from_layer);
	while (layer_to_render <= to_layer) {
		guint64 layers2 = pow2 (layers - layer_to_render);
		guint64 render_layer2 = pow2 (layer_to_render);
		double v_scale = (double) layers2 / im_w;
		double v_tile_w = tile_width * v_scale;
		double v_tile_h = tile_height * v_scale;
		double minx = MAX (0, (vp_ox / v_tile_w));
		double maxx = MIN (vp_ox + vp_w, 1.0);
		double miny = MAX (0, (vp_oy / v_tile_h));
		double maxy = MIN (vp_oy + vp_w / msi_w * msi_h, 1.0 / msi_ar);
		
		for (guint64 i = (guint64) minx; i < render_layer2 && i * v_tile_w < maxx; i++) {
			for (guint64 j = (guint64) miny; j < render_layer2 && j * v_tile_h < maxy; j++) {
				QTreeTile *tile = qtree_lookup_tile (subimage_cache, layer_to_render, i, j);
				
				if (!tile || !tile->image)
					continue;
				
				LOG_MSI ("rendering %d %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT "\n", layer_to_render, i, j);
				cairo_save (cr);
				
				// scale to image size
				cairo_scale (cr, layers2, layers2);
				
				cairo_translate (cr, i * tile_width, j * tile_height);
				
				cairo_set_source_surface (cr, tile->image, 0, 0);
				
				double combined = 1.0;
				
				if (tile->opacity > fade)
					combined = MIN (1.0 - tile->opacity + fade, 1.0);
				
				if (IS_TRANSLUCENT (combined))
					cairo_paint_with_alpha (cr, combined);
				else
					cairo_paint (cr);
				
				cairo_restore (cr);
			}
		}
		
		layer_to_render++;
	}
	
	cairo_restore (cr);
	//cairo_pop_group_to_source (cr);
	
	if (!GetAllowDownloading () || !CanDownloadMoreTiles ())
		return;
	
	// Download the next set of tiles...
	while (from_layer < optimal_layer) {
		from_layer++;
		
		guint64 layers2 = pow2 (layers - from_layer);
		double v_scale = (double) layers2 / im_w;
		double v_tile_w = tile_width * v_scale;
		double v_tile_h = tile_height * v_scale;
		double minx = MAX (0, (vp_ox / v_tile_w));
		double maxx = MIN (vp_ox + vp_w, 1.0);
		double miny = MAX (0, (vp_oy / v_tile_h));
		double maxy = MIN (vp_oy + vp_w / msi_w * msi_h, 1.0 / msi_ar);
		QTree *node;
		
		for (int i = (int) minx; i * v_tile_w < maxx; i++) {
			if (!CanDownloadMoreTiles ())
				return;
			
			for (int j = (int) miny; j * v_tile_h < maxy; j++) {
				if (!CanDownloadMoreTiles ())
					return;
				
				if (!(node = qtree_insert (subimage_cache, from_layer, i, j)))
					continue;
				
				if (!qtree_has_tile (node)) {
					Uri *tile = NULL;
					
					if (source->get_tile_func (from_layer, i, j, &tile, source))
						DownloadTile (tile, node);
					else
						qtree_set_tile (node, NULL, 0.0);
					
					delete tile;
				}
			}
		}
	}
}

void
MultiScaleImage::uri_source_changed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((MultiScaleImage *) closure)->UriSourceChanged ();
}

void
MultiScaleImage::UriSourceChanged ()
{
	DeepZoomImageTileSource *dzits = NULL;
	MultiScaleTileSource *source;
	
	// Abort all downloaders
	StopDownloading ();
	
	if ((source = GetSource ())) {
		if (source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
			dzits = (DeepZoomImageTileSource *) source;
			dzits->Download ();
		} else {
			EmitImageOpenSucceeded ();
			EmitMotionFinished ();
		}
	}
	
	// Reset the viewport
	ClearValue (MultiScaleImage::AnimatedViewportOriginProperty, true);
	ClearValue (MultiScaleImage::AnimatedViewportWidthProperty, true);
	//ClearValue (MultiScaleImage::ViewportOriginProperty, true);
	//ClearValue (MultiScaleImage::ViewportWidthProperty, true);
	
	// Invalidate the whole cache
	if (cache) {
		g_hash_table_destroy (cache);
		cache = g_hash_table_new_full (g_int_hash, g_int_equal, int_free, (GDestroyNotify) qtree_destroy);
	}
	
	// Reset the subimages
	GetSubImages()->Clear ();
	
	Invalidate ();
}

void
MultiScaleImage::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::MULTISCALEIMAGE) {
		MediaBase::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == MultiScaleImage::AnimatedViewportOriginProperty ||
	    args->GetId () == MultiScaleImage::AnimatedViewportWidthProperty) {
		if (GetUseSprings () && HasHandlers (MultiScaleImage::ViewportChangedEvent))
			Emit (MultiScaleImage::ViewportChangedEvent);
		
		Invalidate ();
	} else if (args->GetId () == MultiScaleImage::AllowDownloadingProperty) {
		if (!args->GetNewValue ()->AsBool ())
			StopDownloading ();
		
		Invalidate ();
	} else if (args->GetId () == MultiScaleImage::AspectRatioProperty) {
		InvalidateMeasure ();
		Invalidate ();
	} else if (args->GetId () == MultiScaleImage::BlurFactorProperty) {
		Invalidate ();
	} else if (args->GetId () == MultiScaleImage::ViewportOriginProperty) {
		Point *origin = args->GetNewValue ()->AsPoint ();
		pan_target = Point (origin->x, origin->y);
		AnimateViewportOrigin (origin);
	} else if (args->GetId () == MultiScaleImage::ViewportWidthProperty) {
		zoom_target = args->GetNewValue ()->AsDouble ();
		AnimateViewportWidth (zoom_target);
	} else if (args->GetId () == MultiScaleImage::TileFadeProperty) {
		// There's 2 options here,
		//  - loop all the tiles, update their opacity, and only invalidate a subregion
		//  - Invalidate all, and compute the new opacity on the tiles that needs to be rendered.
		// Both options are unfortunately quite expensive :(
		Invalidate ();
	} else if (args->GetId () == MultiScaleImage::SourceProperty) {
		// disconnect event handlers from the old source
		if (args->GetOldValue ())
			DisconnectSourceEvents (args->GetOldValue ()->AsMultiScaleTileSource ());
		
		// connect event handlers to the new source
		if (args->GetNewValue ())
			ConnectSourceEvents (args->GetNewValue ()->AsMultiScaleTileSource ());
		
		UriSourceChanged ();
	} else if (args->GetId () == MultiScaleImage::UseSpringsProperty) {
		if (!args->GetNewValue()->AsBool ()) {
			if (zoom_sb) {
				double endpoint = zoom_target;
				zoom_sb->StopWithError (NULL);
				zoom_target = NAN;
				SetAnimatedViewportWidth (endpoint);
			}
			
			if (pan_sb) {
				Point endpoint = pan_target;
				pan_sb->StopWithError (NULL);
				pan_target = Point (NAN, NAN);
				SetAnimatedViewportOrigin (&endpoint);
			}
			
			Invalidate ();
		}
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

void
MultiScaleImage::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	MultiScaleSubImage *subimage;
	int count;
	
	if (!PropertyHasValueNoAutoCreate (MultiScaleImage::SubImagesProperty, col)) {
		MediaBase::OnCollectionChanged (col, args);
		return;
	}
	
	switch (args->GetChangedAction ()) {
	case CollectionChangedActionReplace:
		// Disconnect events from the replaced item
		subimage = args->GetOldItem ()->AsMultiScaleSubImage ();
		DisconnectSubImageEvents (subimage);
		// now fall thru to Add
	case CollectionChangedActionAdd:
		// Connect to events on the new item
		subimage = args->GetNewItem ()->AsMultiScaleSubImage ();
		ConnectSubImageEvents (subimage);
		break;
	case CollectionChangedActionRemove:
		// Disconnect events from the removed item
		subimage = args->GetOldItem ()->AsMultiScaleSubImage ();
		DisconnectSubImageEvents (subimage);
		break;
	case CollectionChangedActionClearing:
		// Disconnect events from all subitems
		count = col->GetCount ();
		for (int i = 0; i < count; i++) {
			subimage = col->GetValueAt (i)->AsMultiScaleSubImage ();
			DisconnectSubImageEvents (subimage);
		}
		break;
	case CollectionChangedActionCleared:
		// no-op
		break;
	}
	
	subimages_sorted = false;
	Invalidate ();
}

void
MultiScaleImage::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (args->GetId () == MultiScaleSubImage::ViewportWidthProperty)
		Invalidate ();
	if (args->GetId () == MultiScaleSubImage::ViewportOriginProperty)
		Invalidate ();
	if (args->GetId () == MultiScaleSubImage::ZIndexProperty) {
		subimages_sorted = false;
		Invalidate ();
	}
}

void
MultiScaleImage::EmitImageFailed ()
{
	LOG_MSI ("MSI::Emitting image failed\n");
	
	if (HasHandlers (MultiScaleImage::ImageFailedEvent))
		Emit (MultiScaleImage::ImageFailedEvent);
}

void
MultiScaleImage::EmitImageOpenFailed ()
{
	LOG_MSI ("MSI::Emitting image open failed\n");
	
	if (HasHandlers (MultiScaleImage::ImageOpenFailedEvent)) {
		MoonError moon_error;
		
		MoonError::FillIn (&moon_error, MoonError::EXCEPTION, -2147467259, "");
		Emit (MultiScaleImage::ImageOpenFailedEvent, new ErrorEventArgs (this, UnknownError, moon_error));
	}
}

void
MultiScaleImage::EmitImageOpenSucceeded ()
{
	LOG_MSI ("\nMSI::Emitting open suceeded\n");
	
	if (HasHandlers (MultiScaleImage::ImageOpenSucceededEvent))
		Emit (MultiScaleImage::ImageOpenSucceededEvent);
	
	// This is a hack that removes at least one timeout (#291),
	// possibly because an invalidation gets lost somehow.
	// Since we only start downloading when we try to
	// render the msi, the test effectively hangs.
	FullInvalidate (true);
}

void
MultiScaleImage::EmitMotionFinished ()
{
	LOG_MSI ("Emitting MotionFinished\n");
	
	motion &= ~MOTION_IS_FINISHED;
	
	if (HasHandlers (MultiScaleImage::MotionFinishedEvent))
		Emit (MultiScaleImage::MotionFinishedEvent);
}

void
MultiScaleImage::MotionFinished ()
{
	SetIsIdle (n_downloading == 0);
	EmitMotionFinished ();
}

static void
motion_finished (MultiScaleImage *msi)
{
	msi->MotionFinished ();
}

Point *
MultiScaleImage::GetPanAnimationEndPoint ()
{
	return pan_animation->GetKeyFrames ()->GetValueAt (0)->AsSplinePointKeyFrame ()->GetValue ();
}

void
MultiScaleImage::SetPanAnimationEndPoint (Point value)
{
	pan_animation->GetKeyFrames ()->GetValueAt (0)->AsSplinePointKeyFrame ()->SetValue (value);
}

double
MultiScaleImage::GetZoomAnimationEndPoint ()
{
	return zoom_animation->GetKeyFrames ()->GetValueAt (0)->AsSplineDoubleKeyFrame ()->GetValue ();
}

void
MultiScaleImage::SetZoomAnimationEndPoint (double value)
{
	zoom_animation->GetKeyFrames ()->GetValueAt (0)->AsSplineDoubleKeyFrame ()->SetValue (value);
}

void
MultiScaleImage::AnimateViewportWidth (double width)
{
	if (!GetUseSprings ()) {
		if (motion == 0) {
			//printf ("SetAnimatedViewportWidth(): queueing MotionFinished\n");
			AddTickCall ((TickCallHandler) motion_finished);
			motion = MOTION_IS_FINISHED;
		}
		
		SetAnimatedViewportWidth (width);
		return;
	}
	
	if (!zoom_sb) {
		zoom_sb = MoonUnmanagedFactory::CreateStoryboard ();
		zoom_sb->SetManualTargetWithError (this, NULL);
		zoom_sb->SetTargetProperty (zoom_sb, new PropertyPath ("(MultiScaleImage.AnimatedViewportWidth)"));
		zoom_sb->AddHandler (Storyboard::CompletedEvent, zoom_finished, this);
		zoom_animation = MoonUnmanagedFactory::CreateDoubleAnimationUsingKeyFrames ();
		zoom_animation->SetDuration (Duration::FromSeconds (4));
		zoom_animation->SetKeyFrames (DOPtr<DoubleKeyFrameCollection> (MoonUnmanagedFactory::CreateDoubleKeyFrameCollection ()));
		DOPtr<SplineDoubleKeyFrame> keyframe (MoonUnmanagedFactory::CreateSplineDoubleKeyFrame ());
		keyframe->SetKeySpline (DOPtr<KeySpline> (new KeySpline (.05, .5, 0, 1.0)));
		keyframe->SetKeyTime (KeyTime::FromPercent (1.0));
		zoom_animation->GetKeyFrames ()->Add (static_cast<SplineDoubleKeyFrame*>(keyframe));

		DOPtr<TimelineCollection> tlc (new TimelineCollection ());
		tlc->Add (static_cast<DoubleAnimationUsingKeyFrames*>(zoom_animation));
		zoom_sb->SetChildren(tlc);
#if DEBUG
		zoom_sb->SetName ("Multiscale Zoom");
#endif
	} else {
		zoom_sb->PauseWithError (NULL);
		motion &= ~MOTION_IS_ZOOMING;
	}

	LOG_MSI ("animating zoom from %f to %f\n\n", GetAnimatedViewportWidth (), width);
	
	SetZoomAnimationEndPoint (width);
	
	if (zoom_sb->BeginWithError (NULL))
		motion |= MOTION_IS_ZOOMING;
	
	UpdateIdleStatus ();
}

void
MultiScaleImage::AnimateViewportOrigin (Point *origin)
{
	if (!GetUseSprings ()) {
		if (motion == 0) {
			//printf ("SetAnimatedViewportOrigin(): queueing MotionFinished\n");
			AddTickCall ((TickCallHandler) motion_finished);
			motion = MOTION_IS_FINISHED;
		}
		
		SetAnimatedViewportOrigin (origin);
		return;
	}
	
	if (!pan_sb) {
		pan_sb = MoonUnmanagedFactory::CreateStoryboard ();
		pan_sb->SetManualTargetWithError (this, NULL);
		pan_sb->SetTargetProperty (pan_sb, new PropertyPath ("(MultiScaleImage.AnimatedViewportOrigin)"));
		pan_sb->AddHandler (Storyboard::CompletedEvent, pan_finished, this);
		pan_animation = MoonUnmanagedFactory::CreatePointAnimationUsingKeyFrames ();
		pan_animation->SetDuration (Duration::FromSeconds (4));
		pan_animation->SetKeyFrames (DOPtr<PointKeyFrameCollection> (MoonUnmanagedFactory::CreatePointKeyFrameCollection ()));
		SplinePointKeyFrame *keyframe = MoonUnmanagedFactory::CreateSplinePointKeyFrame ();
		keyframe->SetKeySpline (DOPtr<KeySpline> (new KeySpline (.05, .5, 0, 1.0)));
		keyframe->SetKeyTime (KeyTime::FromPercent (1.0));
		pan_animation->GetKeyFrames ()->Add (keyframe);
		keyframe->unref ();

		TimelineCollection *tlc = new TimelineCollection ();
		tlc->Add (static_cast<PointAnimationUsingKeyFrames*> (pan_animation));
		pan_sb->SetChildren(tlc);
		tlc->unref ();
#if DEBUG
		pan_sb->SetName ("Multiscale Pan");
#endif
	} else {
		pan_sb->PauseWithError (NULL);
		motion &= ~MOTION_IS_PANNING;
	}
	
	SetPanAnimationEndPoint (*origin);
	
	if (pan_sb->BeginWithError (NULL))
		motion |= MOTION_IS_PANNING;
	
	UpdateIdleStatus ();
}

void
MultiScaleImage::SetIsIdle (bool value)
{
	SetValue (MultiScaleImage::IsIdleProperty, Value (value));
}

void
MultiScaleImage::SetIsDownloading (bool value)
{
	SetValue (MultiScaleImage::IsDownloadingProperty, Value (value));
}

void
MultiScaleImage::UpdateIdleStatus ()
{
	SetIsIdle (n_downloading == 0 && !IS_IN_MOTION (motion));
	SetIsDownloading (n_downloading > 0);
}

int
MultiScaleImage::LogicalToElementX (int x, int y)
{
	return LogicalToElementPoint (Point (x, y)).x;
}

int
MultiScaleImage::LogicalToElementY (int x, int y)
{
	return LogicalToElementPoint (Point (x, y)).y;
}

MultiScaleSubImage *
MultiScaleImage::GetIthSubImage (int index)
{
	MultiScaleSubImageCollection *sub_images = GetSubImages ();
	Value *value;
	
	if (sub_images == NULL)
		return NULL;
	
	value = sub_images->GetValueAt (index);
	
	if (value == NULL)
		return NULL;
			
	return value->AsMultiScaleSubImage ();
}

int
MultiScaleImage::GetSubImageCount ()
{
	MultiScaleSubImageCollection *sub_images = GetSubImages ();
	
	if (sub_images == NULL)
		return 0;
	return sub_images->GetCount ();
}

void
MultiScaleImage::InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer)
{
	if (GetSource ()->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
		g_warning ("calling InvalidateTileLayer on DeepZoom Images makes no sense\n");
		return;
	}
	
	StopDownloading ();

	int index = -1;
	QTree *subimage_cache = (QTree *) g_hash_table_lookup (cache, &index);
	if (subimage_cache)
		qtree_remove_at (subimage_cache, level, tilePositionX, tilePositionY, 0);

	Invalidate ();
}

};
