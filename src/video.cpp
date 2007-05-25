#define __STDC_LIMIT_MACROS 1
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include "runtime.h"

// video specific 
#include <stdint.h>
#include <avcodec.h>
#include <avformat.h>

typedef enum  {
	VIDEO_OK,
	VIDEO_ERROR_OPEN,
	VIDEO_ERROR_STREAM_INFO
} VideoError;

struct _Video {
	Item   item;
	double x, y, w, h;
	char   *filename;

	int     error;

	AVFormatContext av_format_context;
};

static void
video_render (Item *item, Surface *s, double *affine, int x, int y, int width, int height)
{

}

static void
video_getbounds (Item *item)
{
	Video *video = (Video *) item;
	double res [6];
	double *affine = item_affine_get_absolute (item, res);

	Surface *s = item_surface_get (item);
	if (s == NULL){
		// not yet attached
		return;
	}

	cairo_save (s->cairo);
	if (affine != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) affine);

	cairo_rectangle (s->cairo, video->x, video->y, video->x + video->w, video->y + video->h);
	cairo_fill_extents (s->cairo, &item->x1, &item->y1, &item->x2, &item->y2);
	cairo_restore (s->cairo);
}

static ItemVtable video_vtable = { &video_render, &video_getbounds };

static int video_inited;

void
video_init (Video *video)
{
	Item *item = &video->item;
	item_init (item);
	item->vtable = &video_vtable;

	if (!video_inited){
		avcodec_init ();
		avcodec_register_all ();
		video_inited = TRUE;
	}
}

Item *
video_new (const char *filename, double x, double y, double w, double h)
{
	Video *video = g_new (Video, 1);
	AVFormatParameters *ap;

	video_init (video);

	video->filename = g_strdup (filename);
	video->x = x;
	video->y = y;
	video->w = w;
	video->h = h;

	// avcodec setup
	//   auto-detect format
	//   default buffer size
	//   no additional parameters
	if (av_open_input_file (&video->av_format_context, filename, NULL, 0, NULL) < 0){
		video->error = VIDEO_ERROR_OPEN;
		return (Item *) video;
	} 
	if (video->av_format_context == &rtsp_demuxer){
		fprintf (stderr, "ERROR: This is a RTSP stream, handle it");
		exit (1);
	}

	av_read_play (video->av_format_context);

        if (av_find_stream_info(video->av_format_context) < 0){
		video->error = VIDEO_ERROR_STREAM_INFO;
		return (Item *) video;
	}

	printf ("OK\n");
	return (Item *) video;
}

void 
video_destroy (Video *video)
{
	g_free (video->filename);
	g_free (video);
}


