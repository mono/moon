/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include <string.h>
#include "pal-android.h"

#include "runtime.h"

#include "pixbuf-android.h"

using namespace Moonlight;

MoonPixbufLoaderAndroid::MoonPixbufLoaderAndroid (const char *imageType)
{
	crc_error = false;
	data = g_byte_array_new ();
	image_type = imageType;
	offset = 0;
	pixbuf = NULL;
}

MoonPixbufLoaderAndroid::MoonPixbufLoaderAndroid ()
{
	crc_error = false;
	data = g_byte_array_new ();
	// FIXME 
	image_type = "png";
	offset = 0;
	pixbuf = NULL;
}

MoonPixbufLoaderAndroid::~MoonPixbufLoaderAndroid ()
{
	g_byte_array_free (data, true);
	data = NULL;
}

void
MoonPixbufLoaderAndroid::Write (const guchar *buffer, int buflen, MoonError **error)
{
	g_byte_array_append (data, buffer, buflen);
}

void
MoonPixbufLoaderAndroid::Close (MoonError **error)
{
	//g_warning ("MPLA::Close ()");
}

cairo_status_t
MoonPixbufLoaderAndroid::cairo_read_func (void *closure, unsigned char *data, unsigned int len)
{
	//g_warning ("MPLA::cairo_read_func");
	MoonPixbufLoaderAndroid *loader = (MoonPixbufLoaderAndroid *)closure;
	return loader->ReadFunc (data, len);
}

cairo_status_t
MoonPixbufLoaderAndroid::ReadFunc (unsigned char *buffer, unsigned int len)
{
	//g_warning ("MPLA::ReadFunc len=%d", len);
	if (data->len < (offset + len))
		return CAIRO_STATUS_NO_MEMORY;
		
	memmove (buffer, data->data + offset, len);
	offset += len;

	return CAIRO_STATUS_SUCCESS;
}

MoonPixbuf*
MoonPixbufLoaderAndroid::GetPixbuf ()
{
	//g_warning ("MPLA::GetPixbuf ()");
	if (pixbuf != NULL)
		return pixbuf;
	
	cairo_surface_t *image_surface = NULL;
	
	if (image_type == NULL || strcmp (image_type, "png") == 0) 
		image_surface = cairo_image_surface_create_from_png_stream (cairo_read_func, this);
	else
		g_warning ("jpeg is currently unsupported");
			 
	g_warning ("MPLA::GetPixbuf-> after create");
	pixbuf = new MoonPixbufAndroid (image_surface, false);
	return pixbuf;	
}

MoonPixbufAndroid::MoonPixbufAndroid (cairo_surface_t *image_surface, bool crc_error)
{
	this->image_surface = image_surface;
	this->crc_error = crc_error;
}

MoonPixbufAndroid::~MoonPixbufAndroid ()
{
	cairo_surface_destroy (image_surface);
	image_surface = NULL;
}

gint
MoonPixbufAndroid::GetWidth ()
{
	if (crc_error)
		return 1;
	return cairo_image_surface_get_width (image_surface);
}

gint
MoonPixbufAndroid::GetHeight ()
{
	if (crc_error)
		return 1;
	return cairo_image_surface_get_height (image_surface);
}

gint
MoonPixbufAndroid::GetRowStride ()
{
	if (crc_error)
		return 4;
	return cairo_image_surface_get_stride (image_surface);
}

gint
MoonPixbufAndroid::GetNumChannels ()
{
	if (crc_error)
		return 4;

	return 4;
}

guchar*
MoonPixbufAndroid::GetPixels ()
{
	if (crc_error)
		return (guchar *) g_malloc0 (4);
	
	return cairo_image_surface_get_data (image_surface);
}

gboolean
MoonPixbufAndroid::IsPremultiplied ()
{
	return TRUE;
}

gpointer
MoonPixbufAndroid::GetPlatformPixbuf ()
{
	// FIXME
	return NULL;
}
