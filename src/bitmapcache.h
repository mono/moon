/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __MOONLIGHT_BITMAPCACHE_H__
#define __MOONLIGHT_BITMAPCACHE_H__

#include <glib.h>
#include "enums.h"
#include "dependencyobject.h"

/* @Namespace=System.Windows.Media */
class CacheMode : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	CacheMode ();

protected:
	virtual ~CacheMode ();
};

/* @Namespace=System.Windows.Media */
class BitmapCache : public CacheMode {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	BitmapCache ();

	/* @PropertyType=double,DefaultValue=1.0,Validator=DoubleGreaterThanZeroValidator,GenerateAccessors */
	const static int RenderAtScaleProperty;

	double GetRenderAtScale ();
	void SetRenderAtScale (double scale);

protected:
	virtual ~BitmapCache ();
};

#endif /* __MOONLIGHT_BITMAPCACHE_H__ */

