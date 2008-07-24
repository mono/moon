/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * collection.cpp: different types of collections
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "collection.h"
#include "geometry.h"
#include "transform.h"
#include "frameworkelement.h"
#include "trigger.h"
#include "namescope.h"
#include "utils.h"


DependencyProperty *Collection::CountProperty;

Collection::Collection ()
{
	array = g_ptr_array_new ();
	generation = 0;
	closure = NULL;
	unique = false;
}

Collection::~Collection ()
{
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		RemovedFromCollection (value);
		delete value;
	}
	
	g_ptr_array_free (array, true);
}

Value *
Collection::GetValue (DependencyProperty *property)
{
	if (property == Collection::CountProperty)
		return new Value ((int) array->len);
	
	return DependencyObject::GetValue (property);
}

int
Collection::Add (Value value)
{
	Value *added;
	
	// make sure value is of the proper type
	if (!value.Is (GetElementType ()))
		return -1;
	
	// make sure the value isn't already in the collection (if the collection must only hold unique values)
	if (unique && Contains (value))
		return -1;
	
	added = new Value (value);
	
	AddedToCollection (added);
	
	g_ptr_array_add (array, added);
	
	EmitChanged (CollectionChangedActionAdd, added, NULL, array->len - 1);
	
	return array->len - 1;
}

void
Collection::Clear ()
{
	Value *value;
	
	if (closure)
		closure->OnCollectionClear (this);
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		RemovedFromCollection (value);
		delete value;
	}
	
	g_ptr_array_set_size (array, 0);
	generation++;
	
	EmitChanged (CollectionChangedActionReset, NULL, NULL, -1);
}

bool
Collection::Contains (Value value)
{
	// make sure value is of the proper type
	if (!value.Is (GetElementType ()))
		return false;
	
	return IndexOf (value) != -1;
}

int
Collection::IndexOf (Value value)
{
	Value *v;
	
	for (guint i = 0; i < array->len; i++) {
		v = (Value *) array->pdata[i];
		if (*v == value)
			return i;
	}
	
	return -1;
}

bool
Collection::Insert (int index, Value value)
{
	Value *added;
	
	// make sure value is of the proper type
	if (!value.Is (GetElementType ()))
		return false;
	
	if (index < 0)
		return false;
	
	if (index > GetCount ())
		index = GetCount ();
	
	added = new Value (value);
	
	AddedToCollection (added);
	
	g_ptr_array_insert (array, index, added);
	
	EmitChanged (CollectionChangedActionAdd, added, NULL, index);
	
	return true;
}

bool
Collection::Remove (Value value)
{
	int index;
	
	if ((index = IndexOf (value)) == -1)
		return false;
	
	return RemoveAt (index);
}

bool
Collection::RemoveAt (int index)
{
	Value *value;
	
	// check bounds
	if (index < 0 || (guint) index >= array->len)
		return false;
	
	value = (Value *) array->pdata[index];
	RemovedFromCollection (value);
	
	g_ptr_array_remove_index (array, index);
	generation++;
	
	EmitChanged (CollectionChangedActionRemove, NULL, value, index);
	
	delete value;
	
	return true;
}

Value *
Collection::GetValueAt (int index)
{
	if (index < 0 || (guint) index >= array->len)
		return NULL;
	
	return (Value *) array->pdata[index];
}

Value *
Collection::SetValueAt (int index, Value value)
{
	Value *added, *removed;
	
	// make sure the object is of the correct type
	if (!value.Is (GetElementType ()))
		return NULL;
	
	// check array bounds
	if (index < 0 || (guint) index >= array->len)
		return NULL;
	
	removed = (Value *) array->pdata[index];
	RemovedFromCollection (removed);
	
	added = new Value (value);
	AddedToCollection (added);
	array->pdata[index] = added;
	
	EmitChanged (CollectionChangedActionReplace, added, removed, index);
	
	return removed;
}

void
Collection::EmitChanged (CollectionChangedAction action, Value *new_value, Value *old_value, int index)
{
	CollectionChangedEventArgs *args;
	
	if (closure) {
		args = new CollectionChangedEventArgs (action, new_value, old_value, index);
		closure->OnCollectionChanged (this, args);
		args->unref ();
	}
}


void
DependencyObjectCollection::AddedToCollection (Value *value)
{
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

void
DependencyObjectCollection::RemovedFromCollection (Value *value)
{
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

DependencyObject *
DependencyObjectCollection::SetValueAt (int index, DependencyObject *obj)
{
	Value *value = Collection::SetValueAt (index, Value (obj));
	DependencyObject *old = NULL;
	
	if (value) {
		old = value->AsDependencyObject ();
		old->ref ();
		
		delete value;
	}
	
	return old;
}

void
DependencyObjectCollection::SetSurface (Surface *surface)
{
	DependencyObject *obj;
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		obj = value->AsDependencyObject ();
		obj->SetSurface (surface);
	}
	
	Collection::SetSurface (surface);
}

void
DependencyObjectCollection::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (closure)
		closure->OnCollectionItemChanged (this, obj, subobj_args);
}

void
DependencyObjectCollection::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	DependencyObject *obj;
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		obj = value->AsDependencyObject ();
		obj->UnregisterAllNamesRootedAt (from_ns);
	}
	
	Collection::UnregisterAllNamesRootedAt (from_ns);
}

void
DependencyObjectCollection::RegisterAllNamesRootedAt (NameScope *to_ns)
{
	DependencyObject *obj;
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		obj = value->AsDependencyObject ();
		obj->RegisterAllNamesRootedAt (to_ns);
	}
	
	Collection::RegisterAllNamesRootedAt (to_ns);
}

void
DependencyObjectCollection::MergeNames (DependencyObject *new_obj)
{
	if (!closure)
		return;
	
	NameScope *ns = NameScope::GetNameScope (new_obj);
	
	/* this should always be true for Canvas subclasses */
	if (ns) {
		if (ns->GetTemporary ()) {
			NameScope *con_ns = closure->FindNameScope ();
			if (con_ns) {
				con_ns->MergeTemporaryScope (ns);
				// get rid of the old namescope after we merge
				new_obj->ClearValue (NameScope::NameScopeProperty, false);
			}
		}
	} else {
		NameScope *con_ns = closure->FindNameScope ();
		if (con_ns)
			new_obj->RegisterAllNamesRootedAt (con_ns);
	}
}


void
collection_init (void)
{
	Collection::CountProperty = DependencyProperty::Register (Type::COLLECTION, "Count", Type::INT32);
}

int
collection_get_count (Collection *collection)
{
	return collection->GetCount ();
}

Type::Kind
collection_get_element_type (Collection *collection)
{
	return collection->GetElementType ();
}

int
collection_add (Collection *collection, Value value)
{
	return collection->Add (value);
}

void 
collection_clear (Collection *collection)
{
	collection->Clear ();
}

bool
collection_contains (Collection *collection, Value *value)
{
	return collection->Contains (*value);
}

int
collection_index_of (Collection *collection, Value *value)
{
	return collection->IndexOf (*value);
}

bool
collection_insert (Collection *collection, int index, Value *value)
{
	return collection->Insert (index, *value);
}

bool
collection_remove (Collection *collection, Value *value)
{
	return collection->Remove (*value);
}

bool
collection_remove_at (Collection *collection, int index)
{
	return collection->RemoveAt (index);
}

Value *
collection_get_value_at (Collection *collection, int index)
{
	return collection->GetValueAt (index);
}

void
collection_set_value_at (Collection *collection, int index, Value *value)
{
	Value *v;
	
	if (!(v = collection->SetValueAt (index, *value)))
		return;
	
	delete v;
}


CollectionIterator *
collection_get_iterator (Collection *collection)
{
	return new CollectionIterator (collection);
}

int
collection_iterator_next (CollectionIterator *iterator)
{
	if (iterator->generation != iterator->collection->Generation ())
		return -1;
	
	if (iterator->index >= iterator->collection->GetCount ())
		return 0;
	
	iterator->index++;
	
	return 1;
}

bool
collection_iterator_reset (CollectionIterator *iterator)
{
	if (iterator->generation != iterator->collection->Generation ())
		return false;
	
	iterator->index = -1;
	
	return true;
}

Value *
collection_iterator_get_current (CollectionIterator *iterator, int *error)
{
	if (iterator->generation != iterator->collection->Generation ()) {
		*error = 1;
		return NULL;
	}
	
	if (iterator->index < 0) {
		*error = 1;
		return NULL;
	}
	
	*error = 0;
	
	return iterator->collection->GetValueAt (iterator->index);
}

void
collection_iterator_destroy (CollectionIterator *iterator)
{
	delete iterator;
}



void
TriggerCollection::AddedToCollection (Value *value)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	EventTrigger *trigger = value->AsEventTrigger ();
	
	trigger->SetTarget (fwe);
	
	DependencyObjectCollection::AddedToCollection (value);
}

void
TriggerCollection::RemovedFromCollection (Value *value)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	EventTrigger *trigger = value->AsEventTrigger ();
	
	trigger->RemoveTarget (fwe);
	
	DependencyObjectCollection::RemovedFromCollection (value);
}



Collection *
collection_new (Type::Kind kind)
{
	Type *t = Type::Find (kind);
	
	if (!t->IsSubclassOf (Type::COLLECTION)) {
		g_warning ("create_collection passed non-collection type");
		return NULL;
	}
	
	return (Collection *) t->CreateInstance();
}

DependencyObjectCollection *
dependency_object_collection_new (void)
{
	return new DependencyObjectCollection ();
}

DoubleCollection *
double_collection_new (void)
{
	return new DoubleCollection ();
}

PointCollection *
point_collection_new (void)
{
	return new PointCollection ();
}

TriggerCollection *
trigger_collection_new (void)
{
	return new TriggerCollection ();
}

TriggerActionCollection *
trigger_action_collection_new (void)
{
	return new TriggerActionCollection ();
}

ResourceDictionary *
resource_dictionary_new (void)
{
	return new ResourceDictionary ();
}

Inlines *
inlines_new (void)
{
	return new Inlines ();
}
