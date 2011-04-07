/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * moonlightconfiguration.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "moonlightconfiguration.h"

namespace Moonlight {

MoonlightConfiguration::MoonlightConfiguration ()
{
#if PLUMB_ME
	filename = g_build_filename (g_get_user_config_dir (), "moonlight", "configuration", NULL);;
	data = g_key_file_new ();
	// We don't care about errors.
	g_key_file_load_from_file (data, filename, (GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS), NULL);
#endif
}

MoonlightConfiguration::~MoonlightConfiguration ()
{
#if PLUMB_ME
	g_key_file_free (data);
	g_free (filename);
#endif
}

void
MoonlightConfiguration::Save ()
{
#if PLUMB_ME
	gsize length;
	gchar *contents = g_key_file_to_data (data, &length, NULL);
	char *dir = g_path_get_dirname (filename);
	GError *error = NULL;

	// Make sure the directory exists
	if (g_mkdir_with_parents (dir, 0700) == -1)
		fprintf (stderr, "Moonlight: Could not create configuration directory '%s': %s.\n", dir, strerror (errno));


	if (!g_file_set_contents (filename, contents, length, &error)) {
		fprintf (stderr, "Moonlight: Could not store configuration in '%s': %s.\n", filename, error->message);
		g_error_free (error);
	}

	g_free (contents);
	g_free (dir);
#endif
}

gchar**
MoonlightConfiguration::GetKeys (const char *group)
{
#if PLUMB_ME
	return g_key_file_get_keys (data, group, NULL, NULL);
#else
	return NULL;
#endif
}

void
MoonlightConfiguration::RemoveKey (const char *group, const char *key)
{
#if PLUMB_ME
	g_key_file_remove_key (data,
			       group,
			       key,
			       NULL);
#endif
}

bool
MoonlightConfiguration::HasKey (const char *group, const char *key)
{
#if PLUMB_ME
	return (bool)g_key_file_has_key (data, group, key, NULL);
#else
	return FALSE;
#endif
}
	
void 
MoonlightConfiguration::SetBooleanValue (const char *group, const char *key, gboolean value)
{
#if PLUMB_ME
	g_key_file_set_boolean (data, group, key, value);
#endif
}

void
MoonlightConfiguration::SetStringValue (const char *group, const char *key, const char *value)
{
#if PLUMB_ME
	g_key_file_set_string (data, group, key, value);
#endif
}

char *
MoonlightConfiguration::GetStringValue (const char *group, const char *key)
{
#if PLUMB_ME
	return g_key_file_get_string (data, group, key, NULL);
#else
	return NULL;
#endif
}

bool
MoonlightConfiguration::GetBooleanValue (const char *group, const char *key)
{
#if PLUMB_ME
	return (bool)g_key_file_get_boolean (data, group, key, NULL);
#else
	return FALSE;
#endif
}


};
