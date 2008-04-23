/*
 * swscale-converter.cpp: Swscale YUV2RGB converter
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>
#include <unistd.h>
#include <pthread.h>

G_BEGIN_DECLS
#include <stdint.h>
#include <limits.h>
#include <swscale.h>
G_END_DECLS

#include "swscale-converter.h"
#include "pipeline.h"
#include "debug.h"

/*
 * FfmpegConverter
 */

FfmpegConverter::FfmpegConverter (Media* media, VideoStream* stream) : IImageConverter (media, stream)
{
	scaler = NULL;
}

FfmpegConverter::~FfmpegConverter ()
{
	if (scaler != NULL) {
		sws_freeContext (scaler);
		scaler = NULL;
	}
}

MediaResult
FfmpegConverter::Open ()
{
	PixelFormat in_format = FfmpegDecoder::ToFfmpegPixFmt (input_format);
	PixelFormat out_format = FfmpegDecoder::ToFfmpegPixFmt (output_format);
	
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

MediaResult
FfmpegConverter::Convert (uint8_t *src[], int srcStride[], int srcSlideY, int srcSlideH, uint8_t* dest[], int dstStride [])
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


/*
 * FfmpegConverterInfo
 */

bool
FfmpegConverterInfo::Supports (MoonPixelFormat input, MoonPixelFormat output)
{
	return FfmpegDecoder::ToFfmpegPixFmt (input)  != PIX_FMT_NONE 
		&& FfmpegDecoder::ToFfmpegPixFmt (output) != PIX_FMT_NONE;
}

IImageConverter*
FfmpegConverterInfo::Create (Media* media, VideoStream* stream)
{
	return new FfmpegConverter (media, stream);
}
