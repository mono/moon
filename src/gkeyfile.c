/*
 * Grouped key/value files
 *
 * Author:
 *   Chris Toshok  (toshok@novell.com)
 *
 * (C) 2011 Novell, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <config.h>

#if !PAL_GTK_WINDOWING
#include <stdlib.h>
#include "gkeyfile.h"

struct _GKeyFile {
	GHashTable *groups;
};

GKeyFile*
g_key_file_new (void)
{
	GKeyFile *rv = g_new (GKeyFile, 1);
	rv->groups = g_hash_table_new_full (g_str_hash, g_str_equal,
					    (GDestroyNotify)g_free, (GDestroyNotify)g_hash_table_destroy);
	return rv;
}

void
g_key_file_free (GKeyFile *key_file)
{
	g_return_if_fail (key_file != NULL);
	g_hash_table_destroy (key_file->groups);
	g_free (key_file);
}

gboolean
g_key_file_load_from_file (GKeyFile *key_file, const gchar *file, GKeyFileFlags flags, GError **error)
{
	char *file_contents;
	gsize file_length;
	gboolean load_rv;

	if (!g_file_get_contents (file, &file_contents, &file_length, error))
		return FALSE;

	load_rv = g_key_file_load_from_data (key_file, file_contents, file_length, flags, error);

	g_free (file_contents);

	return load_rv;
}

typedef struct {
	GKeyFile *key_file;
	GHashTable *current_group;
} GKeyFileParseState;

static gboolean
parse_group_line (GKeyFileParseState *state, gchar *line, GError **error)
{
	char *p;

	line++; /* skip the [ */
	
	p = strchr (line, ']');

	if (!p) {
		/* no closing ] */
		/* FIXME: GError */
		return FALSE;
	}

	if (*(p + 1) != '\0') {
		/* there's more stuff after the ], error out? */
		/* FIXME: GError */
		return FALSE;
	}

	*p = '\0';

	if (strlen (line) == 0) {
		/* the line was [] */
		/* FIXME: GError */
		return FALSE;
	}
	  
	/* create the group */
	/* FIXME: error if the group already exists? */

	state->current_group = g_hash_table_new_full (g_str_hash, g_str_equal,
						      (GDestroyNotify)g_free, (GDestroyNotify)g_free);

	g_hash_table_insert (state->key_file->groups,
			     g_strdup (line),
			     state->current_group);

	return TRUE;
}

static gboolean
parse_key_value_line (GKeyFileParseState *state, gchar *line, GError **error)
{
	if (!state->current_group) {
		/* a key value line without a previous group line */
		/* FIXME: GError */
		return FALSE;
	}

	gchar **split = g_strsplit (line, "=", -1);

	if (!split || !split[0] || !split[1] || split[2]) {
		/* the line doesn't have 2 values (it's either less or more) */
		/* FIXME: error */
		g_strfreev (split);
		return FALSE;
	}

	/* FIXME we don't handle locales in the keys */
	g_hash_table_insert (state->current_group, g_strdup (split[0]), g_strdup (split[1]));
	g_strfreev (split);
	return TRUE;
}

gboolean
g_key_file_load_from_data (GKeyFile *key_file, const gchar *data, gsize length, GKeyFileFlags flags, GError **error)
{
	GKeyFileParseState state;
	const gchar *start_line, *end_line;

	state.current_group = NULL;
	state.key_file = key_file;

	start_line = data;
	do {
		char *line;
		int line_length;

		if (!*start_line) {
			/* EOF */
			break;
		}

		end_line = strchr (start_line, '\n');
		if (end_line)
			line_length = end_line - start_line;
		else {
			end_line = start_line + strlen(start_line);
			line_length = strlen (start_line);
		}

		/* process the line */
		if (start_line[0] == '#') {
			/* comment line, nothing to do */
		}
		else if (start_line[0] == '[')  {
			line = g_strndup (start_line, line_length);
			/* potentially a group name */
			if (!parse_group_line (&state, line, error))
				return FALSE;
		}
		else {
			line = g_strndup (start_line, line_length);
			/* if it's an empty line, do nothing */

			/* otherwise, it's a key=value */

			if (!parse_key_value_line (&state, line, error))
				return FALSE;
		}

		if (*end_line != '\n')
			break;

		start_line = end_line + 1;

	} while (TRUE);

	return TRUE;
}

gchar*
g_key_file_to_data (GKeyFile *key_file, gsize *length, GError **error)
{
	GHashTableIter groups_iter;
	GString *gstr;
	char *group_name;
	GHashTable *group;

	g_return_val_if_fail (key_file != NULL, NULL);

	gstr = g_string_new ("");
	g_hash_table_iter_init (&groups_iter, key_file->groups);

	while (g_hash_table_iter_next (&groups_iter, (gpointer*)&group_name, (gpointer*)&group)) {
		GHashTableIter keys_iter;
		char *key;
		char *value;

		g_string_append_printf (gstr, "\n[%s]\n", group_name);

		g_hash_table_iter_init (&keys_iter, group);

		while (g_hash_table_iter_next (&keys_iter, (gpointer*)&key, (gpointer*)&value))
			g_string_append_printf(gstr, "%s=%s\n", key, value);
	}

	return g_string_free (gstr, FALSE);
}

gchar**
g_key_file_get_groups (GKeyFile *key_file, gsize *length)
{
	GHashTableIter groups_iter;
	gchar *group_name;
	int num_groups;
	gchar **group_names;
	int i;

	g_return_val_if_fail (key_file != NULL, NULL);

	num_groups = g_hash_table_size (key_file->groups);
	group_names = g_new(gchar*, num_groups+1);

	g_hash_table_iter_init (&groups_iter, key_file->groups);

	i = 0;
	while (g_hash_table_iter_next (&groups_iter, (gpointer*)&group_name, NULL)) {
	  group_names[i++] = g_strdup (group_name);
	}
	group_names[i] = NULL;  /* make sure we null terminate the array */

	if (length)
	  *length = num_groups;

	return group_names;
}

gchar**
g_key_file_get_keys (GKeyFile *key_file, const gchar *group_name, gsize *length, GError **error)
{
	GHashTableIter keys_iter;
	char *key_name;
	int num_keys;
	gchar **key_names;
	int i;
	GHashTable *group;


	g_return_val_if_fail (key_file != NULL, NULL);
	g_return_val_if_fail (group_name != NULL, NULL);

	group = g_hash_table_lookup (key_file->groups, group_name);
	if (group == NULL) {
		/* FIXME: error */
		return NULL;
	}

	num_keys = g_hash_table_size (group);
	key_names = g_new(gchar*, num_keys+1);


	g_hash_table_iter_init (&keys_iter, group);

	i = 0;
	while (g_hash_table_iter_next (&keys_iter, (gpointer*)&key_name, NULL)) {
	  key_names[i++] = g_strdup (key_name);
	}
	key_names[i] = NULL;  /* make sure we null terminate the array */

	if (length)
	  *length = num_keys;

	return key_names;
}

gboolean
g_key_file_has_key (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error)
{
	GHashTable *group;

	g_return_val_if_fail (key_file != NULL, FALSE);
	g_return_val_if_fail (group_name != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	group = (GHashTable*)g_hash_table_lookup (key_file->groups, group_name);
	if (!group) {
		// XXX error
		return FALSE;
	}

	return g_hash_table_lookup_extended (group, key, NULL, NULL);
}

gboolean
g_key_file_remove_key (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error)
{
	GHashTable *group;
	
	g_return_val_if_fail (key_file != NULL, FALSE);
	g_return_val_if_fail (group_name != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	group =(GHashTable*)g_hash_table_lookup (key_file->groups, group_name);
	if (!group) {
		// XXX error
		return FALSE;
	}

	g_hash_table_remove (group, key);

	// we should return false and throw an error if the key isn't there
	return TRUE;
}

void
g_key_file_set_boolean (GKeyFile *key_file, const gchar *group_name, const gchar *key, gboolean value)
{
	GHashTable *group;
	
	g_return_if_fail (key_file != NULL);
	g_return_if_fail (group_name != NULL);
	g_return_if_fail (key != NULL);

	group =(GHashTable*)g_hash_table_lookup (key_file->groups, group_name);
	if (!group) {
		group = g_hash_table_new_full (g_str_hash, g_str_equal,
					       (GDestroyNotify)g_free, (GDestroyNotify)g_free);
		g_hash_table_insert (key_file->groups, g_strdup(group_name), group);
	}

	g_hash_table_insert (group, g_strdup (key), g_strdup (value ? "true" : "false"));
}

void
g_key_file_set_string (GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *string)
{
	GHashTable *group;
	
	g_return_if_fail (key_file != NULL);
	g_return_if_fail (group_name != NULL);
	g_return_if_fail (key != NULL);

	group =(GHashTable*)g_hash_table_lookup (key_file->groups, group_name);
	if (!group) {
		group = g_hash_table_new_full (g_str_hash, g_str_equal,
					       (GDestroyNotify)g_free, (GDestroyNotify)g_free);
		g_hash_table_insert (key_file->groups, g_strdup(group_name), group);
	}

	g_hash_table_insert (group, g_strdup (key), g_strdup (string));
}

gboolean
g_key_file_get_boolean (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error)
{
	GHashTable *group;
	char *value;

	g_return_val_if_fail (key_file != NULL, FALSE);
	g_return_val_if_fail (group_name != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	group =(GHashTable*)g_hash_table_lookup (key_file->groups, group_name);
	if (!group) {
		group = g_hash_table_new_full (g_str_hash, g_str_equal,
					       (GDestroyNotify)g_free, (GDestroyNotify)g_free);
		g_hash_table_insert (key_file->groups, g_strdup(group_name), group);
	}

	value = (char*)g_hash_table_lookup (group, key);
	return value ? !strcasecmp (value, "true") : FALSE;
	// XXX we need to check for the key's existence and set error if it's not there
}

gchar*
g_key_file_get_string (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error)
{
	GHashTable *group;

	g_return_val_if_fail (key_file != NULL, FALSE);
	g_return_val_if_fail (group_name != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	group =(GHashTable*)g_hash_table_lookup (key_file->groups, group_name);
	if (!group) {
		group = g_hash_table_new_full (g_str_hash, g_str_equal,
					       (GDestroyNotify)g_free, (GDestroyNotify)g_free);
		g_hash_table_insert (key_file->groups, g_strdup(group_name), group);
	}

	return (char*)g_hash_table_lookup (group, key);
	// XXX we need to check for the key's existence and set error if it's not there
}

gchar*
g_key_file_get_value (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error)
{
	// XXX there are additional rules re: string escaping here (or at least their are differences between _get_value and _get_string),
	// but in the interest of expediency:
	return g_key_file_get_string (key_file, group_name, key, error);
}

#endif
