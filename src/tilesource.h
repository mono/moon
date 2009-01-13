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

#include <stdio.h>
#include "dependencyobject.h"

G_BEGIN_DECLS

typedef gpointer (*get_image_uri_func) (int level, int posX, int posY);

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
	MultiScaleTileSource ();

	virtual Type::Kind GetObjectType () { return Type::MULTISCALETILESOURCE; }

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	int GetImageWidth ();
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void SetImageWidth (int width);

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	int GetImageHeight ();
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void SetImageHeight (int height);

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	int GetTileWidth ();
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void SetTileWidth (int width);

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	int GetTileHeight ();
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void SetTileHeight (int height);

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	int GetTileOverlap ();
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void SetTileOverlap (int overlap);

	get_image_uri_func get_tile_func;
};

void multi_scale_tile_source_set_image_uri_func (MultiScaleTileSource *instance, get_image_uri_func func);

G_END_DECLS

#endif /* __TILESOURCE_H__ */
