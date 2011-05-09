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

#if HAVE_STDINT_H
#include <stdint.h>
#endif

#include "enums.h"
#include "dependencyobject.h"
#include "fontmanager.h"
#include "application.h"
#include "collection.h"
#include "value.h"
#include "network.h"
#include "uri.h"

#if !INCLUDED_MONO_HEADERS
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoClass MonoClass;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoImage MonoImage;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoObject MonoObject;
typedef struct _MonoProperty MonoProperty;
typedef struct _MonoJitInfo MonoJitInfo;
#endif


#if OBJECT_TRACKING
struct _MonoProfiler {
 public:
	const char *type_name; // Stacktraces are stored only for elements of this type
	GPtrArray *gchandles;
	GHashTable *jitted_methods;
	GPtrArray *stacktraces;
	Moonlight::MoonMutex locker;

	_MonoProfiler (bool gchandle, bool jit);

	void DumpJittedMethods ();
	void DumpStrongGCHandles ();
	void DumpTracesByType ();
	static void method_jitted (_MonoProfiler *prof, MonoMethod *method, MonoJitInfo* jinfo, int result);
	static void profiler_shutdown (_MonoProfiler *prof);
	static void track_gchandle (_MonoProfiler *prof, int op, int type, uintptr_t handle, MonoObject *obj);

};

typedef _MonoProfiler MonoProfiler;
#endif /* OBJECT_TRACKING */

namespace Moonlight {

/* @CBindingRequisite */
typedef void (*EnsureManagedPeerCallback)(EventObject *forObj, Type::Kind kind);

/* @Namespace=System.Windows,CallInitialize */
class MOON_API AssemblyPart : public DependencyObject {
public:
 	/* @PropertyType=string,DefaultValue=\"\",Validator=AssemblyPartSourceValidator */
	const static int SourceProperty;
	
protected:
	/* @GeneratePInvoke */
	AssemblyPart ();

	virtual ~AssemblyPart ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class AssemblyPartCollection : public DependencyObjectCollection {
public:
	virtual Type::Kind GetElementType () { return Type::ASSEMBLYPART; }

protected:
	/* @GeneratePInvoke */
	AssemblyPartCollection ();

	virtual ~AssemblyPartCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class ExternalPart : public DependencyObject {
protected:
	/* @GeneratePInvoke */
	ExternalPart ();

	virtual ~ExternalPart ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class ExtensionPart : public ExternalPart {
public:
	/* @PropertyType=Uri,AlwaysChange,IsConstPropertyType,GenerateAccessors,GenerateManagedAccessors=false,DefaultValue=new Uri() */
	const static int SourceProperty;

	void SetSource (const Uri *value);
	const Uri* GetSource ();

protected:
	/* @GeneratePInvoke */
	ExtensionPart ();

	virtual ~ExtensionPart ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class ExternalPartCollection : public DependencyObjectCollection {
public:
	virtual Type::Kind GetElementType () { return Type::EXTERNALPART; }

protected:
	/* @GeneratePInvoke */
	ExternalPartCollection ();

	virtual ~ExternalPartCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class WindowSettings : public DependencyObject {
public:
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
	/* @PropertyType=WindowStyle,DefaultValue=WindowStyleSingleBorderWindow,GenerateAccessors,Validator=OnlyDuringInitializationValidator,ReadOnly */
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
	/* @GeneratePInvoke */
	WindowSettings ();

	virtual ~WindowSettings ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
/* @ContentProperty=Source */
class MOON_API Icon : public DependencyObject {
public:
	/* @PropertyType=Uri,DefaultValue=new Uri(),IsConstPropertyType,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int SourceProperty;
	/* @PropertyType=Size,DefaultValue=Size(),ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int SizeProperty;

	const Uri* GetSource ();
	void SetSource (const Uri *source);

	Size* GetSize ();
	void SetSize (Size *size);

protected:
	/* @GeneratePInvoke */
	Icon ();

	virtual ~Icon ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class IconCollection : public DependencyObjectCollection {
public:
	virtual Type::Kind GetElementType () { return Type::ICON; }

protected:
	/* @GeneratePInvoke */
	IconCollection ();
	
	virtual ~IconCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class MOON_API OutOfBrowserSettings : public DependencyObject {
public:
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
	/* @GeneratePInvoke */
	OutOfBrowserSettings ();

	virtual ~OutOfBrowserSettings ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows,CallInitialize */
class MOON_API Deployment : public DependencyObject {
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
 	/* @PropertyType=AssemblyPartCollection,AutoCreateValue,ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int PartsProperty;
	/* @PropertyType=string,DefaultValue=\"1.0\",ManagedSetterAccess=Internal,GenerateAccessors,Validator=OnlyDuringInitializationValidator */
	const static int RuntimeVersionProperty;
	
	bool InitializeManagedDeployment (gpointer plugin_instance, const char *culture, const char *uiculture);
	bool InitializeAppDomain ();
	/* @GeneratePInvoke */
	bool InitializeAppDomain (const char *system_windows_fullname);

	virtual void Dispose ();

	/* @GeneratePInvoke */
	Types* GetTypes () { return types; }
	
	Surface *GetSurface ();
	/* @GeneratePInvoke */
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
	/* @GeneratePInvoke */
	void SetCurrentApplication (Application* value);

	/* @GeneratePInvoke */
	void SetEnsureManagedPeerCallback (EnsureManagedPeerCallback callback);

	/*
	 * thread-safe, returns false if the media couldn't be registered (if the
	 * deployment is already shutting down, in which case the media should
	 * dispose itself immediately)
	 */
	bool RegisterMedia (EventObject *media);
	/* thread-safe */
	void UnregisterMedia (EventObject *media);

	/* @GeneratePInvoke */
	static Deployment* GetCurrent ();
	/* @GeneratePInvoke */
	static void SetCurrent (Deployment* value);
	static void SetCurrent (Deployment* value, bool domain);

	static bool Initialize (const char *platform_dir, bool create_root_domain);

	static void RegisterThread ();
	static void UnregisterThread ();

	void UnrefDelayed (EventObject *obj);

	void TrackObjectCreated (EventObject *obj);
	void TrackObjectDestroyed (EventObject *obj);

	bool IsLoadedFromXap ();
	/* @GeneratePInvoke */
	void SetIsLoadedFromXap (bool flag);

	bool IsInitializing () { return is_initializing; };
	/* @GeneratePInvoke */
	void SetInitialization (bool init);

	void SetXapLocation (const Uri *location);
	const Uri *GetXapLocation ();

	const Uri *GetSourceLocation (bool *is_xap);

	void SetXapFilename (const char *filename);
	const char *GetXapFilename ();

	// refs object until either SetKeepAlive is called again with a false value, or shutdown
	// starts. Note that the implementation is not symmetric: the first SetKeepAlive (false)
	// will release the object even if SetKeepAlive (true) has been called several times.
	void SetKeepAlive (EventObject *object, bool value);

	FontManager *GetFontManager ();
	
	bool VerifyDownload (const char *filename);

	/* @GeneratePInvoke */
	GlyphTypefaceCollection *GetSystemTypefaces ();
	
	CrossDomainAccess GetExternalCallersFromCrossDomain ();
	void SetExternalCallersFromCrossDomain (CrossDomainAccess value);

	ErrorEventArgs* ManagedExceptionToErrorEventArgs (MonoObject *exc);
	void ManagedExceptionToMoonError (MonoObject *exc, MoonError::ExceptionType type, MoonError *error);

	GCHandle CreateManagedXamlLoader (gpointer plugin_instance, XamlLoader* native_loader, const Uri *resourceBase);
	void DestroyManagedXamlLoader (GCHandle xaml_loader);
	void DestroyManagedApplication (gpointer plugin_instance);

	Value *MonoXamlParserCreateFromFile (const char *file, bool create_namescope, bool validate_templates, MoonError *error);
	Value *MonoXamlParserCreateFromString (const char *xaml, bool create_namescope, bool validate_templates, MoonError *error, DependencyObject* owner = NULL);
	Value *MonoXamlParserHydrateFromString (const char *xaml, Value *obj, bool create_namescope, bool validate_templates, MoonError *error);

	virtual void EnsureManagedPeer ();
	void EnsureManagedPeer (EventObject *forObj);

	void EmitLoaded ();
	void EmitLoadedAsync ();
	void AddAllLoadedHandlers (UIElement *el, bool only_unemitted);
	void RemoveAllLoadedHandlers (UIElement *el);
	void AddLoadedHandler (UIElement *el, int token);
	void RemoveLoadedHandler (UIElement *el, int token);

	static void add_loaded_handler (EventObject *obj, int token, gpointer closure);
	static void remove_loaded_handler (EventObject *obj, int token, gpointer closure);
	static void delete_loaded_closure (EventObject *eo, int event_id, int token, gpointer closure);
	static bool match_loaded_closure (int token, EventHandler cb_handler, gpointer cb_data, gpointer data);
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

#if DEBUG
	// This method should only be used for debug purposes, it's not thread-safe (by the time it has returned,
	// the result is invalid). It's used by the stack printing code to check if it's safe to call into mono
	// for stack frame information.
	bool IsAppDomainAlive () { return shutdown_state <= UnloadDomain; }
#endif

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
	MoonMutex objects_alive_mutex;
	void ReportLeaks ();
#endif

	const char* InternString (const char *str);
	
	void SetHttpHandler (HttpHandler *handler);
	void SetDefaultHttpHandler (HttpHandler *handler);
	Downloader *CreateDownloader ();
	/* @GeneratePInvoke */
	HttpRequest *CreateHttpRequest (HttpRequest::Options options);
	void UnregisterHttpRequest (HttpRequest *request);
	void AbortAllHttpRequests ();

	/* Managed helpers */
	char *CanonicalizeFileName (const char *filename, bool is_xap_mode);
	const UriFunctions *GetUriFunctions () { return &uri_functions; }
	char *CreateMediaLogXml (const char **names, const char **values);
	/* @GeneratePInvoke */
	void SetUriFunctions (const UriFunctions *value);

	// GCHandle helpers
	void FreeGCHandle (GCHandle gchandle);
	GCHandle CloneGCHandle (GCHandle gchandle);
	GCHandle CreateGCHandle (void *mono_object);
	GCHandle CreateWeakGCHandle (void *mono_object);
	void *GetGCHandleTarget (GCHandle gchandle);

	/* The deployment initialization is awkward because the default managed deployment
	 * constructor is public, so everybody can do a "new Deployment ();" in managed.
	 * This will always create a native deployment (even if the managed ctor throws
	 * an exception, which it's supposed to do) - so in our native ctor we do nothing
	 * but zero-initialize fields, and the real consumers must call Initialize[Desktop]
	 * to do what the ctors used to do */
	/* @GeneratePInvoke,ManagedAccess=None */
	Deployment ();
	void Initialize (); /* This method must be called right after the ctor */
	void InitializeDesktop (MonoDomain *domain);
	static void RegisterICalls ();
	
	const static void *CurrentApplicationWeakRef;

	const char *GetUserAgent () { return user_agent; }
	void SetUserAgent (const char *value) { g_free (user_agent); user_agent = g_strdup (value); }

protected:
	virtual ~Deployment ();

private:
#if OBJECT_TRACKING
	static MonoProfiler *profiler;
#endif
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

	void InitializeCommon ();
	void DisposeAllMedias ();
	void DrainUnrefs ();
	static bool DrainUnrefs (gpointer ptr);

	Types* types;
	Surface *surface;
	MoonMutex surface_mutex;
	FontManager *font_manager;
	WeakRef<Application> current_app;
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

	MoonMutex medias_mutex;
	/* accessed from several threads, needs the medias_mutex locked on all accesses */
	List *medias;

	/* A list of objects that must be kept alive until told otherwise */
	GHashTable *keepalive;
	MoonMutex keepalive_mutex;

	bool appdomain_initialized;
	bool appdomain_initialization_result;
	bool is_initializing;
	bool is_shutting_down;
	bool is_network_stopped;
	bool appdomain_unloaded;
	bool is_loaded_from_xap;
	// xap location, to help forging the right uris for downloaders
	Uri *xap_location;

	// xap filename, for use in installing apps
	char *xap_filename;
	char *user_agent;

	// platform dir
	static char *platform_dir;
#if GLIB_CHECK_VERSION(2,10,0)
	volatile gpointer pending_unrefs;
#else
	gpointer pending_unrefs;
#endif	

	gint objects_created;
	gint objects_destroyed;
	
	UriFunctions uri_functions;

	ShutdownState shutdown_state;
	MonoImage *system_windows_image;
	MonoAssembly *system_windows_assembly;
	MonoClass *system_windows_deployment;
	MonoMethod *deployment_shutdown;

	MonoClass *mono_xaml_parser;
	MonoMethod *mono_xaml_parser_create_from_file;
	MonoMethod *mono_xaml_parser_create_from_string;
	MonoMethod *mono_xaml_parser_hydrate_from_string;

	// Methods
	MonoMethod   *moon_load_xaml;
	MonoMethod   *moon_ensure_managed_peer;
	MonoMethod   *moon_initialize_deployment_xap;
	MonoMethod   *moon_initialize_deployment_xaml;
	MonoMethod   *moon_destroy_application;

	MonoClass    *moon_exception;
	MonoProperty *moon_exception_message;
	MonoProperty *moon_exception_error_code;
	
	MonoMethod   *MonoGetMethodFromName (MonoClass *klass, const char *name, int narg);
	MonoProperty *MonoGetPropertyFromName (MonoClass *klass, const char *name);

	EnsureManagedPeerCallback ensure_managed_peer;

	bool InitializeManagedXamlParser (MonoImage *system_windows_image);
	
	static bool ShutdownManagedCallback (gpointer user_data);
	bool ShutdownManaged ();
	
	static Deployment *desktop_deployment;
	static GHashTable *current_hash;
	static gboolean initialized;
	static MoonTlsKey tls_key;
	static MoonMutex hash_mutex;
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

};

#endif /* __DEPLOYMENT_H__ */
