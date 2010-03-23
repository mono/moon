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
	friend void multi_scale_image_handle_parsed (void *userdata);

	MultiScaleTileSource *source;

	int id, n;

	double GetViewportHeight ();

 protected:
	virtual ~MultiScaleSubImage () {}

 public:
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,ReadOnly,GenerateGetter */
	const static int AspectRatioProperty;
 	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateAccessors */
	const static int OpacityProperty;
	/* @PropertyType=Point,DefaultValue=Point(0\,0),Version=2.0,GenerateAccessors */
	const static int ViewportOriginProperty;
	/* @PropertyType=double,DefaultValue=1.0,Version=2.0,GenerateAccessors */
	const static int ViewportWidthProperty;
	/* @PropertyType=gint32,DefaultValue=0,Version=2.0,GenerateAccessors */
	const static int ZIndexProperty;

	
	/* @GenerateCBinding,GeneratePInvoke */
	MultiScaleSubImage ();
	MultiScaleSubImage (const Uri* parent_uri, MultiScaleTileSource *source, int id, int n);

	double GetAspectRatio ();
	void SetAspectRatio (double ratio);

	double GetOpacity ();
	void SetOpacity (double ratio);

	Point* GetViewportOrigin ();
	void SetViewportOrigin (Point* point);

	double GetViewportWidth ();
	void SetViewportWidth (double width);

	/* @GenerateCBinding */
	gint32 GetZIndex ();
	void SetZIndex (gint32 zindex);

	int GetId () {return id;};
};

#endif /* __MULTISCALESUBIMAGE_H__ */
