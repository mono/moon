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
	virtual ~MultiScaleTileSource () {}

 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	MultiScaleTileSource () {}

	virtual Type::Kind GetObjectType () { return Type::MULTISCALETILESOURCE; }
};

/* @Version=2,Namespace=System.Windows.Media */
class DeepZoomImageTileSource : public MultiScaleTileSource {
 protected:
	virtual ~DeepZoomImageTileSource () {}

 public:
	/* @PropertyType=string,ManagedPropertyType=Uri */
	static DependencyProperty *UriSourceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	DeepZoomImageTileSource () {}

	virtual Type::Kind GetObjectType () { return Type::DEEPZOOMIMAGETILESOURCE; }	
};

#endif /* __TILESOURCE_H__ */
