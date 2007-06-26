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
load ()
{
	void *real_plugin = dlopen (PLUGIN_DIR "/plugin/libmoonplugin.so", RTLD_NOW);

	if (real_plugin == NULL){
		fprintf (stderr, "Unable to load the real plugin %s\n", dlerror ());
		return FALSE;
	}

	initialize = (np_initialize_func) dlsym (real_plugin, "NP_Initialize");
	if (initialize == NULL){
		fprintf (stderr, "NP_Initialize not found %s", dlerror ());
		return FALSE;
	}


	getvalue = (np_getvalue_func) dlsym (real_plugin, "NP_GetValue");
	if (getvalue == NULL){
		fprintf (stderr, "NP_GetValue not found %s\n", dlerror);
		return FALSE;
	}

	getmime = (np_getmime_func) dlsym (real_plugin, "NP_GetMIMEDescription");
	if (getmime == NULL){
		fprintf (stderr, "NP_GetMIMEDescription not found %s\n", dlerror);
		return FALSE;
	}

	shutdown = (np_shutdown_func) dlsym (real_plugin, "NP_Shutdown");
	if (shutdown == NULL){
		fprintf (stderr, "NP_Shutdown not found %s\n", dlerror);
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
	return "";
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
