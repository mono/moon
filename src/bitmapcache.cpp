/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <cairo.h>
#include <glib.h>

#include "bitmapcache.h"

namespace Moonlight {

CacheMode::CacheMode ()
{
	SetObjectType (Type::CACHEMODE);
}

CacheMode::~CacheMode ()
{
}

void
CacheMode::GetTransform (cairo_matrix_t *transform)
{
	g_warning ("CacheMode::GetTransform has been called. The derived class should have overridden it.");
}

BitmapCache::BitmapCache ()
{
	SetObjectType (Type::BITMAPCACHE);
}

BitmapCache::~BitmapCache ()
{
}

void
BitmapCache::GetTransform (cairo_matrix_t *transform)
{
	double scale = GetRenderAtScale ();

	cairo_matrix_init_scale (transform, scale, scale);
}

};
