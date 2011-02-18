/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-linux-audio-capture.h
 *
 * Copyright 2010-2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_PAL_LINUX_AUDIO_CAPTURE_H
#define MOON_PAL_LINUX_AUDIO_CAPTURE_H

#include "pal-linux-capture.h"
#include "value.h"

namespace Moonlight {

/*
 * MoonAudioCaptureServiceLinux
 */
class MoonAudioCaptureServiceLinux : public MoonAudioCaptureService {
public:
	MoonAudioCaptureServiceLinux () {}
	virtual ~MoonAudioCaptureServiceLinux () {}
	virtual void GetAvailableCaptureDevices (AudioCaptureDeviceCollection *col);
};

/*
 * MoonAudioCaptureDeviceLinux
 */
class MoonAudioCaptureDeviceLinux : public MoonAudioCaptureDevice {
private:
	AudioRecorder *recorder;

public:
	MoonAudioCaptureDeviceLinux (AudioRecorder *recorder);
	virtual ~MoonAudioCaptureDeviceLinux ();

	virtual void GetSupportedFormats (AudioFormatCollection *col);
	virtual const char* GetFriendlyName ();

	virtual void StartCapturing ();
	virtual void StopCapturing ();

	AudioRecorder *GetRecorder () { return recorder; }
};

};

#endif /* MOON_PAL_LINUX_AUDIO_CAPTURE_H */
