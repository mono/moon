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
#include <glib.h>
#include <errno.h>

#include "collection.h"
#include "geometry.h"
#include "transform.h"
#include "frameworkelement.h"
#include "namescope.h"
#include "utils.h"
#include "error.h"

Collection::Collection ()
{
	array = g_ptr_array_new ();
	generation = 0;
}

Collection::~Collection ()
{
	g_ptr_array_free (array, true);
}

void
Collection::Dispose ()
{
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		RemovedFromCollection (value);
		delete value;
	}
}

void
Collection::SetCount (int count)
{
	SetValue (Collection::CountProperty, Value (count));
}

int
Collection::GetCount ()
{
	return GetValue (Collection::CountProperty)->AsInt32 ();
}

int
Collection::Add (Value *value)
{
	bool rv = Insert (array->len, value);
	return rv ? array->len - 1 : -1;
}

int
Collection::Add (Value value)
{
	return Add (&value);
}

void
Collection::Clear ()
{
	EmitChanged (CollectionChangedActionClearing, NULL, NULL, -1);

	guint len = array->len;
	Value** vals = new Value*[len];
	memmove (vals, array->pdata, len * sizeof(Value*));

	g_ptr_array_set_size (array, 0);
	generation++;
	
	SetCount (0);

	for (guint i = 0; i < len; i++) {
		RemovedFromCollection (vals[i]);
		delete vals[i];
	}
	delete[] vals;
	
	EmitChanged (CollectionChangedActionCleared, NULL, NULL, -1);
}

bool
Collection::Contains (Value *value)
{
	// make sure value is of the proper type
	if (!value->Is (GetElementType ()))
		return false;
	
	return IndexOf (value) != -1;
}

int
Collection::IndexOf (Value *value)
{
	Value *v;
	
	// make sure value is of the proper type
	if (!value->Is (GetElementType ()))
		return -1;
	
	for (guint i = 0; i < array->len; i++) {
		v = (Value *) array->pdata[i];
		if (*v == *value)
			return i;
	}
	
	return -1;
}

bool
Collection::Insert (int index, Value value)
{
	return Insert (index, &value);
}

bool
Collection::Insert (int index, Value *value)
{
	Value *added;
	
	// make sure value is of the proper type
	if (!value->Is (GetElementType ()))
		return false;
	
	// bounds check
	if (index < 0)
		return false;
	
	// Check that the item can be added to our collection
	if (!CanAdd (value))
		return false;
	
	if (index > GetCount ())
		index = GetCount ();
	
	added = new Value (*value);
	g_ptr_array_insert (array, index, added);
	AddedToCollection (added);
	
	SetCount ((int) array->len);
	
	EmitChanged (CollectionChangedActionAdd, added, NULL, index);
	
	return true;
}

bool
Collection::Remove (Value value)
{
	return Remove (&value);
}

bool
Collection::Remove (Value *value)
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
	
	g_ptr_array_remove_index (array, index);
	SetCount ((int) array->len);
	generation++;
	
	RemovedFromCollection (value);
	
	EmitChanged (CollectionChangedActionRemove, NULL, value, index);
	
	delete value;
	
	return true;
}

bool
Collection::RemoveAtWithError (int index, MoonError *error)
{
	// check bounds
	if (index < 0 || (guint) index >= array->len) {
		MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, "");
		return false;
	}
	
	return RemoveAt (index);
}

Value *
Collection::GetValueAt (int index)
{
	if (index < 0 || (guint) index >= array->len)
		return NULL;
	
	return (Value *) array->pdata[index];
}

Value *
Collection::GetValueAtWithError (int index, MoonError *error)
{
	// check array bounds
	if (index < 0 || (guint) index >= array->len) {
		MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, "");
		return NULL;
	}
	
	return GetValueAt (index);
}

bool
Collection::SetValueAt (int index, Value *value)
{
	Value *added, *removed;
	
	// make sure the object is of the correct type
	if (!value->Is (GetElementType ()))
		return false;
	
	// check array bounds
	if (index < 0 || (guint) index >= array->len)
		return false;
	
	// Check that the value can be added to our collection
	if (!CanAdd (value))
		return false;
	
	removed = (Value *) array->pdata[index];
	added = new Value (*value);
	
	array->pdata[index] = added;
	
	RemovedFromCollection (removed);
	AddedToCollection (added);
	
	EmitChanged (CollectionChangedActionReplace, added, removed, index);
	
	delete removed;
	
	return true;
}

bool
Collection::SetValueAtWithError (int index, Value *value, MoonError *error)
{
	// make sure the object is of the correct type
	if (!value->Is (GetElementType ())) {
		MoonError::FillIn (error, MoonError::ARGUMENT, "");
		return false;
	}
	
	// check array bounds
	if (index < 0 || (guint) index >= array->len) {
		MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, "");
		return false;
	}
	
	// Check that the value can be added to our collection
	if (!CanAdd (value)) {
		MoonError::FillIn (error, MoonError::ARGUMENT, "");
		return false;
	}
	
	return SetValueAt (index, value);
}

void
Collection::EmitChanged (CollectionChangedAction action, Value *new_value, Value *old_value, int index)
{
	CollectionChangedEventArgs *args;
	
	if (GetLogicalParent()) {
		args = new CollectionChangedEventArgs (action, new_value, old_value, index);
		GetLogicalParent()->OnCollectionChanged (this, args);
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
	if (GetLogicalParent())
		GetLogicalParent()->OnCollectionItemChanged (this, obj, subobj_args);
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



UIElementCollection::UIElementCollection ()
{
	z_sorted = g_ptr_array_new ();
}

UIElementCollection::~UIElementCollection ()
{
	g_ptr_array_free (z_sorted, true);
}

static int
UIElementZIndexComparer (gconstpointer ui1, gconstpointer ui2)
{
	int z1 = (*((UIElement **) ui1))->GetZIndex ();
	int z2 = (*((UIElement **) ui2))->GetZIndex ();
	
	return z1 - z2;
}

void
UIElementCollection::ResortByZIndex ()
{
	g_ptr_array_set_size (z_sorted, array->len);
	
	if (array->len == 0)
		return;
	
	for (guint i = 0; i < array->len; i++)
		z_sorted->pdata[i] = ((Value *) array->pdata[i])->AsUIElement ();
	
	if (array->len > 1)
		g_ptr_array_sort (z_sorted, UIElementZIndexComparer);
}

void
UIElementCollection::Clear ()
{
	g_ptr_array_set_size (z_sorted, 0);
	DependencyObjectCollection::Clear ();
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
	
	iterator->index++;
	
	if (iterator->index >= iterator->collection->GetCount ())
		return 0;
	
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

DoubleCollection *
double_collection_from_str (const char *s)
{
	GArray *values = double_garray_from_str (s, 0);

	if (values->len == 0) {
		g_array_free (values, true);
		return NULL;
	}

	DoubleCollection *doubles = new DoubleCollection ();
	for (guint i = 0; i < values->len; i ++)
		doubles->Add (g_array_index (values, double, i));
	g_array_free (values, true);

	return doubles;
}

PointCollection *
point_collection_from_str (const char *s)
{
	int i, j, n = 0;
	GArray *values = double_garray_from_str (s, 0);

	n = values->len / 2;
	if (n == 0 || n % 1 == 1) {
		g_array_free (values, true);
		return NULL;
	}

	PointCollection *points = new PointCollection();
	for (i = 0, j = 0; j < n; j++) {
		double x = g_array_index (values, double, i++);
		double y = g_array_index (values, double, i++);

		points->Add (Point (x, y));
	}

	g_array_free (values, true);
	return points;
}

GArray *double_garray_from_str (const char *s, gint max)
{
	char *next = (char *)s;
	GArray *values = g_array_sized_new (false, true, sizeof (double), max > 0 ? max : 16);
	double coord = 0.0;
	guint end = max > 0 ? max : G_MAXINT;

	while (next && values->len < end) {
		while (g_ascii_isspace (*next) || *next == ',')
			next = g_utf8_next_char (next);
		
		if (next) {
			errno = 0;
			char *prev = next;
			coord = g_ascii_strtod (prev, &next);
			if (errno != 0 || next == prev)
				goto error;

			g_array_append_val (values, coord);
		}
	}

error:
	while (values->len < (guint) max) {
		coord = 0.0;
		g_array_append_val (values, coord);
	}

	return values;
}
