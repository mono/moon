/*
 * brush.cpp: Brushes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#define __STDC_CONSTANT_MACROS
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "runtime.h"

struct Consumer {
	BrushChangedNotify notify;
	void *data;
};

void *
Brush::AddListener (BrushChangedNotify notify, void *data)
{
	Consumer *c = new Consumer;
	c->notify = notify;
	c->data = data;

	listeners = g_slist_append (listeners, c);
	return c;
}

void
Brush::RemoveListener (void *listener)
{
	listeners = g_slist_remove (listeners, listener);
	delete (Brush *)listener;
}

void
SolidColorBrush::SetupBrush (cairo_t *target)
{
	cairo_set_source_rgba (target, color.r, color.g, color.b, color.a);
}
