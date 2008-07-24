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
class AssemblyPart : public DependencyObject {
 protected:
	virtual ~AssemblyPart () {}
	
 public:
	static DependencyProperty *SourceProperty;
	
	AssemblyPart () {}
	
	virtual Type::Kind GetObjectType () { return Type::ASSEMBLYPART; }
};

AssemblyPart *assembly_part_new (void);


/* @SilverlightVersion="2" */
class AssemblyPartCollection : public ObjectCollection {
 protected:
	virtual ~AssemblyPartCollection () {}

 public:
	AssemblyPartCollection () {}
	
	virtual Type::Kind GetObjectType ()  { return Type::ASSEMBLYPART_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::ASSEMBLYPART; }
};

AssemblyPartCollection *assembly_part_collection_new (void);

enum CrossDomainAccess {
	CrossDomainAccessNoAccess,
	CrossDomainAccessFullAccess,
	CrossDomainAccessScriptableOnly,
};

/* @SilverlightVersion="2" */
class Deployment : public DependencyObject {
 protected:
	virtual ~Deployment () {}
	
 public:
	// DependencyProperties
	static DependencyProperty *ExternalCallersFromCrossDomainProperty;
	static DependencyProperty *EntryPointAssemblyProperty;
	static DependencyProperty *EntryPointTypeProperty;
	static DependencyProperty *PartsProperty;
	static DependencyProperty *RuntimeVersionProperty;


	Deployment () {} 
	virtual Type::Kind GetObjectType () { return Type::DEPLOYMENT; } 
};

Deployment *deployment_new (void);



/* @SilverlightVersion="2" */
class Application : public DependencyObject {
 protected:
	virtual ~Application () {}
	
 public:

	Application () {} 
	virtual Type::Kind GetObjectType () { return Type::APPLICATION; }
};

Application *application_new (void);


void deployment_init (void);

G_END_DECLS

#endif /* __DEPLOYMENT_H__ */
