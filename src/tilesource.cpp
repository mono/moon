/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * tilesource.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007,2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "tilesource.h"


MultiScaleTileSource::MultiScaleTileSource ()
{
	SetObjectType (Type::MULTISCALETILESOURCE); 
	SetImageWidth (-1);
	get_tile_func = NULL;
	invalidate_cb = NULL;
}

void 
MultiScaleTileSource::set_image_uri_func (get_image_uri_func func)
{
	if (!get_tile_func)
		get_tile_func = func;
	else
		g_warning ("get_tile_func already set\n");
}

void
MultiScaleTileSource::InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer)
{
	if (invalidate_cb)
		invalidate_cb (invalidate_data, level, tilePositionX, tilePositionY, tileLayer);
}

void
MultiScaleTileSource::set_invalidate_tile_layer_func (invalidate_tile_layer_func func, MultiScaleImage *user_data)
{
	invalidate_cb = func;
	invalidate_data = user_data;
}

