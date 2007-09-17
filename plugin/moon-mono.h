
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

gboolean    vm_init (void);
gpointer    vm_xaml_file_loader_new (XamlLoader* loader, gpointer plugin, gpointer surface, const char *file);
gpointer    vm_xaml_str_loader_new (XamlLoader* loader, gpointer plugin, gpointer surface, const char *str);

char       *vm_loader_try      (gpointer loader_object, int *error);
void        vm_insert_mapping  (gpointer loader_object, const char *key, const char *value);
void        vm_loader_destroy  (gpointer loader_object);

G_END_DECLS
#endif
