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

#ifndef __IMAGE_CAPTURE_H___
#define __IMAGE_CAPTURE_H__

class AutoCapture;

#include "glib.h"

#define Visual _XVisual
#include <X11/X.h>
#include <X11/Xlib.h>

#include "gdk/gdk.h"
#include "gdk/gdkx.h"
#undef Visual

#include "plugin.h"

class ScreenCaptureData {
private:
	Display *display;
	int screen;
	Window root_window;
	signed int root_x, root_y;
	unsigned int root_width, root_height, root_depth, root_border_width;
	signed int x, y;
	unsigned int width, height;
	
public:
	ScreenCaptureData (int x, int y, unsigned int width, unsigned int height);
	~ScreenCaptureData ();
	
	bool Capture (const char *filename);
};

class ImageCaptureProvider {
public:
	ImageCaptureProvider ();
	virtual ~ImageCaptureProvider ();

	void CaptureSingleImage (const char* image_dir, const char* file_name, int x, int y, int width, int height);
	void CaptureMultipleImages (const char* test_path, int x, int y, int width, int height,
			int count, int capture_interval, int initial_delay);
};


class AutoCapture {

public:
	AutoCapture ();

	void Run (const char* test_path, ImageCaptureProvider* provider);

	void SetCaptureInterval (int capture_interval) { this->capture_interval = capture_interval; }
	void SetMaxImagesToCapture (int max_images_to_capture) { this->max_images_to_capture = max_images_to_capture; }
	void SetInitialDelay (int initial_delay) { this->initial_delay = initial_delay; }
	void SetCaptureX (int capture_x) { this->capture_x = capture_x; }
	void SetCaptureY (int capture_y) { this->capture_y = capture_y; }
	void SetCaptureWidth (int capture_width) { this->capture_width = capture_width; }
	void SetCaptureHeight (int capture_height) { this->capture_height = capture_height; }

private:
	int capture_interval;
	int max_images_to_capture;
	int initial_delay;
	int capture_x;
	int capture_y;
	int capture_width;
	int capture_height;

};

G_BEGIN_DECLS
void shocker_capture_image (const char *filename, int x, int y, int width, int height);
void CaptureSingleImage (const char *directory, const char *filename, int x, int y, int width, int height, int delay);
G_END_DECLS

#endif  // __IMAGE_CAPTURE_H__

