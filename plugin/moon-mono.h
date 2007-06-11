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

G_BEGIN_DECLS
#include <mono/jit/jit.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>

extern MonoDomain   *moon_domain;
extern MonoAssembly *moon_boot_assembly;

gboolean    vm_init ();

G_END_DECLS
