/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * install-dialog.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "install-dialog.h"
#include "application.h"
#include "runtime.h"
#include "utils.h"
#include "uri.h"

static void install_dialog_class_init (InstallDialogClass *klass);
static void install_dialog_init (InstallDialog *dialog);
static void install_dialog_destroy (GtkObject *obj);
static void install_dialog_finalize (GObject *obj);


static GtkDialogClass *parent_class = NULL;


GType
install_dialog_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		static const GTypeInfo info = {
			sizeof (InstallDialogClass),
			NULL, /* base_class_init */
			NULL, /* base_class_finalize */
			(GClassInitFunc) install_dialog_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (InstallDialog),
			0,    /* n_preallocs */
			(GInstanceInitFunc) install_dialog_init,
		};
		
		type = g_type_register_static (GTK_TYPE_DIALOG, "InstallDialog", &info, (GTypeFlags) 0);
	}
	
	return type;
}

static void
install_dialog_class_init (InstallDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (klass);
	
	parent_class = (GtkDialogClass *) g_type_class_ref (GTK_TYPE_DIALOG);
	
	object_class->finalize = install_dialog_finalize;
	gtk_object_class->destroy = install_dialog_destroy;
}

static void
install_dialog_init (InstallDialog *dialog)
{
	GtkWidget *checkboxes, *input, *container;
	GtkWidget *vbox, *hbox, *label;
	
	gtk_window_set_title ((GtkWindow *) dialog, "Install application");
	gtk_window_set_resizable ((GtkWindow *) dialog, false);
	gtk_window_set_modal ((GtkWindow *) dialog, true);
	
	hbox = gtk_hbox_new (false, 12);
	
	dialog->icon = (GtkImage *) gtk_image_new ();
	gtk_widget_show ((GtkWidget *) dialog->icon);
	
	gtk_box_pack_start ((GtkBox *) hbox, (GtkWidget *) dialog->icon, false, false, 0);
	
	dialog->primary_text = (GtkLabel *) gtk_label_new ("");
	gtk_label_set_line_wrap (dialog->primary_text, true);
	gtk_widget_show ((GtkWidget *) dialog->primary_text);
	
	label = gtk_label_new ("Please confirm the location for the shortcuts.");
	gtk_label_set_line_wrap ((GtkLabel *) label, true);
	gtk_widget_show (label);
	
	dialog->start_menu = (GtkToggleButton *) gtk_check_button_new_with_label ("Start menu");
	gtk_toggle_button_set_active (dialog->start_menu, true);
	gtk_widget_show ((GtkWidget *) dialog->start_menu);
	
	dialog->desktop = (GtkToggleButton *) gtk_check_button_new_with_label ("Desktop");
	gtk_toggle_button_set_active (dialog->desktop, false);
	gtk_widget_show ((GtkWidget *) dialog->desktop);
	
	vbox = gtk_vbox_new (false, 2);
	gtk_box_pack_start ((GtkBox *) vbox, (GtkWidget *) dialog->start_menu, false, false, 0);
	gtk_box_pack_start ((GtkBox *) vbox, (GtkWidget *) dialog->desktop, false, false, 0);
	gtk_widget_show (vbox);
	
	checkboxes = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_alignment_set_padding ((GtkAlignment *) checkboxes, 0, 0, 8, 0);
	gtk_container_add ((GtkContainer *) checkboxes, vbox);
	gtk_widget_show (checkboxes);
	
	vbox = gtk_vbox_new (false, 6);
	gtk_box_pack_start ((GtkBox *) vbox, label, false, false, 0);
	gtk_box_pack_start ((GtkBox *) vbox, checkboxes, false, false, 0);
	gtk_widget_show (vbox);
	
	input = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_alignment_set_padding ((GtkAlignment *) input, 6, 0, 0, 0);
	gtk_container_add ((GtkContainer *) input, vbox);
	gtk_widget_show (input);
	
	vbox = gtk_vbox_new (false, 6);
	gtk_box_pack_start ((GtkBox *) vbox, (GtkWidget *) dialog->primary_text, false, false, 0);
	gtk_box_pack_start ((GtkBox *) vbox, input, false, false, 0);
	gtk_widget_show (vbox);
	
	gtk_box_pack_start ((GtkBox *) hbox, vbox, false, false, 0);
	gtk_container_set_border_width ((GtkContainer *) hbox, 12);
	gtk_widget_show (hbox);
	
	//container = gtk_dialog_get_content_area ((GtkDialog *) dialog);
	container = ((GtkDialog *) dialog)->vbox;
	gtk_container_add ((GtkContainer *) container, hbox);
	
	/* Add OK and Cancel buttons */
	gtk_dialog_set_has_separator ((GtkDialog *) dialog, false);
	gtk_dialog_add_button ((GtkDialog *) dialog, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	gtk_dialog_add_button ((GtkDialog *) dialog, GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_set_default_response ((GtkDialog *) dialog, GTK_RESPONSE_CANCEL);
}

static void
install_dialog_finalize (GObject *obj)
{
	InstallDialog *dialog = (InstallDialog *) obj;
	
	if (!dialog->installed)
		RemoveDir (dialog->install_dir);
	
	g_free (dialog->install_dir);
	
	dialog->deployment->unref ();
	
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
install_dialog_destroy (GtkObject *obj)
{
	GTK_OBJECT_CLASS (parent_class)->destroy (obj);
}

static void
notify_cb (NotifyType type, gint64 args, gpointer user_data)
{
	FILE *fp = (FILE *) user_data;
	
	switch (type) {
	case NotifyProgressChanged:
		break;
	case NotifyCompleted:
	case NotifyFailed:
		fclose (fp);
		break;
	}
}

static void
write_cb (void *buffer, gint32 offset, gint32 n, gpointer user_data)
{
	FILE *fp = (FILE *) user_data;
	
	fwrite (buffer, 1, n, fp); 
}

static void
extract_icons (Application *application, IconCollection *icons, const char *icons_dir)
{
	int count = icons->GetCount ();
	int i;
	
	for (i = 0; icons && i < count; i++) {
		Value *value = icons->GetValueAt (i);
		Icon *icon = value->AsIcon ();
		Uri *uri = icon->GetSource ();
		Size *size = icon->GetSize ();
		char *filename, name[64];
		FILE *fp;
		
		snprintf (name, sizeof (name), "%dx%d.png", (int) size->width, (int) size->height);
		filename = g_build_filename (icons_dir, name, NULL);
		
		if ((fp = fopen (filename, "wb")))
			application->GetResource (NULL, uri, notify_cb, write_cb, MediaPolicy, NULL, fp);
		
		g_free (filename);
	}
}

static const char *icon_names[4] = {
	"128x128.png", "48x48.png", "32x32.png", "16x16.png"
};

GtkDialog *
install_dialog_new (Deployment *deployment)
{
	InstallDialog *dialog = (InstallDialog *) g_object_new (INSTALL_DIALOG_TYPE, NULL);
	OutOfBrowserSettings *settings = deployment->GetOutOfBrowserSettings ();
	Application *application = deployment->GetCurrentApplication ();
	char *filename, *icons_dir, *markup, *location;
	IconCollection *icons = settings->GetIcons ();
	GdkPixbuf *pixbuf = NULL;
	guint i;
	
	dialog->deployment = deployment;
	deployment->ref ();
	
	if (g_ascii_strncasecmp (deployment->GetXapLocation (), "file:", 5) != 0) {
		location = g_path_get_dirname (deployment->GetXapLocation ());
	} else {
		location = g_strdup ("file://");
	}
	
	markup = g_markup_printf_escaped ("You are installing <b>%s</b> from <b>%s</b>",
					  settings->GetShortName (), location);
	gtk_label_set_markup (dialog->primary_text, markup);
	g_free (location);
	g_free (markup);
	
	dialog->install_dir = install_utils_get_install_dir (settings);
	g_mkdir_with_parents (dialog->install_dir, 0777);
	
	if (icons && icons->GetCount () > 0) {
		icons_dir = g_build_filename (dialog->install_dir, "icons", NULL);
		g_mkdir_with_parents (icons_dir, 0777);
		
		extract_icons (application, icons, icons_dir);
		
		for (i = 0; i < G_N_ELEMENTS (icon_names) && !pixbuf; i++) {
			filename = g_build_filename (icons_dir, icon_names[i], NULL);
			pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
			g_free (filename);
		}
		
		g_free (icons_dir);
	}
	
	if (!pixbuf) {
		gtk_image_set_from_icon_name (dialog->icon, "gnome-remote-desktop", GTK_ICON_SIZE_DIALOG);
	} else {
		gtk_image_set_from_pixbuf (dialog->icon, pixbuf);
		g_object_unref (pixbuf);
	}
	
	return (GtkDialog *) dialog;
}

bool
install_dialog_get_install_to_start_menu (InstallDialog *dialog)
{
	g_return_val_if_fail (IS_INSTALL_DIALOG (dialog), false);
	
	return gtk_toggle_button_get_active (dialog->start_menu);
}

bool
install_dialog_get_install_to_desktop (InstallDialog *dialog)
{
	g_return_val_if_fail (IS_INSTALL_DIALOG (dialog), false);
	
	return gtk_toggle_button_get_active (dialog->desktop);
}

static bool
install_xap (Deployment *deployment, const char *path)
{
	Surface *surface = deployment->GetSurface ();
	int fd;
	
	if ((fd = open (path, O_CREAT | O_EXCL | O_WRONLY, 0666)) == -1)
		return false;
	
	if (CopyFileTo (surface->GetSourceLocation (), fd) == -1)
		return false;
	
	return true;
}

static bool
install_html (const char *filename)
{
	FILE *fp;
	
	if (!(fp = fopen (filename, "wt")))
		return false;
	
	// FIXME: write out a template html file to load Application.xap
	
	fclose (fp);
	
	return true;
}

static bool
install_gnome_desktop (OutOfBrowserSettings *settings, const char *app_dir, const char *filename)
{
	char *icon_name;
	struct stat st;
	FILE *fp;
	guint i;
	
	if (!(fp = fopen (filename, "wt")))
		return false;
	
	fprintf (fp, "[Desktop Entry]\n");
	fprintf (fp, "Type=Application\n");
	fprintf (fp, "Encoding=UTF-8\n");
	fprintf (fp, "Version=1.0\n");
	fprintf (fp, "Terminal=false\n");
	fprintf (fp, "Name=%s\n", settings->GetShortName ());
	
	if (settings->GetBlurb ())
		fprintf (fp, "Comment=%s\n", settings->GetBlurb ());
	
	for (i = 1; i < G_N_ELEMENTS (icon_names); i++) {
		icon_name = g_build_filename (app_dir, "icons", icon_names[i], NULL);
		if (stat (icon_name, &st) != -1 && S_ISREG (st.st_mode)) {
			fprintf (fp, "Icon=%s\n", icon_name);
			g_free (icon_name);
			break;
		}
		
		g_free (icon_name);
	}
	
	// FIXME: this won't actually work...
	fprintf (fp, "Exec=MOONLIGHT_OUT_OF_BROWSER=1 firefox \"%s/index.html\"\n", app_dir);
	
	fclose (fp);
	
	return true;
}

bool
install_dialog_install (InstallDialog *dialog)
{
	OutOfBrowserSettings *settings;
	char *filename;
	
	g_return_val_if_fail (IS_INSTALL_DIALOG (dialog), false);
	
	if (dialog->installed)
		return true;
	
	settings = dialog->deployment->GetOutOfBrowserSettings ();
	
	/* Install the XAP */
	filename = g_build_filename (dialog->install_dir, "Application.xap", NULL);
	if (!install_xap (dialog->deployment, filename)) {
		g_free (filename);
		return false;
	}
	
	g_free (filename);
	
	/* Install the HTML page */
	filename = g_build_filename (dialog->install_dir, "index.html", NULL);
	if (!install_html (filename)) {
		g_free (filename);
		return false;
	}
	
	g_free (filename);
	
	dialog->installed = true;
	
	if (install_dialog_get_install_to_start_menu (dialog)) {
		filename = install_utils_get_start_menu_shortcut (settings);
		install_gnome_desktop (settings, dialog->install_dir, filename);
		g_free (filename);
	}
	
	if (install_dialog_get_install_to_desktop (dialog)) {
		filename = install_utils_get_desktop_shortcut (settings);
		install_gnome_desktop (settings, dialog->install_dir, filename);
		g_free (filename);
	}
	
	return true;
}


char *
install_utils_get_app_name (OutOfBrowserSettings *settings)
{
	// FIXME: not safe to use the short name like this... but it'll do for now.
	return g_strdup (settings->GetShortName ());
}

char *
install_utils_get_install_dir (OutOfBrowserSettings *settings)
{
	char *app_name, *install_dir;
	
	app_name = install_utils_get_app_name (settings);
	install_dir = g_build_filename (g_get_home_dir (), ".local", "share", "moonlight", "applications", app_name, NULL);
	g_free (app_name);
	
	return install_dir;
}

char *
install_utils_get_desktop_shortcut (OutOfBrowserSettings *settings)
{
	char *shortcut, *path;
	
	shortcut = g_strdup_printf ("%s.desktop", settings->GetShortName ());
	path = g_build_filename (g_get_home_dir (), "Desktop", shortcut, NULL);
	g_free (shortcut);
	
	return path;
}

char *
install_utils_get_start_menu_shortcut (OutOfBrowserSettings *settings)
{
	char *shortcut, *path;
	
	shortcut = g_strdup_printf ("%s.desktop", settings->GetShortName ());
	path = g_build_filename (g_get_home_dir (), ".local", "share", "applications", shortcut, NULL);
	g_free (shortcut);
	
	return path;
}
