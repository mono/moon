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

enum DownloaderState {
	DownloaderFree,	//Free to be used
	DownloaderBusy,	//Downloading
	DownloaderDone	//Downloaded, but we still need to process it's results before reusing it
};

struct DownloaderContext
{
	DownloaderState state;
	Downloader *downloader;
	Uri *uri;
};

MultiScaleImage::MultiScaleImage ()
{
//	static bool init = true;
//	if (init) {
//		init = false;
//		MultiScaleImage::SubImagesProperty->SetValueValidator (MultiScaleSubImageCollectionValidator);	
//	}
	SetObjectType (Type::MULTISCALEIMAGE); 
	source = NULL;
	downloaders = NULL;
	cache = g_hash_table_new_full ((GHashFunc)uri_get_hash_code, (GCompareFunc)uri_equals, g_free, _cairo_surface_destroy);
	zoom_sb = NULL;
	pan_sb = NULL;
	fadein_sb = NULL;
	subimages_sorted = false;
}

MultiScaleImage::~MultiScaleImage ()
{
	DownloadersAbort ();
	if (cache)
		g_hash_table_destroy (cache);
	cache = NULL;
	if (downloaders)
		g_list_free (downloaders);
	downloaders = NULL;
}

void
MultiScaleImage::ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY)
{
	LOG_MSI ("\nzoomabout logical %f  (%f, %f)\n", zoomIncrementFactor, zoomCenterLogicalX, zoomCenterLogicalY);

	if (zoom_sb)
		zoom_sb->Pause ();
	if (pan_sb)
		pan_sb->Pause ();
	
	double width = GetViewportWidth () / zoomIncrementFactor;
	double height = GetViewportWidth () / (GetAspectRatio () * zoomIncrementFactor);
	SetViewportWidth (width);
	if (!isnan(zoomCenterLogicalX) && !isnan(zoomCenterLogicalY))
		SetViewportOrigin (Point (zoomCenterLogicalX - width/2.0, zoomCenterLogicalY - height/2.0));
}

Point
MultiScaleImage::ElementToLogicalPoint (Point elementPoint)
{
	return Point (GetViewportOrigin()->x + (double)elementPoint.x * GetViewportWidth () / GetActualWidth (),
		      GetViewportOrigin()->y + (double)elementPoint.y * GetViewportWidth () / GetActualWidth ());
}

void
MultiScaleImage::DownloadUri (DownloaderContext *ctx, Uri* url)
{
	LOG_MSI ("MSI::DownloadUri %s\n", url->ToString ());

	GList *list;
	DownloaderContext *dlctx;
	for (list = g_list_first (downloaders); list && (dlctx = (DownloaderContext*)list->data); list = list->next) {
		if (dlctx->state == DownloaderBusy && dlctx->uri->operator== (*url)) {
			LOG_MSI ("this tile is already being downloaded\n");
			return;
		}
	}

	ctx->state = DownloaderBusy;
	if (ctx->uri)
		delete ctx->uri;

	ctx->uri = new Uri (*url);
	ctx->downloader->Open ("GET", url->ToString (), NoPolicy);

	ctx->downloader->Send ();

	if (ctx->downloader->Started () || ctx->downloader->Completed ()) {
		if (ctx->downloader->Completed ())
			DownloaderComplete (ctx);
	} else
		ctx->downloader->Send ();
}

#ifdef WORDS_BIGENDIAN
#define set_pixel_bgra(pixel,index,b,g,r,a) \
	G_STMT_START { \
		((unsigned char *)(pixel))[index]   = a; \
		((unsigned char *)(pixel))[index+1] = r; \
		((unsigned char *)(pixel))[index+2] = g; \
		((unsigned char *)(pixel))[index+3] = b; \
	} G_STMT_END
#define get_pixel_bgr_p(p,b,g,r) \
	G_STMT_START { \
		r = *(p);   \
		g = *(p+1); \
		b = *(p+2); \
	} G_STMT_END
#else
#define set_pixel_bgra(pixel,index,b,g,r,a) \
	G_STMT_START { \
		((unsigned char *)(pixel))[index]   = b; \
		((unsigned char *)(pixel))[index+1] = g; \
		((unsigned char *)(pixel))[index+2] = r; \
		((unsigned char *)(pixel))[index+3] = a; \
	} G_STMT_END
#define get_pixel_bgr_p(p,b,g,r) \
	G_STMT_START { \
		b = *(p);   \
		g = *(p+1); \
		r = *(p+2); \
	} G_STMT_END
#endif
#define get_pixel_bgra(color, b, g, r, a) \
	G_STMT_START { \
		a = ((color & 0xff000000) >> 24); \
		r = ((color & 0x00ff0000) >> 16); \
		g = ((color & 0x0000ff00) >> 8); \
		b = (color & 0x000000ff); \
	} G_STMT_END
#define get_pixel_bgr(color, b, g, r) \
	G_STMT_START { \
		r = ((color & 0x00ff0000) >> 16); \
		g = ((color & 0x0000ff00) >> 8); \
		b = (color & 0x000000ff); \
	} G_STMT_END
#include "alpha-premul-table.inc"

//
// Converts RGBA unmultiplied alpha to ARGB pre-multiplied alpha.
//
static void
unmultiply_rgba_in_place (GdkPixbuf *pixbuf)
{
	guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
	guchar *p;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
		for (int x = 0; x < w; x ++) {
			guint32 color = *(guint32*)p;
			guchar r, g, b, a;

			get_pixel_bgra (color, b, g, r, a);

			/* pre-multipled alpha */
			if (a == 0) {
				r = g = b = 0;
			}
			else if (a < 255) {
				r = pre_multiplied_table [r][a];
				g = pre_multiplied_table [g][a];
				b = pre_multiplied_table [b][a];
			}

			/* store it back, swapping red and blue */
			set_pixel_bgra (p, 0, r, g, b, a);

			p += 4;
		}
	}
}

//
// Expands RGB to ARGB allocating new buffer for it.
//
static guchar*
expand_rgb_to_argb (GdkPixbuf *pixbuf, int *stride)
{
	guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
	guchar *p;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);
	*stride = w * 4;
	guchar *data = (guchar *) g_malloc (*stride * h);
	guchar *out;

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
		out = data + y * (*stride);
		if (false && gdk_pixbuf_get_rowstride (pixbuf) % 4 == 0) {
			for (int x = 0; x < w; x ++) {
				guint32 color = *(guint32*)p;
				guchar r, g, b;

				get_pixel_bgr (color, b, g, r);
				set_pixel_bgra (out, 0, r, g, b, 255);

				p += 3;
				out += 4;
			}
		}
		else {
			for (int x = 0; x < w; x ++) {
				guchar r, g, b;

				get_pixel_bgr_p (p, b, g, r);
				set_pixel_bgra (out, 0, r, g, b, 255);

				p += 3;
				out += 4;
			}
		}
	}

	return data;
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
multi_scale_image_handle_parsed (void *userdata)
{
	MultiScaleImage *msi = (MultiScaleImage*)userdata;
	//if the source is a collection, fill the subimages list
	MultiScaleTileSource *source = msi->GetSource ();

	if (source->GetImageWidth () >= 0 && source->GetImageHeight () >= 0)
		msi->SetValue (MultiScaleImage::AspectRatioProperty, Value ((double)source->GetImageWidth () / (double)source->GetImageHeight ()));

	DeepZoomImageTileSource *dsource = (DeepZoomImageTileSource *)source;
	if (dsource) {
		int i;
		MultiScaleSubImage *si;
		for (i = 0; (si = (MultiScaleSubImage*)g_list_nth_data (dsource->subimages, i)); i++) {
			if (!msi->GetSubImages ())
				msi->SetValue (MultiScaleImage::SubImagesProperty, new MultiScaleSubImageCollection ());

			msi->GetSubImages ()->Add (si);
		}
	}
	msi->Invalidate ();
	LOG_MSI ("\nMSI::Emitting open suceeded\n");
	msi->AddTickCall (MultiScaleImage::EmitImageOpenSucceeded);
}

void
MultiScaleImage::EmitImageOpenSucceeded (EventObject *user_data)
{
	MultiScaleImage *msi = (MultiScaleImage *) user_data;
	
	msi->Emit (MultiScaleImage::ImageOpenSucceededEvent);
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

	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource*)source;

	Rect viewport = Rect (msivp_ox, msivp_oy, msivp_w, msivp_w/msi_ar);

	if (!subimages_sorted) {
		GetSubImages ()->ResortByZIndex ();
		subimages_sorted = true;
	}
	int i;
	for (i = 0; i < GetSubImages ()->GetCount (); i++) {
		MultiScaleSubImage *sub_image = (MultiScaleSubImage*)g_ptr_array_index (GetSubImages ()->z_sorted, i);

		//if the subimage is unparsed, trigger the download
		//FIXME: THIS NOT REQUIRED FOR LAYERS << MaxTileLayer
//		if (sub_image->source->GetImageWidth () < 0) {
//			((DeepZoomImageTileSource*)sub_image->source)->set_parsed_cb (multi_scale_subimage_handle_parsed, this);
//			((DeepZoomImageTileSource*)sub_image->source)->Download ();
//			continue;
//		}

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

						if (opacity && *opacity > GetValue (MultiScaleImage::TileFadeProperty)->AsDouble()) 
							cairo_paint_with_alpha (cr, MIN(1.0 - *opacity + GetValue(MultiScaleImage::TileFadeProperty)->AsDouble (), 1.0));
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

		DownloaderContext *dlctx;
		if (!(dlctx = GetFreeDownloader ()))
			continue;

		//Get the next tile...
		while (from_layer < optimal_layer) {
			from_layer ++;

			double v_tile_w = tile_width * (double)(1 << (layers - from_layer)) * sub_vp.width / sub_w;
			double v_tile_h = tile_height * (double)(1 << (layers - from_layer)) * sub_vp.width / sub_w;

			int i, j;
			for (i = (int)((MAX(msivp_ox, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msivp_ox + msivp_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				if (!(dlctx = GetFreeDownloader ()))
					break;
				for (j = (int)((MAX(msivp_oy, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msivp_oy + msivp_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					if (!(dlctx = GetFreeDownloader ()))
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
						DownloadUri (dlctx, tile);
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
				if (opacity && *opacity > GetValue (MultiScaleImage::TileFadeProperty)->AsDouble()) {
					cairo_paint_with_alpha (cr, MIN(1.0 - *opacity + GetValue(MultiScaleImage::TileFadeProperty)->AsDouble (), 1.0));
				} else
					cairo_paint (cr);
				cairo_restore (cr);
			}
		}
		layer_to_render++;
	}
	cairo_restore (cr);
	//	cairo_pop_group_to_source (cr);

	DownloaderContext *dlctx;
	if (!(dlctx = GetFreeDownloader ()))
		return;

	//Get the next tile...
	while (from_layer < optimal_layer) {
		from_layer ++;

		double v_tile_w = tile_width * (double)(1 << (layers - from_layer)) / im_w;
		double v_tile_h = tile_height * (double)(1 << (layers - from_layer)) / im_w;
		int i, j;

		for (i = MAX(0, (int)(vp_ox / v_tile_w)); i * v_tile_w < MIN(vp_ox + vp_w, 1.0); i++) {
			if (!(dlctx = GetFreeDownloader ()))
				return;
			for (j = MAX(0, (int)(vp_oy / v_tile_h)); j * v_tile_h < MIN(vp_oy + vp_w * msi_w / msi_h, 1.0 / msi_ar); j++) {
				if (!(dlctx = GetFreeDownloader ()))
					return;
				Uri *tile = new Uri ();
				if (source->get_tile_func (from_layer, i, j, tile, source) && !cache_contains (tile, true)) {
					DownloadUri (dlctx, tile);
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

	//Process the downloaded files
	GList *list;
	DownloaderContext *dlctx;
	for (list = g_list_first (downloaders); list && (dlctx = (DownloaderContext*)list->data); list = list->next) {
		const char *filename;
		if (dlctx->state != DownloaderDone)
			continue;
		if ((filename = dlctx->downloader->getFileDownloader ()->GetDownloadedFile ())) {
			guchar *data;
			int stride;
			GError *error = NULL;
			cairo_surface_t *image = NULL;
			cairo_surface_t *similar = NULL;

			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, &error);
			if (error) {
				printf (error->message);
			} else {
				int *p_width = new int (gdk_pixbuf_get_width (pixbuf));
				int *p_height = new int (gdk_pixbuf_get_height (pixbuf));

				bool has_alpha = gdk_pixbuf_get_n_channels (pixbuf) == 4;
				if (has_alpha) {
					unmultiply_rgba_in_place (pixbuf);
					stride = gdk_pixbuf_get_rowstride (pixbuf);
					data = gdk_pixbuf_get_pixels (pixbuf);
				} else {
					data = expand_rgb_to_argb (pixbuf, &stride);
					g_object_unref (pixbuf);
				}

				image = cairo_image_surface_create_for_data (data,
									     has_alpha ? MOON_FORMAT_ARGB : MOON_FORMAT_RGB,
									     *p_width,
									     *p_height,
									     stride);

				//cairo_surface_set_user_data (image, &width_key, p_width, g_free);
				//cairo_surface_set_user_data (image, &height_key, p_height, g_free);
				//FIXME: set it to 1.0 if the tile doesn't intersect with viewport, and do not start the fadein animation
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
					fadein_sb->Pause ();
				}

				//LOG_MSI ("animating Fade from %f to %f\n\n", GetValue(MultiScaleImage::TileFadeProperty)->AsDouble(), GetValue(MultiScaleImage::TileFadeProperty)->AsDouble() + 0.9);
				double *to = new double (GetValue(MultiScaleImage::TileFadeProperty)->AsDouble() + 0.9);
				fadein_animation->SetFrom (GetValue(MultiScaleImage::TileFadeProperty)->AsDouble());
				fadein_animation->SetTo (*to);

				fadein_sb->Begin();

				similar = cairo_surface_create_similar (cairo_get_target (cr),
									has_alpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR,
									*p_width, *p_height);

				cairo_surface_set_user_data (similar, &full_opacity_at_key, to, g_free);

				cairo_t *temp_cr = cairo_create (similar);
				cairo_set_source_surface (temp_cr, image, 0, 0);
				cairo_pattern_set_filter (cairo_get_source (temp_cr), CAIRO_FILTER_FAST);
				cairo_paint (temp_cr);
				cairo_destroy (temp_cr);
				cairo_surface_destroy (image);
				g_free (data);
			}
			LOG_MSI ("caching %s\n", dlctx->uri->ToString());
			g_hash_table_insert (cache, new Uri(*(dlctx->uri)), similar);
		}
		dlctx->state = DownloaderFree;
	}

	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource*) source;
	bool is_collection = dzits && dzits->IsCollection () && GetSubImages ();
	
	if (!(source = GetSource ())) {
		LOG_MSI ("no sources set, nothing to render\n");
		return;
	}

	if (source->GetImageWidth () < 0 && !is_collection) {
		LOG_MSI ("nothing to render so far...\n");
		//FIXME: check for null cast
		((DeepZoomImageTileSource*)source)->set_parsed_cb (multi_scale_image_handle_parsed, this);
		((DeepZoomImageTileSource*)source)->Download ();
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

DownloaderContext *
MultiScaleImage::GetFreeDownloader ()
{
	guint num_dl = 6;
	DownloaderContext *dlctx;
	GList *list;
	for (list = g_list_first (downloaders); list && (dlctx = (DownloaderContext*)list->data); list = list->next) {
		if (dlctx->state == DownloaderFree)
			return dlctx;
	}

	if (g_list_length (downloaders) < num_dl) {
		dlctx = new DownloaderContext ();
		dlctx->state = DownloaderFree;
		dlctx->uri = NULL;

		Surface* surface = GetSurface ();
		if (!surface) {
			delete dlctx;
			return NULL;
		}

		dlctx->downloader = surface->CreateDownloader ();
		if (!dlctx->downloader) {
			delete dlctx;
			return NULL;
		}
		dlctx->downloader->AddHandler (dlctx->downloader->CompletedEvent, downloader_complete, this);
		dlctx->downloader->AddHandler (dlctx->downloader->DownloadFailedEvent, downloader_failed, this);

		downloaders = g_list_append (downloaders, dlctx);
		return dlctx;
	}
	return NULL;
}

DownloaderContext *
MultiScaleImage::GetDownloaderContext (Downloader *dl)
{
	GList *list;
	DownloaderContext *dlctx;
	for (list = g_list_first (downloaders); list && (dlctx = (DownloaderContext*)list->data); list = list->next) {
		if (dlctx->downloader == dl)
			return dlctx;
	}
	return NULL;
}

void
MultiScaleImage::DownloaderComplete (DownloaderContext *ctx)
{
	LOG_MSI ("dl completed %s\n", ctx->downloader->getFileDownloader ()->GetDownloadedFile ());
	ctx->state = DownloaderDone;

	Invalidate ();
}

void
MultiScaleImage::DownloaderFailed (DownloaderContext *ctx)
{
	LOG_MSI ("dl failed, caching a NULL\n");
	g_hash_table_insert (cache, new Uri(*(ctx->uri)), NULL);
	ctx->state = DownloaderFree;

	Invalidate ();
}

void
MultiScaleImage::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->DownloaderComplete (msi->GetDownloaderContext ((Downloader*)sender));
}

void
MultiScaleImage::downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MultiScaleImage *msi = (MultiScaleImage *) closure;
	msi->DownloaderFailed (msi->GetDownloaderContext ((Downloader*)sender));
}


void
MultiScaleImage::DownloadersAbort ()
{
	GList *list;
	DownloaderContext *dlctx;
	for (list = g_list_first (downloaders); list && (dlctx = (DownloaderContext*)list->data); list = list->next) {
		dlctx->downloader->RemoveHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
		dlctx->downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		dlctx->downloader->SetWriteFunc (NULL, NULL, NULL);
		dlctx->downloader->Abort ();
		dlctx->downloader->unref ();
		dlctx->downloader = NULL;
		if (dlctx->uri)
			delete dlctx->uri;
	}

}

void
MultiScaleImage::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == MultiScaleImage::ViewportOriginProperty) {
		Emit (MultiScaleImage::ViewportChangedEvent);
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::ViewportWidthProperty) {
		//LOG_MSI ("ViewportWidth set to %f\n", args->new_value->AsDouble ());
		Emit (MultiScaleImage::ViewportChangedEvent);
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::TileFadeProperty) {
		//There's 2 options here,
		// - loop all the tiles, update their opacity, and only invalidate a subregion
		// - Invalidate all, and compute the new opacity on the tiles that needs to be rendered.
		//Both options are unfortunately quite expensive :(
		//LOG_MSI ("TileFade changed to %f\n", args->new_value->AsDouble ());
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::SourceProperty) {
		DeepZoomImageTileSource *source = args->new_value ? args->new_value->AsDeepZoomImageTileSource () : NULL;
		if (source) {
			source->set_parsed_cb (multi_scale_image_handle_parsed, this);
			source->Download ();
		}
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
}

void
MultiScaleImage::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (args->GetId () == MultiScaleSubImage::ViewportWidthProperty)
		Invalidate ();
	if (args->GetId () == MultiScaleSubImage::ViewportOriginProperty)
		Invalidate ();
	if (args->GetId () == MultiScaleSubImage::ZIndexProperty)
		subimages_sorted = false;
}

void
MultiScaleImage::EmitMotionFinished ()
{
	printf ("Emitting MotionFinished\n");
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
		zoom_animation = new DoubleAnimation ();
		zoom_animation->SetDuration (Duration::FromSecondsFloat (.4));
		TimelineCollection *tlc = new TimelineCollection ();
		tlc->Add (zoom_animation);
		zoom_sb->SetChildren(tlc);
	} else {
		zoom_sb->Pause ();
	}

	LOG_MSI ("animating zoom from %f to %f\n\n", GetViewportWidth(), value)	
	zoom_animation->SetFrom (GetViewportWidth ());
	zoom_animation->SetTo (value);

	zoom_sb->Begin();
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
		pan_animation = new PointAnimation ();
		pan_animation->SetDuration (Duration::FromSecondsFloat (.4));
		TimelineCollection *tlc = new TimelineCollection ();
		tlc->Add (pan_animation);
		pan_sb->SetChildren(tlc);
	} else
		pan_sb->Pause ();

	pan_animation->SetFrom (GetViewportOrigin());
	pan_animation->SetTo (value);

	pan_sb->Begin ();
}


