/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * yuv-converter.h: YUV2RGB converters for the pipeline
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_YUV_CONVERTER__
#define __MOON_YUV_CONVERTER__

#include <glib.h>

#include "pipeline.h"

namespace Moonlight {

class YUVConverter : public IImageConverter {
public:
	YUVConverter (Media* media, VideoStream* stream);	
	virtual ~YUVConverter ();
	
	virtual bool Open ();
	virtual MediaResult Convert (guint8 *src[], int srcStride[], int srcSlideY, int srcSlideH, guint8* dest[], int dstStride []);

	static void YV12ToBGRA (guint8 *src[], int srcStride[], int width, int height, guint8* dest, int dstStride, char *rgb_uv, bool have_mmx, bool have_sse2);

private:
	char *rgb_uv;
	bool have_mmx;
	bool have_sse2;
};

class YUVConverterInfo : public ConverterInfo {
public:
	virtual bool Supports (MoonPixelFormat input, MoonPixelFormat output);
	virtual IImageConverter* Create (Media* media, VideoStream* stream);
	virtual const char* GetName () { return "YUVConverter"; }
};

};
#endif // __MOON_YUV_CONVERTER__
