/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PIXBUF_ANDROID_H
#define MOON_PIXBUF_ANDROID_H

#include "pal.h"

namespace Moonlight {

class MoonPixbufAndroid : public MoonPixbuf {
public:
	MoonPixbufAndroid (cairo_surface_t *image, bool crc_error);
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
	cairo_surface_t *image_surface;
	
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
	cairo_status_t ReadFunc (unsigned char *data, unsigned int len);
	static cairo_status_t cairo_read_func (void *closure, unsigned char *data, unsigned int len);
	
	bool crc_error;
	GByteArray *data;
	const char *image_type;
	guint offset;
	MoonPixbufAndroid *pixbuf;
};

};
#endif /* MOON_PIXBUF_ANDROID_H */
