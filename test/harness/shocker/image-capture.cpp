/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 */

#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <Magick++.h>
#include <list>
#include <sys/time.h>

#include "shocker.h"
#include "shutdown-manager.h"
#include "string.h"
#include "errno.h"

static gint64
get_now (void)
{
	struct timeval tv;
	gint64 res;
#ifdef CLOCK_MONOTONIC
	struct timespec tspec;
	if (clock_gettime (CLOCK_MONOTONIC, &tspec) == 0) {
		res = ((gint64)tspec.tv_sec * 10000000 + tspec.tv_nsec / 100);
		return res;
	}
#endif

	if (gettimeofday (&tv, NULL) == 0) {
		res = (tv.tv_sec * 1000000 + tv.tv_usec) * 10;
		return res;
	}

	// XXX error
	return 0;
}

ImageCaptureProvider::ImageCaptureProvider ()
{
}

ImageCaptureProvider::~ImageCaptureProvider ()
{
}

static char *moonlight_harness_output_dir = NULL;

void
ImageCaptureProvider::CaptureSingleImage (const char* image_dir, const char* file_name, int x, int y, int width, int height)
{
	char *image_path;
#ifdef SHOCKER_DEBUG
	printf ("CaptureSingleImage (%s, %s, %d, %d, %d, %d)\n", image_dir, file_name, x, y, width, height);
#endif

	if (!(image_dir == NULL || image_dir [0] == 0))
		printf ("[Shocker]: CaptureSingleImage ('%s', '%s', %d, %d, %d, %d): Should not be called with an image dir.\n", image_dir, file_name, x, y, width, height);
	
	// get the directory where to put the images.
	if (moonlight_harness_output_dir == NULL) {
		moonlight_harness_output_dir = getenv ("MOONLIGHT_HARNESS_OUTPUT_DIR");
		if (moonlight_harness_output_dir == NULL || moonlight_harness_output_dir [0] == 0)
			moonlight_harness_output_dir = g_get_current_dir ();
	}
		
	image_path = g_build_filename (moonlight_harness_output_dir, file_name, NULL);
	
	ScreenCaptureData sc (x, y, width, height);
	sc.Capture (image_path);
	g_free (image_path);
}

typedef struct capture_multiple_images_data {
	char* image_path;
	int x;
	int y;
	int width;
	int height;
	int count;
	int interval;
	int initial_delay;
} capture_multiple_images_data_t;

void
capture_multiple_images_data_free (capture_multiple_images_data_t* data)
{
	g_free (data->image_path);
	delete data;
}

void*
capture_multiple_images (void* data)
{
	capture_multiple_images_data_t* cmid = (capture_multiple_images_data_t *) data;
	std::list<Magick::Image> image_list;
	pid_t pid = getpid ();
	gint64 start, previous, current, elapsed, next;
	const gint64 ticks_in_ms = 10000;
	const gchar *tmp_dir = g_get_tmp_dir ();
	gchar *pid_dir = g_strdup_printf ("%i", pid);
	gchar *image_dir = g_build_filename (tmp_dir, "moonlight-test-harness", pid_dir, NULL);
	gchar **image_paths = (gchar **) g_malloc0 (sizeof (gchar *) * (cmid->count + 1));
	gchar **image_files = (gchar **) g_malloc0 (sizeof (gchar *) * (cmid->count + 1));
	
	// printf ("Moonlight harness: Capture %i screenshots, initial delay: %i ms, interval: %i ms, width: %i, height: %i\n", cmid->count, cmid->initial_delay, cmid->interval, cmid->width, cmid->height);
	
	usleep (cmid->initial_delay * 1000);

	g_mkdir_with_parents (image_dir, 0700);

	ScreenCaptureData sc (cmid->x, cmid->y, cmid->width, cmid->height);
	
	start = get_now () / ticks_in_ms;
	for (int i = 0; i < cmid->count; i++) {
		image_files [i] = g_strdup_printf ("multilayered-image-%03i.png", i);
		image_paths [i] = g_build_filename (image_dir, image_files [i], NULL);

		// printf (" Capturing screenshot #%2i into %s", i + 1, image_paths [i]);
		previous = get_now () / ticks_in_ms;
		
		sc.Capture (image_paths [i]);
		
		current = get_now () / ticks_in_ms;
		elapsed = current - start;
		next = (start + cmid->interval * (i + 1)) - current;
		if (next <= 0) {
			next = cmid->interval;
			if (current - previous > cmid->interval)
				printf ("\nMoonlight harness: Screen capture can't capture fast enough. Interval %lld ms, time spent taking screenshot: %lld ms\n", (gint64) cmid->interval, (gint64) current - previous);
		}
		
		//printf (" Done in %4llu ms, elapsed: %4lld ms, sleeping %4lld ms\n", current - previous, elapsed, next);
		
		usleep (next * 1000);
	}
	try {
		for (int i = 0; i < cmid->count; i++) {
			Magick::Image image;
			image.read (image_paths [i]);
			image_list.push_front (image);
		}
		Magick::writeImages (image_list.begin (), image_list.end (), cmid->image_path);
	} catch (Magick::Exception & error) {
		// Don't crash if imagemagick freaks out
		printf ("Moonlight harness: ImageMagick threw an exception: %s\n", error.what ());
	}
	// Cleanup after us
	for (int i = 0; i < cmid->count; i++) {
		unlink (image_paths [i]);
	}
	rmdir (image_dir);	
	g_strfreev (image_paths);
	g_strfreev (image_files);
	g_free (pid_dir);
	g_free (image_dir);
	
	capture_multiple_images_data_free (cmid);
	shutdown_manager_wait_decrement ();

	// printf ("capture_multiple_images [Done]\n");

	return NULL;
}


void
ImageCaptureProvider::CaptureMultipleImages (const char* file_name, int x, int y, int width, int height, int count, int interval, int initial_delay)
{
#ifdef SHOCKER_DEBUG
	printf ("CaptureMultipleImages (%s, %d, %d, %d, %d, %d, %d, %d)\n", file_name, x, y, width, height, count, interval, initial_delay);
#endif
	capture_multiple_images_data_t* cmid = new capture_multiple_images_data ();

	// get the directory where to put the images.
	if (moonlight_harness_output_dir == NULL) {
		moonlight_harness_output_dir = getenv ("MOONLIGHT_HARNESS_OUTPUT_DIR");
		if (moonlight_harness_output_dir == NULL || moonlight_harness_output_dir [0] == 0)
			moonlight_harness_output_dir = g_get_current_dir ();
	}

	cmid->image_path = g_build_filename (moonlight_harness_output_dir, file_name, NULL);
	if (!g_str_has_suffix (cmid->image_path, ".tif")) {
		char *tmp = cmid->image_path;
		cmid->image_path = g_strdup_printf ("%s.tif", cmid->image_path);
		g_free (tmp);
	}
	cmid->x = MAX (0, x);
	cmid->y = MAX (0, y);
	cmid->width = width;
	cmid->height = height;
	cmid->count = count;
	cmid->interval = interval;
	cmid->initial_delay = initial_delay;

	GThread* worker;
	GError* error;

	shutdown_manager_wait_increment ();
	worker = g_thread_create ((GThreadFunc) capture_multiple_images, cmid, FALSE, &error);
	if (!worker) {
		g_warning ("Unable to create thread for CaptureMultipleImages: %s\n", error->message);
		g_error_free (error);
		shutdown_manager_wait_decrement ();
		return;
	}
}

// TODO: Figure out defaults, maybe width/height need to be calculated?
AutoCapture::AutoCapture () : capture_interval (1000), max_images_to_capture (1), initial_delay (0), capture_x (0), capture_y (0), capture_width (640), capture_height (480)
{
}

void
AutoCapture::Run (const char* test_path, ImageCaptureProvider* provider)
{
	provider->CaptureMultipleImages (test_path, capture_x, capture_y, capture_width, capture_height, max_images_to_capture, capture_interval, initial_delay);
}


/*
 * ScreenCaptureData
 */

ScreenCaptureData::ScreenCaptureData (int x, int y, unsigned int width, unsigned int height)
{
	Window dummy = NULL;
	
	display = XOpenDisplay (NULL);
	screen = XDefaultScreen (display);
	root_window = XRootWindow (display, screen);
	XGetGeometry (display, root_window, &dummy, &root_x, &root_y, &root_width, &root_height, &root_border_width, &root_depth);

	this->x = MAX (x, root_x);
	this->y = MAX (y, root_x);
	this->width = MIN (width, root_width - this->x);
	this->height = MIN (height, root_height - this->y);

	if (this->x != x)
		printf ("Moonlight harness: Screen capture geometry has been modified (requested x: %i, actual x: %i)\n", x, this->x);
	if (this->y != y)
		printf ("Moonlight harness: Screen capture geometry has been modified (requested y: %i, actual y: %i)\n", y, this->y);
	if (this->width != width)
		printf ("Moonlight harness: Screen capture geometry has been modified (requested width: %u, actual width: %u)\n", width, this->width);
	if (this->height != height)
		printf ("Moonlight harness: Screen capture geometry has been modified (requested height: %u, actual height: %u)\n", height, this->height);
}

ScreenCaptureData::~ScreenCaptureData ()
{
	XCloseDisplay (display);
}

bool
ScreenCaptureData::Capture (const char *filename)
{
	XImage *image;
	int row_stride;
	int offset;
	int red_shift = 0, green_shift = 0, blue_shift = 0;
	guint32 pixel;
	GdkPixbuf *buffer;
	GError* error = NULL;
	
	image = XGetImage (display, root_window, x, y, width, height, AllPlanes, ZPixmap);

	if (image == NULL)
		return false;
	
	// Gdk only supports 24 bits RGB
	// The XImage is (at least on my machine) 32 bits RGB.
	row_stride = image->bytes_per_line;
	
	while (((image->red_mask >> red_shift) & 1) == 0) {
		red_shift++;
	}
	while (((image->green_mask >> green_shift) & 1) == 0) {
		green_shift++;
	}
	while (((image->blue_mask >> blue_shift) & 1) == 0) {
		blue_shift++;
	}
		
	for (int r = 0; r < image->height; r++) {
		for (int c = 0; c < image->width; c++) {
			offset = row_stride * r + c * 4;
			pixel = *(guint32 *) (image->data + offset);
			image->data [offset - c + 0] = (pixel & image->red_mask) >> red_shift;
			image->data [offset - c + 1] = (pixel & image->green_mask) >> green_shift;
			image->data [offset - c + 2] = (pixel & image->blue_mask) >> blue_shift;
		}
	}

	// Create a new pixbuf from our converted data
	buffer = gdk_pixbuf_new_from_data ((const guchar *) image->data, GDK_COLORSPACE_RGB, false, 8, image->width, image->height, row_stride, NULL, NULL);

	// Save to file
	gdk_pixbuf_save (buffer, filename, "png", &error, "tEXt::CREATOR", "moonlight-test-harness", NULL);

	gdk_pixbuf_unref (buffer);

	XDestroyImage (image);

	return true;
}

void shocker_capture_image (const char *filename, int x, int y, int width, int height)
{
	g_type_init (); // when invoked from managed code we need this.
	ScreenCaptureData sc (x, y, width, height);
	sc.Capture (filename);
}

void
CaptureSingleImage (const char *directory, const char *filename, int x, int y, int width, int height, int delay)
{
	if (directory != NULL && directory [0] != 0)
		printf ("[shocker] CaptureSingleImage: a directory was specified, it will not be respected.\n");
	if (delay != 0)
		printf ("[shocker] CaptureSingleImage: a non-zero delay was specified, it will not be respected.\n"); // FIXME: implement delay
	
	shocker_capture_image (filename, x, y, width, height);
}
