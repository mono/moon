/*
 * pipeline.h: Ffmpeg related parts of the pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifdef INCLUDE_FFMPEG

#ifndef __MOON_PIPELINE_FFMPEG__
#define __MOON_PIPELINE_FFMPEG__

#include <glib.h>
#include <pthread.h>

G_BEGIN_DECLS
#include <limits.h>
#if HAVE_LIBAVCODEC_AVCODEC_H
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#else
#include <avcodec.h>
#include <avformat.h>
#endif
G_END_DECLS

#include "pipeline.h"
 
namespace Moonlight {

void register_ffmpeg ();

/*
 * FfmpegDemuxer
 */
class FfmpegDemuxer : public IMediaDemuxer {
private:
	/*
	 * The async data reading API of our pipeline makes the interaction with ffmpeg somewhat painful - ffmpeg
	 * wants data retrieval to block until data is read (or it fails). So we have to spin up another thread 
	 * just for this purpose - wait until data has been read on the media thread. We do all the demuxing interaction
	 * with ffmpeg on this thread. For now there is one ffmpeg demuxing thread which is shared between all
	 * FfmpegDemuxers, if this ends up being a problem we might need to change to one thread per demuxer (demuxing
	 * won't take long, except if the requested data hasn't been downloaded, in which case one slow download will
	 * make all other demuxers wait too) */

	gint32 *ffmpeg_to_moon_index; /*  ffmpeg thread only */
	AVFormatContext *format_context; /* ffmpeg thread only */
	List **frame_queue; /* ffmpeg thread only */
	MemoryBuffer *current_buffer; /* the current memory buffer. accessed from media and ffmpeg thread, needs locking with wait_mutex */
	gint64 current_position; /* the current position. accessed from media and ffmpeg thread, needs locking with wait_mutex */
	bool last_buffer; /* if current_buffer is the last data in the file. accessed from media and ffmpeg thread, needs locking with wait_mutex */
	MediaReadClosure *read_closure; /* the pending read closure. accessed from media and ffmpeg thread, needs locking with wait_mutex */
	ByteIOContext *byte_context; /* ffmpeg thread only */

	class FrameNode : public List::Node {
	public:
		MediaFrame *frame;
		FrameNode (MediaFrame *frame) { this->frame = frame; this->frame->ref (); }
		virtual ~FrameNode () { this->frame->unref (); }
	};

	pthread_mutex_t wait_mutex;
	pthread_cond_t wait_cond; /* signalled when a read has completed */

	static int ReadCallback (void *opaque, uint8_t *buf, int buf_size); /* ffmpeg calls this method when it wants more data. ffmpeg worker thread only */
	static int64_t SeekCallback (void *opaque, int64_t offset, int whence); /* ffmpeg calls this method when it requests a seek. ffmpeg worker thread only */
	int Read (uint8_t *buf, int buf_size); /* instance implementation of ReadCallback. ffmpeg worker thread only */
	int64_t Seek (int64_t offset, int whence); /* instance implementation of SeekCallback. ffmpeg worker thread only */

	void Open (); /* the implementation of OpenDemuxerAsyncInternal that can block (runs on the ffmpeg thread). ffmpeg worker thread only */
	void Seek (guint64 pts); /* the implementation of SeekASyncInternal that can block (runs on the ffmpeg thread). ffmpeg worker thread only */
	void GetFrame (IMediaStream *straem); /* the implementation of GetFrameAsyncInternal that can block (runs  on the ffmpeg thread). ffmpeg worker thread only */

	static MediaResult AsyncReadCallback (MediaClosure *closure); /* this method is called when the pipeline has finished a read request */
	static MediaResult ExecuteAsyncReadCallback (MediaClosure *closure); /* since IMediaSource::ReadAsync can only be called on the media thread, whenever the ffmpeg thread needs more data, we enqueue a call to this method from the ffmpeg thread, and this method does the actual read request */

	static gint32 demuxers; /* the number of ffmpeg demuxers, the ffmpeg thread is created when the first demuxer is created, and the ffmpeg thread is shutdown when the last demuxer is destroyed */
	static pthread_t worker_thread; /* the ffmpeg thread */
	static pthread_mutex_t worker_thread_mutex; /* demuxers can be created simultaneously on several media threads, this mutex is used to serialize access to the worker_thread variable */
	static pthread_mutex_t worker_mutex; /* used to serialize access to worker_list and worker_cond */
	static pthread_cond_t worker_cond; /* signalled when work is added */
	static void *WorkerLoop (void *data);
	static List worker_list; /* the list of work for the ffmpeg thread */
	void AddWork (List::Node *node);

protected:
	virtual ~FfmpegDemuxer ();

	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 pts);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream);

public:
	/* @SkipFactories */
	FfmpegDemuxer (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer);
	virtual void Dispose ();

	static AVInputFormat *GetInputFormat (MemoryBuffer *source); /* Thread-safe (output only depends on input) */
};

/*
 * FfmpegDemuxerInfo
 */
class FfmpegDemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (MemoryBuffer *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer); 
	virtual const char *GetName () { return "FFMpegDemuxer"; }
};

/*
 * FfmpegDecoder
 */
class FfmpegDecoder : public IMediaDecoder {
private:
	AVCodecContext *context;
	guint8* audio_buffer;
	guint8* frame_buffer;
	guint32 frame_buffer_length;
	guint64 last_pts;
	bool has_delayed_frame;

protected:
	virtual ~FfmpegDecoder () {}
	virtual void DecodeFrameAsyncInternal (MediaFrame* frame);
	virtual void OpenDecoderAsyncInternal ();
	
public:
	/* @SkipFactories */
	FfmpegDecoder (Media* media, IMediaStream* stream);
	virtual void Dispose ();	
	virtual void Cleanup (MediaFrame* frame);
	virtual void CleanState ();
	virtual bool HasDelayedFrame () {return has_delayed_frame; }
	virtual void InputEnded ();

	static PixelFormat ToFfmpegPixFmt (MoonPixelFormat format);	
	static MoonPixelFormat ToMoonPixFmt (PixelFormat format);
};

/*
 * FfmpegDecoderInfo
 */
class FfmpegDecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char* codec);
	virtual IMediaDecoder* Create (Media* media, IMediaStream* stream);
	virtual const char* GetName () { return "FfmpegDecoder"; }
};

};
#endif // __MOON_PIPELINE_FFMPEG__
#endif // INCLUDE_FFMPEG
