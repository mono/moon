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

void *
open_file_dialog_create ()
{
	return gtk_file_chooser_dialog_new ("", NULL, 
					    GTK_FILE_CHOOSER_ACTION_OPEN, 
					    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
}

int
open_file_dialog_show (GtkWidget *widget, const char *title, bool multsel, const char *filter, int idx)
{
	gtk_window_set_title (GTK_WINDOW (widget), title);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(widget), multsel);

	gint code = gtk_dialog_run (GTK_DIALOG (widget));

	gtk_widget_hide (widget);
	return (code == GTK_RESPONSE_ACCEPT);
}

char *
open_file_dialog_get_selected_file (GtkWidget *widget)
{
	return gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
}

void
open_file_dialog_destroy (GtkWidget *widget)
{
	gtk_widget_destroy (widget);
}
