/*
 * openfile.h: File open / save interfaces
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007, 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __OPENFILE_H__
#define __OPENFILE_H__

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gtk/gtk.h>

#include "runtime.h"
#include "deployment.h"

G_BEGIN_DECLS

#define MOONLIGHT_MINIMUM_FILE_ENTRY_COST	1024
#define MOONLIGHT_FILE_SIZE_MASK		(MOONLIGHT_MINIMUM_FILE_ENTRY_COST - 1)

/* @GeneratePInvoke */
char **open_file_dialog_show (const char *title, bool multsel, const char *filter, int idx) MOON_API;

/* @GeneratePInvoke */
char *save_file_dialog_show (const char *title, const char *filter, int idx) MOON_API;

// NOTE: this is used from 'mscorlib.dll' System.IO.IsolatedStorage/MoonIsolatedStorageFile.cs
gboolean isolated_storage_increase_quota_to (const char *primary_text, const char* secondary_text) MOON_API;

// NOTE: this is used from 'mscorlib.dll' System.IO.IsolatedStorage/MoonIsolatedStorage.cs
long isolated_storage_get_current_usage (const char* root) MOON_API;

G_END_DECLS

#endif
