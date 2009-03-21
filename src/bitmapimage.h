/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bitmapimage.h
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

/* @Namespace=System.Windows.Media */
class ImageSource : public DependencyObject {
 public:
	virtual void SetUriSource (Uri *uri) = 0;
	virtual Uri* GetUriSource () = 0;
};

/* @Namespace=System.Windows.Media.Imaging */
class BitmapImage : public ImageSource {
 protected:
	virtual ~BitmapImage ();

 public:
	gpointer buffer;
	gint32 size;

	/* @PropertyType=Uri,GenerateAccessors,DefaultValue=Uri() */
	const static int UriSourceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	BitmapImage ();

	/* @GenerateCBinding,GeneratePInvoke */
	void SetBuffer (gpointer buffer, int size);

	void CleanUp ();

	void SetUriSource (Uri* value);
	Uri* GetUriSource ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
};

#endif /* __BITMAPIMAGE_H__ */
