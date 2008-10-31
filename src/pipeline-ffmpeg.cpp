/*
 * pipeline-ffmpeg.cpp: Ffmpeg related parts of the pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
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

#include <stdlib.h>
#include <glib.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>

#include "pipeline-ffmpeg.h"
#include "pipeline.h"
#include "mp3.h"
#include "debug.h"

#define LOG_FFMPEG(...)// printf(__VA_ARGS__);
#define CODECS_DEBUG(...) if (moonlight_flags & RUNTIME_INIT_CODECS_DEBUG) printf (__VA_ARGS__);

// libavcodec
typedef void             (dyn_avcodec_init)                 ();
typedef void             (dyn_avcodec_register_all)         ();
typedef AVCodecContext * (dyn_avcodec_alloc_context)        ();
typedef AVFrame *        (dyn_avcodec_alloc_frame)          ();
typedef int              (dyn_avcodec_close)                (AVCodecContext *avctx);
typedef int              (dyn_avcodec_decode_audio2)        (AVCodecContext *avctx, int16_t *samples, int *frame_size_ptr, const uint8_t *buf, int buf_size);
typedef int              (dyn_avcodec_decode_video)         (AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, const uint8_t *buf, int buf_size);
typedef AVCodec *        (dyn_avcodec_find_decoder_by_name) (const char *name);
typedef void             (dyn_avcodec_flush_buffers)        (AVCodecContext *avctx);
typedef int              (dyn_avcodec_open)                 (AVCodecContext *avctx, AVCodec *codec);
typedef unsigned         (dyn_avcodec_version)              ();

dyn_avcodec_init *                 d_avcodec_init = NULL;
dyn_avcodec_register_all *         d_avcodec_register_all = NULL;
dyn_avcodec_alloc_context *        d_avcodec_alloc_context = NULL;
dyn_avcodec_alloc_frame *          d_avcodec_alloc_frame = NULL;
dyn_avcodec_close *                d_avcodec_close = NULL;
dyn_avcodec_decode_audio2 *        d_avcodec_decode_audio2 = NULL;
dyn_avcodec_decode_video *         d_avcodec_decode_video = NULL;
dyn_avcodec_find_decoder_by_name * d_avcodec_find_decoder_by_name = NULL;
dyn_avcodec_flush_buffers *        d_avcodec_flush_buffers = NULL;
dyn_avcodec_open *                 d_avcodec_open = NULL;
dyn_avcodec_version *              d_avcodec_version = NULL;

#define avcodec_init                 d_avcodec_init
#define avcodec_register_all         d_avcodec_register_all
#define avcodec_alloc_context        d_avcodec_alloc_context
#define avcodec_alloc_frame          d_avcodec_alloc_frame
#define avcodec_close                d_avcodec_close
#define avcodec_decode_audio2        d_avcodec_decode_audio2
#define avcodec_decode_video         d_avcodec_decode_video
#define avcodec_find_decoder_by_name d_avcodec_find_decoder_by_name
#define avcodec_flush_buffers        d_avcodec_flush_buffers
#define avcodec_open                 d_avcodec_open
#define avcodec_version              d_avcodec_version

// libavutil
typedef void             (dyn_av_free)                      (void *ptr);
typedef void *           (dyn_av_mallocz)                   (unsigned int size);

dyn_av_free *                      d_av_free = NULL;
dyn_av_mallocz *                   d_av_mallocz = NULL;

#define av_free                      d_av_free
#define av_mallocz                   d_av_mallocz

bool ffmpeg_initialized = false;
int is_ffmpeg_usable = 0; // 0 - unknown, 1 - yes, 2 - false

pthread_mutex_t ffmpeg_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool
load_ffmpeg ()
{
	bool result = true;
	unsigned int libavcodec_version = 0;
	void *libavcodec = NULL;
	void *libavutil = NULL;
	char *libavcodec_so = NULL;
	char *libavutil_so = NULL;
	
	switch (is_ffmpeg_usable) {
	case 0:
		libavcodec_so = g_strdup_printf ("libavcodec.so.%i", LIBAVCODEC_VERSION_MAJOR);
		libavutil_so = g_strdup_printf ("libavutil.so.%i", LIBAVUTIL_VERSION_MAJOR);
		libavcodec = dlopen (libavcodec_so, RTLD_LAZY);
		libavutil = dlopen (libavutil_so, RTLD_LAZY);

		if (libavcodec == NULL) {
			result = false;
			fprintf (stderr, "Moonlight: Could not find '%s'.\n", libavcodec_so);
		} 
		if (libavutil == NULL) {
			result = false;
			fprintf (stderr, "Moonlight: Could not find '%s'.\n", libavutil_so);
		}

		g_free (libavcodec_so);
		g_free (libavutil_so);
		
		if (!result) {
			is_ffmpeg_usable = 2;
			if (libavutil != NULL)
				dlclose (libavutil);
			if (libavcodec != NULL)
				dlclose (libavcodec);
			return false;
		}

		result &= NULL != (d_avcodec_init                 = (dyn_avcodec_init *)                 dlsym (libavcodec, "avcodec_init"));
		result &= NULL != (d_avcodec_register_all         = (dyn_avcodec_register_all *)         dlsym (libavcodec, "avcodec_register_all"));
		result &= NULL != (d_avcodec_alloc_context        = (dyn_avcodec_alloc_context *)        dlsym (libavcodec, "avcodec_alloc_context"));
		result &= NULL != (d_avcodec_alloc_frame          = (dyn_avcodec_alloc_frame *)          dlsym (libavcodec, "avcodec_alloc_frame"));
		result &= NULL != (d_avcodec_close                = (dyn_avcodec_close *)                dlsym (libavcodec, "avcodec_close"));
		result &= NULL != (d_avcodec_decode_audio2        = (dyn_avcodec_decode_audio2 *)        dlsym (libavcodec, "avcodec_decode_audio2"));
		result &= NULL != (d_avcodec_decode_video         = (dyn_avcodec_decode_video *)         dlsym (libavcodec, "avcodec_decode_video"));
		result &= NULL != (d_avcodec_find_decoder_by_name = (dyn_avcodec_find_decoder_by_name *) dlsym (libavcodec, "avcodec_find_decoder_by_name"));
		result &= NULL != (d_avcodec_flush_buffers        = (dyn_avcodec_flush_buffers *)        dlsym (libavcodec, "avcodec_flush_buffers"));
		result &= NULL != (d_avcodec_open                 = (dyn_avcodec_open *)                 dlsym (libavcodec, "avcodec_open"));
		result &= NULL != (d_avcodec_version              = (dyn_avcodec_version *)              dlsym (libavcodec, "avcodec_version"));
		
		result &= NULL != (d_av_free    = (dyn_av_free *)    dlsym (libavutil, "av_free"));
		result &= NULL != (d_av_mallocz = (dyn_av_mallocz *) dlsym (libavutil, "av_mallocz"));
		
		if (d_avcodec_version != NULL) {
			libavcodec_version = d_avcodec_version ();
			fprintf (stdout, "Moonlight: Loaded libavcodec version: '%02i.%02i.%02i'\n", 
					(libavcodec_version >> 16) & 0xFF, (libavcodec_version >> 8) & 0xFF, libavcodec_version & 0xFF);
		}
		
		if (!result)
			fprintf (stderr, "Moonlight: Failed to load one or more required functions in libavcodec.so or libavutil.so.");

		is_ffmpeg_usable = result ? 1 : 2;
		return result;
	case 1:
		return true;
	default:
		return false;
	}
	
	return true;
}

static void
initialize_ffmpeg ()
{
	if (ffmpeg_initialized)
		return;
	
	if (!load_ffmpeg ())
		return;
		
	avcodec_init ();
	avcodec_register_all ();
		
	ffmpeg_initialized = true;
}

void
register_ffmpeg ()
{
	initialize_ffmpeg ();

	if (is_ffmpeg_usable != 1)
		return;
		
	Media::RegisterDecoder (new FfmpegDecoderInfo ());
	//Media::RegisterDemuxer (new FfmpegDemuxerInfo ());
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
	
	frame_buffer = NULL;
	frame_buffer_length = 0;
	last_pts = G_MAXUINT64;
}

PixelFormat 
FfmpegDecoder::ToFfmpegPixFmt (MoonPixelFormat format)
{
	switch (format) {
	case MoonPixelFormatYUV420P: return PIX_FMT_YUV420P;  
	case MoonPixelFormatRGB32: return PIX_FMT_RGB32;
	default:
		//printf ("FfmpegDecoder::ToFfmpegPixFmt (%i): Unknown pixel format.\n", format);
		return PIX_FMT_NONE;
	}
}

MoonPixelFormat
FfmpegDecoder::ToMoonPixFmt (PixelFormat format)
{
	switch (format) {
	case PIX_FMT_YUV420P: return MoonPixelFormatYUV420P;
	case PIX_FMT_RGB32: return MoonPixelFormatRGB32;
	default:
		//printf ("FfmpegDecoder::ToMoonPixFmt (%i): Unknown pixel format.\n", format);
		return MoonPixelFormatNone;
	};
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
		Media::Warning (MEDIA_UNKNOWN_CODEC, "Unknown codec: %s", stream->codec);
		goto failure;
	}
	
	context = avcodec_alloc_context ();
	
	if (context == NULL) {
		result = MEDIA_OUT_OF_MEMORY;
		Media::Warning (MEDIA_OUT_OF_MEMORY, "Failed to allocate context.");
		goto failure;
	}
	
	if (stream->extra_data_size > 0) {
		//printf ("FfmpegDecoder::Open (): Found %i bytes of extra data.\n", stream->extra_data_size);
		context->extradata_size = stream->extra_data_size;
		context->extradata = (guint8*) av_mallocz (stream->extra_data_size + FF_INPUT_BUFFER_PADDING_SIZE + 100);
		if (context->extradata == NULL) {
			result = MEDIA_OUT_OF_MEMORY;
			Media::Warning (MEDIA_OUT_OF_MEMORY, "Failed to allocate space for extra data.");
			goto failure;
		}
		memcpy (context->extradata, stream->extra_data, stream->extra_data_size);
	}

	if (stream->GetType () == MediaTypeVideo) {
		VideoStream *vs = (VideoStream*) stream;
		context->width = vs->width;
		context->height = vs->height;
#if LIBAVCODEC_VERSION_MAJOR < 52
		context->bits_per_sample = vs->bits_per_sample;
#endif
		context->codec_type = CODEC_TYPE_VIDEO;
	} else if (stream->GetType () == MediaTypeAudio) {
		AudioStream *as = (AudioStream*) stream;
		context->sample_rate = as->sample_rate;
		context->channels = as->channels;
		context->bit_rate = as->bit_rate;
		context->block_align = as->block_align;
		context->codec_type = CODEC_TYPE_AUDIO;
		audio_buffer = (guint8*) av_mallocz (AUDIO_BUFFER_SIZE);
	} else {
		result = MEDIA_FAIL;
		Media::Warning (MEDIA_FAIL, "Invalid stream type.");
		goto failure;
	}

	ffmpeg_result = avcodec_open (context, codec);
	if (ffmpeg_result < 0) {
		result = MEDIA_CODEC_ERROR;
		Media::Warning (MEDIA_CODEC_ERROR, "Failed to open codec (result: %d = %s).", ffmpeg_result, strerror (AVERROR (ffmpeg_result)));
		goto failure;
	}
	
	pixel_format = FfmpegDecoder::ToMoonPixFmt (context->pix_fmt);
		
	//printf ("FfmpegDecoder::Open (): Opened codec successfully.\n");
	
	pthread_mutex_unlock (&ffmpeg_mutex);
	
	return result;
	
failure:
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

	if (frame_buffer != NULL)
		g_free (frame_buffer);
	
	pthread_mutex_unlock (&ffmpeg_mutex);
}

void
FfmpegDecoder::Cleanup (MediaFrame *frame)
{
	AVFrame *av_frame = (AVFrame *) frame->decoder_specific_data;
	
	if (av_frame != NULL) {
		if (av_frame->data[0] != frame->data_stride[0]) {
			for (int i = 0; i < 4; i++)
				free (frame->data_stride[i]);
		}
		
		frame->decoder_specific_data = NULL;
		av_free (av_frame);
	}
}

void
FfmpegDecoder::CleanState ()
{
	int length;
	AVFrame *frame = NULL;
	int got_picture = 0;
	
	LOG_FFMPEG ("FfmpegDecoder::CleanState ()\n");
	
	has_delayed_frame = false;
	last_pts = G_MAXUINT64;
	
	if (context != NULL) {
		// This is what ffmpeg says you should do.
		avcodec_flush_buffers (context);
		
		// The above doesn't seem to be implemented for wmv/vc1 codecs though, so do it the hard way.
		if (stream->GetType () != MediaTypeVideo)
			return; // This is only an issue for video codecs

		frame = avcodec_alloc_frame ();
		length = avcodec_decode_video (context, frame, &got_picture, NULL, 0);
		av_free (frame);
	}		
}

MediaResult
FfmpegDecoder::DecodeFrame (MediaFrame *mf)
{
	AVFrame *frame = NULL;
	guint64 prev_pts;
	//guint64 input_pts = mf->pts;
	int got_picture = 0;
	int length = 0;
	
	LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (%p). pts: %llu ms, context: %p\n", mf, MilliSeconds_FromPts (mf->pts), context);
	
	if (context == NULL)
		return MEDIA_FAIL;
	
	if (stream->GetType () == MediaTypeVideo) {
		frame = avcodec_alloc_frame ();
		prev_pts = last_pts;
		last_pts = mf->pts;
		
		length = avcodec_decode_video (context, frame, &got_picture, mf->buffer, mf->buflen);
		
		if (length < 0 || !got_picture) {
			// This is normally because the codec is a delayed codec,
			// the first decoding request doesn't give any result,
			// then every subsequent request returns the previous frame.
			// TODO: Find a way to get the last frame out of ffmpeg
			// (requires passing NULL as buffer and 0 as buflen)
			if (has_delayed_frame) {
				Media::Warning (MEDIA_CODEC_ERROR, "Error while decoding frame (got length: %d).", length);
				return MEDIA_CODEC_ERROR;
			} else {
				//Media::Warning (MEDIA_CODEC_ERROR, "Error while decoding frame (got length: %d), delaying.", length);
				has_delayed_frame = true;
				return MEDIA_CODEC_DELAYED;
			}
		}
		
		if (prev_pts != G_MAXUINT64 && has_delayed_frame)
			mf->pts = prev_pts;

		LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (%p): got picture, input pts: %llu, actual pts: %llu, has delayed frame: %i, prev_pts: %llu ms\n", 
			mf, MilliSeconds_FromPts (input_pts), MilliSeconds_FromPts (mf->pts), has_delayed_frame, MilliSeconds_FromPts (prev_pts));

		mf->AddState (FRAME_PLANAR);
		
		g_free (mf->buffer);
		mf->buffer = NULL;
		mf->buflen = 0;
		
		mf->srcSlideY = 0;
		mf->srcSlideH = context->height;
		
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
				if (posix_memalign ((void **)&mf->data_stride [i], 16, plane_bytes[i] + stream->min_padding)) {
					g_warning ("Could not allocate memory for data stride");
					return MEDIA_OUT_OF_MEMORY;
				}
				memcpy (mf->data_stride[i], frame->data[i], plane_bytes[i]);
			} else {
				mf->data_stride[i] = frame->data[i];
			}
			
			mf->srcStride[i] = frame->linesize[i];
		}
		
		// We can't free the frame until the data has been used, 
		// so save the frame in decoder_specific_data. 
		// This will cause FfmpegDecoder::Cleanup to be called 
		// when the MediaFrame is deleted.
		// TODO: check if we can free this now, given that we always copy data out from ffmpeg's buffers
		mf->decoder_specific_data = frame;
	} else if (stream->GetType () == MediaTypeAudio) {
		MpegFrameHeader mpeg;
		int remain = mf->buflen;
		int offset = 0;
		int decoded_size = 0;
		guint8 *decoded_frames = NULL;

		if (frame_buffer != NULL) {
			mf->buffer = (guint8 *) g_realloc (mf->buffer, mf->buflen+frame_buffer_length);
			memmove (mf->buffer + frame_buffer_length, mf->buffer, mf->buflen);
			memcpy (mf->buffer, frame_buffer, frame_buffer_length);
			remain += frame_buffer_length;
			
			g_free (frame_buffer);
			frame_buffer = NULL;
		}

		do {
			int frame_size;
			int buffer_size = AUDIO_BUFFER_SIZE;

			if (stream->codec_id == CODEC_MP3 && mpeg_parse_header (&mpeg, mf->buffer+offset)) {
				frame_size = mpeg_frame_length (&mpeg, false);
	
				if (frame_size > remain) {
					frame_buffer_length = remain;
					frame_buffer = (guint8 *) malloc (remain);
					memcpy (frame_buffer, mf->buffer+offset, remain);
					remain = 0;
					continue;
				}
			} else {
				frame_size = mf->buflen;
			}

			length = avcodec_decode_audio2 (context, (gint16 *) audio_buffer, &buffer_size, mf->buffer+offset, frame_size);

			if (length < 0 || buffer_size < frame_size) {
				//Media::Warning (MEDIA_CODEC_ERROR, "Error while decoding audio frame (length: %d, frame_size. %d, buflen: %u).", length, frame_size, mf->buflen);
				return MEDIA_CODEC_ERROR;
			}

			if (buffer_size > 0) {
				decoded_frames = (guint8 *) g_realloc (decoded_frames, buffer_size+decoded_size);
				memcpy (decoded_frames+decoded_size, audio_buffer, buffer_size);
				offset += frame_size;
				decoded_size += buffer_size;
				remain -= frame_size;
			} else {
				if (decoded_frames != NULL)
					g_free (decoded_frames);
				decoded_frames = NULL;
				remain = 0;
				decoded_size = 0;
			}	
		} while (remain > 0);
		
		g_free (mf->buffer);

		mf->buffer = decoded_frames;
		mf->buflen = decoded_size;;
	} else {
		Media::Warning (MEDIA_FAIL, "Invalid media type.");
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
