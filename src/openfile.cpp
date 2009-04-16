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
#include "openfile.h"

// FIXME: Silverlight 1.1 (alpha, refresh) doesn't allow the selection(*) of
// symlinks on Mac OSX. (*) actually it de-selects them on the fly. We should
// be able to duplicate this with lstat and the selection-changed event.

static void
set_filters (GtkFileChooser *chooser, const char* filter, int idx)
{
	if (!filter || (strlen (filter) <= 1))
		return;

	char **filters = g_strsplit (filter, "|", 0);

	// to be valid (managed code) we know we have an even number of items
	// (if not we're still safe by dropping the last one)
	int pos = 0;
	int n = g_strv_length (filters) >> 1;
	for (int i=0; i < n; i++) {
		char *name = g_strstrip (filters[pos++]);
		if (strlen (name) < 1)
			continue;

		char *pattern = g_strstrip (filters[pos++]);
		if (strlen (pattern) < 1)
			continue;

		GtkFileFilter *ff = gtk_file_filter_new ();
		gtk_file_filter_set_name (ff, g_strdup (name));
		// there can be multiple patterns in a single string
		if (g_strrstr (pattern, ";")) {
			int n = 0;
			char **patterns = g_strsplit (pattern, ";", 0);
			while (char *p = patterns[n++])
				gtk_file_filter_add_pattern (ff, g_strdup (p));
			g_strfreev (patterns);
		} else {
			// or a single one
			gtk_file_filter_add_pattern (ff, g_strdup (pattern));
		}
		gtk_file_chooser_add_filter (chooser, ff);
		// idx (FilterIndex) is 1 (not 0) based
		if (i == (idx - 1))
			gtk_file_chooser_set_filter (chooser, ff);
	}
	g_strfreev (filters);
}

char **
open_file_dialog_show (const char *title, bool multsel, const char *filter, int idx)
{
	GtkWidget *widget = gtk_file_chooser_dialog_new (title, NULL, 
					    GTK_FILE_CHOOSER_ACTION_OPEN, 
					    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	GtkFileChooser *chooser = GTK_FILE_CHOOSER (widget);
	set_filters (chooser, filter, idx);
	gtk_file_chooser_set_select_multiple (chooser, multsel ? TRUE : FALSE);

	char **ret = NULL;
	if (gtk_dialog_run (GTK_DIALOG (widget)) == GTK_RESPONSE_ACCEPT){
		GSList *k, *l = gtk_file_chooser_get_filenames (chooser);
		int i, count = g_slist_length (l);

		ret = g_new (char *, count + 1);
		ret [count] = NULL;
		
		for (i = 0, k = l; k; k = k->next)
			ret [i++] = (char *) k->data;

		g_slist_free (l);
	}

	gtk_widget_destroy (widget);

	return ret;
}

char *
save_file_dialog_show (const char *title, const char *filter, int idx)
{
	GtkWidget *widget = gtk_file_chooser_dialog_new (title, NULL, 
					    GTK_FILE_CHOOSER_ACTION_SAVE, 
					    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

	GtkFileChooser *chooser = GTK_FILE_CHOOSER (widget);
	set_filters (chooser, filter, idx);
	gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

	char *ret = NULL;
	if (gtk_dialog_run (GTK_DIALOG (widget)) == GTK_RESPONSE_ACCEPT)
		ret = gtk_file_chooser_get_filename (chooser);

	gtk_widget_destroy (widget);

	return ret;
}

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

