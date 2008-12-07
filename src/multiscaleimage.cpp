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

Point
MultiScaleImage::ElementToLogicalPoint (Point elementPoint)
{
	//FIXME
	return Point (0, 0);
}

void
MultiScaleImage::RenderLayer (cairo_t *cr, MultiScaleTileSource *source, int layer)
{
	printf ("rendering layer %d\n", layer);

	double w = GetWidth ();
	double h = GetHeight ();
	int vp_w = GetViewportWidth ();
	int vp_h = (int)vp_w * GetAspectRatio ();
	int tile_width = source->GetTileWidth ();
	int tile_height = source->GetTileHeight ();
	int vp_ox = GetViewportOrigin()->x;
	int vp_oy = GetViewportOrigin()->y;

	int levels = 13;

	double v_tile_w = tile_width * ldexp (1.0, 13 - layer);
	double v_tile_h = tile_height * ldexp (1.0, 13 - layer);
	double zoom = w / (double)vp_w;

	int i,j;
	double y0 = 0;
	double x0;
	for (j = (int)((double)vp_oy / (double)v_tile_h); (j+1) * v_tile_h < vp_oy + vp_h; j++) {
		x0 = 0;
		for (i = (int)((double)vp_ox / (double)v_tile_w); (i+1) * v_tile_w < vp_ox + vp_w; i++) {
			printf ("drawing %f %f %f %f\n", x0, y0,
					((double)(i + 1) * v_tile_w - (double)vp_ox) * zoom,
					((double)(j + 1) * v_tile_h - (double)vp_oy) * zoom);
			cairo_rectangle (cr, x0, y0,
					((double)(i + 1) * v_tile_w - (double)vp_ox) * zoom,
					((double)(j + 1) * v_tile_h - (double)vp_oy) * zoom);
			if (i%2 == j%2)
				cairo_set_source_rgba (cr, 1, 0, 0, .2);
			else
				cairo_set_source_rgba (cr, 0, 0, 1, .2);
			cairo_fill (cr);
			x0 = ((double)(i+1) * v_tile_w - (double)vp_ox)* zoom;
		}
		printf ("drawing %f %f %f %f\n", x0, y0, w,
				((double)(j + 1) * v_tile_h - (double)vp_oy) * zoom);
		cairo_rectangle (cr, x0, y0, w,
				((double)(j + 1) * v_tile_h - (double)vp_oy) * zoom);
		if (i%2 == j%2)
			cairo_set_source_rgba (cr, 1, 0, 0, .2);
		else
			cairo_set_source_rgba (cr, 0, 0, 1, .2);
		cairo_fill (cr);

		y0 = ((double)(j+1) * v_tile_h - (double)vp_oy) * zoom;
	}
	x0 = 0;
	for (i = (int)((double)vp_ox / (double)v_tile_w); (i+1) * v_tile_w < vp_ox + vp_w; i++) {
		printf ("drawing %f %f %f %f\n", x0, y0,
				((double)(i + 1) * v_tile_w - (double)vp_ox) * zoom,
				h);
		cairo_rectangle (cr, x0, y0,
				((double)(i + 1) * v_tile_w - (double)vp_ox) * zoom,
				h);
		if (i%2 == j%2)
			cairo_set_source_rgba (cr, 1, 0, 0, .2);
		else
			cairo_set_source_rgba (cr, 0, 0, 1, .2);
		cairo_fill (cr);
		x0 = ((double)(i+1) * v_tile_w - (double)vp_ox)* zoom;
	}
	printf ("drawing %f %f %f %f\n", x0, y0, w, h);
	cairo_rectangle (cr, x0, y0, w, h);
	if (i%2 == j%2)
		cairo_set_source_rgba (cr, 1, 0, 0, .2);
	else
		cairo_set_source_rgba (cr, 0, 0, 1, .2);
	cairo_fill (cr);

	y0 = ((double)(j+1) + v_tile_h - (double)vp_oy) * zoom;
}

void
MultiScaleImage::Render (cairo_t *cr, Region *region)
{
printf ("MSI::Render\n");

//	if (!surface)
//		return;

	MultiScaleTileSource* source;
	if (!(source = GetSource ()) || source->GetImageWidth () < 0) {
		printf ("no sources set, nothing to render\n");
		return;
	}

	int i;
	for (i = 12; i <= 12; i++)
		RenderLayer (cr, source, i);
//
	double w = GetWidth ();
	double h = GetHeight ();
	cairo_rectangle (cr, 0, .5*h, .5*w, .5*h);
	cairo_set_source_rgba (cr, 0, 1, 0, 1);
	cairo_fill (cr);
	
	cairo_rectangle (cr, .5*w, 0, .5*w, .5*h);
	cairo_set_source_rgba (cr, 0, 0, 1, 1);
	cairo_fill (cr);

	cairo_rectangle (cr, .5*w, .5*h, .5*w, .5*h);
	cairo_set_source_rgba (cr, 0, 0, 0, 1);
	cairo_fill (cr);

}

