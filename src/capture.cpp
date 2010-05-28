/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "capture.h"
#include "deployment.h"

/*
 * VideoFormat
 */

VideoFormat::VideoFormat (MoonVideoFormat *pal_format)
{
	pixelFormat = pal_format->GetPixelFormat ();
	framesPerSecond = pal_format->GetFramesPerSecond ();
	height = pal_format->GetHeight();
	width = pal_format->GetWidth ();
	stride = pal_format->GetStride ();
}

VideoFormat::VideoFormat (const VideoFormat &format)
{
	pixelFormat = format.pixelFormat;
	framesPerSecond = format.framesPerSecond;
	height = format.height;
	width = format.width;
	stride = format.stride;
}

/*
 * AudioFormat
 */

AudioFormat::AudioFormat (MoonAudioFormat *pal_format)
{
	bitsPerSample = pal_format->GetBitsPerSample ();
	channels = pal_format->GetChannels ();
	samplesPerSecond = pal_format->GetSamplesPerSecond ();
	waveFormat = pal_format->GetWaveFormat ();
}


/*
 * CaptureSource
 */

CaptureSource::CaptureSource ()
{
	SetObjectType (Type::CAPTURESOURCE);

	current_state = CaptureSource::Stopped;

	cached_sampleData = NULL;
	cached_sampleDataLength = 0;
	cached_sampleTime = 0;
	cached_frameDuration = 0;
}

void
CaptureSource::CaptureImageAsync ()
{
	printf ("CaptureSource::CaptureImageAsync ()\n");
}

void
CaptureSource::Start ()
{
	printf ("CaptureSource::Start ()\n");
	AudioCaptureDevice *audio_device = GetAudioCaptureDevice ();
	VideoCaptureDevice *video_device = GetVideoCaptureDevice ();

	if (audio_device)
		audio_device->Start ();
	if (video_device)
		video_device->Start (CaptureSource::ReportSampleCallback,
				     CaptureSource::VideoFormatChangedCallback,
				     this);

	current_state = CaptureSource::Started;
	if (HasHandlers (CaptureSource::CaptureStartedEvent))
		Emit (CaptureSource::CaptureStartedEvent);
}

void
CaptureSource::Stop ()
{
	AudioCaptureDevice *audio_device = GetAudioCaptureDevice ();
	VideoCaptureDevice *video_device = GetVideoCaptureDevice ();

	if (audio_device)
		audio_device->Stop ();
	if (video_device)
		video_device->Stop ();

	current_state = CaptureSource::Stopped;
	if (HasHandlers (CaptureSource::CaptureStoppedEvent))
		Emit (CaptureSource::CaptureStoppedEvent);
}

int
CaptureSource::GetState ()
{
	return current_state;
}

void
CaptureSource::ReportSampleCallback (gint64 sampleTime, gint64 frameDuration, guint8 *sampleData, int sampleDataLength, gpointer data)
{
	((CaptureSource*)data)->ReportSample (sampleTime, frameDuration, sampleData, sampleDataLength);
}

void
CaptureSource::ReportSample (gint64 sampleTime, gint64 frameDuration, guint8 *sampleData, int sampleDataLength)
{
	SetCurrentDeployment ();

	if (!cached_sampleData || sampleDataLength != cached_sampleDataLength) {
		g_free (cached_sampleData);
		cached_sampleData = (guint8*)g_malloc (sampleDataLength);
	}

	memmove (cached_sampleData, sampleData, sampleDataLength);
	cached_sampleDataLength = sampleDataLength;
	cached_sampleTime = sampleTime;
	cached_frameDuration = frameDuration;

	printf ("CaptureSource::ReportSample (%llu, %llu, %d\n", sampleTime, frameDuration, sampleDataLength);
	if (HasHandlers (CaptureSource::SampleReadyEvent)) {
		Emit (CaptureSource::SampleReadyEvent,
		      new SampleReadyEventArgs (cached_sampleTime, cached_frameDuration, cached_sampleData, cached_sampleDataLength));
	}
}

void
CaptureSource::VideoFormatChangedCallback (MoonVideoFormat *format, gpointer data)
{
	((CaptureSource*)data)->VideoFormatChanged (format);
}

void
CaptureSource::VideoFormatChanged (MoonVideoFormat *format)
{
	SetCurrentDeployment ();

	printf ("CaptureSource::VideoFormatChanged\n");
	if (HasHandlers (CaptureSource::FormatChangedEvent)) {
		VideoFormat vformat (format);
		Emit (CaptureSource::FormatChangedEvent,
		      new VideoFormatChangedEventArgs (&vformat));
	}
}

void
CaptureSource::GetSample (gint64 *sampleTime, gint64 *frameDuration, guint8 **sampleData, int *sampleDataLength)
{
	if (sampleTime)
		*sampleTime = cached_sampleTime;
	if (frameDuration)
		*frameDuration = cached_frameDuration;
	if (sampleData)
		*sampleData = cached_sampleData;
	if (sampleDataLength)
		*sampleDataLength = cached_sampleDataLength;
}


/*
 * CaptureDevice
 */

CaptureDevice::CaptureDevice ()
{
	SetObjectType (Type::CAPTUREDEVICE);
}


/*
 * AudioCaptureDevice
 */

AudioCaptureDevice::AudioCaptureDevice ()
{
	SetObjectType (Type::AUDIOCAPTUREDEVICE);
}

AudioCaptureDevice::~AudioCaptureDevice ()
{
}

void
AudioCaptureDevice::SetPalDevice (MoonCaptureDevice *device)
{
	CaptureDevice::SetPalDevice (device);

	MoonAudioCaptureDevice *audio_device = (MoonAudioCaptureDevice*)device;

	AudioFormatCollection *col = new AudioFormatCollection ();

	int num_formats;
	MoonAudioFormat **formats = audio_device->GetSupportedFormats (&num_formats);
	for (int i = 0; i < num_formats; i ++)
	  col->Add (Value (AudioFormat (formats[i])));

	SetSupportedFormats (col);

	SetFriendlyName (audio_device->GetFriendlyName());
}

void
AudioCaptureDevice::Start ()
{
	((MoonAudioCaptureDevice*)GetPalDevice())->StartCapturing ();
}

void
AudioCaptureDevice::Stop ()
{
	((MoonAudioCaptureDevice*)GetPalDevice())->StopCapturing ();
}


/*
 * VideoCaptureDevice
 */

VideoCaptureDevice::VideoCaptureDevice ()
{
	SetObjectType (Type::VIDEOCAPTUREDEVICE);
}


VideoCaptureDevice::~VideoCaptureDevice ()
{
}

void
VideoCaptureDevice::SetPalDevice (MoonCaptureDevice *device)
{
	CaptureDevice::SetPalDevice (device);

	MoonVideoCaptureDevice *video_device = (MoonVideoCaptureDevice*)device;

	VideoFormatCollection *col = new VideoFormatCollection ();

	int num_formats;
	MoonVideoFormat **formats = video_device->GetSupportedFormats (&num_formats);
	for (int i = 0; i < num_formats; i ++)
	  col->Add (Value (VideoFormat (formats[i])));

	SetSupportedFormats (col);

	SetFriendlyName (video_device->GetFriendlyName());
}

void
VideoCaptureDevice::Start (MoonReportSampleFunc report_sample,
			   MoonFormatChangedFunc format_changed,
			   gpointer data)
{
	printf ("VideoCaptureDevice::Start\n");
	((MoonVideoCaptureDevice*)GetPalDevice())->StartCapturing (report_sample, format_changed, data);
}

void
VideoCaptureDevice::Stop ()
{
	((MoonVideoCaptureDevice*)GetPalDevice())->StopCapturing ();

}

