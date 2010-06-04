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
#include "mutex.h"
#include "value.h"
#include "network.h"

#if !INCLUDED_MONO_HEADERS
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoClass MonoClass;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoImage MonoImage;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoObject MonoObject;
typedef struct _MonoProperty MonoProperty;
#endif

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
	/* @PropertyType=Uri,AlwaysChange,GenerateAccessors,GenerateManagedAccessors=false,DefaultValue=Uri() */
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

	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors,ManagedSetterAccess=Private,Validator=NonNullOnlyDuringInitializationValidator */
	const static int TitleProperty;	
	/* @PropertyType=double,DefaultValue=600.0,GenerateAccessors,ManagedSetterAccess=Private,Validator=OnlyDuringInitializationValidator */
	const static int HeightProperty;	
	/* @PropertyType=double,DefaultValue=800.0,GenerateAccessors,ManagedSetterAccess=Private,Validator=OnlyDuringInitializationValidator */
	const static int WidthProperty;	
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,ManagedSetterAccess=Private,Validator=OnlyDuringInitializationValidator */
	const static int LeftProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,ManagedSetterAccess=Private,Validator=OnlyDuringInitializationValidator */
	const static int TopProperty;
	/* @PropertyType=WindowStartupLocation,DefaultValue=WindowStartupLocationCenterScreen,GenerateAccessors,ManagedSetterAccess=Private,Validator=OnlyDuringInitializationValidator */
	const static int WindowStartupLocationProperty;
	/* @PropertyType=WindowStyle,DefaultValue=WindowStyleSingleBorderWindow,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int WindowStyleProperty;

	const char *GetTitle ();
	void SetTitle (const char *title);

	double GetWidth ();
	void SetWidth (double width);

	double GetHeight ();
	void SetHeight (double height);

	double GetLeft ();
	void SetLeft (double value);

	double GetTop ();
	void SetTop (double value);

	WindowStartupLocation GetWindowStartupLocation ();
	void SetWindowStartupLocation (WindowStartupLocation value);

	WindowStyle GetWindowStyle ();
	void SetWindowStyle (WindowStyle style);

protected:
	virtual ~WindowSettings ();
};

/* @Namespace=System.Windows */
class Icon : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	Icon ();

	/* @PropertyType=Uri,DefaultValue=Uri(),ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int SourceProperty;
	/* @PropertyType=Size,DefaultValue=Size(),ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int SizeProperty;

	Uri* GetSource ();
	void SetSource (Uri *source);

	Size* GetSize ();
	void SetSize (Size *size);

protected:
	virtual ~Icon ();
};

/* @Namespace=System.Windows */
class IconCollection : public DependencyObjectCollection {
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

	/* @PropertyType=string,DefaultValue=\"\",ManagedSetterAccess=Internal,GenerateAccessors,Validator=NonNullOnlyDuringInitializationValidator */
	const static int BlurbProperty;
	/* @PropertyType=string,DefaultValue=\"\",ManagedSetterAccess=Internal,GenerateAccessors,Validator=NonNullOnlyDuringInitializationValidator */
	const static int ShortNameProperty;
	/* @PropertyType=bool,DefaultValue=false,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int EnableGPUAccelerationProperty;
	/* @PropertyType=bool,DefaultValue=true,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int ShowInstallMenuItemProperty;
	/* @PropertyType=WindowSettings,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int WindowSettingsProperty;
	/* @PropertyType=IconCollection,AutoCreateValue,HiddenDefaultValue,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int IconsProperty;
	/* @PropertyType=SecuritySettings,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int SecuritySettingsProperty;
	
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

	SecuritySettings *GetSecuritySettings ();
	void SetSecuritySettings (SecuritySettings *value);

protected:
	virtual ~OutOfBrowserSettings ();
};

/* @Namespace=System.Windows */
class Deployment : public DependencyObject {
public:
 	/* @PropertyType=CrossDomainAccess,DefaultValue=CrossDomainAccessNoAccess,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int ExternalCallersFromCrossDomainProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal,Validator=OnlyDuringInitializationValidator */
	const static int EntryPointAssemblyProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal,Validator=OnlyDuringInitializationValidator */
	const static int EntryPointTypeProperty;
	/* @PropertyType=ExternalPartCollection,AutoCreateValue,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int ExternalPartsProperty;
	/* @PropertyType=OutOfBrowserSettings,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int OutOfBrowserSettingsProperty;
 	/* @PropertyType=AssemblyPartCollection,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int PartsProperty;
 	/* @PropertyType=string,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int RuntimeVersionProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Deployment ();
	
	bool InitializeManagedDeployment (gpointer plugin_instance, const char *culture, const char *uiculture);
	bool InitializeAppDomain ();

	virtual void Dispose ();

	/* @GenerateCBinding,GeneratePInvoke */
	Types* GetTypes () { return types; }
	
	Surface *GetSurface ();
	/* @GenerateCBinding,GeneratePInvoke */
	Surface *GetSurfaceReffed (); /* thread-safe */
	void SetSurface (Surface *surface);
	
	AssemblyPartCollection *GetParts ();
	void SetParts (AssemblyPartCollection *col);

	ExternalPartCollection *GetExternalParts ();
	void SetExternalParts (ExternalPartCollection *col);

	OutOfBrowserSettings *GetOutOfBrowserSettings ();
	void SetOutOfBrowserSettings (OutOfBrowserSettings *oob);
	
	void SetRuntimeVersion (const char *version);
	const char *GetRuntimeVersion ();

#if EVENT_ARG_REUSE
	PropertyChangedEventArgs *GetPropertyChangedEventArgs ();
	void ReleasePropertyChangedEventArgs (PropertyChangedEventArgs* args);
#endif

	void Reinitialize ();

	Application* GetCurrentApplication ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetCurrentApplication (Application* value);

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

	bool IsInitializing () { return is_initializing; };
	/* @GenerateCBinding,GeneratePInvoke */
	void SetInitialization (bool init);

	void SetXapLocation (const char *location);
	const char *GetXapLocation ();

	void SetXapFilename (const char *filename);
	const char *GetXapFilename ();
	
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
	static const char *GetPlatformDir () { return platform_dir; }

#if DEBUG
	struct moon_source : List::Node {
		char *uri;
		char *filename;
		virtual ~moon_source ()
		{
			g_free (uri);
			g_free (filename);
		}
	};
	void AddSource (const Uri *uri, const char *filename);
	List *GetSources ();
#endif

	
#if OBJECT_TRACKING
	GHashTable *objects_alive;
	pthread_mutex_t objects_alive_mutex;
	void ReportLeaks ();
#endif

	const char* InternString (const char *str);
	
	void SetHttpHandler (HttpHandler *handler);
	void SetDefaultHttpHandler (HttpHandler *handler);
	Downloader *CreateDownloader ();
	/* @GenerateCBinding,GeneratePInvoke */
	HttpRequest *CreateHttpRequest (HttpRequest::Options options);
	void UnregisterHttpRequest (HttpRequest *request);
	void AbortAllHttpRequests ();

protected:
	virtual ~Deployment ();

private:
	GHashTable *interned_strings;
	HttpHandler *http_handler;
	HttpHandler *default_http_handler;

#if DEBUG
	List *moon_sources;
#endif

	enum ShutdownState {
		ShutdownFailed = -1,
		Running = 0,
		CallManagedShutdown = 1,
		UnloadDomain = 2,
		DisposeDeployment = 3
	};
	Deployment (MonoDomain *domain);
	void InnerConstructor ();

	void DisposeAllMedias ();
	void DrainUnrefs ();
	static gboolean DrainUnrefs (gpointer ptr);

	Types* types;
	Surface *surface;
	Mutex surface_mutex;
	FontManager *font_manager;
	Application *current_app;
	MonoDomain *domain;
	List http_requests;
	List paths;

#if EVENT_ARG_REUSE
	unsigned int num_outstanding_changes;
	GPtrArray *change_args;
#endif
	
	// true if we're going to notify Loaded events on the next
	// tick.
	bool pending_loaded;

	Mutex medias_mutex;
	/* accessed from several threads, needs the medias_mutex locked on all accesses */
	List *medias;

	bool is_initializing;
	bool is_shutting_down;
	bool is_network_stopped;
	bool appdomain_unloaded;
	bool is_loaded_from_xap;
	// xap location, to help forging the right uris for downloaders
	char *xap_location;

	// xap filename, for use in installing apps
	char *xap_filename;

	// platform dir
	static char *platform_dir;
#if GLIB_CHECK_VERSION(2,10,0)
	volatile gpointer pending_unrefs;
#else
	gpointer pending_unrefs;
#endif	

	gint objects_created;
	gint objects_destroyed;
	
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
