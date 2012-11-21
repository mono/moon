/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * debug-ui.cpp: debugging/inspection support for gtk+
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <gtk/gtk.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "animation.h"
#include "namescope.h"

#include "debug-ui-gtk.h"
#include "utils.h"
#include "uri.h"
#include "grid.h"
#include "playlist.h"
#include "mediaelement.h"
#include "mediaplayer.h"
#include "pipeline-asf.h"

using namespace Moonlight;

enum TreeColumns {
	COL_NAME,
	COL_TYPE_NAME,
	COL_VALUE,
	COL_ELEMENT_PTR,
	NUM_COLUMNS
};

#ifdef DEBUG

static void reflect_dependency_object_in_tree (DependencyObject *obj, GtkTreeStore *store,
					       GtkTreeIter *node, bool node_is_self, GHashTable *reflected_objects);

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
reflect_value (GtkTreeStore *store, GtkTreeIter *node, const char *name, const char *type_name, Value *value, GHashTable *reflected_objects)
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
			reflect_dependency_object_in_tree (dobj, store, node, true, reflected_objects);
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
			if (color)
				str = buf = g_strdup_printf ("<b>r=%g, g=%g, b=%g, a=%g</b>", color->r, color->g, color->b, color->a);
			else
				str = buf = g_strdup_printf ("<NULL>");
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
reflect_dependency_object_in_tree (DependencyObject *obj, GtkTreeStore *store, GtkTreeIter *node, bool node_is_self, GHashTable *reflected_objects)
{
	GtkTreeIter iter;

	if (obj == NULL)
		return;

	if (g_hash_table_lookup (reflected_objects, obj)) {
		gtk_tree_store_append (store, &iter, node);
		char *markup = g_strdup_printf ("<b>%s (Already in tree)</b>", obj->GetName() ? obj->GetName() : "");
		gtk_tree_store_set (store, &iter,
				    COL_NAME, markup,
				    COL_TYPE_NAME, obj->GetTypeName(),
				    COL_ELEMENT_PTR, obj,
				    -1);
		g_free (markup);

		node = &iter;
		return;
	}
	g_hash_table_insert (reflected_objects, obj, obj);

	if (!node_is_self) {
		gtk_tree_store_append (store, &iter, node);

		char *markup = g_strdup_printf ("<b>%s</b> (%p)", obj->GetName() ? obj->GetName() : "", obj);
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
			
			reflect_value (store, &iter, markup, prop_type ? prop_type->GetName () : "(unknown)", value, reflected_objects);
			
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
					markup = g_strdup_printf ("<i>[%d]</i> <b>%s</b> (%p)", i, v->AsDependencyObject()->GetName() ? v->AsDependencyObject()->GetName() : "", v->AsDependencyObject());
				else
					markup = g_strdup_printf ("<i>[%d]</i>", i);

				GtkTreeIter child_iter;

				gtk_tree_store_append (store, &child_iter, &elements_iter);

				reflect_value (store, &child_iter, markup, NULL, v, reflected_objects);

				g_free (markup);
			}
		}
	}

	if (obj->Is(Type::FRAMEWORKELEMENT) && !obj->Is(Type::PANEL) && !obj->Is (Type::BORDER)) {
		GtkTreeIter subobject_iter;

		gtk_tree_store_append (store, &subobject_iter, node);

		Value v(((Control*)obj)->GetSubtreeObject());

		reflect_value (store, &subobject_iter, "Visual Child", NULL, &v, reflected_objects);
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
selection_changed (GtkTreeSelection *selection, MoonWindowGtk *window)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	DependencyObject *el;

	window->SetCurrentDeployment ();

	if (window->GetSurface()->debug_selected_element) {
		UIElement *el = window->GetSurface()->debug_selected_element;
		el->Invalidate (el->GetSubtreeBounds().GrowBy(1).RoundOut());
		el->unref ();
		window->GetSurface()->debug_selected_element = NULL;
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
		window->GetSurface()->debug_selected_element = ui;
	}

	Deployment::SetCurrent (NULL);
}

static void
surface_destroyed (EventObject *sender, EventArgs *args, gpointer closure)
{
	gtk_widget_destroy ((GtkWidget *) closure);
}

static void
remove_destroyed_handler (MoonWindowGtk* window, GObject *gtk_window)
{
	window->SetCurrentDeployment ();
	window->GetSurface ()->RemoveHandler (EventObject::DestroyedEvent, surface_destroyed, window);
	Deployment::SetCurrent (NULL);
}

void
show_debug (MoonWindowGtk* window)
{
	Surface *surface = window->GetSurface ();
	
	if (!surface) {
#ifdef MOONLIGHT_GTK3
		GtkWidget *d = gtk_message_dialog_new (NULL,
						       GTK_DIALOG_MODAL,
						       GTK_MESSAGE_ERROR,
						       GTK_BUTTONS_CLOSE,
						       "This moonlight host hasn't been initialized with xaml content yet");
#else		
		GtkWidget *d = gtk_message_dialog_new (NULL,
						       GTK_DIALOG_NO_SEPARATOR,
						       GTK_MESSAGE_ERROR,
						       GTK_BUTTONS_CLOSE,
						       "This moonlight host hasn't been initialized with xaml content yet");
#endif
		gtk_dialog_run (GTK_DIALOG (d));
		g_object_unref (d);
		return;
	}

	GtkWidget *tree_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (tree_win), "Xaml contents");
	gtk_window_set_default_size (GTK_WINDOW (tree_win), 300, 400);
	
	window->SetCurrentDeployment();

	surface->AddHandler (EventObject::DestroyedEvent, surface_destroyed, tree_win);
	g_object_weak_ref (G_OBJECT (tree_win), (GWeakNotify) remove_destroyed_handler, window);
	
	GtkTreeStore *tree_store = gtk_tree_store_new (NUM_COLUMNS,
						       G_TYPE_STRING,
						       G_TYPE_STRING,
						       G_TYPE_STRING,
						       G_TYPE_POINTER);

	GHashTable *reflected_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	reflect_dependency_object_in_tree (window->GetSurface()->GetToplevel (), tree_store, NULL, false, reflected_objects);
	g_hash_table_destroy (reflected_objects);

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
			  G_CALLBACK (selection_changed), window);

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
populate_tree_from_surface (MoonWindowGtk *window, GtkTreeStore *store, GtkTreeIter *parent)
{
	if (window == NULL)
		return;

	GtkTreeIter iter;
	List *sources;
	Deployment::moon_source *src;
	
	sources = Deployment::GetCurrent()->GetSources ();
	
	if (sources == NULL)
		return;
	
	src = (Deployment::moon_source*) sources->First ();
	for (; src != NULL; src = (Deployment::moon_source*) src->next) {
		gtk_tree_store_append (store, &iter, parent);

		gtk_tree_store_set (store, &iter,
				    0, src->uri,
				    1, src->filename,
				    2, src,
				    -1);

	}
}

Deployment::moon_source *selected_source = NULL;

static void
selection_changed_sources (GtkTreeSelection *selection, MoonWindowGtk *window)
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
	char *filename, *url, *buf, *p;
	const char *path;
	const char *q;
	size_t max = (size_t) -1;
	GtkTreeIter iter;
	Uri *uri;
	
	if (!gtk_tree_model_get_iter_first (model, &iter))
		return 0;
	
	gtk_tree_model_get (model, &iter, 0, &url, 1, &filename, -1);
	
	uri = Uri::Create (url);
	if (url == NULL) {
		buf = g_strdup (filename);
	} else {
		buf = g_strdup (uri->GetPath ());
	}
	delete uri;
	
	if ((p = strrchr (buf, '/')))
		max = (p - buf);
	else
		max = 0;
	
	
	while (gtk_tree_model_iter_next (model, &iter)) {
		gtk_tree_model_get (model, &iter, 0, &url, 1, &filename, -1);
		
		uri = Uri::Create (url);
		if (uri == NULL)
			path = filename;
		else
			path = uri->GetPath();
		
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
	const char *uri_path;
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
		
		uri = Uri::Create (url);
		if (uri != NULL)
			uri_path = uri->GetPath();
		else
			uri_path = filename;
		
		path = g_build_filename ("/tmp/moon-dump", uri_path + prelen, NULL);
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
show_sources (MoonWindowGtk *window)
{	
	GtkWidget *tree_win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (tree_win), "Sources");
	gtk_window_set_default_size (GTK_WINDOW (tree_win), 600, 400);
#ifdef MOONLIGHT_GTK3
	GtkBox *vbox = GTK_BOX (gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
#else	
	GtkBox *vbox = GTK_BOX (gtk_vbox_new (false, 0));
#endif

	GtkTreeStore *tree_store = gtk_tree_store_new (3,
						       G_TYPE_STRING,
						       G_TYPE_STRING,
						       G_TYPE_POINTER);

	populate_tree_from_surface (window, tree_store, NULL);

	GtkWidget *tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_store));

	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

	g_signal_connect (G_OBJECT (selection), "changed", 
			  G_CALLBACK (selection_changed_sources), window);

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

static void
debug_info_dialog_response (GtkWidget *dialog, int response, gpointer user_data)
{
	gtk_widget_destroy (dialog);
}

enum DebugColumn {
	DEBUG_COLUMN_TOGGLE,
	DEBUG_COLUMN_NAME,
	DEBUG_COLUMN_FLAG,
	DEBUG_COLUMN_EX
};

static void
debug_cell_toggled (GtkCellRendererToggle *cell_renderer,
		    gchar *path,
		    GtkTreeModel *model) 
{
	GtkTreeIter iter;
	GtkTreePath *tree_path;
	gboolean set;
	gboolean is_ex;
	guint32 flag;

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
			    DEBUG_COLUMN_TOGGLE, &set,
			    DEBUG_COLUMN_FLAG, &flag,
			    DEBUG_COLUMN_EX, &is_ex,
			    -1);

	// we're toggling here
	set = !set;

	// toggle the debug state for moonlight
	if (is_ex)
		moonlight_set_debug_ex_option (flag, set);
	else
		moonlight_set_debug_option (flag, set);

	// and reflect the change in the UI
	gtk_list_store_set (GTK_LIST_STORE (model),
			    &iter,
			    DEBUG_COLUMN_TOGGLE, set,
			    -1);
}

static GtkWidget*
create_debug_treeview (gboolean is_ex)
{
	GtkListStore *model;
	GtkTreeIter iter;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeView *treeview;
	GtkWidget *scrolled;

	const MoonlightDebugOption *options;

	model = gtk_list_store_new (4,
				    /* DEBUG_COLUMN_TOGGLE */ G_TYPE_BOOLEAN,
				    /* DEBUG_COLUMN_NAME   */ G_TYPE_STRING,
				    /* DEBUG_COLUMN_FLAG   */ G_TYPE_INT,
				    /* DEBUG_COLUMN_EX     */ G_TYPE_BOOLEAN);

	options = is_ex ? moonlight_get_debug_ex_options () : moonlight_get_debug_options ();
	
	for (int i = 0; options [i].name != NULL; i++) {
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
				    DEBUG_COLUMN_TOGGLE, moonlight_get_debug_option (options [i].flag),
				    DEBUG_COLUMN_NAME, options [i].name,
				    DEBUG_COLUMN_FLAG, options [i].flag,
				    DEBUG_COLUMN_EX, is_ex,
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
#ifdef MOONLIGHT_GTK3
        g_signal_connect (GTK_WIDGET(renderer), "toggled", G_CALLBACK (debug_cell_toggled), model);
#else
        gtk_signal_connect (GTK_OBJECT(renderer), "toggled", G_CALLBACK (debug_cell_toggled), model);
#endif
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

void
debug_info (MoonWindowGtk *window)
{
	GtkWidget *dialog;
	GtkWidget *table;
	GtkBox *vbox;
	GtkWidget *frame;

#ifdef MOONLIGHT_GTK3
	dialog = gtk_dialog_new_with_buttons ("Debug Info Options", NULL, (GtkDialogFlags)
					      GTK_DIALOG_MODAL,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_NONE, NULL);
#else	
	dialog = gtk_dialog_new_with_buttons ("Debug Info Options", NULL, (GtkDialogFlags)
					      GTK_DIALOG_NO_SEPARATOR,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_NONE, NULL);
#endif  
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 8);
	
#ifdef MOONLIGHT_GTK3
        vbox = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
#else
        vbox = GTK_BOX (GTK_DIALOG (dialog)->vbox);
#endif
	// Debug options
	frame = gtk_frame_new ("Debug Options");
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	table = create_debug_treeview (FALSE);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (vbox, frame, TRUE, TRUE, 4);

	// Debug-ex options
	frame = gtk_frame_new ("Extra Debug Options");
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	table = create_debug_treeview (TRUE);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (vbox, frame, TRUE, TRUE, 4);

	g_signal_connect (dialog, "response", G_CALLBACK (debug_info_dialog_response), NULL);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 250, 400);

	gtk_widget_show_all (dialog);
}

#if OBJECT_TRACKING
struct debug_media_data {
	int count;
	int command;

	MediaElement **elements;
	GtkWidget **labels;
	guint timeout;
	GtkWidget *dialog;
	
	static void deleted_handler (EventObject *sender, EventArgs *args, void *user_data)
	{
		debug_media_data *data = (debug_media_data *) user_data;
		for (int i = 0; i < data->count; i++) {
			if (data->elements [i] == sender)
				data->elements [i] = NULL;
		}
	}
	
	static char *fetch_info (MediaElement *element)
	{
		int current_debug_flags = debug_flags;
		debug_flags = 0;

		element->SetCurrentDeployment ();

		MediaPlayer *mplayer = element->GetMediaPlayer ();
		Playlist *playlist = element->GetPlaylist ();
		PlaylistEntry *entry = playlist == NULL ? NULL : playlist->GetCurrentEntryLeaf ();
		Media *media = entry == NULL ? NULL : entry->GetMedia ();
		IMediaDemuxer *demuxer = media == NULL ? NULL : media->GetDemuxerReffed ();
		ASFDemuxer *asf_demuxer = NULL;
		IMediaSource *src = media == NULL ? NULL : media->GetSource ();
		AudioSource *audio = mplayer == NULL ? NULL : mplayer->GetAudio ();

		GString *fmt = g_string_new ("");
		g_string_append_printf (fmt, "MediaElement\n");
		const Uri *uri = element->GetSource ();
		char *markup;
		markup = uri ? g_markup_escape_text (uri->ToString (), -1) : NULL;
		g_string_append_printf (fmt, "\tSource: %s\n", markup);
		g_free (markup);
		markup = (entry != NULL && entry->GetFullSourceName () != NULL) ? g_markup_escape_text (entry->GetFullSourceName ()->GetOriginalString (), -1) : NULL;
		g_string_append_printf (fmt, "\tCurrent playlist entry's source: %s\n", markup);
		g_free (markup);
		g_string_append_printf (fmt, "\tState: %s\n", MediaElement::GetStateName (element->GetState ()));
		g_string_append_printf (fmt, "\tFlags: %s\n", MediaElement::GetFlagNames (element->GetFlags ()));
		g_string_append_printf (fmt, "\tPosition: %" G_GUINT64_FORMAT " ms NaturalDuration: %" G_GUINT64_FORMAT " AutoPlay: %i Balance: %.2f\n", 
			MilliSeconds_FromPts (element->GetPosition ()), MilliSeconds_FromPts (element->GetNaturalDuration ()->GetTimeSpan ()), element->GetAutoPlay (), element->GetBalance ());
		g_string_append_printf (fmt, "\tDownloadProgress: %.2f BufferingProgress: %.2f BufferingTime: %" G_GUINT64_FORMAT " ms DownloadProgressOffset: %.2f\n",
			element->GetDownloadProgress (), element->GetBufferingProgress (), MilliSeconds_FromPts (element->GetBufferingTime ()), element->GetDownloadProgressOffset ());
		if (mplayer != NULL) {
			g_string_append_printf (fmt, "\tMediaplayer: State: %s\n", MediaPlayer::GetStateName (mplayer->GetState ()));
			g_string_append_printf (fmt, "\t\tTarget pts: %" G_GUINT64_FORMAT " Current pts: %" G_GUINT64_FORMAT " ms Last rendered pts: %" G_GUINT64_FORMAT "\n", MilliSeconds_FromPts (mplayer->GetTargetPts ()), MilliSeconds_FromPts (mplayer->GetCurrentPts ()), MilliSeconds_FromPts (mplayer->GetLastRenderedPts ()));
			g_string_append_printf (fmt, "\t\tRendered fps: %.2f Dropped fps: %.2f\n", mplayer->GetRenderedFramesPerSecond (), mplayer->GetDroppedFramesPerSecond ());
			g_string_append_printf (fmt, "\t\tA/V pts diff:%s %" G_GINT64_FORMAT " ms\n", audio == NULL ? " N/A" : "", audio == NULL ? 0 : (gint64) MilliSeconds_FromPts (audio->GetCurrentPts ()) - (gint64) MilliSeconds_FromPts (mplayer->GetLastRenderedPts ()));
		}
		if (audio != NULL) {
			g_string_append_printf (fmt, "\tAudioPlayer: %s\n", audio->GetTypeName ());
			g_string_append_printf (fmt, "\t\tState: %s\n", AudioSource::GetStateName (audio->GetState ()));
			g_string_append_printf (fmt, "\t\tFlags: %s\n", AudioSource::GetFlagNames (audio->GetFlags ()));
			g_string_append_printf (fmt, "\t\tCurrent pts: %" G_GUINT64_FORMAT " ms Delay: %" G_GUINT64_FORMAT " ms\n", MilliSeconds_FromPts (audio->GetCurrentPts ()), MilliSeconds_FromPts (audio->GetDelay ()));
			g_string_append_printf (fmt, "\t\tBytes per frame: %u to %u\n", audio->GetInputBytesPerFrame (), audio->GetOutputBytesPerFrame ());
			g_string_append_printf (fmt, "\t\tBytes per sample: %u to %u\n", audio->GetInputBytesPerSample (), audio->GetOutputBytesPerSample ());
			g_string_append_printf (fmt, "\t\tVolume: %.2f Balance: %.2f Muted: %i\n", audio->GetVolume (), audio->GetBalance (), audio->GetMuted ());
			audio->unref ();
		}
		if (src != NULL) {
			g_string_append_printf (fmt, "\tSource: %s Size: %" G_GINT64_FORMAT " CanSeek: %i CanSeekToPts: %i Eof: %i", src->GetTypeName (), src->Is (Type::MMSPLAYLISTENTRY) ? -1 : src->GetSize (), src->CanSeek (), src->CanSeekToPts (), src->Eof ());
			if (src->Is (Type::PROGRESSIVESOURCE)) {
				g_string_append_printf (fmt, " Pending read requests: %u", ((ProgressiveSource *) src)->GetPendingReadRequestCount ());
			}
			g_string_append_printf (fmt, "\n");
		}
		if (demuxer != NULL) {
			if (demuxer->Is (Type::ASFDEMUXER))
				asf_demuxer = (ASFDemuxer *) demuxer;
			
			g_string_append_printf (fmt, "\t%s %i DRM: %i first pts: %" G_GUINT64_FORMAT " ms IsOpened: %i IsOpening: %i PendingStream: %s \n", demuxer->GetTypeName (), GET_OBJ_ID (demuxer), demuxer->IsDrm (), MilliSeconds_FromPts (demuxer->GetSeekedToPts ()), demuxer->IsOpened (), demuxer->IsOpening (), demuxer->GetPendingStream () ? demuxer->GetPendingStream ()->GetTypeName () : NULL);
			for (int i = 0; i < demuxer->GetStreamCount (); i++) {
				IMediaStream *stream = demuxer->GetStream (i);
				g_string_append_printf (fmt, "\t\t%s Id: %i Selected: %i Codec: %s Duration: %" G_GUINT64_FORMAT " ms Input ended: %i Output ended: %i\n",
					stream->GetTypeName (), stream->GetId (), stream->GetSelected (), stream->GetCodec (), MilliSeconds_FromPts (stream->GetDuration ()), stream->GetInputEnded (), stream->GetOutputEnded ());
				if (stream->Is (Type::VIDEOSTREAM)) {
					VideoStream *vs = (VideoStream *) stream;
					g_string_append_printf (fmt, 
						"\t\t\tWidth: %u Height: %u Bits per sample: %u Bitrate: %u Time per frame: %" G_GUINT64_FORMAT " ms (%.3f fps) Initial time: %" G_GUINT64_FORMAT " ms\n",
						vs->GetWidth (), vs->GetHeight (), vs->GetBitsPerSample (), vs->GetBitRate (), MilliSeconds_FromPts (vs->GetPtsPerFrame ()), 10000000.0 / vs->GetPtsPerFrame (), MilliSeconds_FromPts (vs->GetInitialPts ()));
				} else if (stream->Is (Type::AUDIOSTREAM)) {
					AudioStream *as = (AudioStream *) stream;
					g_string_append_printf (fmt,
						"\t\t\tBits per sample %u to %u Block align %u to %u Sample rate %u to %u Channels %u to %u Bit rate %u to %u\n",
						as->GetInputBitsPerSample (), as->GetOutputBitsPerSample (), as->GetInputBlockAlign (), as->GetOutputBlockAlign (), as->GetInputSampleRate (), as->GetOutputSampleRate (), as->GetInputChannels (), as->GetOutputChannels (), as->GetInputBitRate (), as->GetOutputBitRate ()
						);
				}
				g_string_append_printf (fmt, "\t\t\tBuffer: %" G_GUINT64_FORMAT " ms Demuxed frames in buffer: %i Decoded frames in buffer: %i Fps: %.2f.\n",
						MilliSeconds_FromPts (stream->GetBufferedSize ()),
						stream->GetDemuxedQueueLength (), stream->GetDecodedQueueLength (),
						(stream->GetDemuxedQueueLength () + stream->GetDecodedQueueLength ()) != 0 ? 1000.0 / (double) (MilliSeconds_FromPts (stream->GetBufferedSize ()) / (double) (stream->GetDemuxedQueueLength () + stream->GetDecodedQueueLength ())) : 0.0);
				g_string_append_printf (fmt, "\t\t\tFirst pts: %" G_GUINT64_FORMAT " ms.\n", MilliSeconds_FromPts (stream->GetFirstPts ()));
				g_string_append_printf (fmt, "\t\t\tLast enqueued demuxer pts: %" G_GUINT64_FORMAT " ms Last enqueued decoder pts: %" G_GUINT64_FORMAT ".\n",
					MilliSeconds_FromPts (stream->GetLastEnqueuedDemuxedPts ()), MilliSeconds_FromPts (stream->GetLastEnqueuedDecodedPts ()));
				g_string_append_printf (fmt, "\t\t\tLast popped pts: %" G_GUINT64_FORMAT " ms.\n", MilliSeconds_FromPts (stream->GetLastPoppedPts ()));
				IMediaDecoder *decoder = stream->GetDecoder ();
				if (decoder != NULL) {
					g_string_append_printf (fmt, "\t\t\tDecoder: %s\n", decoder->GetTypeName ());
				} else {
					g_string_append_printf (fmt, "\t\t\t(No decoder)\n");
				}
				if (asf_demuxer != NULL) {
					g_string_append_printf (fmt, "\t\t\tASF Frame reader: %i payloads in queue.\n", asf_demuxer->GetPayloadCount (stream));
				}
			}
			demuxer->unref ();
		} else {
			g_string_append_printf (fmt, "\t(No demuxer)\n");
		}
		if (playlist != NULL) {
			playlist->Dump (fmt, true);
		} else {
			g_string_append_printf (fmt, "\t(No playlist)\n");
		}

		char *result = fmt->str;
		g_string_free (fmt, false);

		debug_flags = current_debug_flags;		

		return result;
	}

	void update ()
	{
		for (int i = 0; i < count; i++) {
			bool copy = false;
			MediaElement *element = elements [i];
			if (element == NULL)
				continue;
			element->SetCurrentDeployment ();
			Playlist *playlist = element->GetPlaylist ();
			PlaylistEntry *entry = playlist == NULL ? NULL : playlist->GetCurrentEntryLeaf ();
			Media *media = entry == NULL ? NULL : entry->GetMedia ();
			IMediaDemuxer *demuxer = media == NULL ? NULL : media->GetDemuxerReffed ();
	
			if (command != 0) {
				if (command & 1) {
					if (demuxer)
						demuxer->FillBuffers ();
				}
				if (command & 2)
					element->Play ();
				if (command & 4)
					element->Pause ();
				if (command & 8)
					element->Stop ();
				if (command & 16)
					copy = true;
				command = 0;
			}
	
			char *fmt = fetch_info (element);

			gtk_label_set_markup (GTK_LABEL (labels [i]), fmt);
			if (copy)
				gtk_clipboard_set_text( gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), fmt, strlen (fmt) );
			g_free (fmt);

			if (demuxer)
				demuxer->unref ();
		}
	}
	
	static gboolean update_timeout (void *data)
	{
		((debug_media_data *) data)->update ();
		return true;
	}
};

void
dump_media_elements ()
{
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	Deployment *deployment = Deployment::GetCurrent ();
	MediaElement *elements [1024];
	int counter = 0;

	/* Find all media elements */
	deployment->objects_alive_mutex.Lock();
	g_hash_table_iter_init (&iter, deployment->objects_alive);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		if (((EventObject *) key)->GetObjectType () != Type::MEDIAELEMENT)
			continue;
		elements [counter++] = (MediaElement *) key;
		elements [counter - 1]->ref ();
	}
	deployment->objects_alive_mutex.Unlock();

	for (int i = 0; i < counter; i++) {
		MediaElement *mel = elements [i];
		char *fmt = debug_media_data::fetch_info (mel);
		printf (fmt);
		g_free (fmt);
		mel->unref ();
	}
}

static void
debug_media_dialog_response (GtkWidget *dialog, int response, debug_media_data *data)
{
	switch (response) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
		data->command = response;
		break;
	case 1337:
		dump_media_elements ();
		break;
	default:
		g_source_remove (data->timeout);
		gtk_widget_destroy (dialog);
		break;
	}
}

void
debug_media (MoonWindowGtk *window)
{
	GtkBox *vbox;
	Deployment *deployment = window->GetSurface ()->GetUnsafeDeployment ();
	debug_media_data *data;
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	GtkWidget *label_scrollwindow;
	GtkWidget *label_evtbox;
	GdkColor white = {0, 65535, 65535, 65535};
	
	if (deployment == NULL) {
		fprintf (stderr, "Moonlight: plugin hasn't created a deployment yet.\n");
		return;
	}
	
	data = (debug_media_data *) g_malloc0 (sizeof (debug_media_data));
	 
	Deployment::SetCurrent (deployment);
#ifdef MOONLIGHT_GTK3
	data->dialog = gtk_dialog_new_with_buttons ("MediaElements", NULL, (GtkDialogFlags)
					      GTK_DIALOG_MODAL,
					      "Copy", 16, "Play", 2, "Pause", 4, "Stop", 8, "FillBuffers", 1, GTK_STOCK_CLOSE, GTK_RESPONSE_NONE, NULL);
#else
	data->dialog = gtk_dialog_new_with_buttons ("MediaElements", NULL, (GtkDialogFlags)
					      GTK_DIALOG_NO_SEPARATOR,
					      "Copy", 16, "Play", 2, "Pause", 4, "Stop", 8, "FillBuffers", 1, GTK_STOCK_CLOSE, GTK_RESPONSE_NONE, NULL);
#endif	
#ifdef MOONLIGHT_GTK3
	g_object_set (GTK_WIDGET (data->dialog), "resizable", true, NULL);
#else
	gtk_object_set (GTK_OBJECT (data->dialog), "resizable", true, NULL);
#endif
	gtk_window_set_type_hint (GTK_WINDOW (data->dialog), GDK_WINDOW_TYPE_HINT_NORMAL); // to show the maximize button

	gtk_container_set_border_width (GTK_CONTAINER (data->dialog), 8);
	
#ifdef MOONLIGHT_GTK3
        vbox = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(data->dialog)));
#else
	vbox = GTK_BOX (GTK_DIALOG (data->dialog)->vbox);
#endif
	
	/* Find all media elements */
	deployment->objects_alive_mutex.Lock();
	g_hash_table_iter_init (&iter, deployment->objects_alive);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		if (((EventObject *) key)->GetObjectType () != Type::MEDIAELEMENT)
			continue;
		
		data->count++;		
		data->elements = (MediaElement **) g_realloc (data->elements, data->count * sizeof (MediaElement *));
		data->labels = (GtkWidget **) g_realloc (data->labels, data->count * sizeof (GtkWidget *));
		
		data->elements [data->count - 1] = (MediaElement *) key;
		data->elements [data->count - 1]->AddHandler (EventObject::DestroyedEvent, debug_media_data::deleted_handler, data);
		data->labels [data->count - 1] = gtk_label_new (NULL);
		gtk_label_set_selectable (GTK_LABEL (data->labels [data->count - 1]), TRUE);
		gtk_label_set_line_wrap (GTK_LABEL (data->labels [data->count - 1]), FALSE);
	
		gtk_misc_set_alignment (GTK_MISC (data->labels [data->count - 1]), 0.0, 0.5);

		label_scrollwindow = gtk_scrolled_window_new (NULL, NULL);
		label_evtbox = gtk_event_box_new ();

		gtk_widget_modify_bg (GTK_WIDGET (label_evtbox), GTK_STATE_NORMAL, &white);
		gtk_container_add (GTK_CONTAINER (label_evtbox), data->labels [data->count - 1]);

		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (label_scrollwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (label_scrollwindow), GTK_SHADOW_IN);
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (label_scrollwindow), label_evtbox);
		gtk_widget_set_size_request (label_scrollwindow, -1, 225);
		gtk_box_pack_start (vbox, label_scrollwindow, TRUE, TRUE, 0);
	}
	deployment->objects_alive_mutex.Unlock();
	
	data->update ();	
	g_signal_connect (data->dialog, "response", G_CALLBACK (debug_media_dialog_response), data);
	gtk_widget_show_all (data->dialog);
	
	data->timeout = g_timeout_add (100, debug_media_data::update_timeout, data);
}
#endif /* OBJECT_TRACKING */
#endif
