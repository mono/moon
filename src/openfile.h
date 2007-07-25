/*
 * runtime.h: Core surface and canvas definitions.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __OPENFILE_H__
#define __OPENFILE_H__

G_BEGIN_DECLS

void    *open_file_dialog_create            ();
int      open_file_dialog_show              (GtkWidget *widget, const char *title,
					     bool multsel, const char *filter, int idx);
char    *open_file_dialog_get_selected_file (GtkWidget *widget);
void     open_file_dialog_destroy           (GtkWidget *widget);

G_END_DECLS

#endif
