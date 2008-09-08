/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * multiscaleimage.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "multiscaleimage.h"

MultiScaleImage::MultiScaleImage ()
{

}

MultiScaleImage::~MultiScaleImage ()
{

}

void
MultiScaleImage::ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY)
{

}

void
MultiScaleImage::Render (cairo_t *cr, Region *region)
{

//	if (!surface)
//		return;

	double w = GetWidth ();
	double h = GetHeight ();

	cairo_rectangle (cr, 0, 0, .5 * w, .5*h);
	cairo_set_source_rgba (cr, 1, 0, 0, 1);
	cairo_fill (cr);
	
	cairo_rectangle (cr, 0, .5*h, .5*w, .5*h);
	cairo_set_source_rgba (cr, 0, 1, 0, 1);
	cairo_fill (cr);
	
	cairo_rectangle (cr, .5*w, 0, .5*w, .5*h);
	cairo_set_source_rgba (cr, 0, 0, 1, 1);
	cairo_fill (cr);

	cairo_rectangle (cr, .5*w, .5*h, .5*w, .5*h);
	cairo_set_source_rgba (cr, 0, 0, 0, 1);
	cairo_fill (cr);

//	if (create_xlib_surface && !surface->xlib_surface_created) {
//		surface->xlib_surface_created = true;
//		
//		cairo_surface_t *xlib_surface = image_brush_create_similar (cr, surface->width, surface->height);
//		cairo_t *cr = cairo_create (xlib_surface);
//
//		cairo_set_source_surface (cr, surface->cairo, 0, 0);
//
//		//cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
//		cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_FAST);
//
//		cairo_paint (cr);
//		cairo_destroy (cr);
//
//		cairo_surface_destroy (surface->cairo);
//
//		if (surface->backing_pixbuf) {
//			g_object_unref (surface->backing_pixbuf);
//			surface->backing_pixbuf = NULL;
//		}
//
//		if (surface->backing_data) {
//			g_free (surface->backing_data);
//			surface->backing_data =NULL;
//		}
//
//		surface->cairo = xlib_surface;
//	}

//	cairo_save (cr);
//
//	Stretch stretch = GetStretch ();
//	double h = GetHeight ();
//	double w = GetWidth ();
//	
//	if (!pattern)
//		pattern = cairo_pattern_create_for_surface (surface->cairo);
//	
//	cairo_matrix_t matrix;
//	
//	image_brush_compute_pattern_matrix (&matrix, w, h, surface->width, surface->height, stretch, 
//					    AlignmentXCenter, AlignmentYCenter, NULL, NULL);
//	
//	cairo_pattern_set_matrix (pattern, &matrix);
//	cairo_set_source (cr, pattern);
//
//	cairo_set_matrix (cr, &absolute_xform);
//	
//	cairo_rectangle (cr, 0, 0, w, h);
//	cairo_fill (cr);
//
//	cairo_restore (cr);
	
}

