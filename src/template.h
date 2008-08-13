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

#include "dependencyobject.h"

//
// FrameworkTemplate
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class FrameworkTemplate : public DependencyObject {
 protected:
	virtual ~FrameworkTemplate () {}

 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	FrameworkTemplate ();

	virtual Type::Kind GetObjectType () { return Type::FRAMEWORKTEMPLATE; }
};

//
// ControlTemplate
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class ControlTemplate : public FrameworkTemplate {
 protected:
	virtual ~ControlTemplate () {}
	
 public:
	/* @PropertyType=Managed,ManagedPropertyType=System.Type,Access=Internal,ManagedAccessorAccess=Public,ManagedFieldAccess=Private */
	static DependencyProperty *TargetTypeProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ControlTemplate ();

	virtual Type::Kind GetObjectType () { return Type::CONTROLTEMPLATE; }
};

#endif /* __MOON_TEMPLATE_H__ */
