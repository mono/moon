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

#include <config.h>

#include <gtk/gtkmessagedialog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "plugin-debug.h"
#include "utils.h"
#include "uri.h"
#include "grid.h"

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

struct AddNamescopeItemData {
	GtkTreeStore *store;
	GtkTreeIter *node;

	AddNamescopeItemData (GtkTreeStore *store, GtkTreeIter *node)
	{
		this->store = store;
		this->node = node;
	}
};

static void
add_namescope_item (gpointer key, gpointer value, gpointer user_data);

static char *
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
reflect_value (GtkTreeStore *store, GtkTreeIter *node, const char *name, const char *type_name, Value *value)
{
	DependencyObject *dobj;
	const char *str = NULL;
	char *buf = NULL;
	Deployment *deployment = Deployment::GetCurrent ();

	if (value && value->Is (deployment, Type::DEPENDENCY_OBJECT)) {
		dobj = value->AsDependencyObject ();
		
		gtk_tree_store_set (store, node,
				    COL_NAME, name,
				    COL_TYPE_NAME, dobj ? dobj->GetTypeName () : "null",
				    COL_VALUE, "",
				    COL_ELEMENT_PTR, NULL,
				    -1);
		
		if (dobj)
			reflect_dependency_object_in_tree (dobj, store, node, true);
		return;
	}

	if (value != NULL) {
		Type *type = Type::Find (deployment, value->GetKind ());
		type_name = type->GetName ();
		
		switch (value->GetKind()) {
		case Type::DOUBLE:
			str = buf = g_strdup_printf ("<b>%g</b>", value->AsDouble ());
			break;
		case Type::INT32:
			str = buf = g_strdup_printf ("<b>%d</b>", value->AsInt32 ());
			break;
		case Type::INT64:
			str = buf = g_strdup_printf ("<b>%lld</b>", (long long int) value->AsInt64 ());
			break;
		case Type::TIMESPAN: {
			char *ts_string = timespan_to_str (value->AsTimeSpan());
			str = buf = g_strdup_printf ("<b>%s</b>", ts_string);
			g_free (ts_string);
			break;
		}
		case Type::UINT64:
			str = buf = g_strdup_printf ("<b>%llu</b>", (unsigned long long int) value->AsUInt64 ());
			break;
		case Type::STRING:
			str = buf = g_strdup_printf ("<b>%s</b>", value->AsString ());
			break;
		case Type::RECT: {
			Rect *rect = value->AsRect();
			str = buf = g_strdup_printf ("<b>%g, %g, %g, %g</b>", rect->x, rect->y, rect->width, rect->height);
			break;
		}
		case Type::SIZE:
			str = buf = g_strdup_printf ("<b>%g, %g</b>", value->AsSize()->width, value->AsSize()->height);
			break;
		case Type::REPEATBEHAVIOR: {
			RepeatBehavior *rb = value->AsRepeatBehavior();
			if (rb->IsForever ())
				str = "<b>Forever</b>";
			else if (rb->HasCount())
				str = buf = g_strdup_printf ("<b>%gx</b>", rb->GetCount());
			else /*if (rb->HasDuration())*/ {
				char *ts_string = timespan_to_str (rb->GetDuration());
				str = buf = g_strdup_printf ("<b>%s</b>", ts_string);
				g_free (ts_string);
			}
			break;
		}
		case Type::DURATION: {
			Duration *d = value->AsDuration();
			if (d->IsForever ())
				str = "<b>Forever</b>";
			else if (d->IsAutomatic())
				str = "<b>Automatic</b>";
			else /*if (d->HasTimeSpan())*/ {
				char *ts_string = timespan_to_str (d->GetTimeSpan());
				str = buf = g_strdup_printf ("<b>%s</b>", ts_string);
				g_free (ts_string);
			}
			break;
		}
		case Type::COLOR: {
			Color *color = value->AsColor();
			str = buf = g_strdup_printf ("<b>r=%g, g=%g, b=%g, a=%g</b>", color->r, color->g, color->b, color->a);
			break;
		}
		case Type::BOOL:
			str = value->AsBool () ? "<b>true</b>" : "<b>false</b>";
			break;
		case Type::GRIDLENGTH: {
			GridLength *length = value->AsGridLength ();
			str = buf = g_strdup_printf ("<b>%g (%s)</b>", length->val, length->type == GridUnitTypeAuto ?
						     "Auto" : length->type == GridUnitTypeStar ? "*" : "Pixel");
			break;
		}
		case Type::THICKNESS: {
			Thickness *thickness = value->AsThickness ();
			str = buf = g_strdup_printf ("<b>%g, %g, %g, %g</b>", 
						     thickness->left, 
						     thickness->top, 
						     thickness->right, 
						     thickness->bottom);
			break;
		}
		case Type::POINT: {
			Point *point = value->AsPoint ();
			str = buf = g_strdup_printf ("<b>(%g, %g)</b>", point->x, point->y);
			break;
		}
		case Type::CORNERRADIUS: {
			CornerRadius *CornerRadius = value->AsCornerRadius ();
			str = buf = g_strdup_printf ("<b>%g, %g, %g, %g</b>", 
						     CornerRadius->topLeft, 
						     CornerRadius->topRight, 
						     CornerRadius->bottomLeft,
						     CornerRadius->bottomRight);
			break;
		}
		case Type::KEYTIME:
		default:
			str = "<i>(unknown)</i>";
			break;
		}
	} else {
		str = "<b><i>null</i></b>";
	}
	
	gtk_tree_store_set (store, node,
			    COL_NAME, name,
			    COL_TYPE_NAME, type_name,
			    COL_VALUE, str,
			    COL_ELEMENT_PTR, NULL,
			    -1);
	
	g_free (buf);
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
	
	DependencyProperty **properties = obj->GetProperties (true);
	
	if (properties[0] != NULL) {
		GtkTreeIter prop_iter, iter;
		Type *owner_type;
		Type *prop_type;
		char *markup;
		Value *value;
		
		gtk_tree_store_append (store, &prop_iter, node);

		gtk_tree_store_set (store, &prop_iter,
				    COL_NAME, "Properties",
				    COL_TYPE_NAME, "",
				    COL_ELEMENT_PTR, obj,
				    -1);
		
		for (int i = 0; properties[i]; i++) {
			owner_type = Type::Find (obj->GetDeployment (), properties[i]->GetOwnerType ());
			markup = g_strdup_printf ("<i>%s.%s</i>", owner_type ? owner_type->GetName () : "(unknown)",
						  properties[i]->GetName ());
			
			gtk_tree_store_append (store, &iter, &prop_iter);
			
			prop_type = Type::Find (obj->GetDeployment (), properties[i]->GetPropertyType ());
			value = obj->GetValue (properties[i]);
			
			reflect_value (store, &iter, markup, prop_type ? prop_type->GetName () : "(unknown)", value);
			
			g_free (markup);
		}
	}
	
	g_free (properties);
	
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

				if (v->Is (col->GetDeployment (), Type::DEPENDENCY_OBJECT))
					markup = g_strdup_printf ("<i>[%d]</i> <b>%s</b>", i, v->AsDependencyObject()->GetName() ? v->AsDependencyObject()->GetName() : "");
				else
					markup = g_strdup_printf ("<i>[%d]</i>", i);

				GtkTreeIter child_iter;

				gtk_tree_store_append (store, &child_iter, &elements_iter);

				reflect_value (store, &child_iter, markup, NULL, v);

				g_free (markup);
			}
		}
	}

	if (obj->Is(Type::FRAMEWORKELEMENT) && !obj->Is(Type::PANEL) && !obj->Is (Type::BORDER)) {
		GtkTreeIter subobject_iter;

		gtk_tree_store_append (store, &subobject_iter, node);

		Value v(((Control*)obj)->GetSubtreeObject());

		reflect_value (store, &subobject_iter, "Visual Child", NULL, &v);
	}

	if (obj->Is (Type::NAMESCOPE)) {
		NameScope *scope = (NameScope *) obj;

		GHashTable *names = scope->GetNames ();
		if (names && g_hash_table_size (names) > 0) {
			
			AddNamescopeItemData *anid = new AddNamescopeItemData (store, node);
			
			g_hash_table_foreach (names, add_namescope_item, anid);
			delete anid;
		}
	}

}

static void
add_namescope_item (gpointer key, gpointer value, gpointer user_data)
{
	AddNamescopeItemData *anid = (AddNamescopeItemData *) user_data;
	char *name = (char *) key;
	DependencyObject *dob = (DependencyObject *) value;

	GtkTreeIter elements_iter;
	gtk_tree_store_append (anid->store, &elements_iter, anid->node);
	
	char *markup = g_strdup_printf (" <b>%s</b>", name);

	gtk_tree_store_set (anid->store, &elements_iter,
			COL_NAME, markup,
			COL_TYPE_NAME, dob->GetType ()->GetName (),
			COL_VALUE, "",
			COL_ELEMENT_PTR, dob,
			-1);

	g_free (markup);
}

static void
selection_changed (GtkTreeSelection *selection, PluginInstance *plugin)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	DependencyObject *el;

	Deployment::SetCurrent (plugin->GetDeployment ());

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

	if (el && el->Is(Type::UIELEMENT)) {
		UIElement *ui = (UIElement*)el;
		ui->Invalidate (ui->GetSubtreeBounds().GrowBy(1).RoundOut());
		ui->ref ();
		plugin->GetSurface()->debug_selected_element = ui;
	}

	Deployment::SetCurrent (NULL);
}

static void
surface_destroyed (EventObject *sender, EventArgs *args, gpointer closure)
{
	gtk_widget_destroy ((GtkWidget *) closure);
}

static void
remove_destroyed_handler (PluginInstance *plugin, GObject *window)
{
	Deployment::SetCurrent (plugin->GetDeployment ());
	plugin->GetSurface ()->RemoveHandler (EventObject::DestroyedEvent, surface_destroyed, window);
	Deployment::SetCurrent (NULL);
}

void
plugin_debug (PluginInstance *plugin)
{
	Surface *surface = plugin->GetSurface ();
	
	if (!surface) {
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
	
	Deployment::SetCurrent (plugin->GetDeployment ());

	surface->AddHandler (EventObject::DestroyedEvent, surface_destroyed, tree_win);
	g_object_weak_ref (G_OBJECT (tree_win), (GWeakNotify) remove_destroyed_handler, plugin);
	
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

	Deployment::SetCurrent (NULL);
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

static void unxap_callback (GtkWidget *widget, gpointer data)
{
	if (selected_source == NULL) {
		printf ("Select a source first.\n");
	} else {
		gchar* argv [3];
		argv [0] = (gchar*) "munxap";
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
		buf = (char*)uri->GetPath();
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
			path = (char*)uri->GetPath();
		
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
			path = (char*)uri->GetPath();
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
	
	button = gtk_button_new_with_label ("Unxap");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (unxap_callback), tree_store);
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	
	button = gtk_button_new_with_label ("Save (to /tmp/moon-dump/)");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (save_callback), tree_store);
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 0);
	
	gtk_container_add (GTK_CONTAINER (tree_win), GTK_WIDGET (vbox));

	gtk_widget_show_all (tree_win);
}

#endif
