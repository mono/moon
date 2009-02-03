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
	subimages = NULL;
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
to_key (int layer, int x, int y)
{
	char key[16];
	sprintf (key, "%dx%dx%d", layer, x, y);
	return g_strdup (key);
}

bool
MultiScaleImage::cache_contains (int layer, int x, int y, bool empty_tiles)
{
	//empty_tiles = TRUE means that this will return true if a tile is present, but NULL
	if (empty_tiles)
		return g_hash_table_lookup_extended (cache, to_key (layer, x, y), NULL, NULL);
	else
		return g_hash_table_lookup (cache, to_key (layer, x, y)) != NULL;
}

void
multi_scale_image_handle_parsed (void *userdata)
{
	MultiScaleImage *msi = (MultiScaleImage*)userdata;
	//if the source is a collection, fill the subimages list
	DeepZoomImageTileSource *source = (DeepZoomImageTileSource *)msi->source;
	if (source) {
		msi->subimages = source->subimages;
	}
	msi->Invalidate ();
}

void
MultiScaleImage::RenderCollection (cairo_t *cr, Region *region)
{
	LOG_MSI ("MSI::RenderCollection\n");

	int i;
	MultiScaleSubImage *sub_image;
	//FIXME: sort the subimages by ZIndex first
	for (i = 0; (sub_image = (MultiScaleSubImage*)g_list_nth_data (subimages, i)); i++) {
		//render if the subimage viewport intersects with this viewport
		if (sub_image->source->GetImageWidth () < 0) {
			LOG_MSI ("skip\n");
			((DeepZoomImageTileSource*)sub_image->source)->set_parsed_cb (multi_scale_image_handle_parsed, this);
			((DeepZoomImageTileSource*)sub_image->source)->Download ();
			continue;
		}
		LOG_MSI ("subimage #%d (%d, %d), vpo(%f %f) vpw %f\n", sub_image->id, sub_image->source->GetImageWidth (), sub_image->source->GetImageHeight (), 
			sub_image->GetViewportOrigin ()->x, sub_image->GetViewportOrigin()->y, sub_image->GetViewportWidth ());
	}
	
}

void
MultiScaleImage::Render (cairo_t *cr, Region *region)
{
//FIXME: only render region


	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource*) source;
	if (dzits && dzits->isCollection && subimages) {
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

	//FIXME: only need to set this once
	SetValue (MultiScaleImage::AspectRatioProperty, Value ((double)source->GetImageWidth () / (double)source->GetImageHeight ()));

	if (!source->get_tile_func) {
		g_warning ("no get_tile_func set\n");
		return;
	}

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
			for (j = MAX(0, (int)((double)vp_oy * im_h / (double)v_tile_h)); j * v_tile_h < vp_oy * im_h + vp_h * im_h && j * v_tile_h < im_h; j++) {
				count++;
				if (cache_contains (from_layer, i, j, false))
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
		int i, j;
		double v_tile_w = tile_width * ldexp (1.0, layers - layer_to_render);
		double v_tile_h = tile_height * ldexp (1.0, layers - layer_to_render);
		for (i = MAX(0, (int)((double)vp_ox * im_w / (double)v_tile_w)); i * v_tile_w < vp_ox * im_w + vp_w * im_w && i * v_tile_w < im_w; i++) {
			for (j = MAX(0, (int)((double)vp_oy * im_h / (double)v_tile_h)); j * v_tile_h < vp_oy * im_h + vp_h * im_h && j * v_tile_h < im_h; j++) {
				cairo_surface_t *image = (cairo_surface_t*)g_hash_table_lookup (cache, to_key (layer_to_render, i, j));
				if (!image)
					continue;
				LOG_MSI ("rendering %d %d %d\n", layer_to_render, i, j);
//				int *p_w = (int*)(cairo_surface_get_user_data (image, &width_key));
//				int *p_h = (int*)(cairo_surface_get_user_data (image, &height_key));
				cairo_save (cr);

				cairo_rectangle (cr, 0, 0, w, h);
				cairo_scale (cr, w / (vp_w * im_w), h / (vp_h * im_h)); //scale to viewport
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

	if (downloading)
		return;

	//Get the next tile...
	while (from_layer < optimal_layer) {
		from_layer ++;

		double v_tile_w = tile_width * ldexp (1.0, layers - from_layer);
		double v_tile_h = tile_height * ldexp (1.0, layers - from_layer);
		int i, j;


		for (i = MAX(0, (int)((double)vp_ox * im_w / (double)v_tile_w)); i * v_tile_w < vp_ox * im_w + vp_w * im_w && i * v_tile_w < im_w; i++) {
			for (j = MAX(0, (int)((double)vp_oy * im_h / (double)v_tile_h)); j * v_tile_h < vp_oy * im_h + vp_h * im_h && j * v_tile_h < im_h; j++) {
				if (!cache_contains (from_layer, i, j, true)) {
					context = to_key (from_layer, i, j);
				
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
	LOG_MSI ("dl failed\n");
	downloading = false;
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
	if (args->property == MultiScaleImage::ViewportOriginProperty) {
		Invalidate ();
	}

	if (args->property == MultiScaleImage::ViewportWidthProperty) {
		Invalidate ();
	}

	if (args->property->GetOwnerType () != Type::MULTISCALEIMAGE) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}

double
MultiScaleImage::GetViewportHeight ()
{
	return GetAspectRatio () * GetActualHeight() * GetViewportWidth () / GetActualWidth (); 
}

