/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-linux-audio-capture.cpp:
 *
 * Copyright 2010-2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include "factory.h"
#include "capture.h"
#include "collection.h"
#include "pal-linux-audio-capture.h"
#include "audio.h"

using namespace Moonlight;

/*
 * MoonAudioCaptureServiceLinux
 */

void
MoonAudioCaptureServiceLinux::GetAvailableCaptureDevices (AudioCaptureDeviceCollection *col)
{
	const int size = 16;
	AudioRecorder *recorders [size];
	MoonAudioCaptureDeviceLinux *dev;
	AudioCaptureDevice *output;
	int count;

	count = AudioPlayer::CreateRecorders (recorders, size);
	for (int i = 0; i < count; i++) {
		output = MoonUnmanagedFactory::CreateAudioCaptureDevice ();
		dev = new MoonAudioCaptureDeviceLinux (recorders [i]);
		recorders [i]->unref ();
		output->SetPalDevice (dev);
		dev->SetDevice (output);
		dev->GetRecorder ()->SetDevice (output);
		col->Add (output);
		output->unref ();
	}
}

/*
 * MoonAudioCaptureDeviceLinux
 */

MoonAudioCaptureDeviceLinux::MoonAudioCaptureDeviceLinux (AudioRecorder *recorder)
{
	LOG_CAPTURE ("MoonAudioCaptureDeviceLinux ()\n");
	this->recorder = recorder;
	this->recorder->ref ();
}

MoonAudioCaptureDeviceLinux::~MoonAudioCaptureDeviceLinux ()
{
	LOG_CAPTURE ("MoonAudioCaptureDeviceLinux ~()\n");
	recorder->SetDevice (NULL);
	recorder->unref ();
	recorder = NULL;
}

void
MoonAudioCaptureDeviceLinux::GetSupportedFormats (AudioFormatCollection *col)
{
	LOG_CAPTURE ("MoonAudioCaptureDeviceLinux::GetSupportedFormats () recorder: %p\n", recorder);

	if (recorder == NULL)
		return;

	recorder->GetSupportedFormats (col);
}

const char*
MoonAudioCaptureDeviceLinux::GetFriendlyName ()
{
	LOG_CAPTURE ("MoonAudioCaptureDeviceLinux::GetFriendlyName () recorder: %p name: %s\n", recorder, recorder ? recorder->GetFriendlyName () : NULL);
	return recorder ? recorder->GetFriendlyName () : NULL;
}

void
MoonAudioCaptureDeviceLinux::StartCapturing ()
{
	LOG_CAPTURE ("MoonAudioCaptureDeviceLinux::StartCapturing () recorder: %p\n", recorder);

	if (recorder == NULL)
		return;

	recorder->Record ();
}

void MoonAudioCaptureDeviceLinux::StopCapturing ()
{
	LOG_CAPTURE ("MoonAudioCaptureDeviceLinux::StopCapturing () %p\n", recorder);

	if (recorder == NULL)
		return;

	recorder->Stop ();
}

