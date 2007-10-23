/*
 * plugin-entry.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
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
	plugin_path = g_strconcat (g_get_home_dir(), "/.mozilla/plugins/moonlight/libmoonplugin.so", NULL);

	char *moon_path = g_strconcat (g_get_home_dir(), "/.mozilla/plugins/moonlight/libmoon.so", NULL);

	void *real_moon = dlopen (moon_path, RTLD_NOW | RTLD_GLOBAL);

	if (real_moon == NULL){
		fprintf (stderr, "Unable to load the libmoon %s\n", dlerror ());
		return FALSE;
	}

	g_free (moon_path);
#else
	plugin_path = g_strdup (PLUGIN_DIR "/plugin/libmoonplugin.so");
#endif

	void *real_plugin = dlopen (plugin_path, RTLD_NOW);

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
