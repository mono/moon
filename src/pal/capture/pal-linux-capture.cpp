/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-linux-capture.cpp:
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include "pal-linux-capture.h"
#include "deployment.h"
#include "runtime.h"
#include "consent.h"

#if PAL_V4L2_VIDEO_CAPTURE
#include "pal/capture/v4l2/pal-v4l2-video-capture.h"
#endif

MoonCaptureServiceLinux::MoonCaptureServiceLinux ()
{
	// FIXME we should do both compile time and runtime checking
	// for this.

#if PAL_V4L2_VIDEO_CAPTURE
	video_service = new MoonVideoCaptureServiceV4L2 ();
#else
	printf ("Moonlight: no video capture service available\n");
	video_service = NULL;
#endif

#if PAL_PULSE_AUDIO_CAPTURE
	audio_service = new MoonAudioCaptureServicePulse ();
#else
	printf ("Moonlight: no audio capture service available\n");
	audio_service = NULL;
#endif
}

MoonCaptureServiceLinux::~MoonCaptureServiceLinux ()
{
	delete video_service;
	delete audio_service;
}

MoonVideoCaptureService* 
MoonCaptureServiceLinux::GetVideoCaptureService()
{
	return video_service;
}

MoonAudioCaptureService*
MoonCaptureServiceLinux::GetAudioCaptureService()
{
	return audio_service;
}

bool
MoonCaptureServiceLinux::RequiresSystemPermissionForDeviceAccess ()
{
	return false;
}

bool
MoonCaptureServiceLinux::RequestSystemAccess ()
{
	// this must be called from a user initiated event (clicking a
	// button, etc), or else it fails outright.
	if (!Deployment::GetCurrent ()->GetSurface ()->IsUserInitiatedEvent ())
		return false;

	return Consent::PromptUserFor (MOON_CONSENT_VIDEO_CAPTURE);
}

