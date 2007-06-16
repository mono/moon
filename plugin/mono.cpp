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
#include "moon-mono.h"

MonoDomain   *moon_domain;
MonoAssembly *moon_boot_assembly;
static char *boot_assembly;

// Methods
MonoMethod   *moon_load_xaml;
MonoMethod   *moon_try_load;
MonoMethod   *moon_insert_mapping;

gboolean
vm_init ()
{
	const char *home = g_get_home_dir ();
	gboolean result = FALSE;

	boot_assembly = g_build_path ("/", home, ".mozilla", "plugins", "moonlight.exe", NULL);
	mono_config_parse (NULL);
	moon_domain = mono_jit_init (boot_assembly);
	moon_boot_assembly = mono_domain_assembly_open (moon_domain, boot_assembly);
	if (moon_boot_assembly){
		char *argv [2];

		argv [0] = boot_assembly;
		argv [1] = NULL;

		mono_jit_exec (moon_domain, moon_boot_assembly, 1, argv);
		
		MonoMethodDesc *desc;

		desc = mono_method_desc_new ("Moonlight.Hosting:CreateXamlLoader", TRUE);
		moon_load_xaml = mono_method_desc_search_in_image (desc, mono_assembly_get_image (moon_boot_assembly));
		mono_method_desc_free (desc);

		desc = mono_method_desc_new ("Moonlight.Loader:TryLoad", TRUE);
		moon_try_load = mono_method_desc_search_in_image (desc, mono_assembly_get_image (moon_boot_assembly));
		mono_method_desc_free (desc);

		desc = mono_method_desc_new ("Moonlight.Loader:InsertMapping", TRUE);
		moon_insert_mapping = mono_method_desc_search_in_image (desc, mono_assembly_get_image (moon_boot_assembly));
		mono_method_desc_free (desc);

		if (moon_load_xaml != NULL && moon_try_load != NULL){
			result = TRUE;
		}

		
	}
	DEBUGMSG ("Mono Runtime: %s\n", result ? "OK" : "Failed");
	return result;
}

//
// This instructs the managed code to load the Xaml file,
// the signature will change as we add the extra parameters
// that the plugin uses (instead of just <embed src>)
//
gpointer
vm_xaml_loader_new (gpointer plugin, gpointer surface, const char *file)
{
	if (moon_load_xaml == NULL)
		return NULL;

	void *params [3];
	params [0] = &plugin;
	params [1] = &surface;
	params [2] = mono_string_new (moon_domain, file);
	return mono_runtime_invoke (moon_load_xaml, NULL, params, NULL);
}

char *
vm_loader_try (gpointer loader_object, int *error)
{
	if (moon_try_load == NULL)
		return NULL;

	void *params [1];
	params [0] = &error;

	MonoString *ret = (MonoString *) mono_runtime_invoke (moon_try_load, loader_object, params, NULL);
	
	return mono_string_to_utf8 (ret);
}

//
// Inserts the original file request with its mapping into the
// managed loader hashtable  (bin/MyAssembly.dll, ~/.mozilla/tmp/xxx.dll)
//
void
vm_insert_mapping (gpointer loader_object, const char *key, const char *value)
{
	void *params [2];
	params [0] = mono_string_new (moon_domain, key);
	params [1] = mono_string_new (moon_domain, value);

	mono_runtime_invoke (moon_insert_mapping, loader_object, params, NULL);
}
