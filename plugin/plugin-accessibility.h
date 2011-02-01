/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-accessibility.h: DO subclass specifically for use as
 * content.accessibility.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef MOON_PLUGIN_ACCESSIBILITY
#define MOON_PLUGIN_ACCESSIBILITY

#include "moonbuild.h"
#include "dependencyobject.h"

/* @Namespace=None */
/* @ManagedDependencyProperties=Manual */
/* @ManagedEvents=Manual */
class MOON_API Accessibility : public DependencyObject {
public:
 	/* @PropertyType=string,DefaultValue=\"Silverlight Content\" */
	const static int TitleProperty;
 	/* @PropertyType=string,DefaultValue=\"\" */
	const static int DescriptionProperty;
 	/* @PropertyType=string,DefaultValue=\"\" */
	const static int ActionDescriptionProperty;

	Accessibility ();

	void PerformAction ();

	// Events you can AddHandler to
	const static int PerformActionEvent;

protected:
	virtual ~Accessibility ();
};

#endif /* MOON_PLUGIN_ACCESSIBILITY */
