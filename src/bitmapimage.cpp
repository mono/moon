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
#include "factory.h"

namespace Moonlight {

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
expand_rgb_to_argb (MoonPixbuf *pixbuf)
{
	guchar *pb_pixels = pixbuf->GetPixels ();
	guchar *p;
	int w = pixbuf->GetWidth ();
	int h = pixbuf->GetHeight ();
	int stride = w * 4;
	guchar *data = (guchar *) g_malloc (stride * h);
	guchar *out;

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * pixbuf->GetRowStride ();
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
premultiply_rgba (MoonPixbuf *pixbuf)
{
	guchar *pb_pixels = pixbuf->GetPixels ();
	guchar *p;
	int w = pixbuf->GetWidth ();
	int h = pixbuf->GetHeight ();
	int stride = w * 4;
	guchar *data = (guchar *) g_malloc (stride * h);
	guchar *out;

	for (int y = 0; y < h; y ++) {
		p = pb_pixels + y * pixbuf->GetRowStride ();
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
	moon_error = NULL;
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
	Application *app = Application::GetCurrent ();
	const Uri *uri = GetUriSource ();
	
	if (surface == NULL) {
		SetBitmapData (NULL, false);
		return;
	}
	
	if (app && uri) {
		SetProgress (0.0);
		if (get_res_aborter)
			delete get_res_aborter;
		get_res_aborter = new Cancellable ();
		app->GetResource (GetResourceBaseRecursive (), uri, resource_notify, pixbuf_write, policy,
				  HttpRequest::DisableFileStorage, get_res_aborter, this);
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
		const Uri *uri = args->GetNewValue () ? args->GetNewValue ()->AsUri () : NULL;

		Abort ();

		if (Uri::IsNullOrEmpty (uri)) {
			SetBitmapData (NULL, false);
		} else if (uri->IsInvalidPath ()) {
			if (IsBeingParsed ())
				MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, 0, "invalid path found in uri");
			SetBitmapData (NULL, false);
		} else {
			AddTickCall (uri_source_changed_callback);
		}
	} else if (args->GetId () == BitmapImage::ProgressProperty) {
		if (HasHandlers (DownloadProgressEvent))
			Emit (DownloadProgressEvent, new DownloadProgressEventArgs (GetProgress ()));
	}

	NotifyListenersOfPropertyChange (args, error);
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
			downloader->GetHttpRequest ()->AddHandler (HttpRequest::WriteEvent, pixbuf_write, this);
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
	if (downloader)
		CleanupDownloader ();

	SetProgress (1.0);

	if (downloader && loader == NULL) {
		char *filename = downloader->GetDownloadedFilename (part_name);
		guchar b[4096];
		int offset = 0;
		ssize_t n;
		int fd;

		if ((fd = g_open (filename, O_RDONLY)) == -1) {
			moon_error = new MoonError (MoonError::EXCEPTION, 4001, "failed to open file");
			goto failed;
		}

		do {
			do {
				n = read (fd, b, sizeof (b));
			} while (n == -1 && errno == EINTR);

			if (n == -1) break;

			PixbufWrite (b, offset, n);

			offset += n;
		} while (n > 0 && !loader->GetError ());

		close (fd);

		if (moon_error)
			goto failed;
	}

	if (downloader) {
		downloader->unref ();
		downloader = NULL;
	}

	if (get_res_aborter) {
		delete get_res_aborter;
		get_res_aborter = NULL;
	}

	PixmapComplete ();

	return;
failed:
	if (downloader) {
		downloader->unref ();
		downloader = NULL;
	}

	if (get_res_aborter) {
		delete get_res_aborter;
		get_res_aborter = NULL;
	}

	if (loader)
		loader->Close ();

	ImageErrorEventArgs *args = new ImageErrorEventArgs (this, *moon_error);
	CleanupLoader ();
	if (HasHandlers(ImageFailedEvent))
		Emit (ImageFailedEvent, args);
	else
		GetDeployment ()->GetSurface ()->EmitError (args);
}


void
BitmapImage::PixmapComplete ()
{
	MoonPixbuf *pixbuf;
	
	SetProgress (1.0);

	if (!loader) {
		if (!moon_error)
			moon_error = new MoonError (MoonError::EXCEPTION, 4001, "no loader");
		goto failed;
	}

	loader->Close ();
	// If there's a CRC error or similar loading the image, don't emit a failed event.
	// drt #0 relies on this. drt 358 relies on the error being emitted if the downloader failed.
	if (loader->GetError ()) {
		CleanupLoader ();
		return;
	}

	if (moon_error)
		goto failed;

	if (!(pixbuf = loader->GetPixbuf ())) {
		moon_error = new MoonError (MoonError::EXCEPTION, 4001, "failed to create image data");
		goto failed;
	}
	
	SetPixelWidth (pixbuf->GetWidth ());
	SetPixelHeight (pixbuf->GetHeight ());
	
	// PixelFormat has been dropped and only Pbgra32 is supported
	// http://blogs.msdn.com/silverlight_sdk/archive/2009/07/01/breaking-changes-document-errata-silverlight-3.aspx
	// not clear if '3' channel is still supported (converted to 4) in SL3
	if (pixbuf->GetNumChannels () == 4) {
		SetBitmapData (premultiply_rgba (pixbuf), true);
	} else {
		SetBitmapData (expand_rgb_to_argb (pixbuf), true);
	}
	
	Invalidate ();
	
	delete loader;
	loader = NULL;
	
	if (HasHandlers (ImageOpenedEvent))
		Emit (ImageOpenedEvent, MoonUnmanagedFactory::CreateRoutedEventArgs ());
	
	return;
	
failed:
	ImageErrorEventArgs *args = new ImageErrorEventArgs (this, *moon_error);

	CleanupLoader ();
	if (HasHandlers (ImageFailedEvent))
		Emit (ImageFailedEvent, args);
	else
		GetDeployment ()->GetSurface ()->EmitError (args);
}

void
BitmapImage::DownloaderFailed ()
{
	//Uri *uri = GetUriSource ();
	//printf ("\tBitmapImage::DownloaderFailed() for %s\n", uri ? uri->ToString () : "null?");
	
	Abort ();
	ImageErrorEventArgs *args = new ImageErrorEventArgs (this, MoonError (MoonError::EXCEPTION, 4001, "downloader failed"));
	if (HasHandlers (ImageFailedEvent))
		Emit (ImageFailedEvent, args);
	else
		GetDeployment ()->GetSurface ()->EmitError (args);
}

void
BitmapImage::CleanupLoader ()
{
	SetPixelWidth (0);
	SetPixelHeight (0);

	if (loader) {
		loader->Close ();
		delete loader;
		loader = NULL;
	}
	
	if (moon_error) {
		delete moon_error;
		moon_error = NULL;
	}
}

void
BitmapImage::CreateLoader (unsigned char *buffer)
{
	if (!(moonlight_flags & RUNTIME_INIT_ALL_IMAGE_FORMATS)) {
		// 89 50 4E 47 == png magic
		if (buffer[0] == 0x89)
			loader = runtime_get_windowing_system()->CreatePixbufLoader ("png");
		// ff d8 ff e0 == jfif magic
		else if (buffer[0] == 0xff)
			loader = runtime_get_windowing_system()->CreatePixbufLoader ("jpeg");

		else {
			Abort ();
			moon_error = new MoonError (MoonError::EXCEPTION, 4001, "unsupported image type");
		}
	} else {
		loader = runtime_get_windowing_system()->CreatePixbufLoader (NULL);
	}
}

void
BitmapImage::PixbufWrite (gpointer buffer, gint32 offset, gint32 n)
{

	if (loader == NULL && offset == 0)
		CreateLoader ((unsigned char *)buffer);

	if (loader != NULL && !loader->GetError ())
		loader->Write ((const guchar *)buffer, n);
}

void
BitmapImage::pixbuf_write (EventObject *sender, EventArgs *calldata, gpointer data)
{
	BitmapImage *source = (BitmapImage *) data;
	HttpRequestWriteEventArgs *ea = (HttpRequestWriteEventArgs *) calldata;

	source->PixbufWrite ((unsigned char *) ea->GetData (), ea->GetOffset (), ea->GetCount ());
}

};
