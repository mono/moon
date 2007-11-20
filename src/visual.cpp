/*
 * visual.cpp
 *
 * Author:
 *   Chris Toshok (toshok@ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdio.h>
#include <glib.h>

#include "visual.h"


bool
Visual::InsideObject (cairo_t *cr, double x, double y)
{
	printf ("Visual subclass '%s' should implement ::InsideObject\n", GetTypeName ());
	return false;
}

void
Visual::SetSurface (Surface *surface)
{
	this->surface = surface;
}

void
visual_set_surface (Visual* visual, Surface* surface)
{
	if (visual)
		visual->SetSurface (surface);
}
