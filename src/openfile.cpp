/*
 * openfile.cpp: File Access and I/O
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007, 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <glib/gstdio.h>

#include <gtk/gtk.h>

#include "openfile.h"
#include "runtime.h"
#include "deployment.h"

namespace Moonlight {

#define MOONLIGHT_MINIMUM_FILE_ENTRY_COST	1024
#define MOONLIGHT_FILE_SIZE_MASK		(MOONLIGHT_MINIMUM_FILE_ENTRY_COST - 1)

// NOTE: this is used from 'mscorlib.dll' System.IO.IsolatedStorage/MoonIsolatedStorageFile.cs
// NOTE: we let the caller supply the string so i18n can occur in managed land only
gboolean
isolated_storage_increase_quota_to (const char *primary_text, const char* secondary_text)
{
	// the dialog is displayed only if the action leading to this call was initiated directly from the user
	if (!Deployment::GetCurrent ()->GetSurface ()->IsUserInitiatedEvent ())
		return false;

	GtkWidget *widget = gtk_message_dialog_new_with_markup (NULL,
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						primary_text);

	gtk_window_set_title (GTK_WINDOW (widget), PACKAGE_STRING);
	gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (widget), secondary_text);

	gboolean result = (gtk_dialog_run (GTK_DIALOG (widget)) == GTK_RESPONSE_YES);
	gtk_widget_destroy (widget);
	return result;
}

// recursively calculate the size of the specified directory
static long
get_size (const char *root)
{
	// NOTE: just like Silverlight we give a minimum cost of 1KB for each 
	// directory and file to avoid disk exhaustion by empty files/directories.
	long result = MOONLIGHT_MINIMUM_FILE_ENTRY_COST;
	struct stat info;
	if (g_lstat (root, &info) != 0)
		return result;

	// there should be no link in IS but, if any, we're not following them
	if (S_ISLNK (info.st_mode))
		return result;

	if (S_ISDIR (info.st_mode)) {
		// scan everythins inside the directory
		GDir *dir = g_dir_open (root, 0, NULL);
		if (!dir)
			return result; // should never happen

		// note: g_dir_read_name *smartly* skips '.' and '..'
		const char *entry_name = g_dir_read_name (dir);
		while (entry_name) {
			char name [PATH_MAX];
			if (g_snprintf (name, PATH_MAX, "%s/%s", root, entry_name) <= PATH_MAX)
				result += get_size (name);

			entry_name = g_dir_read_name (dir);
		}
		g_dir_close (dir);
	} else {
		// file size is computed at 1KB boundaries, minimum of 1KB (fixed cost for a file-system entry)
		result = (info.st_size & ~MOONLIGHT_FILE_SIZE_MASK);
		if ((result == 0) || (info.st_size & MOONLIGHT_FILE_SIZE_MASK))
			result += MOONLIGHT_MINIMUM_FILE_ENTRY_COST;
	}

	return result;
}

// NOTE: this is used from 'mscorlib.dll' System.IO.IsolatedStorage/MoonIsolatedStorage.cs
long
isolated_storage_get_current_usage (const char* root)
{
	// XXX we should cache the value and invalidate it if something changed (e.g. using inotify)
	return get_size (root);
}


};
