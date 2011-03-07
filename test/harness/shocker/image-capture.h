/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * image-capture.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __IMAGE_CAPTURE_H__
#define __IMAGE_CAPTURE_H__

class AutoCapture;

#include "glib.h"

#define Visual _XVisual
#include <X11/X.h>
#include <X11/Xlib.h>

#include "gdk/gdk.h"
#include "gdk/gdkx.h"
#undef Visual

#include "shocker-plugin.h"

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
	ImageCaptureProvider () {}
	virtual ~ImageCaptureProvider () {}

	static void CaptureSingleImage (const char* image_dir, const char* file_name, int x, int y, int width, int height);
	static void CaptureMultipleImages (const char* test_path, int x, int y, int width, int height,
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

void ImageHelper_CaptureSingleImage (const char *directory, const char *filename, int x, int y, int width, int height, int delay);
int ImageHelper_CaptureMultipleImages (const char *directory, const char *filename, int x, int y, int width, int height, int count, int interval, int delay);
void CompareImages (const char *imageFile1, const char *imageFile2, guint8 tolerance, const char *diffFileName, bool copySourceFiles, guint8 *result);

G_END_DECLS

#endif  // __IMAGE_CAPTURE_H__

