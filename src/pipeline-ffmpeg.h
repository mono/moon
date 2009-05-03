/*
 * pipeline.h: Ffmpeg related parts of the pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
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

G_BEGIN_DECLS
#include <limits.h>
#if HAVE_LIBAVCODEC_AVCODEC_H
#include <libavcodec/avcodec.h>
#else
#include <avcodec.h>
#endif
G_END_DECLS

#include "pipeline.h"
 
#define AUDIO_BUFFER_SIZE (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)
 
void register_ffmpeg ();

class FfmpegDemuxer : public IMediaDemuxer {
public:
	FfmpegDemuxer (Media *media, IMediaSource *source);
};
 
class FfmpegDecoder : public IMediaDecoder {
private:
	AVCodecContext *context;
	guint8* audio_buffer;
	guint8* frame_buffer;
	guint32 frame_buffer_length;
	guint64 last_pts;
	bool has_delayed_frame;
	
	MediaResult Open ();
	
protected:
	virtual ~FfmpegDecoder () {}
	virtual void DecodeFrameAsyncInternal (MediaFrame* frame);
	virtual void OpenDecoderAsyncInternal ();
	
public:
	FfmpegDecoder (Media* media, IMediaStream* stream);
	virtual void Dispose ();	
	virtual void Cleanup (MediaFrame* frame);
	virtual void CleanState ();
	virtual bool HasDelayedFrame () {return has_delayed_frame; }

	static PixelFormat ToFfmpegPixFmt (MoonPixelFormat format);	
	static MoonPixelFormat ToMoonPixFmt (PixelFormat format);
};

class FfmpegDecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char* codec);
	virtual IMediaDecoder* Create (Media* media, IMediaStream* stream);
	virtual const char* GetName () { return "FfmpegDecoder"; }
};

#endif // __MOON_PIPELINE_FFMPEG__
#endif // INCLUDE_FFMPEG
