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

static void
free_value (Value *value)
{
	delete value;
}

ResourceDictionary::ResourceDictionary ()
{
	SetObjectType (Type::RESOURCE_DICTIONARY);
	hash = g_hash_table_new_full (g_str_hash,
				      g_str_equal,
				      (GDestroyNotify)g_free,
				      (GDestroyNotify)free_value);
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
ResourceDictionary::Add (const char* key, Value *value)
{
	MoonError err;
	return AddWithError (key, value, &err);
}

bool
ResourceDictionary::AddWithError (const char* key, Value *value, MoonError *error)
{
	if (!key) {
		MoonError::FillIn (error, MoonError::ARGUMENT_NULL, "key was null");
		return false;
	}

	if (ContainsKey (key)) {
		MoonError::FillIn (error, MoonError::ARGUMENT, "An item with the same key has already been added");
		return false;
	}

	Value *v = new Value (*value);
	g_hash_table_insert (hash, g_strdup (key), v);
	bool result = Collection::AddWithError (v, error) != -1;
	if (!result)
		g_hash_table_remove (hash, key);
	return result;
}

bool
ResourceDictionary::Clear ()
{
#if GLIB_CHECK_VERSION(2,12,0)
	if (glib_check_version (2,12,0))
		g_hash_table_remove_all (hash);
	else
#endif
	g_hash_table_foreach_remove (hash, (GHRFunc) gtk_true, NULL);

	return Collection::Clear ();
}

bool
ResourceDictionary::ContainsKey (const char *key)
{
	if (!key)
		return false;

	gpointer orig_value;
	gpointer orig_key;

	return g_hash_table_lookup_extended (hash, key,
					     &orig_key, &orig_value);
}

bool
ResourceDictionary::Remove (const char *key)
{
	if (!key)
		return false;

	/* check if the item exists first */
	Value* orig_value;
	gpointer orig_key;

	if (!g_hash_table_lookup_extended (hash, key,
					   &orig_key, (gpointer*)&orig_value))
		return false;

	Collection::Remove (orig_value);

	g_hash_table_remove (hash, key);

	return true;
}

bool
ResourceDictionary::Set (const char *key, Value *value)
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
ResourceDictionary::Get (const char *key, bool *exists)
{
	Value *v = NULL;
	gpointer orig_key;

	*exists = g_hash_table_lookup_extended (hash, key,
						&orig_key, (gpointer*)&v);

	return v;
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
bool
ResourceDictionary::AddedToCollection (Value *value, MoonError *error)
{
	if (value->Is(Type::DEPENDENCY_OBJECT)) {
		DependencyObject *obj = value->AsDependencyObject ();
		DependencyObject *parent = obj ? obj->GetParent () : NULL;
		// Call SetSurface() /before/ setting the logical parent
		// because Storyboard::SetSurface() needs to be able to
		// distinguish between the two cases.
	
		if (parent) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Element is already a child of another element.");
			return false;
		}
		
		obj->SetSurface (GetSurface ());
		obj->SetParent (this, error);
		if (error->number)
			return false;

		obj->AddPropertyChangeListener (this);
	}

	return Collection::AddedToCollection (value, error);
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::RemovedFromCollection (Value *value)
{
	if (value->Is (Type::DEPENDENCY_OBJECT)) {
		DependencyObject *obj = value->AsDependencyObject ();
		
		obj->RemovePropertyChangeListener (this);
		obj->SetParent (NULL, NULL);
		obj->SetSurface (NULL);
		
		Collection::RemovedFromCollection (value);
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
ResourceDictionary::RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error)
{
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		if (error->number)
			break;

		value = (Value *) array->pdata[i];
		if (value->Is (Type::DEPENDENCY_OBJECT)) {
			DependencyObject *obj = value->AsDependencyObject ();
			obj->RegisterAllNamesRootedAt (to_ns, error);
		}
	}
	
	Collection::RegisterAllNamesRootedAt (to_ns, error);
}
