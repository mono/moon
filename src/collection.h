/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * collection.h: different types of collections
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_COLLECTION_H__
#define __MOON_COLLECTION_H__

#include <glib.h>

#include "dependencyobject.h"
#include "eventargs.h"
#include "point.h"


//
// Collection: provides a collection that we can monitor for
// changes.   We expose this collection in a few classes to
// the managed world, and when a change happens we get a
// chance to reflect the changes
//
class Collection : public DependencyObject {
 protected:
	GPtrArray *array;
	int generation;
	bool unique;
	
	void EmitChanged (CollectionChangedAction action, Value *new_value, Value *old_value, int index);
	
	virtual void AddedToCollection (Value *value) {}
	virtual void RemovedFromCollection (Value *value) {}
	
	Collection ();
	virtual ~Collection ();
	
 public:
	static DependencyProperty *CountProperty;
	
	DependencyObject *closure;
	
	virtual Type::Kind GetObjectType () = 0;
	virtual Type::Kind GetElementType () = 0;
	
	virtual Value *GetValue (DependencyProperty *property);
	
	int Generation () { return generation; }
	GPtrArray *Array () { return array; }
	
	int GetCount () { return array->len; }
	
	virtual int Add (Value value);
	virtual void Clear ();
	bool Contains (Value value);
	int IndexOf (Value value);
	virtual bool Insert (int index, Value value);
	bool Remove (Value value);
	bool RemoveAt (int index);
	
	Value *GetValueAt (int index);
	Value *SetValueAt (int index, Value value);
};

class DependencyObjectCollection : public Collection {
 protected:
	virtual void AddedToCollection (Value *value);
	virtual void RemovedFromCollection (Value *value);
	
	virtual ~DependencyObjectCollection () {}
	
 public:
	DependencyObjectCollection () { unique = true; }
	
	virtual Type::Kind GetObjectType () { return Type::DEPENDENCY_OBJECT_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }
	
	virtual int Add (Value value) { return Collection::Add (value); }
	virtual void Clear () { Collection::Clear (); }
	virtual bool Insert (int index, Value value) { return Collection::Insert (index, value); }
	
	// Convenience wrappers
	int Add (DependencyObject *value) { return Collection::Add (Value (value)); }
	bool Contains (DependencyObject *value) { return Collection::Contains (Value (value)); }
	int IndexOf (DependencyObject *value) { return Collection::IndexOf (Value (value)); }
	bool Insert (int index, DependencyObject *value) { return Collection::Insert (index, Value (value)); }
	bool Remove (DependencyObject *value) { return Collection::Remove (Value (value)); }
	virtual DependencyObject *SetValueAt (int index, DependencyObject *obj);
	
	virtual void SetSurface (Surface *surface);
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns);
	
	void MergeNames (DependencyObject *new_obj);
};

class DoubleCollection : public Collection {
 protected:
	virtual ~DoubleCollection () {}
	
 public:
	DoubleCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::DOUBLE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::DOUBLE; }
	
	virtual int Add (Value value) { return Collection::Add (value); }
	virtual void Clear () { Collection::Clear (); }
	virtual bool Insert (int index, Value value) { return Collection::Insert (index, value); }
	
	// Convenience wrappers
	int Add (double value) { return Collection::Add (Value (value)); }
	bool Contains (double value) { return Collection::Contains (Value (value)); }
	int IndexOf (double value) { return Collection::IndexOf (Value (value)); }
	bool Insert (int index, double value) { return Collection::Insert (index, Value (value)); }
	bool Remove (double value) { return Collection::Remove (Value (value)); }
};

class PointCollection : public Collection {
 protected:
	virtual ~PointCollection () {}
	
 public:
	PointCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::POINT_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::POINT; }
	
	virtual int Add (Value value) { return Collection::Add (value); }
	virtual void Clear () { Collection::Clear (); }
	virtual bool Insert (int index, Value value) { return Collection::Insert (index, value); }
	
	// Convenience wrappers
	int Add (Point value) { return Collection::Add (Value (value)); }
	bool Contains (Point value) { return Collection::Contains (Value (value)); }
	int IndexOf (Point value) { return Collection::IndexOf (Value (value)); }
	bool Insert (int index, Point value) { return Collection::Insert (index, Value (value)); }
	bool Remove (Point value) { return Collection::Remove (Value (value)); }
};

class CollectionIterator {
 public:
	CollectionIterator (Collection *c)
	{
		generation = c->Generation ();
		collection = c;
		index = -1;
	}
	
	Collection *collection;
	int generation;
	int index;
};

class TriggerCollection : public DependencyObjectCollection {
 protected:
	virtual void AddedToCollection (Value *value);
	virtual void RemovedFromCollection (Value *value);
	
	virtual ~TriggerCollection () {}
	
 public:
	TriggerCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::TRIGGER_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::EVENTTRIGGER; }
};

class TriggerActionCollection : public DependencyObjectCollection {
 protected:
	virtual ~TriggerActionCollection () {}

 public:
	TriggerActionCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TRIGGERACTION_COLLECTION; }
	/* this may seem wrong, but it's what the TriggerActionCollection mandates */
	virtual Type::Kind GetElementType () { return Type::BEGINSTORYBOARD; }
};

class ResourceDictionary : public DependencyObjectCollection {
 protected:
	virtual ~ResourceDictionary () {}
	
 public:
	ResourceDictionary () {}
	virtual Type::Kind GetObjectType () { return Type::RESOURCE_DICTIONARY; }
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }
};

class Inlines : public DependencyObjectCollection {
 protected:
	virtual ~Inlines () {}

 public:
	Inlines () {}
	virtual Type::Kind GetObjectType () { return Type::INLINES; }
	virtual Type::Kind GetElementType () { return Type::INLINE; }
};

G_BEGIN_DECLS

void collection_init (void);

Collection *collection_new (Type::Kind kind);

Type::Kind collection_get_element_type (Collection *collection);
int collection_get_count (Collection *collection);

int collection_add (Collection *collection, Value *value);
void collection_clear (Collection *collection);
bool collection_contains (Collection *collection, Value *value);
int collection_index_of (Collection *collection, Value *value);
bool collection_insert (Collection *collection, int index, Value *value);
bool collection_remove (Collection *collection, Value *value);
bool collection_remove_at (Collection *collection, int index);

Value *collection_get_value_at (Collection *collection, int index);
void collection_set_value_at (Collection *collection, int index, Value *value);

CollectionIterator *collection_get_iterator (Collection *collection);
int collection_iterator_next (CollectionIterator *iterator);
bool collection_iterator_reset (CollectionIterator *iterator);
void collection_iterator_destroy (CollectionIterator *iterator);
Value *collection_iterator_get_current (CollectionIterator *iterator, int *error);

DependencyObjectCollection *dependency_object_collection_new (void);
DoubleCollection *double_collection_new (void);
PointCollection *point_collection_new (void);
TriggerCollection *trigger_collection_new (void);
TriggerActionCollection *trigger_action_collection_new (void);
ResourceDictionary *resource_dictionary_new (void);
Inlines *inlines_new (void);

G_END_DECLS

#endif /* __MOON_COLLECTION_H__ */
