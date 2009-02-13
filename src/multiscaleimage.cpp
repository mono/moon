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
//- drop the friends
//- blend new layers
//- animation for VP changes
//- if opacity is not 1.0, stack the layers internally, then paint at once
//- fix the leaks
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

MultiScaleImage::MultiScaleImage ()
{
//	static bool init = true;
//	if (init) {
//		init = false;
//		MultiScaleImage::SubImagesProperty->SetValueValidator (MultiScaleSubImageCollectionValidator);	
//	}
	SetObjectType (Type::MULTISCALEIMAGE); 
	source = NULL;
	downloader = NULL;
	filename = NULL;
	cache = g_hash_table_new (g_str_hash, g_str_equal);
	downloading = false;
}

MultiScaleImage::~MultiScaleImage ()
{
	DownloaderAbort ();
}

void
MultiScaleImage::ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY)
{

	LOG_MSI ("zoomabout logical %f  (%f, %f)\n", zoomIncrementFactor, zoomCenterLogicalX, zoomCenterLogicalY);
	double width = GetViewportWidth () / zoomIncrementFactor;
	double height = GetViewportHeight () / zoomIncrementFactor;
	SetValue (MultiScaleImage::ViewportWidthProperty, Value (width));
	if (!isnan(zoomCenterLogicalX) && !isnan(zoomCenterLogicalY))
		SetValue (MultiScaleImage::ViewportOriginProperty, Value (Point (zoomCenterLogicalX - width/2.0, zoomCenterLogicalY - height/2.0)));
	Invalidate ();
}

Point
MultiScaleImage::ElementToLogicalPoint (Point elementPoint)
{
	return Point (GetViewportOrigin()->x + (double)elementPoint.x * GetViewportWidth () / GetActualWidth (),
		      GetViewportOrigin()->y + (double)elementPoint.y * GetViewportHeight () / GetActualHeight ());
}

void
MultiScaleImage::DownloadUri (const char* url)
{
	Uri *uri = new Uri ();

	Surface* surface = GetSurface ();
	if (!surface)
		return;

	if (!(uri->Parse (url)))
		return;

	if (!downloader) {
		downloader = surface->CreateDownloader ();
		downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
		downloader->AddHandler (downloader->DownloadFailedEvent, downloader_failed, this);
	}

	if (!downloader)
		return;

	LOG_MSI ("MSI::DownloadUri %s\n", url);

	downloader->Open ("GET", uri->ToString (), NoPolicy);


	downloader->Send ();

	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete ();
	} else
		downloader->Send ();
	delete uri;
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

char *
to_key (int subimage_id, int layer, int x, int y)
{
	char key[32];
	sprintf (key, "%dx%dx%dx%d", subimage_id, layer, x, y);
	return g_strdup (key);
}

bool
MultiScaleImage::cache_contains (int layer, int x, int y, int subimage_id ,bool empty_tiles)
{
	//empty_tiles = TRUE means that this will return true if a tile is present, but NULL
	if (empty_tiles)
		return g_hash_table_lookup_extended (cache, to_key (subimage_id, layer, x, y), NULL, NULL);
	else
		return g_hash_table_lookup (cache, to_key (subimage_id, layer, x, y)) != NULL;
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
			if (!msi->GetSubImageCollection())
				msi->SetValue (MultiScaleImage::SubImageCollectionProperty, new MultiScaleSubImageCollection ());

			msi->GetSubImageCollection()->Add (si);
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

	double msi_x = GetViewportOrigin()->x;
	double msi_y = GetViewportOrigin()->y;
	double msi_w = GetViewportWidth();
	double msi_ar = GetAspectRatio();

	int tile_width = source->GetTileWidth ();
	int tile_height = source->GetTileHeight ();

	Rect viewport = Rect (msi_x, msi_y, msi_w, msi_w/msi_ar);

	//FIXME: sort the subimages by ZIndex first
	CollectionIterator *iter = GetSubImageCollection()->GetIterator();
	Value *val;
	int error;

	while (iter->Next () && (val = iter->GetCurrent(&error))) {
		MultiScaleSubImage *sub_image = val->AsMultiScaleSubImage ();
		//if the subimage is unparsed, trigger the download
		//FIXME: THIS NOT REQUIRED FOR LAYERS << MaxTileLayer
//		if (sub_image->source->GetImageWidth () < 0) {
//			((DeepZoomImageTileSource*)sub_image->source)->set_parsed_cb (multi_scale_subimage_handle_parsed, this);
//			((DeepZoomImageTileSource*)sub_image->source)->Download ();
//			continue;
//		}

		double widget_w = GetActualWidth ();
		double widget_h = GetActualHeight ();
		double sub_x = sub_image->GetViewportOrigin()->x;
		double sub_y = sub_image->GetViewportOrigin()->y;
		double sub_w = sub_image->GetViewportWidth();
		double sub_ar = sub_image->GetAspectRatio();

		//render if the subimage viewport intersects with this viewport
		LOG_MSI ("subimage #%d (%ld, %ld), vpo(%f %f) vpw %f\n", sub_image->id, sub_image->source->GetImageWidth (), sub_image->source->GetImageHeight (), 
			sub_image->GetViewportOrigin ()->x, sub_image->GetViewportOrigin()->y, sub_image->GetViewportWidth ());

		//expressing the subimage viewport in main viewport coordinates.
		Rect sub_vp = Rect (-sub_x / sub_w, -sub_y / sub_w, 1.0/sub_w, 1.0/(sub_ar * sub_w));

		//NOTE, we could reuse the same routine to render single dz images, by setting the sub_vp to an appropriate value

		LOG_MSI ("viewport\t%f\t%f\t%f\t%f\n", viewport.x, viewport.y, viewport.width, viewport.height);
		LOG_MSI ("sub_vp  \t%f\t%f\t%f\t%f\n", sub_vp.x, sub_vp.y, sub_vp.width, sub_vp.height);
		if (!sub_vp.IntersectsWith (viewport))
			continue;


		LOG_MSI ("Intersects with main viewport...rendering\n");

		int layers;
		frexp (MAX (sub_image->source->GetImageWidth(), sub_image->source->GetImageHeight()), &layers);

		int optimal_layer;
		frexp (widget_w / (sub_w * msi_w), &optimal_layer); 
		optimal_layer = MIN (optimal_layer, layers);
		LOG_MSI ("number of layers: %d\toptimal layer for this: %d\n", layers, optimal_layer);

		int to_layer = -1;
		int from_layer = optimal_layer;	
		while (from_layer >= 0) {
			int count = 0;
			int found = 0;

			//in msi relative coord
			double v_tile_w = tile_width * ldexp (1.0, layers - from_layer) * sub_vp.width / (double)sub_image->source->GetImageWidth ();
			double v_tile_h = tile_height * ldexp (1.0, layers - from_layer) * sub_vp.width / (double)sub_image->source->GetImageWidth ();
			//LOG_MSI ("virtual tile size at layer %d; %fx%f\n", from_layer, v_tile_w, v_tile_h);

			int i, j;
			for (i = (int)((MAX(msi_x, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msi_x + msi_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				for (j = (int)((MAX(msi_y, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msi_y + msi_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					//LOG_MSI ("TILE %d %d %d %d\n", sub_image->id, from_layer, i, j);
					count++;
					if (cache_contains (from_layer, i, j, sub_image->id, false))
						found ++;
				}
			}
			if (found > 0 && to_layer < from_layer)
				to_layer = from_layer;
			if (found == count)
				break;

			from_layer --;
		}
	
		//render here
		LOG_MSI ("rendering layers from %d to %d\n", from_layer, to_layer);
		int layer_to_render = from_layer;
		while (from_layer > 0 && layer_to_render <= to_layer) {
			double v_tile_w = tile_width * ldexp (1.0, layers - layer_to_render) * sub_vp.width / (double)sub_image->source->GetImageWidth ();
			double v_tile_h = tile_height * ldexp (1.0, layers - layer_to_render) * sub_vp.width / (double)sub_image->source->GetImageWidth ();

			int i, j;
			for (i = (int)((MAX(msi_x, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msi_x + msi_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				for (j = (int)((MAX(msi_y, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msi_y + msi_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					cairo_surface_t *image = (cairo_surface_t*)g_hash_table_lookup (cache, to_key (sub_image->id, layer_to_render, i, j));
					if (!image)
						continue;
					LOG_MSI ("rendering %d %d %d %d\n", sub_image->id, layer_to_render, i, j);
					cairo_save (cr);

					cairo_rectangle (cr, 0, 0, widget_w, widget_h);
					cairo_clip (cr);
					cairo_scale (cr, widget_w / msi_w, widget_w / msi_w); //scale to widget
					cairo_translate (cr, -msi_x + sub_vp.x + i * v_tile_w, -msi_y + sub_vp.y + j* v_tile_h);
					//cairo_scale (cr, v_tile_w / ldexp (1.0, layer_to_render), v_tile_w / ldexp (1.0, layer_to_render)); //scale to viewport

					//scale to viewport
					cairo_scale (cr, 1.0/sub_image->source->GetImageWidth(), 1.0/sub_image->source->GetImageWidth());
					cairo_scale (cr, sub_vp.width * ldexp(1.0, layers - layer_to_render), sub_vp.width * ldexp (1.0, layers - layer_to_render));

					cairo_set_source_surface (cr, image, 0, 0);
//
					cairo_paint_with_alpha (cr, sub_image->GetOpacity ());
					cairo_restore (cr);

				}
			}
			layer_to_render++;
		}

#if FALSE
		LOG_MSI ("rendering from x = %f to %f\n", MAX(msi_x, sub_vp.x), MIN(msi_x + msi_w, sub_vp.x + sub_vp.width));
		LOG_MSI ("rendering from y = %f to %f\n", MAX(msi_y, sub_vp.y), MIN(msi_y + msi_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar));

		cairo_save (cr);
		cairo_set_source_rgba (cr, 1, 0, 0, .2);

		cairo_rectangle (cr,
			widget_w / msi_w * (-msi_x + MAX(msi_x, sub_vp.x)),
			widget_w / msi_w * (-msi_y + MAX(msi_y, sub_vp.y)),
			widget_w / msi_w * (MIN(msi_x + msi_w, sub_vp.x + sub_vp.width) - MAX(msi_x, sub_vp.x)),
			widget_w / msi_w * (MIN(msi_y + msi_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - MAX(msi_y, sub_vp.y)));
		cairo_fill (cr);
		cairo_restore (cr);
#endif

		if (downloading)
			continue;

		//Get the next tile...
		while (from_layer < optimal_layer) {
			from_layer ++;

			double v_tile_w = tile_width * ldexp (1.0, layers - from_layer) * sub_vp.width / (double)sub_image->source->GetImageWidth ();
			double v_tile_h = tile_height * ldexp (1.0, layers - from_layer) * sub_vp.width / (double)sub_image->source->GetImageWidth ();

			int i, j;
			for (i = (int)((MAX(msi_x, sub_vp.x) - sub_vp.x)/v_tile_w); i * v_tile_w < MIN(msi_x + msi_w, sub_vp.x + sub_vp.width) - sub_vp.x;i++) {
				for (j = (int)((MAX(msi_y, sub_vp.y) - sub_vp.y)/v_tile_h); j * v_tile_h < MIN(msi_y + msi_w/msi_ar, sub_vp.y + sub_vp.width/sub_ar) - sub_vp.y;j++) {
					if (!cache_contains (from_layer, i, j, sub_image->id, true)) {
						context = to_key (sub_image->id, from_layer, i, j);

						const char* ret = g_strdup ((const char*)source->get_tile_func (from_layer, i, j, sub_image->source));
						if (ret) {
							downloading = true;
							DownloadUri (ret);
							return;
						} else {
							LOG_MSI ("caching a NULL %s\n", context);
							g_hash_table_insert (cache, g_strdup(context), NULL);
						}
					}
				}
			}
		}
	}
}

void
MultiScaleImage::Render (cairo_t *cr, Region *region, bool path_only)
{
//FIXME: only render region


	//if there's a downloaded file pending, cache it
	if (filename) {
		guchar *data;
		int stride;
		GError *error = NULL;
		cairo_surface_t *image = NULL;

		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, &error);
		filename = NULL;
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

			//printf ("pb %d %d\n", *p_width, *p_height);

			image = cairo_image_surface_create_for_data (data,
								     has_alpha ? MOON_FORMAT_ARGB : MOON_FORMAT_RGB,
								     *p_width,
								     *p_height, 
								     stride);
			cairo_surface_set_user_data (image, &width_key, p_width, g_free);
			cairo_surface_set_user_data (image, &height_key, p_height, g_free);
		}
		LOG_MSI ("caching %s\n", context);
		g_hash_table_insert (cache, g_strdup(context), image);
	}

	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource*) source;
	if (dzits && dzits->IsCollection() && GetSubImageCollection ()) {
		//Let's render collection in a different method to not break this one right now
		RenderCollection (cr, region);
		return;
	}
	LOG_MSI ("MSI::Render\n");


//	if (!surface)
//		return;

	if (!(source = GetSource ())) {
		LOG_MSI ("no sources set, nothing to render\n");
		return;
	}

	if (source->GetImageWidth () < 0) {
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


	double w = GetActualWidth ();
	double h = GetActualHeight ();
	double vp_w = GetViewportWidth ();
	double vp_h = GetViewportHeight ();
	double im_w = (double) source->GetImageWidth ();
	double im_h = (double) source->GetImageHeight ();
	int tile_width = source->GetTileWidth ();
	int tile_height = source->GetTileHeight ();
	double vp_ox = GetViewportOrigin()->x;
	double vp_oy = GetViewportOrigin()->y;
//printf ("vp %f %f %f %f\n", vp_ox, vp_oy, vp_w, vp_h);
//printf ("image %f %f\n", im_w, im_h);
//printf ("widget %f %f\n", w, h);

	int layers;
	frexp (MAX (im_w, im_h), &layers);

	//optimal layer for this... aka "best viewed at"
	int optimal_layer;
	frexp (w / vp_w, &optimal_layer);
	optimal_layer = MIN (optimal_layer, layers);
	LOG_MSI ("number of layers: %d\toptimal layer for this: %d\n", layers, optimal_layer);

	//We have to figure all the layers that we'll have to render:
	//- from_layer is the highest COMPLETE layer that we can display
	//- to_layer is the highest PARTIAL layer that we can display

	int to_layer = -1;
	int from_layer = optimal_layer;

	while (from_layer >= 0) {
		int count = 0;
		int found = 0;
		double v_tile_w = tile_width * ldexp (1.0, layers - from_layer);
		double v_tile_h = tile_height * ldexp (1.0, layers - from_layer);
		int i, j;

		for (i = MAX(0, (int)((double)vp_ox * im_w / (double)v_tile_w)); i * v_tile_w < vp_ox * im_w + vp_w * im_w && i * v_tile_w < im_w; i++) {
			for (j = MAX(0, (int)((double)vp_oy * im_h / (double)v_tile_h)); j * v_tile_h < vp_oy * im_h + vp_h * im_w && j * v_tile_h < im_h; j++) {
				count++;
				if (cache_contains (from_layer, i, j, 0, false))
					found ++;
			}
		}
		if (found > 0 && to_layer < from_layer)
			to_layer = from_layer;
		if (found == count)
			break;
		from_layer --;
	}

	//render here
	cairo_push_group (cr);
	LOG_MSI ("rendering layers from %d to %d\n", from_layer, to_layer);
	int layer_to_render = from_layer;
	while (from_layer > 0 && layer_to_render <= to_layer) {
		int i, j;
		double v_tile_w = tile_width * ldexp (1.0, layers - layer_to_render);
		double v_tile_h = tile_height * ldexp (1.0, layers - layer_to_render);
		for (i = MAX(0, (int)((double)vp_ox * im_w / (double)v_tile_w)); i * v_tile_w < vp_ox * im_w + vp_w * im_w && i * v_tile_w < im_w; i++) {
			for (j = MAX(0, (int)((double)vp_oy * im_h / (double)v_tile_h)); j * v_tile_h < vp_oy * im_h + vp_h * im_w && j * v_tile_h < im_h; j++) {
				cairo_surface_t *image = (cairo_surface_t*)g_hash_table_lookup (cache, to_key (0, layer_to_render, i, j));
				if (!image)
					continue;
				LOG_MSI ("rendering %d %d %d\n", layer_to_render, i, j);
//				int *p_w = (int*)(cairo_surface_get_user_data (image, &width_key));
//				int *p_h = (int*)(cairo_surface_get_user_data (image, &height_key));
				cairo_save (cr);

				cairo_rectangle (cr, 0, 0, w, h);
				cairo_clip (cr);
				cairo_scale (cr, w / (vp_w * im_w), w / (vp_w * im_w)); //scale to viewport
				cairo_translate (cr, -vp_ox * im_w + i * v_tile_w, -vp_oy * im_h+ j * v_tile_h);
				//cairo_scale (cr, im_w / (double)*p_w, im_h / (double)*p_h); //scale to image size
				cairo_scale (cr, ldexp (1.0, layers - layer_to_render), ldexp (1.0, layers - layer_to_render)); //scale to image size
				cairo_set_source_surface (cr, image, 0, 0);
				cairo_fill (cr);
				cairo_restore (cr);

			}
		}
		layer_to_render++;
	}
	cairo_pop_group_to_source (cr);
	cairo_rectangle (cr, 0, 0, w, h);
	cairo_clip (cr);
	cairo_paint (cr);


	if (downloading)
		return;

	//Get the next tile...
	while (from_layer < optimal_layer) {
		from_layer ++;

		double v_tile_w = tile_width * ldexp (1.0, layers - from_layer);
		double v_tile_h = tile_height * ldexp (1.0, layers - from_layer);
		int i, j;


		for (i = MAX(0, (int)((double)vp_ox * im_w / (double)v_tile_w)); i * v_tile_w < vp_ox * im_w + vp_w * im_w && i * v_tile_w < im_w; i++) {
			for (j = MAX(0, (int)((double)vp_oy * im_h / (double)v_tile_h)); j * v_tile_h < vp_oy * im_h + vp_h * im_w && j * v_tile_h < im_h; j++) {
				if (!cache_contains (from_layer, i, j, 0, true)) {
					context = to_key (0, from_layer, i, j);
				
					const char* ret = g_strdup ((const char*)source->get_tile_func (from_layer, i, j, source));
					if (ret) {
						downloading = true;
						DownloadUri (ret);
						return;
					} else {
						LOG_MSI ("caching a NULL %s\n", context);
						g_hash_table_insert (cache, g_strdup(context), NULL);
					}
				}
			}
		}
	}
}

void
MultiScaleImage::DownloaderComplete ()
{
	if (filename)
		g_free (filename);

	if (!(filename = g_strdup(downloader->getFileDownloader ()->GetDownloadedFile ())))
		return;

	LOG_MSI ("dl completed %s\n", filename);
	downloading = false;

	Invalidate ();
}

void
MultiScaleImage::DownloaderFailed ()
{
	LOG_MSI ("dl failed, caching a NULL\n");
	g_hash_table_insert (cache, g_strdup(context), NULL);
	downloading = false;

	Invalidate ();
}

void
MultiScaleImage::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((MultiScaleImage *) closure)->DownloaderComplete ();
}

void
MultiScaleImage::downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((MultiScaleImage *) closure)->DownloaderFailed ();
}


void
MultiScaleImage::DownloaderAbort ()
{
	if (downloader) {
//		downloader->RemoveHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
		downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->SetWriteFunc (NULL, NULL, NULL);
		downloader->Abort ();
		downloader->unref ();
		g_free (part_name);
		downloader = NULL;
		part_name = NULL;
	}
}

void
MultiScaleImage::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->GetId () == MultiScaleImage::ViewportOriginProperty) {
		Invalidate ();
	}

	if (args->GetId () == MultiScaleImage::ViewportWidthProperty) {
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
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}

double
MultiScaleImage::GetViewportHeight ()
{
	return GetViewportWidth () / GetAspectRatio ();
}

