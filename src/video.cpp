/*
 * video.c: The moonlight's video item
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <stdlib.h>
#include "runtime.h"

Video::Video (const char *filename, double x, double y)
{
	this->filename = g_strdup (filename);
	this->x = x;
	this->y = y;
}

Video::~Video ()
{
	g_free (filename);
}

