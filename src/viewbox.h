/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * viewbox.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_VIEWBOX_H__
#define __MOON_VIEWBOX_H__

#include "frameworkelement.h"

namespace Moonlight {

/* @Namespace=System.Windows.Controls */
/* @ContentProperty=Child */
class Viewbox : public FrameworkElement {
protected:
	virtual ~Viewbox () {}

public:
	/* @GeneratePInvoke,GenerateCBinding */
	Viewbox ();

	/* @PropertyType=UIElement,GenerateAccessors,ManagedFieldAccess=Internal */
	const static int ChildProperty;

	/* @PropertyType=Stretch,DefaultValue=StretchUniform,GenerateAccessors */
	const static int StretchProperty;

	/* @PropertyType=StretchDirection,DefaultValue=StretchDirectionBoth,GenerateAccessors */
	const static int StretchDirectionProperty;

	UIElement *GetChild ();
	void SetChild (UIElement *value);

	Stretch GetStretch ();
	void SetStretch (Stretch value);

	StretchDirection GetStretchDirection ();
	void SetStretchDirection (StretchDirection value);
};

};
#endif /* __MOON_VIEWBOX_H__ */
