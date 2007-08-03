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

#include "namescope.h"

NameScope::NameScope ()
{
	names = g_hash_table_new (g_str_hash, g_str_equal);
	temporary = false;
}

NameScope::~NameScope ()
{
  //	g_hash_table_foreach (/* XXX */);
}

void
NameScope::RegisterName (const char *name, DependencyObject *object)
{
	g_hash_table_insert (names, g_strdup (name) ,object);
}

void
NameScope::UnregisterName (const char *name)
{
}

DependencyObject*
NameScope::FindName (const char *name)
{
	return (DependencyObject*)g_hash_table_lookup (names, name);
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

static void
merge_scope_func (void *key, void *value, void *data)
{
	char *name = (char *) key;
	DependencyObject *obj = (DependencyObject *) value;
	NameScope *scope = (NameScope *) data;

	scope->RegisterName (name, obj);
}

void
NameScope::MergeTemporaryScope (NameScope *temp)
{
	g_hash_table_foreach (temp->names, merge_scope_func, this);
}

DependencyProperty *NameScope::NameScopeProperty;

void
namescope_init (void)
{
	NameScope::NameScopeProperty = DependencyObject::Register (Type::NAMESCOPE, "NameScope", Type::NAMESCOPE);
}
