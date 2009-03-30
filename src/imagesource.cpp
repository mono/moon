/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bitmapsource.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>

#include "application.h"
#include "bitmapsource.h"

ImageSource::ImageSource ()
{
	SetObjectType (Type::IMAGESOURCE);
}

ImageSource::~ImageSource ()
{
}

void
ImageSource::Lock ()
{
}

void
ImageSource::Unlock ()
{
}

gint32
ImageSource::GetPixelWidth ()
{
	return 0;
}

void
ImageSource::SetPixelWidth (gint32 width)
{
}

gint32
ImageSource::GetPixelHeight ()
{
	return 0;
}

void
ImageSource::SetPixelHeight (gint32 height)
{
}

PixelFormats 
ImageSource::GetPixelFormat ()
{
	return PixelFormatBgr32;
}

void
ImageSource::SetPixelFormat (PixelFormats weight)
{
}

cairo_surface_t *
ImageSource::GetSurface (cairo_t *cr)
{
	return NULL;
}
