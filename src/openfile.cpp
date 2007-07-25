/*
 * openfile.cpp: File open interfaces
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <gtk/gtk.h>
#include "openfile.h"

char *
open_file_dialog_show (const char *title, bool multsel, const char *filter, int idx)
{
	GtkWidget *widget = gtk_file_chooser_dialog_new (title, NULL, 
					    GTK_FILE_CHOOSER_ACTION_OPEN, 
					    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(widget), multsel != 0);

	gint code = gtk_dialog_run (GTK_DIALOG (widget));
	char *ret = NULL;

	if (code == GTK_RESPONSE_ACCEPT)
		ret = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));

	gtk_widget_destroy (widget);

	return ret;
}

