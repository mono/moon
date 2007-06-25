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
#include "geometry.h"
#include "brush.h"
#include "animation.h"
#include "transform.h"

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
	
	list->Append (new Collection::Node (data, this));
}

void
Collection::Insert (int index, DependencyObject *data)
{
	g_return_if_fail (Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType()));
	
	list->Insert (new Collection::Node (data, this), index);
}

void
Collection::Remove (DependencyObject *data)
{
	Collection::Node *n;
	
	if (!(n = (Collection::Node *) list->Find (CollectionNodeFinder, data)))
		return;
	
	n->Unlink ();
	
	delete n;
}

void
Collection::Clear ()
{
	list->Clear (true);
}

void 
collection_add (Collection *collection, DependencyObject *data)
{
	collection->Add (data);
}

void 
collection_remove (Collection *collection, DependencyObject *data)
{
	collection->Remove (data);
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

bool
collection_iterator_move_next (CollectionIterator *iterator)
{
	List::Node *next;
	
	if (!iterator->current)
		return false;

	if (!iterator->first && !(next = iterator->current->Next ()))
		return false;
	
	iterator->current = next;
	iterator->first = false;

	return true;
}

void 
collection_iterator_reset (CollectionIterator *iterator)
{
	iterator->current = iterator->collection->list->First ();
	iterator->first = true;
}

DependencyObject *
collection_iterator_get_current (CollectionIterator *iterator)
{
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
  printf ("ResortByZIndex\n");
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
	
	item->parent = panel;
	item->update_xform ();
	item_update_bounds (panel);
	item_invalidate (panel);
}

void
VisualCollection::Add (DependencyObject *data)
{
	DependencyObject *obj = (DependencyObject *) closure;
	UIElement *item = (UIElement *) data;

	Collection::Add (item);
	z_sorted_list->InsertSorted (new UIElementNode (item), UIElementNodeComparer, true);
	if (((UIElement*)closure)->flags & UIElement::IS_LOADED) {
		/* emit loaded events on the new item if the tree
		   we're adding it to has already been "loaded" */
		item->OnLoaded ();
	}

	VisualUpdate (data);
}

void
VisualCollection::Insert (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;

	Collection::Insert (index, item);
	z_sorted_list->InsertSorted (new UIElementNode (item), UIElementNodeComparer, true);

	if (((UIElement*)closure)->flags & UIElement::IS_LOADED) {
		/* emit loaded events on the new item if the tree
		   we're adding it to has already been "loaded" */
		item->OnLoaded ();
	}

	VisualUpdate (data);
}

void
VisualCollection::Remove (DependencyObject *data)
{
	Panel *panel = (Panel *) closure;
	UIElement *item = (UIElement *) data;
	
	item_invalidate (item);
	Collection::Remove (item);
	z_sorted_list->Remove (UIElementNodeFinder, item);
	item_update_bounds (panel);
}

void
VisualCollection::Clear ()
{
	z_sorted_list->Clear (true);
	Collection::Clear ();
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

void
TriggerCollection::Insert (int index, DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	printf ("Inserting %p\n", data);
	EventTrigger *trigger = (EventTrigger *) data;

	Collection::Insert (index, trigger);

	trigger->SetTarget (fwe);
}

void
TriggerCollection::Remove (DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	EventTrigger *trigger = (EventTrigger *) data;

	Collection::Remove (trigger);

	trigger->RemoveTarget (fwe);
}

void
ResourceCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
ResourceCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
StrokeCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
StrokeCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
MediaAttributeCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
MediaAttributeCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
StylusPointCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
StylusPointCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
TimelineMarkerCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
TimelineMarkerCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
Inlines::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
Inlines::Remove (DependencyObject *data)
{
	Collection::Remove (data);
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
	case Type::RESOURCE_COLLECTION:
		return new ResourceCollection ();
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

ResourceCollection *
resource_collection_new (void)
{
	return new ResourceCollection ();
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


