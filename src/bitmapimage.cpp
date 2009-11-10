/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bitmapimage.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <glib/gstdio.h>
#include <fcntl.h>
#include <errno.h>

#include "application.h"
#include "bitmapimage.h"
#include "deployment.h"
#include "runtime.h"
#include "uri.h"
#include "debug.h"

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
		a = *(p+3);	\
		r = *(p+2);	\
		g = *(p+1);	\
		b = *(p+0);	\
	} G_STMT_END
#include "alpha-premul-table.inc"

//
// Expands RGB to ARGB allocating new buffer for it.
//
static gpointer
expand_rgb_to_argb (GdkPixbuf *pixbuf)
{
	guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
	guchar *p;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);
	int stride = w * 4;
	guchar *data = (guchar *) g_malloc (stride * h);
	guchar *out;

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
		out = data + y * (stride);
		for (int x = 0; x < w; x ++) {
			guchar r, g, b;

			get_pixel_bgr_p (p, b, g, r);
			set_pixel_bgra (out, 0, r, g, b, 255);

			p += 3;
			out += 4;
		}
	}

	return (gpointer) data;
}

//
// Converts RGBA unmultiplied alpha to ARGB pre-multiplied alpha.
//
static gpointer
premultiply_rgba (GdkPixbuf *pixbuf)
{
	guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
	guchar *p;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);
	int stride = w * 4;
	guchar *data = (guchar *) g_malloc (stride * h);
	guchar *out;

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
		out = data + y * (stride);
		for (int x = 0; x < w; x ++) {
			guchar r, g, b, a;

			get_pixel_bgra (p, b, g, r, a);

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
			set_pixel_bgra (out, 0, r, g, b, a);

			p += 4;
			out += 4;
		}
	}

	return (gpointer) data;
}

BitmapImage::BitmapImage ()
{
	SetObjectType (Type::BITMAPIMAGE);
	downloader = NULL;
	loader = NULL;
	gerror = NULL;
	part_name = NULL;
	get_res_aborter = NULL;
	policy = MediaPolicy;
}

BitmapImage::~BitmapImage ()
{
	if (downloader)
		downloader->unref ();

	if (part_name)
		g_free (part_name);

	if (get_res_aborter)
		delete get_res_aborter;

	CleanupLoader ();
}

void
BitmapImage::Dispose ()
{
	Abort ();
	BitmapSource::Dispose ();
}

void
BitmapImage::Abort ()
{
	if (downloader) {
		CleanupDownloader ();
		downloader->Abort ();
		downloader->unref ();
		downloader = NULL;
	}

	if (get_res_aborter)
		get_res_aborter->Cancel ();	
}

void
BitmapImage::uri_source_changed_callback (EventObject *user_data)
{
	BitmapImage *image = (BitmapImage *) user_data;
	image->UriSourceChanged ();
}

static void
resource_notify (NotifyType type, gint64 args, gpointer user_data)
{
	BitmapImage *media = (BitmapImage *) user_data;
	
	if (type == NotifyProgressChanged)
		media->SetProgress ((double)(args)/100.0);
	else if (type == NotifyFailed)
		media->DownloaderFailed ();
	else if (type == NotifyCompleted)
		media->DownloaderComplete ();
}

void
BitmapImage::UriSourceChanged ()
{
	Surface *surface = Deployment::GetCurrent ()->GetSurface ();
	Application *current = Application::GetCurrent ();
	Uri *uri = GetUriSource ();
	
	if (surface == NULL) {
		SetBitmapData (NULL);
		return;
	}

	if (current && uri) {
		if (get_res_aborter)
			delete get_res_aborter;
		get_res_aborter = new Cancellable ();
		current->GetResource (GetResourceBase(), uri, resource_notify, pixbuf_write, policy, get_res_aborter, this);
	}
}

void
BitmapImage::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType () != Type::BITMAPIMAGE) {
		BitmapSource::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == BitmapImage::UriSourceProperty) {
		Uri *uri = args->GetNewValue () ? args->GetNewValue ()->AsUri () : NULL;

		Abort ();

		if (Uri::IsNullOrEmpty (uri)) {
			SetBitmapData (NULL);
		} else if (uri->IsInvalidPath ()) {
			MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, 0, "invalid path found in uri");
			SetBitmapData (NULL);
		} else {
			AddTickCall (uri_source_changed_callback);
		}
	} else if (args->GetId () == BitmapImage::ProgressProperty) {
		Emit (DownloadProgressEvent, new DownloadProgressEventArgs (GetProgress ()));
	}

	NotifyListenersOfPropertyChange (args, error);
}

bool
BitmapImage::ValidateDownloadPolicy ()
{
	Surface *surface = Deployment::GetCurrent ()->GetSurface ();
	Uri *uri = GetUriSource ();
	const char *location;
	
	if (!uri)
		return true;
	
	if (!(location = GetDeployment ()->GetXapLocation ()))
		location = surface ? surface->GetSourceLocation () : NULL;
	
	return Downloader::ValidateDownloadPolicy (location, uri, policy);
}

void
BitmapImage::SetDownloader (Downloader *downloader, Uri *uri, const char *part_name)
{
	Abort ();
	
	this->downloader = downloader;
	this->part_name = g_strdup (part_name);

	downloader->ref();

	downloader->AddHandler (Downloader::DownloadProgressChangedEvent, downloader_progress_changed, this);
	downloader->AddHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
	downloader->AddHandler (Downloader::CompletedEvent, downloader_complete, this);

	if (downloader->Completed ()) {
		DownloaderComplete ();
	} else {
		if (!downloader->Started () && uri) {
			downloader->Open ("GET", uri, policy);
			downloader->SetStreamFunctions (pixbuf_write, NULL, this);
			downloader->Send ();
		}
	}
}

void
BitmapImage::CleanupDownloader ()
{
	downloader->RemoveHandler (Downloader::DownloadProgressChangedEvent, downloader_progress_changed, this);
	downloader->RemoveHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
	downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
}

void
BitmapImage::downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	BitmapImage *media = (BitmapImage *) closure;

	media->DownloaderProgressChanged ();
}

void
BitmapImage::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	BitmapImage *media = (BitmapImage *) closure;

	media->DownloaderComplete ();
}

void
BitmapImage::downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	BitmapImage *media = (BitmapImage *) closure;

	media->DownloaderFailed ();
}

void
BitmapImage::DownloaderProgressChanged ()
{
	SetProgress (downloader->GetDownloadProgress ());
}

void
BitmapImage::DownloaderComplete ()
{
	MoonError moon_error;

	if (downloader)
		CleanupDownloader ();

	SetProgress (1.0);

	if (downloader && loader == NULL) {
		char *filename = downloader->GetDownloadedFilename (part_name);

		if (filename == NULL) {
			guchar *buffer = (guchar *)downloader->GetBuffer ();

			if (buffer == NULL) {
				MoonError::FillIn (&moon_error, MoonError::EXCEPTION, 4001, "downloader buffer was NULL");
				goto failed;
			}

			PixbufWrite (buffer, 0, downloader->GetSize ());
		} else {
			guchar b[4096];
			int offset = 0;
			ssize_t n;
			int fd;

			if ((fd = g_open (filename, O_RDONLY)) == -1) {
				MoonError::FillIn (&moon_error, MoonError::EXCEPTION, 4001, "failed to open file");
				goto failed;
			}
	
			do {
				do {
					n = read (fd, b, sizeof (b));
				} while (n == -1 && errno == EINTR);

				if (n == -1) break;

				PixbufWrite (b, offset, n);

				offset += n;
			} while (n > 0 && !gerror);

			close (fd);

			if (gerror) {
				MoonError::FillIn (&moon_error, MoonError::EXCEPTION, 4001, gerror->message);
				goto failed;
			}
		}
	}

	if (downloader) {
		downloader->unref ();
		downloader = NULL;
	}

	PixmapComplete ();

	return;
failed:
	downloader->unref ();
	downloader = NULL;

	if (loader)
		gdk_pixbuf_loader_close (loader, NULL);
	CleanupLoader ();

	Emit (ImageFailedEvent, new ImageErrorEventArgs (moon_error));
}


void
BitmapImage::PixmapComplete ()
{
	MoonError moon_error;

	SetProgress (1.0);

	if (!loader) goto failed;

	gdk_pixbuf_loader_close (loader, gerror == NULL ? &gerror : NULL);

	if (gerror) {
		MoonError::FillIn (&moon_error, MoonError::EXCEPTION, 4001, gerror->message);
		goto failed;
	}

	{
		GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
		
		if (pixbuf == NULL) {
			MoonError::FillIn (&moon_error, MoonError::EXCEPTION, 4001, "failed to create image data");
			goto failed;
		}

		SetPixelWidth (gdk_pixbuf_get_width (pixbuf));
		SetPixelHeight (gdk_pixbuf_get_height (pixbuf));

		if (gdk_pixbuf_get_n_channels (pixbuf) == 4) {
			SetPixelFormat (PixelFormatPbgra32);
			SetBitmapData (premultiply_rgba (pixbuf));
		} else {
			SetPixelFormat (PixelFormatBgr32);
			SetBitmapData (expand_rgb_to_argb (pixbuf));
		}

		Invalidate ();

		g_object_unref (loader);
		loader = NULL;

		Emit (ImageOpenedEvent, new RoutedEventArgs ());

		return;
	}

failed:
	CleanupLoader ();

	Emit (ImageFailedEvent, new ImageErrorEventArgs (moon_error));
}

void
BitmapImage::DownloaderFailed ()
{
	Abort ();
	Emit (ImageFailedEvent, new ImageErrorEventArgs (MoonError (MoonError::EXCEPTION, 4001, "downloader failed")));
}

void
BitmapImage::CleanupLoader ()
{
	SetPixelWidth (0);
	SetPixelHeight (0);

	if (loader) {
		g_object_unref (loader);
		loader = NULL;
	}
	if (gerror) {
		g_error_free (gerror);
		gerror = NULL;
	}
}

void
BitmapImage::CreateLoader (unsigned char *buffer)
{
	if (!(moonlight_flags & RUNTIME_INIT_ALL_IMAGE_FORMATS)) {
		// 89 50 4E 47 == png magic
		if (buffer[0] == 0x89)
			loader = gdk_pixbuf_loader_new_with_type ("png", NULL);
		// ff d8 ff e0 == jfif magic
		else if (buffer[0] == 0xff)
			loader = gdk_pixbuf_loader_new_with_type ("jpeg", NULL);

		else {
			Abort ();
			Emit (ImageFailedEvent, new ImageErrorEventArgs (MoonError (MoonError::EXCEPTION, 4001, "unsupported image type")));
		}
	} else {
		loader = gdk_pixbuf_loader_new ();
	}
}

void
BitmapImage::PixbufWrite (gpointer buffer, gint32 offset, gint32 n)
{

	if (loader == NULL && offset == 0)
		CreateLoader ((unsigned char *)buffer);

	if (loader != NULL && gerror == NULL) {
		gdk_pixbuf_loader_write (GDK_PIXBUF_LOADER (loader), (const guchar *)buffer, n, &gerror);
	}
}

void
BitmapImage::pixbuf_write (void *buffer, gint32 offset, gint32 n, gpointer data)
{
	BitmapImage *source = (BitmapImage *) data;

	source->PixbufWrite ((unsigned char *)buffer, offset, n);
}
