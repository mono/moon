/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PIXBUF_GTK_H
#define MOON_PIXBUF_GTK_H

#include "pal.h"

namespace Moonlight {

class MoonPixbufGtk : public MoonPixbuf {
public:
	MoonPixbufGtk (GdkPixbuf *pixbuf);
	virtual ~MoonPixbufGtk ();

	virtual gint GetWidth ();
	virtual gint GetHeight ();
	virtual gint GetRowStride ();
	virtual gint GetNumChannels ();
	virtual guchar *GetPixels ();

private:
	GdkPixbuf *gdk_pixbuf;
};

class MoonPixbufLoaderGtk : public MoonPixbufLoader {
public:
	MoonPixbufLoaderGtk (const char *imageType);
	MoonPixbufLoaderGtk ();
	virtual ~MoonPixbufLoaderGtk ();

	virtual void Write (const guchar *buffer, int buflen, MoonError **error);
	virtual void Close (MoonError **error);
	virtual MoonPixbuf *GetPixbuf ();

private:
	GdkPixbufLoader *gdk_loader;
};

};
#endif /* MOON_PIXBUF_GTK_H */
