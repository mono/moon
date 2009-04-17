/* 
 * deployment.cpp: Deployment Class support
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include "downloader.h"
#include "deployment.h"
#include "debug.h"
#include "utils.h"
#include "security.h"

#include <stdlib.h>
#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
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

class IDownloaderNode : public List::Node {
public:
	IDownloader *dl;
	IDownloaderNode (IDownloader *dl) { this->dl = dl; }
};

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
		printf ("Moonlight: Enabling MONO_IOMAP=case.\n");
		g_setenv ("MONO_IOMAP", "case", false);
#endif

		mono_config_parse (NULL);
		
		/* if a platform directory is provided then we're running inside the browser and CoreCLR should be enabled */
		if (platform_dir) {
			security_enable_coreclr (platform_dir);

			/* XXX confine mono itself to the platform directory XXX incomplete */
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
		mono_debug_init (MONO_DEBUG_FORMAT_MONO);
	
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
		desktop_app->unref ();
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
	if (deployment && deployment->domain != current_domain && current_domain != NULL) {
		LOG_DEPLOYMENT ("Deployment::GetCurrent (): Domain mismatch, thread %i, current deployment's domain is %p, current domain is: %p\n", (int) pthread_self (), deployment->domain, current_domain);
		pthread_mutex_lock (&hash_mutex);
		deployment = (Deployment *) g_hash_table_lookup (current_hash, current_domain);
		pthread_mutex_unlock (&hash_mutex);
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
		if (deployment != NULL) {
			mono_domain_set (deployment->domain, FALSE);
		} else {
			mono_domain_set (root_domain, FALSE);
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
	char *domain_name = g_strdup_printf ("moonlight-%p", this);
	mono_domain_set (root_domain, FALSE);
	domain = mono_domain_create_appdomain (domain_name, NULL);
	g_free (domain_name);

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
	is_loaded_from_xap = false;
	xap_location = NULL;
	current_app = NULL;
	pending_unrefs = NULL;
	objects_created = 0;
	objects_destroyed = 0;
	
#if OBJECT_TRACKING
	objects_alive = NULL;
	pthread_mutex_init (&objects_alive_mutex, NULL);
#endif

	pthread_setspecific (tls_key, this);

	pthread_mutex_lock (&hash_mutex);
	g_hash_table_insert (current_hash, domain, this);
	pthread_mutex_unlock (&hash_mutex);

	types = new Types ();
	types->Initialize ();
	downloaders = new List ();
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

	pthread_mutex_lock (&hash_mutex);
	g_hash_table_remove (current_hash, domain);
	pthread_mutex_unlock (&hash_mutex);

	mono_domain_set (root_domain, FALSE);

#if MONO_ENABLE_APP_DOMAIN_CONTROL
	if (domain != root_domain)
		mono_domain_unload (domain);
#endif

	LOG_DEPLOYMENT ("Deployment::~Deployment (): %p\n", this);

#if SANITY
	if (pending_unrefs != NULL)
		g_warning ("Deployment::~Deployment (): There are still pending unrefs.\n");
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
	g_hash_table_destroy (EventObject::objects_alive);
#endif

	delete types;
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

		g_hash_table_foreach (objects_alive, accumulate_last_n, last_n);

	 	uint counter = 10;
		counter = MIN(counter, last_n->len);
		if (counter) {
			printf ("\tOldest %d objects alive:\n", counter);
			for (uint i = 0; i < MIN (counter, last_n->len); i ++) {
				EventObject* obj = (EventObject *) last_n->pdata [i];
				printf ("\t\t%i = %s, refcount: %i\n", obj->GetId (), obj->GetTypeName (), obj->GetRefCount ());
			}
		}

		g_ptr_array_free (last_n, true);
	}
}
#endif

void
Deployment::Dispose ()
{
	LOG_DEPLOYMENT ("Deployment::Dispose (): %p\n", this);
	
	Emit (ShuttingDownEvent);
	
	AbortAllDownloaders ();
	
	mono_gc_collect (mono_gc_max_generation ());

#if MONO_ENABLE_APP_DOMAIN_CONTROL
	mono_gc_invoke_finalizers ();

	// mono_gc_invoke_finalizers can cause the current appdomain to change
	// which will cause Deployment::GetCurrent to return null (or another deployment).
	Deployment::SetCurrent (this); 
#endif

	if (current_app != NULL)
		current_app->Dispose ();
		
	EventObject::Dispose ();
}

Types*
Deployment::GetTypes ()
{
	return types;
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
	downloaders->Append (new IDownloaderNode (dl));
}

void
Deployment::UnregisterDownloader (IDownloader *dl)
{
	IDownloaderNode *node = (IDownloaderNode *) downloaders->First ();
	while (node != NULL) {
		if (node->dl == dl) {
			downloaders->Remove (node);
			return;
		}
		node = (IDownloaderNode *) node->next;
	}
}

void
Deployment::AbortAllDownloaders ()
{
	IDownloaderNode *node;

	while ((node = (IDownloaderNode *) downloaders->First ()) != NULL) {
		if (!node->dl->IsAborted ())
			node->dl->Abort ();
		downloaders->Remove (node);
	}
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
	if (IsDisposed () && list == NULL && objects_destroyed != objects_created) {
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
