/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-accessibility-bridge.h: Glue between Moonlight.AtkBridge and NPAPI.
 *
 * Contact:
 *   Moonlight Accessibility List (mono-a11y@forge.novell.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
*
 */

#ifndef __PLUGIN_ACCESSIBILITY_BRIDGE_H__
#define __PLUGIN_ACCESSIBILITY_BRIDGE_H__

#include <config.h>
#if PAL_GTK_A11Y
#include <gdk/gdk.h>
#endif

#define INCLUDED_MONO_HEADERS 1

#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/appdomain.h>
#include <dlfcn.h>

#include "plugin-accessibility-bridge.h"
#include "plugin.h"

namespace Moonlight {

#if PAL_GTK_A11Y
void *atk_bridge_module = NULL;
AtkObject *accessible_root = NULL;

static int moon_atk_root_get_n_children (AtkObject *obj);
static AtkObject* moon_atk_root_get_parent (AtkObject *obj);
static AtkObject* moon_atk_root_ref_child (AtkObject *obj, gint i);
typedef void (*module_init_func) (void);
typedef char* (*get_id_func) (void* plug);

static void* get_atk_bridge_module ();
static gchar *get_path_from_var (const char *var);
static gchar *find_atk_bridge (const gchar *path);

// Unfortunately, we need to hardcode the GTK+ module paths, as there
// is no method, using pkg-config or otherwise, that returns them.
static const char *GTK_MODULE_PATHS [] = {
	"/usr/lib64/gtk-2.0/modules",
	"/usr/lib/gtk-2.0/modules",
	"/usr/lib64/gtk-2.0/modules/at-spi-dbus/modules",
	"/usr/lib/gtk-2.0/modules/at-spi-dbus/modules",
	NULL
};

static void*
get_atk_bridge_module ()
{
	gchar* path = NULL;
	gchar* atk_bridge_path = NULL;
	void* atk_bridge = NULL;

	/* Hack for OpenSUSE: Check GTK_PATH64 first */
	atk_bridge_path = get_path_from_var ("GTK_PATH64");
	if (!atk_bridge_path)
		atk_bridge_path = get_path_from_var ("GTK_PATH");
	if (!atk_bridge_path) {
		for (int i = 0; GTK_MODULE_PATHS [i] != NULL; i++) {
			path = g_module_build_path (GTK_MODULE_PATHS [i],
						    "atk-bridge");
			if (g_file_test (path, G_FILE_TEST_EXISTS)) {
				atk_bridge_path = path;
				break;
			}

			g_free (path);
		}
	}

	if (atk_bridge_path) {
		atk_bridge = dlopen (atk_bridge_path, RTLD_LAZY);
		g_free (atk_bridge_path);
	}

	return atk_bridge;
}

static gchar *
get_path_from_var (const char *var)
{
	const gchar *value = g_getenv (var);
	gchar **values;
	gchar **ptr;
	gchar *path = NULL;

	if (!value)
		return NULL;

	values = g_strsplit (value, ":", 0);
	for (ptr = values; *ptr; ptr++) {
		if (!path)
			path = find_atk_bridge (*ptr);
		g_free (*ptr);
	}
	g_free (values);
	return path;
}

static gchar *
find_atk_bridge (const gchar *path)
{
	gchar *full_path = g_strconcat (path, "/modules/libatk-bridge.so", NULL);

	if (g_file_test (full_path, G_FILE_TEST_EXISTS))
		return full_path;
	return NULL;
}

G_DEFINE_TYPE (MoonAtkRoot, moon_atk_root, ATK_TYPE_OBJECT)

static void
moon_atk_root_class_init (MoonAtkRootClass *klass)
{
	AtkObjectClass *object_class = ATK_OBJECT_CLASS (klass);
	object_class->get_n_children = moon_atk_root_get_n_children;
	object_class->ref_child = moon_atk_root_ref_child;
	object_class->get_parent = moon_atk_root_get_parent;
}

static void
moon_atk_root_init (MoonAtkRoot *root)
{
}

AtkObject*
moon_atk_root_new (void)
{
	return (AtkObject*) g_object_new (MOON_TYPE_ATK_ROOT, NULL);
}

static int
moon_atk_root_get_n_children (AtkObject *obj)
{
	if (!plugin_instances)
		return 0;
	return g_slist_length (plugin_instances);
}

static AtkObject*
moon_atk_root_get_parent (AtkObject *obj)
{
	return NULL;
}

static AtkObject*
moon_atk_root_ref_child (AtkObject *obj, gint i)
{
	g_return_val_if_fail (plugin_instances, NULL);

	PluginInstance* instance;

	instance = (PluginInstance*) g_slist_nth (plugin_instances, i);
	g_return_val_if_fail (instance, NULL);

	return instance->GetRootAccessible ();
}

#endif /* PAL_GTK_A11Y */

AccessibilityBridge::AccessibilityBridge ()
{
	char* plugin_dir;
	char* search;
	char* extensions_dir;
	char* filename;

	MonoMethod* accessibility_enabled;
	MonoImage* image;
	MonoObject* ret;

	is_accessibility_enabled = FALSE;
	bridge_asm = NULL;
	automation_bridge_class = NULL;
	automation_bridge = NULL;

	// Build a path to MoonAtkBridge.dll
	plugin_dir = g_strdup (get_plugin_dir ());

	search = strstr (plugin_dir, "moonlight@novell.com");
	if (search == NULL) {
		g_free (plugin_dir);
		return;
	}

	extensions_dir = g_strndup (plugin_dir, search - plugin_dir);

	filename = g_build_filename (
		extensions_dir,
		"moonlight-a11y@novell.com",
		"components",
		"MoonAtkBridge.dll",
		NULL
	);

	g_free (extensions_dir);
	g_free (plugin_dir);

	bridge_asm = mono_assembly_open (filename, NULL);
	if (!bridge_asm)
		return;

	image = mono_assembly_get_image (bridge_asm);
	g_return_if_fail (image);

	automation_bridge_class
		= mono_class_from_name (image, "Moonlight.AtkBridge",
		                        "AutomationBridge");
	g_return_if_fail (automation_bridge_class);

	accessibility_enabled
		= mono_class_get_method_from_name (automation_bridge_class,
		                                   "IsAccessibilityEnabled", 0);
	g_return_if_fail (accessibility_enabled);

	ret = mono_runtime_invoke (accessibility_enabled, NULL, NULL, NULL);
	if (!ret) {
		is_accessibility_enabled = FALSE;
		return;
	}

	is_accessibility_enabled
		= (bool) (*(MonoBoolean *) mono_object_unbox (ret));
}

bool
AccessibilityBridge::IsAccessibilityEnabled ()
{
	return is_accessibility_enabled;
}

void
AccessibilityBridge::Shutdown ()
{
	if (!is_accessibility_enabled)
		return;

	MonoMethod* shutdown;

	shutdown = mono_class_get_method_from_name (automation_bridge_class,
	                                            "Shutdown", 0);
	if (shutdown)
		mono_runtime_invoke (shutdown, automation_bridge, NULL, NULL);

#if PAL_GTK_A11Y
	if (atk_bridge_module) {
		dlclose (atk_bridge_module);
		atk_bridge_module = NULL;
	}
#endif
}

void
AccessibilityBridge::Initialize ()
{
	if (!is_accessibility_enabled)
		return;

	automation_bridge
		= mono_object_new (mono_domain_get (), automation_bridge_class);
	mono_runtime_object_init (automation_bridge);

	g_return_if_fail (automation_bridge);

#if PAL_GTK_A11Y
	atk_bridge_module = get_atk_bridge_module ();

	AccessibilityBridge::StartAtkBridge ();
#endif
}

#ifdef PAL_GTK_A11Y

AtkObject*
AccessibilityBridge::GetRootAccessible ()
{
	if (!is_accessibility_enabled)
		return NULL;

	MonoMethod* get_accessible;
	MonoObject *root;
	MonoClassField* m_value_field;
	AtkObject* handle;

	g_return_val_if_fail (automation_bridge, NULL);

	get_accessible
		= mono_class_get_method_from_name (automation_bridge_class,
		                                   "GetAccessible", 0);
	g_return_val_if_fail (get_accessible, NULL);

	root = mono_runtime_invoke (get_accessible,
	                            automation_bridge, NULL, NULL);
	if (root == NULL)
		return NULL;

	m_value_field = mono_class_get_field_from_name (mono_get_intptr_class (),
	                                                "m_value");
	g_return_val_if_fail (m_value_field, NULL);

	mono_field_get_value (root, m_value_field, &handle);
	g_return_val_if_fail (handle, NULL);

	return handle;
}


char*
AccessibilityBridge::GetPlugId (AtkObject *rootAccessible)
{
	if (rootAccessible == NULL)
	{
		g_warning ("GetID return NULL");
		return NULL;
	}

	if (ATK_IS_PLUG (rootAccessible)) {
		get_id_func plug_get_id = NULL;

		plug_get_id = (get_id_func) dlsym (
				atk_bridge_module, "atk_plug_get_id");
		if (plug_get_id)
			return plug_get_id (rootAccessible);
		else
			g_warning ("atk_plug_get_id does not exist");
	} else {
		g_warning ("Root Accessible is not an instance of AtkPlug, so it cannot be embedded.");
	}

	return NULL;
}

void
AccessibilityBridge::StartAtkBridge ()
{
	const gchar *at_bridge_val;
	bool bridge_disabled = false;

	at_bridge_val = g_getenv ("NO_AT_BRIDGE");

	// If $BROWSER disabled the atk-bridge, temporarily enable it for
	// ourselves
	if (at_bridge_val
	    && g_strcmp0 (at_bridge_val, "1") == 0) {
		bridge_disabled = true;
		g_setenv ("NO_AT_BRIDGE", "0", true);
	}

	// Load and initialize the bridge
	module_init_func gnome_accessibility_module_init = NULL;

	if (atk_bridge_module) {
		gnome_accessibility_module_init = (module_init_func) dlsym (
			atk_bridge_module, "gnome_accessibility_module_init");
	}

	if (gnome_accessibility_module_init)
		gnome_accessibility_module_init ();

	// Leave things as we found them
	if (bridge_disabled)
		g_setenv ("NO_AT_BRIDGE", "1", true);
}

void
AccessibilityBridge::ShutdownAtkBridge ()
{
	if (!atk_bridge_module)
		return;

	module_init_func gnome_accessibility_module_shutdown = NULL;
	gnome_accessibility_module_shutdown = (module_init_func) dlsym (
		atk_bridge_module, "gnome_accessibility_module_shutdown");
	if (gnome_accessibility_module_shutdown)
		gnome_accessibility_module_shutdown ();
}

#endif /* PAL_GTK_A11Y */

};
#endif /* __PLUGIN_ACCESSIBILITY_BRIDGE_H__ */
