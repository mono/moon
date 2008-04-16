/*
 * collection.h: different types of collections
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <gtk/gtk.h>

#include "collection.h"
#include "panel.h"
#include "geometry.h"
#include "media.h"
#include "transform.h"
#include "trigger.h"
#include "namescope.h"
#include "utils.h"

Collection::Node::Node (DependencyObject *dob, DependencyObject *parent)
{
	dob->SetLogicalParent (parent);
	dob->ref ();
	obj = dob;
	this->parent = parent;
}

Collection::Node::~Node ()
{
	obj->SetLogicalParent (NULL);
	obj->unref ();
}

bool
CollectionNodeFinder (List::Node *n, void *data)
{
	Collection::Node *cn = (Collection::Node *) n;
	
	return cn->obj == (DependencyObject *) data;
}

Collection::Collection ()
{
	list = new List ();
	closure = NULL;
	generation = 0;
}

Collection::~Collection ()
{
	Clear (false);
	delete list;
}

void
Collection::EmitChanged (CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	Collection::ChangeEventArgs *args = new Collection::ChangeEventArgs ();
	args->type = type;
	args->obj = obj;
	args->prop = element_args ? element_args->property : NULL;

	if (closure) {
		closure->OnCollectionChanged (this, type, obj, element_args);
	}
}

void
Collection::MergeNames (DependencyObject *new_obj)
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
	}
	else {
		NameScope *con_ns = closure->FindNameScope ();
		if (con_ns)
			new_obj->RegisterAllNamesRootedAt (con_ns);
	}
}

int
Collection::Add (DependencyObject *data)
{
	if (!Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType())) {
		g_warning ("Cannot add children of type `%s' to a collection of type `%s'.  Its children must be subclasses of `%s'.",
			   data->GetTypeName(), GetTypeName(), Type::Find (GetElementType())->GetName ());
		return -1;
	}

	// Make sure we don't have it already
	if (list->Find (CollectionNodeFinder, data)) {
		return -1;
	}

	generation++;
	list->Append (new Collection::Node (data, this));
	data->AddPropertyChangeListener (this);
	data->SetSurface (GetSurface());

	MergeNames (data);

	if (closure)
		EmitChanged (CollectionChangeTypeItemAdded, data, NULL);

	return list->Length () - 1;
}

bool
Collection::Insert (int index, DependencyObject *data)
{
	if (!Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType())) {
		g_warning ("Cannot add children of type `%s' to a collection of type `%s'.  Its children must be subclasses of `%s'.",
			   data->GetTypeName(), GetTypeName(), Type::Find (GetElementType())->GetName ());
		return false;
	}

	if (index < 0)
		return false;
	
	generation++;
	list->Insert (new Collection::Node (data, this), index);

	data->AddPropertyChangeListener (this);
	data->SetSurface (GetSurface());

	MergeNames (data);

	if (closure)
		EmitChanged (CollectionChangeTypeItemAdded, data, NULL);

	return true;
}

void
Collection::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	EmitChanged (CollectionChangeTypeItemChanged, obj, subobj_args);
}

DependencyObject *
Collection::SetVal (int index, DependencyObject *data)
{
	if (!Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType())) {
		g_warning ("Cannot add children of type `%s' to a collection of type `%s'.  Its children must be subclasses of `%s'.",
			   data->GetTypeName(), GetTypeName(), Type::Find (GetElementType())->GetName ());
		return NULL;
	}

	generation++;
	Node *old = (Collection::Node *) list->Replace (new Collection::Node (data, this), index);
	
	data->AddPropertyChangeListener (this);
	data->SetSurface (GetSurface ());
	old->obj->RemovePropertyChangeListener (this);
	old->obj->SetSurface (NULL);

	NameScope *ns = old->obj->FindNameScope();

	if (ns)
		old->obj->UnregisterAllNamesRootedAt (ns);

	if (closure)
		EmitChanged (CollectionChangeTypeItemRemoved, old->obj, NULL);

	MergeNames (data);

	if (closure)
		EmitChanged (CollectionChangeTypeItemAdded, data, NULL);

	DependencyObject *obj = old->obj;
	delete old;
	return obj;
}

Value *
Collection::GetValue (DependencyProperty *prop)
{
	if (prop == CountProperty)
		return new Value (list->Length ());
	
	return DependencyObject::GetValue (prop);
}

bool
Collection::Remove (DependencyObject *data)
{
	Collection::Node *n;
	
	generation++;
	if (!(n = (Collection::Node *) list->Find (CollectionNodeFinder, data)))
		return false;
	
	list->Unlink (n);

	data->RemovePropertyChangeListener (this);
	data->SetSurface (NULL);

	NameScope *ns;

	// first unregister the name from whatever scope it's registered in
	// if it's got its own, don't worry about it.
	ns = NameScope::GetNameScope (data);
	if (!ns) {
		ns = data->FindNameScope ();
		if (ns)
			data->UnregisterAllNamesRootedAt (ns);
	}

	if (closure)
		EmitChanged (CollectionChangeTypeItemRemoved, data, NULL);

	delete n;
	return true;
}

bool
Collection::RemoveAt (int index)
{
	Collection::Node *n = (Collection::Node *) list->Index (index);
	
	if (n == NULL)
		return false;
	
	generation++;

	list->Unlink (n);
	
	n->obj->RemovePropertyChangeListener (this);
	n->obj->SetSurface (NULL);

	NameScope *ns;

	// first unregister the name from whatever scope it's registered in
	// if it's got its own, don't worry about it.
	ns = n->obj->FindNameScope ();
	if (!ns) {
		ns = n->obj->FindNameScope ();
		if (ns)
			n->obj->UnregisterAllNamesRootedAt (ns);
	}

	if (closure)
		EmitChanged (CollectionChangeTypeItemRemoved, n->obj, NULL);
	
	delete n;
	return true;
}

void
Collection::Clear (bool emit_event)
{
	Collection::Node *n;
	NameScope *con_ns = NULL;
	generation++;


	if (emit_event)
		EmitChanged (CollectionChangeTypeChanging, NULL, NULL);

	if (closure)
		con_ns = closure->FindNameScope ();
	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next) {
		n->obj->RemovePropertyChangeListener (this);
		n->obj->SetSurface (NULL);

		NameScope *ns = NameScope::GetNameScope (n->obj);
		if (!ns) {
			if (con_ns)
				n->obj->UnregisterAllNamesRootedAt (con_ns);
		}
	}

	list->Clear (true);

	if (emit_event)
		EmitChanged (CollectionChangeTypeChanged, NULL, NULL);
}

void
Collection::Clear ()
{
	Clear(true);
}

void
Collection::SetSurface (Surface *surface)
{
	Collection::Node *n;

	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next) {
		n->obj->SetSurface (surface);
	}

	DependencyObject::SetSurface (surface);
}

void
Collection::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	Collection::Node *n;

	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next) {
		n->obj->UnregisterAllNamesRootedAt (from_ns);
	}

	DependencyObject::UnregisterAllNamesRootedAt (from_ns);
}

void
Collection::RegisterAllNamesRootedAt (NameScope *to_ns)
{
	Collection::Node *n;

	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next) {
		n->obj->RegisterAllNamesRootedAt (to_ns);
	}

	DependencyObject::RegisterAllNamesRootedAt (to_ns);
}

DependencyProperty *Collection::CountProperty;

void
collection_init (void)
{
	Collection::CountProperty = DependencyObject::Register (Type::COLLECTION, "Count", Type::INT32);
}

bool
collection_add (Collection *collection, DependencyObject *data)
{
	return collection->Add (data);
}

int
collection_count (Collection *collection)
{
	return collection->list->Length ();
}

DependencyObject *
collection_get_value_at (Collection *collection, int index)
{
	Collection::Node *node = (Collection::Node *) collection->list->Index (index);
	
	if (node == NULL)
		return NULL;

	return node->obj;
}

void
collection_set_value_at (Collection *collection, int index, DependencyObject *dob)
{
	collection->SetVal (index, dob);
}

int
collection_get_index_of (Collection *collection, DependencyObject *dob)
{
	return collection->list->IndexOf (CollectionNodeFinder, dob);
}

bool
collection_remove (Collection *collection, DependencyObject *data)
{
	return collection->Remove (data);
}

bool
collection_remove_at (Collection *collection, int index)
{
	if (index > collection->list->Length ())
		return false;
	
	collection->RemoveAt (index);
	return true;
}

bool
collection_insert (Collection *collection, int index, DependencyObject *data)
{
	return collection->Insert (index, data);
}

//
// Very trivial implementation for now
//
void 
collection_clear (Collection *collection)
{
	collection->Clear ();
}

Type::Kind
collection_get_element_type (Collection *collection)
{
	return collection->GetElementType ();
}

CollectionIterator *
collection_get_iterator (Collection *collection)
{
	return new CollectionIterator (collection);
}

int
collection_iterator_move_next (CollectionIterator *iterator)
{
	if (iterator->collection->generation != iterator->generation)
		return -1;

	if (!iterator->current)
		return 0;

	if (iterator->first) {
		/* we don't actually move the iterator forward here.  it's
		   initialized in the ctor to point to the first element */
		iterator->first = false;
		return 1;
	}

	List::Node *next = iterator->current->next;

	if (next == NULL)
		return 0;
	
	iterator->current = next;

	return 1;
}

bool
collection_iterator_reset (CollectionIterator *iterator)
{
	if (iterator->collection->generation != iterator->generation)
		return false;

	iterator->current = iterator->collection->list->First ();
	iterator->first = true;
	return true;
}

DependencyObject *
collection_iterator_get_current (CollectionIterator *iterator, int *error)
{
	if (iterator->collection->generation != iterator->generation) {
		*error = 1;
		return NULL;
	}
	error = 0;
	if (iterator->current == NULL)
		return NULL;
	
	return ((Collection::Node *) iterator->current)->obj;
}

void
collection_iterator_destroy (CollectionIterator *iterator)
{
	delete iterator;
}


VisualCollection::VisualCollection ()
{
	z_sorted = g_ptr_array_new ();
}

VisualCollection::~VisualCollection ()
{
	Collection::Node *n = (Collection::Node *) list->First ();
	
	g_ptr_array_free (z_sorted, true);
	
	while (n) {
		((UIElement *) n->obj)->SetVisualParent (NULL);
		
		n = (Collection::Node *) n->next;
	}
}

static int
UIElementZIndexComparer (gconstpointer ui1, gconstpointer ui2)
{
	int z1 = (*((UIElement **) ui1))->GetValue (UIElement::ZIndexProperty)->AsInt32 ();
	int z2 = (*((UIElement **) ui2))->GetValue (UIElement::ZIndexProperty)->AsInt32 ();
	
	return z1 - z2;
}

void
VisualCollection::ResortByZIndex ()
{
	Collection::Node *node;
	uint i = 0;
	
	if (z_sorted->len < 1)
		return;
	
	node = (Collection::Node *) list->First ();
	while (node) {
		z_sorted->pdata[i++] = node->obj;
		node = (Collection::Node *) node->next;
	}
	
	g_ptr_array_sort (z_sorted, UIElementZIndexComparer);
}

int
VisualCollection::Add (DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	int n = Collection::Add (item);

	if (n != -1) {
		g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);

		if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
			/* emit loaded events on the new item if the tree
			   we're adding it to has already been "loaded" */
			item->OnLoaded ();
		}
	}
	
	return n;
}

DependencyObject *
VisualCollection::SetVal (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	UIElement *old = (UIElement *) Collection::SetVal (index, item);
	
	if (old) {
		g_ptr_array_remove (z_sorted, old);
	
		g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);
	
		if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
			/* emit loaded events on the new item if the tree
			   we're adding it to has already been "loaded" */
			item->OnLoaded ();
		}
	}

	return old;
}

bool
VisualCollection::Insert (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	if (!Collection::Insert (index, item))
		return false;
	
	g_ptr_array_set_size (z_sorted, z_sorted->len + 1);
	ResortByZIndex ();
	
	if (closure && ((UIElement *)closure)->flags & UIElement::IS_LOADED) {
		/* emit loaded events on the new item if the tree
		   we're adding it to has already been "loaded" */
		item->OnLoaded ();
	}
	
	return true;
}

bool
VisualCollection::Remove (DependencyObject *data)
{
	if (!list->Find (CollectionNodeFinder, data))
		return false;

	UIElement *item = (UIElement *) data;

	if (Collection::Remove (item)) {
		g_ptr_array_remove (z_sorted, item);
		return true;
	}
	
	return false;
}

bool
VisualCollection::RemoveAt (int index)
{
	Collection::Node *n = (Collection::Node *) list->Index (index);
	if (n == NULL)
		return false;
	
	UIElement *item = (UIElement *) n->obj;

	if (Collection::RemoveAt (index)) {
		g_ptr_array_remove (z_sorted, item);
		return true;
	}
	
	return false;
}

void
VisualCollection::Clear ()
{
	Collection::Clear ();
	g_ptr_array_set_size (z_sorted, 0);
}


int
TriggerCollection::Add (DependencyObject *data)
{
	int n = Collection::Add (data);

	if (n != -1) {
		FrameworkElement *fwe = (FrameworkElement *) closure;
		EventTrigger *trigger = (EventTrigger *) data;

		trigger->SetTarget (fwe);
	}

	return n;
}

DependencyObject *
TriggerCollection::SetVal (int index, DependencyObject *data)
{
	FrameworkElement *old = (FrameworkElement *) Collection::SetVal (index, data);

	if (old) {
		FrameworkElement *fwe = (FrameworkElement *) closure;
		EventTrigger *trigger = (EventTrigger *) data;

		trigger->SetTarget (fwe);

		trigger->RemoveTarget (old);
	}

	return old;
}

bool
TriggerCollection::Insert (int index, DependencyObject *data)
{
	bool b = Collection::Insert (index, data);
	if (b) {
		FrameworkElement *fwe = (FrameworkElement *) closure;
		EventTrigger *trigger = (EventTrigger *) data;

		trigger->SetTarget (fwe);
	}

	return b;
}

bool
TriggerCollection::Remove (DependencyObject *data)
{
	bool b = Collection::Remove (data);

	if (b) {
		FrameworkElement *fwe = (FrameworkElement *) closure;
		EventTrigger *trigger = (EventTrigger *) data;

		trigger->RemoveTarget (fwe);
	}

	return b;
}

bool
TriggerCollection::RemoveAt (int index)
{
	Collection::Node *n = (Collection::Node *) list->Index (index);
	if (n == NULL)
		return false;

	bool b = Collection::RemoveAt (index);

	if (b) {
		FrameworkElement *fwe = (FrameworkElement *) closure;
		EventTrigger *trigger = (EventTrigger *) n->obj;
		trigger->RemoveTarget (fwe);
	}

	return b;
}

static bool
media_attribute_by_name_finder (List::Node *node, void *data)
{
	Collection::Node *cn = (Collection::Node *) node;
	MediaAttribute *attribute = (MediaAttribute *) cn->obj;
	const char *name = (const char *) data;

	Value *value = attribute->GetValue (DependencyObject::NameProperty);
	if (!value)
		return false;

	return !strcmp (name, value->AsString ());
}

MediaAttribute *
MediaAttributeCollection::GetItemByName (const char *name)
{
	Collection::Node *cn = (Collection::Node *) list->Find (media_attribute_by_name_finder, (char *) name);
	if (!cn)
		return NULL;

	return (MediaAttribute *) cn->obj;
}

Collection *
collection_new (Type::Kind kind)
{
	Type *t = Type::Find (kind);
	if (!t->IsSubclassOf (Type::COLLECTION)) {
	    g_warning ("create_collection passed non-collection type");
	    return NULL;
	}

	return (Collection*)t->CreateInstance();
}


VisualCollection *
visual_collection_new (void)
{
	return new VisualCollection ();
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




TimelineMarkerCollection *
timeline_marker_collection_new (void)
{
	return new TimelineMarkerCollection ();
}

MediaAttributeCollection *
media_attribute_collection_new (void)
{
	return new MediaAttributeCollection ();
}

MediaAttribute *
media_attribute_collection_get_item_by_name (MediaAttributeCollection *collection, const char *name)
{
	return collection->GetItemByName (name);
}

Inlines *
inlines_new (void)
{
	return new Inlines ();
}


