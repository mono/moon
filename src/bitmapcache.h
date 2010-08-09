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

namespace Moonlight {

/* @Namespace=System.Windows.Media */
class CacheMode : public DependencyObject {
protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	CacheMode ();

	virtual ~CacheMode ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

public:
	virtual void GetTransform (cairo_matrix_t *transform);
};

/* @Namespace=System.Windows.Media */
class BitmapCache : public CacheMode {
public:
	/* @PropertyType=double,DefaultValue=1.0,Validator=DoubleGreaterThanZeroValidator,GenerateAccessors */
	const static int RenderAtScaleProperty;

	double GetRenderAtScale ();
	void SetRenderAtScale (double scale);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	BitmapCache ();

	virtual ~BitmapCache ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

public:
	void GetTransform (cairo_matrix_t *transform);
};

};

#endif /* __MOONLIGHT_BITMAPCACHE_H__ */
