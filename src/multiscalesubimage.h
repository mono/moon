/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * multiscalesubimage.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MULTISCALESUBIMAGE_H__
#define __MULTISCALESUBIMAGE_H__

#include "dependencyobject.h"

/* @Version=2,Namespace=System.Windows.Controls */
class MultiScaleSubImage : public DependencyObject {
	friend class MultiScaleImage;

	MultiScaleTileSource *source;
	MultiScaleSubImage (MultiScaleTileSource *source);

 protected:
	virtual ~MultiScaleSubImage () {}

 public:
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,ReadOnly,GenerateGetter */
	static DependencyProperty *AspectRatioProperty;
 	/* @PropertyType=double,Version=2.0,GenerateAccessors */
	static DependencyProperty *OpacityProperty;
	/* @PropertyType=Point,Version=2.0,GenerateAccessors */
	static DependencyProperty *ViewportOriginProperty;
	/* @PropertyType=double,Version=2.0,GenerateAccessors */
	static DependencyProperty *ViewportWidthProperty;
	/* @PropertyType=gint32,Version=2.0,GenerateAccessors */
	static DependencyProperty *ZIndexProperty;

	
	/* @GenerateCBinding,GeneratePInvoke */
	MultiScaleSubImage ();

	double GetAspectRatio ();
	void SetAspectRatio (double ratio);

	double GetOpacity ();
	void SetOpacity (double ratio);

	Point* GetViewportOrigin ();
	void SetViewportOrigin (Point* point);

	double GetViewportWidth ();
	void SetViewportWidth (double width);

	gint32 GetZIndex ();
	void SetZIndex (gint32 zindex);

};

#endif /* __MULTISCALESUBIMAGE_H__ */
