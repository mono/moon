/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bitmapsource.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __BITMAPSOURCE_H__
#define __BITMAPSOURCE_H__

#include <cairo.h>

#include "dependencyobject.h"
#include "imagesource.h"

namespace Moonlight {

/* @Namespace=System.Windows.Media.Imaging */
class BitmapSource : public ImageSource {
 private:
	gpointer data;
	bool own_data; // if true, we free in the dtor.
 protected:
	cairo_surface_t *image_surface;

	/* @GeneratePInvoke,ManagedAccess=Protected */
	BitmapSource ();

	virtual ~BitmapSource ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=gint32,DefaultValue=0,ManagedSetterAccess=Internal,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	const static int PixelWidthProperty;
	/* @PropertyType=gint32,DefaultValue=0,ManagedSetterAccess=Internal,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	const static int PixelHeightProperty;

	/* @GenerateManagedEvent=false */
	const static int PixelDataChangedEvent;

	virtual gint32 GetPixelWidth ();
	virtual void SetPixelWidth (gint32 width);
	virtual gint32 GetPixelHeight ();
	virtual void SetPixelHeight (gint32 height);

	/* @GeneratePInvoke */
	gpointer GetBitmapData ();
	/* @GeneratePInvoke */
	void SetBitmapData (gpointer data, bool own);
	
	/* @GeneratePInvoke */
	virtual void Invalidate ();
	virtual cairo_surface_t *GetSurface (cairo_t *cr);

	cairo_surface_t *GetImageSurface () { return image_surface; }
	virtual void OnIsAttachedChanged (bool value);
};

};

#endif /* __BITMAPSOURCE_H__ */
