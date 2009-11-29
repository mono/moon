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

//TODO
//
//- only invalidate regions
//- only render changed regions

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
#include "file-downloader.h"
#include "multiscalesubimage.h"
#include "bitmapimage.h"
#include "ptr.h"

#if LOGGING
#include "clock.h"
#define MSI_STARTTIMER(id)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MSI)) TimeSpan id##_t_start = get_now()
#define MSI_ENDTIMER(id,str)		if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MSI)) TimeSpan id##_t_end = get_now(); printf ("timing of '%s' ended took (%f ms)\n", str, id##_t_end, (double)(id##_t_end - id##_t_start) / 10000)
#else
#define STATTIMER(id)
#define ENDTIMER(id,str)
#endif

inline
guint64 pow2(int pow) {
	return ((guint64) 1 << pow);
}

/*
 * Q(uad)Tree.
 */

struct QTree {
	bool has_value;
	void *data;
	QTree* l0; //N-E
	QTree* l1; //N-W
	QTree* l2; //S-E
	QTree* l3; //S-W
	QTree* parent;
};

static QTree*
qtree_new (void)
{
	return g_new0 (QTree, 1);
}

static QTree*
qtree_insert (QTree* root, int level, guint64 x, guint64 y)
{
	if (x >= (pow2 (level)) || y >= (pow2 (level))) {
#if DEBUG
		abort ();
#endif
		g_warning ("QuadTree index out of range.");
		return NULL;
	}

	if (!root) {
		g_warning ("passing a NULL QTree to qtree_insert");
		return NULL;
	}

	QTree *node = root;
	while (level-- > 0) {
		if (y < (pow2 (level))) {
			if (x < (pow2 (level))) {
				if (!node->l0) {
					node->l0 = g_new0 (QTree, 1);
					node->l0->parent = node;
				}
				node = node->l0;
			} else {
				if (!node->l1) {
					node->l1 = g_new0 (QTree, 1);
					node->l1->parent = node;
				}
				node = node->l1;
				x -= (pow2 (level));
			}
		} else {
			if (x < (pow2 (level))) {
				if (!node->l2) {
					node->l2 = g_new0 (QTree, 1);
					node->l2->parent = node;
				}
				node = node->l2;
				y -= (pow2 (level));
			} else {
				if (!node->l3) {
					node->l3 = g_new0 (QTree, 1);
					node->l3->parent = node;
				}
				node = node->l3;
				x -= (pow2 (level));
				y -= (pow2 (level));
			}
		}
	}
	return node;
}

static void
qtree_set_value (QTree* node, void *data)
{
	//FIXME: the destroy method should be a ctor argument
	if (node->has_value && node->data)
		cairo_surface_destroy ((cairo_surface_t*)node->data);

	node->has_value = true;
	node->data = data;	
}

static QTree *
qtree_lookup (QTree* root, int level, guint64 x, guint64 y)
{
	if (x >= (pow2 (level)) || y >= (pow2 (level))) {
#if DEBUG
 		// we seem to run into an infinite loop sporadically here for drt #2014 completely spamming the test output.
 		// abort to get a stack trace. 
		abort ();
#endif
		g_warning ("QuadTree index out of range.");
		return NULL;
	}

	while (level-- > 0) {
		if (!root)
			return NULL;

		if (y < (pow2 (level))) {
			if (x < (pow2 (level))) {
				root = root->l0;
			} else {
				root = root->l1;
				x -= (pow2 (level));
			}
		} else {
			if (x < (pow2 (level))) {
				root = root->l2;
				y -= (pow2 (level));
			} else {
				root = root->l3;
				x -= (pow2 (level));
				y -= (pow2 (level));
			}
		}
	}
	return root;
}

static void *
qtree_lookup_data (QTree* root, int level, guint64 x, guint64 y)
{
	QTree *node = qtree_lookup (root, level, x, y);
	if (node && node->has_value)
		return node->data;
	return NULL;
}

//FIXME: merge qtree_next_sibling and _qtree_next_sibling in a single
//function, with an elegant loop to avoid recursion.
static QTree*
_qtree_next_sibling (QTree *node, guint64 *i, guint64 *j, int l)
{
	if (!node) {
#if DEBUG
		abort ();
#endif
		g_warning ("Empty node");
		return NULL;
	}

	if (!node->parent) //no parent, we're probably at the root
		return NULL;
	
	if (node == node->parent->l0) {
		*i += pow2 (l);
		return node->parent->l1;
	}
	if (node == node->parent->l1) {
		*i -= pow2 (l);
		*j += pow2 (l);
		return node->parent->l2;
	}
	if (node == node->parent->l2) {
		*i += pow2 (l);
		return node->parent->l3;
	}
	if (node == node->parent->l3) {
		*i -= pow2 (l);
		*j -= pow2 (l);
		QTree *next_parent = _qtree_next_sibling (node->parent, i, j, l + 1);
		if (!next_parent)
			return NULL;
		return next_parent->l0;
	}
#if DEBUG
	abort ();
#endif
	g_warning ("Broken parent link, this is bad");
	return NULL;
}

static void
qtree_remove (QTree* node, int depth)
{
	if (node && node->has_value) {
		node->has_value = false;
		if (node->data) {
			cairo_surface_destroy ((cairo_surface_t*)node->data);
			node->data = NULL;
		}
	}	

	if (depth <= 0)
		return;

	qtree_remove (node->l0, depth - 1);
	qtree_remove (node->l1, depth - 1);
	qtree_remove (node->l2, depth - 1);
	qtree_remove (node->l3, depth - 1);

}

static void
qtree_remove_at (QTree* root, int level, guint64 x, guint64 y, int depth)
{
	QTree* node = qtree_lookup (root, level, x, y);
	qtree_remove (node, depth);
}

static inline bool
qtree_has_value (QTree* node)
{
	return node->has_value;	
}

static void
qtree_destroy (QTree *root)
{
	if (!root)
		return;

	//FIXME: the destroy func should be a qtree ctor option
	if (root->data) {
		cairo_surface_destroy ((cairo_surface_t*)(root->data));
		root->data = NULL;
	}

	qtree_destroy (root->l0);
	qtree_destroy (root->l1);
	qtree_destroy (root->l2);
	qtree_destroy (root->l3);
	g_free (root);
}

/*
 * BitmapImageContext
 */

enum BitmapImageStatus {
	BitmapImageFree = 0,
	BitmapImageBusy,
	BitmapImageDone
};

struct BitmapImageContext
{
	BitmapImageStatus state;
	BitmapImage *bitmapimage;
	QTree *node;
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


/*
 * MultiScaleImage
 */

MultiScaleImage::MultiScaleImage () :
	subimages_sorted(false),
	pending_motion_completed(false),
	bitmapimages(NULL),
	is_fading(false),
	is_zooming(false),
	is_panning(false)
{
//	static bool init = true;
//	if (init) {
//		init = false;
//		MultiScaleImage::SubImagesProperty->SetValueValidator (MultiScaleSubImageCollectionValidator);	
//	}
	providers [PropertyPrecedence_DynamicValue] = new MultiScaleImagePropertyValueProvider (this, PropertyPrecedence_DynamicValue);

	SetObjectType (Type::MULTISCALEIMAGE); 
	cache = g_hash_table_new_full (g_int_hash, g_int_equal, g_free, (GDestroyNotify)qtree_destroy);
}

MultiScaleImage::~MultiScaleImage ()
{
	StopDownloading ();
	if (cache)
		g_hash_table_destroy (cache);
	cache = NULL;
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
		viewport_width = GetViewportWidth ();
		viewport_origin = *GetViewportOrigin ();
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
	Point *vp_origin = GetViewportOrigin ();
	double vp_width = GetViewportWidth ();
	double actual_width = GetActualWidth ();
	return Point (vp_origin->x + (double)elementPoint.x * vp_width / actual_width,
		      vp_origin->y + (double)elementPoint.y * vp_width / actual_width);
}

Point
MultiScaleImage::LogicalToElementPoint (Point logicalPoint)
{
	Point *vp_origin = GetViewportOrigin ();
	double vp_width = GetViewportWidth ();
	double actual_width = GetActualWidth ();
	return Point ((logicalPoint.x - vp_origin->x) * actual_width / vp_width,
		      (logicalPoint.y - vp_origin->y) * actual_width / vp_width);
}

void
MultiScaleImage::DownloadTile (BitmapImageContext *bictx, Uri *tile, void *user_data)
{
	GList *list;
	BitmapImageContext *ctx;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next) {
		if (ctx->state != BitmapImageFree && ctx->bitmapimage->GetUriSource()->operator==(*tile)) {
			//LOG_MSI ("Tile %s is already being downloaded\n", tile->ToString ());
			return;
		}
	}

	//LOG_MSI ("downloading tile %s\n", tile->ToString ());

	bictx->state = BitmapImageBusy;
	bictx->node = (QTree *)user_data;
	bictx->retry = 0;
	SetIsDownloading (true);
	bictx->bitmapimage->SetDownloadPolicy (MsiPolicy);
	bictx->bitmapimage->SetUriSource (tile);
}

//Only used for DeepZoom sources
void
MultiScaleImage::HandleDzParsed ()
{
	//if the source is a collection, fill the subimages list
	MultiScaleTileSource *source = GetSource ();
	MultiScaleSubImageCollection *subs = GetSubImages ();

	if (source->GetImageWidth () >= 0 && source->GetImageHeight () >= 0)
		SetValue (MultiScaleImage::AspectRatioProperty, Value ((double)source->GetImageWidth () / (double)source->GetImageHeight ()));

	DeepZoomImageTileSource *dsource;
       
	if (source->Is (Type::DEEPZOOMIMAGETILESOURCE) &&
	    (dsource = (DeepZoomImageTileSource *)source)) {
		int i;
		MultiScaleSubImage *si;
		for (i = 0; (si = (MultiScaleSubImage*)g_list_nth_data (dsource->subimages, i)); i++) {
			if (!subs)
				SetValue (MultiScaleImage::SubImagesProperty, new MultiScaleSubImageCollection ());

			subs->Add (si);
		}
	}
	Invalidate ();

	//Get the first tiles
	BitmapImageContext *bitmapimagectx;

	int shared_index = -1;

	QTree *shared_cache = (QTree*)g_hash_table_lookup (cache, &shared_index);
	if (!shared_cache)
		g_hash_table_insert (cache, new int(shared_index), (shared_cache = qtree_new ()));

	int layer = 0;
	while ((bitmapimagectx = GetFreeBitmapImageContext ())) {
		Uri *tile = new Uri ();
		if (source->get_tile_func (layer, 0, 0, tile, source)) {
			QTree *node = qtree_insert (shared_cache, layer, 0, 0);
			DownloadTile (bitmapimagectx, tile, node);
		}
		delete tile;
		layer ++;
	}

	EmitImageOpenSucceeded ();
}

static void
fade_finished (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->FadeFinished ();	
}

void
MultiScaleImage::FadeFinished ()
{
	is_fading = false;
	if (!is_fading && !is_zooming && !is_panning)
		EmitMotionFinished ();
}

static void
zoom_finished (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->ZoomFinished ();		
}

void
MultiScaleImage::ZoomFinished ()
{
	is_zooming = false;
	if (!is_fading && !is_zooming && !is_panning)
		EmitMotionFinished ();
}

static void
pan_finished (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->PanFinished ();		
}

void
MultiScaleImage::PanFinished ()
{
	is_panning = false;
	if (!is_fading && !is_zooming && !is_panning)
		EmitMotionFinished ();
}

void
tile_available (EventObject *sender, EventArgs *calldata, gpointer closure)
{
//	LOG_MSI ("Tile downloaded %s\n", ((BitmapImage *)sender)->GetUriSource ()->ToString ());
	((MultiScaleImage *)closure)->TileOpened ((BitmapImage *)sender);
}

void
MultiScaleImage::TileOpened (BitmapImage *bitmapimage)
{
	BitmapImageContext *ctx = GetBitmapImageContext (bitmapimage);
	ctx->state = BitmapImageDone;
	GList *list;
	bool is_downloading = false;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next)
		is_downloading |= (ctx->state == BitmapImageBusy);
	SetIsDownloading (is_downloading);
	Invalidate ();
}

void
tile_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
//	LOG_MSI ("Failed to download tile %s\n", ((BitmapImage *)sender)->GetUriSource ()->ToString ());
	((MultiScaleImage *)closure)->TileFailed ((BitmapImage *)sender);
}

void
MultiScaleImage::TileFailed (BitmapImage *bitmapimage)
{
	BitmapImageContext *ctx = GetBitmapImageContext (bitmapimage);
	if (ctx->retry < 5) {
		bitmapimage->SetUriSource (bitmapimage->GetUriSource ());
		ctx->retry = ctx->retry + 1;
	} else {
		ctx->state = BitmapImageFree;

		LOG_MSI ("caching a NULL for %s\n", ctx->bitmapimage->GetUriSource()->ToString ());
		qtree_set_value (ctx->node, NULL);

		GList *list;
		bool is_downloading = false;
		for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next)
			is_downloading |= (ctx->state == BitmapImageBusy);
		SetIsDownloading (is_downloading);
	}
	Invalidate ();
	EmitImageFailed ();
}

BitmapImageContext *
MultiScaleImage::GetBitmapImageContext (BitmapImage *bitmapimage)
{
	BitmapImageContext *ctx;
	GList *list;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next)
		if (ctx->bitmapimage == bitmapimage)
			return ctx;
	return NULL;
}

void
MultiScaleImage::StopDownloading ()
{
	BitmapImageContext *ctx;
	GList *list;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next) {
		ctx->bitmapimage->Abort ();
		ctx->bitmapimage->Dispose ();
		ctx->bitmapimage->unref ();
		ctx->state = BitmapImageFree;
		delete ctx;
	}	

	if (bitmapimages)
		g_list_free (bitmapimages);
	bitmapimages = NULL;
}

BitmapImageContext *
MultiScaleImage::GetFreeBitmapImageContext ()
{
	guint num_dl = 6;
	BitmapImageContext *ctx;
	GList *list;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next)
		if (ctx->state == BitmapImageFree)
			return ctx;

	if (g_list_length (bitmapimages) < num_dl) {
		ctx = new BitmapImageContext ();
		ctx->state = BitmapImageFree;
		ctx->bitmapimage = new BitmapImage ();
		ctx->bitmapimage->AddHandler (ctx->bitmapimage->ImageOpenedEvent, tile_available, this);
		ctx->bitmapimage->AddHandler (ctx->bitmapimage->ImageFailedEvent, tile_failed, this);
		bitmapimages = g_list_append (bitmapimages, ctx);
		return ctx;
	}

	return NULL;
}

void
MultiScaleImage::Render (cairo_t *cr, Region *region, bool path_only)
{
	LOG_MSI ("MSI::Render\n");

	MultiScaleTileSource *source = GetSource ();
	if (!source) {
		LOG_MSI ("no sources set, nothing to render\n");
		return;	
	}

	//Process the downloaded tile
	GList *list;
	BitmapImageContext *ctx;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next) {
		cairo_surface_t *surface;

		if (ctx->state != BitmapImageDone || !(surface = cairo_surface_reference (ctx->bitmapimage->GetSurface (cr))))
			continue;

//		Uri *tile = ctx->bitmapimage->GetUriSource ();
		cairo_surface_set_user_data (surface, &width_key, new int (ctx->bitmapimage->GetPixelWidth ()), g_free);
		cairo_surface_set_user_data (surface, &height_key, new int (ctx->bitmapimage->GetPixelHeight ()), g_free);

		if (!fadein_sb) {
			fadein_sb = new Storyboard ();
			fadein_sb->SetManualTarget (this);
			fadein_sb->SetTargetProperty (fadein_sb, new PropertyPath ("(MultiScaleImage.TileFade)"));
			fadein_animation = new DoubleAnimation ();
			fadein_animation->SetDuration (Duration (source->GetTileBlendTime ()));
			TimelineCollection *tlc = new TimelineCollection ();
			tlc->Add (static_cast<DoubleAnimation*> (fadein_animation));
			fadein_sb->SetChildren(tlc);
			fadein_sb->AddHandler (Storyboard::CompletedEvent, fade_finished, this);
#if DEBUG
			fadein_sb->SetName ("Multiscale Fade-In");
#endif
		} else {
			fadein_sb->PauseWithError (NULL);
		}

		//LOG_MSI ("animating Fade from %f to %f\n\n", GetValue(MultiScaleImage::TileFadeProperty)->AsDouble(), GetValue(MultiScaleImage::TileFadeProperty)->AsDouble() + 0.9);
		double *to = new double (GetValue(MultiScaleImage::TileFadeProperty)->AsDouble() + 0.9);
		fadein_animation->SetFrom (GetValue(MultiScaleImage::TileFadeProperty)->AsDouble());
		fadein_animation->SetTo (*to);

		is_fading = true;

		fadein_sb->BeginWithError(NULL);

		cairo_surface_set_user_data (surface, &full_opacity_at_key, to, g_free);
		LOG_MSI ("caching %s\n", ctx->bitmapimage->GetUriSource()->ToString ());
		qtree_set_value (ctx->node, surface);

		ctx->bitmapimage->SetUriSource (NULL);
		ctx->state = BitmapImageFree;
	}

	bool is_collection = source &&
			     source->Is (Type::DEEPZOOMIMAGETILESOURCE) &&
			     ((DeepZoomImageTileSource *)source)->IsCollection () &&
			     GetSubImages ();

	if (source->GetImageWidth () < 0 && !is_collection) {
		LOG_MSI ("nothing to render so far...\n");
		if (source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
			((DeepZoomImageTileSource*)source)->set_callbacks (multi_scale_image_handle_dz_parsed, multi_scale_image_emit_image_open_failed, multi_scale_image_on_source_property_changed, this);
			((DeepZoomImageTileSource*)source)->Download ();
		}
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
}

void
MultiScaleImage::RenderCollection (cairo_t *cr, Region *region)
{
	LOG_MSI ("\nMSI::RenderCollection\n");

	double msi_w = GetActualWidth ();
	double msi_h = GetActualHeight ();
	double msi_ar = GetAspectRatio();
	double msivp_ox = GetViewportOrigin()->x;
	double msivp_oy = GetViewportOrigin()->y;
	double msivp_w = GetViewportWidth();
	
	if (!GetSource ()->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
		g_warning ("RenderCollection called for a non deepzoom tile source. this should not happen");
		return;
	}
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *)GetSource ();

	Rect viewport = Rect (msivp_ox, msivp_oy, msivp_w, msivp_w/msi_ar);

	MultiScaleSubImageCollection *subs = GetSubImages ();
	if (!subimages_sorted) {
		subs->ResortByZIndex ();
		subimages_sorted = true;
	}

	//using the "-1" index for the shared cache
	int shared_index = -1;
	QTree *shared_cache = (QTree*)g_hash_table_lookup (cache, &shared_index);
	if (!shared_cache)
		g_hash_table_insert (cache, new int(shared_index), (shared_cache = qtree_new ()));

	int i;
	for (i = 0; i < subs->GetCount (); i++) {
		MultiScaleSubImage *sub_image = (MultiScaleSubImage*)g_ptr_array_index (subs->z_sorted, i);

		int index = sub_image->GetId();
		QTree *subimage_cache = (QTree*)g_hash_table_lookup (cache, &index);
		if (!subimage_cache)
			g_hash_table_insert (cache, new int(index), (subimage_cache = qtree_new ()));

		double subvp_ox = sub_image->GetViewportOrigin()->x;
		double subvp_oy = sub_image->GetViewportOrigin()->y;
		double subvp_w = sub_image->GetViewportWidth();
		double sub_w = sub_image->source->GetImageWidth ();
		double sub_h = sub_image->source->GetImageHeight ();
		double sub_ar = sub_image->GetAspectRatio();


		//expressing the subimage viewport in main viewport coordinates.
		Rect sub_vp = Rect (-subvp_ox / subvp_w, -subvp_oy / subvp_w, 1.0/subvp_w, 1.0/(sub_ar * subvp_w));

		//render only if the subimage viewport intersects with this viewport
		if (!sub_vp.IntersectsWith (viewport))
			continue;
		LOG_MSI ("Intersects with main viewport...rendering\n");

		int layers;
		if (frexp (MAX (sub_w, sub_h), &layers) == 0.5)
			layers --;

		int optimal_layer;
		frexp (msi_w / (subvp_w * msivp_w * MIN (1.0, sub_ar)), &optimal_layer);
		optimal_layer = MIN (optimal_layer, layers);
		LOG_MSI ("number of layers: %d\toptimal layer for this: %d\n", layers, optimal_layer);

		int to_layer = -1;
		int from_layer = optimal_layer;	
		while (from_layer >= 0) {
			int count = 0;
			int found = 0;
			bool blending = FALSE; //means at least a tile is not yet fully blended

			int tile_width = (from_layer > dzits->GetMaxLevel () && ((DeepZoomImageTileSource*)sub_image->source)->IsParsed ()) ? sub_image->source->GetTileWidth () : dzits->GetTileWidth ();
			int tile_height = (from_layer > dzits->GetMaxLevel () && ((DeepZoomImageTileSource*)sub_image->source)->IsParsed ()) ? sub_image->source->GetTileHeight (): dzits->GetTileHeight ();

			//in msi relative coord
			double v_tile_w = tile_width * (double) (pow2 (layers - from_layer)) * sub_vp.width / sub_w;
			double v_tile_h = tile_height * (double)(pow2 (layers - from_layer)) * sub_vp.width / sub_w;
			//LOG_MSI ("virtual tile size at layer %d; %fx%f\n", from_layer, v_tile_w, v_tile_h);

			guint64 i, j;
			for (i = (int)((MAX(msivp_ox, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				for (j = (int)((MAX(msivp_oy, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msivp_oy + msivp_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					count++;
					cairo_surface_t* image = NULL;


					if (from_layer > dzits->GetMaxLevel ()) {
						if ((image = (cairo_surface_t*)qtree_lookup_data (subimage_cache, from_layer, i, j)))
							found ++;
					} else if ((image = (cairo_surface_t*)qtree_lookup_data (shared_cache, from_layer,
									  morton_x (sub_image->n) * (pow2 (from_layer)) / tile_width,
									  morton_y (sub_image->n) * (pow2 (from_layer)) / tile_height)))
						found ++;

					if (image && *(double*)(cairo_surface_get_user_data (image, &full_opacity_at_key)) > GetValue(MultiScaleImage::TileFadeProperty)->AsDouble ())
						blending = TRUE;
				}
			}
			if (found > 0 && to_layer < from_layer)
				to_layer = from_layer;
			if (found == count && (!blending || from_layer == 0))
				break;

			from_layer --;
		}
	
		//render here
		LOG_MSI ("rendering layers from %d to %d\n", from_layer, to_layer);
		double fade = GetValue (MultiScaleImage::TileFadeProperty)->AsDouble();
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
				int tile_width = (from_layer > dzits->GetMaxLevel () && ((DeepZoomImageTileSource*)sub_image->source)->IsParsed ()) ?sub_image->source->GetTileWidth () : dzits->GetTileWidth ();
				int tile_height = (from_layer > dzits->GetMaxLevel () && ((DeepZoomImageTileSource*)sub_image->source)->IsParsed ()) ? sub_image->source->GetTileHeight () : dzits->GetTileHeight ();

				double v_tile_w = tile_width * (double)(pow2 (layers - layer_to_render)) * sub_vp.width / sub_w;
				double v_tile_h = tile_height * (double)(pow2 (layers - layer_to_render)) * sub_vp.width / sub_w;

				int i, j;
				for (i = (int)((MAX(msivp_ox, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
					for (j = (int)((MAX(msivp_oy, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msivp_oy + msivp_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
						cairo_surface_t *image = NULL;
						bool shared_tile = false;
						if (layer_to_render > dzits->GetMaxLevel())
							image = (cairo_surface_t*)qtree_lookup_data (subimage_cache, layer_to_render, i, j);
						else {
							//Check in the shared levels
							shared_tile = true;

							image = (cairo_surface_t*)qtree_lookup_data (shared_cache, layer_to_render,
									morton_x(sub_image->n) * pow2 (layer_to_render) / tile_width,
									morton_y(sub_image->n) * pow2 (layer_to_render) / tile_height);
						}

						if (!image)
							continue;

						LOG_MSI ("rendering subimage %d %d %d %d\n", sub_image->id, layer_to_render, i, j);
						cairo_save (cr);

						cairo_scale (cr,
							     pow2 (layers - layer_to_render),
							     pow2 (layers - layer_to_render));

						cairo_translate (cr,
								 i * tile_width,
								 j * tile_height);

						if (shared_tile) {
							cairo_translate (cr,
									 (int)(-morton_x(sub_image->n) * (pow2 (layer_to_render))) % tile_width,
									 (int)(-morton_y(sub_image->n) * (pow2 (layer_to_render))) % tile_height);

						}

						cairo_set_source_surface (cr, image, 0, 0);
						
						double *opacity = (double*)(cairo_surface_get_user_data (image, &full_opacity_at_key));
						double combined = 1.0;

						if (opacity && *opacity > fade) 
							combined = MIN(1.0 - *opacity + fade, 1.0);

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

		if (!GetAllowDownloading ())
			continue;

		BitmapImageContext *bitmapimagectx;
		if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
			continue;

		//Get the next tiles..
		while (from_layer < optimal_layer) {
			from_layer ++;

			//if the subimage is unparsed, trigger the download
			if (from_layer > dzits->GetMaxLevel () && !((DeepZoomImageTileSource *)sub_image->source)->IsDownloaded () ) {
				((DeepZoomImageTileSource*)sub_image->source)->set_callbacks ((void(*)(MultiScaleImage*))uielement_invalidate, multi_scale_image_emit_image_failed, NULL, this);
				((DeepZoomImageTileSource*)sub_image->source)->Download ();
				break;
			}
			
			int tile_width = (from_layer > dzits->GetMaxLevel () && ((DeepZoomImageTileSource*)sub_image->source)->IsParsed ()) ?sub_image->source->GetTileWidth () : dzits->GetTileWidth ();
			int tile_height = (from_layer > dzits->GetMaxLevel () && ((DeepZoomImageTileSource*)sub_image->source)->IsParsed ()) ? sub_image->source->GetTileHeight (): dzits->GetTileHeight ();

			double v_tile_w = tile_width * (double)(pow2 (layers - from_layer)) * sub_vp.width / sub_w;
			double v_tile_h = tile_height * (double)(pow2 (layers - from_layer)) * sub_vp.width / sub_w;

			int i, j;
			for (i = (int)((MAX(msivp_ox, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
					break;
				for (j = (int)((MAX(msivp_oy, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msivp_oy + msivp_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
						break;
					Uri *tile = new Uri ();
					if (from_layer <= dzits->GetMaxLevel ()) {
						QTree *node = qtree_insert (shared_cache,
									    from_layer,
									    morton_x(sub_image->n) * (pow2 (from_layer)) / tile_width,
									    morton_y(sub_image->n) * (pow2 (from_layer)) / tile_height);
						if (!qtree_has_value (node)
						    && dzits->get_tile_func (from_layer,
									      morton_x(sub_image->n) * (pow2 (from_layer)) / tile_width,
									      morton_y(sub_image->n) * (pow2 (from_layer)) / tile_height,
									      tile, dzits))
							DownloadTile (bitmapimagectx, tile, node);
					} else {
						QTree *node = qtree_insert (subimage_cache, from_layer, i, j);
						if (!qtree_has_value (node)
						    && dzits->get_tile_func (from_layer, i, j, tile, sub_image->source))
							DownloadTile (bitmapimagectx, tile, node);
					}
					delete tile;
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
	double vp_ox = GetViewportOrigin()->x;
	double vp_oy = GetViewportOrigin()->y;
	double vp_w = GetViewportWidth ();

	if (msi_w <= 0.0 || msi_h <= 0.0)
		return; //invisible widget, nothing to render

	//Number of layers in the MSI, aka the lowest powerof2 that's bigger than width and height
	int layers;
	if (frexp (MAX (im_w, im_h), &layers) == 0.5)
		layers --;

	//optimal layer for this... aka "best viewed at"
	int optimal_layer;
	if (frexp (msi_w / (vp_w * MIN (1.0, msi_ar))  , &optimal_layer) == 0.5)
		optimal_layer--;
	optimal_layer = MIN (optimal_layer, layers);
	LOG_MSI ("number of layers: %d\toptimal layer for this: %d\n", layers, optimal_layer);

	//We have to figure all the layers that we'll have to render:
	//- from_layer is the highest COMPLETE layer that we can display (all tiles are there and blended (except for level 0, where it might not be blended yet))
	//- to_layer is the highest PARTIAL layer that we can display (contains at least 1 tiles partially blended)

	int to_layer = -1;
	int from_layer = optimal_layer;

	//using the "-1" index for the single image case
	int index = -1;
	QTree *subimage_cache = (QTree*)g_hash_table_lookup (cache, &index);
	if (!subimage_cache)
		g_hash_table_insert (cache, new int(index), (subimage_cache = qtree_new ()));

	while (from_layer >= 0) {
		int count = 0;
		int found = 0;
		bool blending = FALSE; //means at least a tile is not yet fully blended

		//v_tile_X is the virtual tile size at this layer in relative coordinates
		double v_tile_w = tile_width  * (double)(pow2 (layers - from_layer)) / im_w;
		double v_tile_h = tile_height * (double)(pow2 (layers - from_layer)) / im_w;
		int i, j;
		//This double loop iterate over the displayed part of the image and find all (i,j) being top-left corners of tiles
		for (i = MAX(0, (int)(vp_ox / v_tile_w)); i * v_tile_w < MIN(vp_ox + vp_w, 1.0); i++) {
			for (j = MAX(0, (int)(vp_oy / v_tile_h)); j * v_tile_h < MIN(vp_oy + vp_w / msi_w * msi_h, 1.0 / msi_ar); j++) {
				count++;
				cairo_surface_t *image = (cairo_surface_t*)qtree_lookup_data (subimage_cache, from_layer, i, j);

				if (image)
					found ++;
				if (image && *(double*)(cairo_surface_get_user_data (image, &full_opacity_at_key)) > GetValue(MultiScaleImage::TileFadeProperty)->AsDouble ())
					blending = TRUE;

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

	double fade = GetValue (MultiScaleImage::TileFadeProperty)->AsDouble();
	int layer_to_render = MAX (0, from_layer);
	while (layer_to_render <= to_layer) {
		int i, j;
		double v_tile_w = tile_width * (double)(pow2 (layers - layer_to_render)) / im_w;
		double v_tile_h = tile_height * (double)(pow2 (layers - layer_to_render)) / im_w;
		for (i = MAX(0, (int)(vp_ox / v_tile_w)); i * v_tile_w < MIN(vp_ox + vp_w, 1.0); i++) {
			for (j = MAX(0, (int)(vp_oy / v_tile_h)); j * v_tile_h < MIN(vp_oy + vp_w / msi_w * msi_h, 1.0 / msi_ar); j++) {
				cairo_surface_t *image = (cairo_surface_t*)qtree_lookup_data (subimage_cache, layer_to_render, i, j);
				if (!image)
					continue;

				LOG_MSI ("rendering %d %d %d\n", layer_to_render, i, j);
				cairo_save (cr);

				cairo_scale (cr, (pow2 (layers - layer_to_render)), (pow2 (layers - layer_to_render))); //scale to image size

				cairo_translate (cr, i * tile_width, j * tile_height);

				cairo_set_source_surface (cr, image, 0, 0);

				double *opacity = (double*)(cairo_surface_get_user_data (image, &full_opacity_at_key));
				double combined = 1.0;

				if (opacity && *opacity > fade)
					combined = MIN(1.0 - *opacity + fade, 1.0);

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
	//	cairo_pop_group_to_source (cr);

	if (!GetAllowDownloading ())
		return;

	BitmapImageContext *bitmapimagectx;
	if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
		return;

	//Get the next tile(s)...
	while (from_layer < optimal_layer) {
		from_layer ++;

		double v_tile_w = tile_width * (double)(pow2 (layers - from_layer)) / im_w;
		double v_tile_h = tile_height * (double)(pow2 (layers - from_layer)) / im_w;
		int i, j;

		for (i = MAX(0, (int)(vp_ox / v_tile_w)); i * v_tile_w < MIN(vp_ox + vp_w, 1.0); i++) {
			if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
				return;
			for (j = MAX(0, (int)(vp_oy / v_tile_h)); j * v_tile_h < MIN(vp_oy + vp_w / msi_w * msi_h, 1.0 / msi_ar); j++) {
				if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
					return;
				Uri *tile = new Uri ();
				QTree *node = qtree_insert (subimage_cache, from_layer, i, j);
				if (!qtree_has_value (node)) {
					if (source->get_tile_func (from_layer, i, j, tile, source))
						DownloadTile (bitmapimagectx, tile, node);
					else
						qtree_set_value (node, NULL);
				}
				delete tile;
			}
		}
	}
}

void
MultiScaleImage::OnSourcePropertyChanged ()
{
	//abort all downloaders
	StopDownloading ();

	DeepZoomImageTileSource *newsource;
	if (GetSource ()) {
		if (GetSource ()->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
			if ((newsource = GetValue (MultiScaleImage::SourceProperty)->AsDeepZoomImageTileSource ())) {
				newsource->set_callbacks (multi_scale_image_handle_dz_parsed, multi_scale_image_emit_image_open_failed, multi_scale_image_on_source_property_changed, this);
				newsource->Download ();
			}
		} else {
			EmitImageOpenSucceeded ();	
		}
	}

	//Reset the viewport
	ClearValue (MultiScaleImage::InternalViewportWidthProperty, true);
	ClearValue (MultiScaleImage::InternalViewportOriginProperty, true);
	//SetValue (MultiScaleImage::ViewportOriginProperty, Deployment::GetCurrent ()->GetTypes ()->GetProperty (MultiScaleImage::ViewportOriginProperty)->GetDefaultValue());
	//SetValue (MultiScaleImage::ViewportWidthProperty, Deployment::GetCurrent ()->GetTypes ()->GetProperty (MultiScaleImage::ViewportWidthProperty)->GetDefaultValue());

	//Invalidate the whole cache
	if (cache) {
		g_hash_table_destroy (cache);
		cache = g_hash_table_new_full (g_int_hash, g_int_equal, g_free, (GDestroyNotify)qtree_destroy);
	}

	//Reset the subimages
	GetSubImages()->Clear ();

	//register the callback for InvalidateTileLayers
	if (GetSource ())
		GetSource ()->set_invalidate_tile_layer_func (multi_scale_image_invalidate_tile_layer, this);

	Invalidate ();
}

void
MultiScaleImage::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == MultiScaleImage::AllowDownloadingProperty) {
		if (args->GetNewValue()->AsBool ())
			Invalidate();
		else
			StopDownloading ();
	}

	if (args->GetId () == MultiScaleImage::InternalViewportOriginProperty) {
		Emit (MultiScaleImage::ViewportChangedEvent);
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::InternalViewportWidthProperty) {
		Emit (MultiScaleImage::ViewportChangedEvent);
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::ViewportOriginProperty) {
		pan_target = Point (args->GetNewValue ()->AsPoint ()->x, args->GetNewValue ()->AsPoint ()->y);
		SetInternalViewportOrigin (args->GetNewValue ()->AsPoint ());
		ClearValue (MultiScaleImage::ViewportOriginProperty, false);
	}
	
	if (args->GetId () == MultiScaleImage::ViewportWidthProperty) {
		zoom_target = args->GetNewValue ()->AsDouble ();
		SetInternalViewportWidth (args->GetNewValue ()->AsDouble ());
		ClearValue (MultiScaleImage::ViewportWidthProperty, false);
	}

	if (args->GetId () == MultiScaleImage::TileFadeProperty) {
		//There's 2 options here,
		// - loop all the tiles, update their opacity, and only invalidate a subregion
		// - Invalidate all, and compute the new opacity on the tiles that needs to be rendered.
		//Both options are unfortunately quite expensive :(
		//LOG_MSI ("TileFade changed to %f\n", args->GetNewValue()->AsDouble ());
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::SourceProperty) {
		OnSourcePropertyChanged ();
	}

	if (args->GetId () == MultiScaleImage::UseSpringsProperty) {
		if (!args->GetNewValue()->AsBool ()) {
			if (zoom_sb) {
				double *endpoint = GetZoomAnimationEndPoint ();
				zoom_sb->StopWithError (NULL);
				SetViewportWidth (*endpoint);
			}
			if (pan_sb) {
				Point *endpoint = GetPanAnimationEndPoint ();
				pan_sb->StopWithError (NULL);
				SetViewportOrigin (endpoint);
			}
		}
	}

	if (args->GetProperty ()->GetOwnerType () != Type::MULTISCALEIMAGE) {
		MediaBase::OnPropertyChanged (args, error);
		return;
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

void
MultiScaleImage::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
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
	Emit (MultiScaleImage::ImageFailedEvent);
}

void
MultiScaleImage::EmitImageOpenFailed ()
{
	LOG_MSI ("MSI::Emitting image open failed\n");
	MoonError moon_error;
	MoonError::FillIn (&moon_error, MoonError::EXCEPTION, -2147467259, "");
	Emit (MultiScaleImage::ImageOpenFailedEvent, new ErrorEventArgs (UnknownError, moon_error));
}

void
MultiScaleImage::EmitImageOpenSucceeded ()
{
	LOG_MSI ("\nMSI::Emitting open suceeded\n");
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
	pending_motion_completed = false;
	Emit (MultiScaleImage::MotionFinishedEvent);
}

Point*
MultiScaleImage::GetPanAnimationEndPoint ()
{
	return pan_animation->GetKeyFrames ()->GetValueAt (0)->AsSplinePointKeyFrame ()->GetValue ();
}

void
MultiScaleImage::SetPanAnimationEndPoint (Point value)
{
	pan_animation->GetKeyFrames ()->GetValueAt (0)->AsSplinePointKeyFrame ()->SetValue (value);
}

double*
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
MultiScaleImage::SetInternalViewportWidth (double value)
{
	if (!GetUseSprings ()) {
		if (!pending_motion_completed) {
			AddTickCall ((TickCallHandler)multi_scale_image_emit_motion_finished);
			pending_motion_completed = true;
		}
		SetValue (MultiScaleImage::InternalViewportWidthProperty, Value (value));
		return;
	}

	if (!zoom_sb) {
		zoom_sb = new Storyboard ();
		zoom_sb->SetManualTarget (this);
		zoom_sb->SetTargetProperty (zoom_sb, new PropertyPath ("(MultiScaleImage.InternalViewportWidth)"));
		zoom_sb->AddHandler (Storyboard::CompletedEvent, zoom_finished, this);
		zoom_animation = new DoubleAnimationUsingKeyFrames ();
		zoom_animation->SetDuration (Duration::FromSeconds (4));
		zoom_animation->SetKeyFrames (DOPtr<DoubleKeyFrameCollection> (new DoubleKeyFrameCollection ()));
		DOPtr<SplineDoubleKeyFrame> keyframe (new SplineDoubleKeyFrame ());
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
	}

	LOG_MSI ("animating zoom from %f to %f\n\n", GetInternalViewportWidth(), value)	

	is_zooming = true;

	SetZoomAnimationEndPoint (value);
	zoom_sb->BeginWithError (NULL);
}

void
MultiScaleImage::SetInternalViewportOrigin (Point* value)
{
	if (!GetUseSprings ()) {
		if (!pending_motion_completed) {
			AddTickCall ((TickCallHandler)multi_scale_image_emit_motion_finished);
			pending_motion_completed = true;
		}
		SetValue (MultiScaleImage::InternalViewportOriginProperty, Value (*value));
		return;
	}

	if (!pan_sb) {
		pan_sb = new Storyboard ();
		pan_sb->SetManualTarget (this);
		pan_sb->SetTargetProperty (pan_sb, new PropertyPath ("(MultiScaleImage.InternalViewportOrigin)"));
		pan_sb->AddHandler (Storyboard::CompletedEvent, pan_finished, this);
		pan_animation = new PointAnimationUsingKeyFrames ();
		pan_animation->SetDuration (Duration::FromSeconds (4));
		pan_animation->SetKeyFrames (DOPtr<PointKeyFrameCollection> (new PointKeyFrameCollection ()));
		SplinePointKeyFrame *keyframe = new SplinePointKeyFrame ();
		keyframe->SetKeySpline (DOPtr<KeySpline> (new KeySpline (.05, .5, 0, 1.0)));
		keyframe->SetKeyTime (KeyTime::FromPercent (1.0));
		pan_animation->GetKeyFrames ()->Add (keyframe);

		TimelineCollection *tlc = new TimelineCollection ();
		tlc->Add (static_cast<PointAnimationUsingKeyFrames*> (pan_animation));
		pan_sb->SetChildren(tlc);
#if DEBUG
		pan_sb->SetName ("Multiscale Pan");
#endif
	} else
		pan_sb->PauseWithError (NULL);

	is_panning = true;
	SetPanAnimationEndPoint (*value);
	pan_sb->BeginWithError (NULL);
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
	QTree *subimage_cache = (QTree*)g_hash_table_lookup (cache, &index);
	if (subimage_cache)
		qtree_remove_at (subimage_cache, level, tilePositionX, tilePositionY, 0);

	Invalidate ();
}

/*
 * MultiScaleImagePropertyValueProvider
 */

MultiScaleImagePropertyValueProvider::MultiScaleImagePropertyValueProvider (MultiScaleImage *msi, PropertyPrecedence precedence)
	: FrameworkElementProvider (msi, precedence)
{
	viewport_origin = NULL;
	viewport_width = NULL;
}

MultiScaleImagePropertyValueProvider::~MultiScaleImagePropertyValueProvider ()
{
	delete viewport_origin;
	delete viewport_width;
}

Value *
MultiScaleImagePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	// We verify main thread here too in case some object in the pipeline happens to want a property on the media element
	VERIFY_MAIN_THREAD;
	
	if (property->GetId () == MultiScaleImage::ViewportOriginProperty)
		return GetViewportOrigin ();
	if (property->GetId () == MultiScaleImage::ViewportWidthProperty)
		return GetViewportWidth ();
	return FrameworkElementProvider::GetPropertyValue (property);
}

Value *
MultiScaleImagePropertyValueProvider::GetViewportOrigin ()
{
	MultiScaleImage *msi = (MultiScaleImage *) obj;

	delete viewport_origin;
	viewport_origin = new Value (*(msi->GetInternalViewportOrigin ()));
	return viewport_origin;
}

Value *
MultiScaleImagePropertyValueProvider::GetViewportWidth ()
{
	MultiScaleImage *msi = (MultiScaleImage *) obj;

	delete viewport_width;
	viewport_width = new Value (msi->GetInternalViewportWidth ());	
	return viewport_width;
}
