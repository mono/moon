/*
 * namescope.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <glib.h>
#include <stdio.h>

#include "namescope.h"

NameScope::NameScope ()
{
	SetObjectType (Type::NAMESCOPE);
	is_locked = false;
	names = NULL;
	temporary = false;
}

gboolean
NameScope::remove_handler (gpointer key, gpointer value, gpointer data)
{
	DependencyObject *depobj = (DependencyObject*)value;
	depobj->RemoveHandler (EventObject::DestroyedEvent, NameScope::ObjectDestroyedEvent, data);
	return TRUE;
}

NameScope::~NameScope ()
{
	if (names) {
		g_hash_table_foreach_remove (names, remove_handler, this);
		g_hash_table_destroy (names);
	}
}

static void
register_name (char *key, DependencyObject *obj, NameScope* ns)
{
	ns->RegisterName (key, obj);
}

void
NameScope::CloneCore (Types *types, DependencyObject *fromObj)
{
	NameScope *ns = (NameScope*)fromObj;

	g_hash_table_foreach (ns->names, (GHFunc)register_name, this);

	is_locked = ns->is_locked;
	temporary = ns->temporary;
}

void
NameScope::Dispose ()
{
	if (names)
		g_hash_table_foreach_remove (names, remove_handler, this);
		
	DependencyObject::Dispose ();
}

static gboolean
remove_object_from_namescope (gpointer key, gpointer value, gpointer user_data)
{
	return value == user_data;
}

void
NameScope::ObjectDestroyedEvent (EventObject *sender, EventArgs *args, gpointer closure)
{
	NameScope *ns = (NameScope*)closure;
	// XXX this method worries me.. using GetName like this.
	DependencyObject *depobj = (DependencyObject*)sender;
	const char *name = depobj->GetName ();
	if (name != NULL) {
		g_hash_table_remove (ns->names, name);
	} else {
		g_hash_table_foreach_remove (ns->names, remove_object_from_namescope, depobj);
	}
}

void
NameScope::RegisterName (const char *name, DependencyObject *object)
{
	if (GetIsLocked ())
		return;

	if (!names) {
		names = g_hash_table_new_full (g_str_hash, g_str_equal,
					       (GDestroyNotify)g_free,
					       NULL);
	}

	DependencyObject *existing_object = (DependencyObject*)g_hash_table_lookup (names, name);
	if (existing_object == object)
		return;

	if (existing_object) {
		existing_object->RemoveHandler (EventObject::DestroyedEvent, ObjectDestroyedEvent, this);
	}

	object->AddHandler (EventObject::DestroyedEvent, NameScope::ObjectDestroyedEvent, this);
	g_hash_table_insert (names, g_strdup (name), object);
}

void
NameScope::UnregisterName (const char *name)
{
	if (GetIsLocked ())
		return;

	if (!names)
		return;

	DependencyObject *depobj = (DependencyObject*)g_hash_table_lookup (names, name);
	if (depobj) {
		depobj->RemoveHandler (EventObject::DestroyedEvent, ObjectDestroyedEvent, this);

		g_hash_table_remove (names, name);
	}
}

DependencyObject*
NameScope::FindName (const char *name)
{
	if (!names)
		return NULL;

	if (name == NULL) {
		g_warning ("NameScope::FindName (null)");
		return NULL;
	}
	return (DependencyObject *) g_hash_table_lookup (names, name);
}

void
NameScope::merge_name (gpointer key, gpointer value, gpointer user_data)
{
	char *name = (char*)key;
	DependencyObject *obj = (DependencyObject*)value;
	NameScope *scope = (NameScope*)user_data;

	scope->RegisterName (name, obj);
}

struct DuplicatesData {
  NameScope *ns;
  bool duplicate_found;
  char *duplicate_name;
};

static void
look_for_duplicates (gpointer key, gpointer value, gpointer user_data)
{
	DuplicatesData *data = (DuplicatesData*)user_data;

	if (data->duplicate_found)
		return;

	char *name = (char*)key;
	void *o = data->ns->FindName (name);
	if (o && o != value) {
		data->duplicate_found = true;
		data->duplicate_name = g_strdup (name);
	}
}

void
NameScope::MergeTemporaryScope (NameScope *temp, MoonError *error)
{
	if (!temp || !temp->names)
		return;

	DuplicatesData data;
	data.ns = this;
	data.duplicate_found = false;
	data.duplicate_name = NULL;

	g_hash_table_foreach (temp->names, look_for_duplicates, &data);
	if (data.duplicate_found) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 2028,
				   g_strdup_printf ("The name already exists in the tree: %s.",
						    data.duplicate_name));
		g_free (data.duplicate_name);
		return;
	}

	g_hash_table_foreach (temp->names, merge_name, this);
}

static void
dump_namescope_helper (gpointer key, gpointer value, gpointer user_data)
{
	fprintf (stderr, "  %s => %s\n", (char*)key, ((DependencyObject*)value)->GetTypeName());
}


void
NameScope::Dump ()
{
	fprintf (stderr, "  ns = %p\n", this);
	g_hash_table_foreach (names, dump_namescope_helper, NULL);
}
