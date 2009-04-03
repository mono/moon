/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * writeablebitmap.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __WRITEABLEBITMAP_H__
#define __WRITEABLEBITMAP_H__

#include "dependencyobject.h"
#include "bitmapsource.h"

/* @Namespace=System.Windows.Media.Imaging */
class WriteableBitmap : public BitmapSource {
 private:
	pthread_mutex_t surface_mutex;

 protected:
	virtual ~WriteableBitmap ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	WriteableBitmap ();

	/* @GenerateCBinding,GeneratePInvoke */
	gpointer InitializeFromBitmapSource (BitmapSource *source);

	/* @GenerateCBinding,GeneratePInvoke */
	virtual void Render (UIElement *element, Transform *transform);
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void Lock ();
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void Unlock ();

	virtual cairo_surface_t *GetSurface (cairo_t *cr);
};

#endif /* __WRITEABLEBITMAP_H__ */
