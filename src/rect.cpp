/*
 * rect.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "rect.h"

Rect
rect_from_str (const char *s)
{
	// FIXME - not robust enough for production
	char *next = NULL;
	double x = strtod (s, &next);
	double y = 0.0;
	if (next) {
		++next;
		y = strtod (next, &next);
	}
	double w = 0.0;
	if (next) {
		++next;
		w = strtod (next, &next);
	}
	double h = 0.0;
	if (next) {
		++next;
		h = strtod (next, &next);
	}
	return Rect (x, y, w, h);
}

