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

#include <config.h>
#include "tilesource.h"

namespace Moonlight {

TileLayerInvalidatedEventArgs::TileLayerInvalidatedEventArgs (int level, int x, int y, int layer)
{
	this->level = level;
	this->x = x;
	this->y = y;
	this->layer = layer;
}

MultiScaleTileSource::MultiScaleTileSource ()
{
	SetObjectType (Type::MULTISCALETILESOURCE); 
	SetImageWidth (-1);
	get_tile_func = NULL;
}

void
MultiScaleTileSource::set_image_uri_func (get_image_uri_func func)
{
	get_tile_func = func;
}

void
MultiScaleTileSource::InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer)
{
	if (HasHandlers (TileLayerInvalidatedEvent))
		Emit (TileLayerInvalidatedEvent, new TileLayerInvalidatedEventArgs (level, tilePositionX, tilePositionY, tileLayer));
}

};
