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
	
	virtual void Cleanup (MediaFrame* frame)
	{
		AVFrame* av_frame = (AVFrame*) frame->decoder_specific_data;
		if (av_frame != NULL) {
			av_free (av_frame);
			frame->decoder_specific_data = NULL;
		}
	}
	
private:
	AVCodecContext *context;
	uint8_t* audio_buffer;
};

class FfmpegConverter : public IImageConverter {
public:
	FfmpegConverter (Media* media, VideoStream* stream) : IImageConverter (media, stream)
	{
		scaler = NULL;
	}
	
	~FfmpegConverter ()
	{
		if (scaler != NULL) {
			sws_freeContext (scaler);
			scaler = NULL;
		}
	}
	
	static PixelFormat ToFfmpegPixFmt (PIXEL_FORMAT format)
	{
		switch (format) {
		case PIXEL_FORMAT_YUV420P: return PIX_FMT_YUV420P;  
		case PIXEL_FORMAT_RGB32: return PIX_FMT_RGB32;
		default:
			printf ("FfmpegConverter::ToFfmpegPixFmt (%i): Unknown pixel format.\n", format);
			return PIX_FMT_NONE;
		}
	}
	
	static PIXEL_FORMAT ToMoonPixFmt (PixelFormat format)
	{
		switch (format) {
		case PIX_FMT_YUV420P: return PIXEL_FORMAT_YUV420P;
		case PIX_FMT_RGB32: return PIXEL_FORMAT_RGB32;
		default:
			printf ("FfmpegConverter::ToMoonPixFmt (%i): Unknown pixel format.\n", format);
			return PIXEL_FORMAT_NONE;
		};
	}
	
	MediaResult Open ()
	{
		PixelFormat in_format = ToFfmpegPixFmt (input_format);
		PixelFormat out_format = ToFfmpegPixFmt (output_format);
		
		if (in_format == PIX_FMT_NONE) {
			media->AddMessage (MEDIA_CONVERTER_ERROR, "Invalid input format.");
			return MEDIA_CONVERTER_ERROR;
		}
		
		if (out_format == PIX_FMT_NONE) {
			media->AddMessage (MEDIA_CONVERTER_ERROR, "Invalid output format.");
			return MEDIA_CONVERTER_ERROR;
		}
		
		scaler = sws_getContext (stream->width, stream->height, in_format,
				stream->width, stream->height, out_format,
				SWS_BICUBIC, NULL, NULL, NULL);
				
		return MEDIA_SUCCESS;
	}
	
	MediaResult Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride [])
	{
		if (scaler == NULL) {
			media->AddMessage (MEDIA_CONVERTER_ERROR, "Converter closed.");
			return MEDIA_CONVERTER_ERROR;
		}
		
		//printf ("converting...\n");
	
		// There seems to be no documentation about what
		// sws_scale's return value means.
		sws_scale (scaler, src, srcStride, srcSlideY, srcSlideH, dest, dstStride);
		
		return MEDIA_SUCCESS;
	}
	
private:
	struct SwsContext *scaler;
};


