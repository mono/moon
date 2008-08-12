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

G_BEGIN_DECLS

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
	
	/* @GenerateCBinding */
	AssemblyPart () {}
	
	virtual Type::Kind GetObjectType () { return Type::ASSEMBLYPART; }
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class AssemblyPartCollection : public DependencyObjectCollection {
 protected:
	virtual ~AssemblyPartCollection () {}

 public:
	/* @GenerateCBinding */
	AssemblyPartCollection () {}
	
	virtual Type::Kind GetObjectType ()  { return Type::ASSEMBLYPART_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::ASSEMBLYPART; }
};

enum CrossDomainAccess {
	CrossDomainAccessNoAccess,
	CrossDomainAccessFullAccess,
	CrossDomainAccessScriptableOnly,
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class Deployment : public DependencyObject {
 protected:
	virtual ~Deployment () {}
	
 public:
	// DependencyProperties
 	/* @PropertyType=CrossDomainAccess,DefaultValue=CrossDomainAccessNoAccess */
	static DependencyProperty *ExternalCallersFromCrossDomainProperty;
 	/* @PropertyType=string */
	static DependencyProperty *EntryPointAssemblyProperty;
 	/* @PropertyType=string */
	static DependencyProperty *EntryPointTypeProperty;
 	/* @PropertyType=AssemblyPartCollection */
	static DependencyProperty *PartsProperty;
 	/* @PropertyType=string */
	static DependencyProperty *RuntimeVersionProperty;


	/* @GenerateCBinding */
	Deployment () {} 
	virtual Type::Kind GetObjectType () { return Type::DEPLOYMENT; } 
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class Application : public DependencyObject {
 protected:
	virtual ~Application () {}
	
 public:

	/* @GenerateCBinding */
	Application () {} 
	virtual Type::Kind GetObjectType () { return Type::APPLICATION; }

	/* @PropertyType=ResourceDictionary */
	static DependencyProperty *ResourcesProperty;
};

G_END_DECLS

#endif /* __DEPLOYMENT_H__ */
