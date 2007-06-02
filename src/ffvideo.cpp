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
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "runtime.h"

//
#include <fcntl.h>

// video specific 
G_BEGIN_DECLS
#include <avformat.h>
#include <stdint.h>
#include <avcodec.h>
#include <swscale.h>
G_END_DECLS

#include <sys/time.h>

// Alsa
#include <asoundlib.h>

typedef enum {
	// When a new frame is ready
	CMD_NEWFRAME,
} GuiCommand;

// These come from ffmpeg, we might want to tune:
#define MAX_VIDEO_SIZE (1024 * 1024)
#define MAX_AUDIO_SIZE (80 * 1024)

class VideoFfmpeg : public Video {
public:
	int w, h;

	VideoFfmpeg (const char *filename, double x, double y);
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

	AVCodec     *audio_codec;
	AVStream    *audio_stream;
	int          audio_stream_idx;

	snd_pcm_t   *pcm;

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
	uint16_t  audio_buffer [AVCODEC_MAX_AUDIO_FRAME_SIZE];

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

static GMutex *video_lock;

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
	if (absolute_xform != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) absolute_xform);

	cairo_rectangle (s->cairo, x, y, cc->width, cc->height);
	cairo_fill_extents (s->cairo, &x1, &y1, &x2, &y2);

	cairo_restore (s->cairo);
}

static void restart_timer (VideoFfmpeg *video);

static void
send_command (VideoFfmpeg *video, uint8_t cmd)
{
	int ret;
	
	do {
		ret = write (video->pipes [1], &cmd, 1);
	} while (ret == -1 && errno == EINTR);
}

gboolean
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

//
// The decoder thread, runs until complete
//
static gpointer
decoder (gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	AVFrame *video_frame;
	int i;
	int video_stream_idx, audio_stream_idx;
	AVFormatParameters pars;
	
	printf ("Video THREAD STARTS\n");

	g_mutex_lock (video_lock);
	// avcodec setup
	//   auto-detect format
	//   default buffer size
	//   no additional parameters
	memset (&pars, 0, sizeof (pars));
	
	if (av_open_input_file (&video->av_format_context, video->filename, NULL, 0, NULL) < 0){
		g_error ("ERROR: open input file\n");
	}
	if (video->av_format_context == (void *) &rtsp_demuxer){
		g_error ("ERROR: stream info\n");
	}

	av_read_play (video->av_format_context);

        if (av_find_stream_info(video->av_format_context) < 0){
		g_error ("ERROR: stream info\n");
	}

	for (i = 0; i < video->av_format_context->nb_streams; i++){
		AVStream *stream = video->av_format_context->streams [i];
		AVCodecContext *cc;
		int n;

		switch (stream->codec->codec_type){
		case CODEC_TYPE_AUDIO:
			video->audio_codec = avcodec_find_decoder (stream->codec->codec_id);
			video->audio_stream = video->av_format_context->streams [i];
			avcodec_open (video->audio_stream->codec, video->audio_codec);
			video->audio_stream_idx = i;
			audio_stream_idx = video->audio_stream_idx;

			cc = video->audio_stream->codec;
			n = -1;
			//snd_pcm_open (&video->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
			if (n < 0)
				printf ("Error, can not initialize audio %d\n", video->pcm);
			else {
				snd_pcm_set_params (
					video->pcm, 
					SND_PCM_FORMAT_S16, 
					SND_PCM_ACCESS_RW_INTERLEAVED,
					cc->channels,
					cc->sample_rate,
					1, 0);
			}
			break;

		case CODEC_TYPE_VIDEO:
			video->video_codec = avcodec_find_decoder (stream->codec->codec_id);
			video->video_stream = video->av_format_context->streams [i];
			avcodec_open (video->video_stream->codec, video->video_codec);
			video->video_stream_idx = i;
			video_stream_idx = i;
			
			// Allocate our rendering buffer and cairo surface
			//
			cc = video->video_stream->codec;
			video->video_rgb_buffer = (unsigned char *) malloc (cc->width * cc->height * 4);

			// The YUB to RGB scaleter/translator
			printf ("Pix_fmt=%d\n", cc->pix_fmt);
			video->video_scale_context = sws_getContext (
				cc->width, cc->height, cc->pix_fmt, cc->width, cc->height,
				PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
			break;
		}
	}
	g_mutex_unlock (video_lock);

	//
	// notify the main thread that we are done loading the video, and that it
	// can pull the width/height information out of this video
	//
	g_idle_add (callback_video_inited, video);
	int frame = 0;

	while (TRUE){
		AVPacket *pkt;
		int ret;

		// 
		// Sleep while the queues are full
		//
		if ((video->video_frames_size > MAX_VIDEO_SIZE) ||
		    (video->audio_frames_size > MAX_AUDIO_SIZE)){
			g_usleep (1000);
			continue;
		}

		pkt = g_new (AVPacket, 1);
		ret = av_read_frame (video->av_format_context, pkt);
		if (ret == -1){
			printf ("Failed to read frame, terminating\n");
			break;
		}

		frame++;
		g_mutex_lock (video->video_mutex);
		if (video->pcm != NULL && pkt->stream_index == audio_stream_idx){
			int frame_size_ptr = sizeof (video->audio_buffer);
			int decoded_bytes;

			//.	printf ("AUDIO at %d\n", frame);
			decoded_bytes = avcodec_decode_audio2 (
				video->audio_stream->codec, (int16_t *) video->audio_buffer, &frame_size_ptr,
				pkt->data, pkt->size);

			if (decoded_bytes > 0){
				printf ("Sending the video buffer with %d\n", decoded_bytes);
				int n = snd_pcm_writei (video->pcm, video->audio_buffer, decoded_bytes);
				printf ("n=%d %d %d %d\n", n, EBADFD, EPIPE, ESTRPIPE);
			} else {
				printf ("decoded bytes=%d\n", decoded_bytes);
			}
		} if (pkt->stream_index == video_stream_idx){
			g_async_queue_push (video->video_frames, pkt);
			video->video_frames_size += pkt->size;
			if (g_async_queue_length (video->video_frames) == 1)
				send_command (video, CMD_NEWFRAME);
		}
		g_mutex_unlock (video->video_mutex);
	}

	printf ("THREAD COMPLETES\n");
	
	return NULL;
}

//
// Convert the AVframe into an RGB buffer we can render
//
void
convert_to_rgb (VideoFfmpeg *video, AVFrame *frame)
{
	static struct SwsContext *img_convert_ctx;
	AVCodecContext *cc = video->video_stream->codec;
	uint8_t *rgb_dest[3] = { video->video_rgb_buffer, NULL, NULL};
	int rgb_stride [3] = { cc->width * 4, 0, 0 };

	sws_scale (video->video_scale_context, frame->data, frame->linesize,  0,
		   video->video_stream->codec->height,  rgb_dest, rgb_stride);

	// 
	// This is horrible, the ffmpeg scaler wont produce a usable
	// format (RGB32_1, BGR32_1 hang), so we have to manually swap
	// some values here
	//
	unsigned char *p = video->video_rgb_buffer;

// the stupid scaler gives us RGBA
// we need BGRA (CAIRO_FORMAT_ARGB32)

	for (int l = 0; l < cc->height; l++){
		for (int c = 0; c < cc->width; c += 1){
			unsigned char t;

			t = p [2];
			p [2] = p [0];
			p [0] = t;
			p += 4;
		}
	}
}

static gboolean
load_next_frame (gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	uint64_t target_pts = (uint64_t) ((av_gettime () - video->play_start_time) * video->micro_to_pts) + video->initial_pts;
	gboolean cont = TRUE;
	AVFrame *frame = NULL;
	int got_picture;

	while (g_async_queue_length (video->video_frames) != 0){
		AVPacket *pkt;
		long pkt_pts;

		g_mutex_lock (video->video_mutex);
		pkt = (AVPacket *) g_async_queue_pop (video->video_frames);
		video->video_frames_size -= pkt->size;
		g_mutex_unlock (video->video_mutex);

		//
		// Always decode the frame, or we get glitches in the screen if we 
		// avoid the decoding phase
		//
		frame = avcodec_alloc_frame ();
		avcodec_decode_video (video->video_stream->codec, frame, &got_picture, pkt->data, pkt->size);
		pkt_pts = pkt->pts;

		av_free_packet (pkt);
		g_free (pkt);
	
		if (pkt_pts + video->frame_size < target_pts){		
			// We got more stuff on the queue, discard this one
			if (g_async_queue_length (video->video_frames) > 1){
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
				// Remove this to discard
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

			break;
		}
			
	}

	if (got_picture){
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
}

//
// Called on the GUI thread when we got a frame to render.
//
gboolean
video_ready (GIOChannel *source, GIOCondition condition, gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	AVFrame *frame;
	int8_t commands [32];
	int ret, i;
	AVCodecContext *cc;

	do {
		ret = read (video->pipes [0], &commands, sizeof (commands));
	} while ((ret == -1 && ret == EINTR));

	// Nothing.
	if (ret <= 0)
		return TRUE;
	
	for (i = 0; i < ret; i++){
		switch (commands [i]){
		case CMD_NEWFRAME:
			restart_timer (video);
			break;
			
		default:
			fprintf (stderr, "video_ready: unknown command from decoding thread (%d)\n", commands [i]);
		}
	}

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
	if (absolute_xform != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) absolute_xform);
   
	cairo_set_source_surface (s->cairo, video_cairo_surface, this->x, this->y);

	cairo_rectangle (s->cairo, this->x, this->y, this->w, this->h);
	cairo_fill (s->cairo);

	cairo_restore (s->cairo);
}

static int ffmpeg_inited;

static void
ffmpeg_init ()
{
	if (!ffmpeg_inited){

		video_lock = g_mutex_new ();
		avcodec_init ();
		av_register_all ();
		avcodec_register_all ();
		ffmpeg_inited = TRUE;
	}
}
Video *
video_ffmpeg_new (const char *filename, double x, double y)
{
	VideoFfmpeg *video;

	ffmpeg_init ();

	video = new VideoFfmpeg (filename, x, y);

	return (Video *) video;
}

VideoFfmpeg::VideoFfmpeg (const char *filename, double x, double y) : Video (filename, x, y)
{
	video_codec = NULL;
	video_stream = NULL;
	audio_stream = NULL;
	video_rgb_buffer = NULL;
	video_cairo_surface = NULL;
	video_scale_context = NULL;
	av_format_context = NULL;
	
	w = h = 0;
	timeout_handle = 0;
	initial_pts = 0;
	play_start_time = 0;
	frame_size = 0;
	micro_to_pts = 0;
	video_frames_size = 0;

	pipe (pipes);
	fcntl (pipes [0], F_SETFL, O_NONBLOCK);
	
	video_mutex = g_mutex_new ();
	video_frames = g_async_queue_new ();
	pipe_channel = g_io_channel_unix_new (pipes [0]);
	g_io_add_watch (pipe_channel, G_IO_IN, video_ready, this);

	decode_thread_id = g_thread_create (decoder, this, TRUE, NULL);
}

Point
VideoFfmpeg::getxformorigin ()
{
	return Point (x + user_xform_origin.x * w, y + user_xform_origin.y * h);
}

VideoFfmpeg::~VideoFfmpeg ()
{
	// TODO:
	//    * Ask our thread to shutdown nicely.
	//
	fprintf (stderr, "We should stop the thread");

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
video_new (const char *filename, double x, double y)
{
	return video_ffmpeg_new (filename, x, y);
}

