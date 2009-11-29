/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * template.h:
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_TEMPLATE_H__
#define __MOON_TEMPLATE_H__

#include <glib.h>

#include "xaml.h"
#include "dependencyobject.h"
#include "control.h"

class XamlContext;

//
// FrameworkTemplate
//
/* @Namespace=System.Windows */
class FrameworkTemplate : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	FrameworkTemplate ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual DependencyObject *GetVisualTree (FrameworkElement *templateBindingSource = NULL);

	void SetXamlBuffer (XamlContext *context, const char *buffer);

protected:
	virtual ~FrameworkTemplate ();

	char *xaml_buffer;
	XamlContext *xaml_context;
};

//
// ControlTemplate
//
/* @Namespace=System.Windows.Controls */
class ControlTemplate : public FrameworkTemplate {
public:
	/* @PropertyType=ManagedTypeInfo,ManagedPropertyType=System.Type,Access=Internal,ManagedAccessorAccess=Public,ManagedFieldAccess=Private */
	const static int TargetTypeProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ControlTemplate ();

protected:
	virtual ~ControlTemplate () {}
};

//
// DataTemplate
//
/* @Namespace=System.Windows */
class DataTemplate : public FrameworkTemplate {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DataTemplate ();

	virtual DependencyObject *GetVisualTree (FrameworkElement *templateBindingSource);
protected:
	virtual ~DataTemplate () {}
};

#endif /* __MOON_TEMPLATE_H__ */
