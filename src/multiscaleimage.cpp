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
#include "file-downloader.h"

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
	cache = g_hash_table_new (g_str_hash, g_str_equal);
}

MultiScaleImage::~MultiScaleImage ()
{
	DownloaderAbort ();
}

void
MultiScaleImage::ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY)
{

//	printf ("zoomabout logical %f  (%f, %f)\n", zoomIncrementFactor, zoomCenterLogicalX, zoomCenterLogicalY);
	double width = GetViewportWidth () / zoomIncrementFactor;
	double height = GetViewportHeight () / zoomIncrementFactor;
	SetViewportWidth (width);
	SetViewportOrigin (new Point (zoomCenterLogicalX - width/2.0, zoomCenterLogicalY - height/2.0));
}

Point
MultiScaleImage::ElementToLogicalPoint (Point elementPoint)
{
	return Point (GetViewportOrigin()->x + (double)elementPoint.x * (double)GetViewportWidth () / GetWidth(),
		      GetViewportOrigin()->y + (double)elementPoint.y * (double)GetViewportHeight () / GetHeight ());
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
	}

	if (!downloader)
		return;

	printf ("MSI::DownloadUri %s\n", url);
//	gpointer context = g_strdup ("blah");;
//	downloader->SetContext (context);



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
MultiScaleImage::cache_contains (int layer, int x, int y)
{
	return g_hash_table_lookup (cache, to_key (layer, x, y)) != NULL;
}

void
MultiScaleImage::Render (cairo_t *cr, Region *region)
{
printf ("MSI::Render\n");

//	if (!surface)
//		return;

	if (!(source = GetSource ())) {
		printf ("no sources set, nothing to render\n");
		return;
	}

	if (source->GetImageWidth () < 0) {
		printf ("nothing to render so far...\n");
		//FIXME: need to add a callback so we can invalidate the MSI and render again.
		//Note: source->Download is only used for DeepZoomImageTileSource
		source->Download ();
		return;
	}

	if (!source->get_tile_func) {
		printf ("no get_tile_func set\n");
		return;
	}


	double w = GetWidth ();
	double h = GetHeight ();
	double vp_w = GetViewportWidth ();
	double vp_h = GetViewportHeight ();
	double im_w = (double) source->GetImageWidth ();
	double im_h = (double) source->GetImageHeight ();
	int tile_width = source->GetTileWidth ();
	int tile_height = source->GetTileHeight ();
	double vp_ox = GetViewportOrigin()->x;
	double vp_oy = GetViewportOrigin()->y;
//printf ("vp %f %f %f %f\n", vp_ox, vp_oy, vp_w, vp_h);
//printf ("image %d %d\n", source->GetImageWidth (), source->GetImageHeight ());

	int layers;
	frexp (MAX (im_w, im_h), &layers);

	//optimal layer for this...
	//FIXME: need to count AspectRatio too
	int to_layer = MAX (0, layers + 1 - ceil (GetViewportWidth () / GetWidth()));
	printf ("number of layers: %d\toptimal layer for this: %d\n", layers, to_layer);

	//figuring what layer is ready to be rendered
	int layer_to_render = to_layer;
	while (layer_to_render >= 0) {
		if (cache_contains (layer_to_render, 0, 0))
			break;
		printf ("no layer %d in cache\n", layer_to_render);
		layer_to_render --;
	}

	//render here
	if (layer_to_render >= 0) {
		guchar *data;
		int stride;

		GError *error = NULL;
		char* filename = (char*)g_hash_table_lookup (cache, to_key (layer_to_render, 0, 0));
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, &error);
		if (error) {
			printf (error->message);
			return;
		}


		stride = gdk_pixbuf_get_rowstride (pixbuf);
		data = expand_rgb_to_argb (pixbuf, &stride);

//printf ("pb %d %d\n", gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height(pixbuf));

		cairo_surface_t *image = cairo_image_surface_create_for_data (data,
										MOON_FORMAT_RGB,
										gdk_pixbuf_get_width (pixbuf),
										gdk_pixbuf_get_height (pixbuf),
										stride);

		cairo_save (cr);

		cairo_rectangle (cr, 0, 0, w, h);
		cairo_scale (cr, w / vp_w, h / vp_h);
//		cairo_scale (cr, vp_w / w, vp_h / h);
		cairo_translate (cr, -vp_ox, -vp_oy);
//		cairo_rectangle (cr, vp_ox, vp_oy, vp_w, vp_h);
		cairo_scale (cr, (double)source->GetImageWidth() / gdk_pixbuf_get_width (pixbuf), (double)source->GetImageHeight () / gdk_pixbuf_get_height (pixbuf)); 
		cairo_set_source_surface (cr, image, 0, 0);

		cairo_fill (cr);
		cairo_restore (cr);

		g_object_unref (pixbuf);
		cairo_surface_destroy (image);
	}

	if (layer_to_render < to_layer) {
		//Get the next tiles...
		layer_to_render ++;

		double v_tile_w = tile_width * ldexp (1.0, layers - layer_to_render);
		double v_tile_h = tile_height * ldexp (1.0, layers - layer_to_render);
//		double zoom = w / (double)vp_w;


		int i, j;

		context = to_key (layer_to_render, 0, 0);

		for (i = (int)((double)vp_ox / (double)v_tile_w); i * v_tile_w < vp_ox + vp_w; i++) {
			for (j = (int)((double)vp_oy / (double)v_tile_h); j * v_tile_h < vp_oy + vp_h; j++) {
				const char* ret = g_strdup ((const char*)source->get_tile_func (layer_to_render, 2, 3));
				if (!ret)
					return;
				DownloadUri (ret);
			}
		}
	}
}

void
MultiScaleImage::DownloaderComplete ()
{
	char *filename;

	if (!(filename = g_strdup(downloader->getFileDownloader ()->GetDownloadedFile ())))
		return;

//	char* context = (char*)downloader->GetContext ();
	printf ("dl completed %s\n", filename);

	//adding to cache. the key should be passed through context, but it's crashing :/
	g_hash_table_insert (cache, g_strdup(context), filename);

	Invalidate ();
}



void
MultiScaleImage::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((MultiScaleImage *) closure)->DownloaderComplete ();
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

	if (args->property->GetOwnerType () != Type::MULTISCALEIMAGE) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}

double
MultiScaleImage::GetViewportHeight ()
{
	return GetAspectRatio () * GetHeight() * GetViewportWidth () / GetWidth (); 
}

