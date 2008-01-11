/*
 * pipeline-ffmpeg.cpp: Ffmpeg related parts of the pipeline for the media
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

/*
 *	FFmpegDecoder
 */

#include <config.h>

#ifdef INCLUDE_FFMPEG

#include <glib.h>
#include <unistd.h>

G_BEGIN_DECLS
#include <stdint.h>
#include <limits.h>
#include <avformat.h>
#include <avcodec.h>
#include <swscale.h>
G_END_DECLS

#include "pipeline-ffmpeg.h"
#include "pipeline.h"
#include "asf/asf-ffmpeg.h"
#include "debug.h"

bool ffmpeg_initialized = false;

void
initialize_ffmpeg ()
{
	if (ffmpeg_initialized)
		return;
	
	avcodec_init ();
	avcodec_register_all ();
		
	ffmpeg_initialized = true;
}


FfmpegDecoder::FfmpegDecoder (Media* media, IMediaStream* stream) 
	: IMediaDecoder (media, stream),
	audio_buffer (NULL)
{
	printf ("FfmpegDecoder::FfmpegDecoder (%p, %p).\n", media, stream);
	
	if (stream->min_padding < FF_INPUT_BUFFER_PADDING_SIZE)
		stream->min_padding = FF_INPUT_BUFFER_PADDING_SIZE;
	
	initialize_ffmpeg ();
	

}

MediaResult
FfmpegDecoder::Open ()
{
	MediaResult result = MEDIA_SUCCESS;
	int ffmpeg_result = 0;
	AVCodec *codec = NULL;
	
	//printf ("FfmpegDecoder::Open ().\n");
	
	codec = avcodec_find_decoder_by_name (stream->codec);
	
	//printf ("FfmpegDecoder::Open (): Found codec: %p (id: '%s')\n", codec, stream->codec);
	
	if (codec == NULL) {
		result = MEDIA_UNKNOWN_CODEC;
		media->AddMessage (MEDIA_UNKNOWN_CODEC, stream->codec);
		goto failure;
	}
	
	context = avcodec_alloc_context ();
	
	if (context == NULL) {
		result = MEDIA_OUT_OF_MEMORY;
		media->AddMessage (MEDIA_OUT_OF_MEMORY, "Failed to allocate context.");
		goto failure;
	}
	
	if (stream->extra_data_size > 0) {
		//printf ("FfmpegDecoder::Open (): Found %i bytes of extra data.\n", stream->extra_data_size);
		context->extradata_size = stream->extra_data_size;
		context->extradata = (uint8_t*) av_mallocz (stream->extra_data_size + FF_INPUT_BUFFER_PADDING_SIZE + 100);
		if (context->extradata == NULL) {
			result = MEDIA_OUT_OF_MEMORY;
			media->AddMessage (MEDIA_OUT_OF_MEMORY, "Failed to allocate space for extra data.");
			goto failure;
		}
		memcpy (context->extradata, stream->extra_data, stream->extra_data_size);
	}

	if (stream->GetType () == MediaTypeVideo) {
		VideoStream *vs = (VideoStream*) stream;
		context->width = vs->width;
		context->height = vs->height;
	} else if (stream->GetType () == MediaTypeAudio) {
		AudioStream *as = (AudioStream*) stream;
		context->sample_rate = as->sample_rate;
		context->channels = as->channels;
		context->bit_rate = as->bit_rate;
		audio_buffer = (uint8_t*) av_mallocz (AUDIO_BUFFER_SIZE);
	} else {
		result = MEDIA_FAIL;
		media->AddMessage (MEDIA_FAIL, "Invalid stream type.");
		goto failure;
	}

	ffmpeg_result = avcodec_open (context, codec);
	if (ffmpeg_result < 0) {
		result = MEDIA_CODEC_ERROR;
		media->AddMessage (MEDIA_CODEC_ERROR, g_strdup_printf ("Failed to open codec (result: %i = %s).\n", ffmpeg_result, strerror (AVERROR (ffmpeg_result))));
		goto failure;
	}
	
	pixel_format = FfmpegConverter::ToMoonPixFmt (context->pix_fmt);
		
	//printf ("FfmpegDecoder::Open (): Opened codec successfully.\n");
	
	return result;
	
failure:
	if (context != NULL) {
		if (context->codec != NULL) {
			avcodec_close (context);
		}
		av_free (context);
		context = NULL;
	}
	return result;
}

FfmpegDecoder::~FfmpegDecoder ()
{
	if (context != NULL) {
		if (context->codec != NULL) {
			avcodec_close (context);
		}
		av_free (context);
		context = NULL;
	}
	
	av_free (audio_buffer);
	audio_buffer = NULL;
}

MediaResult
FfmpegDecoder::DecodeFrame (MediaFrame* media_frame)
{
	//printf ("FfmpegDecoder::DecodeFrame (%p).\n", media_frame);
	
	if (context == NULL)
		return MEDIA_FAIL;
	
	AVFrame* frame = NULL;
	int got_picture = 0;
	int length = 0;

	//printf ("FfmpegDecoder::DecodeFrame (%p): ", media_frame);
	//media_frame->printf ();
	//printf ("\n");
	
	if (stream->GetType () == MediaTypeVideo) {
		VideoStream* vs = (VideoStream*) stream;
		
		frame = avcodec_alloc_frame ();
	
		length = avcodec_decode_video (context, frame, &got_picture, (uint8_t*) media_frame->compressed_data, media_frame->compressed_size);
		
		if (length < 0) {
			media->AddMessage (MEDIA_CODEC_ERROR, "Error while decoding frame.");
			return MEDIA_CODEC_ERROR;
		}
		
		if (got_picture) {
			//printf ("FfmpegDecoder::DecodeFrame (%p): got picture.\n", media_frame);

			media_frame->uncompressed_size = context->width * context->height * 4;
			media_frame->uncompressed_data = (uint8_t*) g_malloc (media_frame->uncompressed_size);
			
			uint8_t *rgb_dest[3] = { (uint8_t*) media_frame->uncompressed_data, NULL, NULL};
			int rgb_stride [3] = { context->width * 4, 0, 0 };
			
			for (int i = 0; i < 4; i++) {
				media_frame->uncompressed_data_stride [i] = frame->data [i];
				media_frame->srcStride [i] = frame->linesize [i];
			}
			media_frame->srcSlideY = 0;
			media_frame->srcSlideH = context->height;
			 // We can't free the frame until the data has been used, 
			 // so save the frame in decoder_specific_data. 
			 // This will cause FfmpegDecoder::Cleanup to be called 
			 // when the MediaFrame is deleted.
			media_frame->decoder_specific_data = frame;
		} else {
			//printf ("FfmpegDecoder::DecodeFrame (%p): didn't get picture (%i), length = %i.\n", media_frame, got_picture, length);
		}
	} else if (stream->GetType () == MediaTypeAudio) {
		int frame_size_ptr = AUDIO_BUFFER_SIZE;
		length = avcodec_decode_audio2 (context, (int16_t*) audio_buffer, &frame_size_ptr, (uint8_t*) media_frame->compressed_data, media_frame->compressed_size);
		//printf ("FfmpegDecoder::DecodeFrame (), length: %i, frame_size_ptr = %i\n", length, frame_size_ptr);
		
		if (length < 0) {
			media->AddMessage (MEDIA_CODEC_ERROR, "Error while decoding frame.");
			return MEDIA_CODEC_ERROR;
		}
		
		if (frame_size_ptr > 0) {
			media_frame->uncompressed_size = frame_size_ptr;
			media_frame->uncompressed_data = (uint8_t*) g_malloc (media_frame->uncompressed_size);
			memcpy (media_frame->uncompressed_data, audio_buffer, media_frame->uncompressed_size);			
		} else {
			//printf ("FfmpegDecoder::DecodeFrame (%p): didn't get any audio back.\n", media_frame);
		}
		
		if (length != media_frame->compressed_size) {
			//printf ("FfmpegDecoder::DecodeFrame (%p): audio decoder didn't use all the input data, had %i bytes, used %i bytes.\n", media_frame, media_frame->compressed_size, length);
		}
	} else {
		media->AddMessage (MEDIA_FAIL, "Invalid media type.");
		return MEDIA_FAIL;
	}
	
	return MEDIA_SUCCESS;
}

#endif // INCLUDE_FFMPEG

