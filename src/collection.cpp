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
}

Collection::Node::~Node ()
{
	obj->Detach (NULL, obj->GetParent ());
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
	list->Clear (true);
	delete list;
}

void
Collection::Add (DependencyObject *data)
{
	g_return_if_fail (Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType()));
	
	generation++;
	list->Append (new Collection::Node (data, this));
	data->Attach (NULL, this);
/*
	NameScope *ns = data->FindNameScope ();
	NameScope *con_ns = ((DependencyObject *) closure)->FindNameScope ();
	if (ns && con_ns && ns != con_ns) {
		con_ns->MergeTemporaryScope (ns);
		NameScope::SetNameScope (data, con_ns);
	}
*/
	if (closure)
		closure->OnCollectionChanged (this, CollectionChangeTypeItemAdded, data, NULL);
}

void
Collection::Insert (int index, DependencyObject *data)
{
	g_return_if_fail (Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType()));
	
	generation++;
	list->Insert (new Collection::Node (data, this), index);

	data->Attach (NULL, this);
/*
	NameScope *ns = data->FindNameScope ();
	NameScope *con_ns = ((DependencyObject *) closure)->FindNameScope ();
	if (ns && con_ns && ns != con_ns) {
		con_ns->MergeTemporaryScope (ns);
		NameScope::SetNameScope (data, con_ns);
	}
*/
	if (closure)
		closure->OnCollectionChanged (this, CollectionChangeTypeItemAdded, data, NULL);
}

void
Collection::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (closure) {
		/* unfortunately OnSubPropertyChanged doesn't give us
		   enough info to fill in the obj parameter here */
		closure->OnCollectionChanged (this, CollectionChangeTypeItemChanged, NULL, subprop);
	}
}

DependencyObject *
Collection::SetVal (int index, DependencyObject *data)
{
	g_return_val_if_fail (Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType()), NULL);

	generation++;
	Node *old = (Collection::Node *) list->Replace (new Collection::Node (data, this), index);

	old->Unlink ();

	data->Attach (NULL, this);
	old->obj->Detach (NULL, this);

	if (closure){
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
	if (prop == CountProperty){
		return new Value (list->Length ());
	}
	return DependencyObject::GetValue (prop);
}

bool
Collection::Remove (DependencyObject *data)
{
	Collection::Node *n;
	
	generation++;
	if (!(n = (Collection::Node *) list->Find (CollectionNodeFinder, data)))
		return false;
	
	n->Unlink ();

	data->Detach (NULL, this);

	if (closure)
		closure->OnCollectionChanged (this, CollectionChangeTypeItemRemoved, data, NULL);
	
	delete n;
	return true;
}

bool
Collection::RemoveAt (int index)
{
	Collection::Node *n = (Collection::Node *) list->Index (index);

	generation++;
	if (n == NULL)
		return false;

	n->Unlink ();
	n->obj->Detach (NULL, this);
	if (closure)
		closure->OnCollectionChanged (this, CollectionChangeTypeItemRemoved, n->obj, NULL);
	
	delete n;
	return true;
}

void
Collection::Clear ()
{
	Collection::Node *n;

	generation++;
	for (n = (Collection::Node*)list->First(); n; n = (Collection::Node*)n->Next()) {
		n->obj->Detach (NULL, this);
	}

	list->Clear (true);

	if (closure)
		closure->OnCollectionChanged (this, CollectionChangeTypeChanged, NULL, NULL);
}

DependencyProperty *Collection::CountProperty;
void
collection_init ()
{
	Collection::CountProperty = DependencyObject::Register (Type::COLLECTION, "Count", Type::INT32);
}

void
collection_add (Collection *collection, DependencyObject *data)
{
	collection->Add (data);
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

void
collection_insert (Collection *collection, int index, DependencyObject *data)
{
	collection->Insert (index, data);
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

	List::Node *next = iterator->current->Next ();

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
	if (iterator->collection->generation != iterator->generation){
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
	z_sorted_list = new List ();
}

VisualCollection::~VisualCollection ()
{
	z_sorted_list->Clear (true);
	delete z_sorted_list;
}

class UIElementNode : public List::Node {
public:
	UIElement *item;
	
	UIElementNode (UIElement *v) : item (v) { }
};

static bool
UIElementNodeFinder (List::Node *uin, void *data)
{
	return ((UIElementNode *) uin)->item == (UIElement *) data;
}

static int
UIElementNodeComparer (List::Node *ui1, List::Node *ui2)
{
	int z1 = ((UIElementNode*)ui1)->item->GetValue(UIElement::ZIndexProperty)->AsInt32();
	int z2 = ((UIElementNode*)ui2)->item->GetValue(UIElement::ZIndexProperty)->AsInt32();
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
	z_sorted_list->Clear (true);

	UIElementNode* n = (UIElementNode *) list->First ();
	while (n != NULL) {
		z_sorted_list->InsertSorted (new UIElementNode ((UIElement *) n->item), UIElementNodeComparer, true);

		n = (UIElementNode *) n->Next ();
	}
}


void
VisualCollection::VisualUpdate (DependencyObject *data)
{
	Panel *panel = (Panel *) closure;
	UIElement *item = (UIElement *) data;

	if (panel == NULL)
		return;
	
	item->parent = panel;
	item->UpdateTransform ();
	item->UpdateTotalOpacity ();
	item->Invalidate ();
	panel->UpdateBounds ();
}

void
VisualCollection::Add (DependencyObject *data)
{
	UIElement *item = (UIElement *) data;

	Collection::Add (item);
	z_sorted_list->InsertSorted (new UIElementNode (item), UIElementNodeComparer, true);
	if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
		/* emit loaded events on the new item if the tree
		   we're adding it to has already been "loaded" */
		item->OnLoaded ();
	}

	VisualUpdate (data);
}

DependencyObject *
VisualCollection::SetVal (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;

	UIElement *old = (UIElement *) Collection::SetVal (index, item);
	z_sorted_list->Remove (UIElementNodeFinder, old);
	z_sorted_list->InsertSorted (new UIElementNode (item), UIElementNodeComparer, true);

	if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
		/* emit loaded events on the new item if the tree
		   we're adding it to has already been "loaded" */
		item->OnLoaded ();
	}

	VisualUpdate (data);

	return old;
}

void
VisualCollection::Insert (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;

	Collection::Insert (index, item);
	z_sorted_list->InsertSorted (new UIElementNode (item), UIElementNodeComparer, true);

	if (closure && ((UIElement*)closure)->flags & UIElement::IS_LOADED) {
		/* emit loaded events on the new item if the tree
		   we're adding it to has already been "loaded" */
		item->OnLoaded ();
	}

	VisualUpdate (data);
}

bool
VisualCollection::Remove (DependencyObject *data)
{
	Panel *panel = (Panel *) closure;
	UIElement *item = (UIElement *) data;

 	item->Invalidate ();
	bool b = Collection::Remove (item);
	z_sorted_list->Remove (UIElementNodeFinder, item);
	if (panel)
		panel->UpdateBounds ();

	return b;
}

bool
VisualCollection::RemoveAt (int index)
{
	Panel *panel = (Panel *) closure;

	Collection::Node *n = (Collection::Node *) list->Index (index);
	if (n == NULL)
		return false;
	UIElement *item = (UIElement *) n->obj;

 	item->Invalidate ();
	bool b = Collection::RemoveAt (index);
	z_sorted_list->Remove (UIElementNodeFinder, n->obj);
	if (panel)
		panel->UpdateBounds ();

	return b;
}

void
VisualCollection::Clear ()
{
	if (list->Length() == 0)
		return;

	Panel *panel = (Panel *) closure;

	z_sorted_list->Clear (true);
	Collection::Clear ();

	// we need to force the invalidate here if the bounds don't
	// change, as it means the canvas has width/height explicitly
	// set.
	if (panel)
		panel->UpdateBounds (true);
}


void
TriggerCollection::Add (DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	printf ("Adding %p\n", data);
	EventTrigger *trigger = (EventTrigger *) data;

	Collection::Add (trigger);

	trigger->SetTarget (fwe);
}

DependencyObject *
TriggerCollection::SetVal (int index, DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	printf ("Inserting %p\n", data);
	EventTrigger *trigger = (EventTrigger *) data;

	FrameworkElement *old = (FrameworkElement *) Collection::SetVal (index, trigger);

	trigger->SetTarget (fwe);

	trigger->RemoveTarget (old);

	return old;
}

void
TriggerCollection::Insert (int index, DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	printf ("Inserting %p\n", data);
	EventTrigger *trigger = (EventTrigger *) data;

	Collection::Insert (index, trigger);

	trigger->SetTarget (fwe);
}

bool
TriggerCollection::Remove (DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	EventTrigger *trigger = (EventTrigger *) data;

	bool b = Collection::Remove (trigger);

	trigger->RemoveTarget (fwe);
	return b;
}

bool
TriggerCollection::RemoveAt (int index)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	Collection::Node *n = (Collection::Node *) list->Index (index);
	if (n == NULL)
		return false;

	EventTrigger *trigger = (EventTrigger *) n->obj;

	bool b = Collection::RemoveAt (index);

	trigger->RemoveTarget (fwe);
	return b;
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

Inlines *
inlines_new (void)
{
	return new Inlines ();
}


