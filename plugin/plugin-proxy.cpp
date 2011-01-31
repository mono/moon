/*
 * plugin-entry.cpp: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include <config.h>
#include "moonlight.h"

typedef NPError (*np_initialize_func) (void *a, void *b);
typedef NPError (*np_shutdown_func) ();
typedef NPError (*np_getvalue_func) (void *, NPPVariable var, void *avalue);
typedef void    (*config_parse_memory_func) (const char *buffer);
static np_initialize_func initialize;
static np_getvalue_func   getvalue;
static np_shutdown_func   shutdown;


static NPError
load (void)
{
	char *plugin_path;

#if DEBUG
	printf ("Moonlight: " PACKAGE_VERSION "\n");
#endif
	
	Dl_info dlinfo;
	if (dladdr((void *) &load, &dlinfo) == 0) {
		fprintf (stderr, "Moonlight: Unable to find the location of libmoonloaderxpi %s\n", dlerror ());
		return FALSE;
	}

	if (strstr(dlinfo.dli_fname, "libmoonloaderxpi.so")) {

		fprintf (stdout, "Moonlight: Attempting to load libmoonloaderxpi \n");
		char *plugin_dir;
		plugin_dir = g_build_filename (g_path_get_dirname(dlinfo.dli_fname), "moonlight", NULL);

		plugin_path = g_build_filename (plugin_dir, "libmoonpluginxpi.so", NULL);

	#if INCLUDE_FFMPEG
		// load libavutil
		char *avutil_path = g_build_filename (plugin_dir, "libavutil.so", NULL);
		void *real_avutil = dlopen (avutil_path, RTLD_LAZY | RTLD_GLOBAL);
		if (real_avutil == NULL){
			fprintf (stderr, "Moonlight: Unable to load the libavutil %s\n", dlerror ());
			return FALSE;
		}
		g_free (avutil_path);

		// load libavcodec
		char *avcodec_path = g_build_filename (plugin_dir, "libavcodec.so", NULL);
		void *real_avcodec = dlopen (avcodec_path, RTLD_LAZY | RTLD_GLOBAL);
		if (real_avcodec == NULL){
			fprintf (stderr, "Moonlight: Unable to load the libavcodec %s\n", dlerror ());
			return FALSE;
		}
		g_free (avcodec_path);
	#endif

		g_free (plugin_dir);

	} else {
		const gchar *moon_plugin_dir = g_getenv("MOON_PLUGIN_DIR");
		if (moon_plugin_dir == NULL) {
			plugin_path = g_build_filename (PLUGIN_DIR, "plugin", "libmoonplugin.so", NULL);
		} else {
			plugin_path = g_build_filename (moon_plugin_dir, "libmoonplugin.so", NULL);
		}
		fprintf (stdout, "Moonlight: Attempting to load libmoonplugin from: %s\n", plugin_path);
	}

	void *real_plugin = dlopen (plugin_path, RTLD_LAZY | RTLD_GLOBAL);

	if (real_plugin == NULL){
		fprintf (stderr, "Moonlight: Unable to load the real plugin %s\n", dlerror ());
		fprintf (stderr, "Moonlight: plugin_path is %s\n", plugin_path);
		return FALSE;
	}

	config_parse_memory_func mono_config_parse_memory = (config_parse_memory_func) dlsym (real_plugin, "mono_config_parse_memory");
	if (mono_config_parse_memory == NULL){
		fprintf (stderr, "Moonlight: mono_config_parse_memory not found. %s\n", dlerror ());
		return FALSE;
	}

	// Must dllmap moon and moonplugin, otherwise it doesn't know where to get it
	char* plugin_config = g_strdup_printf(
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<configuration>"
	"<dllmap dll=\"moonplugin\" target=\"%s\" />"
	"<dllmap dll=\"moon\" target=\"%s\" />"
"</configuration>", plugin_path, plugin_path);
	mono_config_parse_memory(plugin_config);
	g_free (plugin_config);

	g_free (plugin_path);

	initialize = (np_initialize_func) dlsym (real_plugin, LOADER_RENAMED_NAME(NP_Initialize));
	if (initialize == NULL){
		fprintf (stderr, "Moonlight: NP_Initialize not found %s\n", dlerror ());
		return FALSE;
	}


	getvalue = (np_getvalue_func) dlsym (real_plugin, LOADER_RENAMED_NAME(NP_GetValue));
	if (getvalue == NULL){
		fprintf (stderr, "Moonlight: NP_GetValue not found %s\n", dlerror ());
		return FALSE;
	}

	shutdown = (np_shutdown_func) dlsym (real_plugin, LOADER_RENAMED_NAME(NP_Shutdown));
	if (shutdown == NULL){
		fprintf (stderr, "Moonlight: NP_Shutdown not found %s\n", dlerror ());
		return FALSE;
	}

	return TRUE;
}

char*
NP_GetMIMEDescription (void)
{
	return (char *) (MIME_TYPES_HANDLED);
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
NP_Initialize (NPNetscapeFuncs *mozilla_funcs, NPPluginFuncs *plugin_funcs)
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
