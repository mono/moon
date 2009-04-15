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
#include "runtime.h"

BitmapSource::BitmapSource ()
{
	SetObjectType (Type::BITMAPSOURCE);
	image_surface = NULL;
	native_surface = NULL;
	data = NULL;
}

BitmapSource::~BitmapSource ()
{
	if (image_surface)
		cairo_surface_destroy (image_surface);
	if (native_surface)
		cairo_surface_destroy (native_surface);

	if (this->data)
		g_free (this->data);
}

gpointer
BitmapSource::GetBitmapData ()
{
	return data;
}

void
BitmapSource::SetBitmapData (gpointer data)
{
	if (this->data)
		g_free (this->data);
	this->data = data;
}

void
BitmapSource::Invalidate ()
{
	if (GetPixelWidth () == 0 || GetPixelHeight () == 0)
		return;

	if (native_surface) {
		cairo_surface_destroy (native_surface);
		native_surface = NULL;
	}
	if (image_surface)
		cairo_surface_destroy (image_surface);

	image_surface = cairo_image_surface_create_for_data ((unsigned char *) GetBitmapData (), GetPixelFormat () == PixelFormatBgr32 ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_ARGB32, GetPixelWidth (), GetPixelHeight (), GetPixelWidth ()*4);
}

cairo_surface_t *
BitmapSource::GetSurface (cairo_t *cr)
{
	if (image_surface == NULL)
		return NULL;

	if (native_surface)
		return native_surface;
	
	if (cr == NULL)
		return image_surface;

	native_surface = cairo_surface_create_similar (cairo_get_target (cr), CAIRO_CONTENT_COLOR_ALPHA, GetPixelWidth (), GetPixelHeight ());
	cairo_t *context = cairo_create (native_surface);

	cairo_set_source_surface (context, image_surface, 0, 0);
	cairo_pattern_set_filter (cairo_get_source (context), CAIRO_FILTER_FAST);

	cairo_paint (context);
	cairo_destroy (context);

	return native_surface;
}
