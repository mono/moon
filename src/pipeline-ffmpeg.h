/*
 * pipeline.h: Ffmpeg related parts of the pipeline for the media
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifdef INCLUDE_FFMPEG

#ifndef __MOON_PIPELINE_FFMPEG__
#define __MOON_PIPELINE_FFMPEG__

#include <glib.h>
#include <unistd.h>

G_BEGIN_DECLS
#include <stdint.h>
#include <limits.h>
#include <avformat.h>
#include <avcodec.h>
#include <swscale.h>
G_END_DECLS

#include "pipeline.h"
 
#define AUDIO_BUFFER_SIZE (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)
 
void register_ffmpeg ();

class FfmpegDemuxer : public IMediaDemuxer {
public:
	FfmpegDemuxer (Media* media) : IMediaDemuxer (media) {}
};
 
class FfmpegDecoder : public IMediaDecoder {
public:
	FfmpegDecoder (Media* media, IMediaStream* stream);
	virtual ~FfmpegDecoder ();
	
	virtual MediaResult DecodeFrame (MediaFrame* frame);
	virtual MediaResult Open ();
	virtual void Cleanup (MediaFrame* frame);
	
private:
	AVCodecContext *context;
	uint8_t* audio_buffer;
};

class FfmpegDecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char* codec);
	virtual IMediaDecoder* Create (Media* media, IMediaStream* stream);
	virtual const char* GetName () { return "FfmpegDecoder"; }
};

class FfmpegConverter : public IImageConverter {
public:
	FfmpegConverter (Media* media, VideoStream* stream);	
	~FfmpegConverter ();
	
	MediaResult Open ();
	MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride []);
	
	static PixelFormat ToFfmpegPixFmt (MoonPixelFormat format);	
	static MoonPixelFormat ToMoonPixFmt (PixelFormat format);
	
private:
	struct SwsContext *scaler;
};


class FfmpegConverterInfo : public ConverterInfo {
public:
	virtual bool Supports (MoonPixelFormat input, MoonPixelFormat output);
	virtual IImageConverter* Create (Media* media, VideoStream* stream);
	virtual const char* GetName () { return "FfmpegConverter"; }
};

#endif // __MOON_PIPELINE_FFMPEG__
#endif // INCLUDE_FFMPEG
