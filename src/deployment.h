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
#include "downloader.h"

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
 	/* @PropertyType=Surface,ManagedAccess=Internal,GenerateAccessors */
	static DependencyProperty *SurfaceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Deployment ();
	
	virtual void Dispose ();

	/* @GenerateCBinding,GeneratePInvoke */
	Types* GetTypes();
	
	Surface *GetSurface ();
	void SetSurface (Surface *surface);

	Application* GetCurrentApplication ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetCurrentApplication (Application* value);

	void RegisterIDownloader (IDownloader *idl);
	void UnregisterIDownloader (IDownloader *idl);

	/* @GenerateCBinding,GeneratePInvoke */
	static Deployment* GetCurrent ();
	/* @GenerateCBinding,GeneratePInvoke */
	static void SetCurrent (Deployment* value);
	static void SetCurrent (Deployment* value, bool domain);

	static bool Initialize ();
	static void RegisterThread (Deployment *deployment);

	void UnrefDelayed (EventObject *obj);

	void TrackObjectCreated (EventObject *obj);
	void TrackObjectDestroyed (EventObject *obj);

private:	
	void AbortAllIDownloaders ();
	void DrainUnrefs ();
	static gboolean DrainUnrefs (gpointer ptr);

	Types* types;
	Application* current_app;
	MonoDomain *domain;
	List *idownloaders;
	volatile gpointer pending_unrefs;
	
	gint objects_created;
	gint objects_destroyed;
	
#if OBJECT_TRACKING
	GHashTable *objects_alive;
	pthread_mutex_t objects_alive_mutex;
	void ReportLeaks ();
#endif

	static GHashTable *current_hash;
	static gboolean initialized;
	static pthread_key_t tls_key;
	static pthread_mutex_t hash_mutex;
	static MonoDomain *root_domain;
};

#endif /* __DEPLOYMENT_H__ */
