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

#include <config.h>

#include <stdio.h>

#include "application.h"
#include "bitmapsource.h"
#include "runtime.h"

namespace Moonlight {

BitmapSource::BitmapSource ()
{
	SetObjectType (Type::BITMAPSOURCE);
	image_surface = NULL;
	native_surface = NULL;
	data = NULL;
	own_data = true;
}

BitmapSource::~BitmapSource ()
{
	if (image_surface)
		cairo_surface_destroy (image_surface);
	if (native_surface)
		cairo_surface_destroy (native_surface);

	if (data && own_data)
		g_free (data);
}

gpointer
BitmapSource::GetBitmapData ()
{
	return data;
}

void
BitmapSource::OnIsAttachedChanged (bool value)
{
	ImageSource::OnIsAttachedChanged (value);

	if (!value) {
		if (native_surface) {
			cairo_surface_destroy (native_surface);
			native_surface = NULL;
		}
	} else {
		if (native_surface == NULL && image_surface == NULL) {
			// #246 requires this - it detaches and then reattaches
			// glyphs, and without this they don't show up again.
			Invalidate ();
		}
	}
}

void
BitmapSource::SetBitmapData (gpointer data, bool own_data)
{
	if (data == NULL) {
		SetPixelWidth (0.0);
		SetPixelHeight (0.0);
	}

	if (this->data && this->own_data)
		g_free (this->data);
	this->own_data = own_data;
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

	image_surface = cairo_image_surface_create_for_data ((unsigned char *) GetBitmapData (), CAIRO_FORMAT_ARGB32, 
		GetPixelWidth (), GetPixelHeight (), GetPixelWidth () * 4);

	Emit (BitmapSource::PixelDataChangedEvent);
}

cairo_surface_t *
BitmapSource::GetSurface (cairo_t *cr)
{
	return image_surface;
}

};
