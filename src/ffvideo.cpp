/*
 * video.c: The moonlight's video item
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 * Inspiration comes from ffplay.c, which is:
 *
 *    * Copyright (c) 2003 Fabrice Bellard
 * 
 */
#define __STDC_CONSTANT_MACROS
#include <config.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "runtime.h"

#include <fcntl.h>

// video specific 
G_BEGIN_DECLS
#include <avformat.h>
#include <stdint.h>
#include <avcodec.h>
#include <swscale.h>
G_END_DECLS

#include <sys/time.h>

#include <SDL.h>

#include "cutil.h"


#define ENABLE_AUDIO
#define AUDIO_SAMPLE_SIZE 1024

#define ALIGN(addr,size) (uint8_t *) (((uint32_t) (((uint8_t *) (addr)) + (size) - 1)) & ~((size) - 1))


typedef struct {
	int stream_index;
	int duration;
	int64_t pts;
	size_t size;
	uint8_t data[1];
} QPacket;


static GStaticMutex ffmpeg_lock = G_STATIC_MUTEX_INIT;

// These come from ffmpeg, we might want to tune:
#define MAX_VIDEO_SIZE (1024 * 1024)
#define MAX_AUDIO_SIZE (80 * 1024)

class VideoFfmpeg : public Video {
public:
	int w, h;

	VideoFfmpeg (const char *filename);
	~VideoFfmpeg ();

	// Virtual method overrides
	void getbounds ();
	void render (Surface *s, int x, int y, int width, int height);
	virtual Point getxformorigin ();

	GThread    *decode_thread_id;
	int         pipes [2];
	GIOChannel *pipe_channel;
	
	//
	// AV Fields
	//
	AVFormatContext *av_format_context;

	// Video
	AVCodec     *video_codec;
	AVStream    *video_stream;
	int          video_stream_idx;
	GMutex      *video_mutex;
	
	// Audio
	SDL_AudioSpec audio_spec;
	AVCodec     *audio_codec;
	AVStream    *audio_stream;
	int          audio_stream_idx;
	GMutex      *audio_mutex;
	
	uint64_t     target_pts;
	
	//
	// Video and audio frames + usage of the queues
	//
	GAsyncQueue *video_frames;
	int          video_frames_size;
	GAsyncQueue *audio_frames;
	int          audio_frames_size;
	
	//
	// Timing components, the next time to display.
	//
	double   micro_to_pts;          // how to convert microseconds to pts (pts units are in video_stream->time_base units)
	uint64_t play_start_time;   	// Playback starts at this host time.
	uint64_t initial_pts;		// The PTS of the first frame when we started playing
	int      frame_size;            // microseconds between frames.
	guint    timeout_handle;

	// The constant is in bytes, so we get 2 full seconds.
#define AUDIO_BUFLEN (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)
	uint8_t audio_buffer[AUDIO_BUFLEN + 1];
	uint8_t *audio_outbuf;
	uint8_t *audio_outptr;
	
	//
	// The components used to render the buffer
	//

	// The buffer where we store the RGB data
	unsigned char *video_rgb_buffer;

	// The cairo surface that represents video_rgb_buffer
	cairo_surface_t *video_cairo_surface;

	// The SwsContext image convertor
	struct SwsContext *video_scale_context;
};


void
VideoFfmpeg::getbounds ()
{
	double res [6];
	AVCodecContext *cc;
	
	Surface *s = item_get_surface (this);
	if (s == NULL){
		// not yet attached
		return;
	}

	// If we have not been initialized yet, we cant compute the bounds
	if (video_stream == NULL || video_stream->codec == NULL){
		x1 = y1 = x2 = y2 = 0;
		return;
	}
	cc = video_stream->codec;

	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);

	cairo_rectangle (s->cairo, 0, 0, cc->width, cc->height);
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);

	// The extents are in the coordinates of the transform, translate to device coordinates
	x_cairo_matrix_transform_bounding_box (&absolute_xform, &x1, &y1, &x2, &y2);

	cairo_restore (s->cairo);
}

static QPacket *
pkt_new (AVPacket *pkt)
{
	QPacket *qpkt;
	
	qpkt = (QPacket *) g_malloc (sizeof (QPacket) + pkt->size);
	qpkt->stream_index = pkt->stream_index;
	qpkt->duration = pkt->duration;
	qpkt->size = pkt->size;
	qpkt->pts = pkt->pts;
	
	memcpy (qpkt->data, pkt->data, pkt->size);
	
	return qpkt;
}

#define pkt_free(x) g_free (x)


static void restart_timer (VideoFfmpeg *video);

static void
notify_frame_ready (VideoFfmpeg *video)
{
	uint8_t ready = 0;
	size_t n;
	
	do {
		n = write (video->pipes [1], &ready, 1);
	} while (n == -1 && errno == EINTR);
}

static gboolean
callback_video_inited (gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	AVCodecContext *cc;
	
	cc = video->video_stream->codec;
	video->video_cairo_surface = cairo_image_surface_create_for_data (
		video->video_rgb_buffer, CAIRO_FORMAT_ARGB32,
		cc->width, cc->height, cc->width * 4);
	
	item_update_bounds ((UIElement *) video);
	video->w = video->video_stream->codec->width;
	video->h = video->video_stream->codec->height;
	
	// Track where we are at now
	video->initial_pts = video->video_stream->start_time;
	video->micro_to_pts = av_q2d (video->video_stream->codec->time_base);
	
	restart_timer (video);

	return FALSE;
}

static void
sdl_audio_callback (void *user_data, uint8_t *stream, int len)
{
	VideoFfmpeg *video = (VideoFfmpeg *) user_data;
	uint8_t *inptr, *outbuf;
	int inleft, outlen;
	QPacket *pkt;
	int n;
	
	outlen = video->audio_outptr - video->audio_outbuf;
	
	while (outlen < len) {
		g_mutex_lock (video->audio_mutex);
		pkt = (QPacket *) g_async_queue_pop (video->audio_frames);
		video->audio_frames_size -= pkt->size;
		g_mutex_unlock (video->audio_mutex);
		
		video->target_pts = pkt->pts;
		
		inptr = pkt->data;
		inleft = pkt->size;
		
		while (inleft > 0) {
			/* avcodec_decode_audio2() requires that the outbuf be 16bit word aligned */
			outbuf = ALIGN (video->audio_outptr, 2);
			outlen = (video->audio_outbuf + AUDIO_BUFLEN) - outbuf;
			
			n = avcodec_decode_audio2 (video->audio_stream->codec,
						   (int16_t *) outbuf, &outlen,
						   inptr, inleft);
			
			if (n > 0) {
				inleft -= n;
				inptr += n;
				
				// append the newly decoded buffer to the end of the remaining audio
				// buffer from our previous loop (if re-alignment was required).
				if (outbuf > video->audio_outptr)
					memmove (video->audio_outptr, outbuf, outlen);
				video->audio_outptr += outlen;
			} else {
				break;
			}
		}
		
		pkt_free (pkt);
		
		outlen = video->audio_outptr - video->audio_outbuf;
	}
	
	n = len < outlen ? len : outlen;
	memcpy (stream, video->audio_outbuf, n);
	if (n < outlen) {
		memmove (video->audio_outbuf, video->audio_outbuf + n, outlen - n);
		video->audio_outptr -= n;
	} else {
		video->audio_outptr = video->audio_outbuf;
	}
	
	if (n < len)
		memset (stream + n, 0, len - n);
}

static int
init_video (VideoFfmpeg *video)
{
	AVCodecContext *codec;
	SDL_AudioSpec spec;
	AVStream *stream;
	int i, n;
	
	g_static_mutex_lock (&ffmpeg_lock);
	
	if (av_open_input_file (&video->av_format_context, video->filename, NULL, 0, NULL) < 0) {
		g_static_mutex_unlock (&ffmpeg_lock);
		g_error ("ERROR: open input file `%s'\n", video->filename);
		return -1;
	}
	
	if (video->av_format_context == (void *) &rtsp_demuxer) {
		g_static_mutex_unlock (&ffmpeg_lock);
		g_error ("ERROR: stream info\n");
		return -1;
	}
	
	av_read_play (video->av_format_context);
	
        if (av_find_stream_info (video->av_format_context) < 0) {
		g_static_mutex_unlock (&ffmpeg_lock);
		g_error ("ERROR: stream info\n");
		return -1;
	}
	
	for (i = 0; i < video->av_format_context->nb_streams; i++){
		stream = video->av_format_context->streams [i];
		
		switch (stream->codec->codec_type) {
		case CODEC_TYPE_AUDIO:
			printf ("audio stream id = %d\n", i);
			video->audio_stream_idx = i;
#ifdef ENABLE_AUDIO
			if (!(video->audio_codec = avcodec_find_decoder (stream->codec->codec_id)))
				break;
			
			video->audio_stream = video->av_format_context->streams [i];
			avcodec_open (video->audio_stream->codec, video->audio_codec);
			
			codec = video->audio_stream->codec;
			
			spec.freq = codec->sample_rate;
			spec.format = AUDIO_S16SYS;
			if (codec->channels > 2)
				codec->channels = 2;
			spec.channels = codec->channels;
			spec.silence = 0;
			spec.samples = AUDIO_SAMPLE_SIZE;
			spec.callback = sdl_audio_callback;
			spec.userdata = video;
			if (SDL_OpenAudio (&spec, &video->audio_spec) < 0) {
				printf ("SDL_OpenAudio: %s\n", SDL_GetError ());
				return -1;
			}
			
			//SDL_PauseAudio (0);
#endif /* ENABLE_AUDIO */
			break;
		case CODEC_TYPE_VIDEO:
			printf ("video stream id = %d\n", i);
			video->video_stream_idx = i;
			
			if (!(video->video_codec = avcodec_find_decoder (stream->codec->codec_id)))
				break;
			
			video->video_stream = video->av_format_context->streams [i];
			avcodec_open (video->video_stream->codec, video->video_codec);
			
			// Allocate our rendering buffer and cairo surface
			//
			codec = video->video_stream->codec;
			video->video_rgb_buffer = (unsigned char *) g_malloc (codec->width * codec->height * 4);
			
			// The YUB to RGB scaleter/translator
			printf ("Pix_fmt=%d\n", codec->pix_fmt);
			video->video_scale_context = sws_getContext (
				codec->width, codec->height, codec->pix_fmt, codec->width, codec->height,
				PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
			break;
		}
	}
	
	g_static_mutex_unlock (&ffmpeg_lock);
	
	//
	// notify the main thread that we are done loading the video, and that it
	// can pull the width/height information out of this video
	//
	g_idle_add (callback_video_inited, video);
	
	return 0;
}

//
// The queue-data thread, runs until complete
//
static gpointer
queue_data (gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	int video_stream_idx, audio_stream_idx;
	struct pollfd *ufds = NULL;
	AVFrame *video_frame;
	int i, n = 0;
	
	printf ("video thread %p starting...\n", g_thread_self ());
	
	if (init_video (video) == -1)
		return NULL;
	
	video_stream_idx = video->video_stream_idx;
	audio_stream_idx = video->audio_stream_idx;
	
	while (TRUE) {
		AVPacket pkt;
		int ret;
		
		if (av_read_frame (video->av_format_context, &pkt) < 0) {
			// ffmpeg stream is complete (or error), either way - nothing left to decode.
			break;
		}
		
		n++;
		
		if (pkt.stream_index == video_stream_idx) {
			//while (video->video_frames_size > MAX_VIDEO_SIZE) {
			//	// FIXME: use a GCond for this?
			//	printf ("sleeping...\n");
			//	g_usleep (1000);
			//}
			
			//printf ("queueing video packet\n");
			
			g_mutex_lock (video->video_mutex);
			g_async_queue_push (video->video_frames, pkt_new (&pkt));
			video->video_frames_size += pkt.size;
			g_mutex_unlock (video->video_mutex);
			
			if (g_async_queue_length (video->video_frames) == 1)
				notify_frame_ready (video);
#ifdef ENABLE_AUDIO
		} else if (pkt.stream_index == audio_stream_idx) {
			//printf ("queueing audio packet\n");
			
			g_mutex_lock (video->audio_mutex);
			g_async_queue_push (video->audio_frames, pkt_new (&pkt));
			video->audio_frames_size += pkt.size;
			g_mutex_unlock (video->audio_mutex);
#endif /* ENABLE_AUDIO */
		} else {
			printf ("unknown packet\n");
		}
		
		av_free_packet (&pkt);
	}
	
	while (g_async_queue_length (video->audio_frames) > 0)
		g_usleep (1000);
	
	printf ("video thread %p complete. decoded %d frames\n", g_thread_self (), n);
	
	return NULL;
}

//
// Convert the AVframe into an RGB buffer we can render
//
static void
convert_to_rgb (VideoFfmpeg *video, AVFrame *frame)
{
	AVCodecContext *cc = video->video_stream->codec;
	uint8_t *rgb_dest[3] = { video->video_rgb_buffer, NULL, NULL};
	int rgb_stride [3] = { cc->width * 4, 0, 0 };
	
	if (frame->data == NULL)
		return;
	
	sws_scale (video->video_scale_context, frame->data, frame->linesize, 0,
		   video->video_stream->codec->height, rgb_dest, rgb_stride);
}

static gboolean
load_next_frame (gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	uint64_t target_pts = (uint64_t) ((av_gettime () - video->play_start_time) * video->micro_to_pts) + video->initial_pts;
	AVFrame *frame = NULL;
	gboolean cont = TRUE;
	int redraw = 0;
	
#ifndef ENABLE_AUDIO
	// no audio to sync to
	target_pts = (uint64_t) ((av_gettime () - video->play_start_time) * video->micro_to_pts) + video->initial_pts;
#else /* ENABLE_AUDIO */
	// sync w/ audio
	target_pts = video->target_pts;
#endif
	
	while (g_async_queue_length (video->video_frames) > 0) {
		QPacket *pkt;
		long pkt_pts;
		
		g_mutex_lock (video->video_mutex);
		pkt = (QPacket *) g_async_queue_pop (video->video_frames);
		video->video_frames_size -= pkt->size;
		g_mutex_unlock (video->video_mutex);
		
		//
		// Always decode the frame, or we get glitches in the screen if we 
		// avoid the decoding phase
		//
		frame = avcodec_alloc_frame ();
		avcodec_decode_video (video->video_stream->codec, frame, &redraw, pkt->data, pkt->size);
		
		pkt_pts = pkt->pts;
		pkt_free (pkt);
		
		if (pkt_pts + video->frame_size < target_pts) {
			if (g_async_queue_length (video->video_frames) > 0) {
#ifdef DEBUG
				printf ("Initial PTS=%ld\n", video->initial_pts);
				printf ("Video Play Start: %ld\n", video->play_start_time);
				printf ("Current time: %ld\n", av_gettime ());
				printf ("micro to pts: %g\n", video->micro_to_pts);
				printf ("Frame PTS=%ld\n", frame->pts);
				printf ("Target PTS=%ld\n", target_pts);
				printf ("frame size=%d\n", video->frame_size);

				printf ("discarding packet at time=%g seconds because it is late\n", 
					(pkt_pts - video->video_stream->start_time) *
					av_q2d (video->video_stream->time_base));
#endif
				// we're going to drop this frame and try the next one
				av_free (frame);
				frame = NULL;
			} else {
				printf ("Going to use the packet, and stopping stop\n");
				// We do not have anything else, render this last frame, and
				// stop the timer.
				cont = FALSE;
				video->timeout_handle = 0;
				break;
			}
		} else {
			// we are in sync (or ahead) of audio playback
			break;
		}
	}
	
	if (redraw) {
		convert_to_rgb (video, frame);
		item_invalidate ((UIElement *) video);
	}
	
	if (frame != NULL)
		av_free (frame);
	
	// Federico suggested I could use this to process the expose at once:
	//
	//gdk_window_process_updates (get_surface (video)->drawing_area->window, FALSE);
	
	return cont;
}

static void
restart_timer (VideoFfmpeg *video)
{
	if (video->timeout_handle != 0)
		return;

	//video->frame_size = (int) (1000 * av_q2d (video->video_stream->time_base) * av_q2d (video->video_stream->r_frame_rate));

	// microseconds it takes to advance to the next frame
	video->frame_size = (int) (1000 / av_q2d (video->video_stream->r_frame_rate));

	video->play_start_time = av_gettime ();
	printf ("Adding timer for %d\n", video->frame_size);
	video->timeout_handle = g_timeout_add (video->frame_size, load_next_frame, video);
	SDL_PauseAudio (0);
}

//
// Called on the GUI thread when we got a frame to render.
//
static gboolean
video_ready (GIOChannel *source, GIOCondition condition, gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	uint8_t buf[32];
	ssize_t n;
	
	// clear the pipe
	do {
		n = read (video->pipes[0], buf, sizeof (buf));
	} while (n == -1 && errno == EINTR);
	
	restart_timer (video);
	
	return TRUE;
}

void
VideoFfmpeg::render (Surface *s, int x, int y, int width, int height)
{
	cairo_pattern_t *pattern, *old_pattern;

	// If we are not initialized yet, return.
	if (video_cairo_surface == NULL)
		return;

	if (video_stream == NULL || video_stream->codec == NULL){
		// FIXME: Draw something, some black box or something
		return;
	}
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
   
	cairo_set_source_surface (s->cairo, video_cairo_surface, 0, 0);

	cairo_rectangle (s->cairo, 0, 0, this->w, this->h);

	cairo_fill (s->cairo);

	cairo_restore (s->cairo);
}

static int ffmpeg_inited;

static void
ffmpeg_init ()
{
	if (!ffmpeg_inited) {
		avcodec_init ();
		av_register_all ();
		avcodec_register_all ();
		SDL_Init (SDL_INIT_AUDIO);
		ffmpeg_inited = TRUE;
	}
}

Video *
video_ffmpeg_new (const char *filename)
{
	VideoFfmpeg *video;

	ffmpeg_init ();

	video = new VideoFfmpeg (filename);

	return (Video *) video;
}

VideoFfmpeg::VideoFfmpeg (const char *filename) : Video (filename)
{
	video_codec = NULL;
	video_stream = NULL;
	audio_codec = NULL;
	audio_stream = NULL;
	video_rgb_buffer = NULL;
	video_cairo_surface = NULL;
	video_scale_context = NULL;
	av_format_context = NULL;
	
	audio_outbuf = ALIGN (audio_buffer, 2);
	audio_outptr = audio_outbuf;
	
	w = h = 0;
	timeout_handle = 0;
	initial_pts = 0;
	target_pts = 0;
	play_start_time = 0;
	frame_size = 0;
	micro_to_pts = 0;
	audio_frames_size = 0;
	video_frames_size = 0;

	pipe (pipes);
	fcntl (pipes [0], F_SETFL, O_NONBLOCK);
	
	audio_mutex = g_mutex_new ();
	video_mutex = g_mutex_new ();
	audio_frames = g_async_queue_new ();
	video_frames = g_async_queue_new ();
	pipe_channel = g_io_channel_unix_new (pipes [0]);
	g_io_add_watch (pipe_channel, G_IO_IN, video_ready, this);

	decode_thread_id = g_thread_create (queue_data, this, TRUE, NULL);
}

Point
VideoFfmpeg::getxformorigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	return Point (user_xform_origin.x * w, user_xform_origin.y * h);
}

VideoFfmpeg::~VideoFfmpeg ()
{
	// TODO:
	//    * Ask our thread to shutdown nicely.
	//
	fprintf (stderr, "We should stop the thread");
	
	g_async_queue_unref (audio_frames);
	g_async_queue_unref (video_frames);
	g_mutex_free (video_mutex);
	close (pipes [0]);
	close (pipes [1]);
	g_io_channel_close (pipe_channel);
	g_free (video_rgb_buffer);
	cairo_surface_destroy (video_cairo_surface);
}


/*
 * video_new:
 * @filename:
 * @x, @y: Position where to position the video
 *
 */
Video *
video_new (const char *filename)
{
	return video_ffmpeg_new (filename);
}

