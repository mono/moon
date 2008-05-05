
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

#if INCLUDE_MONO_RUNTIME
G_BEGIN_DECLS

bool vm_is_loaded (void);
bool vm_init (void);

gpointer vm_xaml_loader_new (XamlLoader* loader, gpointer plugin, gpointer surface, const char *file, const char *str);
gpointer vm_xaml_file_loader_new (XamlLoader* loader, gpointer plugin, gpointer surface, const char *file);
gpointer vm_xaml_str_loader_new (XamlLoader* loader, gpointer plugin, gpointer surface, const char *str);

bool     vm_application_create (gpointer plugin, gpointer surface, const char *file);
void     vm_application_destroy (gpointer handle);

void vm_loader_destroy  (gpointer loader_object);

G_END_DECLS
#endif
