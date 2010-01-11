/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * moon-plugin.cpp: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <glib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "plugin.h"
#include "plugin-spinner.h"
#include "plugin-class.h"
#include "browser-bridge.h"
#include "downloader.h"
#include "pipeline-ui.h"
#include "plugin-downloader.h"
#include "npstream-request.h"
#include "xap.h"
#include "window.h"
#include "unzip.h"
#include "deployment.h"
#include "uri.h"
#include "timemanager.h"
#include "pal/gtk/windowless-gtk.h"

#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

#define w(x) x
// Debug NPStreams
#define nps(x)//x

extern guint32 moonlight_flags;

char *
NPN_strdup (const char *tocopy)
{
	int len = strlen(tocopy);
	char *ptr = (char *)NPN_MemAlloc (len+1);
	if (ptr != NULL) {
		strcpy (ptr, tocopy);
		// WebKit should calloc so we dont have to do this
		ptr[len] = 0;
	}

	return ptr;
}

/*** PluginInstance:: *********************************************************/

GSList *plugin_instances = NULL;

#if PAL_GTK
static MoonWindow *
create_gtk_windowless (int width, int height, PluginInstance *forPlugin)
{
	return new MoonWindowlessGtk (width, height, forPlugin);
}
#endif

PluginInstance::PluginInstance (NPP instance, guint16 mode)
{
	refcount = 1;
	this->instance = instance;
	this->mode = mode;
	window = NULL;
	connected_to_container = false;
	
	rootobject = NULL;
	
	surface = NULL;
	moon_window = NULL;
	
	// Property fields
	source_location = NULL;
	source_location_original = NULL;
	initParams = NULL;
	source = NULL;
	source_original = NULL;
	source_idle = 0;
	onLoad = NULL;
	onError = NULL;
	onResize = NULL;
	onSourceDownloadProgressChanged = NULL;
	onSourceDownloadComplete = NULL;
	splashscreensource = NULL;
	background = NULL;
	id = NULL;
	culture = NULL;
	uiCulture = NULL;

	windowless = false;
	cross_domain_app = false;		// false, since embedded xaml (in html) won't load anything (to change this value)
	default_enable_html_access = true;	// should we use the default value (wrt the HTML script supplied value)
	enable_html_access = true;		// an empty plugin must return TRUE before loading anything else (e.g. scripting)
	allow_html_popup_window = false;
	enable_redraw_regions = false;
	xembed_supported = FALSE;
	loading_splash = false;
	is_splash = false;
	is_shutting_down = false;
	has_shutdown = false;

	bridge = NULL;

	// MSDN says the default is 24: http://msdn2.microsoft.com/en-us/library/bb979688.aspx
	// blog says the default is 60: http://blogs.msdn.com/seema/archive/2007/10/07/perf-debugging-tips-enableredrawregions-a-performance-bug-in-videobrush.aspx
	// testing seems to confirm that the default is 60.
	enable_framerate_counter = false;
	maxFrameRate = 60;
	
	xaml_loader = NULL;
	timers = NULL;
	
	wrapped_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	
	cleanup_pointers = NULL;

	if (plugin_instances == NULL) {
		// first plugin is initialized

		// FIXME add some ifdefs + runtime checks here
#if PAL_GTK
		runtime_get_windowing_system()->SetWindowlessCtor (create_gtk_windowless);
#else
#error "no PAL backend"
#endif
	}

	plugin_instances = g_slist_append (plugin_instances, instance);
	
	/* back pointer to us */
	instance->pdata = this;
}

void
PluginInstance::Recreate (const char *source)
{
	//printf ("PluginInstance::Recreate (%s) this: %p, instance->pdata: %p\n", source, this, instance->pdata);
	
	int argc = 16;
	char *maxFramerate = g_strdup_printf ("%i", maxFrameRate);
	const char *argn [] = 
		{ "initParams", "onLoad", "onError", "onResize", 
		"source", "background", "windowless", "maxFramerate", "id",
		"enableFrameRateCounter", "enableRedrawRegions",
		"enablehtmlaccess", "allowhtmlpopupwindow", "splashscreensource",
		"onSourceDownloadProgressChanged", "onSourceDownloadComplete",
		"culture", "uiculture", NULL };
	const char *argv [] = 
		{ initParams, onLoad, onError, onResize,
		source, background, windowless ? "true" : "false", maxFramerate, id,
		GetEnableFrameRateCounter () ? "true" : "false",
		GetEnableRedrawRegions () ? "true" : "false",
		enable_html_access ? "true" : "false", allow_html_popup_window ? "true" : "false", splashscreensource,
		onSourceDownloadProgressChanged, onSourceDownloadComplete,
		culture, uiCulture, NULL };

		
	instance->pdata = NULL;
	
	PluginInstance *result;
	result = new PluginInstance (instance, mode);
	
	// printf ("PluginInstance::Recreate (%s), created %p\n", source, result);
	
	/* steal the root object, we need to use the same instance */
	result->rootobject = rootobject;
	rootobject = NULL;
	if (result->rootobject)
		result->rootobject->PreSwitchPlugin (this, result);
		
	result->cross_domain_app = cross_domain_app;
	result->default_enable_html_access = default_enable_html_access;
	result->connected_to_container = connected_to_container;
	result->Initialize (argc, (char **) argn, (char **) argv);
	
	// printf ("PluginInstance::Recreate (%s), new plugin's deployment: %p, current deployment: %p\n", source, result->deployment, Deployment::GetCurrent ());
	if (surface) {
		result->moon_window = surface->DetachWindow (); /* we reuse the same MoonWindow */
	} else {
		result->moon_window = NULL;
	}
	result->window = window;
	result->CreateWindow ();
	
	g_free (maxFramerate);
	
	/* destroy the current plugin instance */
	Deployment::SetCurrent (deployment);
	Shutdown ();
	unref (); /* the ref instance->pdata has */
	
	/* put in the new plugin instance */
	Deployment::SetCurrent (result->deployment);
	instance->pdata = result;
	
	if (result->rootobject) {
		/* We need to reconnect all event handlers js might have to our root objects */
		result->rootobject->PostSwitchPlugin (this, result);
	}
	
//	printf ("PluginInstance::Recreate (%s) [Done], new plugin: %p\n", source, result);
}

PluginInstance::~PluginInstance ()
{
	deployment->unref_delayed ();
}

void
PluginInstance::ref ()
{
	g_assert (refcount > 0);
	g_atomic_int_inc (&refcount);
}

void
PluginInstance::unref ()
{
	g_assert (refcount > 0);
	int v = g_atomic_int_exchange_and_add (&refcount, -1) - 1;
	if (v == 0)
		delete this;
}

bool
PluginInstance::IsShuttingDown ()
{
	VERIFY_MAIN_THREAD;
	return is_shutting_down;
}

bool
PluginInstance::HasShutdown ()
{
	VERIFY_MAIN_THREAD;
	return has_shutdown;
}

void
PluginInstance::Shutdown ()
{
	// Kill timers
	GSList *p;

	g_return_if_fail (!is_shutting_down);
	g_return_if_fail (!has_shutdown);

	is_shutting_down = true;
	
	Deployment::SetCurrent (deployment);

	// Destroy the XAP application
	DestroyApplication ();

	for (p = timers; p != NULL; p = p->next){
		guint32 source_id = GPOINTER_TO_INT (p->data);

		g_source_remove (source_id);
	}
	g_slist_free (p);
	timers = NULL;

	g_hash_table_destroy (wrapped_objects);
	wrapped_objects = NULL;

	// Remove us from the list.
	plugin_instances = g_slist_remove (plugin_instances, instance);

	for (GSList *l = cleanup_pointers; l; l = l->next) {
		gpointer* p = (gpointer*)l->data;
		*p = NULL;
	}
	g_slist_free (cleanup_pointers);
	cleanup_pointers = NULL;

	if (rootobject) {
		NPN_ReleaseObject ((NPObject*)rootobject);
		rootobject = NULL;
	}

	g_free (background);
	background = NULL;
	g_free (id);
	id = NULL;
	g_free (onSourceDownloadProgressChanged);
	onSourceDownloadProgressChanged = NULL;
	g_free (onSourceDownloadComplete);
	onSourceDownloadComplete = NULL;
	g_free (splashscreensource);
	splashscreensource = NULL;
	g_free (culture);
	culture = NULL;
	g_free (uiCulture);
	uiCulture = NULL;
	g_free (initParams);
	initParams = NULL;
	delete xaml_loader;
	xaml_loader = NULL;

	g_free (source);
	source = NULL;
	g_free (source_original);
	source_original = NULL;
	g_free (source_location);
	source_location = NULL;
	g_free (source_location_original);
	source_location_original = NULL;

	if (source_idle) {
		g_source_remove (source_idle);
		source_idle = 0;
	}
	
	//
	// The code below was an attempt at fixing this, but we are still getting spurious errors
	// we might have another source of problems
	//
	//fprintf (stderr, "Destroying the surface: %p, plugin: %p\n", surface, this);
	if (surface != NULL) {
		//gdk_error_trap_push ();
		surface->Zombify();
		surface->Dispose ();
		surface->unref_delayed();
		//gdk_error_trap_pop ();
		surface = NULL;
	}

	if (bridge) {
		delete bridge;
		bridge = NULL;
	}

	deployment->Shutdown ();

	is_shutting_down = false;
	has_shutdown = true;
}

static bool
same_site_of_origin (const char *url1, const char *url2)
{
	bool result = false;
	Uri *uri1;
	
	if (url1 == NULL || url2 == NULL)
		return true;
	
	uri1 = new Uri ();
	if (uri1->Parse (url1)) {
		Uri *uri2 = new Uri ();

		if (uri2->Parse (url2)) {
			// if only one of the two URI is absolute then the second one is relative to the first, 
			// which makes it part of the same site of origin
			if ((uri1->isAbsolute && !uri2->isAbsolute) || (!uri1->isAbsolute && uri2->isAbsolute))
				result = true;
			else
				result = Uri::SameSiteOfOrigin (uri1, uri2);
		}
		delete uri2;
	}
	delete uri1;
	return result;
}

static bool
parse_bool_arg (const char *arg)
{
	bool b;
	return xaml_bool_from_str (arg, &b) && b;
}

void
PluginInstance::Initialize (int argc, char* argn[], char* argv[])
{
	for (int i = 0; i < argc; i++) {
		if (argn[i] == NULL) {
			//g_warning ("PluginInstance::Initialize, arg %d == NULL", i);
			continue;
		}
		else if (!g_ascii_strcasecmp (argn[i], "initParams")) {
			initParams = g_strdup (argv[i]);
		}
		else if (!g_ascii_strcasecmp (argn[i], "onLoad")) {
			onLoad = argv[i];
		}
		else if (!g_ascii_strcasecmp (argn[i], "onError")) {
			onError = argv[i];
		}
		else if (!g_ascii_strcasecmp (argn[i], "onResize")) {
			onResize = argv[i];
		}
		else if (!g_ascii_strcasecmp (argn[i], "src") || !g_ascii_strcasecmp (argn[i], "source")) {
			/* There is a new design pattern that creates a silverlight object with data="data:application/x-silverlight,"
			 * firefox is passing this to us as the src element.  We need to ensure we dont set source to this value
			 * as this design pattern sets a xap up after the fact, but checks to ensure Source hasn't been set yet.
			 * 
			 * eg: http://theamazingalbumcoveratlas.org/
			 *
			 * TODO: Find a site that has data:application/x-silverlight,SOMEDATA and figure out what we do with it
			 */
			if (g_ascii_strncasecmp (argv[i], "data:application/x-silverlight", 30) != 0 && argv[i][strlen(argv[i])-1] != ',') {
				source = g_strdup (argv[i]);
				// we must be able to retrieve the original source, e.g. after a redirect
				source_original = g_strdup (source);
			}
		}
		else if (!g_ascii_strcasecmp (argn[i], "background")) {
			background = g_strdup (argv[i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "windowless")) {
			windowless = parse_bool_arg (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "maxFramerate")) {
			maxFrameRate = atoi (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "enableFrameRateCounter")) {
			enable_framerate_counter = parse_bool_arg (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "enableRedrawRegions")) {
			enable_redraw_regions = parse_bool_arg (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "id")) {
			id = g_strdup (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "enablehtmlaccess")) {
			default_enable_html_access = false; // we're using the application value, not the default one
			enable_html_access = parse_bool_arg (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "allowhtmlpopupwindow")) {
			allow_html_popup_window = parse_bool_arg (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "splashscreensource")) {
			splashscreensource = g_strdup (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "onSourceDownloadProgressChanged")) {
			onSourceDownloadProgressChanged = g_strdup (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "onSourceDownloadComplete")) {
			onSourceDownloadComplete = g_strdup (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "culture")) {
			culture = g_strdup (argv[i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "uiCulture")) {
			uiCulture = g_strdup (argv[i]);
		}
		else {
		  //fprintf (stderr, "unhandled attribute %s='%s' in PluginInstance::Initialize\n", argn[i], argv[i]);
		}
	}

	// like 'source' the original location can also be required later (for cross-domain checks after redirections)
	source_location_original = GetPageLocation ();

	guint32 supportsWindowless = FALSE; // NPBool + padding

	int plugin_major, plugin_minor;
	int netscape_major, netscape_minor;
	bool try_opera_quirks = FALSE;

	/* Find the version numbers. */
	NPN_Version(&plugin_major, &plugin_minor,
		    &netscape_major, &netscape_minor);

	//d(printf ("Plugin NPAPI version = %d.%d\n", plugin_major, netscape_minor));
	//d(printf ("Browser NPAPI version = %d.%d\n", netscape_major, netscape_minor));

	NPError error;
	error = NPN_GetValue (instance, NPNVSupportsXEmbedBool, &xembed_supported);
	if (error || !xembed_supported) {
		// This should be an error but we'll use it to detect
		// that we are running in opera
		//return NPERR_INCOMPATIBLE_VERSION_ERROR;
		if (!windowless)
			d(printf ("*** XEmbed not supported\n"));

		try_opera_quirks = true;
	}

	error = NPN_GetValue (instance, NPNVSupportsWindowless, &supportsWindowless);
	supportsWindowless = (error == NPERR_NO_ERROR) && supportsWindowless;

#ifdef DEBUG
	if ((moonlight_flags & RUNTIME_INIT_ALLOW_WINDOWLESS) == 0) {
		printf ("plugin wants to be windowless, but we're not going to let it\n");
		windowless = false;
	}
#endif
	if (windowless) {
		if (supportsWindowless) {
			NPN_SetValue (instance, NPPVpluginWindowBool, (void *) FALSE);
			NPN_SetValue (instance, NPPVpluginTransparentBool, (void *) TRUE);
			d(printf ("windowless mode\n"));
		} else {
			d(printf ("browser doesn't support windowless mode.\n"));
			windowless = false;
		}
	}

        // grovel around in the useragent and try to figure out which
        // browser bridge we should use.
        const char *useragent = NPN_UserAgent (instance);

	if (strstr (useragent, "Opera")) {
		// opera based
		TryLoadBridge ("opera");
	}
	else if (strstr (useragent, "AppleWebKit")) {
		// webkit based
		TryLoadBridge ("webkit");
	}
        else if (strstr (useragent, "Gecko")) {
		// gecko based, let's look for 'rv:1.8' vs 'rv:1.9'
		if (strstr (useragent, "rv:1.8")) {
			TryLoadBridge ("ff2");
		}
		else if (strstr (useragent, "rv:1.9")) {
			TryLoadBridge ("ff3");
		}
        }

	// XXX Opera currently claims to be mozilla when we query it
	if (!bridge && try_opera_quirks)
		TryLoadBridge ("opera");

        if (!bridge) {
		g_warning ("probing for browser type failed, user agent = `%s'",
			   useragent);
	}

	if (!CreatePluginDeployment ()) { 
		g_warning ("Couldn't initialize Mono or create the plugin Deployment");
	}
}

typedef BrowserBridge* (*create_bridge_func)();

const char*
get_plugin_dir (void)
{
	static char *plugin_dir = NULL;

	if (!plugin_dir) {
		Dl_info dlinfo;
		if (dladdr((void *) &NPN_strdup, &dlinfo) == 0) {
			fprintf (stderr, "Unable to find the location of libmoonplugin.so: %s\n", dlerror ());
			return NULL;
		}
		plugin_dir = g_path_get_dirname (dlinfo.dli_fname);
	}
	return plugin_dir;
}

void
PluginInstance::TryLoadBridge (const char *prefix)
{
	char *bridge_name = g_strdup_printf ("libmoonplugin-%sbridge.so", prefix);
	char *bridge_path;

	bridge_path = g_build_filename (get_plugin_dir (), bridge_name, NULL);

	void* bridge_handle = dlopen (bridge_path, RTLD_LAZY);

	g_free (bridge_name);
	g_free (bridge_path);

	if (bridge_handle == NULL) {
		g_warning ("failed to load browser bridge: %s", dlerror());
		return;
	}

	create_bridge_func bridge_ctor = (create_bridge_func)dlsym (bridge_handle, "CreateBrowserBridge");
	if (bridge_ctor == NULL) {
		g_warning ("failed to locate CreateBrowserBridge symbol: %s", dlerror());
		return;
	}

	bridge = bridge_ctor ();
}

NPError
PluginInstance::GetValue (NPPVariable variable, void *result)
{
	NPError err = NPERR_NO_ERROR;

	switch (variable) {
	case NPPVpluginNeedsXEmbed:
		*((NPBool *)result) = !windowless;
		break;
	case NPPVpluginScriptableNPObject:
		*((NPObject**) result) = GetRootObject ();
		break;
	default:
		err = NPERR_INVALID_PARAM;
		break;
	}
	
	return err;
}

NPError
PluginInstance::SetValue (NPNVariable variable, void *value)
{
	return NPERR_NO_ERROR;
}

NPError
PluginInstance::SetWindow (NPWindow *window)
{
	Deployment::SetCurrent (deployment);

 	if (moon_window) {
		// XXX opera Window lifetime hack needs this
		this->window = window;

		if (!surface)
			return NPERR_GENERIC_ERROR;
		
		moon_window->Resize (window->width, window->height);
		return NPERR_NO_ERROR;
	}

	this->window = window;
	CreateWindow ();
	
	return NPERR_NO_ERROR;
}

NPObject*
PluginInstance::GetHost()
{
	NPObject *object = NULL;
	if (NPERR_NO_ERROR != NPN_GetValue(instance, NPNVPluginElementNPObject, &object)) {
		d(printf ("Failed to get plugin host object\n"));
	}
	return object;
}

#if 0
void
PluginInstance::ReportCache (Surface *surface, long bytes, void *user_data)
{
	PluginInstance *plugin = (PluginInstance *) user_data;
	char *msg;

	if (bytes < 1048576)
		msg = g_strdup_printf ("Cache size is ~%d KB", (int) (bytes / 1024));
	else
		msg = g_strdup_printf ("Cache size is ~%.2f MB", bytes / 1048576.0);

	NPN_Status (plugin->instance, msg);

	if (plugin->properties_cache_label)
		gtk_label_set_text (GTK_LABEL (plugin->properties_cache_label), msg);
	
	g_free (msg);
}
#endif

static void
register_event (NPP instance, const char *event_name, char *script_name, NPObject *npobj)
{
	if (!script_name)
		return;

	char *retval = NPN_strdup (script_name);
	NPVariant npvalue;

	STRINGZ_TO_NPVARIANT (retval, npvalue);
	NPIdentifier identifier = NPN_GetStringIdentifier (event_name);
	NPN_SetProperty (instance, npobj, identifier, &npvalue);
	NPN_MemFree (retval);
}

bool
PluginInstance::IsLoaded ()
{
	if (!GetSurface () || is_splash)
		return false;

	return GetSurface()->IsLoaded();
}

void
PluginInstance::CreateWindow ()
{
	bool created = false;
	bool success = true;
	
	if (moon_window == NULL) {
		if (windowless) {
			moon_window = runtime_get_windowing_system()->CreateWindowless (window->width, window->height, this);
			moon_window->SetTransparent (true);
		}
		else {
			moon_window = runtime_get_windowing_system()->CreateWindow (false, window->width, window->height);
		}
		created = true;
	} else {
		created = false;
	}

	surface = new Surface (moon_window);
	deployment->SetSurface (surface);
	moon_window->SetSurface (surface);
	
	MoonlightScriptControlObject *root = GetRootObject ();
	register_event (instance, "onSourceDownloadProgressChanged", onSourceDownloadProgressChanged, root);
	register_event (instance, "onSourceDownloadComplete", onSourceDownloadComplete, root);
	register_event (instance, "onError", onError, root);
	//	register_event (instance, "onResize", onResize, rootx->content);

	// NOTE: last testing showed this call causes opera to reenter but moving it is trouble and
	// the bug is on opera's side.
	SetPageURL ();
	success = LoadSplash ();

	surface->SetDownloaderContext (this);
	
	surface->GetTimeManager()->SetMaximumRefreshRate (maxFrameRate);
	
	surface->SetEnableFrameRateCounter (enable_framerate_counter);
	surface->SetEnableRedrawRegions (enable_redraw_regions);
	
	if (background) {
		Color *c = color_from_str (background);
		
		if (c == NULL) {
			d(printf ("error setting background color\n"));
			c = new Color (0x00FFFFFF);
		}
		
		surface->SetBackgroundColor (c);
		delete c;
	}
	
	if (success && !windowless && !connected_to_container) {
		moon_window->ConnectToContainerPlatformWindow (window->window);
		connected_to_container = true;
	}
}

void
PluginInstance::UpdateSource ()
{
	if (source_idle) {
		g_source_remove (source_idle);
		source_idle = 0;
	}

	if (surface != NULL)
		surface->DetachDownloaders ();

	if (!source || strlen (source) == 0)
		return;

	char *pos = strchr (source, '#');
	if (pos) {
		// FIXME: this will crash if this object has been deleted by the time IdleUpdateSourceByReference is called.
		source_idle = g_idle_add (IdleUpdateSourceByReference, this);

		// we're changing the page url as well as the xaml
		// location, so we need to call SetPageUrl.
		// SetPageUrl calls SetSourceLocation on the surface,
		// so we don't need to include the call here.
		SetPageURL ();
	} else {
		// we're setting the source location but not changing
		// the page location, so we need to call
		// SetSourceLocation here.
		Uri *page_uri = new Uri ();
		Uri *source_uri = new Uri ();

		char *page_location = GetPageLocation ();

		if (page_uri->Parse (page_location, true) &&
		    source_uri->Parse (source, true)) {

			// apparently we only do this with a xap?  ugh...
			//
			if (source_uri->path
			    && strlen (source_uri->path) > 4
			    && !strncmp (source_uri->path + strlen (source_uri->path) - 4, ".xap", 4)) {

				if (!source_uri->isAbsolute) {
					Uri *temp = new Uri();
					Uri::Copy (page_uri, temp);
					temp->Combine (source_uri);
					delete source_uri;
					source_uri = temp;
				}

				char* source_string = source_uri->ToString();

				surface->SetSourceLocation (source_string);

				g_free (source_string);
			}
		}

		g_free (page_location);
		delete page_uri;
		delete source_uri;

		StreamNotify *notify = new StreamNotify (StreamNotify::SOURCE, source);
		
		// FIXME: check for errors
		NPN_GetURLNotify (instance, source, NULL, notify);
	}
}

gboolean
PluginInstance::IdleUpdateSourceByReference (gpointer data)
{	
	PluginInstance *instance = (PluginInstance*)data;
	char *pos = NULL;
	
	instance->source_idle = 0;

	if (instance->source)
		pos = strchr (instance->source, '#');

	if (pos && strlen (pos+1) > 0)
		instance->UpdateSourceByReference (pos+1);

	instance->GetSurface ()->EmitSourceDownloadProgressChanged (new DownloadProgressEventArgs (1.0));
	instance->GetSurface ()->EmitSourceDownloadComplete ();
	return FALSE;
}

void
PluginInstance::UpdateSourceByReference (const char *value)
{
	// basically do the equivalent of document.getElementById('@value').textContent
	// all using NPAPI.
	//
	NPVariant _document;
	NPVariant _element;
	NPVariant _elementName;
	NPVariant _textContent;

	Deployment::SetCurrent (deployment);
	
	NPIdentifier id_ownerDocument = NPN_GetStringIdentifier ("ownerDocument");
	NPIdentifier id_getElementById = NPN_GetStringIdentifier ("getElementById");
	NPIdentifier id_textContent = NPN_GetStringIdentifier ("textContent");

	NPObject *host = GetHost();
	if (!host) {
//		printf ("no host\n");
		return;
	}

	// get host.ownerDocument
	bool nperr;
	if (!(nperr = NPN_GetProperty (instance, host, id_ownerDocument, &_document))
	    || !NPVARIANT_IS_OBJECT (_document)) {
//		printf ("no document (type == %d, nperr = %d)\n", _document.type, nperr);
		return;
	}

	// _element = document.getElementById ('@value')
	string_to_npvariant (value, &_elementName);
	if (!(nperr = NPN_Invoke (instance, NPVARIANT_TO_OBJECT (_document), id_getElementById,
				  &_elementName, 1, &_element))
	    || !NPVARIANT_IS_OBJECT (_element)) {
//		printf ("no valid element named #%s (type = %d, nperr = %d)\n", value, _element.type, nperr);
		NPN_ReleaseVariantValue (&_document);
	}

	// _textContent = _element.textContent
	if (!(nperr = NPN_GetProperty (instance, NPVARIANT_TO_OBJECT (_element), id_textContent, &_textContent))
	    || !NPVARIANT_IS_STRING (_textContent)) {
//		printf ("no text content for element named #%s (type = %d, nperr = %d)\n", value, _textContent.type, nperr);
		NPN_ReleaseVariantValue (&_document);
		NPN_ReleaseVariantValue (&_element);
		return;
	}

	char *xaml = g_strndup ((char *) NPVARIANT_TO_STRING (_textContent).utf8characters, NPVARIANT_TO_STRING (_textContent).utf8length);

//	printf ("yay, xaml = %s\n", xaml);

	if (xaml_loader)
		delete xaml_loader;

	xaml_loader = PluginXamlLoader::FromStr (NULL/*FIXME*/, xaml, this, surface);
	LoadXAML ();

	g_free (xaml);

	NPN_ReleaseVariantValue (&_document);
	NPN_ReleaseVariantValue (&_element);
	NPN_ReleaseVariantValue (&_textContent);
}


Downloader *
PluginInstance::CreateDownloader (PluginInstance *instance)
{
	if (instance) {
		return instance->surface->CreateDownloader ();
	} else {
		printf ("PluginInstance::CreateDownloader (%p): Unable to create contextual downloader.\n", instance);
		return new Downloader ();
	}
}

void
PluginInstance::SetInitParams (const char *value)
{
	g_free (initParams);
	initParams = g_strdup (value);
}

char*
PluginInstance::GetPageLocation ()
{
	char *location = NULL;
	NPIdentifier str_location = NPN_GetStringIdentifier ("location");
	NPIdentifier str_href = NPN_GetStringIdentifier ("href");
	NPVariant location_property;
	NPVariant location_object;
	NPObject *window;
	
	if (NPERR_NO_ERROR == NPN_GetValue (instance, NPNVWindowNPObject, &window)) {
		// Get the location property from the window object (which is another object).
		if (NPN_GetProperty (instance, window, str_location, &location_property)) {
			// Get the location property from the location object.
			if (NPN_GetProperty (instance, location_property.value.objectValue, str_href, &location_object )) {
				location = g_strndup (NPVARIANT_TO_STRING (location_object).utf8characters, NPVARIANT_TO_STRING (location_object).utf8length);
				NPN_ReleaseVariantValue (&location_object);
			}
			NPN_ReleaseVariantValue (&location_property);
		}
	}
	NPN_ReleaseObject (window);
	return location;
}

void
PluginInstance::SetPageURL ()
{
	if (source_location != NULL)
		return;
	
	char* location = GetPageLocation ();
	if (location && surface) {
		this->source_location = location;
		surface->SetSourceLocation (this->source_location);
	}
}


NPError
PluginInstance::NewStream (NPMIMEType type, NPStream *stream, NPBool seekable, guint16 *stype)
{
	Deployment::SetCurrent (deployment);
	
	nps (printf ("PluginInstance::NewStream (%p, %p, %i, %p)\n", type, stream, seekable, stype));

	if (IS_NOTIFY_SPLASHSOURCE (stream->notifyData)) {
		SetPageURL ();

		*stype = NP_ASFILEONLY;
		return NPERR_NO_ERROR;
	}
	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
		// See http://developer.mozilla.org/En/Getting_the_page_URL_in_NPAPI_plugin
		//
		// but don't call GetProperty inside SetWindow because it breaks opera by
		// causing it to reenter
		//
		// this->source_location = g_strdup (stream->url);
		SetPageURL ();

		*stype = NP_ASFILE;
		return NPERR_NO_ERROR;
	}

	if (IS_NOTIFY_DOWNLOADER (stream->notifyData)) {
		StreamNotify *notify = (StreamNotify *) stream->notifyData;
		Downloader *dl = (Downloader *) notify->pdata;
		// check if (a) it's a redirection and (b) if it is allowed for the current downloader policy
		if (!dl->CheckRedirectionPolicy (stream->url))
			return NPERR_INVALID_URL;

		NPStreamRequest::SetStreamData (dl, instance, stream);
		*stype = NP_ASFILE;
		return NPERR_NO_ERROR;
	}

	*stype = NP_NORMAL;

	return NPERR_NO_ERROR;
}

NPError
PluginInstance::DestroyStream (NPStream *stream, NPError reason)
{
	nps (printf ("PluginInstance::DestroyStream (%p, %i)\n", stream, reason));
	
	PluginDownloader *pd = (PluginDownloader*) stream->pdata;
	if (pd != NULL) {
		NPStreamRequest *req = (NPStreamRequest *) pd->getRequest ();
		if (req != NULL) 
			req->StreamDestroyed ();
	}

	return NPERR_NO_ERROR;
}

//
// Tries to load the XAML file, the parsing might fail because a
// required dependency is not available, so we need to queue the
// request to fetch the data.
//
bool
PluginInstance::LoadXAML ()
{
	int error = 0;

	//
	// Only try to load if there's no missing files.
	//
	Surface *our_surface = surface;
	AddCleanupPointer (&our_surface);

	if (!deployment->InitializeManagedDeployment (this, NULL, culture, uiCulture))
		return false;

	xaml_loader->LoadVM ();

	MoonlightScriptControlObject *root = GetRootObject ();
	if (!loading_splash) {
		register_event (instance, "onLoad", onLoad, root);
		//register_event (instance, "onError", onError, root);
		register_event (instance, "onResize", onResize, root->content);
		is_splash = false;
		loading_splash = false;
	} else {
		register_event (instance, "onLoad", (char*)"", root);
		//register_event (instance, "onError", "", root);
		register_event (instance, "onResize", (char*)"", root->content);
		is_splash = true;
		loading_splash = false;
	}

	xaml_loader->TryLoad (&error);

	if (!our_surface)
		return false;

	RemoveCleanupPointer (&our_surface);

	return true;
}

//
// Loads a XAP file
//
bool
PluginInstance::LoadXAP (const char *url, const char *fname)
{
	g_free (source_location);

	source_location = g_strdup (url);

	MoonlightScriptControlObject *root = GetRootObject ();
	
	register_event (instance, "onLoad", onLoad, root);
	//register_event (instance, "onError", onError, root);
	register_event (instance, "onResize", onResize, root->content);
	loading_splash = false;
	is_splash = false;

	Deployment::GetCurrent ()->Reinitialize ();
	GetDeployment()->SetXapLocation (url);
	return GetDeployment ()->InitializeManagedDeployment (this, fname, culture, uiCulture);
}

void
PluginInstance::DestroyApplication ()
{
	GetDeployment ()->DestroyManagedApplication (this);
}

/*
 * Prepares a string to be passed to Javascript, escapes the " and '
 * characters and maps the newline to \n sequence and cr to \r sequence
 */
static char*
string_to_js (char *s)
{
	char *res;
	GString *result;

	if (strchr (s, '\'') == NULL && strchr (s, '\n') == NULL)
		return g_strdup (s);

	result = g_string_new ("");

	for (char *p = s; *p != 0; *p++){
		if (*p == '"' || *p == '\''){
			g_string_append_c (result, '\\');
			g_string_append_c (result, *p);
		} else if (*p == '\n'){
			g_string_append_c (result, '\\');
			g_string_append_c (result, 'n');
		} else if (*p == '\r'){
			g_string_append_c (result, '\\');
			g_string_append_c (result, 'r');
		} else
			g_string_append_c (result, *p);
	}

	res = result->str;
	g_string_free (result, FALSE);

	return res;
}

void
PluginInstance::ReportException (char *msg, char *details, char **stack_trace, int num_frames)
{
	NPObject *object;
	NPVariant result;
	char *script, *row_js, *msg_escaped, *details_escaped;
	char **stack_trace_escaped;
	NPString str;
	int i;
	bool res;

	// Get a reference to our element
	object = GetHost();
	if (!object)
		return;

	// FIXME:
	// - make sure the variables do not become global

	// Remove ' from embedded strings
	msg_escaped = string_to_js (msg);
	details_escaped = string_to_js (details);
	stack_trace_escaped = g_new0 (char*, num_frames);
	for (i = 0; i < num_frames; ++i)
		stack_trace_escaped [i] = string_to_js (stack_trace [i]);

	// JS code to create our elements
	row_js = g_strdup (" ");
	for (i = 0; i < num_frames; ++i) {
		char *s;

		s = g_strdup_printf ("%s%s%s", row_js, (i == 0) ? "" : "\\n ", stack_trace_escaped [i]);
		g_free (row_js);
		row_js = s;
	}

	script = g_strdup_printf ("text1 = document.createTextNode ('%s'); text2 = document.createTextNode ('Exception Details: '); text3 = document.createTextNode ('%s'); text4 = document.createTextNode ('Stack Trace:'); parent = this.parentNode; a = document.createElement ('div'); a.appendChild (document.createElement ('hr')); msg = document.createElement ('font'); a.appendChild (msg); h2 = document.createElement ('h2'); i = document.createElement ('i'); b = document.createElement ('b'); msg.appendChild (h2); msg.appendChild (b); msg.appendChild (text3); msg.appendChild (document.createElement ('br')); msg.appendChild (document.createElement ('br')); b2 = document.createElement ('b'); b2.appendChild (text4); msg.appendChild (b2); b.appendChild (text2); h2.appendChild (i); i.appendChild (text1); msg.appendChild (document.createElement ('br')); msg.appendChild (document.createElement ('br')); a.appendChild (document.createElement ('hr')); table = document.createElement ('table'); msg.appendChild (table); table.width = '100%%'; table.bgColor = '#ffffcc'; tbody = document.createElement ('tbody'); table.appendChild (tbody); tr = document.createElement ('tr'); tbody.appendChild (tr); td = document.createElement ('td'); tr.appendChild (td); pre = document.createElement ('pre'); td.appendChild (pre); text = document.createTextNode ('%s'); pre.appendChild (text); previous = parent.firstChild; if (parent.firstChild.tagName == 'DIV') parent.removeChild (parent.firstChild); parent.insertBefore (a, this)", msg_escaped, details_escaped, row_js);

	g_free (msg_escaped);
	g_free (details_escaped);
	for (i = 0; i < num_frames; ++i)
		g_free (stack_trace_escaped [i]);
	g_free (stack_trace_escaped);
	g_free (row_js);

	str.utf8characters = script;
	str.utf8length = strlen (script);

	res = NPN_Evaluate (instance, object, &str, &result);
	if (res)
		NPN_ReleaseVariantValue (&result);
	NPN_ReleaseObject (object);
	g_free (script);
}

void *
PluginInstance::Evaluate (const char *code)
{
	NPObject *object = GetHost ();
	NPString string;
	NPVariant npresult;

	if (object == NULL)
		return NULL;
		
	
	string.utf8characters = code;
	string.utf8length = strlen (code);
		
	bool ret = NPN_Evaluate (instance, object, &string, &npresult);
	
	Value *res = NULL;
	bool keep_ref = false;
	if (ret) {
		if (!NPVARIANT_IS_VOID (npresult) && !NPVARIANT_IS_NULL (npresult)) {
			variant_to_value (&npresult, &res);
			if (npresult.type == NPVariantType_Object)
				keep_ref = true;
		}
	}

	if (!keep_ref)
		NPN_ReleaseVariantValue (&npresult);

	return (void*)res;
}

void
PluginInstance::CrossDomainApplicationCheck (const char *source)
{
	char* page_url = GetPageLocation ();
	// note: source might not be an absolute URL at this stage - but that only indicates that it's relative to the page url
	cross_domain_app = !same_site_of_origin (page_url, source);
	if (!cross_domain_app) {
		// we need also to consider a web page having cross-domain XAP that redirects to a XAP on the SOO
		// this will still be considered a cross-domain
		cross_domain_app = !same_site_of_origin (page_url, source_original);
	}
	g_free (page_url);

	// if the application did not specify 'enablehtmlaccess' then we use its default value
	// which is TRUE for same-site applications and FALSE for cross-domain applications
	if (default_enable_html_access)
		enable_html_access = !cross_domain_app;
}

static bool
is_xap (const char *fname)
{
	// Check for the ZIP magic header

	int fd;
	int nread;
	char buf[4];

	if ((fd = open (fname, O_RDONLY)) == -1)
		return false;

	nread = read (fd, buf, 4);
	if (nread != 4) {
		close (fd);
		return false;
	}

	if (buf [0] != 0x50 || buf [1] != 0x4B || buf [2] != 0x03 || buf [3] != 0x04) {
		close (fd);
		return false;
	}

	close (fd);
	return true;
}

void
PluginInstance::StreamAsFile (NPStream *stream, const char *fname)
{
	nps (printf ("PluginInstance::StreamAsFile (%p, %s)\n", stream, fname));
	
	Deployment::SetCurrent (deployment);
#if DEBUG
	deployment->AddSource (stream->url, fname);
#endif
	if (IS_NOTIFY_SPLASHSOURCE (stream->notifyData)) {
		xaml_loader = PluginXamlLoader::FromFilename (stream->url, fname, this, surface);
		loading_splash = true;
		surface->SetSourceLocation (stream->url);
		LoadXAML ();
		FlushSplash ();

		CrossDomainApplicationCheck (source);
		SetPageURL ();
	}
	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
		delete xaml_loader;
		xaml_loader = NULL;
		
		CrossDomainApplicationCheck (stream->url);

		Uri *uri = new Uri ();


		if (uri->Parse (stream->url, false) && is_xap (fname)) {
			LoadXAP (stream->url, fname);
		} else {
			xaml_loader = PluginXamlLoader::FromFilename (stream->url, fname, this, surface);
			LoadXAML ();
		}

		GetSurface ()->EmitSourceDownloadProgressChanged (new DownloadProgressEventArgs (1.0));
		GetSurface ()->EmitSourceDownloadComplete ();

		delete uri;
	} else if (IS_NOTIFY_DOWNLOADER (stream->notifyData)){
		Downloader *dl = (Downloader *) ((StreamNotify *)stream->notifyData)->pdata;
		
		dl->SetFilename (fname);
	}
}

gint32
PluginInstance::WriteReady (NPStream *stream)
{
	nps (printf ("PluginInstance::WriteReady (%p)\n", stream));
	
	Deployment::SetCurrent (deployment);

	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);
	
	if (notify && notify->pdata) {
		if (IS_NOTIFY_DOWNLOADER (notify)) {
			Downloader *dl = (Downloader *) notify->pdata;
		
			dl->NotifySize (stream->end);
		
			return MAX_STREAM_SIZE;
		}
		if (IS_NOTIFY_SOURCE (notify)) {
			source_size = stream->end;

			return MAX_STREAM_SIZE;
		}
	}
	
	NPN_DestroyStream (instance, stream, NPRES_DONE);
	
	return -1;
}

gint32
PluginInstance::Write (NPStream *stream, gint32 offset, gint32 len, void *buffer)
{
	nps (printf ("PluginInstance::Write (%p, %i, %i, %p)\n", stream, offset, len, buffer));
	
	Deployment::SetCurrent (deployment);

	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);
	
	if (notify && notify->pdata) {
		if (IS_NOTIFY_DOWNLOADER (notify)) {
			Downloader *dl = (Downloader *) notify->pdata;
		
			dl->Write (buffer, offset, len);
		}
		if (IS_NOTIFY_SOURCE (notify)) {
			if (source_size > 0) {
				float progress = (offset+len)/(float)source_size;
				if (GetSurface ()->GetToplevel () != NULL) {
					GetSurface ()->EmitSourceDownloadProgressChanged (new DownloadProgressEventArgs (progress));
				}
			}
		}
	}
	
	return len;
}

class PluginClosure : public EventObject {
public:
	PluginClosure (PluginInstance *plugin)
		: plugin (plugin)
	{
	}

	virtual ~PluginClosure ()
	{
	}

	PluginInstance *plugin;
};

void
PluginInstance::network_error_tickcall (EventObject *data)
{
	PluginClosure *closure = (PluginClosure*)data;
	Surface *s = closure->plugin->GetSurface();

	s->EmitError (new ErrorEventArgs (RuntimeError,
					  MoonError (MoonError::EXCEPTION, 2104, "Failed to download silverlight application.")));
}

void
PluginInstance::splashscreen_error_tickcall (EventObject *data)
{
	PluginClosure *closure = (PluginClosure*)data;
	Surface *s = closure->plugin->GetSurface();
	
	s->EmitError (new ErrorEventArgs (RuntimeError,
					  MoonError (MoonError::EXCEPTION, 2108, "Failed to download the splash screen")));
	closure->plugin->is_splash = false;

	// we need this check beccause the plugin might have been
	// dtor'ed (and the surface zombified) in the amove EmitError.
	if (!s->IsZombie())
		closure->plugin->UpdateSource ();

	closure->unref();
}

void
PluginInstance::UrlNotify (const char *url, NPReason reason, void *notifyData)
{
	nps (printf ("PluginInstance::UrlNotify (%s, %i, %p)\n", url, reason, notifyData));
	
	StreamNotify *notify = STREAM_NOTIFY (notifyData);
	
	Deployment::SetCurrent (deployment);
	
	if (reason != NPRES_DONE) {
		d(printf ("Download of URL %s failed: %i (%s)\n", url, reason,
			  reason == NPRES_USER_BREAK ? "user break" :
			  (reason == NPRES_NETWORK_ERR ? "network error" : "other error")));
		if (IS_NOTIFY_SOURCE (notify))
			GetSurface()->GetTimeManager()->AddTickCall (network_error_tickcall,
								     new PluginClosure (this));
	}
	
	if (notify && notify->pdata && IS_NOTIFY_DOWNLOADER (notify)) {
		Downloader *dl = (Downloader *) notify->pdata;

		if (reason != NPRES_DONE) {
			
			switch (reason) {
			case NPRES_USER_BREAK:
				dl->NotifyFailed ("user break");
				break;
			case NPRES_NETWORK_ERR:
				dl->NotifyFailed ("network error");
				break;
			default:
				dl->NotifyFailed ("unknown error");
				break;
			}
		} else {
			dl->NotifyFinished (url);
		}
	}

	if (notify && notify->pdata && IS_NOTIFY_SPLASHSOURCE (notify)) {
		if (reason == NPRES_NETWORK_ERR)
			GetSurface()->GetTimeManager()->AddTickCall (splashscreen_error_tickcall,
								     new PluginClosure (this));
		else
			UpdateSource ();
	}
	
	if (notify)
		delete notify;
}

bool
PluginInstance::LoadSplash ()
{
	if (splashscreensource != NULL) {
		char *pos = strchr (splashscreensource, '#');
		if (pos) {
			char *original = splashscreensource;
			splashscreensource = g_strdup (pos + 1);
			g_free (original);
			loading_splash = true;
			UpdateSourceByReference (splashscreensource);
			FlushSplash ();
			// this CrossDomainApplicationCheck comes
			// after FlushSplash because in cases where a
			// XDomain XAP uses a local (to the page)
			// splash xaml, the loaded event is fired but
			// not the progress events (App7.xap from drt
			// #283)
			CrossDomainApplicationCheck (source);
			UpdateSource ();
		} else {
			bool cross_domain_splash = false;

			Uri *splash_uri = new Uri ();
			Uri *page_uri = new Uri ();
			Uri *source_uri = new Uri ();
			char *page_location = GetPageLocation ();

			if (page_uri->Parse (page_location, true) &&
			    source_uri->Parse (source, true) &&
			    splash_uri->Parse (splashscreensource, true)) {
			
				if (source_uri->isAbsolute && !splash_uri->isAbsolute) {
					// in the case where the xap is at an
					// absolute xdomain url and the splash
					// xaml file is relative (to the page
					// url).  We can't do a straight
					// SameSiteOfOrigin check because in
					// SL no error (no events at all) is
					// raised during the splash (App6.xap
					// and App8.xap from drt #283)
					CrossDomainApplicationCheck (source);
				}
				else {
					// resolve both xap and splash urls so
					// we can see if they're from the same
					// site.

					// (see App4.xap, App9.xap from drt #283 for this bit)

					if (!source_uri->isAbsolute) {
						Uri *temp = new Uri();
						Uri::Copy (page_uri, temp);
						temp->Combine (source_uri);
						delete source_uri;
						source_uri = temp;
					}
					if (!splash_uri->isAbsolute) {
						Uri *temp = new Uri();
						Uri::Copy (page_uri, temp);
						temp->Combine (splash_uri);
						delete splash_uri;
						splash_uri = temp;
					}

					if (source_uri->isAbsolute || splash_uri->isAbsolute)
						cross_domain_splash = !Uri::SameSiteOfOrigin (source_uri, splash_uri);
				}
			}

			g_free (page_location);
			delete page_uri;
			delete source_uri;
			delete splash_uri;

			if (cross_domain_splash) {
				surface->EmitError (new ErrorEventArgs (RuntimeError,
									MoonError (MoonError::EXCEPTION, 2107, "Splash screens only available on same site as xap")));
				UpdateSource ();
				return false;
			}
			else {
				StreamNotify *notify = new StreamNotify (StreamNotify::SPLASHSOURCE, splashscreensource);

				// FIXME: check for errors
				NPN_GetURLNotify (instance, splashscreensource, NULL, notify);
			}
		}
	} else {
		// this check is for both local and xdomain xaps which
		// have a null splash, in the local case we get no
		// splash load event, but progress events, and in the
		// xdomain case we get no events at all. (App0.xap and
		// App5.xap from drt #283)
		CrossDomainApplicationCheck (source);
		xaml_loader = PluginXamlLoader::FromStr (NULL, PLUGIN_SPINNER, this, surface);
		loading_splash = true;
		if (!LoadXAML ())
			return false;
		FlushSplash ();
		UpdateSource ();
	}
	
	return true;
}

void
PluginInstance::FlushSplash ()
{
	// FIXME we may want to flush all events here but since this is written to the
	// tests I'm not sure.

	UIElement *toplevel = GetSurface ()->GetToplevel ();
	if (toplevel != NULL) {
		toplevel->WalkTreeForLoadedHandlers (NULL, false, false);
		deployment->EmitLoaded ();
	}
	loading_splash = false;
}

void
PluginInstance::Print (NPPrint *platformPrint)
{
	// nothing to do.
}

int16_t
PluginInstance::EventHandle (void *event)
{
	if (!surface) {
		g_warning ("EventHandle called before SetWindow, discarding event.");
		return 0;
	}

	if (!windowless) {
		g_warning ("EventHandle called for windowed plugin, discarding event.");
		return 0;
	}
		
	return moon_window->HandleEvent (event);
}

void
PluginInstance::AddWrappedObject (EventObject *obj, NPObject *wrapper)
{
	g_hash_table_insert (wrapped_objects, obj, wrapper);
}

void
PluginInstance::RemoveWrappedObject (EventObject *obj)
{
	if (wrapped_objects == NULL)
		return;
	g_hash_table_remove (wrapped_objects, obj);
}

NPObject*
PluginInstance::LookupWrappedObject (EventObject *obj)
{
	return (NPObject*)g_hash_table_lookup (wrapped_objects, obj);
}

void
PluginInstance::AddCleanupPointer (gpointer p)
{
	cleanup_pointers = g_slist_prepend (cleanup_pointers, p);
}

void
PluginInstance::RemoveCleanupPointer (gpointer p)
{
	cleanup_pointers = g_slist_remove (cleanup_pointers, p);
}

/*** Getters and Setters ******************************************************/

void
PluginInstance::SetSource (const char *value)
{
	bool changed = false;
	if (source) {
		changed = true;
		g_free (source);
		source = NULL;
	}

	if (changed) {
		Recreate (value);
		return;
	}

	source = g_strdup (value);
	// we may not have an original set at this point (e.g. when source is set via scripting)
	if (!source_original)
		source_original = g_strdup (value);

	UpdateSource ();
}

char *
PluginInstance::GetBackground ()
{
	return background;
}

bool
PluginInstance::SetBackground (const char *value)
{
	g_free (background);
	background = g_strdup (value);
	
	if (surface) {
		Color *c = color_from_str (background);
		
		if (c == NULL)
			return false;
		
		surface->SetBackgroundColor (c);
		delete c;
	}
	
	return true;
}

bool
PluginInstance::GetEnableFrameRateCounter ()
{
	return surface->GetEnableFrameRateCounter ();
}

void
PluginInstance::SetEnableFrameRateCounter (bool value)
{
	surface->SetEnableFrameRateCounter (value);
}

bool
PluginInstance::GetEnableRedrawRegions ()
{
	return surface->GetEnableRedrawRegions ();
}

void
PluginInstance::SetEnableRedrawRegions (bool value)
{
	surface->SetEnableRedrawRegions (value);
}

bool
PluginInstance::GetEnableHtmlAccess ()
{
	return enable_html_access;
}

bool
PluginInstance::GetAllowHtmlPopupWindow ()
{
	return allow_html_popup_window;
}

bool
PluginInstance::GetWindowless ()
{
	return windowless;
}

int
PluginInstance::GetMaxFrameRate ()
{
	if (surface)
		return surface->GetTimeManager ()->GetMaximumRefreshRate ();
	
	return maxFrameRate;
}

Deployment*
PluginInstance::GetDeployment ()
{
	return deployment;
}

void
PluginInstance::SetMaxFrameRate (int value)
{
	if (surface)
		surface->GetTimeManager()->SetMaximumRefreshRate (value);
	
	maxFrameRate = value;
}

gint32
PluginInstance::GetActualHeight ()
{
	return surface && surface->GetWindow () ? surface->GetWindow()->GetHeight() : 0;
}

gint32
PluginInstance::GetActualWidth ()
{
	return surface && surface->GetWindow () ? surface->GetWindow()->GetWidth() : 0;
}

MoonlightScriptControlObject *
PluginInstance::GetRootObject ()
{
	if (rootobject == NULL)
		rootobject = (MoonlightScriptControlObject *) NPN_CreateObject (instance, MoonlightScriptControlClass);

	NPN_RetainObject (rootobject);
	return rootobject;
}

NPP
PluginInstance::GetInstance ()
{
	return instance;
}

NPWindow*
PluginInstance::GetWindow ()
{
	return window;
}

char*
plugin_instance_get_id (PluginInstance *instance)
{
	return instance->GetId ();
}

void
plugin_instance_get_browser_runtime_settings (bool *debug, bool *html_access,
					      bool *httpnet_access, bool *script_access)
{
	*debug = *html_access = *httpnet_access = *script_access = false;
}

/*
	XamlLoader
*/

bool
PluginXamlLoader::LoadVM ()
{
	return InitializeLoader ();
}

bool
PluginXamlLoader::InitializeLoader ()
{
	if (initialized)
		return true;

	if (managed_loader)
		return true;

	if (GetFilename ()) {
		managed_loader = plugin->ManagedCreateXamlLoaderForFile (this, GetResourceBase(), GetFilename ());
	} else if (GetString ()) {
		managed_loader = plugin->ManagedCreateXamlLoaderForString (this, GetResourceBase(), GetString ());
	} else {
		return false;
	}

	initialized = managed_loader != NULL;

	return initialized;
}

//
// On error it sets the @error ref to 1
// Returns the filename that we are missing
//
void
PluginXamlLoader::TryLoad (int *error)
{
	DependencyObject *element;
	Type::Kind element_type;
	
	*error = 0;
	
	//d(printf ("PluginXamlLoader::TryLoad, filename: %s, str: %s\n", GetFilename (), GetString ()));
	
	GetSurface ()->Attach (NULL);
	
	if (GetFilename ()) {
		element = CreateDependencyObjectFromFile (GetFilename (), true, &element_type);
	} else if (GetString ()) {
		element = CreateDependencyObjectFromString (GetString (), true, &element_type);
	} else {
		*error = 1;
		return;
	}
	
	if (!element) {
		if (error_args && error_args->GetErrorCode() != -1) {
			d(printf ("PluginXamlLoader::TryLoad: Could not load xaml %s: %s (error: %s attr=%s)\n",
				  GetFilename () ? "file" : "string", GetFilename () ? GetFilename () : GetString (),
				  error_args->xml_element, error_args->xml_attribute));
			error_args->ref ();
			GetSurface ()->EmitError (error_args);
			return;
		} else {
			return;
		}
	}
	
	Type *t = Type::Find(element->GetDeployment (), element_type);
	if (!t) {
		d(printf ("PluginXamlLoader::TryLoad: Return value does not subclass Canvas, it is an unregistered type\n"));
		element->unref ();
		GetSurface ()->EmitError (new ErrorEventArgs (RuntimeError,
							      MoonError (MoonError::EXCEPTION, 2101, "Failed to initialize the application's root visual")));
		return;
	}

	if (!t->IsSubclassOf(Type::PANEL)) {
		d(printf ("PluginXamlLoader::TryLoad: Return value does not subclass of Panel, it is a %s\n",
			  element->GetTypeName ()));
		element->unref ();
		GetSurface ()->EmitError (new ErrorEventArgs (RuntimeError,
							      MoonError (MoonError::EXCEPTION, 2101, "Failed to initialize the application's root visual")));
		return;
	}
	
	//d(printf ("PluginXamlLoader::TryLoad () succeeded.\n"));

	GetSurface ()->Attach ((Panel*) element);

	// xaml_create_from_* passed us a ref which we don't need to
	// keep.
	element->unref ();

	return;
}

bool
PluginXamlLoader::SetProperty (void *parser, Value *top_level, const char *xmlns, Value* target, void* target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value* value, void* value_data, int flags)
{
	if (XamlLoader::SetProperty (parser, top_level, xmlns, target, target_data, target_parent, prop_xmlns, name, value, value_data))
		return true;

	if (value->GetKind () != Type::STRING)
		return false;

	if (!xaml_is_valid_event_name (plugin->GetDeployment (), target->GetKind(), name, false))
		return false;

	const char* function_name = value->AsString ();

	if (!strncmp (function_name, "javascript:", strlen ("javascript:")))
		return false;

	event_object_add_xaml_listener (target->AsDependencyObject (), plugin, name, function_name);
	
	return true;
}

PluginXamlLoader::PluginXamlLoader (const char *resourceBase, const char *filename, const char *str, PluginInstance *plugin, Surface *surface)
	: XamlLoader (resourceBase, filename, str, surface)
{
	this->plugin = plugin;
	xaml_is_managed = false;
	initialized = false;
	error_args = NULL;

	xap = NULL;

	managed_loader = NULL;
}

PluginXamlLoader::~PluginXamlLoader ()
{
	if (xap)
		delete xap;
	
	if (managed_loader)
		plugin->GetDeployment ()->DestroyManagedXamlLoader (managed_loader);
}

PluginXamlLoader *
plugin_xaml_loader_from_str (const char *resourceBase, const char *str, PluginInstance *plugin, Surface *surface)
{
	return PluginXamlLoader::FromStr (resourceBase, str, plugin, surface);
}

bool
PluginInstance::CreatePluginDeployment ()
{
	deployment = new Deployment ();
	Deployment::SetCurrent (deployment);
	
	/* 
	 * Give a ref to the deployment, this is required since managed code has
	 * pointers to this PluginInstance instance. This way we ensure that the
	 * PluginInstance isn't deleted before managed code has shutdown.
	 * We unref just after the appdomain is unloaded (in the event handler).
	 */
	ref ();
	deployment->AddHandler (Deployment::AppDomainUnloadedEvent, AppDomainUnloadedEventCallback, this);

	if (!deployment->InitializeAppDomain ()) {
		g_warning ("Moonlight: Couldn't initialize the AppDomain");
		return false;
	}

	return true;
}

void
PluginInstance::AppDomainUnloadedEventHandler (Deployment *deployment, EventArgs *args)
{
	unref (); /* See comment in CreatePluginDeployment */
}

gpointer
PluginInstance::ManagedCreateXamlLoaderForFile (XamlLoader *native_loader, const char *resourceBase, const char *file)
{
	return GetDeployment ()->CreateManagedXamlLoader (this, native_loader, resourceBase, file, NULL);
}

gpointer
PluginInstance::ManagedCreateXamlLoaderForString (XamlLoader* native_loader, const char *resourceBase, const char *str)
{
	return GetDeployment ()->CreateManagedXamlLoader (this, native_loader, resourceBase, NULL, str);
}

gint32
PluginInstance::GetPluginCount ()
{
	return g_slist_length (plugin_instances);
}
