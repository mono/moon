



#ifndef __IMAGE_CAPTURE_H___
#define __IMAGE_CAPTURE_H__


#include "glib.h"

#define Visual _XVisual
#include <X11/X.h>
#include <X11/Xlib.h>

#include "gdk/gdk.h"
#include "gdk/gdkx.h"
#undef Visual


class ImageCaptureProvider {

public:
	ImageCaptureProvider ();
	virtual ~ImageCaptureProvider ();

	void CaptureSingleImage (const char* image_dir, const char* file_name, int x, int y, int width, int height);
	void CaptureMultipleImages (const char* test_path, int x, int y, int width, int height,
			int count, int capture_interval, int initial_delay);

private:
	Window xroot_window;
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

#endif  // __IMAGE_CAPTURE_H__

