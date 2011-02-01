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

#if PLUGIN_SL_2_0
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#endif

typedef NPError (*np_initialize_func) (void *a, void *b);
typedef NPError (*np_shutdown_func) ();
typedef NPError (*np_getvalue_func) (void *, NPPVariable var, void *avalue);

static np_initialize_func initialize;
static np_getvalue_func   getvalue;
static np_shutdown_func   shutdown;


static NPError
load (void)
{
	char *plugin_path;

	Dl_info dlinfo;
	if (dladdr((void *) &load, &dlinfo) == 0) {
		fprintf (stderr, "Unable to find the location of libmoonloaderxpi %s\n", dlerror ());
		return FALSE;
	}

	if (strstr(dlinfo.dli_fname, "libmoonloaderxpi.so")) {

		fprintf (stdout, "Attempting to load libmoonloaderxpi \n");
		char *plugin_dir;
		plugin_dir = g_build_filename (g_path_get_dirname(dlinfo.dli_fname), "moonlight", NULL);

		plugin_path = g_build_filename (plugin_dir, "libmoonpluginxpi.so", NULL);

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

	#if PLUGIN_SL_2_0
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
		char *moon_path = g_build_filename (plugin_dir, "libmoonxpi.so", NULL);
		void *real_moon = dlopen (moon_path, RTLD_LAZY | RTLD_GLOBAL);
		if (real_moon == NULL){
			fprintf (stderr, "Unable to load the libmoonxpi %s\n", dlerror ());
			return FALSE;
		}

		char* moon_config = g_strdup_printf("<?xml version=\"1.0\" encoding=\"utf-8\"?><configuration><dllmap dll=\"moon\" target=\"%s\" /></configuration>",moon_path);
		mono_config_parse_memory(moon_config);
		g_free (moon_config);
		g_free (moon_path);

		g_free (plugin_dir);

	} else {

		fprintf (stdout, "Attempting to load the system libmoon \n");
		const gchar *moon_plugin_dir = g_getenv("MOON_PLUGIN_DIR");
		if (moon_plugin_dir == NULL) {
			plugin_path = g_build_filename (PLUGIN_DIR, "plugin", "libmoonplugin.so", NULL);
		} else {
			plugin_path = g_build_filename (moon_plugin_dir, "libmoonplugin.so", NULL);
		}
	}

	void *real_plugin = dlopen (plugin_path, RTLD_LAZY | RTLD_GLOBAL);

	if (real_plugin == NULL){
		fprintf (stderr, "Unable to load the real plugin %s\n", dlerror ());
		fprintf (stderr, "plugin_path is %s\n", plugin_path);
		return FALSE;
	}

	// Must dllmap moonplugin, otherwise it doesn't know where to get it
	char* plugin_config = g_strdup_printf("<?xml version=\"1.0\" encoding=\"utf-8\"?><configuration><dllmap dll=\"moonplugin\" target=\"%s\" /></configuration>",plugin_path);
	mono_config_parse_memory(plugin_config);
	g_free (plugin_config);

	g_free (plugin_path);

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
	return MIME_TYPES_HANDLED;
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
