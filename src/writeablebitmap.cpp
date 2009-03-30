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

#include "writeablebitmap.h"

WriteableBitmap::WriteableBitmap ()
{
	SetObjectType (Type::WRITEABLEBITMAP);
	pthread_mutex_init (&surface_mutex, NULL);
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
