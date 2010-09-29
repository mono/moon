/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include "deployment.h"
#include "factory.h"

namespace Moonlight {

#define INTERNAL_TYPE_KEY_MAGIC_COOKIE "___internal___moonlight___key___do___not__use___it___will___kill__cats__"

//
// ResourceDictionaryIterator
//

#ifndef HAVE_G_HASH_TABLE_ITER
struct KeyValuePair {
	gpointer key, value;
};

static void
add_key_value_pair (gpointer key, gpointer value, gpointer user_data)
{
	GArray *array = (GArray *) user_data;
	KeyValuePair pair;
	
	pair.value = value;
	pair.key = key;
	
	g_array_append_val (array, pair);
}
#endif

ResourceDictionaryIterator::ResourceDictionaryIterator (ResourceDictionary *resources) : CollectionIterator (resources)
{
#ifdef HAVE_G_HASH_TABLE_ITER
	Init ();
#else
	array = g_array_sized_new (false, false, sizeof (KeyValuePair), resources->array->len);
	g_hash_table_foreach (resources->hash, add_key_value_pair, array);
#endif
}

ResourceDictionaryIterator::~ResourceDictionaryIterator ()
{
	g_array_free (array, true);
}

#ifdef HAVE_G_HASH_TABLE_ITER
void
ResourceDictionaryIterator::Init ()
{
	g_hash_table_iter_init (&iter, ((ResourceDictionary *) collection)->hash);
	value = NULL;
	key = NULL;
}
#endif

bool
ResourceDictionaryIterator::Next (MoonError *err)
{
#ifdef HAVE_G_HASH_TABLE_ITER
	if (generation != collection->Generation ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "The underlying collection has mutated");
		return false;
	}
	
	if (!g_hash_table_iter_next (&iter, &key, &value)) {
		key = value = NULL;
		return false;
	}
	
	return true;
#else
	return CollectionIterator::Next (err);
#endif
}

bool
ResourceDictionaryIterator::Reset ()
{
#ifdef HAVE_G_HASH_TABLE_ITER
	if (generation != collection->Generation ())
		return false;
	
	Init ();
	
	return true;
#else
	return CollectionIterator::Reset ();
#endif
}

Value *
ResourceDictionaryIterator::GetCurrent (MoonError *err)
{
	if (generation != collection->Generation ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "The underlying collection has mutated");
		return NULL;
	}
	
#ifdef HAVE_G_HASH_TABLE_ITER
	if (key == NULL) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "Index out of bounds");
		return NULL;
	}
	
	return (Value *) value;
#else
	KeyValuePair pair;
	
	if (index < 0 || index >= collection->GetCount ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "Index out of bounds");
		return NULL;
	}
	
	pair = g_array_index (array, KeyValuePair, index);
	
	return (Value *) pair.value;
#endif
}

const char *
ResourceDictionaryIterator::GetCurrentKey (MoonError *err)
{
	if (generation != collection->Generation ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "The underlying collection has mutated");
		return NULL;
	}
	
#ifdef HAVE_G_HASH_TABLE_ITER
	if (key == NULL) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "Index out of bounds");
		return NULL;
	}
	
	return (const char *) key;
#else
	KeyValuePair pair;
	
	if (index < 0 || index >= collection->GetCount ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "Index out of bounds");
		return NULL;
	}
	
	pair = g_array_index (array, KeyValuePair, index);
	
	return (const char *) pair.key;
#endif
}


//
// ResourceDictionary
//

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
	from_resource_dictionary_api = false;
	source = NULL;

#if EVENT_ARG_REUSE
	changedEventArgs = NULL;
#endif
}

ResourceDictionary::~ResourceDictionary ()
{
	g_hash_table_destroy (hash);

	g_free (source);

#if EVENT_ARG_REUSE
	if (changedEventArgs)
		changedEventArgs->unref ();
#endif
}

CollectionIterator *
ResourceDictionary::GetIterator ()
{
	return new ResourceDictionaryIterator (this);
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

	Value *v = NULL;
	gpointer orig_key;

	gboolean exists = g_hash_table_lookup_extended (hash, key,
							&orig_key, (gpointer*)&v);

	if (exists) {
		MoonError::FillIn (error, MoonError::ARGUMENT, "An item with the same key has already been added");
		return false;
	}

	v = new Value (*value);
	
	from_resource_dictionary_api = true;
	bool result = Collection::AddWithError (v, error) != -1;
	from_resource_dictionary_api = false;
	if (result) {
		DependencyObject *ob = v->Is (GetDeployment (), Type::DEPENDENCY_OBJECT) ? v->AsDependencyObject () : NULL;
		if (ob && GetMentor ())
			ob->SetMentor (GetMentor ());

		g_hash_table_insert (hash, g_strdup (key), v);

		Value *v_copy = new Value (*v);

		EmitChanged (CollectionChangedActionAdd, v_copy, NULL, key);

		delete v_copy;

		if (addStrongRef && ob) {
			addStrongRef (this, ob, key);
			ob->unref();
			v->SetNeedUnref (false);
		}

		if (!strncmp (key, INTERNAL_TYPE_KEY_MAGIC_COOKIE, sizeof (INTERNAL_TYPE_KEY_MAGIC_COOKIE) - 1)
		    && v->Is (GetDeployment (), Type::STYLE)) {
			DependencyObject *p = GetParent();
			if (!p)
				return result;

			if (p->Is (Type::APPLICATION)) {
				// we modified the application's resources, so we need to traverse all layers

				CollectionIterator *iterator = p->GetDeployment()->GetSurface()->GetLayers()->GetIterator();
				while (iterator->Next (NULL)) {
					Value *v = iterator->GetCurrent(NULL);
					UIElement *ui = v->AsUIElement();

					DeepTreeWalker walker (ui);

					while (UIElement *el = walker.Step()) {
						((FrameworkElement*)el)->ApplyDefaultStyle();
					}
				}

				delete iterator;
			}
			else if (p->Is (Type::FRAMEWORKELEMENT)) {
				// just traverse down from this frameworkelement

				// FIXME grossly inefficient.  causes
				// all FWE's under the root to update
				// their implicit style, regardless of
				// whether or not it was their type.
				FrameworkElement *root = (FrameworkElement*)p;

				DependencyObject *owner = root->GetTemplateOwner();
				DeepTreeWalker walker (root);

				while (UIElement *el = walker.Step()) {
					if (el->GetTemplateOwner() != owner) {
						walker.SkipBranch();
						continue;
					}
					((FrameworkElement*)el)->ApplyDefaultStyle();
				}
				
			}
		}

	}
	return result;
}

static gboolean
_true ()
{
	return TRUE;
}

bool
ResourceDictionary::Clear ()
{
	EmitChanged (CollectionChangedActionClearing, NULL, NULL, NULL);
	
#if GLIB_CHECK_VERSION(2,12,0)
	if (glib_check_version (2,12,0))
		g_hash_table_remove_all (hash);
	else
#endif
	g_hash_table_foreach_remove (hash, (GHRFunc) _true, NULL);

	// FIXME: we need to clearStrongRef

	from_resource_dictionary_api = true;
	bool rv = Collection::Clear ();
	from_resource_dictionary_api = false;

	EmitChanged (CollectionChangedActionCleared, NULL, NULL, NULL);

	return rv;
}

bool
ResourceDictionary::ContainsKey (const char *key)
{
	bool exists = false;
	if (key)
		Get (key, &exists);
	return exists;
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

	from_resource_dictionary_api = true;
	Collection::Remove (orig_value);
	from_resource_dictionary_api = false;

	DependencyObject *ob = orig_value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT) ? orig_value->AsDependencyObject () : NULL;
	if (ob) {
		ob->SetMentor (NULL);
		if (clearStrongRef)
			clearStrongRef (this, ob, key);
	}

	Value *orig_copy = new Value (*orig_value);

	g_hash_table_remove (hash, key);

	EmitChanged (CollectionChangedActionRemove, NULL, orig_copy, key);

	delete orig_copy;

	return true;
}

bool
ResourceDictionary::Set (const char *key, Value *value)
{
	/* check if the item exists first */
	Value* orig_value;
	gpointer orig_key;

	if (g_hash_table_lookup_extended (hash, key,
					  &orig_key, (gpointer*)&orig_value)) {
		return false;
	}

	Value *orig_copy = new Value (*orig_value);
	Value *v = new Value (*value);

	from_resource_dictionary_api = true;
	Collection::Remove (orig_value);
	if (clearStrongRef && orig_value->Is (GetDeployment(), Type::DEPENDENCY_OBJECT))
		clearStrongRef (this, orig_value->AsDependencyObject(), key);
	Collection::Add (v);
	if (addStrongRef && v->Is (GetDeployment(), Type::DEPENDENCY_OBJECT))
		addStrongRef (this, v->AsDependencyObject(), key);
	from_resource_dictionary_api = false;

	g_hash_table_replace (hash, g_strdup (key), v);

	EmitChanged (CollectionChangedActionReplace, v, orig_copy, key);
	delete orig_copy;

	return true; // XXX
}

Value*
ResourceDictionary::Get (const char *key, bool *exists)
{
	Value *v = NULL;
	gpointer orig_key;

	*exists = g_hash_table_lookup_extended (hash, key,
						&orig_key, (gpointer*)&v);

	if (!*exists)
		v = GetFromMergedDictionaries (key, exists);

	return v;
}

Value *
ResourceDictionary::GetFromMergedDictionaries (const char *key, bool *exists)
{
	Value *v = NULL;

	ResourceDictionaryCollection *merged = GetMergedDictionaries ();

	if (!merged) {
		*exists = false;
		return NULL;
	}

	for (int i = merged->GetCount () - 1; i >= 0; i--) {
		ResourceDictionary *dict = merged->GetValueAt (i)->AsResourceDictionary ();
		v = dict->Get (key, exists);
		if (*exists)
			break;
	}

	return v;
}

static bool
can_be_added_twice (Deployment *deployment, Value *value)
{
	static Type::Kind twice_kinds [] = {
		Type::FRAMEWORKTEMPLATE,
		Type::STYLE,
		Type::STROKE_COLLECTION,
		Type::DRAWINGATTRIBUTES,
		Type::TRANSFORM,
		Type::BRUSH,
		Type::STYLUSPOINT_COLLECTION,
		Type::BITMAPIMAGE,
		Type::STROKE,
		Type::INVALID
	};

	for (int i = 0; twice_kinds [i] != Type::INVALID; i++) {
		if (Type::IsSubclassOf (deployment, value->GetKind (), twice_kinds [i]))
			return true;
	}

	return false;
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
bool
ResourceDictionary::AddedToCollection (Value *value, MoonError *error)
{
	DependencyObject *obj = NULL;
	bool rv = false;
	
	if (value->Is(GetDeployment (), Type::DEPENDENCY_OBJECT)) {
		obj = value->AsDependencyObject ();
		DependencyObject *parent = obj ? obj->GetParent () : NULL;
		// Call SetSurface() /before/ setting the logical parent
		// because Storyboard::SetSurface() needs to be able to
		// distinguish between the two cases.
	
		if (parent && !can_be_added_twice (GetDeployment (), value)) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, g_strdup_printf ("Element is already a child of another element.  %s", Type::Find (GetDeployment (), value->GetKind ())->GetName ()));
			return false;
		}
		
		obj->SetIsAttached (IsAttached ());
		obj->SetParent (this, error);
		if (error->number)
			return false;

		obj->AddPropertyChangeListener (this);

		if (!from_resource_dictionary_api) {
			const char *key = obj->GetName();

			if (!key) {
				MoonError::FillIn (error, MoonError::ARGUMENT_NULL, "key was null");
				goto cleanup;
			}

			if (ContainsKey (key)) {
				MoonError::FillIn (error, MoonError::ARGUMENT, "An item with the same key has already been added");
				goto cleanup;
			}
		}
	}

	rv = Collection::AddedToCollection (value, error);

	if (rv && !from_resource_dictionary_api && obj != NULL) {
		const char *key = obj->GetName();

		Value *obj_value = new Value (obj);

		obj->unref ();
		obj_value->SetNeedUnref (false);

		g_hash_table_insert (hash, g_strdup (key), obj_value);

		Value *obj_value_copy = new Value (*obj_value);

		EmitChanged (CollectionChangedActionAdd, obj_value_copy, NULL, key);

		delete obj_value_copy;

		if (addStrongRef)
			addStrongRef (this, obj, key);
	}

cleanup:
	if (!rv) {
		if (obj) {
			/* If we set the parent, but the object wasn't added to the collection, make sure we clear the parent */
			printf ("ResourceDictionary::AddedToCollection (): not added, clearing parent from %p\n", obj);
			obj->SetParent (NULL, NULL);
		}
	}

	return rv;
}

static gboolean
remove_from_hash_by_value (gpointer  key,
			   gpointer  value,
			   gpointer  user_data)
{
	Value *v = (Value*)value;
	DependencyObject *obj = (DependencyObject *) user_data;
	// FIXME: clearStrongRef
	return (v->GetKind () == obj->GetObjectType () && v->AsDependencyObject() == obj);
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::RemovedFromCollection (Value *value, bool is_value_safe)
{
	if (is_value_safe & value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT)) {
		DependencyObject *obj = value->AsDependencyObject ();

		if (obj) {
			obj->RemovePropertyChangeListener (this);
			obj->SetParent (NULL, NULL);
			obj->SetIsAttached (false);
		}
	}

	Collection::RemovedFromCollection (value, is_value_safe);

	if (is_value_safe && value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT)) {
		if (!from_resource_dictionary_api && value->AsDependencyObject()) {
			g_hash_table_foreach_remove (hash, remove_from_hash_by_value, value->AsDependencyObject ());

			// FIXME we need to EmitChanged something here so the managed RD can remain in sync
		}
	}
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::OnIsAttachedChanged (bool attached)
{
	Collection::OnIsAttachedChanged (attached);

	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		if (value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT)) {
			DependencyObject *obj = value->AsDependencyObject ();
			if (obj)
				obj->SetIsAttached (attached);
		}
	}
}

// XXX this was (mostly, except for the type check) c&p from DependencyObjectCollection
void
ResourceDictionary::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		if (value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT)) {
			DependencyObject *obj = value->AsDependencyObject ();
			if (obj)
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
		if (value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT)) {
			DependencyObject *obj = value->AsDependencyObject ();
			obj->RegisterAllNamesRootedAt (to_ns, error);
		}
	}
	
	Collection::RegisterAllNamesRootedAt (to_ns, error);
}

void
ResourceDictionary::EmitChanged (CollectionChangedAction action, Value *new_value, Value *old_value, const char *key)
{
#if EVENT_ARG_REUSE
	if (!HasHandlers (ResourceDictionary::ChangedEvent))
		return;

	if (!changedEventArgs)
		changedEventArgs = MoonUnmanagedFactory::CreateResourceDictionaryChangedEventArgs ();

	changedEventArgs->SetChangedAction (action);
	changedEventArgs->SetNewItem (new_value);
	changedEventArgs->SetOldItem (old_value);
	changedEventArgs->SetKey (key);

	changedEventArgs->ref ();

	Emit (ResourceDictionary::ChangedEvent, changedEventArgs);

	changedEventArgs->SetNewItem (NULL);
	changedEventArgs->SetOldItem (NULL);
	changedEventArgs->SetKey (NULL);
#else
	if (HasHandlers (ResourceDictionary::ChangedEvent))
		Emit (ResourceDictionary::ChangedEvent, new ResourceDictionaryChangedEventArgs (action, new_value, old_value, key));
#endif
}

void
ResourceDictionary::SetInternalSourceWithError (const char* source, MoonError *error)
{
	this->source = g_strdup (source);
	if (source == NULL)
		return;

	/* we need to walk up the tree checking for resource
	   dictionaries with the same source, generate an error if it
	   matches */
	DependencyObject *p = GetParent();
	while (p) {
		if (p->Is (Type::RESOURCE_DICTIONARY)) {
			ResourceDictionary* rd = (ResourceDictionary*)p;
			const char *rd_source = rd->GetInternalSource();
			if (rd_source && !strcmp (rd_source, source)) {
				MoonError::FillIn (error,
						   MoonError::INVALID_OPERATION, /* FIXME: verify exception type */
						   "cycle found in resource dictionaries");
				return;
			}
		}
		p = p->GetParent();
	}
}

const char*
ResourceDictionary::GetInternalSource ()
{
	return source;
}

};
