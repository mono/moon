/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * install-dialog.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef MOON_INSTALL_DIALOG_H
#define MOON_INSTALL_DIALOG_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <time.h>

#include "deployment.h"
#include "downloader.h"

#define INSTALL_DIALOG_TYPE            (install_dialog_get_type ())
#define INSTALL_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), INSTALL_DIALOG_TYPE, InstallDialog))
#define INSTALL_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), INSTALL_DIALOG_TYPE, InstallDialogClass))
#define IS_INSTALL_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INSTALL_DIALOG_TYPE))
#define IS_INSTALL_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), INSTALL_DIALOG_TYPE))
#define INSTALL_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), INSTALL_DIALOG_TYPE, InstallDialogClass))

namespace Moonlight {

typedef struct _InstallDialog InstallDialog;
typedef struct _InstallDialogClass InstallDialogClass;
typedef struct _InstallDialogPrivate InstallDialogPrivate;

struct _InstallDialog {
	GtkDialog parent_object;
	
	InstallDialogPrivate *priv;
};

struct _InstallDialogClass {
	GtkDialogClass parent_class;
};

GType install_dialog_get_type (void);

GtkDialog *install_dialog_new (GtkWindow *parent, Deployment *deployment, const char *install_dir, bool unattended);

bool install_dialog_get_install_to_start_menu (InstallDialog *dialog);
bool install_dialog_get_install_to_desktop (InstallDialog *dialog);

bool install_dialog_install (InstallDialog *dialog);

/* utility functions useful outside of the dialog */
char *install_utils_get_desktop_shortcut (OutOfBrowserSettings *settings);
char *install_utils_get_start_menu_shortcut (OutOfBrowserSettings *settings);

};
#endif /* MOON_INSTALL_DIALOG_H */
