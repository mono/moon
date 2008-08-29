/*
 * resources.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>

#include "runtime.h"
#include "resources.h"
#include "namescope.h"
#include "error.h"

ResourceDictionary::ResourceDictionary ()
{
	hash = g_hash_table_new_full (g_str_hash,
				      g_str_equal,
				      (GDestroyNotify)g_free,
				      (GDestroyNotify)value_free_value);
}

ResourceDictionary::~ResourceDictionary ()
{
	g_hash_table_destroy (hash);
}

bool
ResourceDictionary::CanAdd (Value *value)
{
	return true;
}

bool
ResourceDictionary::Add (char* key, Value *value)
{
	if (ContainsKey (key))
		return false;

	Value *v = new Value (*value);
	
	g_hash_table_insert (hash, g_strdup (key), v);

	Collection::Add (v);

	return true;
}

void
ResourceDictionary::AddWithError (char* key, Value *value, MoonError *error)
{
	if (!Add (key, value))
		MoonError::FillIn (error, MoonError::ARGUMENT, "An item with the same key has already been added");
}

bool
ResourceDictionary::Clear ()
{
	Collection::Clear ();

	g_hash_table_remove_all (hash);

	return true;
}

bool
ResourceDictionary::ContainsKey (char *key)
{
	gpointer orig_value;
	gpointer orig_key;

	return g_hash_table_lookup_extended (hash, key,
					     &orig_key, &orig_value);
}

bool
ResourceDictionary::Remove (char *key)
{
	/* check if the item exists first */
	Value* orig_value;
	gpointer orig_key;

	if (g_hash_table_lookup_extended (hash, key,
					  &orig_key, (gpointer*)&orig_value)) {
		return false;
	}

	Collection::Remove (orig_value);

	g_hash_table_remove (hash, key);

	return true;
}

bool
ResourceDictionary::Set (char *key, Value *value)
{
	Value *v = new Value (*value);

	/* check if the item exists first */
	Value* orig_value;
	gpointer orig_key;

	if (g_hash_table_lookup_extended (hash, key,
					  &orig_key, (gpointer*)&orig_value)) {
		return false;
	}

	Collection::Remove (orig_value);
	Collection::Add (v);

	g_hash_table_replace (hash, g_strdup (key), v);

	return true; // XXX
}

Value*
ResourceDictionary::Get (char *key, bool *exists)
{
	Value *v = NULL;
	gpointer orig_key;

	*exists = g_hash_table_lookup_extended (hash, key,
						&orig_key, (gpointer*)&v);

	return v;
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::AddedToCollection (Value *value)
{
	if (value->Is(Type::DEPENDENCY_OBJECT)) {
		DependencyObject *obj = value->AsDependencyObject ();
	
		// Call SetSurface() /before/ setting the logical parent
		// because Storyboard::SetSurface() needs to be able to
		// distinguish between the two cases.
	
		obj->SetSurface (GetSurface ());
		obj->SetLogicalParent (this);
		obj->AddPropertyChangeListener (this);
	
		MergeNames (obj);
	
		Collection::AddedToCollection (value);
	}
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::RemovedFromCollection (Value *value)
{
	if (value->Is (Type::DEPENDENCY_OBJECT)) {
		DependencyObject *obj = value->AsDependencyObject ();
		NameScope *ns;
		
		obj->RemovePropertyChangeListener (this);
		obj->SetLogicalParent (NULL);
		obj->SetSurface (NULL);
		
		// unregister the name from whatever scope it's registered in
		// if it's got its own, don't worry about it.
		if (!(ns = NameScope::GetNameScope (obj))) {
			if ((ns = obj->FindNameScope ()))
				obj->UnregisterAllNamesRootedAt (ns);
		}
		
		Collection::RemovedFromCollection (value);
	}
}

// XXX this was c&p from DependencyObjectCollection
void
ResourceDictionary::MergeNames (DependencyObject *new_obj)
{
	if (!GetLogicalParent())
		return;
	
	NameScope *ns = NameScope::GetNameScope (new_obj);
	
	/* this should always be true for Canvas subclasses */
	if (ns) {
		if (ns->GetTemporary ()) {
			NameScope *con_ns = GetLogicalParent()->FindNameScope ();
			if (con_ns) {
				con_ns->MergeTemporaryScope (ns);
				// get rid of the old namescope after we merge
				new_obj->ClearValue (NameScope::NameScopeProperty, false);
			}
		}
	} else {
		NameScope *con_ns = GetLogicalParent()->FindNameScope ();
		if (con_ns)
			new_obj->RegisterAllNamesRootedAt (con_ns);
	}
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::SetSurface (Surface *surface)
{
	if (GetSurface() == surface)
		return;

	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		if (value->Is (Type::DEPENDENCY_OBJECT)) {
			DependencyObject *obj = value->AsDependencyObject ();
			obj->SetSurface (surface);
		}
	}
	
	Collection::SetSurface (surface);
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		if (value->Is (Type::DEPENDENCY_OBJECT)) {
			DependencyObject *obj = value->AsDependencyObject ();
			obj->UnregisterAllNamesRootedAt (from_ns);
		}
	}
	
	Collection::UnregisterAllNamesRootedAt (from_ns);
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::RegisterAllNamesRootedAt (NameScope *to_ns)
{
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		if (value->Is (Type::DEPENDENCY_OBJECT)) {
			DependencyObject *obj = value->AsDependencyObject ();
			obj->RegisterAllNamesRootedAt (to_ns);
		}
	}
	
	Collection::RegisterAllNamesRootedAt (to_ns);
}
