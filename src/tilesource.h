/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * tilesource.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __TILESOURCE_H__
#define __TILESOURCE_H__

#include "dependencyobject.h"

/* @Version=2,Namespace=System.Windows.Media */
class MultiScaleTileSource : public DependencyObject {
 protected:
	int imageWidth; //width of the DeepZoom Image
	int imageHeight;
	int tileWidth;	//width of the tiles
	int tileHeight;
	int tileOverlap; //how much the tiles overlap

	virtual ~MultiScaleTileSource () {}

 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	MultiScaleTileSource () {}

	virtual Type::Kind GetObjectType () { return Type::MULTISCALETILESOURCE; }

};

#endif /* __TILESOURCE_H__ */
