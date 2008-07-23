/*
 * collection.h: different types of collections
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
	Collection::ChangeEventArgs *args;
	
	if (closure) {
		args = new Collection::ChangeEventArgs ();
		args->type = type;
		args->obj = obj;
		args->prop = element_args ? element_args->property : NULL;

		closure->OnCollectionChanged (this, type, obj, element_args);
		args->unref ();
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
	int result;
	
	if (!Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType())) {
		g_warning ("Cannot add children of type `%s' to a collection of type `%s'.  Its children must be subclasses of `%s'.",
			   data->GetTypeName(), GetTypeName(), Type::Find (GetElementType())->GetName ());
		return -1;
	}

	// Make sure we don't have it already
	if (list->Find (CollectionNodeFinder, data)) {
		return -1;
	}

	// do this *before* creating the Collection::Node, since that
	// sets the logical parent on data, and we need to
	// differentiate the two cases in Storyboard::SetSurface.
	data->SetSurface (GetSurface());

	generation++;
	result = AddToList (new Collection::Node (data, this));
	
	data->AddPropertyChangeListener (this);

	MergeNames (data);

	if (closure)
		EmitChanged (CollectionChangeTypeItemAdded, data, NULL);

	return result;
}

int
Collection::AddToList (Collection::Node *node)
{
	list->Append (node);
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

int
Collection::GetCount ()
{
	return list->Length ();
}

void
collection_init (void)
{
	Collection::CountProperty = DependencyProperty::Register (Type::COLLECTION, "Count", Type::INT32);
}

int
collection_add (Collection *collection, DependencyObject *data)
{
	return collection->Add (data);
}

int
collection_get_count (Collection *collection)
{
	return collection->GetCount ();
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


