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


CacheMode::CacheMode ()
{
	SetObjectType (Type::CACHEMODE);
}

CacheMode::~CacheMode ()
{
}

BitmapCache::BitmapCache ()
{
	SetObjectType (Type::BITMAPCACHE);
}

BitmapCache::~BitmapCache ()
{
}
