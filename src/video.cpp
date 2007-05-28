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
#include <avformat.h>
#include <stdint.h>
#include <avcodec.h>
#include <swscale.h>

Video::Video (const char *filename, double x, double y)
{
	this->filename = g_strdup (filename);
	this->x = x;
	this->y = y;
}

Video::~Video ()
{
	g_free (filename);
}

typedef enum {
	// When a new frame is ready
	CMD_NEWFRAME,

	// When the video has been loaded, this updates the bounding box in the Gtk+ GUI thread.
	CMD_INITED
} GuiCommand;

class VideoFfmpeg : public Video {
public:
	int w, h;

	VideoFfmpeg (const char *filename, double x, double y);
	~VideoFfmpeg ();

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

	GAsyncQueue *video_frames;

	//
	// The components used to render the buffer
	//

	// The buffer where we store the RGB data
	unsigned char *video_rgb_buffer;

	// The cairo surface that represents video_rgb_buffer
	cairo_surface_t *video_cairo_surface;

	// The SwsContext image convertor
	struct SwsContext *video_scale_context;

	// Virtual method overrides
	void getbounds ();
	void render (Surface *s, double *affine, int x, int y, int width, int height);

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

	cairo_rectangle (s->cairo, x, y, x + cc->width, y + cc->height);
	cairo_fill_extents (s->cairo, &x1, &y1, &x2, &y2);

	printf ("New video bounds: %g %g %g %g\n", x1, y1, x2, y2);
	cairo_restore (s->cairo);
}

static void
send_command (VideoFfmpeg *video, uint8_t cmd)
{
	int ret;
	
	do {
		ret = write (video->pipes [1], &cmd, 1);
	} while (ret == -1 && errno == EINTR);
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
	int video_stream_idx;
	
	printf ("Video THREAD STARTS\n");

	g_mutex_lock (video_lock);
	// avcodec setup
	//   auto-detect format
	//   default buffer size
	//   no additional parameters
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

		switch (stream->codec->codec_type){
		case CODEC_TYPE_VIDEO:
			video->video_codec = avcodec_find_decoder (stream->codec->codec_id);
			video->video_stream = video->av_format_context->streams [i];
			avcodec_open (video->video_stream->codec, video->video_codec);
			video->video_stream_idx = i;
			video_stream_idx = i;

			//
			// Allocate our rendering buffer and cairo surface
			//
			cc = video->video_stream->codec;
			video->video_rgb_buffer = (unsigned char *) malloc (cc->width * cc->height * 4);

			video->video_cairo_surface = cairo_image_surface_create_for_data (
				video->video_rgb_buffer, CAIRO_FORMAT_ARGB32,
				cc->width, cc->height, cc->width * 4);

			// The YUB to RGB scaleter/translator
			video->video_scale_context = sws_getContext (cc->width, cc->height, cc->pix_fmt, cc->width, cc->height,
								     PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
			break;
		}
	}
	g_mutex_unlock (video_lock);

	send_command (video, CMD_INITED);

	while (TRUE){
		AVPacket packet, *pkt = &packet;

		int ret;
		
		ret = av_read_frame (video->av_format_context, pkt);
		if (ret == -1){
			printf ("Failed to read frame, terminating\n");
			break;
		}

		if (pkt->stream_index == video_stream_idx){
			int got_picture, n;

			printf ("Decoding frame for time %7.2f\r", pkt->pts);
			fflush (stdout);
			video_frame = avcodec_alloc_frame ();

			while (g_async_queue_length (video->video_frames) > 10)
				g_usleep (1000);

			printf ("pkt=%ld\r", pkt->pts);
			avcodec_decode_video (video->video_stream->codec, video_frame, &got_picture, pkt->data, pkt->size);
			
			if (got_picture){
				g_mutex_lock (video->video_mutex);
				send_command (video, CMD_NEWFRAME);
				g_async_queue_push (video->video_frames, video_frame);
				g_mutex_unlock (video->video_mutex);
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

	do {
		ret = read (video->pipes [0], &commands, sizeof (commands));
	} while ((ret == -1 && ret == EINTR));

	// Nothing.
	if (ret <= 0)
		return TRUE;
	
	for (i = 0; i < ret; i++){
		switch (commands [i]){
		case CMD_NEWFRAME:
			item_invalidate ((Item *) video);
			break;
			
		case CMD_INITED:
			item_update_bounds ((Item *) video);
			video->w = video->video_stream->codec->width;
			video->h = video->video_stream->codec->height;
			break;

		default:
			fprintf (stderr, "video_ready: unknown command from decoding thread (%d)\n", commands [i]);
		}
	}

	return TRUE;
}

static void
update_frame (VideoFfmpeg *video)
{
	AVFrame *frame = NULL;

	//
	// Experimental strategy: the strategy needs to be entirely redone
	// for now I just pull the last frame and discard all the other
	// intermediate frames that we already decoded.
	//
	//
	g_mutex_lock (video->video_mutex);
	while (g_async_queue_length (video->video_frames) != 0){
		void *rgb;

		if (frame != NULL)
			av_free (frame);
		
		frame = (AVFrame *) g_async_queue_pop (video->video_frames);
	}
	g_mutex_unlock (video->video_mutex);

	if (frame != NULL){
		convert_to_rgb (video, frame);
		av_free (frame);
	}
}

void
VideoFfmpeg::render (Surface *s, double *affine, int x, int y, int width, int height)
{
	double actual [6];
	double *use_affine = item_get_affine (affine, xform, actual);
	cairo_pattern_t *pattern, *old_pattern;

	update_frame (this);
	
	cairo_save (s->cairo);
	if (use_affine != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) use_affine);

	cairo_translate (s->cairo, this->x, this->y);
	cairo_set_source_surface (s->cairo, video_cairo_surface, 0, 0);
	cairo_paint (s->cairo);

	cairo_rectangle (s->cairo, x, y, w, h);

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
	video_rgb_buffer = NULL;
	video_cairo_surface = NULL;
	video_scale_context = NULL;
	av_format_context = NULL;
	
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

