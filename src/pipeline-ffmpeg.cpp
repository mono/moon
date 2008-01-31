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
#include <pthread.h>

G_BEGIN_DECLS
#include <stdint.h>
#include <limits.h>
#include <avformat.h>
#include <avcodec.h>
#include <swscale.h>
G_END_DECLS

#include "pipeline-ffmpeg.h"
#include "pipeline.h"
#include "debug.h"

bool ffmpeg_initialized = false;
bool ffmpeg_registered = false;

pthread_mutex_t ffmpeg_mutex = PTHREAD_MUTEX_INITIALIZER;

void
initialize_ffmpeg ()
{
	if (ffmpeg_initialized)
		return;
	
	avcodec_init ();
	avcodec_register_all ();
		
	ffmpeg_initialized = true;
}

void
register_ffmpeg ()
{
	initialize_ffmpeg ();
	
	if (ffmpeg_registered)
		return;
	
	Media::RegisterConverter (new FfmpegConverterInfo ());
	Media::RegisterDecoder (new FfmpegDecoderInfo ());
	//Media::RegisterDemuxer (new FfmpegDemuxerInfo ());
	
	ffmpeg_registered = true;
}

/*
 * FfmpegDecoder
 */

FfmpegDecoder::FfmpegDecoder (Media* media, IMediaStream* stream) 
	: IMediaDecoder (media, stream),
	audio_buffer (NULL), has_delayed_frame (false)
{
	//printf ("FfmpegDecoder::FfmpegDecoder (%p, %p).\n", media, stream);
	
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
	
	pthread_mutex_lock (&ffmpeg_mutex);
	
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
	
	pthread_mutex_unlock (&ffmpeg_mutex);
	
	return result;
	
failure:
	if (context != NULL) {
		if (context->codec != NULL) {
			avcodec_close (context);
		}
		av_free (context);
		context = NULL;
	}
	pthread_mutex_unlock (&ffmpeg_mutex);
	
	return result;
}

FfmpegDecoder::~FfmpegDecoder ()
{
	pthread_mutex_lock (&ffmpeg_mutex);
	
	if (context != NULL) {
		if (context->codec != NULL) {
			avcodec_close (context);
		}
		if (context->extradata != NULL) {
			av_free (context->extradata);
			context->extradata = NULL;
		}
		av_free (context);
		context = NULL;
	}
	
	av_free (audio_buffer);
	audio_buffer = NULL;
	
	pthread_mutex_unlock (&ffmpeg_mutex);
}

void
FfmpegDecoder::Cleanup (MediaFrame *frame)
{
	AVFrame *av_frame = (AVFrame *) frame->decoder_specific_data;
	
	if (av_frame != NULL) {
		if (av_frame->data[0] != frame->data_stride[0]) {
			for (int i = 0; i < 4; i++)
				g_free (frame->data_stride[i]);
		}
		
		frame->decoder_specific_data = NULL;
		av_free (av_frame);
	}
}

MediaResult
FfmpegDecoder::DecodeFrame (MediaFrame *mf)
{
	AVFrame *frame = NULL;
	int got_picture = 0;
	int length = 0;
	
	//printf ("FfmpegDecoder::DecodeFrame (%p).\n", mf);
	
	if (context == NULL)
		return MEDIA_FAIL;
	
	if (stream->GetType () == MediaTypeVideo) {
		frame = avcodec_alloc_frame ();
		
		if (mf->IsKeyFrame ())
			printf ("FfmpegDecoder::DecodeFrame (): decoded key frame\n");
		
		length = avcodec_decode_video (context, frame, &got_picture, mf->buffer, mf->buflen);
		
		if (length < 0 || !got_picture) {
			// This is normally because the codec is a delayed codec,
			// the first decoding request doesn't give any result,
			// then every subsequent request returns the previous frame.
			// TODO: Find a way to get the last frame out of ffmpeg
			// (requires passing NULL as buffer and 0 as buflen)
			if (has_delayed_frame) {
				media->AddMessage (MEDIA_CODEC_ERROR, g_strdup_printf ("Error while decoding frame (got length: %i).", length));
				return MEDIA_CODEC_ERROR;
			} else {
				//media->AddMessage (MEDIA_CODEC_ERROR, g_strdup_printf ("Error while decoding frame (got length: %i), delaying.", length));
				has_delayed_frame = true;
				return MEDIA_CODEC_DELAYED;
			}
		}
		
		//printf ("FfmpegDecoder::DecodeFrame (%p): got picture.\n", mf);
		
		mf->AddState (FRAME_PLANAR);
		
		g_free (mf->buffer);
		mf->buffer = NULL;
		mf->buflen = 0;
		
		mf->srcSlideY = 0;
		mf->srcSlideH = context->height;
		
		if (mf->IsCopyDecodedData ()) {
			int height = context->height;
			int plane_bytes [4];
			
			switch (pixel_format) {
			case MoonPixelFormatYUV420P:
				plane_bytes [0] = height * frame->linesize [0];
				plane_bytes [1] = height * frame->linesize [1] / 2;
				plane_bytes [2] = height * frame->linesize [2] / 2;
				plane_bytes [3] = 0;
				break;
			default:
				printf ("FfmpegDecoder::DecodeFrame (): Unknown output format, can't calculate byte number.\n");
				plane_bytes [0] = 0;
				plane_bytes [1] = 0;
				plane_bytes [2] = 0;
				plane_bytes [3] = 0;
				break;
			}
			
			for (int i = 0; i < 4; i++) {
				if (plane_bytes [i] != 0) {
					mf->data_stride [i] = (uint8_t *) g_malloc (plane_bytes[i] + stream->min_padding);
					memcpy (mf->data_stride[i], frame->data[i], plane_bytes[i]);
				} else {
					mf->data_stride[i] = frame->data[i];
				}
				
				mf->srcStride[i] = frame->linesize[i];
			}
		} else {
			for (int i = 0; i < 4; i++) {
				mf->data_stride[i] = frame->data[i];
				mf->srcStride[i] = frame->linesize[i];
			}
		}
		
		// We can't free the frame until the data has been used, 
		// so save the frame in decoder_specific_data. 
		// This will cause FfmpegDecoder::Cleanup to be called 
		// when the MediaFrame is deleted.
		mf->decoder_specific_data = frame;
	} else if (stream->GetType () == MediaTypeAudio) {
		int frame_size = AUDIO_BUFFER_SIZE;
		
		length = avcodec_decode_audio2 (context, (int16_t *) audio_buffer, &frame_size, mf->buffer, mf->buflen);
		
		if (length < 0 || (uint32_t) frame_size < mf->buflen) {
			media->AddMessage (MEDIA_CODEC_ERROR, g_strdup_printf ("Error while decoding audio frame (length: %i, frame_size. %i, buflen: %u).", length, frame_size, mf->buflen));
			return MEDIA_CODEC_ERROR;
		}
		
		g_free (mf->buffer);
		
		if (frame_size > 0) {
			mf->buffer = (uint8_t *) g_malloc (frame_size);
			memcpy (mf->buffer, audio_buffer, frame_size);
			mf->buflen = frame_size;
		} else {
			mf->buffer = NULL;
			mf->buflen = 0;
		}
	} else {
		media->AddMessage (MEDIA_FAIL, "Invalid media type.");
		return MEDIA_FAIL;
	}
	
	mf->AddState (FRAME_DECODED);
	
	return MEDIA_SUCCESS;
}

/*
 * FfmpegDecoderInfo
 */

bool
FfmpegDecoderInfo::Supports (const char* codec)
{
	return avcodec_find_decoder_by_name (codec) != NULL;
}

IMediaDecoder*
FfmpegDecoderInfo::Create (Media* media, IMediaStream* stream)
{
	return new FfmpegDecoder (media, stream);
}


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

PixelFormat 
FfmpegConverter::ToFfmpegPixFmt (MoonPixelFormat format)
{
	switch (format) {
	case MoonPixelFormatYUV420P: return PIX_FMT_YUV420P;  
	case MoonPixelFormatRGB32: return PIX_FMT_RGB32;
	default:
		//printf ("FfmpegConverter::ToFfmpegPixFmt (%i): Unknown pixel format.\n", format);
		return PIX_FMT_NONE;
	}
}

MoonPixelFormat
FfmpegConverter::ToMoonPixFmt (PixelFormat format)
{
	switch (format) {
	case PIX_FMT_YUV420P: return MoonPixelFormatYUV420P;
	case PIX_FMT_RGB32: return MoonPixelFormatRGB32;
	default:
		//printf ("FfmpegConverter::ToMoonPixFmt (%i): Unknown pixel format.\n", format);
		return MoonPixelFormatNone;
	};
}

MediaResult
FfmpegConverter::Open ()
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
	return FfmpegConverter::ToFfmpegPixFmt (input)  != PIX_FMT_NONE 
		&& FfmpegConverter::ToFfmpegPixFmt (output) != PIX_FMT_NONE;
}

IImageConverter*
FfmpegConverterInfo::Create (Media* media, VideoStream* stream)
{
	return new FfmpegConverter (media, stream);
}

#endif // INCLUDE_FFMPEG

