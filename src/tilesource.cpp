/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * tilesource.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "tilesource.h"

MultiScaleTileSource::MultiScaleTileSource ()
{
	imageWidth = -1;
}

int 
MultiScaleTileSource::GetImageWidth ()
{
	return imageWidth;
}

void
MultiScaleTileSource::SetImageWidth (int width)
{
	imageWidth = width;
}

int 
MultiScaleTileSource::GetImageHeight ()
{
	return imageHeight;
}

void
MultiScaleTileSource::SetImageHeight (int height)
{
	imageHeight = height;
}

int
MultiScaleTileSource::GetTileWidth ()
{
	return tileWidth;
}

void 
MultiScaleTileSource::SetTileWidth (int width)
{
	tileWidth = width;
}

int 
MultiScaleTileSource::GetTileHeight ()
{
	return tileHeight;
}

void 
MultiScaleTileSource::SetTileHeight (int height)
{
	tileHeight = height;
}

int 
MultiScaleTileSource::GetTileOverlap ()
{
	return tileOverlap;
}

void 
MultiScaleTileSource::SetTileOverlap (int overlap)
{
	tileOverlap = overlap;
}


