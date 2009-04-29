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

typedef struct _MonoDomain MonoDomain;

/* @Namespace=System.Windows */
class AssemblyPart : public DependencyObject {
public:
 	/* @PropertyType=string,DefaultValue=\"\" */
	const static int SourceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	AssemblyPart ();

protected:
	virtual ~AssemblyPart ();
};


/* @Namespace=System.Windows */
class AssemblyPartCollection : public DependencyObjectCollection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	AssemblyPartCollection ();

	virtual Type::Kind GetElementType () { return Type::ASSEMBLYPART; }

protected:
	virtual ~AssemblyPartCollection ();
};

/* @Namespace=System.Windows */
class Deployment : public DependencyObject {
public:
 	/* @PropertyType=CrossDomainAccess,DefaultValue=CrossDomainAccessNoAccess,ManagedSetterAccess=Internal */
	const static int ExternalCallersFromCrossDomainProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	const static int EntryPointAssemblyProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	const static int EntryPointTypeProperty;
 	/* @PropertyType=AssemblyPartCollection,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int PartsProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	const static int RuntimeVersionProperty;
 	/* @PropertyType=Surface,ManagedAccess=Internal,GenerateAccessors */
	const static int SurfaceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Deployment ();
	
	virtual void Dispose ();

	/* @GenerateCBinding,GeneratePInvoke */
	Types* GetTypes();
	
	Surface *GetSurface ();
	void SetSurface (Surface *surface);
	
	AssemblyPartCollection *GetParts ();
	void SetParts (AssemblyPartCollection *col);


	Application* GetCurrentApplication ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetCurrentApplication (Application* value);

	void RegisterDownloader (IDownloader *dl);
	void UnregisterDownloader (IDownloader *dl);

	/* @GenerateCBinding,GeneratePInvoke */
	static Deployment* GetCurrent ();
	/* @GenerateCBinding,GeneratePInvoke */
	static void SetCurrent (Deployment* value);
	static void SetCurrent (Deployment* value, bool domain);

	static bool Initialize (const char *platform_dir, bool create_root_domain);

	static void RegisterThread (Deployment *deployment);

	void UnrefDelayed (EventObject *obj);

	void TrackObjectCreated (EventObject *obj);
	void TrackObjectDestroyed (EventObject *obj);

	bool IsLoadedFromXap ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetIsLoadedFromXap (bool flag);

	void SetXapLocation (const char *location);
	const char *GetXapLocation ();

	const static int ShuttingDownEvent;

protected:
	virtual ~Deployment ();

private:
	Deployment (MonoDomain *domain);
	void InnerConstructor ();

	void AbortAllDownloaders ();
	void DrainUnrefs ();
	static gboolean DrainUnrefs (gpointer ptr);

	Types* types;
	Application* current_app;
	MonoDomain *domain;
	List *downloaders;

	bool is_loaded_from_xap;
	// xap location, to help forging the right uris for downloaders
	char *xap_location;

#if GLIB_CHECK_VERSION(2,10,0)
	volatile gpointer pending_unrefs;
#else
	gpointer pending_unrefs;
#endif	

	gint objects_created;
	gint objects_destroyed;
	
#if OBJECT_TRACKING
	GHashTable *objects_alive;
	pthread_mutex_t objects_alive_mutex;
	void ReportLeaks ();
#endif
	
	static Deployment *desktop_deployment;
	static GHashTable *current_hash;
	static gboolean initialized;
	static pthread_key_t tls_key;
	static pthread_mutex_t hash_mutex;
	static MonoDomain *root_domain;
};

#endif /* __DEPLOYMENT_H__ */
