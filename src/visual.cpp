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
#include "runtime.h"

bool
Visual::InsideObject (cairo_t *cr, double x, double y)
{
	printf ("Visual subclass '%s' should implement ::InsideObject\n", GetTypeName ());
	return false;
}

void
visual_set_surface (Visual* visual, Surface* surface)
{
	if (visual)
		visual->SetSurface (surface);
}

TimeManager*
Visual::GetTimeManager ()
{
	Surface *surface = GetSurface ();
	return surface ? surface->GetTimeManager() : NULL;
}
