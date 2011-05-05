/*
 * pipeline-ffmpeg.cpp: Ffmpeg related parts of the pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
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
#include <stdlib.h>
#include <pthread.h>

#include "pipeline-ffmpeg.h"
#include "pipeline.h"
#include "mp3.h"
#include "clock.h"
#include "debug.h"
#include "deployment.h"

#include <mono/io-layer/atomic.h>

namespace Moonlight {

#define AUDIO_BUFFER_SIZE (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)

MoonMutex ffmpeg_mutex;

void
register_ffmpeg ()
{
	static bool ffmpeg_registered = false;

	/* No locking required here, this is executed serially at startup */
	if (!ffmpeg_registered) {
		/* we need this flag, since the pipeline can be initialized/cleaned up several times during the life-time of a process */
		ffmpeg_registered = true;
		avcodec_init ();
		avcodec_register_all ();
		av_register_all ();
	}

	Media::RegisterDecoder (new FfmpegDecoderInfo ());
	Media::RegisterDemuxer (new FfmpegDemuxerInfo ());
}

/*
 * FfmpegDecoder
 */

FfmpegDecoder::FfmpegDecoder (Media* media, IMediaStream* stream) 
	: IMediaDecoder (Type::FFMPEGDECODER, media, stream)
{
	stream->SetMinPadding (FF_INPUT_BUFFER_PADDING_SIZE);

	audio_buffer = NULL;
	has_delayed_frame = false;
	frame_buffer = NULL;
	frame_buffer_length = 0;
	last_pts = G_MAXUINT64;
}

void
FfmpegDecoder::InputEnded ()
{
	GetStream ()->SetOutputEnded (true);
}

PixelFormat 
FfmpegDecoder::ToFfmpegPixFmt (MoonPixelFormat format)
{
	switch (format) {
	case MoonPixelFormatYUV420P: return PIX_FMT_YUV420P;  
	case MoonPixelFormatRGB32: return PIX_FMT_RGB32;
	default:
		LOG_FFMPEG ("FfmpegDecoder::ToFfmpegPixFmt (%i): Unknown pixel format.\n", format);
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
		LOG_FFMPEG ("FfmpegDecoder::ToMoonPixFmt (%i): Unknown pixel format.\n", format);
		return MoonPixelFormatNone;
	};
}

void
FfmpegDecoder::OpenDecoderAsyncInternal ()
{
	IMediaStream *stream = GetStream ();
	int ffmpeg_result = 0;
	AVCodec *codec = NULL;
	
	ffmpeg_mutex.Lock();
	codec = avcodec_find_decoder_by_name (stream->GetCodec ());
	ffmpeg_mutex.Unlock();
	
	LOG_FFMPEG ("FfmpegDecoder::Open (): Found codec: '%s'\n", stream->GetCodec ());
	
	if (codec == NULL) {
		char *str = g_strdup_printf ("Unknown codec in FfmpegDecoder (%s)", stream->GetCodec ());
		ReportErrorOccurred (str);
		g_free (str);
		return;
	}
	
	context = avcodec_alloc_context ();
	
	if (context == NULL) {
		ReportErrorOccurred ("Failed to allocate context in FfmpegDecoder");
		return;
	}
	
	if (stream->GetExtraDataSize () > 0) {
		context->extradata_size = stream->GetExtraDataSize ();
		context->extradata = (guint8*) av_mallocz (stream->GetExtraDataSize () + FF_INPUT_BUFFER_PADDING_SIZE + 100);
		if (context->extradata == NULL) {
			ReportErrorOccurred ("Failed to allocate space for extra data in FfmpegDecoder");
			return;
		}
		memcpy (context->extradata, stream->GetExtraData (), stream->GetExtraDataSize ());
	}

	if (stream->IsVideo ()) {
		VideoStream *vs = (VideoStream*) stream;
		context->width = vs->GetWidth ();
		context->height = vs->GetHeight ();
		context->codec_type = CODEC_TYPE_VIDEO;
		if (!strcmp (vs->GetCodec (), "h264")) {
			/* h264 apparently supports a 444/422 color space too, but it won't work with SL (which will only decode 420)
			 * ffmpeg doesn't seem to support 444/422 either, so it's not testable really
			 * As a matter of fact I only found 1 encoder (the JM reference encoder: http://iphome.hhi.de/suehring/tml/download/)
			 * that *seems* to support the 444/422 color spaces, though I couldn't find any decoders to play those files,
			 * so I'm not entirely sure what I made it encode to. */
			context->pix_fmt = PIX_FMT_YUV420P;
		}
	} else if (stream->IsAudio ()) {
		AudioStream *as = (AudioStream*) stream;
		context->sample_rate = as->GetSampleRate ();
		context->channels = as->GetChannels ();
		context->bit_rate = as->GetBitRate ();
		context->block_align = as->GetBlockAlign ();
		context->codec_type = CODEC_TYPE_AUDIO;
		audio_buffer = (guint8*) av_mallocz (AUDIO_BUFFER_SIZE);
	} else {
		ReportErrorOccurred ("Invalid stream type in FfmpegDecoder");
		return;
	}

	ffmpeg_mutex.Lock();
	ffmpeg_result = avcodec_open (context, codec);
	ffmpeg_mutex.Unlock();

	if (ffmpeg_result < 0) {
		char *str = g_strdup_printf ("FfmpegDecoder failed to open codec (result: %d = %s)", ffmpeg_result, strerror (AVERROR (ffmpeg_result)));
		ReportErrorOccurred (str);
		g_free (str);
		return;
	}
	
	SetPixelFormat (FfmpegDecoder::ToMoonPixFmt (context->pix_fmt));
	
	ReportOpenDecoderCompleted ();
}

void
FfmpegDecoder::Dispose ()
{
	if (context != NULL) {
		if (context->codec != NULL) {
			ffmpeg_mutex.Lock();
			avcodec_close (context);
			ffmpeg_mutex.Unlock();
		}
		av_freep (&context->extradata);
		av_freep (&context);
	}

	av_freep (&audio_buffer);

	g_free (frame_buffer);
	frame_buffer = NULL;
	
	IMediaDecoder::Dispose ();
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
	AVPacket packet;
	int got_picture = 0;
	IMediaStream *stream = GetStream ();
	
	LOG_FFMPEG ("FfmpegDecoder::CleanState ()\n");
	
	has_delayed_frame = false;
	last_pts = G_MAXUINT64;
	
	if (context != NULL) {
		// This is what ffmpeg says you should do.
		avcodec_flush_buffers (context);
		
		// The above doesn't seem to be implemented for wmv/vc1 codecs though, so do it the hard way.
		if (!stream->IsVideo ())
			return; // This is only an issue for video codecs

		frame = avcodec_alloc_frame ();
		av_init_packet (&packet);
		packet.data = NULL;
		packet.size = 0;
		length = avcodec_decode_video2 (context, frame, &got_picture, &packet);
		av_free (frame);
	}		
}

void
FfmpegDecoder::DecodeFrameAsyncInternal (MediaFrame *mf)
{
	IMediaStream *stream = GetStream ();
	AVFrame *frame = NULL;
	AVPacket packet;
	guint64 prev_pts;
	//guint64 input_pts = mf->pts;
	int got_picture = 0;
	int length = 0;
	
	LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (%p). pts: %" G_GUINT64_FORMAT " ms, context: %p\n", mf, MilliSeconds_FromPts (mf->pts), context);
	
	if (context == NULL) {
		ReportErrorOccurred ("FfmpegDecoder: no context");
		return;
	}
	
	if (stream->IsVideo ()) {
		frame = avcodec_alloc_frame ();
		prev_pts = last_pts;
		last_pts = mf->pts;
		
		av_init_packet (&packet);
		packet.data = mf->GetBuffer ();
		packet.size = mf->GetBufLen ();
		length = avcodec_decode_video2 (context, frame, &got_picture, &packet);
		
		if (length < 0 || !got_picture) {
			av_free (frame);
			// This is normally because the codec is a delayed codec,
			// the first decoding request doesn't give any result,
			// then every subsequent request returns the previous frame.
			// TODO: Find a way to get the last frame out of ffmpeg
			// (requires passing NULL as buffer and 0 as buflen)
			if (has_delayed_frame) {
				char *str = g_strdup_printf ("FfmpegDecoder: error while decoding frame (got length: %d)", length);
				ReportErrorOccurred (str);
				g_free (str);
				return;
			} else {
				has_delayed_frame = true;
				// return MEDIA_CODEC_DELAYED;
				return;
			}
		}
		
		if (prev_pts != G_MAXUINT64 && has_delayed_frame)
			mf->pts = prev_pts;

		LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (%p): got picture, actual pts: %" G_GUINT64_FORMAT ", has delayed frame: %i, prev_pts: %" G_GUINT64_FORMAT " ms\n", 
			mf, MilliSeconds_FromPts (mf->pts), has_delayed_frame, MilliSeconds_FromPts (prev_pts));

		mf->AddState (MediaFramePlanar);
		mf->FreeBuffer ();
		mf->SetBufLen (0);
		
		mf->srcSlideY = 0;
		mf->srcSlideH = context->height;
		
		mf->width = context->width;
		mf->height = context->height;

		int height = context->height;
		int plane_bytes [4];
		
		switch (GetPixelFormat ()) {
		case MoonPixelFormatYUV420P:
			plane_bytes [0] = height * frame->linesize [0];
			plane_bytes [1] = height * frame->linesize [1] / 2;
			plane_bytes [2] = height * frame->linesize [2] / 2;
			plane_bytes [3] = 0;
			break;
		default:
			LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (): Unknown output format, can't calculate byte number.\n");
			plane_bytes [0] = 0;
			plane_bytes [1] = 0;
			plane_bytes [2] = 0;
			plane_bytes [3] = 0;
			break;
		}
		
		for (int i = 0; i < 4; i++) {
			if (plane_bytes [i] != 0) {
				if (posix_memalign ((void **)&mf->data_stride [i], 16, plane_bytes[i] + stream->GetMinPadding ())) {
					av_free (frame);
					ReportErrorOccurred ("FfmpegDecoder: out of memory");
					return;
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
	} else if (stream->IsAudio ()) {
		MpegFrameHeader mpeg;
		int remain = mf->GetBufLen ();
		int offset = 0;
		int decoded_size = 0;
		guint8 *decoded_frames = NULL;

		LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (), got %i bytes to decode.\n", mf->GetBufLen ());
		
		if (frame_buffer != NULL) {
			// copy data previously not decoded in front of this data
			LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (), adding %i bytes previously not decoded.\n", frame_buffer_length);
			if (!mf->PrependData (frame_buffer_length, frame_buffer)) {
				/* Error has already been reported */
				return;
			}
			remain += frame_buffer_length;
			
			g_free (frame_buffer);
			frame_buffer = NULL;
		}

		do {
			int frame_size;
			int buffer_size = AUDIO_BUFFER_SIZE;

			if (stream->GetCodecId () == CODEC_MP3 && mpeg_parse_header (&mpeg, mf->GetBuffer () + offset)) {
				frame_size = mpeg_frame_length (&mpeg);
	
				if (frame_size > remain) {
					// the remaining data is not a complete mp3 frame
					// save it and decode it next time we're called.
					frame_buffer_length = remain;
					frame_buffer = (guint8 *) g_memdup (mf->GetBuffer () + offset, remain);
					remain = 0;
					continue;
				}
			} else {
				frame_size = mf->GetBufLen () - offset;
			}

			av_init_packet (&packet);
			packet.data = mf->GetBuffer () + offset;
			packet.size = frame_size;
			length = avcodec_decode_audio3 (context, (gint16 *) audio_buffer, &buffer_size, &packet);
			packet.data = NULL;
			packet.size = 0;

			if (length <= 0 || (buffer_size < frame_size && buffer_size != 0)) {
				char *msg = g_strdup_printf ("FfmpegDecoder: Error while decoding audio frame (length: %d, frame_size. %d, buflen: %u pts: %" G_GUINT64_FORMAT ")", length, frame_size, mf->GetBufLen (), mf->pts);
				ReportErrorOccurred (msg);
				g_free (msg);
				return;
			}

			LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (), used %i bytes of %i input bytes to get %i output bytes\n", length, mf->GetBufLen (), buffer_size);

			if (buffer_size > 0) {
				decoded_frames = (guint8 *) g_realloc (decoded_frames, buffer_size+decoded_size);
				memcpy (decoded_frames+decoded_size, audio_buffer, buffer_size);
				offset += length;
				remain -= length;
				decoded_size += buffer_size;
			} else {
				if (decoded_frames != NULL)
					g_free (decoded_frames);
				decoded_frames = NULL;
				remain = 0;
				decoded_size = 0;
			}	
		} while (remain > 0);

		mf->FreeBuffer ();

		mf->SetBuffer (decoded_frames);
		mf->SetBufLen (decoded_size);

		LOG_FFMPEG ("FfmpegDecoder::DecodeFrame (), got a total of %i output bytes.\n", mf->GetBufLen ());
	} else {
		ReportErrorOccurred ("Invalid media type.");
		return;
	}
	
	mf->AddState (MediaFrameDecoded);
	
	ReportDecodeFrameCompleted (mf);
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
 * FfmpegDemuxer
 */

gint32 FfmpegDemuxer::demuxers = 0;
MoonThread* FfmpegDemuxer::worker_thread = NULL;
MoonMutex FfmpegDemuxer::worker_mutex;
MoonMutex FfmpegDemuxer::worker_thread_mutex;
MoonCond FfmpegDemuxer::worker_cond;
List FfmpegDemuxer::worker_list;

#ifdef SANITY
#define VERIFY_FFMPEG_THREAD 											\
	if (!MoonThread::IsThread (worker_thread)) {								\
		printf ("%s: this method should only be called on the ffmpeg demuxer thread.\n", __func__);	\
	}
#else
#define VERIFY_FFMPEG_THREAD
#endif

class FfmpegNode : public List::Node {
private:
	FfmpegDemuxer *demuxer;
	IMediaStream *stream;

public:
	enum Action {
		Open,
		GetFrame,
		Seek
	};
	Action action;
	guint64 pts;

	FfmpegNode (Action action, FfmpegDemuxer *demuxer, IMediaStream *stream = NULL)
	{
		this->action = action;
		this->demuxer = demuxer;
		this->demuxer->ref ();
		this->stream = stream;
		if (this->stream)
			this->stream->ref ();
	}
	virtual ~FfmpegNode ()
	{
		demuxer->unref ();
		if (stream)
			stream->unref ();
	}
	FfmpegDemuxer *GetDemuxer () { return demuxer; }
	IMediaStream *GetStream () { return stream; }
};

FfmpegDemuxer::FfmpegDemuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer)
	: IMediaDemuxer (Type::FFMPEGDEMUXER, media, source)
{
	ffmpeg_to_moon_index = NULL;
	format_context = NULL;
	frame_queue = NULL;
	current_position = 0;
	last_buffer = false;
	read_closure = NULL;
	current_buffer = initial_buffer;
	current_buffer->ref ();
	io_context = NULL;

	worker_thread_mutex.Lock();
	if (InterlockedExchangeAdd (&demuxers, 1) == 0) {
		MoonThread::Start (&worker_thread, WorkerLoop);
	}
	worker_thread_mutex.Unlock();
}

void
FfmpegDemuxer::Dispose ()
{
	worker_thread_mutex.Lock();
	if (InterlockedDecrement (&demuxers) == 0) {
		/* No more demuxers, stop the thread */
		worker_mutex.Lock();
		worker_list.Clear (true);
		worker_cond.Signal();
		worker_mutex.Unlock();
		/* We need to signal any waiting reads that we don't have to read more */
		wait_mutex.Lock();
		wait_cond.Signal();
		wait_mutex.Unlock();
		/* We need to unlock the worker_mutex before joining the thread, otherwise we'll deadlock. This is also the reason we're using two mutexes. */
		worker_thread->Join ();
	} else {
		worker_mutex.Lock();
		/* Remove any pending work for this demuxer */
		FfmpegNode *next;
		FfmpegNode *node = (FfmpegNode *) worker_list.First ();
		while (node != NULL) {
			next = (FfmpegNode *) node->next;
			if (node->GetDemuxer () == this)
				worker_list.Remove (node);
			node = next;
		}
		worker_mutex.Unlock();
	}
	worker_thread_mutex.Unlock();

	if (current_buffer) {
		current_buffer->unref ();
		current_buffer = NULL;
	}
	IMediaDemuxer::Dispose ();
}

FfmpegDemuxer::~FfmpegDemuxer ()
{
	for (int i = 0; i < GetStreamCount (); i++) {
		delete frame_queue [i];
	}
	g_free (frame_queue);
	g_free (ffmpeg_to_moon_index);

	if (format_context != NULL) {
		for (guint32 i = 0; i < format_context->nb_streams; i++) {
			av_freep (&format_context->streams [i]->codec);
			av_freep (&format_context->streams [i]);
		}
		av_free (format_context);
	}

	if (io_context) {
		g_free (io_context->buffer);
		av_free (io_context);
	}
}

AVInputFormat * 
FfmpegDemuxer::GetInputFormat (MemoryBuffer *source)
{
	/* Thread-safe method (output only depends on input) */
	AVProbeData data;
	data.filename = NULL;
	data.buf = (guint8 *) source->GetCurrentPtr ();
	data.buf_size = source->GetRemainingSize ();

	ffmpeg_mutex.Lock();
	AVInputFormat *format = av_probe_input_format (&data, true);
	ffmpeg_mutex.Unlock();

	LOG_FFMPEG ("FfmpegDemuxer::GetInputFormat (): got format: %s %s\n", format ? format->name : "<none>", format ? format->long_name : "");

	return format;
}

void
FfmpegDemuxer::SeekAsyncInternal (guint64 pts)
{
	FfmpegNode *node = new FfmpegNode (FfmpegNode::Seek, this);
	node->pts = pts;
	AddWork (node);
}

void
FfmpegDemuxer::Seek (guint64 pts)
{
	LOG_FFMPEG ("FfmpegDemuxer::Seek (%" G_GUINT64_FORMAT ")\n", pts);

	VERIFY_FFMPEG_THREAD;

	wait_mutex.Lock();
	for (int i = 0; i < GetStreamCount (); i++) {
		frame_queue [i]->Clear (true);
	}
	if (current_buffer != NULL) {
		current_buffer->unref ();
		current_buffer = NULL;
	}
	current_position = 0;
	last_buffer = false;
	wait_mutex.Unlock();

	for (int i = 0; i < GetStreamCount (); i++) {
		int ffmpeg_index = -1;
		for (unsigned int f = 0; f < format_context->nb_streams; f++) {
			if (ffmpeg_to_moon_index [f] == i) {
				ffmpeg_index = f;
				break;
			}
		}
		if (ffmpeg_index == -1)
			continue;
		int res = av_seek_frame (format_context, ffmpeg_index, pts, 0);
		if (res < 0) {
			ReportErrorOccurred ("FfmpegDemuxer: seek error in ffmpeg.\n");
			return;
		}
	}

	LOG_FFMPEG ("FfmpegDemuxer::Seek (%" G_GUINT64_FORMAT "): Completed\n", pts);

	ReportSeekCompleted (pts);
}

void
FfmpegDemuxer::SwitchMediaStreamAsyncInternal (IMediaStream *stream)
{
	printf ("FfmpegDemuxer::SwitchMediaStreamAsyncInternal (%s): Not implemented\n", stream->GetTypeName ());
	ReportSwitchMediaStreamCompleted (stream);
}

int
FfmpegDemuxer::ReadCallback (void *opaque, uint8_t *buf, int buf_size)
{
	return ((FfmpegDemuxer *) opaque)->Read (buf, buf_size);
}

MediaResult
FfmpegDemuxer::AsyncReadCallback (MediaClosure *c)
{
	VERIFY_MEDIA_THREAD;
	MediaReadClosure *closure = (MediaReadClosure *) c;
	FfmpegDemuxer *demuxer = (FfmpegDemuxer *) closure->GetContext ();

	demuxer->wait_mutex.Lock();
	if (demuxer->current_buffer)
		demuxer->current_buffer->unref ();
	demuxer->current_buffer = closure->GetData ();
	demuxer->current_buffer->ref ();
	demuxer->last_buffer = demuxer->current_buffer->GetSize () != closure->GetCount ();
#if SANITY
	if (demuxer->read_closure != closure) {
		printf ("FfmpegDemuxer::AsyncReadCallback (): the instance read_callback (%p) isn't the one we got back (%p)!\n", demuxer->read_closure, closure);
	}
#endif
	if (demuxer->read_closure) {
		demuxer->read_closure->unref ();
		demuxer->read_closure = NULL;
	}
	LOG_FFMPEG ("FfmpegDemuxer::AsyncReadCallback (): got data, signalling ffmpeg thread that it can continue reading.\n");
	demuxer->wait_cond.Signal();
	demuxer->wait_mutex.Unlock();

	return MEDIA_SUCCESS;
}

MediaResult
FfmpegDemuxer::ExecuteAsyncReadCallback (MediaClosure *closure)
{
	LOG_FFMPEG ("FfmpegDemuxer::ExecuteAsyncReadCallback ()\n");
	FfmpegDemuxer *demuxer = (FfmpegDemuxer *) closure->GetContext ();
	if (demuxer->source != NULL)
		demuxer->source->ReadAsync (demuxer->read_closure);
	return MEDIA_SUCCESS;
}

int
FfmpegDemuxer::Read (uint8_t *buf, int buf_size)
{
	Media *media;
	int result = 0;

	LOG_FFMPEG ("FfmpegDemuxer::Read (%i)\n", buf_size);
	
	VERIFY_FFMPEG_THREAD;

	media = GetMediaReffed ();

	if (media == NULL) {
		LOG_FFMPEG ("FfmpegDemuxer::Read (%i): no media.\n", buf_size);
		return 0;
	}

	wait_mutex.Lock();

	/* Request more data if we don't have enough */
	while (current_buffer == NULL || (current_buffer->GetRemainingSize () < buf_size && !last_buffer)) {
		if (read_closure == NULL) {
			if (current_buffer != NULL) {
				current_position += current_buffer->GetPosition ();
			}

			read_closure = new MediaReadClosure (media, AsyncReadCallback, this, current_position, buf_size < 10240 ? 10240 : buf_size);

			LOG_FFMPEG ("FfmpegDemuxer::Read (%i): not enough data, requesting %u bytes at %" G_GUINT64_FORMAT " (we have %" G_GUINT64_FORMAT " bytes available, last buffer: %i)\n",
				buf_size, read_closure->GetCount (), read_closure->GetOffset (), current_buffer ? current_buffer->GetRemainingSize () : -1, last_buffer);
			
			MediaClosure *execute_closure = new MediaClosure (media, ExecuteAsyncReadCallback, this, "FfmpegDemuxer::Read [execute closure]");
			media->EnqueueWork (execute_closure);
			execute_closure->unref ();
		} else {
			LOG_FFMPEG ("FfmpegDemuxer::Read (%i): previous read closure still present (probably due to a spurious wakeup from the wait) - not creating a new one. Waiting again.\n", buf_size);
		}
		if (demuxers == 0) {
			/* We're shutting down the ffmpeg thread */
			break;
		}
		wait_cond.Wait(wait_mutex);
	}
	/* read the data we have */
	if (current_buffer != NULL) {
		if (current_buffer->GetRemainingSize () < buf_size) {
			result = current_buffer->GetRemainingSize ();
		} else {
			result = buf_size;
		}
		LOG_FFMPEG ("FfmpegDemuxer::Read (): reading %u bytes (has %" G_GINT64_FORMAT " bytes available)\n", result, current_buffer->GetRemainingSize ());
		current_buffer->Read (buf, result);
	}
	wait_mutex.Unlock();

	media->unref ();
	
	LOG_FFMPEG ("FfmpegDemuxer::Read (requested %i bytes): %i bytes read\n", buf_size, result);

	return result;
}

int64_t
FfmpegDemuxer::SeekCallback (void *opaque, int64_t offset, int whence)
{
	return ((FfmpegDemuxer *) opaque)->Seek (offset, whence);
}

int64_t
FfmpegDemuxer::Seek (int64_t offset, int whence)
{
	LOG_FFMPEG ("FfmpegDemuxer::Seek (%" G_GINT64_FORMAT ", %i)\n", offset, whence);

	VERIFY_FFMPEG_THREAD;
	
	switch (whence) {
	case SEEK_SET:
		wait_mutex.Lock();
		current_position = offset;
		if (current_buffer) {
			current_buffer->unref ();
			current_buffer = NULL;
		}
		wait_mutex.Unlock();
		return offset;
	case SEEK_CUR:
		wait_mutex.Lock();
		current_position += offset;
		if (current_buffer) {
			current_buffer->unref ();
			current_buffer = NULL;
		}
		wait_mutex.Unlock();
		return offset;
	case AVSEEK_SIZE:
		return source->GetSize ();
	case SEEK_END:
		/* not supported */
	default:
		return -1;
	}
}

void
FfmpegDemuxer::OpenDemuxerAsyncInternal ()
{
	AddWork (new FfmpegNode (FfmpegNode::Open, this));
}

void
FfmpegDemuxer::Open ()
{
	int res;
	IMediaStream **streams;
	IMediaStream *stream = NULL;
	AVInputFormat *input_format;
	Media *media;
	int number_of_streams = 0; /* Number of audio and video streams to be outputted */
	int current_stream_index = 0; /* In case there are streams that are ignored */

	VERIFY_FFMPEG_THREAD;

	media = GetMediaReffed ();
	if (media == NULL) {
		/* Possibly disposed, do nothing */
		goto cleanup;
	}
	
	/* Get the input format. This should never return null since we checked for support earlier */
	input_format = GetInputFormat (current_buffer);
	if (input_format == NULL){
		ReportErrorOccurred ("FfmpegDemuxer: Input format could not be found");
		goto cleanup;
	}

	current_buffer->SeekSet (0);

	/* Create the byte context */
	io_context = avio_alloc_context ((unsigned char *) g_malloc (1024), 1024, false, this, ReadCallback, NULL, SeekCallback);

	/* Open the stream */
	if (av_open_input_stream (&format_context, io_context, "Moonlight stream", input_format, NULL) != 0) {
		ReportErrorOccurred ("FfmpegDemuxer: Input stream could not be opened");
		goto cleanup;
	}

	/* Get stream info (some header-less or header-weak formats needs this) */
	res = av_find_stream_info (format_context);
	if (res < 0) {
		ReportErrorOccurred ("FfmpegDemuxer: Could not get stream info");
		goto cleanup;
	}

	/* Count the streams that we will actually use */
	for (unsigned int i = 0; i < format_context->nb_streams; i++) {
		if (format_context->streams [i]->codec->codec_type == CODEC_TYPE_VIDEO ||
			(format_context->streams [i]->codec->codec_type == CODEC_TYPE_AUDIO)) {
			number_of_streams++;
		}
	}

	/* Allocate memory */
	streams = (IMediaStream **) g_malloc0 (sizeof (IMediaStream *) * (number_of_streams + 1));
	frame_queue = (List **) g_malloc0 (sizeof (List *) * number_of_streams);
	ffmpeg_to_moon_index = (gint32*) g_malloc (sizeof (gint32) * format_context->nb_streams);

	/* Iterate through all the streams and find the audio and video. Set stream-specific data */
	for (guint32 i = 0; i < format_context->nb_streams; i++) {
		AVCodecContext *codec_context = format_context->streams [i]->codec;
		ffmpeg_to_moon_index [i] = -1;

		if (codec_context->codec_type == CODEC_TYPE_VIDEO) {
			VideoStream *video = new VideoStream (media);

			video->SetWidth (codec_context->width);
			video->SetHeight (codec_context->height);
			video->SetBitRate (codec_context->bit_rate);
			video->SetBitsPerSample (codec_context->bits_per_coded_sample);
			video->SetPtsPerFrame (PTS_PER_MILLISECOND * 1000ULL * format_context->streams [i]->time_base.num / format_context->streams [i]->time_base.den);

			stream = video;
		} else if (codec_context->codec_type == CODEC_TYPE_AUDIO) {
			AudioStream* audio = new AudioStream (media);

			audio->SetChannels (codec_context->channels);
			audio->SetBlockAlign (codec_context->block_align);
			audio->SetSampleRate (codec_context->sample_rate);
			audio->SetBitRate (codec_context->bit_rate);
			audio->SetBitsPerSample (codec_context->bits_per_coded_sample);
			
			stream = audio;
		} else {
			/* TODO: marker stream (CODEC_TYPE_DATA). Ignore other type of streams (SUBTITLE, et al.) */
			continue;
		}

		/* Fill in common data */
		stream->SetCodecId (codec_context->codec_tag);
		stream->SetIndex (current_stream_index);
		stream->SetDuration (format_context->streams [i]->duration);
		stream->SetExtraDataSize (codec_context->extradata_size);
		stream->SetExtraData (NULL);

		if (stream->GetExtraDataSize () > 0) {
			stream->SetExtraData (g_malloc0 (stream->GetExtraDataSize ()));
			memcpy (stream->GetExtraData (), codec_context->extradata, stream->GetExtraDataSize ());
		}

		streams [current_stream_index] = stream;
		frame_queue [current_stream_index] = new List ();
		ffmpeg_to_moon_index [i] = current_stream_index;

		current_stream_index++;
	}

	LOG_FFMPEG ("FfmpegDemuxer::Open (): succeeded.\n");
	SetStreams (streams, number_of_streams);
	ReportOpenDemuxerCompleted ();

	for (int i = 0; i < number_of_streams; i++)
		streams [i]->unref ();

cleanup:
	if (media != NULL)
		media->unref ();
}

void 
FfmpegDemuxer::GetFrameAsyncInternal (IMediaStream *stream)
{
	AddWork (new FfmpegNode (FfmpegNode::GetFrame, this, stream));
}

void
FfmpegDemuxer::GetFrame (IMediaStream *stream)
{
	/* av_read_frame returns both audio and video frames, so keep iterating through 
	 * all frames and if they aren't for the stream we are looking for
	 * add them to the proper queue to be returned later */

	int moon_stream_index;
	AVPacket packet;
	FrameNode *node;
	List *current_queue = frame_queue [stream->GetIndex ()];

	/* Check if we have a frame in our list of previously demuxed frames */
	node = (FrameNode *) current_queue->First ();
	if (node != NULL) {
		LOG_FFMPEG ("FfmpegDemuxer::GetFrame (%s): returning %" G_GUINT64_FORMAT " [queued]\n", stream->GetTypeName (), MilliSeconds_FromPts (node->frame->pts));
		ReportGetFrameCompleted (node->frame);
		current_queue->Remove (node);
		return;
	}

	while (av_read_frame (format_context, &packet) >= 0) {
		AVStream *av_stream = format_context->streams [packet.stream_index];
		moon_stream_index = ffmpeg_to_moon_index [packet.stream_index];

		if (moon_stream_index == -1) {
			/* We don't care about this packet */
			av_free_packet (&packet);
			continue;
		}

		MediaFrame *frame = new MediaFrame (GetStream (moon_stream_index));
		frame->pts = PTS_PER_MILLISECOND * 1000ULL * packet.pts * av_stream->time_base.num / av_stream->time_base.den;
		frame->SetDuration (packet.duration);
		if (!frame->FetchData (packet.size, packet.data)) {
			frame->unref ();
			av_free_packet (&packet);
			return;
		}

		frame->AddState (MediaFrameDemuxed);
		if (packet.flags & PKT_FLAG_KEY)
			frame->AddState (MediaFrameKeyFrame);

		av_free_packet (&packet);

		if (moon_stream_index != stream->GetIndex ()) {
			frame_queue [moon_stream_index]->Append (new FrameNode (frame));
			frame->unref ();
		} else {
			LOG_FFMPEG ("FfmpegDemuxer::GetFrame (%s): returning %" G_GUINT64_FORMAT "\n", stream->GetTypeName (), MilliSeconds_FromPts (frame->pts));
			ReportGetFrameCompleted (frame);
			frame->unref ();
			return;
		}
	} 

	/* No more frames found */
	ReportGetFrameCompleted (NULL);
}

void
FfmpegDemuxer::AddWork (List::Node *node)
{
	if (IsDisposed ())
		return;

	worker_mutex.Lock();
	worker_list.Append (node);
	worker_cond.Signal();
	worker_mutex.Unlock();
}

void *
FfmpegDemuxer::WorkerLoop (void *data)
{
	bool quit = false;
	FfmpegNode *work;
	LOG_FFMPEG ("FfmpegDemuxer::WorkerLoop () [Starting]\n");
	do {
		worker_mutex.Lock();
		quit = demuxers == 0;
		while (!quit && worker_list.First () == NULL) {
			worker_cond.Wait(worker_mutex);
			quit = demuxers == 0;
		}
		work = NULL;
		if (!quit) {
			work = (FfmpegNode *) worker_list.First ();
			if (work != NULL)
				worker_list.Unlink (work);
		}
		worker_mutex.Unlock();
		if (work != NULL) {
			LOG_FFMPEG ("ffmpeg thread got work: %i\n", work->action);
			work->GetDemuxer ()->SetCurrentDeployment (false);
			switch (work->action) {
			case FfmpegNode::Open:
				work->GetDemuxer ()->Open ();
				break;
			case FfmpegNode::Seek:
				work->GetDemuxer ()->Seek (work->pts);
				break;
			case FfmpegNode::GetFrame:
				work->GetDemuxer ()->GetFrame (work->GetStream ());
				break;
			}
			LOG_FFMPEG ("ffmpeg thread got work: %i [Done]\n", work->action);
			delete work;
		}
	} while (!quit);
	LOG_FFMPEG ("FfmpegDemuxer::WorkerLoop () [Exiting]\n");
	return NULL;
}

/*
 * FfmpegDemuxerInfo
 */
MediaResult
FfmpegDemuxerInfo::Supports (MemoryBuffer *source)
{
	if (FfmpegDemuxer::GetInputFormat (source) == NULL)
		return MEDIA_FAIL;

	return MEDIA_SUCCESS;
}
 
IMediaDemuxer*
FfmpegDemuxerInfo::Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer)
{
	return new FfmpegDemuxer (media, source, initial_buffer);
}

};
