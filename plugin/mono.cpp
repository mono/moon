/*
 * mono.cpp: Support routines to load the Mono VM as a browser plugin.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "moonlight.h"
#include <stdlib.h>
#include <glib.h>

#if INCLUDE_MONO_RUNTIME
#include "moon-mono.h"
G_BEGIN_DECLS
#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-config.h>
G_END_DECLS

static MonoDomain   *moon_domain;
static MonoAssembly *moon_boot_assembly;
static char *boot_assembly;
static bool moon_vm_loaded = FALSE;

// Methods
static MonoMethod   *moon_load_xaml;

static MonoMethod *
vm_get_method_from_name (const char *name)
{
	MonoMethod *method;
	MonoMethodDesc *desc;

	desc = mono_method_desc_new (name, TRUE);
	method = mono_method_desc_search_in_image (desc, mono_assembly_get_image (moon_boot_assembly));
	mono_method_desc_free (desc);

	return method;
}

bool
vm_is_loaded (void)
{
	return moon_vm_loaded;
}

gboolean
vm_init (void)
{
	gboolean result = FALSE;

	if (moon_vm_loaded)
		return TRUE;

#if PLUGIN_INSTALL
	boot_assembly = g_build_filename (g_get_home_dir(), ".mozilla", "plugins", "moonlight", "moonlight.exe", NULL);
#else
	boot_assembly = g_build_filename (PLUGIN_DIR, "plugin", "moonlight.exe", NULL);
#endif
	printf ("The file is %s\n", boot_assembly);

	mono_config_parse (NULL);
	mono_debug_init (MONO_DEBUG_FORMAT_MONO);
	moon_domain = mono_jit_init_version (boot_assembly, "moonlight");
	moon_boot_assembly = mono_domain_assembly_open (moon_domain, boot_assembly);
	if (moon_boot_assembly){
		char *argv [2];

		argv [0] = boot_assembly;
		argv [1] = NULL;

		mono_jit_exec (moon_domain, moon_boot_assembly, 1, argv);

		moon_load_xaml = vm_get_method_from_name ("Moonlight.Loader:CreateXamlLoader");

		if (moon_load_xaml != NULL) {
			result = TRUE;
		}		
	}
	DEBUGMSG ("Mono Runtime: %s", result ? "OK" : "Failed");
	
	moon_vm_loaded = TRUE;

#if STACK_DEBUG
	enable_vm_stack_trace ();
#endif

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
	return GUINT_TO_POINTER (mono_gchandle_new (loader, FALSE));
}

void
vm_loader_destroy (gpointer loader_object)
{
	guint32 loader = GPOINTER_TO_UINT (loader_object);
	if (loader)
		mono_gchandle_free (loader);
}
#endif
