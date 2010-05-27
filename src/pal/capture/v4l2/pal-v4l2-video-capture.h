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

class MoonVideoCaptureServiceV4L2;

class MoonVideoCaptureDeviceV4L2 : public MoonVideoCaptureDevice {
public:
	MoonVideoCaptureDeviceV4L2 (MoonVideoCaptureServiceV4L2* service, int fd);
	virtual ~MoonVideoCaptureDeviceV4L2 ();

	virtual MoonVideoFormat* GetDesiredFormat ();
	virtual void SetDesiredFormat (MoonVideoFormat *format);
	virtual MoonVideoFormat** GetSupportedFormats (int* count);

	virtual const char* GetFriendlyName();
	virtual bool GetIsDefaultDevice();

	virtual void StartCapturing (MoonReportSampleFunc report_sample,
				     MoonFormatChangedFunc format_changed,
				     gpointer data);
	virtual void StopCapturing ();

private:
	void ReadNextFrame (guint8 **buffer, guint32 *buflen, gint64 *pts);
	static gboolean ReadNextFrame (gpointer context);

	int fd;
	MoonVideoFormat *desired_format;
	MoonVideoFormat *capturing_format;
	bool need_to_notify_format;
	MoonVideoCaptureServiceV4L2* service;
	char *friendly_name;

	typedef struct {
		void *start;
		size_t length;
	} Buffer;

	Buffer *buffers;

	gint64 first_pts;

	MoonReportSampleFunc report_sample;
	MoonFormatChangedFunc format_changed;
	gpointer callback_data;

	gint idle_id;
};

class MoonVideoCaptureServiceV4L2 : public MoonVideoCaptureService {
public:
	MoonVideoCaptureServiceV4L2 ();
	virtual ~MoonVideoCaptureServiceV4L2 ();

	virtual MoonVideoCaptureDevice* GetDefaultCaptureDevice ();
	virtual MoonVideoCaptureDevice** GetAvailableCaptureDevices (int *num_devices);

	bool IsDefaultDevice (MoonVideoCaptureDeviceV4L2 *device);

private:
	GPtrArray *devices;
	MoonVideoCaptureDeviceV4L2* default_device;
};

#endif /* MOON_PAL_VIDEO_CAPTURE_V4L2_H */
