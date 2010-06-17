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
	Surface *surface = Deployment::GetCurrent ()->GetSurface ();
	// this must be called from a user initiated event (clicking a
	// button, etc), or else it fails outright.
	if (!surface->IsUserInitiatedEvent ())
		return false;

	MoonWindowingSystem *windowing_system = runtime_get_windowing_system ();

	char *msg = g_strdup_printf ("Allow %s to access your video/audio hardware?",
				     surface->GetSourceLocation());

	MoonMessageBoxResult result = windowing_system->ShowMessageBox (MessageBoxTypeQuestion,
									"Capture access",
									msg,
									MessageBoxButtonYesNo);

	g_free (msg);

	return result == MessageBoxResultYes;
}

