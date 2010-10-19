/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include <string.h>
#include <gdk/gdk.h>
#include "pal-gtk.h"

#include "runtime.h"

#include "pixbuf-gtk.h"

using namespace Moonlight;

MoonPixbufLoaderGtk::MoonPixbufLoaderGtk (const char *imageType)
{
	gdk_loader = gdk_pixbuf_loader_new_with_type (imageType, NULL);
	crc_error = false;
}

MoonPixbufLoaderGtk::MoonPixbufLoaderGtk ()
{
	gdk_loader = gdk_pixbuf_loader_new ();
}

MoonPixbufLoaderGtk::~MoonPixbufLoaderGtk ()
{
	g_object_unref (gdk_loader);
}

void
MoonPixbufLoaderGtk::Write (const guchar *buffer, int buflen, MoonError **error)
{
	GError *gerror = NULL;

	if (crc_error)
		return;

	gdk_pixbuf_loader_write (gdk_loader, buffer, buflen, &gerror);

	if (gerror && error) {
		if (strstr (gerror->message, "CRC error")) {
			printf ("MoonPixbufLoaderGtk: not reporting CRC error in image.\n");
			crc_error = true;
		} else {
			*error = new MoonError (MoonError::EXCEPTION, 4001, gerror->message);
		}
	}
}

void
MoonPixbufLoaderGtk::Close (MoonError **error)
{
	GError *gerror = NULL;

	if (crc_error)
		return;

	gdk_pixbuf_loader_close (gdk_loader, &gerror);

	if (gerror && error) {
		if (strstr (gerror->message, "CRC error")) {
			printf ("MoonPixbufLoaderGtk: not reporting CRC error in image.\n");
			crc_error = true;
		} else {
			*error = new MoonError (MoonError::EXCEPTION, 4001, gerror->message);
		}
	}
}

MoonPixbuf*
MoonPixbufLoaderGtk::GetPixbuf ()
{
	GdkPixbuf *gdk_pixbuf = gdk_pixbuf_loader_get_pixbuf (gdk_loader);
	return new MoonPixbufGtk (gdk_pixbuf, crc_error); // we pass off the ref here, so don't unref
}


MoonPixbufGtk::MoonPixbufGtk (GdkPixbuf *pixbuf, bool crc_error)
{
	gdk_pixbuf = pixbuf;
	this->crc_error = crc_error;
}

MoonPixbufGtk::~MoonPixbufGtk ()
{
	g_object_unref (gdk_pixbuf);
}

gint
MoonPixbufGtk::GetWidth ()
{
	if (crc_error)
		return 1;
	return gdk_pixbuf_get_width (gdk_pixbuf);
}

gint
MoonPixbufGtk::GetHeight ()
{
	if (crc_error)
		return 1;
	return gdk_pixbuf_get_height (gdk_pixbuf);
}

gint
MoonPixbufGtk::GetRowStride ()
{
	if (crc_error)
		return 4;
	return gdk_pixbuf_get_rowstride (gdk_pixbuf);
}

gint
MoonPixbufGtk::GetNumChannels ()
{
	if (crc_error)
		return 4;
	return gdk_pixbuf_get_n_channels (gdk_pixbuf);
}

guchar*
MoonPixbufGtk::GetPixels ()
{
	if (crc_error)
		return (guchar *) g_malloc0 (4);
	return gdk_pixbuf_get_pixels (gdk_pixbuf);
}

gpointer
MoonPixbufGtk::GetPlatformPixbuf ()
{
	return gdk_pixbuf;
}
