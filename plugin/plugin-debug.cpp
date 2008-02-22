
#include "plugin-debug.h"
#include <gtk/gtkmessagedialog.h>

#ifdef DEBUG

static void
populate_tree_from_xaml (UIElement *el, GtkTreeStore *store, GtkTreeIter *parent)
{
	if (el == NULL)
		return;

	GtkTreeIter iter;

	gtk_tree_store_append (store, &iter, parent);

	gtk_tree_store_set (store, &iter,
			    0, el->GetName() ? el->GetName() : "",
			    1, el->GetTypeName(),
			    2, el,
			    -1);

	if (el->Is(Type::PANEL)) {
		VisualCollection *children = ((Panel*)el)->GetChildren ();
		if (children != NULL) {
			Collection::Node *cn;
			cn = (Collection::Node *) children->list->First ();
			for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
				UIElement *item = (UIElement *) cn->obj;
				populate_tree_from_xaml (item, store, &iter);
			}
		}
	}
}

static void
selection_changed (GtkTreeSelection *selection, PluginInstance *plugin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	UIElement *el;

	if (plugin->surface->debug_selected_element) {
		UIElement *el = plugin->surface->debug_selected_element;
		el->Invalidate (el->GetSubtreeBounds().GrowBy(1).RoundOut());
		el->unref ();
		plugin->surface->debug_selected_element = NULL;
	}

	if (!gtk_tree_selection_get_selected (selection, 
					      &model,
					      &iter)) {
		return;
	}

	gtk_tree_model_get (model, &iter,
			    2, &el,
			    -1);

	if (el) {
		printf ("%p selected, name = %s, type = %s\n", el, el->GetName(), el->GetTypeName());
		el->Invalidate (el->GetSubtreeBounds().GrowBy(1).RoundOut());
		el->ref ();
		plugin->surface->debug_selected_element = el;
	}
}

void
plugin_debug (PluginInstance *plugin)
{
	if (!plugin->getIsLoaded() || !plugin->surface) {
		GtkWidget *d = gtk_message_dialog_new (NULL,
						       GTK_DIALOG_NO_SEPARATOR,
						       GTK_MESSAGE_ERROR,
						       GTK_BUTTONS_CLOSE,
						       "The plugin hasn't been initialized with xaml content yet");
		gtk_dialog_run (GTK_DIALOG (d));
		g_object_unref (d);
		return;
	}

	GtkWidget *tree_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (tree_win), "Xaml contents");
	gtk_window_set_default_size (GTK_WINDOW (tree_win), 300, 400);

	GtkTreeStore *tree_store = gtk_tree_store_new (3,
						       G_TYPE_STRING,
						       G_TYPE_STRING,
						       G_TYPE_POINTER);

	populate_tree_from_xaml (plugin->surface->GetToplevel (), tree_store, NULL);

	GtkWidget* tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store));

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	g_signal_connect (G_OBJECT (selection), "changed", 
			  G_CALLBACK (selection_changed), plugin);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col;

	/* The Name column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Name");
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", 0);

	/* The Type column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Type");
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", 1);

	GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (scrolled), tree_view);
	gtk_container_add (GTK_CONTAINER (tree_win), scrolled);

	gtk_widget_show_all (tree_win);
}

static void
populate_tree_from_surface (PluginInstance *plugin, GtkTreeStore *store, GtkTreeIter *parent)
{
	if (plugin == NULL)
		return;

	GtkTreeIter iter;
	PluginInstance::moon_source *src = (PluginInstance::moon_source*) plugin->GetSources ()->First ();
	for (; src != NULL; src = (PluginInstance::moon_source*) src->next) {
		gtk_tree_store_append (store, &iter, parent);

		gtk_tree_store_set (store, &iter,
				    0, src->uri,
				    1, src->filename,
				    2, src,
				    -1);

	}
}

PluginInstance::moon_source *selected_source = NULL;

static void
selection_changed_sources (GtkTreeSelection *selection, PluginInstance *plugin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	selected_source = NULL;
	
	if (!gtk_tree_selection_get_selected (selection, 
					      &model,
					      &iter)) {
		return;
	}

	gtk_tree_model_get (model, &iter,
			    2, &selected_source,
			    -1);

}

static void clicked_callback (GtkWidget *widget, gpointer data)
{
	if (selected_source == NULL) {
		printf ("Select a source first.\n");
	} else {
		gchar* argv [3];
		argv [0] = (gchar*) "xdg-open";
		argv [1] = (gchar*) selected_source->filename;
		argv [2] = NULL;
		g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL); 
	}
}

static void save_callback (GtkWidget *widget, gpointer data)
{
	PluginInstance *plugin = (PluginInstance*) data;
	
	PluginInstance::moon_source *src = (PluginInstance::moon_source*) plugin->GetSources ()->First ();
	const gchar *dir = "/tmp/moon-dump";
	mkdir (dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	for (; src != NULL; src = (PluginInstance::moon_source*) src->next) {
		gchar *name = g_path_get_basename (src->uri);
		gchar *path = g_build_filename (dir, name, NULL);
		gchar *contents;
		gsize length;
		GError *err1 = NULL, *err2 = NULL;
		
		printf ("Copying uri '%s' with local filename '%s' to '%s'...\n", src->uri, src->filename, path);
		
		if (!g_file_get_contents (src->filename, &contents, &length, &err1)) {
			printf (" Failed: Could not read file: %s\n", err1->message);
		} else if (!g_file_set_contents (path, contents, length, &err2)) {
			printf (" Failed: Could not write file: %s\n", err2->message);
		} else {
			printf (" Success\n");
		}
		
		g_free (name);
		g_free (path);
	}
}

void
plugin_sources (PluginInstance *plugin)
{	
	GtkWidget *tree_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (tree_win), "Sources");
	gtk_window_set_default_size (GTK_WINDOW (tree_win), 600, 400);
	GtkBox *vbox = GTK_BOX (gtk_vbox_new (false, 0));

	GtkTreeStore *tree_store = gtk_tree_store_new (3,
						       G_TYPE_STRING,
						       G_TYPE_STRING,
						       G_TYPE_POINTER);

	populate_tree_from_surface (plugin, tree_store, NULL);

	GtkWidget* tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store));

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	g_signal_connect (G_OBJECT (selection), "changed", 
			  G_CALLBACK (selection_changed_sources), plugin);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col;

	/* The Name column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Uri");
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", 0);

	/* The Type column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Filename");
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", 1);

	GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (scrolled), tree_view);
	//gtk_container_add (GTK_CONTAINER (tree_win), scrolled);
	gtk_box_pack_start (vbox, scrolled, TRUE, TRUE, 0);

	GtkWidget *button;
	button = gtk_button_new_with_label ("Open file");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (clicked_callback), NULL);
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	
	button = gtk_button_new_with_label ("Save (to /tmp/moon-dump/)");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (save_callback), plugin);
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	
	gtk_container_add (GTK_CONTAINER (tree_win), GTK_WIDGET (vbox));

	gtk_widget_show_all (tree_win);
}

#endif
