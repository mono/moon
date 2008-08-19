/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * deployment.h: Deployment
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __DEPLOYMENT_H__
#define __DEPLOYMENT_H__

#include <glib.h>

#include "enums.h"
#include "dependencyobject.h"
#include "collection.h"

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class AssemblyPart : public DependencyObject {
 protected:
	virtual ~AssemblyPart () {}
	
 public:
 	/* @PropertyType=string */
	static DependencyProperty *SourceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	AssemblyPart () { }
	
	virtual Type::Kind GetObjectType () { return Type::ASSEMBLYPART; }
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class AssemblyPartCollection : public DependencyObjectCollection {
 protected:
	virtual ~AssemblyPartCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	AssemblyPartCollection () { }
	
	virtual Type::Kind GetObjectType ()  { return Type::ASSEMBLYPART_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::ASSEMBLYPART; }
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class Deployment : public DependencyObject {
 protected:
	virtual ~Deployment () {}
	
 public:
 	/* @PropertyType=CrossDomainAccess,DefaultValue=CrossDomainAccessNoAccess,ManagedSetterAccess=Internal */
	static DependencyProperty *ExternalCallersFromCrossDomainProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	static DependencyProperty *EntryPointAssemblyProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	static DependencyProperty *EntryPointTypeProperty;
 	/* @PropertyType=AssemblyPartCollection,ManagedSetterAccess=Internal */
	static DependencyProperty *PartsProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	static DependencyProperty *RuntimeVersionProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Deployment () { }
	
	virtual Type::Kind GetObjectType () { return Type::DEPLOYMENT; } 
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
/* @ManagedName=ApplicationInternal */
class Application : public DependencyObject {
 protected:
	virtual ~Application () {}
	
 public:
	/* @PropertyType=ResourceDictionary */
	static DependencyProperty *ResourcesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Application () { }
	
	virtual Type::Kind GetObjectType () { return Type::APPLICATION; }
};

#endif /* __DEPLOYMENT_H__ */
