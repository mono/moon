/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * deployment.cpp: Deployment Class support
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#define INCLUDED_MONO_HEADERS 1

#include <glib.h>

#include <stdlib.h>
#include <mono/mini/jit.h>
#include <mono/metadata/debug-helpers.h>
G_BEGIN_DECLS
/* because this header sucks */
#include <mono/metadata/mono-debug.h>
G_END_DECLS
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/profiler.h>

#include <mono/metadata/assembly.h>
#include <mono/metadata/appdomain.h>

#include "factory.h"
#include "downloader.h"
#include "deployment.h"
#include "timemanager.h"
#include "debug.h"
#include "utils.h"
#include "security.h"
#include "namescope.h"
#include "pipeline.h"
#if HAVE_CURL
#include "network-curl.h"
#endif
#include "uri.h"

namespace Moonlight {

#if PROPERTY_LOOKUP_DIAGNOSTICS
extern gint64 provider_property_lookups;
#endif

/*
 * Deployment
 */


#if OBJECT_TRACKING
MonoProfiler *Deployment::profiler = NULL;
#endif

gboolean Deployment::initialized = FALSE;
pthread_key_t Deployment::tls_key = 0;
pthread_mutex_t Deployment::hash_mutex;
GHashTable* Deployment::current_hash = NULL;
MonoDomain* Deployment::root_domain = NULL;
Deployment *Deployment::desktop_deployment = NULL;
gint32 Deployment::deployment_count = 0;
char *Deployment::platform_dir = NULL;

class HttpRequestNode : public List::Node {
public:
	HttpRequest *request;
	HttpRequestNode (HttpRequest *request)
	{
		this->request = request;
	}
	virtual ~HttpRequestNode ()
	{
		if (request != NULL)
			request->Abort ();
	}
};


class StringNode : public List::Node {
public:
	char *str;

	StringNode (char *str) {
		this->str = g_strdup (str);
	}
};

static MonoBreakPolicy
moonlight_should_insert_breakpoint (MonoMethod *method)
{
	return MONO_BREAK_POLICY_ON_DBG;
}

void *event_object_get_managed_object (EventObject *eo)
{
#if DEBUG
	if (!eo->hadManagedPeer && eo->GetObjectType () != Type::DEPLOYMENT)
		printf ("*** MOONLIGHT ERROR ***: No managed peer was ever created for %s\n", eo->GetTypeName ());
#endif
	return eo->GetDeployment ()->GetGCHandleTarget (eo->GetManagedHandle ());
}


bool
Deployment::Initialize (const char *platform_dir, bool create_root_domain)
{
	if (initialized)
		return true;

	initialized = true;
	Deployment::platform_dir = g_strdup (platform_dir);

	current_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
	pthread_key_create (&tls_key, NULL);
	pthread_mutex_init (&hash_mutex, NULL);
	
#if MONO_ENABLE_APP_DOMAIN_CONTROL
	if (create_root_domain) {
		const gchar *trace_options;
		const gchar *moon_path;
		const gchar *profiler;
#if DEBUG
		const gchar *soft_debug;
#endif

#if DEBUG && SANITY
		// Install signal handlers for crash reporting
		// Note that this only works if mono hasn't been 
		// initialized yet (i.e. this must not be done
		// for mopen, etc).
		moonlight_install_signal_handlers ();
#endif

#if DEBUG
		printf ("Moonlight: Enabling MONO_DEBUG=keep-delegates,reverse-pinvoke-exceptions and MOONLIGHT_ENABLE_CONSOLE=1\n");
		g_setenv ("MONO_DEBUG", "keep-delegates,reverse-pinvoke-exceptions", false);
		g_setenv ("MOONLIGHT_ENABLE_CONSOLE", "1", false);
#endif

		mono_config_parse (NULL);
		
		/* if a platform directory is provided then we're running inside the browser and CoreCLR should be enabled */
		if (platform_dir) {
			// confine mono itself to the platform directory, default GAC is relative to mono_assembly_getrootdir
			// which we set to 'platform_dir' then disable (unset) any extra GAC directory
			security_enable_coreclr (platform_dir);
			g_setenv ("MONO_PATH", platform_dir, true);
			g_unsetenv ("MONO_GAC_PREFIX");
		} else {
			moon_path = g_getenv ("MOON_PATH");
			if (moon_path != NULL && moon_path [0] != 0) {
				printf ("Setting moonlight root directory to: %s\n", moon_path);
				mono_assembly_setrootdir (moon_path);
			}
		}

		trace_options = g_getenv ("MOON_TRACE");
		if (trace_options != NULL){
			printf ("Setting trace options to: %s\n", trace_options);
			mono_jit_set_trace_options (trace_options);
		}
	
		profiler = g_getenv ("MOON_PROFILER");
		if (profiler != NULL) {
			printf ("Setting profiler to: %s\n", profiler);
			if (!strcmp ("gchandle", profiler)) {
#if OBJECT_TRACKING
				Deployment::profiler = new MonoProfiler ();
#endif
			} else {
				mono_profiler_load (profiler);
			}
		}

		mono_set_signal_chaining (true);

#if DEBUG
		soft_debug = g_getenv ("MOON_SOFT_DEBUG");
		if (soft_debug != NULL) {
			gchar *opt = g_strdup_printf ("--debugger-agent=%s", soft_debug);
			mono_jit_parse_options (1, &opt);

			g_free (opt);
		}
#endif
		mono_debug_init (MONO_DEBUG_FORMAT_MONO);
		mono_set_break_policy (moonlight_should_insert_breakpoint);
	
		root_domain = mono_jit_init_version ("Moonlight Root Domain", "moonlight");
		mono_add_internal_call ("Mono.NativeMethods::event_object_get_managed_object", (const void *)event_object_get_managed_object);

		LOG_DEPLOYMENT ("Deployment::Initialize (): Root domain is %p\n", root_domain);
	}
	else {
#endif
		root_domain = mono_domain_get ();
		mono_add_internal_call ("Mono.NativeMethods::event_object_get_managed_object", (const void *)event_object_get_managed_object);

		Deployment::desktop_deployment = new Deployment ();
		Deployment::desktop_deployment->InitializeDesktop (root_domain);
		Deployment::SetCurrent (Deployment::desktop_deployment);
		Deployment::desktop_deployment->EnsureManagedPeer ();

		// we need to call this here so that the application's
		// and surface's managed peer actually gets created.
		// otherwise mopen ends up with an NRE doing
		// Surface.get_Native.
		Deployment::desktop_deployment->InitializeAppDomain ("System.Windows, Version=3.0.0.0, Culture=neutral, PublicKeyToken=0738eb9f132ed756");

		Application *desktop_app = MoonUnmanagedFactory::CreateApplication ();
		desktop_deployment->SetCurrentApplication (desktop_app);
#if MONO_ENABLE_APP_DOMAIN_CONTROL
	}
#endif

	return true;
}

void
Deployment::SetSurface (Surface *surface)
{
	Surface *old;
	
	VERIFY_MAIN_THREAD;
	
	surface_mutex.Lock ();
	old = this->surface;
	this->surface = surface;
	if (this->surface)
		this->surface->ref ();
	surface_mutex.Unlock ();
	
	if (old)
		old->unref (); /* unref with the mutex unlocked */
}

Surface *
Deployment::GetSurface ()
{
	VERIFY_MAIN_THREAD;
	return surface;
}

Surface *
Deployment::GetSurfaceReffed ()
{
	Surface *result;
	
	surface_mutex.Lock ();
	result = this->surface;
	if (result)
		result->ref ();
	surface_mutex.Unlock ();
	
	return result;
}

void
Deployment::SetHttpHandler (HttpHandler *handler)
{
	VERIFY_MAIN_THREAD;
	if (http_handler != NULL)
		http_handler->unref ();
	http_handler = handler;
	if (http_handler != NULL)
		http_handler->ref ();
}

void
Deployment::SetDefaultHttpHandler (HttpHandler *handler)
{
	VERIFY_MAIN_THREAD;
	if (default_http_handler != NULL)
		default_http_handler->unref ();
	default_http_handler = handler;
	if (default_http_handler != NULL)
		default_http_handler->ref ();
}

HttpRequest *
Deployment::CreateHttpRequest (HttpRequest::Options options)
{
	HttpRequest *result = NULL;

	VERIFY_MAIN_THREAD;

	/* We must not create any http requests after shutdown has started */
	if (is_network_stopped)
		return result;

	if (http_handler != NULL)
		result = http_handler->CreateRequest (options);

	if (result == NULL) {
#if HAVE_CURL
		if (default_http_handler == NULL)
			default_http_handler = new CurlHttpHandler ();
#endif

		if (default_http_handler != NULL)
			result = default_http_handler->CreateRequest (options);
	}

	if (result != NULL)
		http_requests.Append (new HttpRequestNode (result));

	return result;
}

Downloader *
Deployment::CreateDownloader ()
{
	VERIFY_MAIN_THREAD;
	return new Downloader ();
}

void
Deployment::RegisterThread ()
{
	LOG_DEPLOYMENT ("Deployment::RegisterThread () thread: %p\n", (void*) pthread_self ());
	mono_thread_attach (root_domain);
}

void
Deployment::UnregisterThread ()
{
	LOG_DEPLOYMENT ("Deployment::UnregisterThread () thread: %p\n", (void*) pthread_self ());
	mono_thread_detach (mono_thread_current ());
}

Deployment*
Deployment::GetCurrent()
{
	Deployment *deployment;
	MonoDomain *current_domain;

	if (!initialized)
		return NULL;

	deployment = (Deployment *) pthread_getspecific (tls_key);
	current_domain = mono_domain_get ();

	/*
	 * If we dont have a Deployment* in the TLS slot then we are in a thread created
	 * by mono.  In this case we look up in the hsah table the deployment against 
	 * the current appdomain
	 */ 
	if (deployment == NULL && current_domain != NULL) {
		if (current_domain != NULL) {
			pthread_mutex_lock (&hash_mutex);
			deployment = (Deployment *) g_hash_table_lookup (current_hash, current_domain);
			pthread_mutex_unlock (&hash_mutex);
			pthread_setspecific (tls_key, deployment);
			LOG_DEPLOYMENT ("Deployment::GetCurrent (): Couldn't find deployment in our tls, searched current domain %p and found: %p\n", current_domain, deployment);
		}
	}

	/*
	 * If we have a domain mismatch, we likely got here from managed land and need
	 * to get the deployment tied to this domain
	 */
	if (deployment) {
		bool mismatch;
		if (current_domain == NULL) {
			/* this may happen for threads which are not registered with managed code (audio threads for instance). Everything ok. */
			mismatch = false;
		} else if (current_domain == root_domain) {
			if (deployment->domain == NULL) {
				/* we're in a deployment whose domain has been unloaded (but we're in the right deployment) */
				mismatch = false;
			} else {
				/* something is very wrong, I can't see how this can happen */
				//g_warning ("Deployment::GetCurrent (): Domain mismatch, but the current domain is the root domain?");
				mismatch = false;
			}
		} else {
			if (deployment->domain == NULL) {
				/* we switched from a deployment whose domain has been unloaded to a normal deployment */
				mismatch = true;
			} else if (deployment->domain != current_domain) {
				/* we're in the wrong deployment: our tls entry is wrong, most likely because we got here on a managed thread */
				mismatch = true;
			} else {
				/* everything ok */
				mismatch = false;
			}
		}			
		
		if (mismatch) {
			LOG_DEPLOYMENT ("Deployment::GetCurrent (): Domain mismatch, thread %u, (tls) deployment: %p, deployment->domain: %p, (mono_domain_get) current_domain: %p, root_domain: %p, hash deployment: %p\n",
				(int) pthread_self (), deployment, deployment->domain, current_domain, root_domain, g_hash_table_lookup (current_hash, current_domain));
			pthread_mutex_lock (&hash_mutex);
			if (current_hash != NULL)
				deployment = (Deployment *) g_hash_table_lookup (current_hash, current_domain);
			pthread_mutex_unlock (&hash_mutex);
			
			/* Fixup our tls entry */
			if (deployment != NULL) {
				pthread_setspecific (tls_key, deployment);
			}
		}
	}

	if (deployment == NULL) {
		// Currently this happens because we end up here during libmoon initialization.
		// The fix is to not create objects as default values for our static dependency properties.
		LOG_DEPLOYMENT ("Deployment::GetCurrent (): Didn't find a deployment. This should never happen.\n");
	}

	return deployment;
}

void
Deployment::SetCurrent (Deployment* deployment)
{
	SetCurrent (deployment, true);
}

void
Deployment::SetCurrent (Deployment* deployment, bool domain)
{
#if DEBUG
	if (deployment && mono_domain_get () != deployment->domain) {
		LOG_DEPLOYMENT ("Deployment::SetCurrent (%p), thread: %i domain mismatch, is: %p\n", deployment, (int) pthread_self (), mono_domain_get ());
	} else if (pthread_getspecific (tls_key) != deployment) {
		LOG_DEPLOYMENT ("Deployment::SetCurrent (%p), thread: %i deployment mismatch, is: %p\n", deployment, (int) pthread_self (), pthread_getspecific (tls_key));
	}
#endif
	
	if (domain) {
		if (deployment != NULL && deployment->domain != NULL) {
			mono_domain_set (deployment->domain, TRUE);
		} else {
			mono_domain_set (root_domain, TRUE);
		}
	}
	pthread_setspecific (tls_key, deployment);
}

Deployment::Deployment()
	: DependencyObject (this, Type::DEPLOYMENT), current_app (this, CurrentApplicationWeakRef)
{
	system_windows_image = NULL;
	system_windows_assembly = NULL;

	ensure_managed_peer = NULL;

	moon_load_xaml = NULL;
	moon_initialize_deployment_xap = NULL;
	moon_initialize_deployment_xaml = NULL;
	moon_destroy_application = NULL;
	moon_exception = NULL;
	moon_exception_message = NULL;
	moon_exception_error_code = NULL;

#if DEBUG
	moon_sources = NULL;
#endif	
	
#if EVENT_ARG_REUSE
	num_outstanding_changes = 0;
	change_args = NULL;
#endif
	surface = NULL;
	medias = NULL;
	keepalive = NULL;
	appdomain_initialized = false;
	appdomain_initialization_result = false;
	is_initializing = false;
	is_shutting_down = false;
	is_network_stopped = false;
	deployment_count++;
	appdomain_unloaded = false;
	system_windows_assembly = NULL;
	system_windows_deployment = NULL;
	deployment_shutdown = NULL;
	shutdown_state = Running; /* 0 */
	is_loaded_from_xap = false;
	xap_location = NULL;
	xap_filename = NULL;
	pending_unrefs = NULL;
	pending_loaded = false;
	objects_created = 0;
	objects_destroyed = 0;
	http_handler = NULL;
	default_http_handler = NULL;
	domain = NULL;

#if OBJECT_TRACKING
	objects_alive = NULL;
#endif

	font_manager = NULL;
	interned_strings = NULL;
	types = NULL;
}

void
Deployment::InitializeCommon ()
{
#if EVENT_ARG_REUSE
	change_args = g_ptr_array_new ();
#endif
#if OBJECT_TRACKING
	pthread_mutex_init (&objects_alive_mutex, NULL);
#endif
	pthread_setspecific (tls_key, this);

	pthread_mutex_lock (&hash_mutex);
	g_hash_table_insert (current_hash, domain, this);
	pthread_mutex_unlock (&hash_mutex);
	
	font_manager = new FontManager ();
	types = new Types ();
	types->Initialize ();

	interned_strings = g_hash_table_new_full (g_str_hash, g_str_equal,
						  (GDestroyNotify)g_free, (GDestroyNotify)g_free);
}

void
Deployment::InitializeDesktop (MonoDomain *domain)
{
	this->domain = domain;
	InitializeCommon ();
}

void
Deployment::Initialize ()
{
	MonoDomain *current = mono_domain_get ();

#if MONO_ENABLE_APP_DOMAIN_CONTROL
	mono_domain_set (root_domain, FALSE);
	domain = mono_domain_create_appdomain ((char *) "Silverlight AppDomain", NULL);

	LOG_DEPLOYMENT ("Deployment::Deployment (): Created domain %p for deployment %p\n", domain, this);

	mono_domain_set (domain, FALSE);
#endif

	InitializeCommon ();

	mono_domain_set (current, FALSE);
}

ErrorEventArgs *
Deployment::ManagedExceptionToErrorEventArgs (MonoObject *exc)
{
	int errorCode = -1;
	char* message = NULL;

	if (mono_object_isinst (exc, mono_get_exception_class())) {
		MonoObject *ret = mono_property_get_value (moon_exception_message, exc, NULL, NULL);

		message = mono_string_to_utf8 ((MonoString*)ret);
	}
	if (mono_object_isinst (exc, moon_exception)) {
		MonoObject *ret = mono_property_get_value (moon_exception_error_code, exc, NULL, NULL);

		errorCode = *(int*) mono_object_unbox (ret);
	}
	
	// FIXME: we need to figure out what type of exception it is
	// and map it to the right MoonError::ExceptionType enum
	return new ErrorEventArgs (RuntimeError, MoonError (MoonError::EXCEPTION, errorCode, message));
}

void
Deployment::ManagedExceptionToMoonError (MonoObject *exc, MoonError::ExceptionType type, MoonError *error)
{
	int errorCode = -1;
	char* message = NULL;

	if (mono_object_isinst (exc, mono_get_exception_class())) {
		MonoObject *ret = mono_property_get_value (moon_exception_message, exc, NULL, NULL);

		message = mono_string_to_utf8 ((MonoString*)ret);
	}
	if (mono_object_isinst (exc, moon_exception)) {
		MonoObject *ret = mono_property_get_value (moon_exception_error_code, exc, NULL, NULL);

		errorCode = *(int*) mono_object_unbox (ret);
	}

	MoonError::FillIn (error, type, errorCode, message);
}

static void
unref_kept_alive_object (gpointer obj)
{
	((EventObject *) obj)->unref_delayed ();
}

void
Deployment::SetKeepAlive (EventObject *object, bool value)
{
	// Don't keep anything alive after shutdown
	if (is_shutting_down)
		return;

	keepalive_mutex.Lock ();
	if (value) {
		if (!keepalive)
			keepalive = g_hash_table_new_full (g_direct_hash, g_direct_equal, unref_kept_alive_object, NULL);

		object->ref ();
		g_hash_table_insert (keepalive, object, NULL);
	} else {
		if (keepalive)
			g_hash_table_remove (keepalive, object);
	}
	keepalive_mutex.Unlock ();
}

void
Deployment::EnsureManagedPeer ()
{
	EnsureManagedPeer(this);
}

void
Deployment::EnsureManagedPeer (EventObject *forObj)
{
	Type::Kind kind = forObj->GetObjectType ();
	if (ensure_managed_peer) {
		ensure_managed_peer (forObj, kind);
	}
	else if (moon_ensure_managed_peer) {
		MonoObject *exc = NULL;
		void *params [2];

		Deployment::SetCurrent (this);

		params [0] = &forObj;
		params [1] = &kind;
		mono_runtime_invoke (moon_ensure_managed_peer, NULL, params, &exc);

		if (exc)
			surface->EmitError (ManagedExceptionToErrorEventArgs (exc));
	}
}

GCHandle
Deployment::CreateManagedXamlLoader (gpointer plugin_instance, XamlLoader* native_loader, const Uri *resourceBase)
{
	MonoObject *loader;
	MonoObject *exc = NULL;
	GCHandle resource_base;

	if (resourceBase)
		resource_base = resourceBase->GetGCHandle ();
	
	if (moon_load_xaml == NULL)
		return GCHandle::Zero;

	void *params [6];
	Surface *surface = GetSurface ();

	Deployment::SetCurrent (this);

	params [0] = &native_loader;
	params [1] = &plugin_instance;
	params [2] = &surface;
	params [3] = &resource_base;
	loader = mono_runtime_invoke (moon_load_xaml, NULL, params, &exc);

	if (exc) {
		surface->EmitError (ManagedExceptionToErrorEventArgs (exc));
		return GCHandle::Zero;
	}

	return GCHandle (mono_gchandle_new (loader, false));
}

void
Deployment::DestroyManagedXamlLoader (GCHandle xaml_loader)
{
	FreeGCHandle (xaml_loader);
}

void
Deployment::FreeGCHandle (GCHandle gchandle)
{
	if (!gchandle.IsAllocated ())
		return;

	/* If shutdown has started, the gchandle we have might have been freed automatically,
	 * and actually reused by another managed object in another appdomain. So don't free
	 * any gchandles after shutdown has started. */

	if (Surface::InMainThread () && IsShuttingDown ()) {
		/* If we're not on the main thread, our appdomain is still alive. */
		return;
	}

	mono_gchandle_free (gchandle.ToInt ());
}

GCHandle
Deployment::CloneGCHandle (GCHandle gchandle)
{
	if (!gchandle.IsAllocated ())
		return GCHandle::Zero;

	return GCHandle (mono_gchandle_new (mono_gchandle_get_target (gchandle.ToInt ()), false));
}

GCHandle
Deployment::CreateWeakGCHandle (void *mono_object)
{
	if (mono_object == NULL)
		return GCHandle::Zero;

	return GCHandle (mono_gchandle_new_weakref ((MonoObject *) mono_object, false));
}

GCHandle
Deployment::CreateGCHandle (void *mono_object)
{
	if (mono_object == NULL)
		return GCHandle::Zero;

	return GCHandle (mono_gchandle_new ((MonoObject *) mono_object, false));
}

void *
Deployment::GetGCHandleTarget (GCHandle handle)
{
	return mono_gchandle_get_target (handle.ToInt ());
}

void
Deployment::DestroyManagedApplication (gpointer plugin_instance)
{
	if (moon_destroy_application == NULL)
		return;

	MonoObject *exc = NULL;
	void *params [1];
	params [0] = &plugin_instance;

	Deployment::SetCurrent (this);

	mono_runtime_invoke (moon_destroy_application, NULL, params, &exc);

	if (exc)
		GetSurface()->EmitError (ManagedExceptionToErrorEventArgs (exc));
}

bool
Deployment::InitializeAppDomain ()
{
	return InitializeAppDomain ("System.Windows, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e");
}

bool
Deployment::InitializeAppDomain (const char *system_windows_fullname)
{
	bool result = false;

	LOG_DEPLOYMENT ("Deployment::InitializeAppDomain ('%s'): initialized: %i result: %s\n",
		system_windows_fullname, appdomain_initialized, appdomain_initialized ? (appdomain_initialization_result ? "OK" : "FAILED") : "N/A");

	if (appdomain_initialized)
		return appdomain_initialization_result;

	system_windows_assembly = mono_assembly_load_with_partial_name (system_windows_fullname, NULL);

	if (system_windows_assembly) {
		MonoClass *app_launcher;

		system_windows_image = mono_assembly_get_image (system_windows_assembly);
		
		LOG_DEPLOYMENT ("Assembly: %s\n", mono_image_get_filename (system_windows_image));
		
		app_launcher = mono_class_from_name (system_windows_image, "Mono", "ApplicationLauncher");
		if (!app_launcher) {
			fprintf (stderr, "Moonlight: Plugin AppDomain Creation Failure: could not find ApplicationLauncher type.\n");
			goto completed;
		}

		moon_exception = mono_class_from_name (system_windows_image, "Mono", "MoonException");
		if (!moon_exception) {
			fprintf (stderr, "Moonlight: Plugin AppDomain Creation Failure: could not find MoonException type.\n");
			goto completed;
		}
		
		moon_load_xaml  = MonoGetMethodFromName (app_launcher, "CreateXamlLoader", -1);
		moon_ensure_managed_peer  = MonoGetMethodFromName (app_launcher, "EnsureManagedPeer", -1);
		moon_initialize_deployment_xap   = MonoGetMethodFromName (app_launcher, "InitializeDeployment", 4);
		moon_initialize_deployment_xaml   = MonoGetMethodFromName (app_launcher, "InitializeDeployment", 2);
		moon_destroy_application = MonoGetMethodFromName (app_launcher, "DestroyApplication", -1);

		if (moon_load_xaml == NULL || moon_ensure_managed_peer == NULL || moon_initialize_deployment_xap == NULL || moon_initialize_deployment_xaml == NULL || moon_destroy_application == NULL) {
			fprintf (stderr, "Moonlight: Plugin AppDomain Creation Failure: lookup of ApplicationLauncher methods failed.\n");
			goto completed;
		}

		moon_exception_message = MonoGetPropertyFromName (mono_get_exception_class(), "Message");
		moon_exception_error_code = MonoGetPropertyFromName (moon_exception, "ErrorCode");

		if (moon_exception_message == NULL || moon_exception_error_code == NULL) {
			fprintf (stderr, "Moonlight: Plugin AppDomain Creation Failure: lookup of MoonException properties failed.\n");
			goto completed;
		}

		if (!InitializeManagedXamlParser (system_windows_image)) {
			fprintf (stderr, "Moonlight: Plugin AppDomain Creation Failure: unable to initialize the managed xaml parser.\n");
			goto completed;
		}

		result = true;
	} else {
		fprintf (stderr, "Moonlight: Plugin AppDomain Creation Failure: could not find assembly '%s'.\n", system_windows_fullname);
	}

#if DEBUG
	printf ("Moonlight: Plugin AppDomain Creation: %s\n", result ? "OK" : "Failed");
#endif

completed:
	appdomain_initialization_result = result;
	appdomain_initialized = true;

	if (result)
		EnsureManagedPeer ();

	return result;
}

bool
Deployment::InitializeManagedXamlParser (MonoImage *system_windows_image)
{
	mono_xaml_parser = mono_class_from_name (system_windows_image, "Mono.Xaml", "XamlParser");
	if (!mono_xaml_parser) {
		g_warning ("Could not find XamlParser type.");
		return false;
	}

	
	mono_xaml_parser_create_from_file = MonoGetMethodFromName (mono_xaml_parser, "CreateFromFile", 3);
	if (!mono_xaml_parser_create_from_file)
		return false;

	mono_xaml_parser_create_from_string = MonoGetMethodFromName (mono_xaml_parser, "CreateFromString", 4);
	if (!mono_xaml_parser_create_from_string)
		return false;

	mono_xaml_parser_hydrate_from_string = MonoGetMethodFromName (mono_xaml_parser, "HydrateFromString", 4);
	if (!mono_xaml_parser_hydrate_from_string)
		return false;

	return true;
}

Value *
Deployment::MonoXamlParserCreateFromFile (const char *file, bool create_namescope, bool validate_templates, MoonError *error)
{
	Value *v;
	void *params [3];
	MonoObject *ret;
	MonoObject *exc = NULL;

	Deployment::SetCurrent (this);

	params [0] = mono_string_new (mono_domain_get (), file);
	params [1] = &create_namescope;
	params [2] = &validate_templates;

	ret = mono_runtime_invoke (mono_xaml_parser_create_from_file, NULL, params, &exc);

	if (exc) {
		ManagedExceptionToMoonError (exc, MoonError::XAML_PARSE_EXCEPTION, error);
		return NULL;
	}

	// Note, we need to free the one we get from managed, but not delete.
	v = (Value *) mono_object_unbox (ret);
	Value *clone = new Value (*v);
	Value::FreeValue (v);
	return clone;
}

Value *
Deployment::MonoXamlParserCreateFromString (const char *xaml, bool create_namescope, bool validate_templates, MoonError *error, DependencyObject* owner)
{
	Value *v;
	void *params [4];
	MonoObject *ret;
	MonoObject *exc = NULL;

	Deployment::SetCurrent (this);

	params [0] = mono_string_new (mono_domain_get (), xaml);
	params [1] = &create_namescope;
	params [2] = &validate_templates;
	params [3] = &owner;

	ret = mono_runtime_invoke (mono_xaml_parser_create_from_string, NULL, params, &exc);

	if (exc) {
		ManagedExceptionToMoonError (exc, MoonError::XAML_PARSE_EXCEPTION, error);
		return NULL;
	}

	// Note, we need to free the one we get from managed, but not delete.
	v = (Value *) mono_object_unbox (ret);
	Value *clone = new Value (*v);
	Value::FreeValue (v);
	return clone;
}

Value *
Deployment::MonoXamlParserHydrateFromString (const char *xaml, Value *obj, bool create_namescope, bool validate_templates, MoonError *error)
{
	Value *v;
	void *params [4];
	MonoObject *ret;
	MonoObject *exc = NULL;

	Deployment::SetCurrent (this);

	params [0] = mono_string_new (mono_domain_get (), xaml);
	params [1] = obj;
	params [2] = &create_namescope;
	params [3] = &validate_templates;

	ret = mono_runtime_invoke (mono_xaml_parser_hydrate_from_string, NULL, params, &exc);

	if (exc) {
		ManagedExceptionToMoonError (exc, MoonError::XAML_PARSE_EXCEPTION, error);
		return NULL;
	}

	// Note, we need to free the one we get from managed, but not delete.
	v = (Value *) mono_object_unbox (ret);
	Value *clone = new Value (*v);
	Value::FreeValue (v);
	return clone;
}

bool
Deployment::InitializeManagedDeployment (gpointer plugin_instance, const char *culture, const char *uiCulture)
{
	if (moon_initialize_deployment_xap == NULL && moon_initialize_deployment_xaml)
		return false;

	void *params [4];
	MonoObject *ret;
	MonoObject *exc = NULL;

	Deployment::SetCurrent (this);

	if (GetXapFilename() != NULL) {
		params [0] = &plugin_instance;
		params [1] = mono_string_new (mono_domain_get (), GetXapFilename());
		params [2] = culture ? mono_string_new (mono_domain_get (), culture) : NULL;
		params [3] = uiCulture ? mono_string_new (mono_domain_get (), uiCulture) : NULL;
		ret = mono_runtime_invoke (moon_initialize_deployment_xap, NULL, params, &exc);
	} else {
		params [0] = culture ? mono_string_new (mono_domain_get (), culture) : NULL;
		params [1] = uiCulture ? mono_string_new (mono_domain_get (), uiCulture) : NULL;
		ret = mono_runtime_invoke (moon_initialize_deployment_xaml, NULL, params, &exc);
	}
	
	if (exc) {
		GetSurface()->EmitError (ManagedExceptionToErrorEventArgs (exc));
		return false;
	}

	return (bool) (*(MonoBoolean *) mono_object_unbox (ret));
}

MonoMethod *
Deployment::MonoGetMethodFromName (MonoClass *klass, const char *name, int narg)
{
	MonoMethod *method;
	method = mono_class_get_method_from_name (klass, name, narg);

	if (!method)
		g_warning ("Could not find method: %s", name);

	return method;
}

MonoProperty *
Deployment::MonoGetPropertyFromName (MonoClass *klass, const char *name)
{
	MonoProperty *property;
	property = mono_class_get_property_from_name (klass, name);

	if (!property)
		g_warning ("Could not find property: %s", name);

	return property;
}

#if OBJECT_TRACKING
static int
IdComparer (gconstpointer base1, gconstpointer base2)
{
	int id1 = (*(EventObject **) base1)->GetId ();
	int id2 = (*(EventObject **) base2)->GetId ();

	int iddiff = id1 - id2;
	
	if (iddiff == 0)
		return 0;
	else if (iddiff < 0)
		return -1;
	else
		return 1;
}

static int
ByTypeComparer (gconstpointer base1, gconstpointer base2, gpointer user_data)
{
	GHashTable *by_type = (GHashTable *) user_data;
	char *left = *((char **) base1);
	char *right = *((char **) base2);

	int iddiff =  GPOINTER_TO_INT (g_hash_table_lookup (by_type, left)) - GPOINTER_TO_INT (g_hash_table_lookup (by_type, right));

	if (iddiff == 0)
		return 0;
	else if (iddiff < 0)
		return -1;
	else
		return 1;
}

static void
accumulate_last_n (gpointer key,
		   gpointer value,
		   gpointer user_data)
{
	GPtrArray *last_n = (GPtrArray*)user_data;
	g_ptr_array_insert_sorted (last_n, IdComparer, key);
}

static void
accumulate_by_type (gpointer key,
		   gpointer value,
		   gpointer user_data)
{
	GHashTable *by_type = (GHashTable*) user_data;
	EventObject *ob = (EventObject *) key;
	int count = GPOINTER_TO_INT (g_hash_table_lookup (by_type, ob->GetType ()->GetName ())) + 1;
	g_hash_table_insert (by_type, (void*)ob->GetType ()->GetName (), GINT_TO_POINTER (count));
}

static void
add_keys_to_array (gpointer key, gpointer value, gpointer user_data)
{
	GPtrArray *by_type = (GPtrArray*) user_data;
	g_ptr_array_add (by_type, key);
}
#endif

Deployment::~Deployment()
{
	delete font_manager;
	
	LOG_DEPLOYMENT ("Deployment::~Deployment (): %p\n", this);

#if SANITY
	if (pending_unrefs != NULL)
		g_warning ("Deployment::~Deployment (): There are still pending unrefs.");
	if (medias != NULL)
		g_warning ("Deployment::~Deployment (): There are still medias waiting to get disposed.");
#endif

#if OBJECT_TRACKING
	pthread_mutex_destroy (&objects_alive_mutex);
	if (objects_alive != NULL)
		g_hash_table_destroy (objects_alive);
#endif

	if (types != NULL) {
		types->DeleteProperties ();
		delete types;
		types = NULL;
	}

#if DEBUG
	delete moon_sources;
	moon_sources = NULL;
#endif


#if PROPERTY_LOOKUP_DIAGNOSTICS
	printf ("at Deployment::dtor time, there were %lld property lookups\n", provider_property_lookups);
#endif

	if (this == Deployment::GetCurrent())
		Deployment::SetCurrent(NULL);

	deployment_count--;
}

#if OBJECT_TRACKING
void 
Deployment::ReportLeaks ()
{
	printf ("Deployment leak report for %p/%i: leaked %i objects (%.1f%%) ", this, this->GetId (), objects_created - objects_destroyed, (100.0 * objects_destroyed) / objects_created);
	float used = (float)(mono_gc_get_used_size () / 1024.0 / 1024.0);
	float total = (float)(mono_gc_get_heap_size () / 1024.0 / 1024.0);
	printf ("Managed heap used: %.2f MB. Managed heap total: %.2f MB\n", used, total);

	if (objects_created != objects_destroyed) {
		printf ("\tObjects created: %i\n", objects_created);
		printf ("\tObjects destroyed: %i\n", objects_destroyed);

		GPtrArray* last_n = g_ptr_array_new ();
		GHashTable *by_type = g_hash_table_new (g_str_hash, g_str_equal);;
		GPtrArray *top_n_by_type = g_ptr_array_new ();

		pthread_mutex_lock (&objects_alive_mutex);
		g_hash_table_foreach (objects_alive, accumulate_last_n, last_n);
		g_hash_table_foreach (objects_alive, accumulate_by_type, by_type);
		g_hash_table_foreach (by_type, add_keys_to_array, top_n_by_type);

		g_ptr_array_sort_with_data (top_n_by_type, ByTypeComparer, by_type);
		pthread_mutex_unlock (&objects_alive_mutex);


		bool strong_handled = false;
		guint32 counter = 10;
		const char *counter_str = getenv ("MOONLIGHT_OBJECT_TRACKING_COUNTER");
		if (counter_str != NULL) {
			if (strcmp (counter_str, "all") == 0) {
				counter = G_MAXUINT32;
			} else if (strcmp (counter_str, "strong") == 0) {
				// Only care about objects which are immortal (have a strong gchandle)
				strong_handled = true;
				counter = last_n->len;
			} else {
				counter = atoi (counter_str);
			}
		}
		counter = MIN(counter, last_n->len);
		if (counter) {
			printf ("\tOldest %d objects alive:\n", counter);
			for (uint i = 0; i < MIN (counter, last_n->len); i ++) {
				EventObject* obj = (EventObject *) last_n->pdata [i];
				GCHandle gchandle = obj->GetManagedHandle ();
				bool is_weak_handle = gchandle.IsWeak ();
				if (obj->GetRefCount () == 1 && !is_weak_handle && gchandle.IsAllocated ())
					printf ("**** Refcount of 1 with a strong handle ****\n");
				if (strong_handled && obj->GetRefCount () < 2)
					continue;
				printf ("\t\t%p\t%i = %s, refcount: %i\n", obj, obj->GetId (), obj->GetTypeName (), obj->GetRefCount ());
				if (getenv ("MOONLIGHT_OBJECT_TRACK_STORE_REPORT") != NULL)
					show_reftrace (obj);
			}
		}

		printf ("Leaked objects by type:\n");
		for (int i = 0; i < (int) top_n_by_type->len; i++) {
			printf ("\t%d instances leaked of type %s\n", GPOINTER_TO_INT (g_hash_table_lookup (by_type, top_n_by_type->pdata [i])), (char *) top_n_by_type->pdata [i]);
		}
		g_ptr_array_free (top_n_by_type, true);
		g_ptr_array_free (last_n, true);
		g_hash_table_destroy (by_type);
	}
}
#endif

#if DEBUG
void
Deployment::AddSource (const Uri *uri, const char *filename)
{
	moon_source *src = new moon_source ();
	src->uri = g_strdup (uri->ToString ());
	src->filename = g_strdup (filename);
	if (moon_sources == NULL)
		moon_sources = new List ();
	moon_sources->Append (src);
}

List*
Deployment::GetSources ()
{
	return moon_sources;
}
#endif


const char*
Deployment::InternString (const char *str)
{
	if (str == NULL)
		return NULL;

	char *interned = NULL;
	gpointer unused;

	// first look it up in our hash
	if (!g_hash_table_lookup_extended (interned_strings,
					   str,
					   (gpointer*)&interned,
					   &unused)) {
		interned = g_strdup (str);
		g_hash_table_insert (interned_strings, interned, NULL);
	}

	return interned;
}

#if EVENT_ARG_REUSE
PropertyChangedEventArgs *
Deployment::GetPropertyChangedEventArgs ()
{
	if (num_outstanding_changes >= change_args->len) {
		for (int i = 0; i < 10; i ++)
			g_ptr_array_add (change_args, new PropertyChangedEventArgs ());
	}

	return (PropertyChangedEventArgs*)g_ptr_array_index (change_args, num_outstanding_changes ++);
}

void
Deployment::ReleasePropertyChangedEventArgs (PropertyChangedEventArgs *args)
{
	args->SetProperty (NULL);
	args->SetId (0);
	args->SetOldValue (NULL);
	args->SetNewValue (NULL);
	num_outstanding_changes --;
}
#endif

void
Deployment::Reinitialize ()
{
	http_requests.Clear (true);
	AssemblyPartCollection * parts = MoonUnmanagedFactory::CreateAssemblyPartCollection ();
	SetParts (parts);
	parts->unref ();
#if DEBUG
	if (moon_sources)
		moon_sources->Clear (true);
#endif
}

bool
Deployment::IsShuttingDown ()
{
	VERIFY_MAIN_THREAD;
	return is_shutting_down;
}

void
Deployment::Dispose ()
{
	Surface *surface;
	LOG_DEPLOYMENT ("Deployment::Dispose (): %p\n", this);

	Deployment::SetCurrent (this);

	surface_mutex.Lock ();
	surface = this->surface;
	this->surface = NULL;
	surface_mutex.Unlock ();
	if (surface)
		surface->unref ();

#if EVENT_ARG_REUSE
	if (change_args) {
		for (guint i = 0; i < change_args->len; i ++) {
			((PropertyChangedEventArgs*)g_ptr_array_index (change_args, i))->unref();
		}
		g_ptr_array_free (change_args, true);
		change_args = NULL;
	}
#endif

	if (interned_strings)
		g_hash_table_destroy (interned_strings);
	interned_strings = NULL;

#if OBJECT_TRACKING
	if (getenv ("MOONLIGHT_OBJECT_TRACK_IMMORTALS") != NULL) {
		printf ("Deployment disposing, with %i leaked EventObjects.\n", objects_created - objects_destroyed);
		if (objects_created != objects_destroyed)
			ReportLeaks ();
	}
#endif
	DependencyObject::Dispose ();
}

void
Deployment::Shutdown ()
{
#if DEBUG
	printf ("Moonlight: Shutting down\n");
#endif
	LOG_DEPLOYMENT ("Deployment::Shutdown ()\n");

	/*
	 * Shutting down is a complicated process with quite a few pitfalls.
	 * The current process is as follows:
	 * - Abort all downloaders. Firefox has a habit of calling into our
	 *   downloader callbacks in bad moments, aborting all downloaders
	 *   will prevent this from happening. We need to do this *before*
	 *   setting the 'is_shutting_down' flag, since aborting downloaders
	 *   may cause (important) events to be raised (events which may have
	 *   managed handlers). To stop new network requests from being created
	 *   from now on, we use a 'is_network_stopped' flag.
	 * - Ensure nothing is executed on the media threadpool threads and
	 *   audio threads.
	 * - Unload our appdomain. We still have code executing on separate
	 *   threads (user code can have threads, and there is always the
	 *   finalizer thread).
	 * - The browser plugin is freed (the plugin needs to go away after
	 *   after the appdomain, since managed code has lots of pointers
	 *   to the plugin instance).
	 * - By now everything should have gotten unreffed, and the final object
	 *   to be deleted is the deployment (every other object references the
	 *   deployment to ensure this).
	 */

	is_network_stopped = true;

	if (http_handler != NULL) {
		http_handler->Dispose ();
		http_handler->unref ();
		http_handler = NULL;
	}

	if (default_http_handler != NULL) {
		default_http_handler->Dispose ();
		default_http_handler->unref ();
		default_http_handler = NULL;
	}

	AbortAllHttpRequests ();

	is_shutting_down = true;
	
	g_return_if_fail (!IsDisposed ());

	Emit (ShuttingDownEvent);

	/*
	 * Dispose all Media instances so that we can be sure nothing is executed
	 * on the media threadpool threads after this point.
	 * This will also stop all media from playing, so there should be no audio
	 * threads doing anything either (note that there might be both media
	 * threadpool threads and audio threads still alive, just not executing
	 * anything related to this deployment).
	 */
	DisposeAllMedias ();
	
	// Detach all loaded handlers we may have, they cause circular refs
	RemoveMatchingHandlers (Deployment::LoadedEvent, NULL, NULL);

	/* The objects kept alive must now be allowed to die */
	keepalive_mutex.Lock ();
	if (keepalive != NULL) {
		g_hash_table_destroy (keepalive);
		keepalive = NULL;
	}
	keepalive_mutex.Unlock ();

	if (current_app != NULL) {
		current_app->Dispose ();
		current_app = NULL;
	}

	StringNode *node;
	while ((node = (StringNode *) paths.First ())) {
		RemoveDir (node->str);
		g_free (node->str);
		paths.Remove (node);
	}

	if (GetParts ())
		SetParts (NULL);

	if (GetValue (NameScope::NameScopeProperty))
		SetValue (NameScope::NameScopeProperty, NULL);

#if MONO_ENABLE_APP_DOMAIN_CONTROL
	if (system_windows_assembly == NULL) {
		/* this can happen if initialization fails, i.e. xap downloading fails for instance */
		shutdown_state = DisposeDeployment; /* skip managed shutdown entirely, since managed code wasn't initialized */
	} else {
		shutdown_state = CallManagedShutdown;
	}
	this->ref (); /* timemanager is dead, so we need to add timeouts directly to pal */
	Runtime::GetWindowingSystem ()->AddTimeout (MOON_PRIORITY_DEFAULT, 1, ShutdownManagedCallback, this);
#endif

	if (types)
		types->Dispose ();

#if OBJECT_TRACKING
	if (Deployment::profiler)
		Deployment::profiler->DumpStrongGCHandles ();

	if (getenv ("MOONLIGHT_OBJECT_TRACK_IMMORTALS") != NULL) {
		printf ("Deployment shutting down, with %i leaked EventObjects.\n", objects_created - objects_destroyed);
		if (objects_created != objects_destroyed)
			ReportLeaks ();
	}
#endif
}

#if MONO_ENABLE_APP_DOMAIN_CONTROL
bool
Deployment::ShutdownManagedCallback (gpointer user_data)
{
	return ((Deployment *) user_data)->ShutdownManaged ();
}

bool
Deployment::ShutdownManaged ()
{
	if (domain == root_domain) {
		fprintf (stderr, "Moonlight: Can't unload the root domain!\n");
		this->unref (); /* the ref taken in Shutdown */
		return false;
	}
	
	VERIFY_MAIN_THREAD;
	
	/*
	 * Managed shutdown is complicated, with a few gotchas:
	 * - managed finalizers are run on a separate thread (multi-threaded issues)
	 * - after the appdomain has unloaded, we can't call into it anymore (for 
	 *   instance using function pointers into managed delegates).
	 * 
	 * To do have a safe shutdown we have two different approaches:
	 * 
	 * 1) Protect the function pointers in native code with mutex, both during
	 *    callbacks and when setting them. This has the drawback of having a
	 *    mutex locked during a potentially long time (the mutex is always
	 *    locked while executing the callback), and the advantage that the 
	 *    callbacks can be executed from any thread and the cleanup can be done
	 *    directly in the managed dtor.
	 *    ExternalDemuxer uses this approach.
	 * 
	 * 2) If the callbacks will only be executed on the main thread, we can
	 *    avoid the native locks ensuring that everything related to the
	 *    callbacks will be done on the main thread by doing the following:
	 *    - During execution we keep a list in managed code of cleanup actions
	 *      to execute upon shutdown. If a managed object is finalized during
	 *      normal execution, it removes any applicable actions from the list.
	 *      This list is protected with a lock, so it can be accessed from all
	 *      threads (main thread + finalizer thread).
	 *    - When shutdown is requested, we set a flag to disallow further
	 *      additions to the list, and execute all the cleanup actions.
	 *    There are two cases where the managed finalizer is executed:
	 *    a) Normal execution, in this case the native object has one ref left
	 *       (the one ToggleRef has), so it is guaranteed that nobody can call
	 *       the callbacks anymore -> no cleanup is needed in the managed dtor.
	 *    b) Shutdown, in this case the cleanup code has already been executed
	 *       (by Deployment.Shutdown), which again means that no cleanup is
	 *       needed in the managed dtor.
	 *    This approach only works if the callbacks are only called on the main
	 *    thread (since otherwise there is a race condition between calling the
	 *    callbacks and cleaning them up). It also only works for ToggleReffed/
	 *    refcounted objects.
	 *    MultiScaleTileSource uses this approach.
	 */
	
	LOG_DEPLOYMENT ("Deployment::ShutdownManaged (): shutdown_state: %i, appdomain: %p, deployment: %p\n", shutdown_state, domain, this);
	
	Deployment::SetCurrent (this, true);
		
	switch (shutdown_state) {
	case Running:        /*  0  */
		/* this shouldn't happen */
	case ShutdownFailed: /* -1 */ {
		/* There has been an error during shutdown and we can't continue shutting down */
		fprintf (stderr, "Moonlight: Shutdown aborted due to unexpected error(s)\n");
		this->unref (); /* the ref taken in Shutdown */
		return false;
	}
	case CallManagedShutdown: /* 1 */{
		/* Call the managed System.Windows.Deployment:Shutdown method */
		MonoObject *ret;
		MonoObject *exc = NULL;
		bool result;

		if (system_windows_image == NULL) {
			shutdown_state = ShutdownFailed;
			fprintf (stderr, "Moonlight: Can't find the System.Windows.dll image.\n");
			break;
		}
		
		if (system_windows_assembly == NULL) {
			shutdown_state = ShutdownFailed;
			fprintf (stderr, "Moonlight: Can't find the System.Windows.Deployment's assembly.\n");
			break;
		}
		
		if (system_windows_deployment == NULL) {
			system_windows_deployment = mono_class_from_name (system_windows_image, "System.Windows", "Deployment");
			if (system_windows_deployment == NULL) {
				shutdown_state = ShutdownFailed;
				fprintf (stderr, "Moonlight: Can't find the System.Windows.Deployment class.\n");
				break;
			}
		}
		
		if (deployment_shutdown == NULL) {
			deployment_shutdown = mono_class_get_method_from_name (system_windows_deployment, "Shutdown", 0);
			if (deployment_shutdown == NULL) {
				shutdown_state = ShutdownFailed;
				fprintf (stderr, "Moonlight: Can't find the System.Windows.Deployment:Shutdown method.\n");
				break;
			}
		}
		
		ret = mono_runtime_invoke (deployment_shutdown, NULL, NULL, &exc);
	
		if (exc) {
			shutdown_state = ShutdownFailed;
			fprintf (stderr, "Moonlight: Exception while cleaning up managed code.\n");  // TODO: print exception message/details
			break;
		}
	
		result = (bool) (*(MonoBoolean *) mono_object_unbox (ret));
		
		if (!result) {
			/* Managed code isn't ready to shutdown quite yet, try again later */
			break;
		}
		
		/* Managed shutdown successfully completed */
		LOG_DEPLOYMENT ("Deployment::ShutdownManaged (): managed call to Deployment:Shutdown () on domain %p succeeded.\n", domain);
		
		shutdown_state = UnloadDomain;
		/* fall through */
	}
	case UnloadDomain: /* 2 */ {
		MonoException *exc = NULL;
		
		/*
		 * When unloading an appdomain, all threads in that appdomain are aborted.
		 * This includes the main thread. According to Paolo it's safe if we first
		 * switch to the root domain (and there are no managed frames on the stack,
		 * which is guaranteed since we're in a glib timeout).
		 */
		mono_domain_set (root_domain, TRUE);
		
		/* Unload the domain */
		mono_domain_try_unload (domain, (MonoObject **) &exc);

		if (exc) {
			shutdown_state = ShutdownFailed;
			MonoObject *msg = mono_property_get_value (moon_exception_message, exc, NULL, NULL);
			char *message = mono_string_to_utf8 ((MonoString *) msg);
			fprintf (stderr, "Moonlight: Exception while unloading appdomain: %s\n", message);
			g_free (message);
			break;
		}

		/* Set back to our current domain while emitting AppDomainUnloadedEvent */
		mono_domain_set (domain, TRUE);
		appdomain_unloaded = true;
		Emit (Deployment::AppDomainUnloadedEvent);
			
		/* Remove the domain from the hash table (since from now on the same ptr may get reused in subsquent calls to mono_domain_create_appdomain) */
		pthread_mutex_lock (&hash_mutex);
		g_hash_table_remove (current_hash, domain);
		pthread_mutex_unlock (&hash_mutex);

		/* Since the domain ptr may get reused we have to leave the root domain as the current domain */
		mono_domain_set (root_domain, TRUE);
		
		/* Clear out the domain ptr to detect any illegal uses asap */
		/* CHECK: do we need to call mono_domain_free? */
		domain = NULL;

		/* AppDomain successfully unloaded */
		LOG_DEPLOYMENT ("Deployment::ShutdownManaged (): appdomain successfully unloaded.\n");
		
		shutdown_state = DisposeDeployment;
		/* fall through */
	}
	case DisposeDeployment: /* 3 */{
		LOG_DEPLOYMENT ("Deployment::ShutdownManaged (): managed code has shutdown successfully, calling Dispose.\n");
		Dispose ();
		this->unref (); /* the ref taken in Shutdown */
		return false;
	}
	}
	
	return true; /* repeat the callback, we're not done yet */
}
#endif

class LoadedClosure {
public:
	UIElement *obj;
	int token;

	LoadedClosure (UIElement *obj, int token)
		: obj (obj), token (token)
	{

	}

	~LoadedClosure ()
	{
		
	}
};

void
Deployment::delete_loaded_closure (gpointer closure)
{
	LoadedClosure *c = (LoadedClosure *) closure;
	c->obj->unref ();
	delete c;
}

bool
Deployment::match_loaded_closure (int token, EventHandler cb_handler, gpointer cb_data, gpointer data)
{
	LoadedClosure *closure_to_match = (LoadedClosure*)data;
	LoadedClosure *closure = (LoadedClosure*)cb_data;

	return (closure_to_match->obj == closure->obj &&
		closure_to_match->token == closure->token);

}

void
Deployment::proxy_loaded_event (EventObject *sender, EventArgs *arg, gpointer closure)
{
	Deployment *deployment  = (Deployment *) sender;
	LoadedClosure *lclosure = (LoadedClosure*)closure;

// FIXME: in a perfect world this would be all that was needed, but
// there are times we don't do the tree walk to add handlers to the
// deployment at all, so elements won't have their
// OnLoaded/InvokeLoaded called at all.

// 	if (!lclosure->obj->IsLoaded ())
// 		lclosure->obj->OnLoaded ();


	lclosure->obj->EmitOnly (UIElement::LoadedEvent, lclosure->token);
	deployment->RemoveHandler (Deployment::LoadedEvent, proxy_loaded_event, lclosure);
}

void
Deployment::add_loaded_handler (EventObject *obj, int token, gpointer closure)
{
	Deployment *deployment = (Deployment*)closure;
	LoadedClosure *lclosure = new LoadedClosure ((UIElement*)obj, token);

	// This is unrefed in delete_loaded_closure
	obj->ref ();
	deployment->AddHandler (Deployment::LoadedEvent, proxy_loaded_event, lclosure, delete_loaded_closure);
}

void
Deployment::remove_loaded_handler (EventObject *obj, int token, gpointer closure)
{
	Deployment *deployment = (Deployment*)closure;
	LoadedClosure *lclosure = new LoadedClosure ((UIElement*)obj, token);
	deployment->RemoveMatchingHandlers (Deployment::LoadedEvent, match_loaded_closure, lclosure);
	delete lclosure;
}

void
Deployment::AddAllLoadedHandlers (UIElement *el, bool only_new)
{
	el->ForeachHandler (UIElement::LoadedEvent, only_new, add_loaded_handler, this);
}

void
Deployment::RemoveAllLoadedHandlers (UIElement *el)
{
	el->ForeachHandler (UIElement::LoadedEvent, false, remove_loaded_handler, this);
}

void
Deployment::AddLoadedHandler (UIElement *el, int token)
{
	el->ForHandler (UIElement::LoadedEvent, token, add_loaded_handler, this);
}

void
Deployment::RemoveLoadedHandler (UIElement *el, int token)
{
	el->ForHandler (UIElement::LoadedEvent, token, remove_loaded_handler, this);
}

void
Deployment::EmitLoaded ()
{
	// Sometimes we need to force emission of loaded events, like when flushing
	// the splashscreen events.
	Emit (Deployment::LoadedEvent, NULL, true);
}

void
Deployment::EmitLoadedAsync ()
{
	if (GetSurface ()->IsLoaded ())
		EmitAsync (Deployment::LoadedEvent, NULL, true);
}

void
Deployment::LayoutUpdated ()
{
	Emit (Deployment::LayoutUpdatedEvent);
}

FontManager *
Deployment::GetFontManager ()
{
	return font_manager;
}

GlyphTypefaceCollection *
Deployment::GetSystemTypefaces ()
{
	return font_manager->GetSystemGlyphTypefaces ();
}

Application*
Deployment::GetCurrentApplication ()
{
	return current_app;
}

void
Deployment::SetCurrentApplication (Application* value)
{
	current_app = value;
}

void
Deployment::SetEnsureManagedPeerCallback (EnsureManagedPeerCallback callback)
{
	this->ensure_managed_peer = callback;
}

void
Deployment::UnregisterHttpRequest (HttpRequest *request)
{
	HttpRequestNode *node = (HttpRequestNode *) http_requests.First ();
	while (node != NULL) {
		if (node->request == request) {
			node->request = NULL;
			http_requests.Remove (node);
			return;
		}
		node = (HttpRequestNode *) node->next;
	}
}

void
Deployment::AbortAllHttpRequests ()
{
	http_requests.Clear (true);
}

class MediaNode : public List::Node {
private:
	Media *media;
	
public:
	MediaNode (Media *media)
	{
		this->media = media;
		this->media->ref ();
	}
	void Clear (bool dispose)
	{
		if (media) {
			if (dispose)
				media->DisposeObject (media);
			media->unref ();
			media = NULL;
		}
	}
	virtual ~MediaNode ()
	{
		Clear (true);
	}
	Media *GetMedia () { return media; }
};

bool
Deployment::RegisterMedia (EventObject *media)
{
	bool result;

	LOG_DEPLOYMENT ("Deployment::RegisterMedia (%p)\n", media);

	medias_mutex.Lock ();
	if (is_shutting_down) {
		result = false;
	} else {
		if (medias == NULL)
			medias = new List ();
		medias->Append (new MediaNode ((Media *) media));
		result = true;
	}
	medias_mutex.Unlock ();

	return result;
}

void
Deployment::UnregisterMedia (EventObject *media)
{
	MediaNode *node = NULL;
	
	LOG_DEPLOYMENT ("Deployment::UnregisterMedia (%p)\n", media);

	medias_mutex.Lock ();
	if (medias != NULL) {
		node = (MediaNode *) medias->First ();
		while (node != NULL) {
			if (node->GetMedia () == media) {
				medias->Unlink (node);
				break;
			}
			node = (MediaNode *) node->next;
		}
	}
	medias_mutex.Unlock ();
	
	/* Don't delete with the lock held, it may reenter and dead-lock */
	if (node) {
		node->Clear (false);
		delete node;
	}
}

void
Deployment::DisposeAllMedias ()
{
	List *list;
	
	medias_mutex.Lock ();
	list = medias;
	medias = NULL;
	medias_mutex.Unlock ();
	
	/* Don't delete with the lock held, it may reenter and dead-lock */
	delete list; /* the node destructor calls Dispose on the media */
	
	MediaThreadPool::WaitForCompletion (this);
}

struct UnrefData {	
	EventObject *obj;
	UnrefData *next;
};

bool
Deployment::DrainUnrefs (gpointer context)
{
	Deployment *deployment = (Deployment *) context;
	Deployment::SetCurrent (deployment);
	deployment->DrainUnrefs ();
	deployment->unref ();
	//Deployment::SetCurrent (NULL);
	return false;
}

void
Deployment::DrainUnrefs ()
{
	UnrefData *list;
	UnrefData *next;
#if DEBUG
	bool processed_data = false;
#endif
	
loop:
	// Get the list of objects to unref.
	do {
		list = (UnrefData *) g_atomic_pointer_get (&pending_unrefs);
		
		if (list == NULL)
			break;
		
	} while (!g_atomic_pointer_compare_and_exchange (&pending_unrefs, list, NULL));
	
	// Loop over all the objects in the list and unref them.
	while (list != NULL) {
#if DEBUG
		processed_data = true;
#endif
		list->obj->unref ();
		next = list->next;
		g_free (list);
		list = next;
	}

	list = (UnrefData *) g_atomic_pointer_get (&pending_unrefs);
	if (list != NULL)
		goto loop;
	
#if DEBUG
	if (processed_data && IsDisposed ()) {
		static const char *leak_log = NULL;

		if (leak_log == NULL) {
			leak_log = getenv ("MOONLIGHT_LEAK_LOG");
			if (leak_log == NULL)
				leak_log = "";
		}

		if (leak_log != NULL && leak_log [0] != 0) {
			FILE *log = fopen (leak_log, "a");
			/* Make sure the test doesn't pass if we can't create a leak log, so that it doesn't go unnoticed */
			g_assert (log); /* #if DEBUG */
			fprintf (log, "Deployment: %p/%i, objects created: %i, objects destroyed: %i, objects leaked: %i\n",
				this, this->GetId (), objects_created, objects_destroyed, objects_created - objects_destroyed);
			fclose (log);
		}
#if OBJECT_TRACKING
		printf ("Moonlight: the current deployment (%p) has detected that probably no more objects will get freed on this deployment.\n", this);
		ReportLeaks ();
#endif
	}
#endif
}

void
Deployment::UnrefDelayed (EventObject *obj)
{
	UnrefData *list;
	UnrefData *item;
		
#if SANITY
	if (Deployment::GetCurrent () != this && obj->GetObjectType () != Type::DEPLOYMENT)
		g_warning ("Deployment::UnrefDelayed (%p): The current deployment (%p) should be %p.", obj, Deployment::GetCurrent (), this);
	if (obj->GetObjectType () != Type::DEPLOYMENT &&  obj->GetUnsafeDeployment () != this && obj->GetUnsafeDeployment () != NULL)
		g_warning ("Deployment::UnrefDelayed (%p): obj's deployment %p should be %p. type: %s", obj, obj->GetUnsafeDeployment (), this, obj->GetTypeName ());
#endif

	// Create the new list item
	item = (UnrefData *) g_malloc (sizeof (UnrefData));
	item->obj = obj;
	
	// Prepend the list item into the list
	do {
		list = (UnrefData *) g_atomic_pointer_get (&pending_unrefs);
		item->next = list;
	} while (!g_atomic_pointer_compare_and_exchange (&pending_unrefs, list, item));
	
	// If we created a new list instead of prepending to an existing one, add a idle tick call.
	if (list == NULL) { // don't look at item->next, item might have gotten freed already.
		Runtime::GetWindowingSystem ()->AddIdle (DrainUnrefs, (gpointer)this);
		ref (); // keep us alive until we've processed the unrefs.
	}
}

void
Deployment::TrackObjectCreated (EventObject *obj)
{
	g_atomic_int_inc (&objects_created);

#if OBJECT_TRACKING
	pthread_mutex_lock (&objects_alive_mutex);
	if (objects_alive == NULL)
		objects_alive = g_hash_table_new (g_direct_hash, g_direct_equal);
	g_hash_table_insert (objects_alive, obj, GINT_TO_POINTER (1));
	pthread_mutex_unlock (&objects_alive_mutex);
#endif
}

void
Deployment::TrackObjectDestroyed (EventObject *obj)
{
	g_atomic_int_inc (&objects_destroyed);
	
#if OBJECT_TRACKING
	pthread_mutex_lock (&objects_alive_mutex);
	g_hash_table_remove (objects_alive, obj);
	pthread_mutex_unlock (&objects_alive_mutex);
#endif
}

bool
Deployment::IsLoadedFromXap ()
{
	return is_loaded_from_xap;
}

void
Deployment::SetIsLoadedFromXap (bool flag)
{
	is_loaded_from_xap = flag;
}

void
Deployment::SetInitialization (bool init)
{
	if (!is_initializing) {
		// this is a "set once" property, we set it to its default in case it was not set as part of the manifest
		if (!GetValueNoDefault (Deployment::ExternalCallersFromCrossDomainProperty))
			SetExternalCallersFromCrossDomain (CrossDomainAccessNoAccess);
	}
	is_initializing = init;
}

void
Deployment::SetXapLocation (const Uri *location)
{
	if (is_shutting_down)
		return;

	delete xap_location;
	xap_location = Uri::Clone (location);
}

const Uri*
Deployment::GetXapLocation ()
{
	return xap_location;
}

const Uri *
Deployment::GetSourceLocation (bool *is_xap)
{
	const Uri *result;
	bool xap = false;

	if (GetCurrentApplication ()->IsRunningOutOfBrowser ()) {
		result = GetSurface ()->GetSourceLocation ();
		xap = true;
	} else {
		result = GetXapLocation ();
		if (result == NULL) {
			result = GetSurface ()->GetSourceLocation ();
		} else {
			xap = true;
		}
	}

	if (is_xap != NULL)
		*is_xap = xap;

	return result;
}

void
Deployment::SetXapFilename (const char *filename)
{
	g_free (xap_filename);
	xap_filename = g_strdup (filename);
}

const char*
Deployment::GetXapFilename ()
{
	return xap_filename;
}

void
Deployment::TrackPath (char *path)
{
	paths.Append (new StringNode (path));
}

gint32
Deployment::GetDeploymentCount ()
{
	return deployment_count;
}

char *
Deployment::CanonicalizeFileName (const char *filename, bool is_xap_mode)
{
	MonoObject *exc = NULL;
	MonoObject *result;
	MonoClass *mono_helper;
	MonoMethod *canonicalize_filename;
	void *params [2];

	LOG_DEPLOYMENT ("Deployment::CanonicalizeFileName (%s, %i)\n", filename, is_xap_mode);

	mono_helper = mono_class_from_name (system_windows_image, "Mono", "Helper");
	if (mono_helper == NULL) {
		printf ("Moonlight: Could not find managed Mono.Helper type.\n");
		return g_strdup (filename);
	}

	canonicalize_filename  = MonoGetMethodFromName (mono_helper, "CanonicalizeFileName", 2);

	if (!canonicalize_filename) {
		printf ("Moonlight: Could not find managed method Mono.Helper.CanonicalizeFileName.\n");
		return g_strdup (filename);
	}

	params [0] = mono_string_new (domain, filename);
	params [1] = &is_xap_mode;
	result = mono_runtime_invoke (canonicalize_filename, NULL, params, &exc);

	if (exc) {
		printf ("Moonlight: Exception in call to Mono.Helper.CanonicalizeFileName.\n");
		// surface->EmitError (ManagedExceptionToErrorEventArgs (exc));
		return g_strdup (filename);
	}

	char *r = mono_string_to_utf8 ((MonoString *) result);

	LOG_DEPLOYMENT ("Deployment::CanonicalizeFileName (%s, %i) => %s\n", filename, is_xap_mode, r);

	return r;
}

void
Deployment::SetUriFunctions (const UriFunctions *value)
{
	memcpy (&uri_functions, value, sizeof (UriFunctions));
}

bool
Deployment::VerifyDownload (const char *filename)
{
	static MonoMethod *moon_codec_integrity = NULL;

	if (!moon_codec_integrity) {
		MonoAssembly *sw = mono_assembly_load_with_partial_name ("System.Windows, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e", NULL);
		if (!sw)
			return false;

		MonoImage *image = mono_assembly_get_image (sw);
		if (!image)
			return false;

		MonoClass *klass = mono_class_from_name (image, "Mono", "Helper");
		if (!klass)
			return false;

		moon_codec_integrity = mono_class_get_method_from_name (klass, "CheckFileIntegrity", 1);
		if (!moon_codec_integrity)
			return false;
	}

	void *params [1];
	params [0] = mono_string_new (mono_domain_get (), filename);
	MonoObject *exc = NULL;

	MonoObject *ret = mono_runtime_invoke (moon_codec_integrity, NULL, params, &exc);
	if (exc)
		return false;

	return (bool) (*(MonoBoolean *) mono_object_unbox (ret));
}

/*
 * AssemblyPart
 */

AssemblyPart::AssemblyPart ()
{
	SetObjectType (Type::ASSEMBLYPART);
}

AssemblyPart::~AssemblyPart ()
{
}

AssemblyPartCollection::AssemblyPartCollection ()
{
	SetObjectType (Type::ASSEMBLYPART_COLLECTION);
}

AssemblyPartCollection::~AssemblyPartCollection ()
{
}

/*
 * ExtensionPart
 */

ExtensionPart::ExtensionPart ()
{
	SetObjectType (Type::EXTENSIONPART);
}

ExtensionPart::~ExtensionPart ()
{
}

ExternalPart::ExternalPart ()
{
	SetObjectType (Type::EXTERNALPART);
}

ExternalPart::~ExternalPart ()
{
}

ExternalPartCollection::ExternalPartCollection ()
{
	SetObjectType (Type::EXTERNALPART_COLLECTION);
}

ExternalPartCollection::~ExternalPartCollection ()
{
}

/* OutOfBrowserSettings */
OutOfBrowserSettings::OutOfBrowserSettings ()
{
	SetObjectType (Type::OUTOFBROWSERSETTINGS);
}

OutOfBrowserSettings::~OutOfBrowserSettings ()
{
}

/* WindowSettings */
WindowSettings::WindowSettings ()
{
	SetObjectType (Type::WINDOWSETTINGS);
}

WindowSettings::~WindowSettings ()
{
}

/* Icon */
Icon::Icon ()
{
	SetObjectType (Type::ICON);
}

Icon::~Icon ()
{
}

/* IconCollection */
IconCollection::IconCollection ()
{
	SetObjectType (Type::ICON_COLLECTION);
}

IconCollection::~IconCollection ()
{
}

};

#if OBJECT_TRACKING
_MonoProfiler::_MonoProfiler ()
{
	type_name = g_getenv ("GCHANDLES_FOR_TYPE");

	gchandles = g_ptr_array_new ();
	stacktraces = g_ptr_array_new_with_free_func (g_free);

	mono_profiler_install (this, profiler_shutdown);
	mono_profiler_install_gc_roots (track_gchandle, NULL);
	mono_profiler_set_events (MONO_PROFILE_GC_ROOTS);
}

void
MonoProfiler::profiler_shutdown (MonoProfiler *prof)
{

}

void
MonoProfiler::track_gchandle (MonoProfiler *prof, int op, int type, uintptr_t handle, MonoObject *obj)
{
	// Ignore anything that isn't a strong GC handle
	if (type != 2)
		return;

	prof->locker.Lock ();

	GPtrArray *gchandles = prof->gchandles;
	GPtrArray *stacktraces = prof->stacktraces;

	if (op == MONO_PROFILER_GC_HANDLE_CREATED) {
		g_ptr_array_add (gchandles, (gpointer) handle);
		if (prof->type_name && !strcmp (prof->type_name, mono_class_get_name (mono_object_get_class(obj))))
			g_ptr_array_add (stacktraces, get_stack_trace ());
		else
			g_ptr_array_add (stacktraces, NULL);
	} else if (op == MONO_PROFILER_GC_HANDLE_DESTROYED) {
		for (int i = 0; i < (int)gchandles->len; i++) {
			if (g_ptr_array_index (gchandles, i) == (gpointer) handle) {
				g_ptr_array_remove_index_fast (gchandles, i);
				g_ptr_array_remove_index_fast (stacktraces, i);
				break;
			}
		}
	}

	prof->locker.Unlock ();
}


void
accumulate_g_ptr_array_by_type (gpointer data, gpointer user_data)
{
	GHashTable *by_type = (GHashTable*) user_data;
	MonoObject *ob = mono_gchandle_get_target (GPOINTER_TO_INT (data));
	if (!ob)
		return;

	const char *name = mono_class_get_name (mono_object_get_class(ob));
	int count = GPOINTER_TO_INT (g_hash_table_lookup (by_type, name)) + 1;
	g_hash_table_insert (by_type, (void*) name, GINT_TO_POINTER (count));
}


void
MonoProfiler::DumpStrongGCHandles ()
{
	GHashTable *by_type = g_hash_table_new (g_str_hash, g_str_equal);
	GPtrArray *top_n_by_type = g_ptr_array_new ();

	g_ptr_array_foreach (gchandles, accumulate_g_ptr_array_by_type, by_type);
	g_hash_table_foreach (by_type, Moonlight::add_keys_to_array, top_n_by_type);
	g_ptr_array_sort_with_data (top_n_by_type, Moonlight::ByTypeComparer, by_type);

	for (int i = 0; i < (int) top_n_by_type->len; i++) {
		printf ("\t%d instances GCHandled of type %s\n", GPOINTER_TO_INT (g_hash_table_lookup (by_type, top_n_by_type->pdata [i])), (char *) top_n_by_type->pdata [i]);
	}

	DumpTracesByType ();
}

void
MonoProfiler::DumpTracesByType ()
{
	if (!type_name)
		return;

	// For all allocated handles, see if any of them are referencing objects of the type
	// we care about. If they are, print out the allocation trace of all handles targetting
	// that object
	for (int i = 0; i < (int) gchandles->len; i ++) {
		MonoObject *obj = mono_gchandle_get_target (GPOINTER_TO_INT (g_ptr_array_index (gchandles, i)));
		if (!obj)
			continue;
		const char *name = mono_class_get_name (mono_object_get_class(obj));
		if (!strcmp (name, type_name)) {
			printf ("Strong GCHandles allocated for object %p:\n", obj);
			for (int j = i; j < (int) gchandles->len; j++) {
				if (mono_gchandle_get_target (GPOINTER_TO_INT (g_ptr_array_index (gchandles, j))) == obj) {
					printf ("%s\n\n", (char *) g_ptr_array_index (stacktraces, j));
					g_ptr_array_remove_index_fast (gchandles, j);
					g_ptr_array_remove_index_fast (stacktraces, j);
					j --;
				}
			}
			i --;
		}
	}
}
#endif /* OBJECT_TRACKING */
