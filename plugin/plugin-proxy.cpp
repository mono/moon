/*
 * plugin-entry.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *   Andrew Jorgensen (ajorgensen@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "npapi.h"
#include "npupp.h"

#include "moonlight.h"

#if INCLUDE_MONO_RUNTIME
#include <mono/metadata/assembly.h>
#endif

#ifdef XP_UNIX
typedef NPError (*np_initialize_func) (void *a, void *b);
#else
typedef NPError (*np_initialize_func) (void *a);
#endif
typedef NPError (*np_shutdown_func) ();
typedef NPError (*np_getvalue_func) (void *, NPPVariable var, void *avalue);
typedef char *  (*np_getmime_func)  ();

static np_initialize_func initialize;
static np_getvalue_func   getvalue;
static np_shutdown_func   shutdown;
static np_getmime_func    getmime;

static NPError
load (void)
{
	char *plugin_path;

#if PLUGIN_INSTALL
	Dl_info dlinfo;
	if (dladdr((void *) &load, &dlinfo) == 0) {
		fprintf (stderr, "Unable to find the location of libmoonloader %s\n", dlerror ());
		return FALSE;
	}

	char *plugin_dir;
	plugin_dir = g_build_filename (g_path_get_dirname(dlinfo.dli_fname), "moonlight", NULL);

	plugin_path = g_build_filename (plugin_dir, "libmoonplugin.so", NULL);

#if INCLUDE_FFMPEG
	// load libavutil
	char *avutil_path = g_build_filename (plugin_dir, "libavutil.so", NULL);
	void *real_avutil = dlopen (avutil_path, RTLD_LAZY | RTLD_GLOBAL);
	if (real_avutil == NULL){
		fprintf (stderr, "Unable to load the libavutil %s\n", dlerror ());
		return FALSE;
	}
	g_free (avutil_path);

#if INCLUDE_SWSCALE
	// load libswscale
	char *swscale_path = g_build_filename (plugin_dir, "libswscale.so", NULL);
	void *real_swscale = dlopen (swscale_path, RTLD_LAZY | RTLD_GLOBAL);
	if (real_swscale == NULL){
		fprintf (stderr, "Unable to load the libswscale %s\n", dlerror ());
		return FALSE;
	}
	g_free (swscale_path);
#endif

	// load libavcodec
	char *avcodec_path = g_build_filename (plugin_dir, "libavcodec.so", NULL);
	void *real_avcodec = dlopen (avcodec_path, RTLD_LAZY | RTLD_GLOBAL);
	if (real_avcodec == NULL){
		fprintf (stderr, "Unable to load the libavcodec %s\n", dlerror ());
		return FALSE;
	}
	g_free (avcodec_path);
#endif

#if INCLUDE_MONO_RUNTIME
	// load libmono
	char *mono_path = g_build_filename (plugin_dir, "libmono.so", NULL);
	void *real_mono = dlopen (mono_path, RTLD_LAZY | RTLD_GLOBAL);
	if (real_mono == NULL){
		fprintf (stderr, "Unable to load the libmono %s\n", dlerror ());
		return FALSE;
	}
	mono_set_dirs (plugin_dir, plugin_dir);
	g_free (mono_path);
#endif

	// load libmoon
	char *moon_path = g_build_filename (plugin_dir, "libmoon.so", NULL);
	void *real_moon = dlopen (moon_path, RTLD_LAZY | RTLD_GLOBAL);
	if (real_moon == NULL){
		fprintf (stderr, "Unable to load the libmoon %s\n", dlerror ());
		return FALSE;
	}
	g_free (moon_path);

	g_free (plugin_dir);
#else
	// allow the user to override the plugin directory
	// by setting MOON_PLUGIN_DIR
	const gchar *moon_plugin_dir = g_getenv("MOON_PLUGIN_DIR");
	if (moon_plugin_dir == NULL) {
		plugin_path = g_build_filename (PLUGIN_DIR, "plugin", "libmoonplugin.so", NULL);
	} else {
		plugin_path = g_build_filename (moon_plugin_dir, "libmoonplugin.so", NULL);
	}
#endif

	void *real_plugin = dlopen (plugin_path, RTLD_LAZY);

	g_free (plugin_path);

	if (real_plugin == NULL){
		fprintf (stderr, "Unable to load the real plugin %s\n", dlerror ());
		return FALSE;
	}

	initialize = (np_initialize_func) dlsym (real_plugin, LOADER_RENAMED_NAME(NP_Initialize));
	if (initialize == NULL){
		fprintf (stderr, "NP_Initialize not found %s\n", dlerror ());
		return FALSE;
	}


	getvalue = (np_getvalue_func) dlsym (real_plugin, LOADER_RENAMED_NAME(NP_GetValue));
	if (getvalue == NULL){
		fprintf (stderr, "NP_GetValue not found %s\n", dlerror ());
		return FALSE;
	}

	getmime = (np_getmime_func) dlsym (real_plugin, LOADER_RENAMED_NAME(NP_GetMIMEDescription));
	if (getmime == NULL){
		fprintf (stderr, "NP_GetMIMEDescription not found %s\n", dlerror ());
		return FALSE;
	}

	shutdown = (np_shutdown_func) dlsym (real_plugin, LOADER_RENAMED_NAME(NP_Shutdown));
	if (shutdown == NULL){
		fprintf (stderr, "NP_Shutdown not found %s\n", dlerror ());
		return FALSE;
	}

	return TRUE;
}

char*
NP_GetMIMEDescription (void)
{
	if (getmime == NULL)
		load ();

	if (getmime != NULL){
		return (*getmime)();
	}
	return (char *) "";
}

NPError
NP_GetValue (void *future, NPPVariable variable, void *value)
{
	if (getvalue == NULL)
		load ();

	if (getvalue != NULL)
		return (*getvalue) (future, variable, value);

	return NPERR_GENERIC_ERROR;
}

NPError OSCALL
#ifdef XP_UNIX
NP_Initialize (NPNetscapeFuncs *mozilla_funcs, NPPluginFuncs *plugin_funcs)
#else
NP_Initialize (NPNetscapeFuncs *mozilla_funcs)
#endif
{
	if (initialize == NULL)
		load ();

	if (initialize == NULL)
		return NPERR_GENERIC_ERROR;

	NPError res;

#ifdef XP_UNIX
	res = (*initialize) (mozilla_funcs, plugin_funcs);
#else
	res = (*initialize) (mozilla_funcs);
#endif
	return res;
}

NPError OSCALL
NP_Shutdown (void)
{
	if (shutdown == NULL)
		load ();
	if (shutdown != NULL)
		return (*shutdown) ();
	return NPERR_GENERIC_ERROR;
}
