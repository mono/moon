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
#include "utils.h"
#include <glib/gstdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
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
	if (devices) {
		for (guint i = 0; i < devices->len; i ++)
			delete ((MoonVideoCaptureDeviceV4L2*)devices->pdata[i]);
		g_ptr_array_free (devices, FALSE);
	}
}

MoonVideoCaptureDevice*
MoonVideoCaptureServiceV4L2::GetDefaultCaptureDevice ()
{
	int unused;

	// make sure we probe all the devices so that the default is
	// populated.
	GetAvailableCaptureDevices (&unused);

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
	char default_device_name [PATH_MAX];

	default_device_name[0] = 0;

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
				struct stat st;

				if (-1 == g_lstat (name, &st))
					continue;
				
				if (S_ISLNK (st.st_mode)) {
					if (-1 == readlink (name, default_device_name, PATH_MAX))
						continue;
				}
				else {
					// if it's /dev/video treat it like the
					// default device if probing it works
					default_device = device;
				}
			}
			else if (!strcmp (entry_name,
					  default_device_name)) {
				// we found the destination of the /dev/video symlink, make this our default device
				default_device = device;
			}
			else if (!strcmp (entry_name, "video0") &&
				 !default_device_name[0] &&
				 !default_device) {
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
	this->formats = NULL;
	this->first_pts = G_MAXINT64;
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
	if (formats) {
		for (guint i = 0; i < formats->len; i ++)
			delete ((MoonVideoFormat*)formats->pdata[i]);
		g_ptr_array_free (formats, FALSE);
	}
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
		desired_format = format->Clone ();
	}
}

static int
video_format_comparer (gconstpointer vf1, gconstpointer vf2)
{
	// we order formats based on ones we prefer, so that the
	// default is always at the 0 index.
	MoonVideoFormatV4L2* format1 = (*(MoonVideoFormatV4L2**)vf1);
	MoonVideoFormatV4L2* format2 = (*(MoonVideoFormatV4L2**)vf2);

#define FORMAT(fmt) G_STMT_START {					\
		if (format1->GetV4L2PixelFormat() == (fmt)) { \
			if (format1->GetV4L2PixelFormat () == format2->GetV4L2PixelFormat ()) \
				return format2->GetWidth() * format2->GetHeight() - format1->GetWidth() * format1->GetHeight(); \
			else						\
				return -1;				\
		}							\
		else if (format2->GetV4L2PixelFormat() == (fmt))	\
			return 1;					\
	} G_STMT_END

	// we prefer YUYV if we can get it
	FORMAT (V4L2_PIX_FMT_YUYV);
	// with YV12 a close second
	FORMAT (V4L2_PIX_FMT_YVU420);

	// for now we don't care enough about the rest
	return 0;
}

MoonVideoFormat**
MoonVideoCaptureDeviceV4L2::GetSupportedFormats (int* count)
{
	if (formats) {
		*count = formats->len;
		return (MoonVideoFormat**)formats->pdata;
	}

	formats = g_ptr_array_new ();

	struct v4l2_fmtdesc fmt;
	struct v4l2_frmsizeenum frame_size;
	struct v4l2_frmivalenum frame_interval;

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	for (fmt.index = 0; ; fmt.index ++) {
		if (-1 == ioctl (fd, VIDIOC_ENUM_FMT, &fmt)) {
			if (errno != EINVAL)
				perror ("VIDIOC_ENUM_FMT");
			break;
		}

		// we skip compressed formats since we don't want to
		// be decoding, e.g. mjpg
		if (fmt.flags & V4L2_FMT_FLAG_COMPRESSED)
			continue;

		frame_size.pixel_format = fmt.pixelformat;

		bool discrete_framesizes = true;
		for (frame_size.index = 0; ; frame_size.index ++) {
			guint32 frame_width;
			guint32 frame_height;
			guint32 frame_rate;

			if (-1 == ioctl (fd, VIDIOC_ENUM_FRAMESIZES, &frame_size)) {
				if (errno != EINVAL)
					perror ("VIDIOC_ENUM_FRAMESIZES");
				break;
			}

			// for now we treat stepwise sizes as their
			// maximum size, since there's no way to
			// communicate this to SL (FIXME: maybe we can
			// expand them to many different sizes and
			// represent them as separate formats?)
			if (frame_size.type == V4L2_FRMSIZE_TYPE_CONTINUOUS ||
			    frame_size.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
				frame_width = frame_size.stepwise.max_width;
				frame_height = frame_size.stepwise.max_height;
				discrete_framesizes = false;
			}
			else {
				frame_width = frame_size.discrete.width;
				frame_height = frame_size.discrete.height;
			}
			    
			frame_interval.pixel_format = fmt.pixelformat;
			frame_interval.width = frame_width;
			frame_interval.height = frame_height;

			bool discrete_frameintervals = true;
			for (frame_interval.index = 0; ; frame_interval.index ++) {
				if (-1 == ioctl (fd, VIDIOC_ENUM_FRAMEINTERVALS, &frame_interval)) {
					if (errno != EINVAL)
						perror ("VIDIOC_ENUM_FRAMEINTERVALS");
					break;
				}

				if (frame_interval.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
					frame_rate = frame_interval.stepwise.max.denominator / frame_interval.stepwise.max.numerator;
					discrete_frameintervals = false;
				}
				else {
					frame_rate = frame_interval.discrete.denominator / frame_interval.discrete.numerator;;
				}

				g_ptr_array_insert_sorted (formats,
							   video_format_comparer,
							   new MoonVideoFormatV4L2 (MoonPixelFormatRGBA32,
										    frame_rate,
										    frame_width * 4,
										    frame_width,
										    frame_height,
										    fmt.pixelformat,
										    0 /* no way to get this here */));

				printf ("Format: %c%c%c%c ",
					(char)(fmt.pixelformat & 0xff),
					(char)((fmt.pixelformat >> 8) & 0xff),
					(char)((fmt.pixelformat >> 16) & 0xff),
					(char)((fmt.pixelformat >> 24) & 0xff));
				printf ("  frame size: %d x %d, frame rate = %d\n", frame_width, frame_height, frame_rate);

				if (!discrete_frameintervals)
					break;
			}

			if (!discrete_framesizes) {
				// according to the v4l2 docs:
				//
				// if the first element is DISCRETE then
				// 1) there can be more than one
				// 2) they'll all be discrete
				//
				// if the first element is not
				// DISCRETE, then there will be only
				// one
				break;
			}
		}
	}

	*count = formats->len;
	return (MoonVideoFormat**)formats->pdata;
}

const char*
MoonVideoCaptureDeviceV4L2::GetFriendlyName()
{
	if (!friendly_name) {
		struct v4l2_capability cap;

		memset (&cap, 0, sizeof (cap));

		if (-1 == ioctl (fd, VIDIOC_QUERYCAP, &cap)) {
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
		else {
			friendly_name = g_strdup_printf ("%s (%s)", cap.card, cap.driver);
		}
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

	// printf ("POLL>>>>>>>>>>>>>>>>>\n");
	if (-1 == poll(fds, 1, 2000)) {
		perror ("poll");
		return;
	}
	// printf ("POLL<<<<<<<<<<<<<<<<<\n");

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

	// printf ("******************\n");
	// printf ("got a frame, length = %d, bytesused = %d\n", v4l2buf.length, v4l2buf.bytesused);

#if COPY_BUFFER
	*buffer = (guint8*)g_malloc (buffers[v4l2buf.index].length);

	memcpy (*buffer, buffers[v4l2buf.index].start, buffers[v4l2buf.index].length);

	*buflen = buffers[v4l2buf.index].length;
#else
	guint8 *output_buffer = (guint8*)g_malloc (buffers[v4l2buf.index].length * 2);
	guint8 *op = output_buffer;
	guint32 *ip = (guint32*)buffers[v4l2buf.index].start;

	guint32 pixelformat = capturing_format->GetV4L2PixelFormat ();

	if (pixelformat == V4L2_PIX_FMT_YUYV) {
		while ((size_t) ((char*)ip - (char*)buffers[v4l2buf.index].start) < buffers[v4l2buf.index].length) {
			guint8 y, u, y2, v;

			y  = ((*ip & 0x000000ff));
			y2 = ((*ip & 0x00ff0000)>>16);
			u =  ((*ip & 0x0000ff00)>>8);
			v =  ((*ip & 0xff000000)>>24);

			YUV444ToBGRA (y, u, v, op); op += 4;
			YUV444ToBGRA (y2, u, v, op); op += 4;
			
			ip++;
		}
	} else if (pixelformat == (V4L2_PIX_FMT_YUV420) || pixelformat == (V4L2_PIX_FMT_YVU420)) {
		int width = capturing_format->GetWidth ();
		int height = capturing_format->GetHeight ();		
		bool u_first = (pixelformat == V4L2_PIX_FMT_YUV420);
		guint8 *yp = (guint8 *)buffers[v4l2buf.index].start;
		guint8 *up = (guint8 *)buffers[v4l2buf.index].start + width * height + (u_first ? 0 : width * height / 4);
		guint8 *vp = (guint8 *)buffers[v4l2buf.index].start + width * height + (u_first ? width * height / 4 : 0);
		int i = 0, j = 0;

		while (i * j < width * height) { 
			guint8 y  =  yp[j * width + i];
			guint8 u =  up[j/2 * width/2 + i/2];
			guint8 v  =  vp[j/2 * width/2 + i/2];
			
			YUV444ToBGRA (y, u, v, op); op += 4;
			
			ip++;
			i++;

			if (i >= width) {
				i = 0;
				j++;
			}
		}
	}
	*buffer = output_buffer;
	*buflen = buffers[v4l2buf.index].length * 2;
#endif

	gint64 new_pts = v4l2buf.timestamp.tv_usec * 10ULL + v4l2buf.timestamp.tv_sec * 10000000ULL;

	if (first_pts == G_MAXINT64)
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
MoonVideoCaptureDeviceV4L2::SetCallbacks (MoonReportSampleFunc report_sample,
					  MoonFormatChangedFunc format_changed,
					  gpointer data)
{
	this->report_sample = report_sample;
	this->format_changed = format_changed;
	this->callback_data = data;
}


void
MoonVideoCaptureDeviceV4L2::StartCapturing ()
{
	printf ("MoonVideoCaptureDeviceV4L2::StartCapturing ()\n");

	delete capturing_format;

	if (desired_format)
		capturing_format = (MoonVideoFormatV4L2*)desired_format;
	else
		capturing_format = ((MoonVideoFormatV4L2*)formats->pdata[0]);

	struct v4l2_format fmt;
	memset (&fmt, 0, sizeof (fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = ((MoonVideoFormatV4L2*)capturing_format)->GetV4L2PixelFormat ();
	fmt.fmt.pix.width = capturing_format->GetWidth();
	fmt.fmt.pix.height = capturing_format->GetHeight();
	fmt.fmt.pix.bytesperline = 0; // the driver will fill this in

	if (-1 == ioctl (fd, VIDIOC_S_FMT, &fmt)) {
		perror ("VIDIOC_S_FMT");
		capturing_format = NULL;
		return;
	}

	if (-1 == ioctl (fd, VIDIOC_G_FMT, &fmt)) {
		perror ("VIDIOC_S_FMT");
		capturing_format = NULL;
		return;
	}

	capturing_format = new MoonVideoFormatV4L2 (MoonPixelFormatRGBA32,   /* the pixel format we report to moonlight */
						    0                        /* fps for moonlight FIXME from where? */,
						    fmt.fmt.pix.width * 4    /* stride */,
						    fmt.fmt.pix.width        /* width */,
						    fmt.fmt.pix.height       /* height */,

						    fmt.fmt.pix.pixelformat,  /* the pixel format v4l2 gave us */
						    fmt.fmt.pix.bytesperline /* the stride of the unconverted data */
						    );

	printf ("capturing with format %c%c%c%c, %d x %d, %d stride\n",
		(char)(fmt.fmt.pix.pixelformat & 0xff),
		(char)((fmt.fmt.pix.pixelformat >> 8) & 0xff),
		(char)((fmt.fmt.pix.pixelformat >> 16) & 0xff),
		(char)((fmt.fmt.pix.pixelformat >> 24) & 0xff),
		fmt.fmt.pix.width,
		fmt.fmt.pix.height,
		fmt.fmt.pix.bytesperline);
		
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

	g_free (buffer);

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
