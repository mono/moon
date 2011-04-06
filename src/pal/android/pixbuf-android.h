/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PIXBUF_ANDROID_H
#define MOON_PIXBUF_ANDROID_H

#include "pal.h"

namespace Moonlight {

class MoonPixbufAndroid : public MoonPixbuf {
public:
	MoonPixbufAndroid (/* FIXME GdkPixbuf *pixbuf,*/ bool crc_error);
	virtual ~MoonPixbufAndroid ();

	virtual gint GetWidth ();
	virtual gint GetHeight ();
	virtual gint GetRowStride ();
	virtual gint GetNumChannels ();
	virtual guchar *GetPixels ();
	virtual gboolean IsPremultiplied ();

	virtual gpointer GetPlatformPixbuf ();

private:
	bool crc_error;
};

class MoonPixbufLoaderAndroid : public MoonPixbufLoader {
public:
	MoonPixbufLoaderAndroid (const char *imageType);
	MoonPixbufLoaderAndroid ();
	virtual ~MoonPixbufLoaderAndroid ();

	virtual void Write (const guchar *buffer, int buflen, MoonError **error);
	virtual void Close (MoonError **error);
	virtual MoonPixbuf *GetPixbuf ();

private:
	bool crc_error;
};

};
#endif /* MOON_PIXBUF_ANDROID_H */
