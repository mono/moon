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

#include "pal/capture/v4l2/pal-v4l2-video-capture.h"

MoonVideoCaptureServiceV4L2::MoonVideoCaptureServiceV4L2 ()
{
}

MoonVideoCaptureServiceV4L2::~MoonVideoCaptureServiceV4L2 ()
{
}

MoonVideoCaptureDevice*
MoonVideoCaptureServiceV4L2::GetDefaultCaptureDevice ()
{
	return NULL;
}

List*
MoonVideoCaptureServiceV4L2::GetAvailableCaptureDevices ()
{
	return NULL;
}
