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
	virtual void Dispose ();

	/* @GenerateCBinding,GeneratePInvoke */
	/* This method always returns a reffed object */
	virtual DependencyObject *GetVisualTreeWithError (FrameworkElement *templateBindingSource, MoonError *error);

	void SetXamlBuffer (XamlContext *context, const char *buffer);

protected:
	virtual ~FrameworkTemplate () {}

	char *xaml_buffer;
	XamlContext *xaml_context;

private:
	EVENTHANDLER (FrameworkTemplate, ShuttingDownEvent, Deployment, EventArgs);
	void ClearXamlBuffer ();
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

	/* This method always returns a reffed object */
	virtual DependencyObject *GetVisualTreeWithError (FrameworkElement *templateBindingSource, MoonError *error);
protected:
	virtual ~DataTemplate () {}
};

#endif /* __MOON_TEMPLATE_H__ */
