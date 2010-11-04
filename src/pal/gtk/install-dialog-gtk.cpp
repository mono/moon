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

#include <glib.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "install-dialog-gtk.h"
#include "application.h"
#include "network.h"
#include "runtime.h"
#include "utils.h"
#include "uri.h"
#include "debug.h"

namespace Moonlight {

typedef struct {
	GdkPixbufLoader *loader;
	InstallDialog *dialog;
	int size;
} IconLoader;

struct _InstallDialogPrivate {
	Application *application;
	Deployment *deployment;
	HttpRequest *request;
	GPtrArray *loaders;
	GList *icon_list;
	
	char *install_dir;
	bool installed;
	bool unattended;
	
	GtkToggleButton *start_menu;
	GtkToggleButton *desktop;
	GtkLabel *primary_text;
	GtkProgressBar *progress;
	GtkWidget *ok_button;
	GtkImage *icon;
};

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
	GtkWidget *checkboxes, *primary, *secondary, *container;
	GtkWidget *vbox, *hbox, *label;
	InstallDialogPrivate *priv;
	
	dialog->priv = priv = g_new0 (InstallDialogPrivate, 1);
	
	gtk_window_set_title ((GtkWindow *) dialog, "Install application");
	gtk_window_set_resizable ((GtkWindow *) dialog, false);
	gtk_window_set_modal ((GtkWindow *) dialog, true);
	
	hbox = gtk_hbox_new (false, 12);
	
	priv->icon = (GtkImage *) gtk_image_new ();
	gtk_widget_show ((GtkWidget *) priv->icon);
	
	gtk_box_pack_start ((GtkBox *) hbox, (GtkWidget *) priv->icon, false, false, 0);
	
	priv->primary_text = (GtkLabel *) gtk_label_new ("");
	gtk_label_set_line_wrap (priv->primary_text, true);
	gtk_widget_show ((GtkWidget *) priv->primary_text);
	
	label = gtk_label_new ("Please confirm the location for the shortcuts.");
	gtk_label_set_line_wrap ((GtkLabel *) label, true);
	gtk_widget_show (label);
	
	priv->start_menu = (GtkToggleButton *) gtk_check_button_new_with_label ("Start menu");
	gtk_toggle_button_set_active (priv->start_menu, true);
	gtk_widget_show ((GtkWidget *) priv->start_menu);
	
	priv->desktop = (GtkToggleButton *) gtk_check_button_new_with_label ("Desktop");
	gtk_toggle_button_set_active (priv->desktop, false);
	gtk_widget_show ((GtkWidget *) priv->desktop);
	
	priv->progress = (GtkProgressBar *) gtk_progress_bar_new ();
	gtk_progress_bar_set_fraction (priv->progress, 0.0);
	gtk_widget_show ((GtkWidget *) priv->progress);
	
	vbox = gtk_vbox_new (false, 2);
	gtk_box_pack_start ((GtkBox *) vbox, (GtkWidget *) priv->start_menu, false, false, 0);
	gtk_box_pack_start ((GtkBox *) vbox, (GtkWidget *) priv->desktop, false, false, 0);
	gtk_widget_show (vbox);
	
	checkboxes = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_alignment_set_padding ((GtkAlignment *) checkboxes, 0, 0, 8, 0);
	gtk_container_add ((GtkContainer *) checkboxes, vbox);
	gtk_widget_show (checkboxes);
	
	vbox = gtk_vbox_new (false, 6);
	gtk_box_pack_start ((GtkBox *) vbox, label, false, false, 0);
	gtk_box_pack_start ((GtkBox *) vbox, checkboxes, false, false, 0);
	gtk_box_pack_start ((GtkBox *) vbox, (GtkWidget *) priv->progress, false, false, 0);
	gtk_widget_show (vbox);
	
	primary = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_alignment_set_padding ((GtkAlignment *) primary, 0, 0, 0, 0);
	gtk_container_add ((GtkContainer *) primary, (GtkWidget *) priv->primary_text);
	gtk_widget_show (primary);
	
	secondary = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_alignment_set_padding ((GtkAlignment *) secondary, 6, 0, 0, 0);
	gtk_container_add ((GtkContainer *) secondary, vbox);
	gtk_widget_show (secondary);
	
	vbox = gtk_vbox_new (false, 6);
	gtk_box_pack_start ((GtkBox *) vbox, primary, false, false, 0);
	gtk_box_pack_start ((GtkBox *) vbox, secondary, false, false, 0);
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
	priv->ok_button = gtk_dialog_add_button ((GtkDialog *) dialog, "_Install Now", GTK_RESPONSE_OK);
	gtk_dialog_set_default_response ((GtkDialog *) dialog, GTK_RESPONSE_CANCEL);
}

static void
install_dialog_finalize (GObject *obj)
{
	InstallDialog *dialog = (InstallDialog *) obj;
	InstallDialogPrivate *priv = dialog->priv;
	IconLoader *loader;
	guint i;
	
	if (priv->request) {
		if (!priv->request->IsCompleted ())
			priv->request->Abort ();
		priv->request->unref ();
	}

	if (!priv->installed) {
		// FIXME: use the InstallerService to uninstall so we don't
		// leave dummy records in the database...
		RemoveDir (priv->install_dir);
	}
	
	g_free (priv->install_dir);
	
	if (priv->loaders) {
		for (i = 0; i < priv->loaders->len; i++) {
			loader = (IconLoader *) priv->loaders->pdata[i];
			if (loader->loader)
				g_object_unref (loader->loader);
			g_free (loader);
		}
		
		g_ptr_array_free (priv->loaders, true);
	}
	
	g_list_free (priv->icon_list);
	
	priv->application->unref ();
	priv->deployment->unref ();
	
	g_free (priv);
	
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
install_dialog_destroy (GtkObject *obj)
{
	GTK_OBJECT_CLASS (parent_class)->destroy (obj);
}

static void
icon_loader_notify_cb (NotifyType type, gint64 args, gpointer user_data)
{
	IconLoader *loader = (IconLoader *) user_data;
	InstallDialogPrivate *priv = loader->dialog->priv;
	GdkPixbuf *pixbuf;
	
	switch (type) {
	case NotifyCompleted:
		if (loader->loader) {
			if (gdk_pixbuf_loader_close (loader->loader, NULL)) {
				/* get the pixbuf and add it to our icon_list */
				pixbuf = gdk_pixbuf_loader_get_pixbuf (loader->loader);
				priv->icon_list = g_list_prepend (priv->icon_list, pixbuf);
				
				if (loader->size == 128) {
					/* set the 128x128 pixbuf as the icon in our dialog */
					gtk_image_set_from_pixbuf (priv->icon, pixbuf);
				}
				
				gtk_window_set_icon_list ((GtkWindow *) loader->dialog, priv->icon_list);
				break;
			}
			
			/* fall through as if we got a NotifyFailed */
		}
	case NotifyFailed:
		if (loader->loader) {
			/* load default icon and destroy the loader */
			gtk_image_set_from_icon_name (priv->icon, "gnome-remote-desktop", GTK_ICON_SIZE_DIALOG);
			g_object_unref (loader->loader);
			loader->loader = NULL;
		}
		break;
	default:
		break;
	}
}

static void
icon_loader_write_cb (EventObject *sender, EventArgs *calldata, gpointer user_data)
{
	HttpRequestWriteEventArgs *args = (HttpRequestWriteEventArgs *) calldata;
	IconLoader *loader = (IconLoader *) user_data;
	
	if (loader->loader && !gdk_pixbuf_loader_write (loader->loader, (const guchar *) args->GetData (), args->GetCount (), NULL)) {
		/* loading failed, destroy the loader */
		g_object_unref (loader->loader);
		loader->loader = NULL;
	}
}

static void
error_dialog_response (GtkDialog *dialog, int response_id, gpointer user_data)
{
	GtkDialog *installer = (GtkDialog *) user_data;
	
	// cancel the install dialog
	gtk_dialog_response (installer, GTK_RESPONSE_CANCEL);
	
	// destroy the error dialog
	gtk_widget_destroy ((GtkWidget *) dialog);
}

static gboolean
click_ok_button (gpointer user_data)
{
	LOG_OOB ("OOB install: clicking ok button for real.\n");
	/* If we end up crashing here since the gtk dialog has been destroyed,
	 * have in mind that it can only happen while running tests, since the user
	 * can't install unattended */
	gtk_dialog_response ((GtkDialog *) user_data, GTK_RESPONSE_OK);
	return false;
}

static void
downloader_stopped (EventObject *sender, EventArgs *args, gpointer user_data)
{
	InstallDialog *installer = (InstallDialog *) user_data;
	InstallDialogPrivate *priv = installer->priv;
	GtkWidget *dialog;
	HttpRequestStoppedEventArgs *ea = (HttpRequestStoppedEventArgs *) args;

	LOG_OOB ("OOB install: downloader_stopped (IsSuccess: %i)\n", ea->IsSuccess ());

	if (ea->IsSuccess ()) {
		gtk_widget_set_sensitive (priv->ok_button, true);
		gtk_widget_hide ((GtkWidget *) priv->progress);
		if (priv->unattended) {
			LOG_OOB ("OOB install: downloader_stopped (): async clicking ok button since we're doing an unattended install\n");
			/* We can't click directly, for some reason the click is lost when running from file:// urls
			 * (maybe it's all happening too fast? it happens with #422). */
			g_timeout_add (1000, click_ok_button, installer);
		}
	} else {
		dialog = gtk_message_dialog_new ((GtkWindow *) installer,
						 (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR),
						 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
						 ea->GetErrorMessage ());

		gtk_window_set_title ((GtkWindow *) dialog, "Install Error");
		
		gtk_message_dialog_format_secondary_text ((GtkMessageDialog *) dialog,
							  "Failed to download application from %s",
							  priv->deployment->GetXapLocation ()->GetOriginalString ());
		
		g_signal_connect (dialog, "response", G_CALLBACK (error_dialog_response), installer);

		gtk_widget_show (dialog);
	}
}

static void
downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer user_data)
{
	HttpRequestProgressChangedEventArgs *args = (HttpRequestProgressChangedEventArgs *) calldata;
	InstallDialog *installer = (InstallDialog *) user_data;
	InstallDialogPrivate *priv = installer->priv;
	double fraction;
	char *label;
	
	fraction = args->GetProgress ();
	label = g_strdup_printf ("Downloading... %d%%", (int) (fraction * 100));
	gtk_progress_bar_set_fraction (priv->progress, fraction);
	gtk_progress_bar_set_text (priv->progress, label);
	g_free (label);
}

GtkDialog *
install_dialog_new (GtkWindow *parent, Deployment *deployment, const char *install_dir, bool unattended)
{
	InstallDialog *dialog = (InstallDialog *) g_object_new (INSTALL_DIALOG_TYPE, NULL);
	OutOfBrowserSettings *settings = deployment->GetOutOfBrowserSettings ();
	Application *application = deployment->GetCurrentApplication ();
	IconCollection *icons = settings->GetIcons ();
	InstallDialogPrivate *priv = dialog->priv;
	char *markup, *location;
	IconLoader *loader;
	int count, i;
	
	if (parent) {
		gtk_window_set_transient_for ((GtkWindow *) dialog, parent);
		gtk_window_set_destroy_with_parent ((GtkWindow *) dialog, true);
	}
	
	priv->application = application;
	application->ref ();
	
	priv->deployment = deployment;
	deployment->ref ();
	
	if (!deployment->GetXapLocation ()->IsScheme ("file")) {
		location = g_path_get_dirname (deployment->GetXapLocation ()->GetPath ());
	} else {
		location = g_strdup ("file://");
	}
	
	markup = g_markup_printf_escaped ("You are installing <b>%s</b> from <b>%s</b>",
					  settings->GetShortName (), location);
	gtk_label_set_markup (priv->primary_text, markup);
	g_free (location);
	g_free (markup);
	
	priv->install_dir = g_strdup (install_dir);
	priv->unattended = unattended;
	
	/* desensitize the OK button until the httprequest is complete */
	gtk_widget_set_sensitive (priv->ok_button, false);
	
	/* spin up a httprequest for the xap */
	priv->request = deployment->CreateHttpRequest (HttpRequest::DisableAsyncSend /* FIXME: why can't it be async? */);
	priv->request->AddHandler (HttpRequest::StoppedEvent, downloader_stopped, dialog);
	priv->request->AddHandler (HttpRequest::ProgressChangedEvent, downloader_progress_changed, dialog);
	priv->request->Open ("GET", deployment->GetXapLocation (), NULL, XamlPolicy);
	priv->request->Send ();
	
	/* load the icons */
	if (icons && (count = icons->GetCount ()) > 0) {
		priv->loaders = g_ptr_array_sized_new (count);
		
		for (i = 0; i < count; i++) {
			Value *value = icons->GetValueAt (i);
			Icon *icon = value->AsIcon ();
			Size *size = icon->GetSize ();
			const Uri *uri = icon->GetSource ();
			
			loader = g_new (IconLoader, 1);
			loader->size = MAX ((int) size->width, (int) size->height);
			loader->loader = gdk_pixbuf_loader_new ();
			loader->dialog = dialog;
			
			g_ptr_array_add (priv->loaders, loader);
			
			application->GetResource (NULL, uri, icon_loader_notify_cb, icon_loader_write_cb, MediaPolicy, HttpRequest::DisableFileStorage, NULL, loader);
		}
	}
	
	if (!priv->loaders)
		gtk_image_set_from_icon_name (priv->icon, "gnome-remote-desktop", GTK_ICON_SIZE_DIALOG);
	
	return (GtkDialog *) dialog;
}

bool
install_dialog_get_install_to_start_menu (InstallDialog *dialog)
{
	g_return_val_if_fail (IS_INSTALL_DIALOG (dialog), false);
	
	return gtk_toggle_button_get_active (dialog->priv->start_menu);
}

bool
install_dialog_get_install_to_desktop (InstallDialog *dialog)
{
	g_return_val_if_fail (IS_INSTALL_DIALOG (dialog), false);
	
	return gtk_toggle_button_get_active (dialog->priv->desktop);
}

static bool
install_xap (const char *xap_filename, const char *app_dir)
{
	GByteArray xap;
	char *content;
	gsize size;
	char *dirname;
	bool rv;
	
	LOG_OOB ("OOB install: install_xap (%s, %s)\n", xap_filename, app_dir);

	// Note: Instead of saving the compressed Xap, we'll be extracting it
	// to disk to both improve performance of OOB apps a bit but also so
	// that we don't have to clutter /tmp when the OOB app is run.
	
	dirname = g_build_filename (app_dir, "Application.xap", NULL);
	if (g_mkdir (dirname, 0777) == -1) {
		g_free (dirname);
		return false;
	}

	
	if (g_file_get_contents (xap_filename, &content, &size, NULL)) {
		xap.data = (guint8 *) content;
		xap.len = size;
		rv = UnzipByteArrayToDir (&xap, dirname, CanonModeXap);
		g_free (content);
	}
	g_free (dirname);
	
	return rv;
}

static bool
install_html (OutOfBrowserSettings *settings, const char *app_dir)
{
	WindowSettings *window = settings->GetWindowSettings ();
	int height = (int) window->GetHeight ();
	int width = (int) window->GetWidth ();
	char *filename, *title;
	FILE *fp;
	
	LOG_OOB ("OOB install: install_html (%s)\n", app_dir);

	filename = g_build_filename (app_dir, "index.html", NULL);
	if (!(fp = fopen (filename, "wt"))) {
		g_free (filename);
		return false;
	}
	
	g_free (filename);
	
	if (window && window->GetTitle ())
		title = g_markup_escape_text (window->GetTitle (), -1);
	else
		title = g_markup_escape_text (settings->GetShortName (), -1);
	
	fprintf (fp, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf (fp, "  <head>\n");
	fprintf (fp, "    <title>%s</title>\n", title);
	fprintf (fp, "    <script type=\"text/javascript\">\n");
	fprintf (fp, "      function ResizeWindow () {\n");
	if (window)
		fprintf (fp, "        window.resizeTo (%d, %d);\n", width, height);
	fprintf (fp, "      }\n");
	fprintf (fp, "    </script>\n");
	fprintf (fp, "    <style type='text/css'>");
	fprintf (fp, "      html, body { height: 100%%; overflow: auto; }");
	fprintf (fp, "      body { padding: 0; margin: 0; }");
	fprintf (fp, "      #MoonlightControl { height: 100%%; }");
	fprintf (fp, "    </style>");
	fprintf (fp, "  </head>\n");
	fprintf (fp, "  <body>\n");
	fprintf (fp, "    <div id=\"MoonlightControl\">\n");
	fprintf (fp, "      <object data=\"data:application/x-silverlight-2,\" type=\"application/x-silverlight-2\" width=\"100%%\" height=\"100%%\" onload=\"ResizeWindow\">\n");
	fprintf (fp, "        <param name=\"source\" value=\"Application.xap\"/>\n");
	fprintf (fp, "        <param name=\"out-of-browser\" value=\"true\"/>\n");
	fprintf (fp, "        <param name=\"enableGPUAcceleration\" value=\"False\"/>\n");
	fprintf (fp, "      </object>\n");
	fprintf (fp, "    </div>\n");
	fprintf (fp, "  </body>\n");
	fprintf (fp, "</html>\n");
	
	g_free (title);
	fclose (fp);
	
	return true;
}

static void
notify_cb (NotifyType type, gint64 args, gpointer user_data)
{
	FILE *fp = (FILE *) user_data;
	
	switch (type) {
	case NotifyCompleted:
	case NotifyFailed:
		fclose (fp);
		break;
	default:
		break;
	}
}

static void
write_cb (EventObject *sender, EventArgs *calldata, gpointer user_data)
{
	HttpRequestWriteEventArgs *args = (HttpRequestWriteEventArgs *) calldata;
	FILE *fp = (FILE *) user_data;
	
	fwrite (args->GetData (), 1, args->GetCount (), fp);
}

static void
install_icons (Application *application, OutOfBrowserSettings *settings, const char *install_dir)
{
	IconCollection *icons = settings->GetIcons ();
	char *filename, *icons_dir, name[64];
	int i, count;
	
	if (icons && (count = icons->GetCount ()) > 0) {
		icons_dir = g_build_filename (install_dir, "icons", NULL);
		
		for (i = 0; i < count; i++) {
			Value *value = icons->GetValueAt (i);
			Icon *icon = value->AsIcon ();
			Size *size = icon->GetSize ();
			
			if ((int) size->width == 48 || (int) size->height == 48) {
				/* we only need to extract the 48x48 icon for the .desktop files */
				const Uri *uri = icon->GetSource ();
				FILE *fp;
				
				g_mkdir_with_parents (icons_dir, 0777);
				
				snprintf (name, sizeof (name), "%dx%d.png", (int) size->width, (int) size->height);
				filename = g_build_filename (icons_dir, name, NULL);
				
				if ((fp = fopen (filename, "wb")))
					application->GetResource (NULL, uri, notify_cb, write_cb, MediaPolicy, HttpRequest::DisableFileStorage, NULL, fp);
				
				g_free (filename);
			}
		}
		
		g_free (icons_dir);
	}
}

static bool
install_update_uri (Deployment *deployment, OutOfBrowserSettings *settings, const char *app_dir)
{
	const Uri *uri = deployment->GetXapLocation ();
	bool error = false;
	char *path;
	FILE *fp;
	
	path = g_build_filename (app_dir, "update-uri", NULL);
	if (!(fp = fopen (path, "wt"))) {
		g_free (path);
		return false;
	}
	
	g_free (path);
	
	if (fprintf (fp, "%s\n", uri->GetOriginalString ()) < 0)
		error = true;
	
	fclose (fp);
	
	return !error;
}

static bool
install_gnome_desktop (OutOfBrowserSettings *settings, const char *app_dir, const char *filename)
{
	char *dirname, *icon_name, *quoted, *launcher;
	const char *platform_dir, *app_id;
	bool error = false;
	struct stat st;
	FILE *fp;
	int fd;
	
	LOG_OOB ("OOB install: install_gnome_desktop (%s, %s)\n", app_dir, filename);

	if (!(app_id = strrchr (app_dir, G_DIR_SEPARATOR)))
		return false;
	
	app_id++;
	
	dirname = g_path_get_dirname (filename);
	g_mkdir_with_parents (dirname, 0777);
	g_free (dirname);
	
	if ((fd = open (filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1)
		return false;
	
	if (!(fp = fdopen (fd, "wt"))) {
		close (fd); 
		return false;
	}
	
	if (fprintf (fp, "[Desktop Entry]\n") < 0)
		error = true;
	if (fprintf (fp, "Type=Application\n") < 0)
		error = true;
	if (fprintf (fp, "Encoding=UTF-8\n") < 0)
		error = true;
	if (fprintf (fp, "Version=1.0\n") < 0)
		error = true;
	if (fprintf (fp, "Terminal=false\n") < 0)
		error = true;
	if (fprintf (fp, "Categories=Application;Utility;\n") < 0)
		error = true;
	if (fprintf (fp, "Name=%s\n", settings->GetShortName ()) < 0)
		error = true;
	
	if (settings->GetBlurb ()) {
		if (fprintf (fp, "Comment=%s\n", settings->GetBlurb ()) < 0)
			error = true;
	}
	
	icon_name = g_build_filename (app_dir, "icons", "48x48.png", NULL);
	if (stat (icon_name, &st) != -1 && S_ISREG (st.st_mode)) {
		if (fprintf (fp, "Icon=%s\n", icon_name) < 0)
			error = true;
	}
	
	g_free (icon_name);
	
	if ((platform_dir = Deployment::GetPlatformDir ()))
		launcher = g_build_filename (platform_dir, "lunar-launcher", NULL);
	else
		launcher = g_strdup ("lunar-launcher");
	
	quoted = g_shell_quote (launcher);
	
	if (fprintf (fp, "Exec=%s %s\n", quoted, app_id) < 0)
		error = true;
	g_free (launcher);
	g_free (quoted);
	
	fclose (fp);
	
	return !error;
}

bool
install_dialog_install (InstallDialog *dialog)
{
	InstallDialogPrivate *priv = dialog->priv;
	OutOfBrowserSettings *settings;
	char *filename;
	
	g_return_val_if_fail (IS_INSTALL_DIALOG (dialog), false);
	
	if (priv->installed) {
		LOG_OOB ("OOB install: install_dialog_install: already installed\n");
		return true;
	}
	
	settings = priv->deployment->GetOutOfBrowserSettings ();
	
	if (g_mkdir_with_parents (priv->install_dir, 0777) == -1 && errno != EEXIST)
		return false;
	
	/* install the XAP */
	if (!install_xap (priv->request->GetFilename (), priv->install_dir)) {
		LOG_OOB ("OOB install: install_dialog_install: could not install xap\n");
		RemoveDir (priv->install_dir);
		return false;
	}
	
	/* install the HTML page */
	if (!install_html (settings, priv->install_dir)) {
		LOG_OOB ("OOB install: install_dialog_install: could not install html\n");
		RemoveDir (priv->install_dir);
		return false;
	}
	
	/* install the update uri */
	if (!install_update_uri (priv->deployment, settings, priv->install_dir)) {
		LOG_OOB ("OOB install: install_dialog_install: could not install update uri\n");
		RemoveDir (priv->install_dir);
		return false;
	}
	
	LOG_OOB ("OOB install: install_dialog_install: installed\n");

	priv->installed = true;
	
	/* install the icon(s) */
	install_icons (priv->application, settings, priv->install_dir);
	
	/* conditionally install start menu shortcut */
	if (install_dialog_get_install_to_start_menu (dialog)) {
		filename = install_utils_get_start_menu_shortcut (settings);
		install_gnome_desktop (settings, priv->install_dir, filename);
		g_free (filename);
	}
	
	/* conditionally install desktop shortcut */
	if (install_dialog_get_install_to_desktop (dialog)) {
		filename = install_utils_get_desktop_shortcut (settings);
		install_gnome_desktop (settings, priv->install_dir, filename);
		g_free (filename);
	}
	
	return true;
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

};
