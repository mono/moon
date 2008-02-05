
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
	  plugin->surface->debug_selected_element->Invalidate ();
		plugin->surface->debug_selected_element->unref ();
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
		el->Invalidate ();
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

	gtk_container_add (GTK_CONTAINER (scrolled), tree_view);
	gtk_container_add (GTK_CONTAINER (tree_win), scrolled);

	gtk_widget_show_all (tree_win);
}

#endif
