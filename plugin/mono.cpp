/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mono.cpp: Support routines to load the Mono VM as a browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "moonlight.h"
#if SL_2_0
#include "moon-mono.h"
G_BEGIN_DECLS
#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-config.h>
G_END_DECLS

#define d(x) x

static MonoDomain   *moon_domain;
static MonoAssembly *moon_boot_assembly;
static char *boot_assembly;
static bool moon_vm_loaded = false;

// Methods
static MonoMethod   *moon_load_xaml;
static MonoMethod   *moon_load_xap;
static MonoMethod   *moon_destroy_application;

static MonoMethod *
vm_get_method_from_name (const char *name)
{
	MonoMethod *method;
	MonoMethodDesc *desc;

	desc = mono_method_desc_new (name, true);
	method = mono_method_desc_search_in_image (desc, mono_assembly_get_image (moon_boot_assembly));
	mono_method_desc_free (desc);

	if (method == NULL)
		printf ("Warning: could not find method %s in the assembly", name);
	
	return method;
}


bool
vm_is_loaded (void)
{
	return moon_vm_loaded;
}

bool
vm_init (void)
{
	bool result = false;
	
	if (moon_vm_loaded)
		return true;
	
#if PLUGIN_INSTALL
	Dl_info dlinfo;
	char *dirname;
	
	if (dladdr ((void *) &vm_init, &dlinfo) == 0) {
		fprintf (stderr, "Unable to find the location of libmoonplugin %s\n", dlerror ());
		return false;
	}
	
	dirname = g_path_get_dirname (dlinfo.dli_fname);
	boot_assembly = g_build_filename (dirname, "moonlight.exe", NULL);
	g_free (dirname);
#else
	boot_assembly = g_build_filename (PLUGIN_DIR, "plugin", "moonlight.exe", NULL);
#endif
	
	d(printf ("The file is %s\n", boot_assembly));
	
	mono_config_parse (NULL);
	mono_debug_init (MONO_DEBUG_FORMAT_MONO);
	moon_domain = mono_jit_init_version (boot_assembly, "moonlight");
	moon_boot_assembly = mono_domain_assembly_open (moon_domain, boot_assembly);
	
	if (moon_boot_assembly) {
		char *argv [2];
		
		argv [0] = boot_assembly;
		argv [1] = NULL;
		
		mono_jit_exec (moon_domain, moon_boot_assembly, 1, argv);
		
		moon_load_xaml  = vm_get_method_from_name ("Moonlight.ApplicationLauncher:CreateXamlLoader");
		moon_load_xap   = vm_get_method_from_name ("Moonlight.ApplicationLauncher:CreateApplication");
		moon_destroy_application = vm_get_method_from_name ("Moonlight.ApplicationLauncher:DestroyApplication");

		if (moon_load_xaml != NULL && moon_load_xap != NULL && moon_destroy_application != NULL)
			result = true;
	}

	printf ("Mono Runtime: %s\n", result ? "OK" : "Failed");
	
	moon_vm_loaded = true;
	
	d(enable_vm_stack_trace ());
	
	return result;
}

//
// This instructs the managed code to load the Xaml file,
// the signature will change as we add the extra parameters
// that the plugin uses (instead of just <embed src>)
//
gpointer
vm_xaml_file_loader_new (XamlLoader* native_loader, gpointer plugin, gpointer surface, const char *file)
{
	return vm_xaml_loader_new (native_loader, plugin, surface, file, NULL);
}

gpointer
vm_xaml_str_loader_new (XamlLoader* native_loader, gpointer plugin, gpointer surface, const char *str)
{
	return vm_xaml_loader_new (native_loader, plugin, surface, NULL, str);
}

gpointer
vm_xaml_loader_new (XamlLoader* native_loader, gpointer plugin, gpointer surface, const char *file, const char *str)
{
	MonoObject *loader;
	if (moon_load_xaml == NULL)
		return NULL;

	void *params [5];
	params [0] = &native_loader;
	params [1] = &plugin;
	params [2] = &surface;
	params [3] = file ? mono_string_new (moon_domain, file) : NULL;
	params [4] = str ? mono_string_new (moon_domain, str) : NULL;
	loader = mono_runtime_invoke (moon_load_xaml, NULL, params, NULL);
	return GUINT_TO_POINTER (mono_gchandle_new (loader, false));
}

void
vm_loader_destroy (gpointer loader_object)
{
	guint32 loader = GPOINTER_TO_UINT (loader_object);
	if (loader)
		mono_gchandle_free (loader);
}

bool
vm_application_create (gpointer plugin, gpointer surface, const char *file)
{
	if (moon_load_xap == NULL)
		return NULL;

	void *params [3];
	params [0] = &plugin;
	params [1] = &surface;
	params [2] = mono_string_new (moon_domain, file);
	MonoObject *ret = mono_runtime_invoke (moon_load_xap, NULL, params, NULL);
	
	return (bool) (*(MonoBoolean *) mono_object_unbox(ret));
}

void
vm_application_destroy (gpointer plugin)
{
	void *params [1];
	params [0] = &plugin;

	mono_runtime_invoke (moon_destroy_application, NULL, params, NULL);
}
#endif
