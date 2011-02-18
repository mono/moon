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
#include "writeablebitmap.h"
#include "bitmapimage.h"
#include "factory.h"
#include "debug.h"
#include "clock.h"
#include "moonlightconfiguration.h"

namespace Moonlight {

/*
 * VideoFormat
 */

VideoFormat::VideoFormat ()
{
	pixelFormat = MoonPixelFormatNone;
	framesPerSecond = 0;
	height = 0;
	width = 0;
	stride = 0;
}

VideoFormat::VideoFormat (MoonPixelFormat format, float framesPerSecond, int stride, int width, int height)
	: framesPerSecond (framesPerSecond), height (height), width (width), stride (stride), pixelFormat (format)
{
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

AudioFormat::AudioFormat ()
{
	bitsPerSample = 0;
	channels = 0;
	samplesPerSecond = 0;
	waveFormat = MoonWaveFormatTypePCM;
}

AudioFormat::AudioFormat (MoonWaveFormatType format, int bitsPerSample, int channels, int samplesPerSecond)
	: bitsPerSample (bitsPerSample), channels (channels), samplesPerSecond (samplesPerSecond), waveFormat (format)
{
}

/*
 * CaptureSource
 */

CaptureSource::CaptureSource ()
	: DependencyObject (Type::CAPTURESOURCE)
{
	LOG_CAPTURE ("CaptureSource::CaptureSource (): %p = %i\n", this, GetId ());
	current_state = CaptureSource::Stopped;

	cached_sampleData = NULL;
	cached_sampleDataLength = 0;
	cached_sampleTime = 0;
	cached_frameDuration = 0;

	need_image_capture = false;
	video_capture_format = NULL;
}

CaptureSource::~CaptureSource ()
{
	LOG_CAPTURE ("CaptureSource::~CaptureSource (): %p = %i\n", this, GetId ());

	g_free (cached_sampleData);
	cached_sampleData = NULL;

	delete video_capture_format;
	video_capture_format = NULL;
}

void
CaptureSource::CaptureImageAsync ()
{
	LOG_CAPTURE ("CaptureSource::CaptureImageAsync ()\n");

	if (current_state != CaptureSource::Started) {
		VideoCaptureDevice *video_device = GetVideoCaptureDevice ();
		// we weren't started, so let's start now
		if (video_device) {
			video_device->SetCaptureSource (this);
			video_device->Start ();
		}
	}

	need_image_capture = true;
}

void
CaptureSource::Start ()
{
	AudioCaptureDevice *audio_device;
	VideoCaptureDevice *video_device;

	LOG_CAPTURE ("CaptureSource::Start () %p = %i current_state: %i audio device: %p video device: %p CaptureStarted %i FormatChanged: %i SampleReady: %i CaptureStopped: %i\n",
		this, GetId (), current_state, GetAudioCaptureDevice (), GetVideoCaptureDevice (), HasHandlers (CaptureSource::CaptureStartedEvent), HasHandlers (CaptureSource::FormatChangedEvent), HasHandlers (CaptureSource::SampleReadyEvent), HasHandlers (CaptureSource::CaptureStoppedEvent));

	if (current_state == CaptureSource::Started)
		return;

	// We need to emit CaptureStarted before actually
	// starting the devices, since their Start implementations
	// will cause FormatChanged to be emitted, which must be
	// emitted after CaptureStarted.
	current_state = CaptureSource::Started;
	if (HasHandlers (CaptureSource::CaptureStartedEvent))
		Emit (CaptureSource::CaptureStartedEvent);

	audio_device = GetAudioCaptureDevice ();
	if (audio_device) {
		audio_device->SetCaptureSource (this);
		audio_device->Start ();
	}

	video_device = GetVideoCaptureDevice ();
	if (video_device) {
		video_device->SetCaptureSource (this);
		if (!need_image_capture) {
			// if need_image_capture is true the image
			// capture stuff has already started the
			// device
			video_device->Start ();
		}
	}
}

void
CaptureSource::Stop ()
{
	LOG_CAPTURE ("CaptureSource::Stop () %p = %i\n", this, GetId ());

	AudioCaptureDevice *audio_device = GetAudioCaptureDevice ();
	VideoCaptureDevice *video_device = GetVideoCaptureDevice ();

	if (current_state != CaptureSource::Started && !need_image_capture)
		return;

	if (audio_device)
		audio_device->Stop ();
	if (video_device) {
		if (need_image_capture) {
			// if we're waiting for an image capture,
			// wait until the image capture has completed
		}
		else {
			video_device->Stop ();
		}
	}

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
CaptureSource::OnSampleReadyCallback (EventObject *obj)
{
	((CaptureSource *) obj)->OnSampleReady ();
}

void
CaptureSource::OnSampleReady ()
{
	SampleData *data = NULL;

	mutex.Lock ();
	data = (SampleData *) samples.First ();
	if (data != NULL)
		samples.Unlink (data);
	mutex.Unlock ();

	if (data) {
		ReportSample (data->sampleTime, data->frameDuration, data->sampleData, data->sampleDataLength);
		data->sampleData = NULL;
		delete data;
	}
}

void
CaptureSource::ReportSample (gint64 sampleTime, gint64 frameDuration, void *sampleData, int sampleDataLength)
{
	SampleData *data;

	SetCurrentDeployment (false); // we can't call into managed code here, we might be on the audio thread

	if (!Surface::InMainThread ()) {
		data = new SampleData ();
		data->sampleTime = sampleTime;
		data->frameDuration = frameDuration;
		data->sampleData = sampleData;
		data->sampleDataLength = sampleDataLength;
		mutex.Lock ();
		samples.Append (data);
		mutex.Unlock ();
		AddTickCall (OnSampleReadyCallback);
		return;
	}

	SetCurrentDeployment (); // now we're not on the audio thread anymore

	g_free (cached_sampleData);
	cached_sampleData = sampleData;
	cached_sampleDataLength = sampleDataLength;
	cached_sampleTime = sampleTime;
	cached_frameDuration = frameDuration;

	if (need_image_capture) {
		CaptureImageReportSample (sampleTime, frameDuration, cached_sampleData, sampleDataLength);
		need_image_capture = false;
	}

	LOG_CAPTURE ("CaptureSource::ReportSample (%" G_GINT64_FORMAT " = %" G_GINT64_FORMAT "ms, %" G_GINT64_FORMAT " = %" G_GINT64_FORMAT " ms, %d) %p %i has handlers: %i\n",
		sampleTime, MilliSeconds_FromPts (sampleTime), frameDuration, MilliSeconds_FromPts (frameDuration), sampleDataLength, this, GetId (), HasHandlers (CaptureSource::SampleReadyEvent));

	if (HasHandlers (CaptureSource::SampleReadyEvent)) {
		Emit (CaptureSource::SampleReadyEvent,
		      new SampleReadyEventArgs (cached_sampleTime, cached_frameDuration, cached_sampleData, cached_sampleDataLength));
	}
}

void
CaptureSource::VideoFormatChanged (VideoFormat *format)
{
	SetCurrentDeployment ();

	LOG_CAPTURE ("CaptureSource::VideoFormatChanged\n");

	delete video_capture_format;
	video_capture_format = new VideoFormat (*format);

	if (HasHandlers (CaptureSource::FormatChangedEvent)) {
		Emit (CaptureSource::FormatChangedEvent, new CaptureFormatChangedEventArgs (format));
	}
}

void
CaptureSource::AudioFormatChanged (AudioFormat *format)
{
	SetCurrentDeployment ();

	LOG_CAPTURE ("CaptureSource::AudioFormatChanged () %p = %i has handlers: %i %s channels: %i rate: %i bits per sample: %i\n",
		this, GetId (), HasHandlers (CaptureSource::FormatChangedEvent), GetTypeName (), format->channels, format->samplesPerSecond, format->bitsPerSample);

	if (HasHandlers (CaptureSource::FormatChangedEvent)) {
		Emit (CaptureSource::FormatChangedEvent, new CaptureFormatChangedEventArgs (format));
	}
}

void
CaptureSource::CaptureImageReportSample (gint64 sampleTime, gint64 frameDuration, void *sampleData, int sampleDataLength)
{
	SetCurrentDeployment ();

	LOG_CAPTURE ("CaptureSource::CaptureImageReportSample (%lld, %lld, %d\n", (long long) sampleTime, (long long) frameDuration, sampleDataLength);

	if (HasHandlers (CaptureSource::CaptureImageCompletedEvent)) {
		BitmapImage *source = MoonUnmanagedFactory::CreateBitmapImage ();
		source->SetPixelWidth (video_capture_format->width);
		source->SetPixelHeight (video_capture_format->height);

		source->SetBitmapData (sampleData, false);

		source->Invalidate (); // causes the BitmapSource to create its image_surface

		CaptureImageCompletedEventArgs *args = MoonUnmanagedFactory::CreateCaptureImageCompletedEventArgs (NULL);

		args->SetSource (source);

		source->unref ();

		Emit (CaptureSource::CaptureImageCompletedEvent, args);
	}

	// if we started the pal device strictly for an image capture
	// (or kept it running after the user had called
	// CaptureSource::Stop), stop it now.
	if (current_state != CaptureSource::Started) {
		VideoCaptureDevice *video_device = GetVideoCaptureDevice ();
		video_device->Stop();
	}
}

void
CaptureSource::GetSample (gint64 *sampleTime, gint64 *frameDuration, void **sampleData, int *sampleDataLength)
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

CaptureDevice::CaptureDevice (Type::Kind object_type)
	: DependencyObject (object_type)
{
	capture_source = NULL;
	pal_device = NULL;
}

CaptureDevice::~CaptureDevice ()
{
	delete pal_device;
}

void
CaptureDevice::SetCaptureSource (CaptureSource *value)
{
	LOG_CAPTURE ("%s::SetCaptureSource (%p)\n", GetTypeName (), value);
	capture_source = value;
}

/*
 * AudioCaptureDevice
 */

AudioCaptureDevice::AudioCaptureDevice ()
	: CaptureDevice (Type::AUDIOCAPTUREDEVICE)
{
	LOG_CAPTURE ("AudioCaptureDevice ()\n");
}

AudioCaptureDevice::~AudioCaptureDevice ()
{
}

void
AudioCaptureDevice::SetPalDevice (MoonCaptureDevice *device)
{
	AudioFormatCollection *col;
	MoonAudioCaptureDevice *audio_device;

	CaptureDevice::SetPalDevice (device);

	audio_device = (MoonAudioCaptureDevice*)device;
	audio_device->SetDevice (this);

	col = MoonUnmanagedFactory::CreateAudioFormatCollection ();
	audio_device->GetSupportedFormats (col);
	SetSupportedFormats (col);
	col->unref ();

	SetFriendlyName (audio_device->GetFriendlyName ());
}

void
AudioCaptureDevice::Start ()
{
	LOG_CAPTURE ("AudioCaptureDevice::Start ()\n");
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
	: CaptureDevice (Type::VIDEOCAPTUREDEVICE)
{
	LOG_CAPTURE ("VideoCaptureDevice ()\n");
}

VideoCaptureDevice::~VideoCaptureDevice ()
{
}

void
VideoCaptureDevice::SetPalDevice (MoonCaptureDevice *device)
{
	VideoFormatCollection *col;
	MoonVideoCaptureDevice *video_device;

	CaptureDevice::SetPalDevice (device);

	video_device = (MoonVideoCaptureDevice*)device;

	col = MoonUnmanagedFactory::CreateVideoFormatCollection ();
	video_device->GetSupportedFormats (col);
	SetSupportedFormats (col);
	col->unref ();

	SetFriendlyName (video_device->GetFriendlyName());
}

void
VideoCaptureDevice::Start ()
{
	LOG_CAPTURE ("VideoCaptureDevice::Start ()\n");
	((MoonVideoCaptureDevice*)GetPalDevice())->StartCapturing ();
}

void
VideoCaptureDevice::Stop ()
{
	LOG_CAPTURE ("VideoCaptureDevice::Stop ()\n");
	((MoonVideoCaptureDevice*)GetPalDevice())->StopCapturing ();

}

/*
 * CaptureDeviceConfiguration
 */

AudioCaptureDevice *
CaptureDeviceConfiguration::GetDefaultAudioCaptureDevice ()
{
	AudioCaptureDeviceCollection *col = new AudioCaptureDeviceCollection ();
	AudioCaptureDevice *result;

	GetAvailableAudioCaptureDevices (col);
	result = (AudioCaptureDevice *) SelectDefaultDevice (col, "Audio");
	if (result)
		result->ref ();
	col->unref ();

	return result;
}

VideoCaptureDevice *
CaptureDeviceConfiguration::GetDefaultVideoCaptureDevice ()
{
	VideoCaptureDeviceCollection *col = new VideoCaptureDeviceCollection ();
	VideoCaptureDevice *result;

	GetAvailableVideoCaptureDevices (col);
	result = (VideoCaptureDevice *) SelectDefaultDevice (col, "Video");
	if (result)
		result->ref ();
	col->unref ();

	return result;
}

void
CaptureDeviceConfiguration::GetAvailableVideoCaptureDevices (VideoCaptureDeviceCollection *col)
{
	MoonCaptureService *service = Runtime::GetCaptureService ();
	MoonVideoCaptureService *video;
	if (service == NULL)
		return;
	video = service->GetVideoCaptureService ();
	if (video == NULL)
		return;
	video->GetAvailableCaptureDevices (col);
	SelectDefaultDevice (col, "Video");
}

void
CaptureDeviceConfiguration::GetAvailableAudioCaptureDevices (AudioCaptureDeviceCollection *col)
{
	MoonCaptureService *service = Runtime::GetCaptureService ();
	MoonAudioCaptureService *audio;
	if (service == NULL)
		return;
	audio = service->GetAudioCaptureService ();
	if (audio == NULL)
		return;
	audio->GetAvailableCaptureDevices (col);
	SelectDefaultDevice (col, "Audio");
}

bool
CaptureDeviceConfiguration::RequestSystemAccess ()
{
	MoonCaptureService *service = Runtime::GetCaptureService ();
	if (service == NULL)
		return false;
	return service->RequestSystemAccess ();
}

CaptureDevice *
CaptureDeviceConfiguration::SelectDefaultDevice (DependencyObjectCollection *col, const char *type)
{
	MoonlightConfiguration config;
	char *default_device;
	CaptureDevice *cd;
	CaptureDevice *user_default = NULL;
	CaptureDevice *service_default = NULL;

	if (col->GetCount () == 0)
		return NULL;

	if (col->GetCount () == 1)
		return col->GetValueAt (0)->AsCaptureDevice ();

	/* Find the default selected by the capture service - the first device that has IsDefaultDevice = true, or if none are default, the first device */
	service_default = col->GetValueAt (0)->AsCaptureDevice ();
	for (int i = 0; i < col->GetCount (); i++) {
		cd = col->GetValueAt (0)->AsCaptureDevice ();
		if (!cd->GetIsDefaultDevice ())
			continue;
		service_default = cd;
		break;
	}

	if (!config.HasKey ("Capture", type)) {
		/* The default selected by the capture services is perfectly fine, nothing has been configured by the user */
		return service_default;
	}

	default_device = config.GetStringValue ("Capture", type);

	if (default_device == NULL || default_device [0] == 0) {
		g_free (default_device);
		return service_default;
	}

	/* Check if this device exists in the collection */
	for (int i = 0; i < col->GetCount (); i++) {
		cd = col->GetValueAt (i)->AsCaptureDevice ();
		if (strcmp (cd->GetFriendlyName (), default_device) == 0) {
			user_default = cd;
			break;
		}
	}

	g_free (default_device);

	if (user_default == NULL) {
		/* The default device doesn't exist. Let's stay with what the capture service selected as the default device */
		return service_default;
	}

	/* Update IsDefaultDevice for all devices */
	for (int i = 0; i < col->GetCount (); i++) {
		cd = col->GetValueAt (i)->AsCaptureDevice ();
		cd->SetIsDefaultDevice (cd == user_default);
	}

	return user_default;
}
};
