/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * writeablebitmap.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "runtime.h"
#include "transform.h"
#include "rect.h"
#include "region.h"
#include "panel.h"
#include "writeablebitmap.h"

WriteableBitmap::WriteableBitmap ()
{
	SetObjectType (Type::WRITEABLEBITMAP);
	pthread_mutex_init (&surface_mutex, NULL);
}

gpointer
WriteableBitmap::InitializeFromBitmapSource (BitmapSource *source)
{
	cairo_t *cr;

	if (!source)
		return NULL;

	SetPixelHeight (source->GetPixelHeight ());
	SetPixelWidth (source->GetPixelWidth ());
	SetPixelFormat (PixelFormatPbgra32);

	image_surface = cairo_image_surface_create (GetPixelFormat () == PixelFormatBgr32 ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_ARGB32, GetPixelWidth (), GetPixelHeight ());
	cr = cairo_create (image_surface);
	cairo_set_source_surface (cr, source->GetImageSurface (), 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);

	SetBitmapData (cairo_image_surface_get_data (image_surface));

	return GetBitmapData ();
}

WriteableBitmap::~WriteableBitmap ()
{
	pthread_mutex_destroy (&surface_mutex);
}

void
WriteableBitmap::Lock ()
{
	pthread_mutex_lock (&surface_mutex);
}

void
WriteableBitmap::Unlock ()
{
	pthread_mutex_unlock (&surface_mutex);
}

cairo_surface_t *
WriteableBitmap::GetSurface (cairo_t *cr)
{
	return image_surface;
}

void
WriteableBitmap::Render (UIElement *element, Transform *transform)
{
	cairo_t *cr;
	Region *region;
	Rect bounds;

	if (!element)
		return;

	if (!GetSurface (NULL)) {
		Invalidate ();
	}

        cr = cairo_create (GetSurface (NULL));

	// Region is the entire WB size
	region = new Region (Rect (0, 0, GetPixelWidth (), GetPixelHeight ()));

	// FIXME is this supposed to clear the surface?
	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint (cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	cairo_matrix_t xform;
	if (transform)
		transform->GetTransform (&xform);

       	element->Paint (cr, region, &xform);

	cairo_destroy (cr);
	cairo_surface_flush (image_surface);
}
