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

#include <stdio.h>
#include <unistd.h>
#include <Magick++.h>
#include <list>

#include "shocker.h"
#include "shutdown-manager.h"


static Magick::Image acquire_screenshot (Window window, int x, int y, int w, int h);

ImageCaptureProvider::ImageCaptureProvider ()
{
	Display* display = XOpenDisplay (NULL);
	xroot_window = XRootWindow (display, 0);
	XCloseDisplay (display);
}

ImageCaptureProvider::~ImageCaptureProvider ()
{
}


void
ImageCaptureProvider::CaptureSingleImage (const char* image_dir, const char* file_name, int x, int y, int width, int height)
{
#ifdef SHOCKER_DEBUG
	printf ("CaptureSingleImage (%s, %s, %d, %d, %d, %d)\n", image_dir, file_name, x, y, width, height);
#endif
	char* image_path = g_build_filename (image_dir, file_name, NULL);

	Magick::Image screenshot = acquire_screenshot (xroot_window, x, y, width, height);
	screenshot.write (image_path);

	/*
	GdkWindow* root = gdk_window_foreign_new (GDK_ROOT_WINDOW ());
	GdkPixbuf* buf = gdk_pixbuf_get_from_drawable (NULL, root, NULL, x, y, 0, 0, width, height);
	GError* error = NULL;

	gdk_pixbuf_save (buf, image_path, "png", &error, "tEXt::CREATOR", "moonlight-test-harness", NULL);
	*/

	g_free (image_path);
}

typedef struct capture_multiple_images_data {
	char* image_path;
	int xroot_window;
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

	usleep (cmid->initial_delay * 1000);

	std::list<Magick::Image> image_list;
	for (int i = 0; i < cmid->count; i++) {
		Magick::Image image = acquire_screenshot (cmid->xroot_window, cmid->x, cmid->y, cmid->width, cmid->height);
		image_list.push_front (image);

		usleep (cmid->interval * 1000);
	}

	Magick::writeImages (image_list.begin (), image_list.end (), cmid->image_path);

	capture_multiple_images_data_free (cmid);
	shutdown_manager_wait_decrement ();
	g_thread_exit (NULL);
	
	return NULL;
}


void
ImageCaptureProvider::CaptureMultipleImages (const char* test_path, int x, int y, int width, int height, int count, int interval, int initial_delay)
{
#ifdef SHOCKER_DEBUG
	printf ("CaptureMultipleImages (%s, %d, %d, %d, %d, %d, %d, %d)\n", test_path, x, y, width, height, count, interval, initial_delay);
#endif
	capture_multiple_images_data_t* cmid = new capture_multiple_images_data ();

	cmid->image_path = g_strdup_printf ("%s.tif", test_path);
	cmid->xroot_window = xroot_window;
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


static Magick::Image
acquire_screenshot (Window window, int x, int y, int width, int height)
{
	Magick::Image image;
	Magick::Geometry crop = Magick::Geometry (width, height, MAX (0, x), MAX (0, y));
	char* id = g_strdup_printf ("x:%d", (int) window);

	// Why won't this work for me?  This should make things fast, but it is still taking
	// ~2 seconds to take a screenshot
	// image.defineValue ("x", "screen", "true");

	image.depth (8);
	image.read (crop, id);
	image.matte (true);
	image.crop (crop);

	g_free (id);

	return image;
}


// TODO: Figure out defaults, maybe width/height need to be calculated?
AutoCapture::AutoCapture () : capture_interval (1000), max_images_to_capture (1), initial_delay (0), capture_width (640), capture_height (480)
{
}

void
AutoCapture::Run (const char* test_path, ImageCaptureProvider* provider)
{
	provider->CaptureMultipleImages (test_path, 0, 0, capture_width, capture_height, max_images_to_capture, capture_interval, initial_delay);
}


