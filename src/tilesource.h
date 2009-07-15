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

/* @CBindingRequisite */
typedef bool (*get_image_uri_func) (int level, int posX, int posY, Uri *uri, void* user_data);

typedef void (*invalidate_tile_layer_func) (int level, int tilePositionX, int tilePositionY, int tileLayer);

/* @Version=2,Namespace=System.Windows.Media */
/* @CallInitialize */
class MultiScaleTileSource : public DependencyObject {
 protected:
	long imageWidth; //width of the DeepZoom Image
	long imageHeight;
	int tileWidth;	//width of the tiles
	int tileHeight;
	int tileOverlap; //how much the tiles overlap

	virtual ~MultiScaleTileSource () {}

	invalidate_tile_layer_func invalidate_cb;

 public:
	get_image_uri_func get_tile_func;

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	MultiScaleTileSource ();

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	long GetImageWidth ();
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void SetImageWidth (long width);

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	long GetImageHeight ();
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void SetImageHeight (long height);

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

	/* @GenerateCBinding,GeneratePInvoke */
	void set_image_uri_func (get_image_uri_func func);

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	void InvalidateTileLayer (int level, int tilePositionX, int tilePositionY, int tileLayer);

	void set_invalidate_tile_layer_func (invalidate_tile_layer_func func);
};

#endif /* __TILESOURCE_H__ */
