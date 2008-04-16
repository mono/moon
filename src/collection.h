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

#include "dependencyobject.h"
#include "utils.h"
#include "list.h"
#include "eventargs.h"

//
// Collection: provides a collection that we can monitor for
// changes.   We expose this collection in a few classes to
// the managed world, and when a change happens we get a
// chance to reflect the changes
//
class Collection : public DependencyObject {
 protected:
	virtual ~Collection ();

 public:
	class ChangeEventArgs : public EventArgs {
	public:
		CollectionChangeType type;
		DependencyObject *obj;
		DependencyProperty *prop;
		virtual Type::Kind GetObjectType () { return Type::CHANGEEVENTARGS; };
	};

	class Node : public List::Node {
	public:
		DependencyObject *obj, *parent;
		
		Node (DependencyObject *dob, DependencyObject *parent);
		virtual ~Node ();
	};

	int generation;
	
	List *list;
	DependencyObject *closure;
	
	Collection ();
	virtual Type::Kind GetObjectType () { return Type::COLLECTION; };	
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }

	virtual int  Add    (DependencyObject *data);
	virtual bool Remove (DependencyObject *data);
	virtual bool RemoveAt (int index);
	virtual bool Insert (int index, DependencyObject *data);
	virtual void Clear  ();

	virtual void SetSurface (Surface *surface);

	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns);

	void MergeNames (DependencyObject *new_obj);

	// non virtual version callable from the dtor.
	void Clear (bool emit_event);

	//
	// Returns the old value
	//
	virtual DependencyObject *SetVal (int index, DependencyObject *data);
	virtual Value *GetValue (DependencyProperty *property);
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

	void EmitChanged (CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);

	static DependencyProperty *CountProperty;
};

bool CollectionNodeFinder (List::Node *n, void *data);

class CollectionIterator {
 public:
	CollectionIterator (Collection *c) {
		first = true;
		collection = c;
		current = c->list->First ();
		generation = c->generation;
	}

	bool first;
	int generation;
	Collection *collection;
	List::Node *current;
};

class VisualCollection : public Collection {
 protected:
	virtual ~VisualCollection ();

 public:
	VisualCollection ();
	virtual Type::Kind GetObjectType () { return Type::VISUAL_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::VISUAL; }

	virtual int  Add    (DependencyObject *data);
	virtual bool Remove (DependencyObject *data);
	virtual bool RemoveAt (int index);
	virtual bool Insert (int index, DependencyObject *data);
	virtual void Clear  ();
	virtual DependencyObject *SetVal (int index, DependencyObject *data);

	void ResortByZIndex ();
	GPtrArray *z_sorted;
};

class TriggerCollection : public Collection {
 protected:
	virtual ~TriggerCollection () {}

 public:
	TriggerCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TRIGGER_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::EVENTTRIGGER; }

	virtual int  Add    (DependencyObject *data);
	virtual bool Remove (DependencyObject *data);
	virtual bool RemoveAt (int index);
	virtual bool Insert (int index, DependencyObject *data);
	virtual DependencyObject *SetVal (int index, DependencyObject *data);
};

class TriggerActionCollection : public Collection {
 protected:
	virtual ~TriggerActionCollection () {}

 public:
	TriggerActionCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TRIGGERACTION_COLLECTION; }
	/* this may seem wrong, but it's what the TriggerActionCollection mandates */
	virtual Type::Kind GetElementType () { return Type::BEGINSTORYBOARD; }
};

class ResourceDictionary : public Collection {
 protected:
	virtual ~ResourceDictionary () {}

 public:
	ResourceDictionary () {}
	virtual Type::Kind GetObjectType () { return Type::RESOURCE_DICTIONARY; }
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }
};

class TimelineMarkerCollection : public Collection {
 protected:
	virtual ~TimelineMarkerCollection () {}

 public:
	TimelineMarkerCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TIMELINEMARKER_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::TIMELINEMARKER; }
};

class MediaAttributeCollection : public Collection {
 protected:
	virtual ~MediaAttributeCollection () {}

public:
	MediaAttributeCollection () {}
	virtual Type::Kind GetObjectType () { return Type::MEDIAATTRIBUTE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::MEDIAATTRIBUTE; }

	MediaAttribute *GetItemByName (const char *name);
};

class Inlines : public Collection {
 protected:
	virtual ~Inlines () {}

 public:
	Inlines () {}
	virtual Type::Kind GetObjectType () { return Type::INLINES; }
	virtual Type::Kind GetElementType () { return Type::INLINE; }
};

G_BEGIN_DECLS

bool collection_add    (Collection *collection, DependencyObject *data);
bool collection_remove (Collection *collection, DependencyObject *data);
bool collection_remove_at (Collection *collection, int index);
bool collection_insert (Collection *collection, int index, DependencyObject *data);
void collection_clear  (Collection *collection);
int  collection_count  (Collection *collection);

DependencyObject   *collection_get_value_at (Collection *collection, int index);
void                collection_set_value_at (Collection *collection, int index, DependencyObject *obj);
Type::Kind          collection_get_element_type (Collection *collection);
CollectionIterator *collection_get_iterator (Collection *collection);
int                 collection_get_index_of (Collection *collection, DependencyObject *obj);

int    collection_iterator_move_next   (CollectionIterator *iterator);
bool   collection_iterator_reset       (CollectionIterator *iterator);
void   collection_iterator_destroy     (CollectionIterator *iterator);
DependencyObject *collection_iterator_get_current (CollectionIterator *iterator, int *error);


Collection *collection_new (Type::Kind kind);

VisualCollection *visual_collection_new (void);
TriggerCollection *trigger_collection_new (void);
TriggerActionCollection *trigger_action_collection_new (void);
ResourceDictionary *resource_dictionary_new (void);
TimelineMarkerCollection *timeline_marker_collection_new (void);
GradientStopCollection *gradient_stop_collection_new (void);
Inlines *inlines_new (void);

MediaAttributeCollection *media_attribute_collection_new (void);
MediaAttribute *media_attribute_collection_get_item_by_name (MediaAttributeCollection *collection, const char *name);

void collection_init (void);

G_END_DECLS

#endif /* __MOON_COLLECTION_H__ */
