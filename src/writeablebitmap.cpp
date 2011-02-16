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

#include <config.h>
#include "runtime.h"
#include "transform.h"
#include "rect.h"
#include "region.h"
#include "panel.h"
#include "writeablebitmap.h"
#include "surface-cairo.h"

namespace Moonlight {

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

	image_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, GetPixelWidth (), GetPixelHeight ());
	cr = cairo_create (image_surface);
	cairo_set_source_surface (cr, source->GetImageSurface (), 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);

	SetBitmapData (cairo_image_surface_get_data (image_surface), false);

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

void
WriteableBitmap::Render (UIElement *element, Transform *transform)
{
	CairoSurface *target;

	if (!element)
		return;

	cairo_surface_t *surface = GetImageSurface ();
	if (!surface)
		return;

	Rect bounds (0, 0, GetPixelWidth (), GetPixelHeight ());

	target = new CairoSurface (surface, bounds.width, bounds.height);

	cairo_matrix_t xform;
	cairo_matrix_init_identity (&xform);

	if (transform)
		transform->GetTransform (&xform);

	FrameworkElement *fe = (FrameworkElement *)element;
	if (fe->GetFlowDirection () == FlowDirectionRightToLeft) {
		cairo_matrix_translate (&xform, fe->GetActualWidth (), 0.0);
		cairo_matrix_scale (&xform, -1, 1);
	}

	element->Paint (target, bounds, &xform);

	target->unref ();
	cairo_surface_flush (surface);
}

};
