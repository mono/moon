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

#ifndef __G_KEY_FILE_H__
#define __G_KEY_FILE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GKeyFile GKeyFile;

typedef enum {
	G_KEY_FILE_ERROR_UNKNOWN_ENCODING,
	G_KEY_FILE_ERROR_PARSE,
	G_KEY_FILE_ERROR_NOT_FOUND,
	G_KEY_FILE_ERROR_KEY_NOT_FOUND,
	G_KEY_FILE_ERROR_GROUP_NOT_FOUND,
	G_KEY_FILE_ERROR_INVALID_VALUE
} GKeyFileError;

typedef enum {
	G_KEY_FILE_NONE              = 0,
	G_KEY_FILE_KEEP_COMMENTS     = 1 << 0,
	G_KEY_FILE_KEEP_TRANSLATIONS = 1 << 1
} GKeyFileFlags;

GKeyFile *g_key_file_new (void);
void      g_key_file_free (GKeyFile *key_file);
gboolean  g_key_file_load_from_file (GKeyFile *key_file, const gchar *file, GKeyFileFlags flags, GError **error);
gboolean  g_key_file_load_from_data (GKeyFile *key_file, const gchar *data, gsize length, GKeyFileFlags flags, GError **error);
gchar    *g_key_file_to_data (GKeyFile *key_file, gsize *length, GError **error);
gchar   **g_key_file_get_groups (GKeyFile *key_file, gsize *length);
gchar   **g_key_file_get_keys (GKeyFile *key_file, const gchar *group_name, gsize *length, GError **error);
gboolean  g_key_file_has_key (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
gboolean  g_key_file_remove_key (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
void      g_key_file_set_boolean (GKeyFile *key_file, const gchar *group_name, const gchar *key, gboolean value);
void      g_key_file_set_string (GKeyFile *key_file, const gchar *group_name, const gchar *key, const gchar *string);
gboolean  g_key_file_get_boolean (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
gchar    *g_key_file_get_string (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);
gchar    *g_key_file_get_value (GKeyFile *key_file, const gchar *group_name, const gchar *key, GError **error);

G_END_DECLS

#endif /* __G_KEY_FILE_H__ */
