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
#include "timesource.h"
#include "consent.h"

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

	pending_capture = false;
}

void
CaptureSource::Dispose ()
{
	CaptureDevice *dev;

	dev = GetAudioCaptureDevice ();
	if (dev != NULL)
		dev->RemoveAllHandlers (this);
	dev = GetVideoCaptureDevice ();
	if (dev != NULL)
		dev->RemoveAllHandlers (this);

	DependencyObject::Dispose ();
}

void
CaptureSource::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == CaptureSource::AudioCaptureDeviceProperty ||
		args->GetId () == CaptureSource::VideoCaptureDeviceProperty) {
		CaptureDevice *old_device = args->GetOldValue () ? args->GetOldValue ()->AsCaptureDevice () : NULL;
		CaptureDevice *new_device = args->GetNewValue () ? args->GetNewValue ()->AsCaptureDevice () : NULL;
		if (old_device != NULL) {
			old_device->Stop ();
			old_device->RemoveAllHandlers (this);
		}
		if (new_device != NULL) {
			new_device->AddHandler (CaptureDevice::SampleReadyEvent, SampleReadyCallback, this);
			new_device->AddHandler (CaptureDevice::CaptureStartedEvent, CaptureStartedCallback, this);
			new_device->AddHandler (CaptureDevice::FormatChangedEvent, FormatChangedCallback, this);
		}
	}

	DependencyObject::OnPropertyChanged (args, error);
}

void
CaptureSource::CaptureImageAsync ()
{
	LOG_CAPTURE ("CaptureSource::CaptureImageAsync ()\n");

	pending_capture = true;
}

void
CaptureSource::Start ()
{
	AudioCaptureDevice *audio_device;
	VideoCaptureDevice *video_device;

	LOG_CAPTURE ("CaptureSource::Start () %p %i current_state: %i audio device: %p video device: %p\n",
		this, GetId (), current_state, GetAudioCaptureDevice (), GetVideoCaptureDevice ());

	if (current_state == CaptureSource::Started)
		return;

	audio_device = GetAudioCaptureDevice ();
	if (audio_device) {
		audio_device->Start ();
	}

	video_device = GetVideoCaptureDevice ();
	if (video_device) {
		video_device->Start ();
	}

	current_state = CaptureSource::Started;
}

void
CaptureSource::Stop ()
{
	AudioCaptureDevice *audio_device;
	VideoCaptureDevice *video_device;

	LOG_CAPTURE ("CaptureSource::Stop () %p = %i state: %i\n", this, GetId (), current_state);

	if (current_state != CaptureSource::Started)
		return;

	audio_device = GetAudioCaptureDevice ();
	if (audio_device)
		audio_device->Stop ();

	video_device = GetVideoCaptureDevice ();
	if (video_device)
		video_device->Stop ();

	current_state = CaptureSource::Stopped;
	Emit (CaptureSource::CaptureStoppedEvent); // this is emitted sync.
}

int
CaptureSource::GetState ()
{
	return current_state;
}

void
CaptureSource::CaptureStartedHandler (CaptureDevice *source, EventArgs *args)
{
	if (HasHandlers (CaptureSource::CaptureStartedEvent)) {
		Emit (CaptureSource::CaptureStartedEvent);
	}
}

void
CaptureSource::FormatChangedHandler (CaptureDevice *source, CaptureFormatChangedEventArgs *args)
{
	if (HasHandlers (CaptureSource::FormatChangedEvent)) {
		args->ref ();
		Emit (CaptureSource::FormatChangedEvent, args);
	}
}

void
CaptureSource::SampleReadyHandler (CaptureDevice *source, SampleReadyEventArgs *args)
{
	if (current_state != Started) {
#if SANITY
		printf ("CaptureSource::SampleReadyHandler (): got sample when not started (state: %i)\n", current_state);
#endif
		return;
	}

	if (source->Is (Type::AUDIOCAPTUREDEVICE)) {
		/* Nothing to do here */
		LOG_CAPTURE ("CaptureSource::SampleReadyHandler (%s)  %p %i sampleTime: %" G_GINT64_FORMAT " = %" G_GINT64_FORMAT "ms, frameDuration: %" G_GINT64_FORMAT " = %" G_GINT64_FORMAT " ms, sampleDataLength: %d\n",
			source->GetTypeName (), this, GetId (), args->GetSampleTime (), MilliSeconds_FromPts (args->GetSampleTime ()), args->GetFrameDuration (), MilliSeconds_FromPts (args->GetFrameDuration ()), args->GetSampleDataLength ());
	} else if (source->Is (Type::VIDEOCAPTUREDEVICE)) {
		VideoFormat *format = args->GetVideoFormat ();
		void *sampleData = args->GetSampleData ();
		guint32 sampleDataLength = args->GetSampleDataLength ();

		if (pending_capture) {
			if (HasHandlers (CaptureSource::CaptureImageCompletedEvent)) {
				BitmapImage *source = MoonUnmanagedFactory::CreateBitmapImage ();
				CaptureImageCompletedEventArgs *capture_args;
				source->SetPixelWidth (format->width);
				source->SetPixelHeight (format->height);
				source->SetBitmapData (g_memdup (sampleData, sampleDataLength), true);
				source->Invalidate (); // causes the BitmapSource to create its image_surface

				capture_args = MoonUnmanagedFactory::CreateCaptureImageCompletedEventArgs ();
				capture_args->SetSource (source);
				source->unref ();

				Emit (CaptureSource::CaptureImageCompletedEvent, capture_args);
			}

			pending_capture = false;
		}

		LOG_CAPTURE ("CaptureSource::SampleReadyHandler (%s)  %p %i %ix%i sampleTime: %" G_GINT64_FORMAT " = %" G_GINT64_FORMAT "ms, frameDuration: %" G_GINT64_FORMAT " = %" G_GINT64_FORMAT " ms, sampleDataLength: %d\n",
			source->GetTypeName (), this, GetId (), args->GetVideoFormat ()->width, args->GetVideoFormat ()->height, args->GetSampleTime (), MilliSeconds_FromPts (args->GetSampleTime ()), args->GetFrameDuration (), MilliSeconds_FromPts (args->GetFrameDuration ()), args->GetSampleDataLength ());
	}

	if (HasHandlers (CaptureSource::SampleReadyEvent)) {
		args->ref ();
		Emit (CaptureSource::SampleReadyEvent, args);
	}
}

/*
 * CaptureDevice
 */

CaptureDevice::CaptureDevice (Type::Kind object_type)
	: DependencyObject (object_type)
{
	pal_device = NULL;
	start_position = 0;
	audio_buffer = NULL;
	audio_buffer_size = 0;
	audio_buffer_used = 0;
	audio_position = 0;
}

void
CaptureDevice::Dispose ()
{
	delete pal_device;
	pal_device = NULL;

	DependencyObject::Dispose ();
}

CaptureDevice::~CaptureDevice ()
{
	g_free (audio_buffer);
	audio_buffer = NULL;
}

void
CaptureDevice::SetPalDevice (MoonCaptureDevice *value)
{
	VERIFY_MAIN_THREAD;
#if SANITY
	g_warn_if_fail (pal_device == NULL);
#endif
	pal_device = value;
}

MoonCaptureDevice *
CaptureDevice::GetPalDevice ()
{
	VERIFY_MAIN_THREAD;
	return pal_device;
}

void
CaptureDevice::EmitEventCallback (EventObject *sender)
{
	((CaptureDevice *) sender)->EmitEvent ();
}

void
CaptureDevice::EmitEvent ()
{
	EventData *data;

	VERIFY_MAIN_THREAD;

	events_mutex.Lock ();
	data = (EventData *) events.First ();
	if (data != NULL)
		events.Unlink (data);
	events_mutex.Unlock ();

	if (data == NULL)
		return;

	if (data->event == CaptureDevice::SampleReadyEvent) {
		LOG_CAPTURE ("%s::EmitEvent () SampleReadyEvent\n", GetTypeName ());
		if (Is (Type::AUDIOCAPTUREDEVICE)) {
			AudioCaptureDevice *device = (AudioCaptureDevice *) this;
			AudioFormat *format = data->aformat;
			guint64 audio_duration = format->GetDuration (audio_buffer_size);
			guint8 *buf_ptr;
			guint32 buf_left;
			guint32 buf_cp;
	
			LOG_CAPTURE ("AudioCaptureDevice::SampleReadyHandler () %p %i sample data length: %d audio_buffer_used: %i audio_buffer_size: %i frame size: %i\n",
				this, GetId (), data->sampleDataLength, audio_buffer_used, audio_buffer_size, device->GetClampedAudioFrameSize ());

			buf_left = data->sampleDataLength;
			buf_ptr = (guint8 *) data->sampleData;
	
			do {
				buf_cp = MIN (audio_buffer_size - audio_buffer_used, buf_left);
				memcpy (audio_buffer + audio_buffer_used, buf_ptr, buf_cp);
				buf_ptr += buf_cp;
				buf_left -= buf_cp;
				audio_buffer_used += buf_cp;

				if (audio_buffer_used == audio_buffer_size) {
					/* Report the audio data we have */
					LOG_CAPTURE ("AudioCaptureDevice::SampleReadyHandler () emitting: position: %" G_GUINT64_FORMAT " ms duration: %" G_GUINT64_FORMAT " ms bytes: %i\n",
						MilliSeconds_FromPts (audio_position), MilliSeconds_FromPts (audio_duration), audio_buffer_size);
					Emit (CaptureDevice::SampleReadyEvent, new SampleReadyEventArgs (audio_position, audio_duration, audio_buffer, audio_buffer_size, data->aformat, data->vformat));
					audio_position += audio_duration;
					audio_buffer_used = 0;
				}
			} while (buf_left > 0);
		} else if (Is (Type::VIDEOCAPTUREDEVICE)) {
			gint64 sampleTime = get_now () - start_position;
			gint64 frameDuration = 10000000000ULL / data->vformat->framesPerSecond;
			LOG_CAPTURE ("CaptureDevice::EmitEvent () emitting video SampleReadyEvent %ix%i, %p, %i)\n", data->vformat->width, data->vformat->height, data->sampleData, data->sampleDataLength);
			Emit (CaptureDevice::SampleReadyEvent, new SampleReadyEventArgs (sampleTime, frameDuration, data->sampleData, data->sampleDataLength, data->aformat, data->vformat));
		}
	} else if (data->event == CaptureDevice::CaptureStartedEvent) {
		LOG_CAPTURE ("%s::EmitEvent () CaptureStartedEvent\n", GetTypeName ());
		Emit (CaptureDevice::CaptureStartedEvent);
	} else if (data->event == CaptureDevice::FormatChangedEvent) {
		LOG_CAPTURE ("%s::EmitEvent () FormatChangedEvent\n", GetTypeName ());
		if (Is (Type::AUDIOCAPTUREDEVICE)) {
			AudioCaptureDevice *device = (AudioCaptureDevice *) this;
			AudioFormat *format = data->aformat;
			/* resize buffer */
			audio_buffer_size = format->GetByteCount (device->GetClampedAudioFrameSize ());
			audio_buffer = (guint8 *) g_realloc (audio_buffer, audio_buffer_size);
			audio_buffer_used = 0;
			Emit (CaptureDevice::FormatChangedEvent, new CaptureFormatChangedEventArgs (data->aformat));
		} else if (Is (Type::VIDEOCAPTUREDEVICE)) {
			Emit (CaptureDevice::FormatChangedEvent, new CaptureFormatChangedEventArgs (data->vformat));
		}
	}

	delete data;
}

void
CaptureDevice::EmitCaptureStarted ()
{
	SetCurrentDeployment (false);
	events_mutex.Lock ();
	events.Append (new EventData (CaptureDevice::CaptureStartedEvent));
	events_mutex.Unlock ();
	AddTickCall (EmitEventCallback);
}

void
CaptureDevice::EmitVideoFormatChanged (VideoFormat *format)
{
	SetCurrentDeployment (false);
	events_mutex.Lock ();
	events.Append (new EventData (CaptureDevice::FormatChangedEvent, new VideoFormat (*format)));
	events_mutex.Unlock ();
	AddTickCall (EmitEventCallback);
}

void
CaptureDevice::EmitAudioFormatChanged (AudioFormat *format)
{
	SetCurrentDeployment (false);
	events_mutex.Lock ();
	events.Append (new EventData (CaptureDevice::FormatChangedEvent, new AudioFormat (*format)));
	events_mutex.Unlock ();
	AddTickCall (EmitEventCallback);
}

void
CaptureDevice::EmitAudioSampleReady (AudioFormat *format, void *sampleData, int sampleDataLength)
{
	SetCurrentDeployment (false);
	events_mutex.Lock ();
	events.Append (new EventData (CaptureDevice::SampleReadyEvent, new AudioFormat (*format), sampleData, sampleDataLength));
	events_mutex.Unlock ();
	AddTickCall (EmitEventCallback);
}

void
CaptureDevice::EmitVideoSampleReady (VideoFormat *format, void *sampleData, int sampleDataLength)
{
	LOG_CAPTURE ("CaptureDevice::EmitVideoSampleReady (%ix%i, %p, %i)\n", format->width, format->height, sampleData, sampleDataLength);
	SetCurrentDeployment (false);
	events_mutex.Lock ();
	if (events.Length () < 50) {
		events.Append (new EventData (CaptureDevice::SampleReadyEvent, new VideoFormat (*format), sampleData, sampleDataLength));
	} else {
		g_free (sampleData);
	}
	events_mutex.Unlock ();
	AddTickCall (EmitEventCallback);
}

void
CaptureDevice::Start ()
{
	LOG_CAPTURE ("%s::Start ()\n", GetTypeName ());
	start_position = get_now ();
	audio_position = 0;
	GetPalDevice ()->StartCapturing ();
}

void
CaptureDevice::Stop ()
{
	LOG_CAPTURE ("%s::Stop ()\n", GetTypeName ());
	GetPalDevice ()->StopCapturing ();
	events_mutex.Lock ();
	events.Clear (true);
	events_mutex.Unlock ();
}

/*
 * AudioCaptureDevice
 */

AudioCaptureDevice::AudioCaptureDevice ()
	: CaptureDevice (Type::AUDIOCAPTUREDEVICE)
{
	LOG_CAPTURE ("AudioCaptureDevice ()\n");
}

int
AudioCaptureDevice::GetClampedAudioFrameSize ()
{
	int v = GetValue (AudioCaptureDevice::AudioFrameSizeProperty)->AsInt32 ();
	if (v < 10)
		return 10;
	if (v > 2000)
		return 2000;
	return v;
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

/*
 * VideoCaptureDevice
 */

VideoCaptureDevice::VideoCaptureDevice ()
	: CaptureDevice (Type::VIDEOCAPTUREDEVICE)
{
	LOG_CAPTURE ("VideoCaptureDevice ()\n");
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

/*
 * CaptureDeviceConfiguration
 */

AudioCaptureDevice *
CaptureDeviceConfiguration::GetDefaultAudioCaptureDevice ()
{
	AudioCaptureDeviceCollection *col = MoonUnmanagedFactory::CreateAudioCaptureDeviceCollection ();
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
	VideoCaptureDeviceCollection *col = MoonUnmanagedFactory::CreateVideoCaptureDeviceCollection ();
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
