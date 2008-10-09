/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-debug.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtkmessagedialog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "plugin-debug.h"
#include "utils.h"
#include "uri.h"


enum TreeColumns {
	COL_NAME,
	COL_TYPE_NAME,
	COL_VALUE,
	COL_ELEMENT_PTR,
	NUM_COLUMNS
};

#ifdef DEBUG

static void reflect_dependency_object_in_tree (DependencyObject *obj, GtkTreeStore *store,
					       GtkTreeIter *node, bool node_is_self);

struct ReflectForeachData {
	GtkTreeStore *store;
	GtkTreeIter *parent;
	DependencyObject *obj;
};

char*
timespan_to_str (TimeSpan ts)
{
	bool negative;
	int days;
	int hours;
	int minutes;
	double seconds;

	double ts_frac = (double)ts;

	negative = (ts < 0);

	ts_frac /= 10000000.0;

	days = (int)(ts_frac / 86400);
	ts_frac -= days * 86400;
	hours = (int)(ts_frac / 3600);
	ts_frac -= hours * 3600;
	minutes = (int)(ts_frac / 60);
	ts_frac -= minutes * 60;
	seconds = ts_frac;

	// XXX someone who can remember printf specifiers should
	// remove that %s from in there in such a way that we get a
	// float zero padded to 2 spaces to the left of the decimal.
	return g_strdup_printf ("%02d:%02d:%02d:%s%.4f", days, hours, minutes, seconds < 10.0 ? "0" : "", seconds);
}

static void
reflect_value (GtkTreeStore *store, GtkTreeIter *node, const char *name, Value *value)
{
	if (value->Is(Type::DEPENDENCY_OBJECT)) {
		gtk_tree_store_set (store, node,
				    COL_NAME, name,
				    COL_TYPE_NAME, value->AsDependencyObject()->GetTypeName(),
				    COL_VALUE, "",
				    COL_ELEMENT_PTR, NULL,
				    -1);

		reflect_dependency_object_in_tree (value->AsDependencyObject(), store, node, true);
	}
	else {
		Type* t = Type::Find (value->GetKind());

		char *val_string = NULL;

		switch (value->GetKind()) {
		case Type::DOUBLE:
			val_string = g_strdup_printf ("<b>%g</b>", value->AsDouble());
			break;
		case Type::INT32:
			val_string = g_strdup_printf ("<b>%d</b>", value->AsInt32());
			break;
		case Type::INT64:
			val_string = g_strdup_printf ("<b>%lld</b>", value->AsInt64());
			break;
		case Type::TIMESPAN: {
			char *ts_string = timespan_to_str (value->AsTimeSpan());
			val_string = g_strdup_printf ("<b>%s</b>", ts_string);
			g_free (ts_string);
			break;
		}
		case Type::UINT64:
			val_string = g_strdup_printf ("<b>%llu</b>", value->AsUint64());
			break;
		case Type::STRING:
			val_string = g_strdup_printf ("<b>%s</b>", value->AsString());
			break;
		case Type::RECT: {
			Rect *rect = value->AsRect();
			val_string = g_strdup_printf ("<b>%g, %g, %g, %g</b>", rect->x, rect->y, rect->width, rect->height);
			break;
		}
		case Type::SIZE:
			val_string = g_strdup_printf ("<b>%g, %g</b>", value->AsSize()->width, value->AsSize()->height);
			break;
		case Type::REPEATBEHAVIOR: {
			RepeatBehavior *rb = value->AsRepeatBehavior();
			if (rb->IsForever ())
				val_string = g_strdup_printf ("<b>Forever</b>");
			else if (rb->HasCount())
				val_string = g_strdup_printf ("<b>%gx</b>", rb->GetCount());
			else /*if (rb->HasDuration())*/ {
				char *ts_string = timespan_to_str (rb->GetDuration());
				val_string = g_strdup_printf ("<b>%s</b>", ts_string);
				g_free (ts_string);
			}
			break;
		}
		case Type::DURATION: {
			Duration *d = value->AsDuration();
			if (d->IsForever ())
				val_string = g_strdup_printf ("<b>Forever</b>");
			else if (d->IsAutomatic())
				val_string = g_strdup_printf ("<b>Automatic</b>");
			else /*if (d->HasTimeSpan())*/ {
				char *ts_string = timespan_to_str (d->GetTimeSpan());
				val_string = g_strdup_printf ("<b>%s</b>", ts_string);
				g_free (ts_string);
			}
			break;
		}
		case Type::COLOR: {
			Color *color = value->AsColor();
			val_string = g_strdup_printf ("<b>r=%g, g=%g, b=%g, a=%g</b>", color->r, color->g, color->b, color->a);
			break;
		}
		case Type::KEYTIME:
		case Type::GRIDLENGTH:
		case Type::THICKNESS:
		case Type::CORNERRADIUS:
		default:
			val_string = g_strdup ("<i>(unknown)</i>");
			break;
		}

		gtk_tree_store_set (store, node,
				    COL_NAME, name,
				    COL_TYPE_NAME, t->GetName (),
				    COL_VALUE, val_string,
				    COL_ELEMENT_PTR, NULL,
				    -1);

		g_free (val_string);
	}
}

static void
reflect_foreach_current_value (gpointer key, gpointer val, gpointer user_data)
{
	ReflectForeachData *data = (ReflectForeachData *)user_data;
	DependencyProperty *prop = (DependencyProperty *)key;
	Value *value = (Value*)val;

	GtkTreeIter iter;

	gtk_tree_store_append (data->store, &iter, data->parent);

	char *markup = g_strdup_printf ("<i>%s.%s</i>", Type::Find(prop->GetOwnerType())->GetName(), prop->GetName());

	reflect_value (data->store, &iter, markup, value);

	g_free (markup);
}

static void
reflect_dependency_object_in_tree (DependencyObject *obj, GtkTreeStore *store, GtkTreeIter *node, bool node_is_self)
{
	if (obj == NULL)
		return;

	GtkTreeIter iter;

	if (!node_is_self) {
		gtk_tree_store_append (store, &iter, node);

		char *markup = g_strdup_printf ("<b>%s</b>", obj->GetName() ? obj->GetName() : "");
		gtk_tree_store_set (store, &iter,
				    COL_NAME, markup,
				    COL_TYPE_NAME, obj->GetTypeName(),
				    COL_ELEMENT_PTR, obj,
				    -1);
		g_free (markup);

		node = &iter;
	}

	GHashTable *ht = obj->GetCurrentValues();

	if (g_hash_table_size (ht) > 0) {
		GtkTreeIter prop_iter;

		gtk_tree_store_append (store, &prop_iter, node);

		gtk_tree_store_set (store, &prop_iter,
				    COL_NAME, "Properties",
				    COL_TYPE_NAME, "",
				    COL_ELEMENT_PTR, obj,
				    -1);

		ReflectForeachData reflect_data;
		reflect_data.store = store;
		reflect_data.parent = &prop_iter;
		reflect_data.obj = obj;

		g_hash_table_foreach (ht, reflect_foreach_current_value, &reflect_data);
	}

	if (obj->Is(Type::COLLECTION)) {
		Collection *col = (Collection*)obj;

		if (col->GetCount() > 0) {
			GtkTreeIter elements_iter;
		
			gtk_tree_store_append (store, &elements_iter, node);

			gtk_tree_store_set (store, &elements_iter,
					    COL_NAME, "Elements",
					    COL_TYPE_NAME, "",
					    COL_ELEMENT_PTR, obj,
					    -1);

			for (int i = 0; i < col->GetCount(); i ++) {
				Value *v = col->GetValueAt (i);
				char *markup;

				if (v->Is (Type::DEPENDENCY_OBJECT))
					markup = g_strdup_printf ("<i>[%d]</i> <b>%s</b>", i, v->AsDependencyObject()->GetName() ? v->AsDependencyObject()->GetName() : "");
				else
					markup = g_strdup_printf ("<i>[%d]</i>", i);

				GtkTreeIter child_iter;

				gtk_tree_store_append (store, &child_iter, &elements_iter);

				reflect_value (store, &child_iter, markup, v);

				g_free (markup);
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

	if (plugin->GetSurface()->debug_selected_element) {
		UIElement *el = plugin->GetSurface()->debug_selected_element;
		el->Invalidate (el->GetSubtreeBounds().GrowBy(1).RoundOut());
		el->unref ();
		plugin->GetSurface()->debug_selected_element = NULL;
	}

	if (!gtk_tree_selection_get_selected (selection, 
					      &model,
					      &iter)) {
		return;
	}

	gtk_tree_model_get (model, &iter,
			    COL_ELEMENT_PTR, &el,
			    -1);

	if (el) {
		el->Invalidate (el->GetSubtreeBounds().GrowBy(1).RoundOut());
		el->ref ();
		plugin->GetSurface()->debug_selected_element = el;
	}
}

void
plugin_debug (PluginInstance *plugin)
{
	if (!plugin->GetSurface()) {
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

	GtkTreeStore *tree_store = gtk_tree_store_new (NUM_COLUMNS,
						       G_TYPE_STRING,
						       G_TYPE_STRING,
						       G_TYPE_STRING,
						       G_TYPE_POINTER);

	reflect_dependency_object_in_tree (plugin->GetSurface()->GetToplevel (), tree_store, NULL, false);

#if false
 	GtkTreeModel *sorted_model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (tree_store));

 	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (sorted_model),
 					      COL_NAME, GTK_SORT_ASCENDING);

	GtkWidget* tree_view = gtk_tree_view_new_with_model (sorted_model);
#else
	GtkWidget* tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store));
#endif

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
	gtk_tree_view_column_add_attribute (col, renderer, "markup", COL_NAME);
	gtk_tree_view_column_set_resizable (col, TRUE);

	gtk_tree_view_column_set_sort_column_id (col, COL_NAME);

	/* The Type column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Type");
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "markup", COL_TYPE_NAME);
	gtk_tree_view_column_set_resizable (col, TRUE);

	/* The Value column */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Value");
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col);

	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "markup", COL_VALUE);
	gtk_tree_view_column_set_resizable (col, TRUE);

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
	List *sources;
	PluginInstance::moon_source *src;
	
	sources = plugin->GetSources ();
	
	if (sources == NULL)
		return;
	
	src = (PluginInstance::moon_source*) sources->First ();
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


static size_t
get_common_prefix_len (GtkTreeModel *model)
{
	char *filename, *path, *url, *buf, *p, *q;
	size_t max = (size_t) -1;
	GtkTreeIter iter;
	Uri *uri;
	
	if (!gtk_tree_model_get_iter_first (model, &iter))
		return 0;
	
	gtk_tree_model_get (model, &iter, 0, &url, 1, &filename, -1);
	
	uri = new Uri ();
	if (!uri->Parse (url)) {
		buf = g_strdup (filename);
	} else {
		buf = uri->path;
		uri->path = NULL;
	}
	
	if ((p = strrchr (buf, '/')))
		max = (p - buf);
	else
		max = 0;
	
	delete uri;
	
	while (gtk_tree_model_iter_next (model, &iter)) {
		gtk_tree_model_get (model, &iter, 0, &url, 1, &filename, -1);
		
		uri = new Uri ();
		if (!uri->Parse (url))
			path = filename;
		else
			path = uri->path;
		
		for (p = buf, q = path; *p && *q; p++, q++) {
			if (*p != *q)
				break;
		}
		
		if ((size_t) (p - buf) < max)
			max = p - buf;
		
		delete uri;
	}
	
	g_free (buf);
	
	return max;
}

static void
save_callback (GtkWidget *widget, gpointer data)
{
	GtkTreeModel *model = (GtkTreeModel *) data;
	char *filename, *dirname, *url, *path;
	GtkTreeIter iter;
	size_t prelen;
	Uri *uri;
	int fd;
	
	if (mkdir ("/tmp/moon-dump", 0777) == -1 && errno != EEXIST)
		return;
	
	prelen = get_common_prefix_len (model);
	
	if (!gtk_tree_model_get_iter_first (model, &iter))
		return;
	
	do {
		gtk_tree_model_get (model, &iter, 0, &url, 1, &filename, -1);
		
		uri = new Uri ();
		if (uri->Parse (url))
			path = uri->path;
		else
			path = filename;
		
		path = g_build_filename ("/tmp/moon-dump", path + prelen, NULL);
		delete uri;
		
		dirname = g_path_get_dirname (path);
		g_mkdir_with_parents (dirname, 0777);
		g_free (dirname);
		
		if ((fd = open (path, O_CREAT | O_WRONLY | O_EXCL, 0644)) != -1) {
			if (CopyFileTo (filename, fd) == -1)
				printf (" Failed: Could not copy file `%s' to `%s': %s\n", filename, path, g_strerror (errno));
		} else if (errno != EEXIST) {
			printf (" Failed: Could not create file `%s': %s\n", path, g_strerror (errno));
		}
		
		g_free (path);
	} while (gtk_tree_model_iter_next (model, &iter));
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

	GtkWidget *tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store));

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
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (save_callback), tree_store);
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	
	gtk_container_add (GTK_CONTAINER (tree_win), GTK_WIDGET (vbox));

	gtk_widget_show_all (tree_win);
}

#endif
