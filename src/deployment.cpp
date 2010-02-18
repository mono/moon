/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * deployment.cpp: Deployment Class support
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <glib.h>

#include "downloader.h"
#include "deployment.h"
#include "timemanager.h"
#include "debug.h"
#include "utils.h"
#include "security.h"
#include "namescope.h"
#include "pipeline.h"

#include <stdlib.h>
#include <mono/jit/jit.h>
#include <mono/metadata/debug-helpers.h>
G_BEGIN_DECLS
/* because this header sucks */
#include <mono/metadata/mono-debug.h>
G_END_DECLS
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/profiler.h>

#include <mono/metadata/appdomain.h>

/*
 * Deployment
 */

gboolean Deployment::initialized = FALSE;
pthread_key_t Deployment::tls_key = 0;
pthread_mutex_t Deployment::hash_mutex;
GHashTable* Deployment::current_hash = NULL;
MonoDomain* Deployment::root_domain = NULL;
Deployment *Deployment::desktop_deployment = NULL;
gint32 Deployment::deployment_count = 0;

class IDownloaderNode : public List::Node {
public:
	IDownloader *dl;
	IDownloaderNode (IDownloader *dl)
	{
		this->dl = dl;
	}
	virtual ~IDownloaderNode ()
	{
		if (dl && !dl->IsAborted ())
			dl->Abort ();
	}
};


class StringNode : public List::Node {
public:
	char *str;

	StringNode (char *str) {
		this->str = g_strdup (str);
	}
};

bool
find_string (List::Node *node, void *data)
{
	StringNode *tp = (StringNode*)node;
	char *p = (char*)data;

	return !strcmp (tp->str, p);
}

static MonoBreakPolicy
moonlight_should_insert_breakpoint (MonoMethod *method)
{
	return MONO_BREAK_POLICY_ON_DBG;
}

bool
Deployment::Initialize (const char *platform_dir, bool create_root_domain)
{
	if (initialized)
		return true;

	initialized = true;

	current_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
	pthread_key_create (&tls_key, NULL);
	pthread_mutex_init (&hash_mutex, NULL);
	
	enable_vm_stack_trace ();

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
		printf ("Moonlight: Enabling MONO_DEBUG=keep-delegates.\n");
		g_setenv ("MONO_DEBUG", "keep-delegates", false);
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
			mono_profiler_load (profiler);
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
		
		LOG_DEPLOYMENT ("Deployment::Initialize (): Root domain is %p\n", root_domain);
	}
	else {
#endif
		root_domain = mono_domain_get ();

		Deployment::desktop_deployment = new Deployment (root_domain);
		Deployment::SetCurrent (Deployment::desktop_deployment);

		Application *desktop_app = new Application ();
		desktop_deployment->SetCurrentApplication (desktop_app);
#if MONO_ENABLE_APP_DOMAIN_CONTROL
	}
#endif

	return true;
}

void
Deployment::RegisterThread (Deployment *deployment)
{
	LOG_DEPLOYMENT ("Deployment::RegisterThread (): Deployment: %p Domain: %p\n", deployment, deployment->domain);
	mono_thread_attach (deployment->domain);
}

Deployment*
Deployment::GetCurrent()
{
	Deployment *deployment = (Deployment *) pthread_getspecific (tls_key);
	MonoDomain *current_domain = mono_domain_get ();

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
				//g_warning ("Deployment::GetCurrent (): Domain mismatch, but the current domain is the root domain?\n");
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

Deployment::Deployment (MonoDomain *domain)
	: DependencyObject (this, Type::DEPLOYMENT)
{
	this->domain = domain;
	InnerConstructor ();
}

Deployment::Deployment()
	: DependencyObject (this, Type::DEPLOYMENT)
{
	MonoDomain *current = mono_domain_get ();
#if MONO_ENABLE_APP_DOMAIN_CONTROL
	mono_domain_set (root_domain, FALSE);
	domain = mono_domain_create_appdomain ((char *) "Silverlight AppDomain", NULL);

	LOG_DEPLOYMENT ("Deployment::Deployment (): Created domain %p for deployment %p\n", domain, this);

	mono_domain_set (domain, FALSE);
#else
	domain = NULL;
#endif
	InnerConstructor ();

	mono_domain_set (current, FALSE);
}

void
Deployment::InnerConstructor ()
{
	system_windows_image = NULL;
	system_windows_assembly = NULL;

	moon_load_xaml = NULL;
	moon_initialize_deployment_xap = NULL;
	moon_initialize_deployment_xaml = NULL;
	moon_destroy_application = NULL;
	moon_exception = NULL;
	moon_exception_message = NULL;
	moon_exception_error_code = NULL;
	
	medias = NULL;
	is_shutting_down = false;
	deployment_count++;
	appdomain_unloaded = false;
	system_windows_assembly = NULL;
	system_windows_deployment = NULL;
	deployment_shutdown = NULL;
	shutdown_state = Running; /* 0 */
	is_loaded_from_xap = false;
	xap_location = NULL;
	current_app = NULL;
	pending_unrefs = NULL;
	pending_loaded = false;
	objects_created = 0;
	objects_destroyed = 0;
	
	types = NULL;

#if OBJECT_TRACKING
	objects_alive = NULL;
	pthread_mutex_init (&objects_alive_mutex, NULL);
#endif

	pthread_setspecific (tls_key, this);

	pthread_mutex_lock (&hash_mutex);
	g_hash_table_insert (current_hash, domain, this);
	pthread_mutex_unlock (&hash_mutex);
	
	font_manager = new FontManager ();
	types = new Types ();
	types->Initialize ();
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

gpointer
Deployment::CreateManagedXamlLoader (gpointer plugin_instance, XamlLoader* native_loader, const char *resourceBase, const char *file, const char *str)
{
	MonoObject *loader;
	MonoObject *exc = NULL;
	
	if (moon_load_xaml == NULL)
		return NULL;

	void *params [6];
	Surface *surface = GetSurface ();

	Deployment::SetCurrent (this);

	params [0] = &native_loader;
	params [1] = &plugin_instance;
	params [2] = &surface;
	params [3] = resourceBase ? mono_string_new (mono_domain_get (), resourceBase) : NULL;
	params [4] = file ? mono_string_new (mono_domain_get (), file) : NULL;
	params [5] = str ? mono_string_new (mono_domain_get (), str) : NULL;
	loader = mono_runtime_invoke (moon_load_xaml, NULL, params, &exc);

	if (exc) {
		surface->EmitError (ManagedExceptionToErrorEventArgs (exc));
		return NULL;
	}

	return GUINT_TO_POINTER (mono_gchandle_new (loader, false));
}

void
Deployment::DestroyManagedXamlLoader (gpointer xaml_loader)
{
	guint32 loader = GPOINTER_TO_UINT (xaml_loader);
	if (loader)
		mono_gchandle_free (loader);
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
	bool result = false;

	system_windows_assembly = mono_assembly_load_with_partial_name ("System.Windows, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e", NULL);
	
	if (system_windows_assembly) {
		MonoClass *app_launcher;

		result = true;

		system_windows_image = mono_assembly_get_image (system_windows_assembly);
		
		LOG_DEPLOYMENT ("Assembly: %s\n", mono_image_get_filename (system_windows_image));
		
		app_launcher = mono_class_from_name (system_windows_image, "Mono", "ApplicationLauncher");
		if (!app_launcher) {
			g_warning ("could not find ApplicationLauncher type");
			return false;
		}

		moon_exception = mono_class_from_name (system_windows_image, "Mono", "MoonException");
		if (!moon_exception) {
			g_warning ("could not find MoonException type");
			return false;
		}
		
		moon_load_xaml  = MonoGetMethodFromName (app_launcher, "CreateXamlLoader", -1);
		moon_initialize_deployment_xap   = MonoGetMethodFromName (app_launcher, "InitializeDeployment", 4);
		moon_initialize_deployment_xaml   = MonoGetMethodFromName (app_launcher, "InitializeDeployment", 2);
		moon_destroy_application = MonoGetMethodFromName (app_launcher, "DestroyApplication", -1);

		if (moon_load_xaml == NULL || moon_initialize_deployment_xap == NULL || moon_initialize_deployment_xaml == NULL || moon_destroy_application == NULL) {
			g_warning ("lookup for ApplicationLauncher methods failed");
			result = false;
		}

		moon_exception_message = MonoGetPropertyFromName (mono_get_exception_class(), "Message");
		moon_exception_error_code = MonoGetPropertyFromName (moon_exception, "ErrorCode");

		if (moon_exception_message == NULL || moon_exception_error_code == NULL) {
			g_warning ("lookup for MoonException properties failed");
			result = false;
		}
	} else {
		printf ("Moonlight: Plugin AppDomain Creation: could not find System.Windows.dll.\n");
	}

	printf ("Moonlight: Plugin AppDomain Creation: %s\n", result ? "OK" : "Failed");

	return result;
}

bool
Deployment::InitializeManagedDeployment (gpointer plugin_instance, const char *file, const char *culture, const char *uiCulture)
{
	if (moon_initialize_deployment_xap == NULL && moon_initialize_deployment_xaml)
		return false;

	void *params [4];
	MonoObject *ret;
	MonoObject *exc = NULL;

	Deployment::SetCurrent (this);

	if (file != NULL) {
		params [0] = &plugin_instance;
		params [1] = mono_string_new (mono_domain_get (), file);
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
		printf ("Warning could not find method %s\n", name);

	return method;
}

MonoProperty *
Deployment::MonoGetPropertyFromName (MonoClass *klass, const char *name)
{
	MonoProperty *property;
	property = mono_class_get_property_from_name (klass, name);

	if (!property)
		printf ("Warning could not find property %s\n", name);

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

static void
accumulate_last_n (gpointer key,
		   gpointer value,
		   gpointer user_data)
{
	GPtrArray *last_n = (GPtrArray*)user_data;

	g_ptr_array_insert_sorted (last_n, IdComparer, key);
}
#endif

Deployment::~Deployment()
{
	g_free (xap_location);
	
	delete font_manager;
	
	LOG_DEPLOYMENT ("Deployment::~Deployment (): %p\n", this);

#if SANITY
	if (pending_unrefs != NULL)
		g_warning ("Deployment::~Deployment (): There are still pending unrefs.\n");
	if (medias != NULL)
		g_warning ("Deployment::~Deployment (): There are still medias waiting to get disposed.\n");
#endif

#if OBJECT_TRACKING
	printf ("Deployment destroyed, with %i leaked EventObjects.\n", objects_created - objects_destroyed);
	if (objects_created != objects_destroyed)
		ReportLeaks ();
#elif DEBUG
	if (objects_created != objects_destroyed) {
		printf ("Deployment destroyed, with %i leaked EventObjects.\n", objects_created - objects_destroyed);
	}
#endif

#if OBJECT_TRACKING
	pthread_mutex_destroy (&objects_alive_mutex);
	g_hash_table_destroy (objects_alive);
#endif

	if (types != NULL) {
		types->DeleteProperties ();
		delete types;
		types = NULL;
	}
	
	deployment_count--;
}

#if OBJECT_TRACKING
void 
Deployment::ReportLeaks ()
{
	printf ("Deployment leak report:\n");
	if (objects_created == objects_destroyed) {
		printf ("\tno leaked objects.\n");
	} else {
		printf ("\tObjects created: %i\n", objects_created);
		printf ("\tObjects destroyed: %i\n", objects_destroyed);
		printf ("\tDifference: %i (%.1f%%)\n", objects_created - objects_destroyed, (100.0 * objects_destroyed) / objects_created);

		GPtrArray* last_n = g_ptr_array_new ();

		pthread_mutex_lock (&objects_alive_mutex);
		g_hash_table_foreach (objects_alive, accumulate_last_n, last_n);
		pthread_mutex_unlock (&objects_alive_mutex);

	 	uint counter = 10;
		counter = MIN(counter, last_n->len);
		if (counter) {
			printf ("\tOldest %d objects alive:\n", counter);
			for (uint i = 0; i < MIN (counter, last_n->len); i ++) {
				EventObject* obj = (EventObject *) last_n->pdata [i];
				printf ("\t\t%p\t%i = %s, refcount: %i\n", obj, obj->GetId (), obj->GetTypeName (), obj->GetRefCount ());
			}
		}

		g_ptr_array_free (last_n, true);
	}
}
#endif

void
Deployment::Reinitialize ()
{
	downloaders.Clear (true);
	AssemblyPartCollection * parts = new AssemblyPartCollection ();
	SetParts (parts);
	parts->unref ();
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
	LOG_DEPLOYMENT ("Deployment::Dispose (): %p\n", this);
	
	DependencyObject::Dispose ();
}

void
Deployment::Shutdown ()
{
	LOG_DEPLOYMENT ("Deployment::Shutdown ()\n");

	/*
	 * Shutting down is a complicated process with quite a few pitfalls.
	 * The current process is as follows:
	 * - Abort all downloaders. Firefox has a habit of calling into our
	 *   downloader callbacks in bad moments, aborting all downloaders
	 *   will prevent this from happening.
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
		
	is_shutting_down = true;
	
	g_return_if_fail (!IsDisposed ());

	Emit (ShuttingDownEvent);
	
	AbortAllDownloaders ();
	/*
	 * Dispose all Media instances so that we can be sure nothing is executed
	 * on the media threadpool threads after this point.
	 * This will also stop all media from playing, so there should be no audio
	 * threads doing anything either (note that there might be both media
	 * threadpool threads and audio threads still alive, just not executing
	 * anything related to this deployment).
	 */
	DisposeAllMedias ();
	
	if (current_app != NULL) {
		current_app->Dispose ();
		current_app->unref ();
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
	this->ref (); /* timemanager is dead, so we need to add timeouts directly to glib */
	g_timeout_add_full (G_PRIORITY_DEFAULT, 1, ShutdownManagedCallback, this, NULL);
#endif

	if (types)
		types->Dispose ();
}

#if MONO_ENABLE_APP_DOMAIN_CONTROL
gboolean
Deployment::ShutdownManagedCallback (gpointer user_data)
{
	return ((Deployment *) user_data)->ShutdownManaged ();
}

gboolean
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

		if (exc) {
			shutdown_state = ShutdownFailed;
			fprintf (stderr, "Moonlight: Exception while unloading appdomain.\n"); // TODO: print exception message/details
			break;
		}
		
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
	EventHandler handler;
	gpointer handler_data;

	LoadedClosure (UIElement *obj, EventHandler handler, gpointer handler_data)
		: obj (obj), handler (handler), handler_data (handler_data)
	{
	}
};

void
Deployment::delete_loaded_closure (gpointer closure)
{
	delete (LoadedClosure*)closure;
}

bool
Deployment::match_loaded_closure (EventHandler cb_handler, gpointer cb_data, gpointer data)
{
	LoadedClosure *closure_to_match = (LoadedClosure*)data;
	LoadedClosure *closure = (LoadedClosure*)cb_data;

	return (closure_to_match->obj == closure->obj &&
		closure_to_match->handler == closure->handler &&
		closure_to_match->handler_data == closure->handler_data);

}

void
Deployment::proxy_loaded_event (EventObject *sender, EventArgs *arg, gpointer closure)
{
	LoadedClosure *lclosure = (LoadedClosure*)closure;

// FIXME: in a perfect world this would be all that was needed, but
// there are times we don't do the tree walk to add handlers to the
// deployment at all, so elements won't have their
// OnLoaded/InvokeLoaded called at all.

// 	if (!lclosure->obj->IsLoaded ())
// 		lclosure->obj->OnLoaded ();

	if (lclosure->handler)
		lclosure->handler (lclosure->obj, new RoutedEventArgs (lclosure->obj), lclosure->handler_data);
}

void
Deployment::add_loaded_handler (EventObject *obj, EventHandler handler, gpointer handler_data, gpointer closure)
{
	Deployment *deployment = (Deployment*)closure;
	LoadedClosure *lclosure = new LoadedClosure ((UIElement*)obj,
						     handler, handler_data);
	deployment->AddHandler (Deployment::LoadedEvent, proxy_loaded_event, lclosure, delete_loaded_closure);
}

void
Deployment::remove_loaded_handler (EventObject *obj, EventHandler handler, gpointer handler_data, gpointer closure)
{
	Deployment *deployment = (Deployment*)closure;
	LoadedClosure *lclosure = new LoadedClosure ((UIElement*)obj,
						     handler, handler_data);
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
Deployment::emit_delayed_loaded (EventObject *data)
{
	Deployment *deployment = (Deployment*)data;
	deployment->EmitLoaded ();
}

void
Deployment::PostLoaded ()
{
 	if (pending_loaded)
 		return;
	GetSurface()->GetTimeManager()->AddTickCall (emit_delayed_loaded, this);
	pending_loaded = true;
}

void
Deployment::EmitLoaded ()
{
	if (pending_loaded) {
		GetSurface()->GetTimeManager()->RemoveTickCall (emit_delayed_loaded, this);
		pending_loaded = false;
	}
	Emit (Deployment::LoadedEvent, NULL, true);
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

Application*
Deployment::GetCurrentApplication ()
{
	return current_app;
}

void
Deployment::SetCurrentApplication (Application* value)
{
	if (current_app == value)
		return;

	if (current_app)
		current_app->unref ();

	current_app = value;

	if (current_app)
	  current_app->ref ();
}

void
Deployment::RegisterDownloader (IDownloader *dl)
{
	downloaders.Append (new IDownloaderNode (dl));
}

void
Deployment::UnregisterDownloader (IDownloader *dl)
{
	IDownloaderNode *node = (IDownloaderNode *) downloaders.First ();
	while (node != NULL) {
		if (node->dl == dl) {
			node->dl = NULL;
			downloaders.Remove (node);
			return;
		}
		node = (IDownloaderNode *) node->next;
	}
}

void
Deployment::AbortAllDownloaders ()
{
	downloaders.Clear (true);
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

gboolean
Deployment::DrainUnrefs (gpointer context)
{
	Deployment *deployment = (Deployment *) context;
	Deployment::SetCurrent (deployment);
	deployment->DrainUnrefs ();
	deployment->unref ();
	Deployment::SetCurrent (NULL);
	return false;
}

void
Deployment::DrainUnrefs ()
{
	UnrefData *list;
	UnrefData *next;
	
	// Get the list of objects to unref.
	do {
		list = (UnrefData *) g_atomic_pointer_get (&pending_unrefs);
		
		if (list == NULL)
			return;
		
	} while (!g_atomic_pointer_compare_and_exchange (&pending_unrefs, list, NULL));
	
	// Loop over all the objects in the list and unref them.
	while (list != NULL) {
		list->obj->unref ();
		next = list->next;
		g_free (list);
		list = next;
	}
	
#if OBJECT_TRACKING
	if (IsDisposed () && g_atomic_pointer_get (&pending_unrefs) == NULL && objects_destroyed != objects_created) {
		printf ("Moonlight: the current deployment (%p) has detected that probably no more objects will get freed on this deployment.\n", this);
		ReportLeaks ();
	}
#endif
}

void
Deployment::UnrefDelayed (EventObject *obj)
{
	UnrefData *list;
	UnrefData *item;
		
#if SANITY
	if (Deployment::GetCurrent () != this)
		g_warning ("Deployment::UnrefDelayed (%p): The current deployment (%p) should be %p.\n", obj, Deployment::GetCurrent (), this);
	if (obj->GetObjectType () != Type::DEPLOYMENT &&  obj->GetUnsafeDeployment () != this && obj->GetUnsafeDeployment () != NULL)
		g_warning ("Deployment::UnrefDelayed (%p): obj's deployment %p should be %p. type: %s\n", obj, obj->GetUnsafeDeployment (), this, obj->GetTypeName ());
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
		g_idle_add (DrainUnrefs, this);
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

	Track ("Destroyed", "");
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
Deployment::SetXapLocation (const char *location)
{
	g_free (xap_location);
	xap_location = g_strdup (location);
}

const char*
Deployment::GetXapLocation ()
{
	return xap_location;
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
