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

#ifdef USE_GALLIUM
#define __MOON_GALLIUM__
#include "context-gallium.h"
extern "C" {
#include "pipe/p_screen.h"
#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"
#include "util/u_debug.h"
#define template templat
#include "state_tracker/sw_winsys.h"
#include "sw/null/null_sw_winsys.h"
#include "softpipe/sp_public.h"
#ifdef USE_LLVM
#include "llvmpipe/lp_public.h"
#endif
};

static struct pipe_screen *
swrast_screen_create (struct sw_winsys *ws)
{
	const char         *default_driver;
	const char         *driver;
	struct pipe_screen *screen = NULL;

#ifdef USE_LLVM
	default_driver = "llvmpipe";
#else
	default_driver = "softpipe";
#endif

	driver = debug_get_option ("GALLIUM_DRIVER", default_driver);

#ifdef USE_LLVM
	if (screen == NULL && strcmp (driver, "llvmpipe") == 0)
		screen = llvmpipe_create_screen (ws);
#endif

	if (screen == NULL)
		screen = softpipe_create_screen (ws);

	return screen;
}
#endif

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
	MoonSurface *src;
	Context     *ctx;

	if (!element)
		return;

	cairo_surface_t *surface = GetImageSurface ();
	if (!surface)
		return;

	Rect bounds (0, 0, GetPixelWidth (), GetPixelHeight ());

#ifdef USE_GALLIUM
	struct pipe_resource pt, *texture;
	GalliumSurface       *target;
	struct pipe_screen   *screen =
		swrast_screen_create (null_sw_create ());

	memset (&pt, 0, sizeof (pt));
	pt.target = PIPE_TEXTURE_2D;
	pt.format = PIPE_FORMAT_B8G8R8A8_UNORM;
	pt.width0 = 1;
	pt.height0 = 1;
	pt.depth0 = 1;
	pt.last_level = 0;
	pt.bind = PIPE_BIND_RENDER_TARGET | PIPE_BIND_TRANSFER_WRITE |
		PIPE_BIND_TRANSFER_READ;

	texture = (*screen->resource_create) (screen, &pt);

	target = new GalliumSurface (texture);
	pipe_resource_reference (&texture, NULL);
	ctx = new GalliumContext (target);
	target->unref ();

	ctx->Push (Context::Group (bounds));
#else
	CairoSurface *target;

	target = new CairoSurface (surface, bounds.width, bounds.height);
	ctx = new CairoContext (target);
	target->unref ();

	ctx->Push (Context::Clip (bounds));
#endif

	cairo_matrix_t xform;
	cairo_matrix_init_identity (&xform);

	if (transform)
		transform->GetTransform (&xform);

	FrameworkElement *fe = (FrameworkElement *)element;
	if (fe->GetFlowDirection () == FlowDirectionRightToLeft) {
		cairo_matrix_translate (&xform, fe->GetActualWidth (), 0.0);
		cairo_matrix_scale (&xform, -1, 1);
	}

	element->Paint (ctx, bounds, &xform);

	bounds = ctx->Pop (&src);
	if (!bounds.IsEmpty ()) {
		cairo_surface_t *image = src->Cairo ();
		cairo_t         *cr = cairo_create (surface);

		cairo_set_source_surface (cr, image, 0, 0);
		cairo_paint (cr);
		cairo_destroy (cr);
		cairo_surface_destroy (image);
		src->unref ();
	}

	delete ctx;

#ifdef USE_GALLIUM
	screen->destroy (screen);
#endif

}

};
