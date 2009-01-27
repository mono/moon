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
#include "application.h"
#include "collection.h"

#include <mono/metadata/appdomain.h>

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class AssemblyPart : public DependencyObject {
 protected:
	virtual ~AssemblyPart ();
	
 public:
 	/* @PropertyType=string */
	static DependencyProperty *SourceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	AssemblyPart ();
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class AssemblyPartCollection : public DependencyObjectCollection {
 protected:
	virtual ~AssemblyPartCollection ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	AssemblyPartCollection ();

	virtual Type::Kind GetElementType () { return Type::ASSEMBLYPART; }
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class Deployment : public DependencyObject {
 protected:
	virtual ~Deployment ();

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
	Deployment ();

	/* @GenerateCBinding,GeneratePInvoke */
	Types* GetTypes();

	Application* GetCurrentApplication ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetCurrentApplication (Application* value);
	/* @GenerateCBinding,GeneratePInvoke */
	void SetCurrent ();

	/* @GenerateCBinding,GeneratePInvoke */
	static Deployment* GetCurrent ();
	/* @GenerateCBinding,GeneratePInvoke */
	static void SetCurrent (Deployment* value);
	/* @GenerateCBinding,GeneratePInvoke */
	static bool Initialize ();

private:
	Types* types;
	Application* current_app;
	MonoDomain *domain;
	static GHashTable *current_hash;
	static gboolean initialized;
	static pthread_key_t tls_key;
	static MonoDomain *root_domain;
};

#endif /* __DEPLOYMENT_H__ */
