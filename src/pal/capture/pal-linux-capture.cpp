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
#include "pal-linux-audio-capture.h"
#include "deployment.h"
#include "runtime.h"

#if PAL_V4L2_VIDEO_CAPTURE
#include "pal/capture/v4l2/pal-v4l2-video-capture.h"
#endif

using namespace Moonlight;

MoonCaptureServiceLinux::MoonCaptureServiceLinux ()
{
#if PAL_V4L2_VIDEO_CAPTURE
	video_service = new MoonVideoCaptureServiceV4L2 ();
#else
	video_service = NULL;
#endif
	audio_service = new MoonAudioCaptureServiceLinux ();
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
