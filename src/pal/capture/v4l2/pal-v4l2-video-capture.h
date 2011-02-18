/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-video-capture-v4l2.h:
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_PAL_VIDEO_CAPTURE_V4L2_H
#define MOON_PAL_VIDEO_CAPTURE_V4L2_H

#include "pal.h"

namespace Moonlight {

class MoonVideoCaptureServiceV4L2;

struct MoonVideoFormatV4L2 {
public:
	guint32 v4l2PixelFormat;
	float framesPerSecond;
	int height;
	int width;
	int stride;
};

class MoonVideoCaptureDeviceV4L2 : public MoonVideoCaptureDevice {
public:
	MoonVideoCaptureDeviceV4L2 (int fd, const char *friendly_name);
	virtual ~MoonVideoCaptureDeviceV4L2 ();

	virtual void GetSupportedFormats (VideoFormatCollection *col);

	virtual const char* GetFriendlyName();

	virtual void StartCapturing ();
	virtual void StopCapturing ();

private:
	static void * CaptureLoopCallback (gpointer context);
	void CaptureLoop ();
	void RetrieveFormats ();

	int fd;
	MoonVideoFormatV4L2 capturing_format;
	char *friendly_name;

	typedef struct {
		void *start;
		size_t length;
	} Buffer;

	int buffer_count;
	Buffer *buffers;

	guint64 first_pts;

	guint32 format_count;
	MoonVideoFormatV4L2 **formats;

	int capture_pipe [2]; // this is the pipe we use to wake up the capture thread
	bool capturing; // write on main thread only
	pthread_t capture_thread;
};

class MoonVideoCaptureServiceV4L2 : public MoonVideoCaptureService {
public:
	MoonVideoCaptureServiceV4L2 () {}
	virtual ~MoonVideoCaptureServiceV4L2 () {}

	virtual void GetAvailableCaptureDevices (VideoCaptureDeviceCollection *col);
};

};
#endif /* MOON_PAL_VIDEO_CAPTURE_V4L2_H */
