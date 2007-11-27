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

#include "garray-ext.h"
#include "collection.h"
#include "panel.h"
#include "geometry.h"
#include "brush.h"
#include "animation.h"
#include "transform.h"
#include "namescope.h"

Collection::Node::Node (DependencyObject *dob, DependencyObject *parent)
{
	dob->Attach (NULL, parent);
	dob->SetParent (parent);
	dob->ref ();
	obj = dob;
	this->parent = parent;
}

Collection::Node::~Node ()
{
	obj->Detach (NULL, parent);
	obj->SetParent (NULL);
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
	Collection::Node *n;

	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next)
		n->obj->Detach (NULL, this);

	list->Clear(true);
	delete list;
}

bool
Collection::Add (DependencyObject *data)
{
	if (!Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType())) {
		g_warning ("Cannot add children of type `%s' to a collection of type `%s'.  Its children must be subclasses of `%s'.",
			   data->GetTypeName(), GetTypeName(), Type::Find (GetElementType())->name);
		return false;
	}
	
	generation++;
	list->Append (new Collection::Node (data, this));
	data->Attach (NULL, this);

	if (closure) {
		NameScope *ns = NameScope::GetNameScope (data);
		/* this should always be true for Canvas subclasses */
		if (ns) {
			if (ns->GetTemporary ()) {
				NameScope *con_ns = closure->FindNameScope ();
				if (con_ns)
					con_ns->MergeTemporaryScope (ns);
			}
		}
		else {
			const char *n = data->GetName();
			NameScope *con_ns = closure->FindNameScope ();
			if (con_ns && n) {
				con_ns->RegisterName (n, data);
			}
		}

		closure->OnCollectionChanged (this, CollectionChangeTypeItemAdded, data, NULL);
	}

	return true;
}

bool
Collection::Insert (int index, DependencyObject *data)
{
	if (!Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType())) {
		g_warning ("Cannot add children of type `%s' to a collection of type `%s'.  Its children must be subclasses of `%s'.",
			   data->GetTypeName(), GetTypeName(), Type::Find (GetElementType())->name);
		return false;
	}
	
	generation++;
	list->Insert (new Collection::Node (data, this), index);

	data->Attach (NULL, this);

	if (closure) {
		NameScope *ns = NameScope::GetNameScope (data);
		if (ns && ns->GetTemporary ()) {
			NameScope *con_ns = closure->FindNameScope ();
			if (con_ns)
				con_ns->MergeTemporaryScope (ns);
		}

		closure->OnCollectionChanged (this, CollectionChangeTypeItemAdded, data, NULL);
	}

	return true;
}

void
Collection::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop)
{
	if (closure) {
		/* unfortunately OnSubPropertyChanged doesn't give us
		   enough info to fill in the obj parameter here */
		closure->OnCollectionChanged (this, CollectionChangeTypeItemChanged, obj, subprop);
	}
}

DependencyObject *
Collection::SetVal (int index, DependencyObject *data)
{
	if (!Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType())) {
		g_warning ("Cannot add children of type `%s' to a collection of type `%s'.  Its children must be subclasses of `%s'.",
			   data->GetTypeName(), GetTypeName(), Type::Find (GetElementType())->name);
		return NULL;
	}

	generation++;
	Node *old = (Collection::Node *) list->Replace (new Collection::Node (data, this), index);
	
	data->Attach (NULL, this);
	old->obj->Detach (NULL, this);
	
	if (closure) {
		closure->OnCollectionChanged (this, CollectionChangeTypeItemRemoved, old->obj, NULL);
		closure->OnCollectionChanged (this, CollectionChangeTypeItemAdded, data, NULL);
	}

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

	data->Detach (NULL, this);

	if (closure) {
		NameScope *ns = NameScope::GetNameScope (data);
		if (ns && ns->GetMerged ()) {
			NameScope *con_ns = closure->FindNameScope();
			if (con_ns)
				con_ns->UnmergeTemporaryScope (ns);
		}

		closure->OnCollectionChanged (this, CollectionChangeTypeItemRemoved, data, NULL);
	}
	
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
	
	n->obj->Detach (NULL, this);

	if (closure) {
		NameScope *ns = NameScope::GetNameScope (n->obj);
		if (ns && ns->GetMerged ()) {
			NameScope *con_ns = closure->FindNameScope();
			if (con_ns)
				con_ns->UnmergeTemporaryScope (ns);
		}

		closure->OnCollectionChanged (this, CollectionChangeTypeItemRemoved, n->obj, NULL);
	}
	
	delete n;
	return true;
}

void
Collection::Clear ()
{
	Collection::Node *n;

	generation++;
	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next) {
		n->obj->Detach (NULL, this);

		NameScope *ns = NameScope::GetNameScope (n->obj);
		if (ns && ns->GetMerged ()) {
			NameScope *parent_ns = n->obj->FindNameScope();
			if (parent_ns)
				parent_ns->UnmergeTemporaryScope (ns);
		}
	}

	list->Clear (true);

	if (closure)
		closure->OnCollectionChanged (this, CollectionChangeTypeChanged, NULL, NULL);
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
	g_ptr_array_free (z_sorted, true);
}

class UIElementNode : public List::Node {
public:
	UIElement *item;
	
	UIElementNode (UIElement *v) : item (v) { }
};

static int
UIElementZIndexComparer (gconstpointer ui1, gconstpointer ui2)
{
	int z1 = (*((UIElement **) ui1))->GetValue (UIElement::ZIndexProperty)->AsInt32 ();
	int z2 = (*((UIElement **) ui2))->GetValue (UIElement::ZIndexProperty)->AsInt32 ();
	int zdiff = z1 - z2;
	
	if (zdiff == 0)
		return 0;
	else if (zdiff < 0)
		return -1;
	else
		return 1;
}

void
VisualCollection::ResortByZIndex ()
{
	g_ptr_array_sort (z_sorted, UIElementZIndexComparer);
}


void
VisualCollection::VisualAdded (Visual *visual)
{
	Panel *panel = (Panel *) closure;
	UIElement *item = (UIElement *) visual;

	if (panel == NULL)
		return;
	
	item->parent = panel;
	item->UpdateTransform ();
	item->UpdateTotalOpacity ();
	item->UpdateTotalRenderVisibility ();
	item->UpdateTotalHitTestVisibility ();
	item->Invalidate ();
	item->SetSurface (panel->GetSurface ());
}

void
VisualCollection::VisualRemoved (Visual *visual)
{
	UIElement *item = (UIElement *) visual;

	item->SetSurface (NULL);

	if (item->parent == NULL)
		return;

	// we can't just call item->Invalidate() here, since dirty.cpp
	// relies on the child->parent link being present.  Instead we
	// directly call item->parent->Invalidate with the entire
	// bounds of the child (which is likely suboptimal,
	// considering panels without backgrounds might have a more
	// optimized region we can redraw).
	item->parent->Invalidate (item->GetSubtreeBounds());
	item->parent = NULL;
}

bool
VisualCollection::Add (DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	bool b = Collection::Add (item);

	if (b) {
		g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);

		VisualAdded (item);

		if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
			/* emit loaded events on the new item if the tree
			   we're adding it to has already been "loaded" */
			item->OnLoaded ();
		}
	}
	return b;
}

DependencyObject *
VisualCollection::SetVal (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	UIElement *old = (UIElement *) Collection::SetVal (index, item);
	
	if (old) {
		VisualRemoved (old);

		g_ptr_array_remove (z_sorted, old);
	
		g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);
	
		if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
			/* emit loaded events on the new item if the tree
			   we're adding it to has already been "loaded" */
			item->OnLoaded ();
		}

		VisualAdded (item);
	}

	return old;
}

bool
VisualCollection::Insert (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	bool b = Collection::Insert (index, item);

	if (b) {
		g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);
	
		if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
			/* emit loaded events on the new item if the tree
			   we're adding it to has already been "loaded" */
			item->OnLoaded ();
		}

		VisualAdded (item);
	}

	return b;
}

bool
VisualCollection::Remove (DependencyObject *data)
{
	UIElement *item = (UIElement *) data;

	VisualRemoved (item);

	bool b = Collection::Remove (item);
	
	if (b)
		g_ptr_array_remove (z_sorted, item);

	return b;
}

bool
VisualCollection::RemoveAt (int index)
{
	Collection::Node *n = (Collection::Node *) list->Index (index);
	if (n == NULL)
		return false;
	
	UIElement *item = (UIElement *) n->obj;

	VisualRemoved (item);

	bool b = Collection::RemoveAt (index);

	if (b)
		g_ptr_array_remove (z_sorted, item);

	return b;
}

void
VisualCollection::Clear ()
{
	Collection::Node *n;
	for (n = (Collection::Node *) list->First (); n; n = (Collection::Node *) n->next) 
		VisualRemoved ((UIElement*)n->obj);
	
	g_ptr_array_set_size (z_sorted, 0);
	Collection::Clear ();
}


bool
TriggerCollection::Add (DependencyObject *data)
{
	bool b = Collection::Add (data);

	if (b) {
		FrameworkElement *fwe = (FrameworkElement *) closure;
		EventTrigger *trigger = (EventTrigger *) data;

		trigger->SetTarget (fwe);
	}

	return b;
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
	switch (kind) {
	case Type::GEOMETRY_COLLECTION:
		return new GeometryCollection ();
	case Type::GRADIENTSTOP_COLLECTION:
		return new GradientStopCollection ();
	case Type::INLINES:
		return new Inlines ();
	case Type::KEYFRAME_COLLECTION:
		return new KeyFrameCollection ();
	case Type::MEDIAATTRIBUTE_COLLECTION:
		return new MediaAttributeCollection ();
	case Type::PATHFIGURE_COLLECTION:
		return new PathFigureCollection ();
	case Type::PATHSEGMENT_COLLECTION:
		return new PathSegmentCollection ();
	case Type::RESOURCE_DICTIONARY:
		return new ResourceDictionary ();
	case Type::STROKE_COLLECTION:
		return new StrokeCollection ();
	case Type::STYLUSPOINT_COLLECTION:
		return new StylusPointCollection ();
	case Type::TIMELINE_COLLECTION:
		return new TimelineCollection ();
	case Type::TIMELINEMARKER_COLLECTION:
		return new TimelineMarkerCollection ();
	case Type::TRANSFORM_COLLECTION:
		return new TransformCollection ();
	case Type::TRIGGER_COLLECTION:
		return new TriggerCollection ();
	case Type::TRIGGERACTION_COLLECTION:
		return new TriggerActionCollection ();
	case Type::VISUAL_COLLECTION:
		return new VisualCollection ();
	default:
		return NULL;
	}
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

StrokeCollection *
stroke_collection_new (void)
{
	return new StrokeCollection ();
}

StylusPointCollection *
stylus_point_collection_new (void)
{
	return new StylusPointCollection ();
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


