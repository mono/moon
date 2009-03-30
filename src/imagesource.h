/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * imagesource.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __IMAGESOURCE_H__
#define __IMAGESOURCE_H__

#include "dependencyobject.h"

/* @Namespace=System.Windows.Media */
class ImageSource : public DependencyObject {
 protected:
	virtual ~ImageSource ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	ImageSource ();

	virtual void Lock ();
	virtual void Unlock ();
	
	virtual gint32 GetPixelWidth ();
	virtual void SetPixelWidth (gint32 width);
	virtual gint32 GetPixelHeight ();
	virtual void SetPixelHeight (gint32 height);
	virtual PixelFormats GetPixelFormat ();
	virtual void SetPixelFormat (PixelFormats weight);

	virtual cairo_surface_t *GetSurface (cairo_t *cr);
};

#endif /* __IMAGESOURCE_H__ */
