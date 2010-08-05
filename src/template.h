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

namespace Moonlight {

class XamlContext;

//
// FrameworkTemplate
//
/* @Namespace=System.Windows */
class FrameworkTemplate : public DependencyObject {
public:
	virtual void Dispose ();

	/* @GenerateCBinding,GeneratePInvoke */
	/* This method always returns a reffed object */
	virtual DependencyObject *GetVisualTreeWithError (FrameworkElement *templateBindingSource, MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	void SetXamlBuffer (parse_template_func parse_template, Value *parse_template_data, const char *buffer);

protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	FrameworkTemplate ();

	virtual ~FrameworkTemplate () {}

	char *xaml_buffer;
	parse_template_func *parse_template;
	Value *parse_template_data;

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

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
	/* @PropertyType=ManagedTypeInfo,AutoCreator=ControlTypeCreator,GenerateAccessors,ManagedPropertyType=System.Type,Access=Internal,ManagedAccessorAccess=Public,ManagedFieldAccess=Private */
	const static int TargetTypeProperty;

	void SetTargetType (ManagedTypeInfo *info);
	ManagedTypeInfo *GetTargetType ();

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	ControlTemplate ();
	virtual ~ControlTemplate () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

//
// DataTemplate
//
/* @Namespace=System.Windows */
class DataTemplate : public FrameworkTemplate {
public:
	/* This method always returns a reffed object */
	virtual DependencyObject *GetVisualTreeWithError (FrameworkElement *templateBindingSource, MoonError *error);
protected:
	/* @GenerateCBinding,GeneratePInvoke */
	DataTemplate ();

	virtual ~DataTemplate () {}
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

};
#endif /* __MOON_TEMPLATE_H__ */
