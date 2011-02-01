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
#include "fontmanager.h"
#include "application.h"
#include "collection.h"
#include "downloader.h"
#include "mutex.h"
#include "value.h"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>

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
class ExternalPart : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ExternalPart ();

protected:
	virtual ~ExternalPart ();
};

/* @Namespace=System.Windows */
class ExtensionPart : public ExternalPart {
public:
	/* @PropertyType=Uri,AlwaysChange,GenerateAccessors,DefaultValue=Uri() */
	const static int SourceProperty;

	void SetSource (Uri *value);
	Uri* GetSource ();

	/* @GenerateCBinding,GeneratePInvoke */
	ExtensionPart ();

protected:
	virtual ~ExtensionPart ();
};

/* @Namespace=System.Windows */
class ExternalPartCollection : public DependencyObjectCollection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ExternalPartCollection ();

	virtual Type::Kind GetElementType () { return Type::EXTERNALPART; }

protected:
	virtual ~ExternalPartCollection ();
};

/* @Namespace=System.Windows */
class WindowSettings : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	WindowSettings ();

	/* @PropertyType=string,DefaultValue=\"\",Validator=NonNullValidator,GenerateAccessors,ManagedSetterAccess=Private */
	const static int TitleProperty;	
	/* @PropertyType=double,DefaultValue=\"\",GenerateAccessors,ManagedSetterAccess=Private */
	const static int HeightProperty;	
	/* @PropertyType=double,DefaultValue=\"\",GenerateAccessors,ManagedSetterAccess=Private */
	const static int WidthProperty;	

	const char *GetTitle ();
	void SetTitle (const char *title);

	double GetWidth ();
	void SetWidth (double width);

	double GetHeight ();
	void SetHeight (double height);

protected:
	virtual ~WindowSettings ();
};

/* @Namespace=System.Windows */
class Icon : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	Icon ();

	/* @PropertyType=Uri,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int SourceProperty;
	/* @PropertyType=Size,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int SizeProperty;

	Uri* GetSource ();
	void SetSource (Uri *source);

	Size* GetSize ();
	void SetSize (Size *size);

protected:
	virtual ~Icon ();
};

/* @Namespace=System.Windows */
class IconCollection : public Collection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	IconCollection ();
	
	virtual Type::Kind GetElementType () { return Type::ICON; }

protected:
	virtual ~IconCollection ();
};

/* @Namespace=System.Windows */
class OutOfBrowserSettings : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	OutOfBrowserSettings ();

	/* @PropertyType=string,DefaultValue=\"\",Validator=NonNullValidator,GenerateAccessors */
	const static int BlurbProperty;	
	/* @PropertyType=string,DefaultValue=\"\",Validator=NonNullValidator,GenerateAccessors */
	const static int ShortNameProperty;	
	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	const static int EnableGPUAccelerationProperty;
	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	const static int ShowInstallMenuItemProperty;
	/* @PropertyType=WindowSettings,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int WindowSettingsProperty;
	/* @PropertyType=IconCollection,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int IconsProperty;
	
	const char *GetBlurb ();
	void SetBlurb (const char *blurb);

	const char *GetShortName ();
	void SetShortName (const char *shortName);

	bool GetEnableGPUAcceleration ();
	void SetEnableGPUAcceleration (bool enable);

	bool GetShowInstallMenuItem ();
	void SetShowInstallMenuItem (bool show);

	WindowSettings* GetWindowSettings ();
	void SetWindowSettings (WindowSettings* settings);

	IconCollection* GetIcons ();
	void SetIcons (IconCollection* icons);

protected:
	virtual ~OutOfBrowserSettings ();
};

/* @Namespace=System.Windows */
class MOON_API Deployment : public DependencyObject {
public:
 	/* @PropertyType=CrossDomainAccess,DefaultValue=CrossDomainAccessNoAccess,ManagedSetterAccess=Internal,GenerateAccessors,Validator=CrossDomainValidator */
	const static int ExternalCallersFromCrossDomainProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	const static int EntryPointAssemblyProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal */
	const static int EntryPointTypeProperty;
	/* @PropertyType=ExternalPartCollection,AutoCreateValue,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int ExternalPartsProperty;
	/* @PropertyType=OutOfBrowserSettings,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int OutOfBrowserSettingsProperty;
 	/* @PropertyType=AssemblyPartCollection,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int PartsProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int RuntimeVersionProperty;
 	/* @PropertyType=Surface,ManagedAccess=Internal,GenerateAccessors */
	const static int SurfaceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Deployment ();
	
	bool InitializeManagedDeployment (gpointer plugin_instance, const char *file, const char *culture, const char *uiculture);
	bool InitializeAppDomain ();

	virtual void Dispose ();

	/* @GenerateCBinding,GeneratePInvoke */
	Types* GetTypes () { return types; }
	
	Surface *GetSurface ();
	void SetSurface (Surface *surface);
	
	AssemblyPartCollection *GetParts ();
	void SetParts (AssemblyPartCollection *col);

	ExternalPartCollection *GetExternalParts ();
	void SetExternalParts (ExternalPartCollection *col);

	OutOfBrowserSettings *GetOutOfBrowserSettings ();
	void SetOutOfBrowserSettings (OutOfBrowserSettings *oob);
	
	void SetRuntimeVersion (const char *version);
	const char *GetRuntimeVersion ();
	
	void Reinitialize ();

	Application* GetCurrentApplication ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetCurrentApplication (Application* value);

	void RegisterDownloader (IDownloader *dl);
	void UnregisterDownloader (IDownloader *dl);
	/*
	 * thread-safe, returns false if the media couldn't be registered (if the
	 * deployment is already shutting down, in which case the media should
	 * dispose itself immediately)
	 */
	bool RegisterMedia (EventObject *media);
	/* thread-safe */
	void UnregisterMedia (EventObject *media);

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
	
	FontManager *GetFontManager ();
	
	CrossDomainAccess GetExternalCallersFromCrossDomain ();
	void SetExternalCallersFromCrossDomain (CrossDomainAccess value);

	ErrorEventArgs* ManagedExceptionToErrorEventArgs (MonoObject *exc);
	gpointer CreateManagedXamlLoader (gpointer plugin_instance, XamlLoader* native_loader, const char *resourceBase, const char *file, const char *str);
	void DestroyManagedXamlLoader (gpointer xaml_loader);
	void DestroyManagedApplication (gpointer plugin_instance);

	void PostLoaded ();
	void EmitLoaded ();
	void AddAllLoadedHandlers (UIElement *el, bool only_unemitted);
	void RemoveAllLoadedHandlers (UIElement *el);
	void AddLoadedHandler (UIElement *el, int token);
	void RemoveLoadedHandler (UIElement *el, int token);

	static void emit_delayed_loaded (EventObject *data);

	static void add_loaded_handler (EventObject *obj, EventHandler handler, gpointer handler_data, gpointer closure);
	static void remove_loaded_handler (EventObject *obj, EventHandler handler, gpointer handler_data, gpointer closure);
	static void delete_loaded_closure (gpointer closure);
	static bool match_loaded_closure (EventHandler cb_handler, gpointer cb_data, gpointer data);
	static void proxy_loaded_event (EventObject *sender, EventArgs *arg, gpointer closure);

	/* @GenerateManagedEvent=false */
	const static int LayoutUpdatedEvent;

	/* @GenerateManagedEvent=false */
	const static int LoadedEvent;

	/* @GenerateManagedEvent=false */
	const static int ShuttingDownEvent;
	/* @GenerateManagedEvent=false */
	const static int AppDomainUnloadedEvent; /* this is emitted just after the appdomain has successfully unloaded */

	void LayoutUpdated ();

	void Shutdown (); /* main thread only */
	bool IsShuttingDown (); /* main thread only */

	void TrackPath (char *path);

	static gint32 GetDeploymentCount (); /* returns the number of deployments currently alive */
protected:
	virtual ~Deployment ();

private:
	enum ShutdownState {
		ShutdownFailed = -1,
		Running = 0,
		CallManagedShutdown = 1,
		UnloadDomain = 2,
		DisposeDeployment = 3
	};
	Deployment (MonoDomain *domain);
	void InnerConstructor ();

	void AbortAllDownloaders ();
	void DisposeAllMedias ();
	void DrainUnrefs ();
	static gboolean DrainUnrefs (gpointer ptr);

	Types* types;
	FontManager *font_manager;
	Application *current_app;
	MonoDomain *domain;
	List downloaders;
	List paths;

	// true if we're going to notify Loaded events on the next
	// tick.
	bool pending_loaded;

	Mutex medias_mutex;
	/* accessed from several threads, needs the medias_mutex locked on all accesses */
	List *medias;

	bool is_shutting_down;
	bool appdomain_unloaded;
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

	ShutdownState shutdown_state;
	MonoImage *system_windows_image;
	MonoAssembly *system_windows_assembly;
	MonoClass *system_windows_deployment;
	MonoMethod *deployment_shutdown;

	// Methods
	MonoMethod   *moon_load_xaml;
	MonoMethod   *moon_initialize_deployment_xap;
	MonoMethod   *moon_initialize_deployment_xaml;
	MonoMethod   *moon_destroy_application;

	MonoClass    *moon_exception;
	MonoProperty *moon_exception_message;
	MonoProperty *moon_exception_error_code;
	
	MonoMethod   *MonoGetMethodFromName (MonoClass *klass, const char *name, int narg);
	MonoProperty *MonoGetPropertyFromName (MonoClass *klass, const char *name);
	
	
	static gboolean ShutdownManagedCallback (gpointer user_data);
	gboolean ShutdownManaged ();
	
	static Deployment *desktop_deployment;
	static GHashTable *current_hash;
	static gboolean initialized;
	static pthread_key_t tls_key;
	static pthread_mutex_t hash_mutex;
	static MonoDomain *root_domain;
	static gint32 deployment_count;
};

/*
 * DeploymentStack: 
 *  for all calls into javascript we need to push/pop the current deployment.
 *  this class is (ab)uses C++ for this, just put a "DeploymentStack deployment_push_pop;" 
 *  in every method which needs to push/pop the current deployment. the compiler will
 *  call the ctor in the beginning of the method, and the dtor in the end.
 */
class DeploymentStack {
public:
	DeploymentStack ()
	{
		deployment = Deployment::GetCurrent ();
	}
	
	~DeploymentStack ()
	{
		Deployment::SetCurrent (deployment);
	}

private:
	Deployment *deployment;
};

#endif /* __DEPLOYMENT_H__ */
