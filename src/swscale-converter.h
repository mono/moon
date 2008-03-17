/*
 * swscale-converter.h: Swscale related parts of the pipeline
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifdef INCLUDE_SWSCALE

#ifndef __MOON_PIPELINE_SWSCALE__
#define __MOON_PIPELINE_SWSCALE__

#include <glib.h>
#include <unistd.h>

G_BEGIN_DECLS
#include <stdint.h>
#include <swscale.h>
G_END_DECLS

#include "pipeline-ffmpeg.h"
#include "pipeline.h"

class FfmpegConverter : public IImageConverter {
public:
	FfmpegConverter (Media* media, VideoStream* stream);	
	~FfmpegConverter ();
	
	MediaResult Open ();
	MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride []);

private:
	struct SwsContext *scaler;
};


class FfmpegConverterInfo : public ConverterInfo {
public:
	virtual bool Supports (MoonPixelFormat input, MoonPixelFormat output);
	virtual IImageConverter* Create (Media* media, VideoStream* stream);
	virtual const char* GetName () { return "FfmpegConverter"; }
};

#endif // __MOON_PIPELINE_SWSCALE__
#endif // INCLUDE_SWSCALE
