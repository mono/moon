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
	names = g_hash_table_new_full (g_str_hash, g_str_equal,
				       (GDestroyNotify)g_free,
				       NULL);
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
	g_hash_table_foreach_remove (names, remove_handler, this);
	g_hash_table_destroy (names);
}

void
NameScope::ObjectDestroyedEvent (EventObject *sender, EventArgs *args, gpointer closure)
{
	NameScope *ns = (NameScope*)closure;
	// XXX this method worries me.. using GetName like this.
	DependencyObject *depobj = (DependencyObject*)sender;
	g_hash_table_remove (ns->names, depobj->GetName());
}

void
NameScope::RegisterName (const char *name, DependencyObject *object)
{
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
	DependencyObject *depobj = (DependencyObject*)g_hash_table_lookup (names, name);
	if (depobj) {
		depobj->RemoveHandler (EventObject::DestroyedEvent, ObjectDestroyedEvent, this);

		g_hash_table_remove (names, name);
	}
}

DependencyObject*
NameScope::FindName (const char *name)
{
	if (name == NULL) {
		g_warning ("NULL passed to FindName");
		return NULL;
	}

	DependencyObject *o = (DependencyObject*)g_hash_table_lookup (names, name);
	if (o)
		return o;

	return NULL;
}

NameScope*
NameScope::GetNameScope (DependencyObject *obj)
{
	Value *v = obj->GetValue (NameScope::NameScopeProperty);
	return v == NULL ? NULL : v->AsNameScope();
}

void
NameScope::SetNameScope (DependencyObject *obj, NameScope *scope)
{
	obj->SetValue (NameScope::NameScopeProperty, scope);
}

void
NameScope::merge_name (gpointer key, gpointer value, gpointer user_data)
{
	char *name = (char*)key;
	DependencyObject *obj = (DependencyObject*)value;
	NameScope *scope = (NameScope*)user_data;

	scope->RegisterName (name, obj);
}

void
NameScope::MergeTemporaryScope (NameScope *temp)
{
	g_hash_table_foreach (temp->names, merge_name, this);
}

DependencyProperty *NameScope::NameScopeProperty;

void
namescope_init (void)
{
	NameScope::NameScopeProperty = DependencyObject::Register (Type::NAMESCOPE, "NameScope", Type::NAMESCOPE);
}
