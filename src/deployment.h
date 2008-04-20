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
 public:
	static DependencyProperty *Source;

	AssemblyPart () {} 
	virtual Type::Kind GetObjectType () { return Type::ASSEMBLYPART; } 
};

void assembly_part_init (void);

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

 public:
	// DependencyProperties

	static DependencyProperty *AllowInboundCallsFromXDomain;
	static DependencyProperty *EntryPointAssembly;
	static DependencyProperty *EntryPointType;
	static DependencyProperty *NeutralResourcesLanguage;
	static DependencyProperty *Parts;
	static DependencyProperty *SupportedCultures;


	Deployment () {} 
	virtual Type::Kind GetObjectType () { return Type::DEPLOYMENT; } 
};

void        deployment_init (void);
Deployment *deployment_new (void);

G_END_DECLS

#endif /* __DEPLOYMENT_H__ */
