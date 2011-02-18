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
#include "capture.h"
#include "factory.h"
#include "debug.h"

using namespace Moonlight;

void
MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (VideoCaptureDeviceCollection *col)
{
	GError *err = NULL;
	GDir *dir;
	bool found_default = false;
	const char *entry_name;
	char default_device_name [PATH_MAX];
	char name [PATH_MAX];
	int fd;
	MoonVideoCaptureDeviceV4L2 *device;
	VideoCaptureDevice *vcd;
	char *friendly_name;
	v4l2_capability cap;
	v4l2_input input;

	LOG_CAPTURE ("MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices ()\n");

	dir = g_dir_open ("/dev", 0, &err);
	if (!dir) { 
		// should hopefully never happen.  what is this, !linux?
		LOG_CAPTURE ("MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (): Could not open /dev: %s\n", err->message);
		g_error_free (err);
		return;
	}

	default_device_name [0] = 0;

	while ((entry_name = g_dir_read_name (dir))) {
		if (strncmp (entry_name, "video", 5))
			continue;

		if (g_snprintf (name, PATH_MAX, "/dev/%s", entry_name) > PATH_MAX)
			continue;

		// found a video device, make sure it works.
		fd = open (name, O_RDWR | O_NONBLOCK);
		if (fd == -1) {
			LOG_CAPTURE ("MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (): Could not open %s: %s\n", name, strerror (errno));
			continue;
		}

		// find the friendly name
		memset (&cap, 0, sizeof (cap));
		friendly_name = NULL;
		if (-1 == ioctl (fd, VIDIOC_QUERYCAP, &cap)) {
			memset (&input, 0, sizeof (input));
			if (-1 == ioctl (fd, VIDIOC_G_INPUT, &input.index)) {
				LOG_CAPTURE ("MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (): VIDIOC_G_INPUT failed: %s, setting friendly name to: %s\n", strerror (errno), name);
				friendly_name = g_strdup (name);
			} else if (-1 == ioctl (fd, VIDIOC_ENUMINPUT, &input)) {
				LOG_CAPTURE ("MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (): VIDIOC_ENUM_INPUT failed: %s, setting friendly name to: %s\n", strerror (errno), name);
				friendly_name = g_strdup (name);
			} else {
				friendly_name = g_strdup ((char*) input.name);
			}
		} else {
			friendly_name = g_strdup_printf ("%s (%s)", cap.card, cap.driver);
		}

		device = new MoonVideoCaptureDeviceV4L2 (fd, friendly_name);
		vcd = MoonUnmanagedFactory::CreateVideoCaptureDevice ();

		vcd->SetPalDevice (device);
		device->SetDevice (vcd);
		col->Add (vcd);
		vcd->unref ();
		g_free (friendly_name);
		friendly_name = NULL;

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
				vcd->SetIsDefaultDevice (true);
				found_default = true;
			}
		}
		else if (!strcmp (entry_name, default_device_name)) {
			// we found the destination of the /dev/video symlink, make this our default device
			vcd->SetIsDefaultDevice (true);
			found_default = true;
		}
		else if (!strcmp (entry_name, "video0") && !default_device_name [0] && !found_default) {
			// if it's /dev/video0 we also treat
			// it like the default device, but
			// only if there wasn't a viable
			// /dev/video.
			vcd->SetIsDefaultDevice (true);
			found_default = true;
		}

		LOG_CAPTURE ("MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (): found device: %s = %s\n", name, device->GetFriendlyName ());
	}

	LOG_CAPTURE ("MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices (): found %i devices\n", col->GetCount ());

	g_dir_close (dir);
}

MoonVideoCaptureDeviceV4L2::MoonVideoCaptureDeviceV4L2 (int fd, const char *friendly_name)
{
	this->fd = fd;
	this->friendly_name = g_strdup (friendly_name);
	this->formats = NULL;
	this->format_count = 0;
	this->first_pts = G_MAXUINT64;
	this->buffers = NULL;
	this->capturing = false;
	this->capture_pipe [0] = this->capture_pipe [1] = -1;
	memset (&capturing_format, 0, sizeof (capturing_format));
}

MoonVideoCaptureDeviceV4L2::~MoonVideoCaptureDeviceV4L2 ()
{
	StopCapturing ();

	if (capture_pipe [0] != -1) {
		close (capture_pipe [0]);
		capture_pipe [0] = -1;
	}
	if (capture_pipe [1] != -1) {
		close (capture_pipe [1]);
		capture_pipe [1] = -1;
	}

	close (fd);
	g_free (friendly_name);
	if (formats) {
		for (guint i = 0; i < format_count; i ++)
			delete formats [i];
		g_free (formats);
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
		if (format1->v4l2PixelFormat == (fmt)) { \
			if (format1->v4l2PixelFormat == format2->v4l2PixelFormat) \
				return format1->width * format1->height - format2->width * format2->height; \
			else						\
				return -1;				\
		}							\
		else if (format2->v4l2PixelFormat == (fmt))	\
			return 1;					\
	} G_STMT_END

	// we prefer YUYV if we can get it
	FORMAT (V4L2_PIX_FMT_YUYV);
	// with YV12 a close second
	FORMAT (V4L2_PIX_FMT_YVU420);
	// we can do jpeg too now
	FORMAT (V4L2_PIX_FMT_JPEG);

	// for now we don't care enough about the rest
	return 0;
}

void
MoonVideoCaptureDeviceV4L2::GetSupportedFormats (VideoFormatCollection *col)
{
	RetrieveFormats ();

	for (unsigned int i = 0; i < format_count; i++) {
		VideoFormat vf;
		MoonVideoFormatV4L2 *fmt = formats [i];
		vf.framesPerSecond = fmt->framesPerSecond;
		vf.height = fmt->height;
		vf.width = fmt->width;
		vf.stride = fmt->stride;
		vf.pixelFormat = MoonPixelFormatRGBA32;
		col->Add (Value (vf));
	}
}

void
MoonVideoCaptureDeviceV4L2::RetrieveFormats ()
{
	GPtrArray *array;
	MoonVideoFormatV4L2 *format;

	v4l2_fmtdesc fmt;
	v4l2_frmsizeenum frame_size;
	v4l2_frmivalenum frame_interval;

	if (formats)
		return;

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat ()\n");

	array = g_ptr_array_new ();

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	for (fmt.index = 0; ; fmt.index ++) {
		if (-1 == ioctl (fd, VIDIOC_ENUM_FMT, &fmt)) {
			if (errno != EINVAL) {
				LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () VIDIOC_ENUM_FMT failed: %s\n", strerror (errno));
			}
			break;
		}

		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () format: %s compressed: %i format: %c%c%c%c\n",
			fmt.description, fmt.flags & V4L2_FMT_FLAG_COMPRESSED,
			(char) (fmt.pixelformat & 0xFF),
			(char) ((fmt.pixelformat >> 8) & 0xFF),
			(char) ((fmt.pixelformat >> 16) & 0xFF),
			(char) ((fmt.pixelformat >> 24) & 0xFF));

		switch (fmt.pixelformat) {
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_YVU420:
		case V4L2_PIX_FMT_JPEG:
			break;
		default:
			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () The format: %c%c%c%c is not supported. Recording is allowed, but will likely produce garbage output.\n",
				(char) (fmt.pixelformat & 0xFF),
				(char) ((fmt.pixelformat >> 8) & 0xFF),
				(char) ((fmt.pixelformat >> 16) & 0xFF),
				(char) ((fmt.pixelformat >> 24) & 0xFF));
			break;
		}

		frame_size.pixel_format = fmt.pixelformat;

		bool discrete_framesizes = true;
		for (frame_size.index = 0; ; frame_size.index ++) {
			guint32 frame_width;
			guint32 frame_height;
			guint32 frame_rate;

			if (-1 == ioctl (fd, VIDIOC_ENUM_FRAMESIZES, &frame_size)) {
				if (errno != EINVAL) {
					LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () VIDIOC_ENUM_FRAMESIZES failed: %s\n", strerror (errno));
				}
				break;
			}

			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () format: %s compressed: %i format: %c%c%c%c frame size type: %s width: %i height: %i\n",
				fmt.description, fmt.flags & V4L2_FMT_FLAG_COMPRESSED,
				(char) (fmt.pixelformat & 0xFF),
				(char) ((fmt.pixelformat >> 8) & 0xFF),
				(char) ((fmt.pixelformat >> 16) & 0xFF),
				(char) ((fmt.pixelformat >> 24) & 0xFF),
				frame_size.type == V4L2_FRMSIZE_TYPE_DISCRETE ? "discrete" :
				(frame_size.type == V4L2_FRMSIZE_TYPE_STEPWISE ? "stepwise" :
				(frame_size.type == V4L2_FRMSIZE_TYPE_CONTINUOUS ? "continuous" : "?")),
				frame_size.discrete.width, frame_size.discrete.height);

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
					if (errno != EINVAL) {
						LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () VIDIOC_ENUM_FRAMEINTERVALS failed: %s\n", strerror (errno));
						break;
					}
					LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () no frame intervals, setting frame rate to 24\n");
					frame_rate = 24; /* I like this number */
					discrete_frameintervals = false;
				} else {
					if (frame_interval.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
						frame_rate = frame_interval.stepwise.max.denominator / frame_interval.stepwise.max.numerator;
						discrete_frameintervals = false;
					}
					else {
						frame_rate = frame_interval.discrete.denominator / frame_interval.discrete.numerator;;
					}
					LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () format: %s compressed: %i format: %c%c%c%c frame interval type: %s frame rate: %i\n",
						fmt.description, fmt.flags & V4L2_FMT_FLAG_COMPRESSED,
						(char) (fmt.pixelformat & 0xFF),
						(char) ((fmt.pixelformat >> 8) & 0xFF),
						(char) ((fmt.pixelformat >> 16) & 0xFF),
						(char) ((fmt.pixelformat >> 24) & 0xFF),
						frame_interval.type == V4L2_FRMIVAL_TYPE_DISCRETE ? "discrete" :
						(frame_interval.type == V4L2_FRMIVAL_TYPE_STEPWISE ? "stepwise" :
						(frame_interval.type == V4L2_FRMIVAL_TYPE_CONTINUOUS ? "continuous" : "?")),
						frame_rate);
				}

				format = new MoonVideoFormatV4L2 ();
				format->framesPerSecond = frame_rate;
				format->stride = frame_width * 4;
				format->width = frame_width;
				format->height = frame_height;
				format->v4l2PixelFormat = fmt.pixelformat;
				g_ptr_array_insert_sorted (array, video_format_comparer, format);
				format = NULL;

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

	format_count = array->len;
	formats = (MoonVideoFormatV4L2 **) array->pdata;
	g_ptr_array_free (array, false);

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::RetrieveFormat () retrieved %i formats\n", format_count);
}

const char*
MoonVideoCaptureDeviceV4L2::GetFriendlyName()
{
	return friendly_name;
}

static inline void YUV444ToBGRA(guint8 Y, guint8 U, guint8 V, guint8 *dst)
{
	dst[2] = CLAMP((298 * (Y - 16) + 409 * (V - 128) + 128) >> 8, 0, 255);
	dst[1] = CLAMP((298 * (Y - 16) - 100 * (U - 128) - 208 * (V - 128) + 128) >> 8, 0, 255);
	dst[0] = CLAMP((298 * (Y - 16) + 516 * (U - 128) + 128) >> 8, 0, 255);
	dst[3] = 0xFF;
}

void *
MoonVideoCaptureDeviceV4L2::CaptureLoopCallback (gpointer context)
{
	((MoonVideoCaptureDeviceV4L2 *) context)->CaptureLoop ();
	return NULL;
}

void
MoonVideoCaptureDeviceV4L2::CaptureLoop ()
{
	guint8 *buffer;
	guint8 *op;
	guint32 *ip;
	guint32 buflen;
	guint64 pts;
	guint64 new_pts;
	v4l2_buffer v4l2buf = { 0 };
	v4l2_requestbuffers reqbuf = { 0 };
	pollfd fds [2];
	int result;

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop ()\n");

	// we poll for the capture fd and the pipe at the same time
	fds [0].fd = fd;
	fds [0].events = POLLIN | POLLERR;
	fds [1].fd = capture_pipe [0];
	fds [1].events = POLLIN;

	while (true) {
		if (!capturing)
			break;

		// block here until the hardware is actually ready to start feeding us frames
		do {
			if (!capturing)
				break;

			fds [0].revents = 0;
			fds [1].revents = 0;

			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop () polling...\n");
			result = poll (fds, 2, 2000); // Have a timeout of 2 seconds, just in case something goes wrong.
			if (result == 0) { // Timed out
				// Nothing to do here
			} else if (result < 0) { // Some error poll exit condition
				// Doesn't matter what happened (happens quite often due to interrupts)
			} else { // Something woke up the poll
				if (fds [1].revents & POLLIN) {
					// We were asked to wake up
					// Read whatever was written into the pipe so that the pipe doesn't fill up.
					read (capture_pipe [0], &buffer, sizeof (int));
					LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop () woken up by ourselves\n");
				} else if (fds [0].revents & POLLIN) {
					// We've got a frame!
					break;
				} else if (fds [0].revents & POLLERR) {
					// Ugh. Not sure what this may require
					LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop () poll error on the capture fd\n");
				} else {
					// Now what?
				}
			}
		} while (true);

		if (!capturing)
			break;

		v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2buf.memory = V4L2_MEMORY_MMAP;
	
		if (-1 == ioctl (fd, VIDIOC_DQBUF, &v4l2buf)) {
			if (errno == EAGAIN) {
				// Non-blocking I/O has been selected using O_NONBLOCK and no buffer was in the outgoing queue.
				// No frame ready yet
				continue;
			}

			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop () VIDIOC_DQBUF failed: %s\n", strerror (errno));
			continue;
		}

		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop () got a frame, length: %d bytes used: %d\n", v4l2buf.length, v4l2buf.bytesused);
	
		buflen = capturing_format.height * capturing_format.stride;
		buffer = (guint8 *) g_malloc (buflen);
		op = buffer;
		ip = (guint32 *) buffers [v4l2buf.index].start;

		switch (capturing_format.v4l2PixelFormat) {
		case V4L2_PIX_FMT_YUYV: {
			while ((size_t) ((char *) ip - (char *) buffers [v4l2buf.index].start) < buffers [v4l2buf.index].length) {
				guint8 y, u, y2, v;
	
				y  = ((*ip & 0x000000ff));
				y2 = ((*ip & 0x00ff0000)>>16);
				u =  ((*ip & 0x0000ff00)>>8);
				v =  ((*ip & 0xff000000)>>24);
	
				YUV444ToBGRA (y, u, v, op); op += 4;
				YUV444ToBGRA (y2, u, v, op); op += 4;
				
				ip++;
			}
			break;
		}
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_YVU420: {
			int width = capturing_format.width;
			int height = capturing_format.height;
			bool u_first = (capturing_format.v4l2PixelFormat == V4L2_PIX_FMT_YUV420);
			guint8 *yp = (guint8 *) buffers [v4l2buf.index].start;
			guint8 *up = (guint8 *) buffers [v4l2buf.index].start + width * height + (u_first ? 0 : width * height / 4);
			guint8 *vp = (guint8 *) buffers [v4l2buf.index].start + width * height + (u_first ? width * height / 4 : 0);
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
			break;
		}
		case V4L2_PIX_FMT_JPEG: {
			Runtime::GetWindowingSystem ()->ConvertJPEGToBGRA (
				buffers [v4l2buf.index].start,
				buffers [v4l2buf.index].length,
				buffer,
				capturing_format.stride,
				capturing_format.height);
			break;
		}
		default: {
			// Make the default be to just copy the input video to the output
			// This will show something at least, even though it's just garbage
			memcpy (buffer, buffers [v4l2buf.index].start, MIN (buffers [v4l2buf.index].length, buflen));
			break;
		}
		}
	
		new_pts = v4l2buf.timestamp.tv_usec * 10ULL + v4l2buf.timestamp.tv_sec * 10000000ULL;
		if (first_pts == G_MAXUINT64)
			first_pts = new_pts;
		pts = new_pts - first_pts;

		int index = v4l2buf.index;
		memset (&v4l2buf, 0, sizeof (v4l2buf));
	
		v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2buf.memory = V4L2_MEMORY_MMAP;
		v4l2buf.index = index;
	
		// we've copied the data, give the buffer back to the device
		if (-1 == ioctl (fd, VIDIOC_QBUF, &v4l2buf)) {
			/* How can this fail? and how should it be dealt with? */
			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StopCapturing () VIDIOC_QBUF failed: %s\n", strerror (errno));
			/* Do nothing, we just continue trying to read frames */
		}

		GetDevice ()->GetCaptureSource ()->ReportSample (pts, 10000000LL / capturing_format.framesPerSecond, buffer, buflen);
	}

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop () Loop ended, stopping capture\n");

	int j = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (fd, VIDIOC_STREAMOFF, &j)) {
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StopCapturing () VIDIOC_STREAMOFF failed: %s\n", strerror (errno));
	}

	// release the video buffers
	if (-1 == ioctl (fd, VIDIOC_REQBUFS, &reqbuf)) {
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): failed to start capture (%s)\n", strerror (errno));
		return;
	}

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::CaptureLoop () Done\n");
}

void
MoonVideoCaptureDeviceV4L2::StartCapturing ()
{
	VideoFormat *desired_format;
	bool found_desired_format = false;
	v4l2_format fmt;
	v4l2_requestbuffers reqbuf = { 0 };
	v4l2_buffer buffer;

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing ()\n");
	VERIFY_MAIN_THREAD;

	if (capturing) {
		/* we're already capturing */
		return;
	}

	if (formats == NULL) {
		/* Huh? */
		return;
	}

	desired_format = GetDevice ()->GetDesiredFormat ();

	if (desired_format) {
		for (guint32 i = 0; i < format_count; i++) {
			if (formats [i]->width == desired_format->width &&
				formats [i]->height == desired_format->height &&
				formats [i]->stride == desired_format->stride &&
				formats [i]->framesPerSecond == desired_format->framesPerSecond) {
				capturing_format = *formats [i];
				found_desired_format = true;
				break;
			}
		}
	}

	if (!found_desired_format)
		capturing_format = *formats [0];

	memset (&fmt, 0, sizeof (fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = capturing_format.v4l2PixelFormat;
	fmt.fmt.pix.width = capturing_format.width;
	fmt.fmt.pix.height = capturing_format.height;
	fmt.fmt.pix.bytesperline = 0; // the driver will fill this in

	if (-1 == ioctl (fd, VIDIOC_S_FMT, &fmt)) {
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): VIDIOC_S_FMT failed: %s\n", strerror (errno));
		return;
	}

	if (-1 == ioctl (fd, VIDIOC_G_FMT, &fmt)) {
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): VIDIOC_S_FMT failed: %s\n", strerror (errno));
		return;
	}

	capturing_format.stride = fmt.fmt.pix.width * 4;
	capturing_format.width = fmt.fmt.pix.width;
	capturing_format.height = fmt.fmt.pix.height;
	capturing_format.v4l2PixelFormat = fmt.fmt.pix.pixelformat;

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing () capturing with format %c%c%c%c, %d x %d, %d stride %f fps\n",
		(char)(fmt.fmt.pix.pixelformat & 0xff),
		(char)((fmt.fmt.pix.pixelformat >> 8) & 0xff),
		(char)((fmt.fmt.pix.pixelformat >> 16) & 0xff),
		(char)((fmt.fmt.pix.pixelformat >> 24) & 0xff),
		capturing_format.width,
		capturing_format.height,
		fmt.fmt.pix.bytesperline,
		capturing_format.framesPerSecond);

	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = VIDEO_MAX_FRAME;

	if (-1 == ioctl (fd, VIDIOC_REQBUFS, &reqbuf)) {
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): failed to start capture (%s)\n", strerror (errno));
		return;
	}

	buffer_count = reqbuf.count;
	buffers = g_new0 (Buffer, reqbuf.count);

	for (unsigned int i = 0; i < reqbuf.count; i++) {
		memset (&buffer, 0, sizeof (buffer));
		buffer.type = reqbuf.type;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;

		if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buffer)) {
			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): VIDIOC_QUERYBUF failed: %s\n", strerror (errno));
			return;
		}

		buffers[i].length = buffer.length; /* remember for munmap() */

		buffers[i].start = mmap (NULL, buffer.length,
					 PROT_READ | PROT_WRITE,
					 MAP_SHARED,
					 fd, buffer.m.offset);

#if 0
		if (buffer.m.offset % 16 == 0) {
			printf ("buffer %d is 16 byte aligned\n", i);
		}
		else if (buffer.m.offset % 8 == 0) {
			printf ("buffer %d is 8 byte aligned\n", i);
		}
		else if (buffer.m.offset % 4 == 0) {
			printf ("buffer %d is 4 byte aligned\n", i);
		}
#endif

		if (MAP_FAILED == buffers[i].start) {
			/* If you do not exit here you should unmap() and free()
			   the buffers mapped so far. */
			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): mmap failed: %s\n", strerror (errno));
			return;
		}

		if (-1 == ioctl (fd, VIDIOC_QBUF, &buffer)) {
			LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): VIDIOC_QBUF failed: %s\n", strerror (errno));
			return;
		}
	}

	int j = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl (fd, VIDIOC_STREAMON, &j)) {
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): VIDIOC_STREAMON failed: %s\n", strerror (errno));
		return;
	}

	VideoFormat vfmt (MoonPixelFormatRGBA32, capturing_format.framesPerSecond, capturing_format.stride, capturing_format.width, capturing_format.height);
	GetDevice ()->GetCaptureSource ()->VideoFormatChanged (&vfmt);

	if (pipe (capture_pipe) != 0) {
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): Unable to create pipe (%s).\n", strerror (errno));
		return;
	}

	first_pts = G_MAXUINT64;

	capturing = true;
	if (pthread_create (&capture_thread, NULL, CaptureLoopCallback, this) != 0) {
		capturing = false;
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): failed to create capture thread: %s\n", strerror (errno));
		return;
	}

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StartCapturing (): capture started successfully\n");
}

void
MoonVideoCaptureDeviceV4L2::StopCapturing ()
{
	int result;

	LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StopCapturing ()\n");
	VERIFY_MAIN_THREAD;

	if (capturing) {
		capturing = false;

		// Wake up the capture thread in case it's polling
		do {
			result = write (capture_pipe [1], "c", 1);
		} while (result == 0);

		// Wait for the capture thread to end
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StopCapturing () waiting for capture thread to end...\n");
		pthread_join (capture_thread, NULL);
		LOG_CAPTURE ("MoonVideoCaptureDeviceV4L2::StopCapturing () capture thread ended.\n");

		close (capture_pipe [0]);
		close (capture_pipe [1]);
		capture_pipe [0] = -1;
		capture_pipe [1] = -1;
	}

	if (buffers != NULL) {
		for (int i = 0; i < buffer_count; i++)
			munmap (buffers [i].start, buffers [i].length);
		g_free (buffers);
		buffers = NULL;
	}
}
