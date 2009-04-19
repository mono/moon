/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * multiscaleimage.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

//TODO
//
//- only invalidate regions
//- only render changed regions

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "cbinding.h"
#include "multiscaleimage.h"
#include "tilesource.h"
#include "deepzoomimagetilesource.h"
#include "file-downloader.h"
#include "multiscalesubimage.h"

#if LOGGING
#include "clock.h"
#define MSI_STARTTIMER(id)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MSI)) TimeSpan id##_t_start = get_now()
#define MSI_ENDTIMER(id,str)		if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MSI)) TimeSpan id##_t_end = get_now(); printf ("timing of '%s' ended took (%f ms)\n", str, id##_t_end, (double)(id##_t_end - id##_t_start) / 10000)
#else
#define STATTIMER(id)
#define ENDTIMER(id,str)
#endif

void _cairo_surface_destroy (void* surface) {cairo_surface_destroy((cairo_surface_t*)surface);}

enum BitmapImageStatus {
	BitmapImageFree = 0,
	BitmapImageBusy,
	BitmapImageDone
};

struct BitmapImageContext
{
	BitmapImageStatus state;
	BitmapImage *bitmapimage;
};

void
morton (int n, int *x, int *y) {
	n = (n & 0x99999999) + ((n & 0x22222222) << 1) + ((n & 0x44444444) >> 1);
	n = (n & 0xc3c3c3c3) + ((n & 0x0c0c0c0c) << 2) + ((n & 0x30303030) >> 2);
	n = (n & 0xf00ff00f) + ((n & 0x00f000f0) << 4) + ((n & 0x0f000f00) >> 4);
	n = (n & 0xff0000ff) + ((n & 0x0000ff00) << 8) + ((n & 0x00ff0000) >> 8);
	*x = n & 0x0000ffff;
	*y = n >> 16;
}

inline int
morton_x (int n)
{
	n = (n & 0x11111111) + ((n & 0x44444444) >> 1);
	n = (n & 0x03030303) + ((n & 0x30303030) >> 2);
	n = (n & 0x000f000f) + ((n & 0x0f000f00) >> 4);
	return  (n & 0x000000ff) + ((n & 0x00ff0000) >> 8);
}

inline int
morton_y (int n)
{
	n = (n & 0x88888888) + ((n & 0x22222222) << 1);
	n = (n & 0xc0c0c0c0) + ((n & 0x0c0c0c0c) << 2);
	n = (n & 0xf000f000) + ((n & 0x00f000f0) << 4);
	n = (n & 0xff000000) + ((n & 0x0000ff00) << 8);

	return n >> 16;
}

MultiScaleImage::MultiScaleImage ()
{
//	static bool init = true;
//	if (init) {
//		init = false;
//		MultiScaleImage::SubImagesProperty->SetValueValidator (MultiScaleSubImageCollectionValidator);	
//	}
	SetObjectType (Type::MULTISCALEIMAGE); 
	source = NULL;
	bitmapimages = NULL;
	cache = g_hash_table_new_full ((GHashFunc)uri_get_hash_code, (GCompareFunc)uri_equals, g_free, _cairo_surface_destroy);
	zoom_sb = NULL;
	pan_sb = NULL;
	fadein_sb = NULL;
	subimages_sorted = false;
}

MultiScaleImage::~MultiScaleImage ()
{
	if (cache)
		g_hash_table_destroy (cache);
	cache = NULL;
	if (bitmapimages)
		g_list_free (bitmapimages);
	bitmapimages = NULL;
}

void
MultiScaleImage::ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY)
{
	LOG_MSI ("\nzoomabout logical %f  (%f, %f)\n", zoomIncrementFactor, zoomCenterLogicalX, zoomCenterLogicalY);

	if (zoom_sb)
		zoom_sb->PauseWithError (NULL);
	if (pan_sb)
		pan_sb->PauseWithError (NULL);


	double width = GetViewportWidth () / zoomIncrementFactor;
	SetViewportWidth (width);
	if (!isnan(zoomCenterLogicalX) && !isnan(zoomCenterLogicalY)) {
		SetViewportOrigin (Point (zoomCenterLogicalX - (zoomCenterLogicalX - GetViewportOrigin()->x) / zoomIncrementFactor,
					  zoomCenterLogicalY - (zoomCenterLogicalY - GetViewportOrigin()->y) / zoomIncrementFactor));
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

//test if the cache contains a tile at the @filename key
//if @empty_tiles is TRUE, it'll return TRUE even if the cached tile is NULL. if @empty_tiles is FALSE, a NULL tile will be treated as missing
bool
MultiScaleImage::cache_contains (Uri *uri, bool empty_tiles)
{
	if (!uri)
		return empty_tiles;
	if (empty_tiles)
		return g_hash_table_lookup_extended (cache, uri, NULL, NULL);
	else
		return g_hash_table_lookup (cache, uri) != NULL;
}

void
MultiScaleImage::DownloadTile (BitmapImageContext *bictx, Uri *tile)
{
	GList *list;
	BitmapImageContext *ctx;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next) {
		if (ctx->bitmapimage->GetUriSource()->operator==(*tile)) {
			LOG_MSI ("Tile %s is already being downloaded\n", tile->ToString ());
			return;
		}
	}

	bictx->state = BitmapImageBusy;
	SetIsDownloading (true);
	bictx->bitmapimage->SetUriSource (tile);
}

void
multi_scale_image_handle_parsed (void *userdata)
{
	MultiScaleImage *msi = (MultiScaleImage*)userdata;
	//if the source is a collection, fill the subimages list
	MultiScaleTileSource *source = msi->GetSource ();

	if (source->GetImageWidth () >= 0 && source->GetImageHeight () >= 0)
		msi->SetValue (MultiScaleImage::AspectRatioProperty, Value ((double)source->GetImageWidth () / (double)source->GetImageHeight ()));

	DeepZoomImageTileSource *dsource;
	if (source->Is (Type::DEEPZOOMIMAGETILESOURCE) &&
	    (dsource = (DeepZoomImageTileSource *)source)) {
		int i;
		MultiScaleSubImage *si;
		for (i = 0; (si = (MultiScaleSubImage*)g_list_nth_data (dsource->subimages, i)); i++) {
			if (!msi->GetSubImages ())
				msi->SetValue (MultiScaleImage::SubImagesProperty, new MultiScaleSubImageCollection ());

			msi->GetSubImages ()->Add (si);
		}
	}
	msi->Invalidate ();

	//FIXME: we're only emitting this in deepzoom case
	msi->EmitImageOpenSucceeded ();
}

void
multi_scale_subimage_handle_parsed (void *userdata)
{
	MultiScaleImage *msi = (MultiScaleImage*)userdata;
	msi->Invalidate ();
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

	int tile_width = source->GetTileWidth ();
	int tile_height = source->GetTileHeight ();

	if (!source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
		g_warning ("RenderCollection called for a non deepzoom tile source. this should not happen");
		return;
	}
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *)source;

	Rect viewport = Rect (msivp_ox, msivp_oy, msivp_w, msivp_w/msi_ar);

	if (!subimages_sorted) {
		GetSubImages ()->ResortByZIndex ();
		subimages_sorted = true;
	}
	int i;
	for (i = 0; i < GetSubImages ()->GetCount (); i++) {
		MultiScaleSubImage *sub_image = (MultiScaleSubImage*)g_ptr_array_index (GetSubImages ()->z_sorted, i);

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

			//in msi relative coord
			double v_tile_w = tile_width * (double)(1 << (layers - from_layer)) * sub_vp.width / sub_w;
			double v_tile_h = tile_height * (double)(1 << (layers - from_layer)) * sub_vp.width / sub_w;
			//LOG_MSI ("virtual tile size at layer %d; %fx%f\n", from_layer, v_tile_w, v_tile_h);

			int i, j;
			for (i = (int)((MAX(msivp_ox, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				for (j = (int)((MAX(msivp_oy, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msivp_oy + msivp_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					count++;
					Uri *tile = new Uri ();
					cairo_surface_t* image = NULL;

					if (source->get_tile_func (from_layer, i, j, tile, sub_image->source) &&
					    (image = (cairo_surface_t*)g_hash_table_lookup (cache, tile)) ) {
						found ++;
					} else if (from_layer <= dzits->GetMaxLevel () &&
						 source->get_tile_func (from_layer,
									morton_x (sub_image->n) * (1 << from_layer) / tile_width,
									morton_y (sub_image->n) * (1 << from_layer) / tile_height,
									tile,
									source) &&
						(image = (cairo_surface_t*)g_hash_table_lookup (cache, tile)) ) {
						found ++;
					}

					delete tile;

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
				double v_tile_w = tile_width * (double)(1 << (layers - layer_to_render)) * sub_vp.width / sub_w;
				double v_tile_h = tile_height * (double)(1 << (layers - layer_to_render)) * sub_vp.width / sub_w;

				int i, j;
				for (i = (int)((MAX(msivp_ox, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
					for (j = (int)((MAX(msivp_oy, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msivp_oy + msivp_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
						Uri *tile = new Uri ();
						cairo_surface_t *image = NULL;
						bool shared_tile = false;
						if ((!source->get_tile_func (layer_to_render, i, j, tile, sub_image->source) ||
						     !(image = (cairo_surface_t*)g_hash_table_lookup (cache, tile))		)
						    && layer_to_render <= dzits->GetMaxLevel()) {
							//Check in the shared levels
							shared_tile = true;

							if (source->get_tile_func (layer_to_render,
									morton_x(sub_image->n) * (1 << layer_to_render) / tile_width,
									morton_y(sub_image->n) * (1 << layer_to_render) / tile_height,
									tile,
									source) ) {
								image = (cairo_surface_t*)g_hash_table_lookup (cache, tile);
							}
						}

						delete tile;

						if (!image)
							continue;

						LOG_MSI ("rendering subimage %d %d %d %d\n", sub_image->id, layer_to_render, i, j);
						cairo_save (cr);

						cairo_scale (cr,
							     1 << (layers - layer_to_render),
							     1 << (layers - layer_to_render));

						cairo_translate (cr,
								 i * tile_width,
								 j * tile_height);

						if (shared_tile) {
							cairo_translate (cr,
									 (-morton_x(sub_image->n) * (1 << layer_to_render)) % tile_width,
									 (-morton_y(sub_image->n) * (1 << layer_to_render)) % tile_height);

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
				cairo_pop_group_to_source (cr);
				cairo_paint_with_alpha (cr, sub_image->GetOpacity ());
			}

			cairo_restore (cr);
		}

		if (!GetAllowDownloading ())
			continue;

		BitmapImageContext *bitmapimagectx;
		if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
			continue;

		//Get the next tile...
		while (from_layer < optimal_layer) {
			from_layer ++;

			//if the subimage is unparsed, trigger the download
			if (from_layer > ((DeepZoomImageTileSource *)source)->GetMaxLevel () && !((DeepZoomImageTileSource *)sub_image->source)->IsDownloaded () ) {
				((DeepZoomImageTileSource*)sub_image->source)->set_parsed_cb (multi_scale_subimage_handle_parsed, this);
				((DeepZoomImageTileSource*)sub_image->source)->Download ();
				break;
			}

			double v_tile_w = tile_width * (double)(1 << (layers - from_layer)) * sub_vp.width / sub_w;
			double v_tile_h = tile_height * (double)(1 << (layers - from_layer)) * sub_vp.width / sub_w;

			int i, j;
			for (i = (int)((MAX(msivp_ox, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
					break;
				for (j = (int)((MAX(msivp_oy, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msivp_oy + msivp_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
						break;
					Uri * tile = new Uri ();
					bool ret = false;
					if (from_layer <= dzits->GetMaxLevel ())
						ret = source->get_tile_func (from_layer,
									     morton_x(sub_image->n) * (1 << from_layer) / tile_width,
									     morton_y(sub_image->n) * (1 << from_layer) / tile_height,
									     tile,
									     source);
					else 
						ret = source->get_tile_func (from_layer, i, j, tile, sub_image->source);
					if (ret && !cache_contains (tile, true)) {
						DownloadTile (bitmapimagectx, tile);
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
	double msi_w = GetActualWidth ();
	double msi_h = GetActualHeight ();
	double msi_ar = GetAspectRatio ();
	double im_w = (double) source->GetImageWidth ();
	double im_h = (double) source->GetImageHeight ();
	int tile_width = source->GetTileWidth ();
	int tile_height = source->GetTileHeight ();
	double vp_ox = GetViewportOrigin()->x;
	double vp_oy = GetViewportOrigin()->y;
	double vp_w = GetViewportWidth ();

	int layers;

	if (msi_w <= 0.0 && msi_h <= 0.0)
		return;

	if (frexp (MAX (im_w, im_h), &layers) == 0.5)
		layers --;

	//optimal layer for this... aka "best viewed at"
	int optimal_layer;
	frexp (msi_w / (vp_w * MIN (1.0, msi_ar))  , &optimal_layer);
	optimal_layer = MIN (optimal_layer, layers);
	LOG_MSI ("number of layers: %d\toptimal layer for this: %d\n", layers, optimal_layer);

	//We have to figure all the layers that we'll have to render:
	//- from_layer is the highest COMPLETE layer that we can display (all tiles are there and blended (except for level 0, where it might not be blended yet))
	//- to_layer is the highest PARTIAL layer that we can display (contains at least 1 tiles partially blended)

	int to_layer = -1;
	int from_layer = optimal_layer;

	while (from_layer >= 0) {
		int count = 0;
		int found = 0;
		bool blending = FALSE; //means at least a tile is not yet fully blended

		//v_tile_X is the virtual tile size at this layer in relative coordinates
		double v_tile_w = tile_width  * (double)(1 << (layers - from_layer)) / im_w;
		double v_tile_h = tile_height * (double)(1 << (layers - from_layer)) / im_w;

		int i, j;
		//This double loop iterate over the displayed part of the image and find all (i,j) being top-left corners of tiles
		for (i = MAX(0, (int)(vp_ox / v_tile_w)); i * v_tile_w < MIN(vp_ox + vp_w, 1.0); i++) {
			for (j = MAX(0, (int)(vp_oy / v_tile_h)); j * v_tile_h < MIN(vp_oy + vp_w * msi_w / msi_h, 1.0 / msi_ar); j++) {
				count++;
				Uri *tile = new Uri();
				if (!source->get_tile_func (from_layer, i, j, tile, source))
					continue;
				cairo_surface_t *image = (cairo_surface_t*)g_hash_table_lookup (cache, tile);
				if (image)
					found ++;
				if (image && *(double*)(cairo_surface_get_user_data (image, &full_opacity_at_key)) > GetValue(MultiScaleImage::TileFadeProperty)->AsDouble ())
					blending = TRUE;

				delete tile;
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

	cairo_rectangle (cr, 0, 0, msi_w, msi_h);
	cairo_clip (cr);
	cairo_paint (cr);

	cairo_scale (cr, msi_w / vp_w, msi_w / vp_w); //scale to viewport
	cairo_translate (cr, -vp_ox, -vp_oy);
	cairo_scale (cr, 1.0 / im_w, 1.0 / im_w);

	LOG_MSI ("rendering layers from %d to %d\n", from_layer, to_layer);

	double fade = GetValue (MultiScaleImage::TileFadeProperty)->AsDouble();
	int layer_to_render = from_layer;
	while (from_layer >= 0 && layer_to_render <= to_layer) {
		int i, j;
		double v_tile_w = tile_width * (double)(1 << (layers - layer_to_render)) / im_w;
		double v_tile_h = tile_height * (double)(1 << (layers - layer_to_render)) / im_w;
		for (i = MAX(0, (int)(vp_ox / v_tile_w)); i * v_tile_w < MIN(vp_ox + vp_w, 1.0); i++) {
			for (j = MAX(0, (int)(vp_oy / v_tile_h)); j * v_tile_h < MIN(vp_oy + vp_w * msi_w / msi_h, 1.0 / msi_ar); j++) {
				Uri *tile = new Uri ();
				if (!source->get_tile_func (layer_to_render, i, j, tile, source))
					continue;
				cairo_surface_t *image = (cairo_surface_t*)g_hash_table_lookup (cache, tile);
				delete tile;

				if (!image)
					continue;

				LOG_MSI ("rendering %d %d %d\n", layer_to_render, i, j);
				cairo_save (cr);

				cairo_scale (cr, 1 << (layers - layer_to_render), 1 << (layers - layer_to_render)); //scale to image size

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

		double v_tile_w = tile_width * (double)(1 << (layers - from_layer)) / im_w;
		double v_tile_h = tile_height * (double)(1 << (layers - from_layer)) / im_w;
		int i, j;

		for (i = MAX(0, (int)(vp_ox / v_tile_w)); i * v_tile_w < MIN(vp_ox + vp_w, 1.0); i++) {
			if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
				return;
			for (j = MAX(0, (int)(vp_oy / v_tile_h)); j * v_tile_h < MIN(vp_oy + vp_w * msi_w / msi_h, 1.0 / msi_ar); j++) {
				if (!(bitmapimagectx = GetFreeBitmapImageContext ()))
					return;
				Uri *tile = new Uri ();
				if (source->get_tile_func (from_layer, i, j, tile, source) && !cache_contains (tile, true)) {
					DownloadTile (bitmapimagectx, tile);
				}
				delete tile;
			}
		}
	}
}

void
MultiScaleImage::Render (cairo_t *cr, Region *region, bool path_only)
{
	LOG_MSI ("MSI::Render\n");

	//Process the downloaded tile
	GList *list;
	BitmapImageContext *ctx;
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next) {
		cairo_surface_t *surface;

		if (ctx->state != BitmapImageDone || !(surface = cairo_surface_reference (ctx->bitmapimage->GetSurface (cr))))
			continue;

		Uri *tile = ctx->bitmapimage->GetUriSource ();
		cairo_surface_set_user_data (surface, &width_key, new int (ctx->bitmapimage->GetPixelWidth ()), g_free);
		cairo_surface_set_user_data (surface, &height_key, new int (ctx->bitmapimage->GetPixelHeight ()), g_free);

		if (!fadein_sb) {
			fadein_sb = new Storyboard ();
			fadein_sb->SetManualTarget (this);
			fadein_sb->SetTargetProperty (fadein_sb, new PropertyPath ("(MultiScaleImage.TileFade)"));
			fadein_animation = new DoubleAnimation ();
			fadein_animation->SetDuration (Duration::FromSecondsFloat (0.5));
			TimelineCollection *tlc = new TimelineCollection ();
			tlc->Add (fadein_animation);
			fadein_sb->SetChildren(tlc);
		} else {
			fadein_sb->PauseWithError (NULL);
		}

		//LOG_MSI ("animating Fade from %f to %f\n\n", GetValue(MultiScaleImage::TileFadeProperty)->AsDouble(), GetValue(MultiScaleImage::TileFadeProperty)->AsDouble() + 0.9);
		double *to = new double (GetValue(MultiScaleImage::TileFadeProperty)->AsDouble() + 0.9);
		fadein_animation->SetFrom (GetValue(MultiScaleImage::TileFadeProperty)->AsDouble());
		fadein_animation->SetTo (*to);

		fadein_sb->BeginWithError(NULL);

		cairo_surface_set_user_data (surface, &full_opacity_at_key, to, g_free);
		LOG_MSI ("caching %s\n", tile->ToString ());
		g_hash_table_insert (cache, new Uri(*tile), surface);

		ctx->state = BitmapImageFree;
	}

	bool is_collection = source &&
			     source->Is (Type::DEEPZOOMIMAGETILESOURCE) &&
			     ((DeepZoomImageTileSource *)source)->IsCollection () &&
			     GetSubImages ();

	if (!(source = GetSource ())) {
		LOG_MSI ("no sources set, nothing to render\n");
		return;
	}

	if (source->GetImageWidth () < 0 && !is_collection) {
		LOG_MSI ("nothing to render so far...\n");
		if (source->Is (Type::DEEPZOOMIMAGETILESOURCE)) {
			((DeepZoomImageTileSource*)source)->set_parsed_cb (multi_scale_image_handle_parsed, this);
			((DeepZoomImageTileSource*)source)->Download ();
		}
		return;
	}


	if (!source->get_tile_func) {
		g_warning ("no get_tile_func set\n");
		return;
	}

	if (is_collection)
		RenderCollection (cr, region);
	else
		RenderSingle (cr, region);
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
tile_available (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	LOG_MSI ("Tile downloaded %s\n", ((BitmapImage *)sender)->GetUriSource ()->ToString ());
	((MultiScaleImage *)closure)->TileOpened ((BitmapImage *)sender);
}

void
MultiScaleImage::TileFailed (BitmapImage *bitmapimage)
{
	BitmapImageContext *ctx = GetBitmapImageContext (bitmapimage);
	ctx->state = BitmapImageFree;
	g_hash_table_insert (cache, new Uri(*(ctx->bitmapimage->GetUriSource())), NULL);
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
	LOG_MSI ("Failed to download tile %s\n", ((BitmapImage *)sender)->GetUriSource ()->ToString ());
	((MultiScaleImage *)closure)->TileFailed ((BitmapImage *)sender);
}

BitmapImageContext *
MultiScaleImage::GetFreeBitmapImageContext ()
{
	guint num_dl = 6;
	BitmapImageContext *ctx;
	GList *list;
#if RUNTIME_DEBUG_MSI
	LOG_MSI ("\nBitmapImages: %d\n", num_dl);
	for (list = g_list_first (bitmapimages); list && (ctx = (BitmapImageContext *)list->data); list = list->next)
		LOG_MSI ("\tstate: %d, source: %s\n", ctx->state, ctx->bitmapimage->GetUriSource ()->ToString());
#endif
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
MultiScaleImage::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == MultiScaleImage::AllowDownloadingProperty && args->GetNewValue () && args->GetNewValue()->AsBool ())
		Invalidate();

	if (args->GetId () == MultiScaleImage::ViewportOriginProperty) {
		Emit (MultiScaleImage::ViewportChangedEvent);
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::ViewportWidthProperty) {
		//LOG_MSI ("ViewportWidth set to %f\n", args->GetNewValue()->AsDouble ());
		Emit (MultiScaleImage::ViewportChangedEvent);
		Invalidate ();
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
		DeepZoomImageTileSource *source;
		if (args->GetNewValue() &&
		    args->GetNewValue ()->Is (Type::DEEPZOOMIMAGETILESOURCE) && 
		    (source = args->GetNewValue()->AsDeepZoomImageTileSource ())) {
			source->set_parsed_cb (multi_scale_image_handle_parsed, this);
			source->Download ();
		}

		//FIXME: On source change
		// - abort all downloaders
		// - invalidate the cache
		// - invalidate the control
	}

	if (args->GetProperty ()->GetOwnerType () != Type::MULTISCALEIMAGE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
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
MultiScaleImage::EmitImageOpenSucceeded ()
{
	LOG_MSI ("\nMSI::Emitting open suceeded\n");
	Emit (MultiScaleImage::ImageOpenSucceededEvent);
}

void
MultiScaleImage::EmitMotionFinished ()
{
	LOG_MSI ("Emitting MotionFinished\n");
	Emit (MultiScaleImage::MotionFinishedEvent);
}

void
motion_finished (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->EmitMotionFinished ();
}

void
MultiScaleImage::SetViewportWidth (double value)
{
	if (!GetUseSprings ()) {
		SetValue (MultiScaleImage::ViewportWidthProperty, Value (value));
		return;
	}

	if (!zoom_sb) {
		zoom_sb = new Storyboard ();
		zoom_sb->SetManualTarget (this);
		zoom_sb->SetTargetProperty (zoom_sb, new PropertyPath ("(MultiScaleImage.ViewportWidth)"));
		zoom_sb->AddHandler (zoom_sb->CompletedEvent, motion_finished, this);
		zoom_animation = new DoubleAnimationUsingKeyFrames ();
		zoom_animation->SetDuration (Duration::FromSeconds (4));
		zoom_animation->SetKeyFrames (new DoubleKeyFrameCollection ());
		SplineDoubleKeyFrame *keyframe = new SplineDoubleKeyFrame ();
		keyframe->SetKeySpline (new KeySpline (0, 1.0, 0, 1.0));
		keyframe->SetKeyTime (KeyTime::FromPercent (1.0));
		zoom_animation->GetKeyFrames ()->Add (keyframe);

		TimelineCollection *tlc = new TimelineCollection ();
		tlc->Add (zoom_animation);
		zoom_sb->SetChildren(tlc);
	} else {
		zoom_sb->PauseWithError (NULL);
	}

	LOG_MSI ("animating zoom from %f to %f\n\n", GetViewportWidth(), value)	

	zoom_animation->GetKeyFrames ()->GetValueAt (0)->AsSplineDoubleKeyFrame ()->SetValue (value);
	zoom_sb->BeginWithError (NULL);
}

void
MultiScaleImage::SetViewportOrigin (Point value)
{
	if (!GetUseSprings ()) {
		SetValue (MultiScaleImage::ViewportOriginProperty, Value (value));
		return;
	}

	if (!pan_sb) {
		pan_sb = new Storyboard ();
		pan_sb->SetManualTarget (this);
		pan_sb->SetTargetProperty (pan_sb, new PropertyPath ("(MultiScaleImage.ViewportOrigin)"));
		pan_sb->AddHandler (pan_sb->CompletedEvent, motion_finished, this);
		pan_animation = new PointAnimationUsingKeyFrames ();
		pan_animation->SetDuration (Duration::FromSeconds (4));
		pan_animation->SetKeyFrames (new PointKeyFrameCollection ());
		SplinePointKeyFrame *keyframe = new SplinePointKeyFrame ();
		keyframe->SetKeySpline (new KeySpline (0, 1.0, 0, 1.0));
		keyframe->SetKeyTime (KeyTime::FromPercent (1.0));
		pan_animation->GetKeyFrames ()->Add (keyframe);

		TimelineCollection *tlc = new TimelineCollection ();
		tlc->Add (pan_animation);
		pan_sb->SetChildren(tlc);
	} else
		pan_sb->PauseWithError (NULL);

	pan_animation->GetKeyFrames ()->GetValueAt (0)->AsSplinePointKeyFrame ()->SetValue (value);
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


