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
		
		MonoMethodDesc *desc = mono_method_desc_new ("Moonlight.Hosting:FromXaml", TRUE);
		moon_load_xaml = mono_method_desc_search_in_image (desc, mono_assembly_get_image (moon_boot_assembly));
		if (moon_load_xaml != NULL){
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
void
vm_load_xaml (gpointer surface, const char *file)
{
	if (moon_load_xaml == NULL)
		return;

	void *params [2];
	params [0] = &surface;
	params [1] = mono_string_new (moon_domain, file);
	mono_runtime_invoke (moon_load_xaml, NULL, params, NULL);
}
