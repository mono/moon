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

#ifndef __BITMAPIMAGE_H__
#define __BITMAPIMAGE_H__

#include "dependencyobject.h"

/* @Version=2,Namespace=System.Windows.Media.Imaging */
class BitmapImage : public DependencyObject {
 protected:
	virtual ~BitmapImage () {}

 public:
	/* @PropertyType=string,ManagedPropertyType=Uri */
	static DependencyProperty *UriSourceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	BitmapImage () {}

	virtual Type::Kind GetObjectType () { return Type::BITMAPIMAGE; }	
};

#endif /* __BITMAPIMAGE_H__ */
