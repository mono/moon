/*
 * moon-mono.cpp: Support routines to load the Mono VM as a browser plugin.
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

		result = TRUE;
		mono_jit_exec (moon_domain, moon_boot_assembly, 1, argv);
	}
	return result;
}
