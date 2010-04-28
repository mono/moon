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

#include "authors.h"
#include "window-gtk.h"
#include "openfile.h"
#include "pipeline.h"
#include "pipeline-ui.h"
#include "timemanager.h"
#include "debug-ui.h"

#define PLUGIN_OURNAME      "Novell Moonlight"


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
show_about ()
{
	GtkAboutDialog *about = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());

	gtk_about_dialog_set_name (about, PLUGIN_OURNAME);
	gtk_about_dialog_set_version (about, VERSION);

	gtk_about_dialog_set_copyright (about, "Copyright 2007-2010 Novell, Inc. (http://www.novell.com/)");
#if FINAL_RELEASE
	gtk_about_dialog_set_website (about, "http://moonlight-project.com/");
#else
	gtk_about_dialog_set_website (about, "http://moonlight-project.com/Beta");
#endif

	gtk_about_dialog_set_website_label (about, "Project Website");

	gtk_about_dialog_set_authors (about, moonlight_authors);

	/* Newer gtk+ versions require this for the close button to work */
	g_signal_connect_swapped (about,
				  "response",
				  G_CALLBACK (gtk_widget_destroy),
				  about);

	gtk_dialog_run (GTK_DIALOG (about));
}


GtkWidget*
AboutConfigDialogPage::GetContentWidget ()
{
	GtkWidget *button = gtk_button_new_with_label ("About Moonlight...");

	g_signal_connect (button, "clicked", G_CALLBACK (show_about), NULL);

	return button;
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
	CodecDownloader::ShowUI (page->GetDialog()->GetSurface(), true);
}

GtkWidget*
PlaybackConfigDialogPage::GetContentWidget ()
{
	GtkWidget *vbox, *button;
	const char *label = "Install Microsoft Media Pack";
	gboolean sensitive = TRUE;

	vbox = gtk_vbox_new (FALSE, 2);

	if (Media::IsMSCodecsInstalled ()) {
#if DEBUG
		label = "Reinstall Microsoft Media Pack";
#else
		sensitive = FALSE;
#endif
	}

	button = gtk_button_new_with_label (label);
	gtk_widget_set_sensitive (button, sensitive);

	g_signal_connect_swapped (G_OBJECT(button), "clicked", G_CALLBACK (install_media_pack), this);

	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

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
	GtkBox *vbox;
	GtkWidget *button;

	vbox = GTK_BOX (gtk_vbox_new (TRUE, 4));

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

	return GTK_WIDGET (vbox);
}

////// Advanced page
AdvancedConfigDialogPage::AdvancedConfigDialogPage()
{
}

AdvancedConfigDialogPage::~AdvancedConfigDialogPage ()
{
}

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
	GtkWidget *table, *treeview;
	char buffer[60];
	GtkBox *vbox;
	int row = 0;

	Surface *surface = GetDialog()->GetSurface();
	Deployment *deployment = GetDialog()->GetDeployment();
	MoonWindowGtk *window = GetDialog()->GetWindow();

	vbox = GTK_BOX (gtk_vbox_new (FALSE, 0));
	
	// Silverlight Application properties
	gtk_box_pack_start (vbox, title ("Properties"), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, gtk_hseparator_new (), FALSE, FALSE, 8);
	
	table = gtk_table_new (11, 2, FALSE);
	gtk_box_pack_start (vbox, table, FALSE, FALSE, 0);

	table_add (table, row++, "Source", deployment->GetXapLocation());

	snprintf (buffer, sizeof (buffer), "%dpx", window->GetWidth ());
	table_add (table, row++, "Width:", buffer);

	snprintf (buffer, sizeof (buffer), "%dpx", window->GetHeight ());
	table_add (table, row++, "Height:", buffer);

	const char *color_str = color_to_string (surface->GetBackgroundColor ());
	table_add (table, row++, "Background:", color_str);

	table_add (table, row++, "RuntimeVersion:", deployment->GetRuntimeVersion() ? deployment->GetRuntimeVersion() : "(Unknown)");
	table_add (table, row++, "Windowless:", window->GetWidget() == NULL ? "yes" : "no");

	snprintf (buffer, sizeof (buffer), "%i", surface->GetTimeManager()->GetMaximumRefreshRate());
	table_add (table, row++, "MaxFrameRate:", buffer);

	table_add (table, row++, "Codecs:",
		   Media::IsMSCodecsInstalled () ? "ms-codecs" :
#if INCLUDE_FFMPEG
		   "ffmpeg"
#else
		   "none"
#endif
		   );

	int size = 0;
#if DEBUG
	size = snprintf (buffer, sizeof (buffer), "debug");
#else
	size = snprintf (buffer, sizeof (buffer), "release");
#endif
#if SANITY
	size += snprintf (buffer + size, sizeof (buffer) - size, ", sanity checks");
#endif
#if OBJECT_TRACKING
	size += snprintf (buffer + size, sizeof (buffer) - size, ", object tracking");
#endif
	table_add (table, row++, "Build configuration:", buffer);

	// Runtime debug options
	gtk_box_pack_start (vbox, title ("Runtime Debug Options"), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, gtk_hseparator_new (), FALSE, FALSE, 8);

	treeview = create_option_treeview (surface);

	gtk_box_pack_start (vbox, treeview, TRUE, TRUE, 0);

	return GTK_WIDGET (vbox);
}

