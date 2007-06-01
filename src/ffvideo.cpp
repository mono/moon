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

class VideoFfmpeg : public Video {
public:
	int w, h;

	VideoFfmpeg (const char *filename, double x, double y);
	~VideoFfmpeg ();

	// Virtual method overrides
	void getbounds ();
	void render (Surface *s, double *affine, int x, int y, int width, int height);


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
	AVStream    *audio_stream;
	int          audio_stream_idx;

	snd_pcm_t   *pcm;

	GAsyncQueue *video_frames;

	//
	// Timing components, the next time to display.
	//
	double   micro_to_pts;          // how to convert microseconds to pts (pts units are in video_stream->time_base units)
	uint64_t play_start_time;   	// Playback starts at this host time.
	uint64_t initial_pts;		// The PTS of the first frame when we started playing
	int      frame_size;            // microseconds between frames.
	guint    timeout_handle;

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
	double *affine = item_affine_get_absolute (this, res);
	AVCodecContext *cc;
	
	Surface *s = item_surface_get (this);
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
	if (affine != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) affine);

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
	
	item_update_bounds ((Item *) video);
	video->w = video->video_stream->codec->width;
	video->h = video->video_stream->codec->height;
	
	// Track where we are at now
	video->initial_pts = video->video_stream->start_time;
	video->micro_to_pts = 0.001;

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
			video->audio_stream_idx = i;
			video->audio_stream = video->av_format_context->streams [i];
			audio_stream_idx = video->audio_stream_idx;
			cc = video->audio_stream->codec;
			n = snd_pcm_open (&video->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
			if (n < 0)
				printf ("Error, can not initialize audio\n");				
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

	while (TRUE){
		AVPacket packet, *pkt = &packet;

		int ret;
		
		ret = av_read_frame (video->av_format_context, pkt);
		if (ret == -1){
			printf ("Failed to read frame, terminating\n");
			break;
		}
		if (pkt->stream_index == audio_stream_idx){
			//snd_pcm_writei (video->pcm, 
		}

		if (pkt->stream_index == video_stream_idx){
			int got_picture, n;
			static int count;
			
			video_frame = avcodec_alloc_frame ();

			while (g_async_queue_length (video->video_frames) > 10)
				g_usleep (1000);

			//printf ("pkt=%7.2f %8d\r\n", (double) pkt->pts, count++);
			//fflush (stdout);
			avcodec_decode_video (video->video_stream->codec, video_frame, &got_picture, pkt->data, pkt->size);

			if (got_picture){
				// Copy the PTS here, dont know why decode video does not set it
				if (video_frame->pts == 0)
					video_frame->pts = pkt->pts;

				//printf ("Adding PTS=%ld\n", video_frame->pts);
				g_mutex_lock (video->video_mutex);
				g_async_queue_push (video->video_frames, video_frame);
				g_mutex_unlock (video->video_mutex);

				send_command (video, CMD_NEWFRAME);
			}
		}
		av_free_packet (pkt);
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
	AVPicture pict;
	uint8_t *rgb_dest[3] = { video->video_rgb_buffer, NULL, NULL};
	int rgb_stride [3] = { cc->width * 4, 0, 0 };

	sws_scale (video->video_scale_context, frame->data, frame->linesize,  0,
		   video->video_stream->codec->height,  rgb_dest, rgb_stride);
}

static gboolean
load_next_frame (gpointer data)
{
	VideoFfmpeg *video = (VideoFfmpeg *) data;
	uint64_t target_pts = (uint64_t) ((av_gettime () - video->play_start_time) * video->micro_to_pts) + video->initial_pts;
	gboolean cont = TRUE;
	AVFrame *frame = NULL;

	g_mutex_lock (video->video_mutex);

	while (g_async_queue_length (video->video_frames) != 0){
		void *rgb;
		uint64_t diff;

		frame = (AVFrame *) g_async_queue_pop (video->video_frames);

		if (frame->pts + video->frame_size < target_pts){
			
			// We got more stuff on the queue, discard this one
			if (g_async_queue_length (video->video_frames) > 1){
				printf ("Initial PTS=%ld\n", video->initial_pts);
				printf ("Video Play Start: %ld\n", video->play_start_time);
				printf ("Current time: %ld\n", av_gettime ());
				printf ("micro to pts: %g\n", video->micro_to_pts);
				printf ("Frame PTS=%ld\n", frame->pts);
				printf ("Target PTS=%ld\n", target_pts);
				printf ("frame size=%d\n", video->frame_size);
				printf ("discarding frame at time=%g seconds because it is late\n", 
					(frame->pts - video->video_stream->start_time) *
					av_q2d (video->video_stream->time_base));
				av_free (frame);
			} else {
				printf ("Going to use the frame, and stopping stop\n");

				// We do not have anything else, render this last frame, and
				// stop the timer.
				cont = FALSE;
				video->timeout_handle = 0;
				break;
			}
		} else {
			// Got a frame in the proper time, break the loop, and continue.
			break;
		}
			
	}
	g_mutex_unlock (video->video_mutex);

	if (frame != NULL){
		convert_to_rgb (video, frame);
		av_free (frame);
	}

	item_invalidate ((Item *) video);

	//gdk_window_process_updates (get_surface (video)->drawing_area->window, FALSE);
	
	return cont;
}

static void
restart_timer (VideoFfmpeg *video)
{
	//video->frame_size = (int) (1000 * av_q2d (video->video_stream->time_base) * av_q2d (video->video_stream->r_frame_rate));

	// microseconds it takes to advance to the next frame
	video->frame_size = 1000 / av_q2d (video->video_stream->r_frame_rate);

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
			if (video->timeout_handle == 0)
				restart_timer (video);
			break;
			
		default:
			fprintf (stderr, "video_ready: unknown command from decoding thread (%d)\n", commands [i]);
		}
	}

	return TRUE;
}

void
VideoFfmpeg::render (Surface *s, double *affine, int x, int y, int width, int height)
{
	double actual [6];
	double *use_affine = item_get_affine (affine, xform, actual);
	cairo_pattern_t *pattern, *old_pattern;

	// If we are not initialized yet, return.
	if (video_cairo_surface == NULL)
		return;

	cairo_save (s->cairo);
	if (use_affine != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) use_affine);

	cairo_translate (s->cairo, this->x, this->y);
	cairo_set_source_surface (s->cairo, video_cairo_surface, 0, 0);
	cairo_paint (s->cairo);

	cairo_restore (s->cairo);
}

static int ffmpeg_inited;
static snd_pcm_t *pcm;

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
	video_rgb_buffer = NULL;
	video_cairo_surface = NULL;
	video_scale_context = NULL;
	av_format_context = NULL;
	
	timeout_handle = 0;
	initial_pts = 0;
	play_start_time = 0;
	frame_size = 0;
	micro_to_pts = 0;

	pipe (pipes);
	fcntl (pipes [0], F_SETFL, O_NONBLOCK);
	
	video_mutex = g_mutex_new ();
	video_frames = g_async_queue_new ();
	pipe_channel = g_io_channel_unix_new (pipes [0]);
	g_io_add_watch (pipe_channel, G_IO_IN, video_ready, this);

	decode_thread_id = g_thread_create (decoder, this, TRUE, NULL);
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

