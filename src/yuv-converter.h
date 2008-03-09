/*
 * yuv-converter.h: YUV2RGB converters for the pipeline
 *
 * Author:
 *   Geoff Norton (RKvinge@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_YUV_CONVERTER__
#define __MOON_YUV_CONVERTER__

#include <glib.h>
#include <unistd.h>

G_BEGIN_DECLS
#include <stdint.h>
#include <limits.h>
G_END_DECLS

#include "pipeline.h"

class YUVConverter : public IImageConverter {
public:
	YUVConverter (Media* media, VideoStream* stream);	
	~YUVConverter ();
	
	MediaResult Open ();
	MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride []);
private:
	bool have_mmx;
	bool have_sse2;
};

class YUVConverterInfo : public ConverterInfo {
public:
	virtual bool Supports (MoonPixelFormat input, MoonPixelFormat output);
	virtual IImageConverter* Create (Media* media, VideoStream* stream);
	virtual const char* GetName () { return "YUVConverter"; }
};
 
#endif // __MOON_YUV_CONVERTER__
