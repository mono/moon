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
}

MoonPixbufLoaderAndroid::MoonPixbufLoaderAndroid ()
{
}

MoonPixbufLoaderAndroid::~MoonPixbufLoaderAndroid ()
{
}

void
MoonPixbufLoaderAndroid::Write (const guchar *buffer, int buflen, MoonError **error)
{
}

void
MoonPixbufLoaderAndroid::Close (MoonError **error)
{
}

MoonPixbuf*
MoonPixbufLoaderAndroid::GetPixbuf ()
{
	// FIXME
	return NULL;
}


MoonPixbufAndroid::MoonPixbufAndroid (/* FIXME GdkPixbuf *pixbuf, */ bool crc_error)
{
}

MoonPixbufAndroid::~MoonPixbufAndroid ()
{
}

gint
MoonPixbufAndroid::GetWidth ()
{
	if (crc_error)
		return 1;
	// FIXME
	return 0;
}

gint
MoonPixbufAndroid::GetHeight ()
{
	if (crc_error)
		return 1;
	// FIXME
	return 0;
}

gint
MoonPixbufAndroid::GetRowStride ()
{
	if (crc_error)
		return 4;
	// FIXME
	return 0;
}

gint
MoonPixbufAndroid::GetNumChannels ()
{
	if (crc_error)
		return 4;
	// FIXME
	return 0;
}

guchar*
MoonPixbufAndroid::GetPixels ()
{
	if (crc_error)
		return (guchar *) g_malloc0 (4);
	// FIXME
	return (guchar *) g_malloc0 (4);
}

gboolean
MoonPixbufAndroid::IsPremultiplied ()
{
	// FIXME
	return FALSE;
}

gpointer
MoonPixbufAndroid::GetPlatformPixbuf ()
{
	// FIXME
	return NULL;
}
