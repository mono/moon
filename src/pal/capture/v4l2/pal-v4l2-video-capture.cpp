/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-video-capture-v4l2.h:
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "pal/capture/v4l2/pal-v4l2-video-capture.h"

MoonVideoCaptureServiceV4L2::MoonVideoCaptureServiceV4L2 ()
{
	default_device = NULL;
	devices = NULL;
}

MoonVideoCaptureServiceV4L2::~MoonVideoCaptureServiceV4L2 ()
{
	g_ptr_array_free (devices, true);
}

MoonVideoCaptureDevice*
MoonVideoCaptureServiceV4L2::GetDefaultCaptureDevice ()
{
	return default_device;
}

MoonVideoCaptureDevice**
MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (int *num_devices)
{
	if (devices) {
		*num_devices = devices->len;
		return (MoonVideoCaptureDevice**)devices->pdata;
	}

	devices = g_ptr_array_new ();

	*num_devices = 0;

	GDir *dir = g_dir_open ("/dev", 0, NULL);
	if (!dir) { 
		// should hopefully never happen.  what is this, !linux?
		*num_devices = devices->len;
		return (MoonVideoCaptureDevice**)devices->pdata;
	}

	const char *entry_name;
	while ((entry_name = g_dir_read_name (dir))) {
		if (!strncmp (entry_name, "video", 5)) {
			char name [PATH_MAX];
			if (g_snprintf (name, PATH_MAX, "/dev/%s", entry_name) > PATH_MAX)
				continue;

			// found a video device, make sure it works.
			int fd = open (name, O_RDWR | O_NONBLOCK);
			if (fd == -1) {
				switch (errno) {
					// from the v4l2 spec:

				case EACCES: // The caller has no permission to access the device.
				case EBUSY: // The driver does not support multiple opens and the device is already in use.
				case ENXIO: // No device corresponding to this device special file exists.
				case ENOMEM: // Not enough kernel memory was available to complete the request.
				case EMFILE: // The process already has the maximum number of files open.
				case ENFILE: // The limit on the total number of files open on the system has been reached.

					// FIXME do we need to do something here for all these error conditions?
					continue;
				}
			}

			MoonVideoCaptureDeviceV4L2 *device = new MoonVideoCaptureDeviceV4L2 (this, fd);

			g_ptr_array_add (devices, device);

			if (!strcmp (entry_name, "video")) {
				// if it's /dev/video treat it like the
				// default device it probing it works
				default_device = device;
			}
			else if (!strcmp (entry_name, "video0")
				 && !default_device) {
				// if it's /dev/video0 we also treat
				// it like the default device, but
				// only if there wasn't a viable
				// /dev/video.
				default_device = device;
			}
		}
	}

	*num_devices = devices->len;
	return (MoonVideoCaptureDevice**)devices->pdata;
}

bool
MoonVideoCaptureServiceV4L2::IsDefaultDevice (MoonVideoCaptureDeviceV4L2* device)
{
	return device == default_device;
}

MoonVideoCaptureDeviceV4L2::MoonVideoCaptureDeviceV4L2 (MoonVideoCaptureServiceV4L2* service, int fd)
{
	this->service = service;
	this->fd = fd;
	this->desired_format = NULL;
	this->friendly_name = NULL;
	this->capturing_format = NULL;
	this->need_to_notify_format = true;
	this->idle_id = -1;
}

MoonVideoCaptureDeviceV4L2::~MoonVideoCaptureDeviceV4L2 ()
{
	int j = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (fd, VIDIOC_STREAMOFF, &j)) {
		perror ("VIDIOC_STREAMOFF");
	}
	close (fd);
	delete desired_format;
	g_free (friendly_name);
	delete capturing_format;
	if (idle_id != -1)
		g_source_remove (idle_id);
}

MoonVideoFormat*
MoonVideoCaptureDeviceV4L2::GetDesiredFormat ()
{
	return desired_format;
}

void
MoonVideoCaptureDeviceV4L2::SetDesiredFormat (MoonVideoFormat *format)
{
	delete desired_format;
	desired_format = NULL;
	if (format) {
		// XXX we need to validate the format
		desired_format = new MoonVideoFormat (*format);
	}
}

MoonVideoFormat**
MoonVideoCaptureDeviceV4L2::GetSupportedFormats (int* count)
{
	*count = 0;
	return NULL;
}

const char*
MoonVideoCaptureDeviceV4L2::GetFriendlyName()
{
	if (!friendly_name) {
		struct v4l2_input input;

		memset (&input, 0, sizeof (input));

		if (-1 == ioctl (fd, VIDIOC_G_INPUT, &input.index)) {
			perror ("VIDIOC_G_INPUT");
			return NULL;
		}

		if (-1 == ioctl (fd, VIDIOC_ENUMINPUT, &input)) {
			perror ("VIDIOC_ENUM_INPUT");
			return NULL;
		}

		friendly_name = g_strdup ((char*)input.name);
	}

	return friendly_name;
}

bool
MoonVideoCaptureDeviceV4L2::GetIsDefaultDevice()
{
	return service->IsDefaultDevice (this);
}

static inline void YUV444ToBGRA(guint8 Y, guint8 U, guint8 V, guint8 *dst)
{
	dst[2] = CLAMP((298 * (Y - 16) + 409 * (V - 128) + 128) >> 8, 0, 255);
	dst[1] = CLAMP((298 * (Y - 16) - 100 * (U - 128) - 208 * (V - 128) + 128) >> 8, 0, 255);
	dst[0] = CLAMP((298 * (Y - 16) + 516 * (U - 128) + 128) >> 8, 0, 255);
	dst[3] = 0xFF;
}

void
MoonVideoCaptureDeviceV4L2::ReadNextFrame (guint8 **buffer, guint32 *buflen, gint64 *pts)
{
	if (need_to_notify_format && capturing_format) {
		format_changed (capturing_format, callback_data);
		need_to_notify_format = false;
	}

	struct v4l2_buffer v4l2buf;

	// block here until the hardware is actually ready to start feeding us frames
	struct pollfd fds[1];

	fds[0].fd = fd;
	fds[0].events = POLLIN | POLLERR;
	fds[0].revents = 0;

	printf ("POLL>>>>>>>>>>>>>>>>>\n");
	if (-1 == poll(fds, 1, 2000)) {
		perror ("poll");
		return;
	}
	printf ("POLL<<<<<<<<<<<<<<<<<\n");

	if (fds[0].revents == POLLERR) {
		printf ("boooo\n");
		return;
	}

	memset (&v4l2buf, 0, sizeof (v4l2buf));

	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = V4L2_MEMORY_MMAP;


	if (-1 == ioctl (fd, VIDIOC_DQBUF, &v4l2buf)) {
		if (errno != EAGAIN) {
			perror ("VIDIOC_DQBUF");
			return;
		}
	}

	printf ("******************\n");
	printf ("got a frame, length = %d, bytesused = %d\n", v4l2buf.length, v4l2buf.bytesused);

#if COPY_BUFFER
	*buffer = (guint8*)g_malloc (buffers[v4l2buf.index].length);

	memcpy (*buffer, buffers[v4l2buf.index].start, buffers[v4l2buf.index].length);

	*buflen = buffers[v4l2buf.index].length;
#else
	guint8 *output_buffer = (guint8*)g_malloc (buffers[v4l2buf.index].length * 2);
	guint8 *op = output_buffer;
	guint32 *ip = (guint32*)buffers[v4l2buf.index].start;

	while (((char*)ip - (char*)buffers[v4l2buf.index].start) < buffers[v4l2buf.index].length) {
		guint8 y  =  ((*ip & 0x000000ff));
		guint8 u =  ((*ip & 0x0000ff00)>>8);
		guint8 y2  =  ((*ip & 0x00ff0000)>>16);
		guint8 v  =  ((*ip & 0xff000000)>>24);

		YUV444ToBGRA (y, u, v, op); op += 4;
		YUV444ToBGRA (y2, u, v, op); op += 4;

		ip ++;
	}

	*buffer = output_buffer;
	*buflen = buffers[v4l2buf.index].length * 2;
#endif

	gint64 new_pts = v4l2buf.timestamp.tv_usec * 10ULL + v4l2buf.timestamp.tv_sec * 10000000ULL;

	if (first_pts == G_MAXUINT64)
		first_pts = new_pts;
	*pts = new_pts - first_pts;
	//      *pts = MilliSeconds_ToPts (v4l2buf.timestamp.tv_usec / 1000 + v4l2buf.timestamp.tv_sec * 1000);

	int index = v4l2buf.index;
	memset (&v4l2buf, 0, sizeof (v4l2buf));

	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = V4L2_MEMORY_MMAP;
	v4l2buf.index = index;

       // we've copied the data, give the buffer back to the device
	if (-1 == ioctl (fd, VIDIOC_QBUF, &v4l2buf)) {
		perror ("VIDIOC_QBUF");
		return;
	}
}

void
MoonVideoCaptureDeviceV4L2::StartCapturing (MoonReportSampleFunc report_sample,
					    MoonFormatChangedFunc format_changed,
					    gpointer data)
{
	this->report_sample = report_sample;
	this->format_changed = format_changed;
	this->callback_data = data;

	printf ("MoonVideoCaptureDeviceV4L2::StartCapturing ()\n");

	struct v4l2_format fmt;

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	if (-1 == ioctl (fd, VIDIOC_S_FMT, &fmt)) {
		perror ("VIDIOC_S_FMT");
		return;
	}

	capturing_format = new MoonVideoFormat (MoonPixelFormatYUV420P,
						0                      /* fps */,
						fmt.fmt.pix.width * 4  /* stride */,
						fmt.fmt.pix.width      /* width */,
						fmt.fmt.pix.height     /* height */);

	struct v4l2_requestbuffers reqbuf;

	memset (&reqbuf, 0, sizeof (reqbuf));
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = VIDEO_MAX_FRAME;

	if (-1 == ioctl (fd, VIDIOC_REQBUFS, &reqbuf)) {
		if (errno == EAGAIN)
			printf ("Video capturing or mmap-streaming is not supported\n");

		return;
	}

	buffers = g_new0 (Buffer, reqbuf.count);

	for (unsigned int i = 0; i < reqbuf.count; i++) {
		struct v4l2_buffer buffer;

		memset (&buffer, 0, sizeof (buffer));
		buffer.type = reqbuf.type;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;

		if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buffer)) {
			perror ("VIDIOC_QUERYBUF");
			return;
		}

		buffers[i].length = buffer.length; /* remember for munmap() */

		buffers[i].start = mmap (NULL, buffer.length,
					 PROT_READ | PROT_WRITE,
					 MAP_SHARED,
					 fd, buffer.m.offset);

		if (buffer.m.offset % 16 == 0) {
			printf ("buffer %d is 16 byte aligned\n", i);
		}
		else if (buffer.m.offset % 8 == 0) {
			printf ("buffer %d is 8 byte aligned\n", i);
		}
		else if (buffer.m.offset % 4 == 0) {
			printf ("buffer %d is 4 byte aligned\n", i);
		}

		if (MAP_FAILED == buffers[i].start) {
			/* If you do not exit here you should unmap() and free()
			   the buffers mapped so far. */
			perror ("mmap");
			return;
		}

		if (-1 == ioctl (fd, VIDIOC_QBUF, &buffer)) {
			perror ("VIDIOC_QBUF");
			return;
		}
	}

	int j = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl (fd, VIDIOC_STREAMON, &j)) {
		perror ("VIDIOC_STREAMON");
		return;
	}

	idle_id = g_idle_add (ReadNextFrame, this);
}

gboolean
MoonVideoCaptureDeviceV4L2::ReadNextFrame (gpointer context)
{
	MoonVideoCaptureDeviceV4L2 *device = (MoonVideoCaptureDeviceV4L2*)context;
	guint8 *buffer;
	guint32 buflen;
	gint64 pts;

	device->ReadNextFrame (&buffer, &buflen, &pts);

	device->report_sample (pts, 0 /* FIXME */, buffer, buflen, device->callback_data);

	return TRUE;
}

void
MoonVideoCaptureDeviceV4L2::StopCapturing ()
{
	printf ("MoonVideoCaptureDeviceV4L2::StopCapturing ()\n");

	int j = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (fd, VIDIOC_STREAMOFF, &j)) {
		perror ("VIDIOC_STREAMOFF");
	}

	g_free (buffers);
	if (idle_id != -1)
		g_source_remove (idle_id);
	idle_id = -1;
}
