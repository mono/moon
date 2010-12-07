/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * config-dialog-gtk.cpp: right click dialog for gtk
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <glib.h>

#include "config-dialog-gtk.h"

#include "window-gtk.h"
#include "openfile.h"
#include "pipeline.h"
#include "timemanager.h"
#include "debug-ui-gtk.h"
#include "consent.h"
#include "icon128.h"

#define PLUGIN_OURNAME      "Novell Moonlight"

using namespace Moonlight;

MoonConfigDialogGtk::MoonConfigDialogGtk (MoonWindowGtk *window, Surface *surface, Deployment *deployment)
  : window (window), surface (surface), deployment (deployment)
{
	GtkBox *vbox;

	dialog = gtk_dialog_new_with_buttons ("Novell Moonlight Configuration", NULL, (GtkDialogFlags)
					      GTK_DIALOG_NO_SEPARATOR,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_NONE, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 8);

	vbox = GTK_BOX (GTK_DIALOG (dialog)->vbox);

	notebook = gtk_notebook_new ();

	g_signal_connect (notebook, "switch-page", G_CALLBACK (MoonConfigDialogGtk::notebook_switch_page), this);

	pages = new ArrayList ();

	AddNotebookPage ("About", new AboutConfigDialogPage ());
	AddNotebookPage ("Playback", new PlaybackConfigDialogPage ());
	AddNotebookPage ("Storage", new StorageConfigDialogPage ());
	AddNotebookPage ("Permissions", new PermissionsConfigDialogPage ());
#if 0
	AddNotebookPage ("Applications", new ApplicationConfigDialogPage ());
#endif

#if DEBUG
	AddNotebookPage ("Debug", new DebugConfigDialogPage ());
#endif
	AddNotebookPage ("Advanced", new AdvancedConfigDialogPage ());

	gtk_box_pack_start (vbox, notebook, TRUE, TRUE, 0);

	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
}

MoonConfigDialogGtk::~MoonConfigDialogGtk ()
{
	delete pages;
}

void
MoonConfigDialogGtk::notebook_switch_page (GtkNotebook     *notebook,
					   GtkNotebookPage *page,
					   guint            page_num,
					   MoonConfigDialogGtk *dialog)
{
	((ConfigDialogPage*)(*dialog->pages)[page_num])->PageActivated ();
}

void
MoonConfigDialogGtk::AddNotebookPage (const char *label_str,
				      ConfigDialogPage *page)
{
	pages->Add (page);

	page->SetDialog (this);

	GtkWidget *label = gtk_label_new (label_str);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  page->GetContentWidget(),
				  label);
}

void
MoonConfigDialogGtk::Show ()
{
	gtk_widget_show_all (dialog);
}

////// About page
AboutConfigDialogPage::AboutConfigDialogPage()
{
}

AboutConfigDialogPage::~AboutConfigDialogPage ()
{
}

static void
bug_report_info (AboutConfigDialogPage *page)
{
	Surface *surface = page->GetDialog()->GetSurface();
	Deployment *deployment = page->GetDialog()->GetDeployment();
	MoonWindowGtk *window = page->GetDialog()->GetWindow();

	GtkWidget *dlg = gtk_dialog_new_with_buttons ("But Report Info",
						      NULL /* FIXME parent */,
						      (GtkDialogFlags)(GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_DESTROY_WITH_PARENT),
						      "Close", GTK_RESPONSE_CLOSE,
						      NULL);

	GtkWidget *vbox = GTK_DIALOG (dlg)->vbox;

	GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
	GtkWidget *image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING,
						     GTK_ICON_SIZE_BUTTON);
	GtkWidget *label = gtk_label_new ("Cut and paste the information below into your bug report.  "
					  "Be aware this contains information about the current silverlight "
					  "application you are viewing.");
	GtkWidget *textview;
	GtkTextBuffer *buffer = gtk_text_buffer_new (NULL);
	GString *str = g_string_new ("");

	g_string_append_printf (str, "Source: %s\n", deployment->GetXapLocation () ? deployment->GetXapLocation ()->GetOriginalString () : NULL);
	g_string_append_printf (str, "Width: %dpx\n", window->GetWidth ());
	g_string_append_printf (str, "Height: %dpx\n", window->GetHeight ());
	g_string_append_printf (str, "Background: %s\n", color_to_string (surface->GetBackgroundColor ()));
	g_string_append_printf (str, "RuntimeVersion: %s\n", deployment->GetRuntimeVersion() ? deployment->GetRuntimeVersion() : "(Unknown)");
	g_string_append_printf (str, "Windowless: %s\n", window->GetWidget() == NULL ? "yes" : "no");
	g_string_append_printf (str, "MaxFrameRate: %i\n", surface->GetTimeManager()->GetMaximumRefreshRate());

	g_string_append_printf (str, "Codecs: %s\n",
				Media::IsMSCodecsInstalled () ? "ms-codecs" :
#if INCLUDE_FFMPEG
				"ffmpeg"
#else
				"none"
#endif
				);


	g_string_append (str, "Build configuration: ");

#if DEBUG
	g_string_append (str, "debug");
#else
	g_string_append (str, "release");
#endif
#if SANITY
	g_string_append (str, ", sanity checks");
#endif
#if OBJECT_TRACKING
	g_string_append (str, ", object tracking");
#endif

	g_string_append (str, "\n");

	gtk_text_buffer_set_text (buffer, str->str, str->len);

	textview = gtk_text_view_new_with_buffer (buffer);

	g_string_free (str, TRUE);

	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

	GtkWidget *image_align = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (image_align), image);

	gtk_box_pack_start (GTK_BOX (hbox), image_align, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), textview, TRUE, TRUE, 0);

	gtk_widget_show_all (vbox);

	if (GTK_RESPONSE_CLOSE == gtk_dialog_run (GTK_DIALOG (dlg)))
		gtk_widget_destroy (dlg);
}

GtkWidget*
AboutConfigDialogPage::GetContentWidget ()
{
	GtkWidget *label, *linkbutton, *vbox, *align, *image, *button;
	GtkTable *table = GTK_TABLE (gtk_table_new (3, 3, false));

	GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();

	gdk_pixbuf_loader_write (loader, ICON_128, sizeof (ICON_128), NULL);

	GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);

	image = gtk_image_new_from_pixbuf (pixbuf);

	gdk_pixbuf_loader_close (loader, NULL);
	g_object_unref (G_OBJECT (loader));

	align = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (align), image);

	gtk_table_attach (table, align, 0, 1, 0, 1,
			  (GtkAttachOptions)0,
			  (GtkAttachOptions)0,
			  0, 0);

	vbox = gtk_vbox_new (FALSE, 0);

	label = gtk_label_new ("");

	gtk_label_set_markup (GTK_LABEL (label),
			      "<span size=\"xx-large\" weight=\"bold\">" PLUGIN_OURNAME " " VERSION "</span>\n"
			      "<span size=\"small\">Copyright 2007-2010 Novell, Inc. (http://www.novell.com/)</span>");

	gtk_label_set_justify (GTK_LABEL (label),
			       GTK_JUSTIFY_LEFT);

	align = gtk_alignment_new (0.0, 0.0, 0.0, 1.0);
	gtk_container_add (GTK_CONTAINER (align), label);

	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (align), FALSE, FALSE, 0);


	linkbutton = gtk_link_button_new_with_label (
#if FINAL_RELEASE
						     "http://moonlight-project.com/",
#else
						     "http://moonlight-project.com/Beta",
#endif
						     "Project Web site");

	align = gtk_alignment_new (0.0, 0.0, 0.0, 1.0);
	gtk_container_add (GTK_CONTAINER (align), linkbutton);

	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (align), FALSE, FALSE, 0);

	gtk_table_attach (table, vbox, 1, 2, 0, 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  0, 0);

	button = gtk_button_new_with_label ("Bug Report Info...");

	g_signal_connect_swapped (button, "clicked", G_CALLBACK (bug_report_info), this);

	align = gtk_alignment_new (0.0, 1.0, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (align), button);

	gtk_table_attach (table, align, 2, 3, 2, 3,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  0, 0);
	
	return GTK_WIDGET (table);
}


////// Playback page
PlaybackConfigDialogPage::PlaybackConfigDialogPage()
{
}

PlaybackConfigDialogPage::~PlaybackConfigDialogPage ()
{
}

void
PlaybackConfigDialogPage::install_media_pack (PlaybackConfigDialogPage *page)
{
	Media::InstallMSCodecs (true);
}

GtkWidget*
PlaybackConfigDialogPage::GetContentWidget ()
{
	GtkWidget *vbox, *button, *label, *align;
	const char *button_str = "Install Microsoft Media Pack";
	gboolean sensitive = TRUE;

	vbox = gtk_vbox_new (FALSE, 2);

	if (Media::IsMSCodecsInstalled ()) {
#if DEBUG
		button_str = "Reinstall Microsoft Media Pack";
#else
		sensitive = FALSE;
#endif
	}

	label = gtk_label_new ("Moonlight requires the Microsoft Media Pack to play many types of content, such as VC-1 for video and MP3 for audio.");
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

	align = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);

	gtk_container_add (GTK_CONTAINER (align), label);

	gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);

	button = gtk_button_new_with_label (button_str);
	gtk_widget_set_sensitive (button, sensitive);

	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK (install_media_pack), this);

	align = gtk_alignment_new (0.5, 0.0, 0.0, 1.0);

	gtk_container_add (GTK_CONTAINER (align), button);

	gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);

	return vbox;
}


////// Storage page
enum StorageColumn {
	STORAGE_COLUMN_WEB_SITE,
	STORAGE_COLUMN_CURRENT_SIZE,
	STORAGE_COLUMN_QUOTA_SIZE,
	STORAGE_COLUMN_DIRECTORY
};

StorageConfigDialogPage::StorageConfigDialogPage()
{
}

StorageConfigDialogPage::~StorageConfigDialogPage ()
{
}

void
StorageConfigDialogPage::PageActivated ()
{
	PopulateModel ();
}

void
StorageConfigDialogPage::PopulateModel ()
{
	// first off clear whatever happened to be there before
	gtk_list_store_clear (GTK_LIST_STORE (model));


	// now populate the model
	GtkTreeIter iter;

	char *root = g_build_filename (g_get_user_data_dir (),
				       "moonlight", "is", NULL);

	GDir *dir = g_dir_open (root, 0, NULL);
	if (!dir) {
		g_free (root);
		return; // should never happen
	}

	const char *entry_name = g_dir_read_name (dir);
	while (entry_name) {
		char *entry_dir = g_build_filename (root, entry_name, NULL);
		char *config_filename = g_build_filename (entry_dir, "config", NULL);

		FILE *fp = fopen (config_filename, "r");

		if (fp) {
			// we have a valid config file, so we'll include this isostore
			long entry_size = isolated_storage_get_current_usage (entry_dir);
			gint64 quota_size = -1LL;
			char *uri = NULL;

			char file_line[256];

			while (fgets (file_line, sizeof (file_line), fp)) {
				if (!strncmp ("QUOTA = ", file_line, sizeof ("QUOTA = ") - 1))
					quota_size = strtoll (&file_line[sizeof ("QUOTA = ") - 1], NULL, 10);
				else if (!strncmp ("URI = ", file_line, sizeof ("URI = ") - 1)) {
					uri = g_strdup (file_line + sizeof("URI = ") - 1);
					/* get rid of the \n fgets leaves in */
					if (*uri && uri[strlen (uri) - 1] == '\n')
						uri[strlen (uri) - 1] = 0;
				}
			}

			fclose (fp);

			if (quota_size != -1LL && uri) {
				char current_size_buf[50];
				char quota_size_buf[50];

				snprintf (current_size_buf, sizeof (current_size_buf), "%.2f",
					  (double)entry_size / (1024 * 1024));

				snprintf (quota_size_buf, sizeof (quota_size_buf), "%.2f",
					  (double)quota_size / (1024 * 1024));

				gtk_list_store_append (model, &iter);
				gtk_list_store_set (model, &iter,
						    STORAGE_COLUMN_WEB_SITE, uri,
						    STORAGE_COLUMN_CURRENT_SIZE, current_size_buf,
						    STORAGE_COLUMN_QUOTA_SIZE, quota_size_buf,
						    STORAGE_COLUMN_DIRECTORY, entry_dir,
						    -1);
				
			}
		}


		g_free (entry_dir);
		g_free (config_filename);

		entry_name = g_dir_read_name (dir);
	}
	g_dir_close (dir);
	g_free (root);

}

GtkWidget*
StorageConfigDialogPage::GetContentWidget ()
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *scrolled;
	GtkWidget *vbox;
	GtkWidget *label;

	vbox = gtk_vbox_new (FALSE, 0);

	label = gtk_label_new ("The following Web sites are currently using application storage on your computer.");

	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 4);

	model = gtk_list_store_new (4,
				    /* STORAGE_COLUMN_WEB_SITE     */ G_TYPE_STRING,
				    /* STORAGE_COLUMN_CURRENT_SIZE */ G_TYPE_STRING,
				    /* STORAGE_COLUMN_QUOTA_SIZE   */ G_TYPE_STRING,
				    /* STORAGE_COLUMN_DIRECTORY    */ G_TYPE_STRING);

	treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (model)));

        selection = gtk_tree_view_get_selection (treeview);

	g_signal_connect_swapped (G_OBJECT (selection), "changed", G_CALLBACK (StorageConfigDialogPage::selection_changed), this);

	gtk_tree_view_set_headers_visible (treeview, TRUE);

	g_object_unref (model);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (column), "Web site");
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", STORAGE_COLUMN_WEB_SITE);
	gtk_tree_view_append_column (treeview, column);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (column), "Current (MB)");
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", STORAGE_COLUMN_CURRENT_SIZE);
	gtk_tree_view_append_column (treeview, column);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (column), "Quota (MB)");
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", STORAGE_COLUMN_QUOTA_SIZE);
	gtk_tree_view_append_column (treeview, column);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 4);


	GtkWidget *hbox = gtk_hbox_new (TRUE, 4);

	delete_button = gtk_button_new_with_label ("Delete...");
	gtk_widget_set_sensitive (delete_button, FALSE);
	g_signal_connect_swapped (G_OBJECT (delete_button), "clicked", G_CALLBACK (StorageConfigDialogPage::delete_selected_storage), this);

	gtk_box_pack_start (GTK_BOX(hbox), delete_button, FALSE, FALSE, 0);

	delete_all_button = gtk_button_new_with_label ("Delete all...");
	g_signal_connect_swapped (G_OBJECT (delete_all_button), "clicked", G_CALLBACK (StorageConfigDialogPage::delete_all_storage), this);

	gtk_box_pack_start (GTK_BOX(hbox), delete_all_button, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	return vbox;
}

void
StorageConfigDialogPage::selection_changed (StorageConfigDialogPage *page)
{
	gtk_widget_set_sensitive (page->delete_button, gtk_tree_selection_count_selected_rows (page->selection) > 0);
}

void
StorageConfigDialogPage::delete_selected_storage (StorageConfigDialogPage *page)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!gtk_tree_selection_get_selected (page->selection,
					      &model,
					      &iter))
		return;

	char *directory;

	gtk_tree_model_get (model,
			    &iter,
			    STORAGE_COLUMN_DIRECTORY, &directory,
			    -1);

	RemoveDir (directory);

	page->PopulateModel ();
}

void
StorageConfigDialogPage::delete_all_storage (StorageConfigDialogPage *page)
{
	char *root = g_build_filename (g_get_user_data_dir (),
				       "moonlight", "is", NULL);

	GDir *dir = g_dir_open (root, 0, NULL);
	if (!dir) {
		g_free (root);
		return;
	}

	const char *entry_name = g_dir_read_name (dir);
	while (entry_name) {
		char *entry_dir = g_build_filename (root, entry_name, NULL);

		RemoveDir (entry_dir);

		g_free (entry_dir);
		entry_name = g_dir_read_name (dir);
	}

	g_dir_close (dir);
	g_free (root);

	page->PopulateModel ();
}


////// Permissions page
enum PermissionsColumn {
	PERMISSIONS_COLUMN_NAME,
	PERMISSIONS_COLUMN_PERMISSION,

	// these columns aren't shown
	PERMISSIONS_COLUMN_ALLOWED,
	PERMISSIONS_COLUMN_CONSENT_TYPE
};

PermissionsConfigDialogPage::PermissionsConfigDialogPage()
{
	configuration = NULL;
	website_path_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
						   (GDestroyNotify)g_free,
						   (GDestroyNotify)gtk_tree_path_free);
}

PermissionsConfigDialogPage::~PermissionsConfigDialogPage ()
{
	delete configuration;
	g_hash_table_destroy (website_path_hash);
}

void
PermissionsConfigDialogPage::PageActivated ()
{
	PopulateModel ();
}

static gboolean
_true ()
{
	return TRUE;
}

void
PermissionsConfigDialogPage::PopulateModel ()
{
	// first off clear whatever happened to be there before
	gtk_tree_store_clear (GTK_TREE_STORE (model));

#if GLIB_CHECK_VERSION(2,12,0)
	if (glib_check_version (2,12,0))
		g_hash_table_remove_all (website_path_hash);
	else
#endif
	g_hash_table_foreach_remove (website_path_hash, (GHRFunc) _true, NULL);

	delete configuration;
	configuration = new MoonlightConfiguration ();

	gchar **permission_keys = configuration->GetKeys ("Permissions");

	if (!permission_keys)
		return;

	int i;
	for (i = 0; permission_keys && permission_keys[i] != NULL; i ++) {
		gchar *key = permission_keys[i];
		char *minus = strrchr(key, '-');
		char *website = g_strndup (key, minus - key);
		char *perm_name = minus + 1;
		MoonConsentType consent_type = Consent::GetConsentType (perm_name);

		bool value = configuration->GetBooleanValue ("Permissions", key);

		GtkTreePath *website_path;
		GtkTreeIter website_iter, perm_iter;

		website_path = (GtkTreePath*)g_hash_table_lookup (website_path_hash, website);
		if (!website_path) {
			gtk_tree_store_append (GTK_TREE_STORE (model),
					       &website_iter,
					       NULL);
			gtk_tree_store_set (GTK_TREE_STORE (model),
					    &website_iter,
					    PERMISSIONS_COLUMN_NAME, website,
					    PERMISSIONS_COLUMN_CONSENT_TYPE, -1,
					    -1);
			website_path = gtk_tree_model_get_path (GTK_TREE_MODEL (model),
								&website_iter);

			g_hash_table_insert (website_path_hash, website, website_path);
		}

		gtk_tree_store_append (GTK_TREE_STORE (model),
				       &perm_iter,
				       &website_iter);

		gtk_tree_store_set (GTK_TREE_STORE (model),
				    &perm_iter,
				    PERMISSIONS_COLUMN_NAME, Consent::GetConsentDescription (consent_type),
				    PERMISSIONS_COLUMN_PERMISSION, value ? "Allow" : "Deny",
				    PERMISSIONS_COLUMN_ALLOWED, value,
				    PERMISSIONS_COLUMN_CONSENT_TYPE, consent_type,
				    -1);
	}
}

GtkWidget*
PermissionsConfigDialogPage::GetContentWidget ()
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *scrolled;
	GtkWidget *vbox;
	GtkWidget *label;

	vbox = gtk_vbox_new (FALSE, 0);

	label = gtk_label_new ("Permissions");

	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 4);

	model = gtk_tree_store_new (4,
				    /* PERMISSIONS_COLUMN_NAME         */ G_TYPE_STRING,
				    /* PERMISSIONS_COLUMN_PERMISSION   */ G_TYPE_STRING,
				    /* PERMISSIONS_COLUMN_ALLOWED      */ G_TYPE_BOOLEAN,
				    /* PERMISSIONS_COLUMN_CONSENT_TYPE */ G_TYPE_INT);

	treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (model)));

        selection = gtk_tree_view_get_selection (treeview);

	g_signal_connect_swapped (G_OBJECT (selection), "changed", G_CALLBACK (PermissionsConfigDialogPage::selection_changed), this);

	gtk_tree_view_set_headers_visible (treeview, TRUE);

	g_object_unref (model);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (column), "Web site");
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", PERMISSIONS_COLUMN_NAME);
	gtk_tree_view_append_column (treeview, column);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (column), "Permission");
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", PERMISSIONS_COLUMN_PERMISSION);
	gtk_tree_view_append_column (treeview, column);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 4);


	GtkWidget *hbox = gtk_hbox_new (TRUE, 4);

	allow_button = gtk_button_new_with_label ("Allow");
	gtk_widget_set_sensitive (allow_button, FALSE);
	g_signal_connect_swapped (G_OBJECT (allow_button), "clicked", G_CALLBACK (PermissionsConfigDialogPage::allow_selected_permission), this);

	gtk_box_pack_start (GTK_BOX(hbox), allow_button, FALSE, FALSE, 0);

	deny_button = gtk_button_new_with_label ("Deny");
	gtk_widget_set_sensitive (allow_button, FALSE);
	g_signal_connect_swapped (G_OBJECT (deny_button), "clicked", G_CALLBACK (PermissionsConfigDialogPage::deny_selected_permission), this);

	gtk_box_pack_start (GTK_BOX(hbox), deny_button, FALSE, FALSE, 0);

	remove_button = gtk_button_new_with_label ("Remove");
	gtk_widget_set_sensitive (allow_button, FALSE);
	g_signal_connect_swapped (G_OBJECT (remove_button), "clicked", G_CALLBACK (PermissionsConfigDialogPage::remove_selected_permission), this);

	gtk_box_pack_start (GTK_BOX(hbox), remove_button, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	return vbox;
}

void
PermissionsConfigDialogPage::UpdateButtonSensitivity ()
{
	GtkTreeIter iter, parent_iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (selection,
					     &model,
					     &iter)
	    && gtk_tree_model_iter_parent (model,
					   &parent_iter,
					   &iter)) {

		gboolean allowed;
		gtk_tree_model_get (GTK_TREE_MODEL (model),
				    &iter,
				    PERMISSIONS_COLUMN_ALLOWED, &allowed,
				    -1);

		gtk_widget_set_sensitive (allow_button, !allowed);
		gtk_widget_set_sensitive (deny_button, allowed);
		gtk_widget_set_sensitive (remove_button, TRUE);
	}
	else {
		gtk_widget_set_sensitive (allow_button, FALSE);
		gtk_widget_set_sensitive (deny_button, FALSE);
		gtk_widget_set_sensitive (remove_button, FALSE);
	}
}

void
PermissionsConfigDialogPage::selection_changed (PermissionsConfigDialogPage *page)
{
	page->UpdateButtonSensitivity ();
}

void
PermissionsConfigDialogPage::allow_selected_permission (PermissionsConfigDialogPage *page)
{
	GtkTreeIter iter;
	GtkTreeIter parent_iter;
	GtkTreeModel *model;
	char *website;
	MoonConsentType consent_type;

	if (!gtk_tree_selection_get_selected (page->selection,
					      &model,
					      &iter))
		return;

	if (!gtk_tree_model_iter_parent (model,
					 &parent_iter,
					 &iter))
		return;

	gtk_tree_model_get (model,
			    &parent_iter,
			    PERMISSIONS_COLUMN_NAME, &website,
			    -1);

	gtk_tree_model_get (model,
			    &iter,
			    PERMISSIONS_COLUMN_CONSENT_TYPE, &consent_type,
			    -1);

	char *key = Consent::GeneratePermissionConfigurationKey (consent_type, website);

	page->configuration->SetBooleanValue ("Permissions", key, true);
	page->configuration->Save ();

	g_free (key);

	gtk_tree_store_set (GTK_TREE_STORE (model),
			    &iter,
			    PERMISSIONS_COLUMN_PERMISSION, "Allow",
			    PERMISSIONS_COLUMN_ALLOWED, TRUE,
			    -1);

	page->UpdateButtonSensitivity ();
}

void
PermissionsConfigDialogPage::deny_selected_permission (PermissionsConfigDialogPage *page)
{
	GtkTreeIter iter;
	GtkTreeIter parent_iter;
	GtkTreeModel *model;
	char *website;
	MoonConsentType consent_type;

	if (!gtk_tree_selection_get_selected (page->selection,
					      &model,
					      &iter))
		return;

	if (!gtk_tree_model_iter_parent (model,
					 &parent_iter,
					 &iter))
		return;

	gtk_tree_model_get (model,
			    &parent_iter,
			    PERMISSIONS_COLUMN_NAME, &website,
			    -1);

	gtk_tree_model_get (model,
			    &iter,
			    PERMISSIONS_COLUMN_CONSENT_TYPE, &consent_type,
			    -1);

	char *key = Consent::GeneratePermissionConfigurationKey (consent_type, website);

	page->configuration->SetBooleanValue ("Permissions", key, false);
	page->configuration->Save ();

	g_free (key);

	gtk_tree_store_set (GTK_TREE_STORE (model),
			    &iter,
			    PERMISSIONS_COLUMN_PERMISSION, "Deny",
			    PERMISSIONS_COLUMN_ALLOWED, FALSE,
			    -1);

	page->UpdateButtonSensitivity ();
}

void
PermissionsConfigDialogPage::remove_selected_permission (PermissionsConfigDialogPage *page)
{
	GtkTreeIter iter;
	GtkTreeIter parent_iter;
	GtkTreeModel *model;
	char *website;
	MoonConsentType consent_type;

	if (!gtk_tree_selection_get_selected (page->selection,
					      &model,
					      &iter))
		return;

	if (!gtk_tree_model_iter_parent (model,
					 &parent_iter,
					 &iter))
		return;

	gtk_tree_model_get (model,
			    &parent_iter,
			    PERMISSIONS_COLUMN_NAME, &website,
			    -1);

	gtk_tree_model_get (model,
			    &iter,
			    PERMISSIONS_COLUMN_CONSENT_TYPE, &consent_type,
			    -1);

	char *key = Consent::GeneratePermissionConfigurationKey (consent_type, website);

	page->configuration->RemoveKey ("Permissions", key);
	page->configuration->Save ();

	g_free (key);

	page->PopulateModel ();
}

////// Applications page
ApplicationConfigDialogPage::ApplicationConfigDialogPage()
{
}

ApplicationConfigDialogPage::~ApplicationConfigDialogPage ()
{
}

GtkWidget*
ApplicationConfigDialogPage::GetContentWidget ()
{
	return gtk_label_new ("hi there");
}

////// Debug page
DebugConfigDialogPage::DebugConfigDialogPage()
{
}

DebugConfigDialogPage::~DebugConfigDialogPage ()
{
}

GtkWidget*
DebugConfigDialogPage::GetContentWidget ()
{
	GtkBox *vbox, *hbox;
	GtkWidget *button;
	GtkWidget *align;

	align = gtk_alignment_new (0.5, 0.0, 0.0, 1.0);
	hbox = GTK_BOX (gtk_hbox_new (TRUE, 0));
	gtk_container_add (GTK_CONTAINER (align), GTK_WIDGET (hbox));

	vbox = GTK_BOX (gtk_vbox_new (TRUE, 4));

	gtk_box_pack_start (hbox, GTK_WIDGET (vbox), FALSE, FALSE, 0);

#if DEBUG
	button = gtk_button_new_with_label ("Show XAML Hierarchy");
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK (show_debug), GetDialog()->GetWindow());

	button = gtk_button_new_with_label ("Show Sources");
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK (show_sources), GetDialog()->GetWindow());

	button = gtk_button_new_with_label ("Debug info");
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK (debug_info), GetDialog()->GetWindow());
	
#if OBJECT_TRACKING
	button = gtk_button_new_with_label ("Media Debugging");
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK (debug_media), GetDialog()->GetWindow());
#endif
#endif

	return GTK_WIDGET (align);
}

////// Advanced page
AdvancedConfigDialogPage::AdvancedConfigDialogPage()
{
}

AdvancedConfigDialogPage::~AdvancedConfigDialogPage ()
{
}

#if 0
static void
table_add (GtkWidget *table, int row, const char *label, const char *value)
{
	GtkWidget *l = gtk_label_new (label);
	GtkWidget *v = gtk_label_new (value);

	gtk_label_set_selectable (GTK_LABEL (v), TRUE);

	gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE(table), l, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) 0, 4, 0);

	gtk_misc_set_alignment (GTK_MISC (v), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE(table), v, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) 0, 4, 0);
}
#endif

static GtkWidget *
title (const char *txt)
{
	char *fmt = g_strdup_printf ("<b>%s</b>", txt);
	GtkWidget *label = gtk_label_new (NULL);

	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), fmt);
	g_free (fmt);

	return label;
}


enum OptionColumn {
	OPTION_COLUMN_TOGGLE,
	OPTION_COLUMN_NAME,
	OPTION_COLUMN_FLAG,
	OPTION_COLUMN_SURFACE
};

static void
option_cell_toggled (GtkCellRendererToggle *cell_renderer,
		     gchar *path,
		     GtkTreeModel *model) 
{
	GtkTreeIter iter;
	GtkTreePath *tree_path;
	gboolean set;
	guint32 flag;
	Surface *surface;

	tree_path = gtk_tree_path_new_from_string (path);

	if (!gtk_tree_model_get_iter (model,
				      &iter,
				      tree_path)) {
		gtk_tree_path_free (tree_path);
		return;
	}

	gtk_tree_path_free (tree_path);

	gtk_tree_model_get (model,
			    &iter,
			    OPTION_COLUMN_TOGGLE, &set,
			    OPTION_COLUMN_FLAG, &flag,
			    OPTION_COLUMN_SURFACE, &surface,
			    -1);

	// we're toggling here
	set = !set;

	surface->SetCurrentDeployment ();

	// toggle the debug state for moonlight
	surface->SetRuntimeOption ((RuntimeInitFlag)flag, set);

	// and reflect the change in the UI
	gtk_list_store_set (GTK_LIST_STORE (model),
			    &iter,
			    OPTION_COLUMN_TOGGLE, set,
			    -1);
}

static GtkWidget*
create_option_treeview (Surface *surface)
{
	GtkListStore *model;
	GtkTreeIter iter;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeView *treeview;
	GtkWidget *scrolled;

	const MoonlightRuntimeOption *options;

	model = gtk_list_store_new (4,
				    /* OPTION_COLUMN_TOGGLE  */ G_TYPE_BOOLEAN,
				    /* OPTION_COLUMN_NAME    */ G_TYPE_STRING,
				    /* OPTION_COLUMN_FLAG    */ G_TYPE_INT,
				    /* OPTION_COLUMN_SURFACE */ G_TYPE_POINTER);

	options = moonlight_get_runtime_options ();
	
	for (int i = 0; options [i].name != NULL; i++) {
		if (!options[i].runtime_changeable)
			continue;
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
				    OPTION_COLUMN_TOGGLE, surface->GetRuntimeOption (options[i].flag),
				    OPTION_COLUMN_NAME, options [i].description,
				    OPTION_COLUMN_FLAG, options [i].flag,
				    OPTION_COLUMN_SURFACE, surface,
				    -1);
	}

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	
	treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (model)));

	gtk_tree_view_set_headers_visible (treeview, FALSE);

	g_object_unref (model);

	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_toggle_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "active", 0);
	gtk_signal_connect (GTK_OBJECT(renderer), "toggled", G_CALLBACK (option_cell_toggled), model);
	gtk_tree_view_append_column (treeview, column);

	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", 1);
	gtk_tree_view_append_column (treeview, column);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (treeview));

	return scrolled;
}

GtkWidget*
AdvancedConfigDialogPage::GetContentWidget ()
{
	GtkWidget *treeview;
	GtkBox *vbox;

	Surface *surface = GetDialog()->GetSurface();

	vbox = GTK_BOX (gtk_vbox_new (FALSE, 0));

	// Runtime debug options
	gtk_box_pack_start (vbox, title ("Runtime Debug Options"), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, gtk_hseparator_new (), FALSE, FALSE, 8);

	treeview = create_option_treeview (surface);

	gtk_box_pack_start (vbox, treeview, TRUE, TRUE, 0);

	return GTK_WIDGET (vbox);
}

