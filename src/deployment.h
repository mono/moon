/* 
 * deployment.h: Deployment
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __DEPLOYMENT_H__
#define __DEPLOYMENT_H__

G_BEGIN_DECLS

#include "enums.h"
#include "dependencyobject.h"
#include "collection.h"


class AssemblyPart : public DependencyObject {
 protected:
	virtual ~AssemblyPart () {}
	
 public:
	static DependencyProperty *SourceProperty;
	
	AssemblyPart () {} 
	virtual Type::Kind GetObjectType () { return Type::ASSEMBLYPART; }
};

AssemblyPart *assembly_part_new (void);


class AssemblyPartCollection : public Collection {
 protected:
	virtual ~AssemblyPartCollection () {}

 public:
	AssemblyPartCollection () {}
	virtual Type::Kind GetObjectType ()  { return Type::ASSEMBLYPART_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::ASSEMBLYPART; }
};

AssemblyPartCollection *assembly_part_collection_new (void);


class SupportedCulture : public DependencyObject {
 protected:
	virtual ~SupportedCulture () {}
	
 public:
	SupportedCulture () {}
	virtual Type::Kind GetObjectType () { return Type::SUPPORTEDCULTURE; }
};


class SupportedCulturesCollection : public Collection {
 protected:
	virtual ~SupportedCulturesCollection () {}

 public:
	SupportedCulturesCollection () {}
	virtual Type::Kind GetObjectType ()  { return Type::SUPPORTEDCULTURES_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::SUPPORTEDCULTURE; }
};

SupportedCulturesCollection *supported_cultures_collection_new (void);


class Deployment : public DependencyObject {
 protected:
	virtual ~Deployment () {}
	
 public:
	// DependencyProperties
	static DependencyProperty *AllowInboundCallsFromXDomainProperty;
	static DependencyProperty *EntryPointAssemblyProperty;
	static DependencyProperty *EntryPointTypeProperty;
	static DependencyProperty *NeutralResourcesLanguageProperty;
	static DependencyProperty *PartsProperty;
	static DependencyProperty *SupportedCulturesProperty;


	Deployment () {} 
	virtual Type::Kind GetObjectType () { return Type::DEPLOYMENT; } 
};

Deployment *deployment_new (void);


void deployment_init (void);

G_END_DECLS

#endif /* __DEPLOYMENT_H__ */
