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

#include "canvas.h"
#include "collection.h"
#include "geometry.h"
#include "transform.h"
#include "frameworkelement.h"
#include "namescope.h"
#include "textblock.h"
#include "utils.h"
#include "error.h"
#include "deployment.h"
#include "multiscalesubimage.h"
#include "factory.h"

namespace Moonlight {

//
// BlockCollection
//

BlockCollection::BlockCollection ()
	: TextElementCollection (Type::BLOCK_COLLECTION)
{
}

//
// TextElementCollection
//

TextElementCollection::TextElementCollection ()
	: DependencyObjectCollection (Type::TEXTELEMENT_COLLECTION)
{
}

TextElementCollection::TextElementCollection (Type::Kind object_type)
	: DependencyObjectCollection (object_type)
{
}

//
// Collection
//

Collection::Collection ()
	: DependencyObject (Type::COLLECTION)
{
	Init ();
}

Collection::Collection (Type::Kind object_type)
	: DependencyObject (object_type)
{
	Init ();
}

void
Collection::Init ()
{
	array = g_ptr_array_new ();
	generation = 0;

#if EVENT_ARG_REUSE
	itemChangedEventArgs = NULL;
	changedEventArgs = NULL;
#endif
}

void
Collection::CloneCore (Types* types, DependencyObject* fromObj)
{
	DependencyObject::CloneCore (types, fromObj);

	Collection *c = (Collection*)fromObj;

	for (guint i = 0; i < c->array->len; i++) {
		Value *value = Value::Clone ((Value *) c->array->pdata[i]);
		Add (value);
		delete value;
	}
}

Collection::~Collection ()
{
	g_ptr_array_free (array, true);

#if EVENT_ARG_REUSE
	if (itemChangedEventArgs)
		itemChangedEventArgs->unref ();
	if (changedEventArgs)
		changedEventArgs->unref ();
#endif
}

void
Collection::Dispose ()
{
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		RemovedFromCollection (value, false);
		delete value;
	}
	g_ptr_array_set_size (array, 0);
	
	DependencyObject::Dispose ();
}

CollectionIterator *
Collection::GetIterator ()
{
	return new CollectionIterator (this);
}

int
Collection::Add (Value *value)
{
	MoonError error;
	return AddWithError (value, &error);
}

int
Collection::Add (Value value)
{
	return Add (&value);
}

int
Collection::AddWithError (Value *value, MoonError *error)
{
	bool rv = InsertWithError (array->len, value, error);
	return rv ? array->len - 1 : -1;
}

bool
Collection::Clear ()
{
	EmitChanged (CollectionChangedActionClearing, NULL, NULL, -1);

	GPtrArray *old_array = array;

	array = g_ptr_array_new ();

	guint len = old_array->len;
	Value** vals = (Value**)old_array->pdata;

	generation++;
	
	SetCount (0);

	for (guint i = 0; i < len; i++) {
		RemovedFromCollection (vals[i], true);
		delete vals[i];
	}

	g_ptr_array_free (old_array, TRUE);
	
	EmitChanged (CollectionChangedActionCleared, NULL, NULL, -1);

	return true;
}

bool
Collection::Contains (Value *value)
{
	return IndexOf (value) != -1;
}

int
Collection::IndexOf (Value *value)
{
	Value *v;
	
	for (guint i = 0; i < array->len; i++) {
		v = (Value *) array->pdata[i];
		if (*v == *value)
			return i;
	}
	
	return -1;
}

int
Collection::IndexOf (Value value)
{
	return IndexOf (&value);
}

bool
Collection::Insert (int index, Value value)
{
	return Insert (index, &value);
}

bool
Collection::InsertWithError (int index, Value *value, MoonError *error)
{
	Value *added;
	
	// Check that the item can be added to our collection
	if (!CanAdd (value))
		return false;
	
	// bounds check
	if (index < 0)
		return false;

	int count = GetCount ();
	if (index > count)
		index = count;
	
	added = new Value (*value);

	if (AddedToCollection (added, error)) {
		g_ptr_array_insert (array, index, added);
	
		SetCount ((int) array->len);

		EmitChanged (CollectionChangedActionAdd, added, NULL, index);

		Deployment *deployment = GetDeployment ();
		if (addManagedRef && added->HoldManagedRef (deployment) && !deployment->IsShuttingDown ()) {
			/* The managed StylusPointCollection contains StylusPoint objects, while the native StylusPointCollection
			* contains UnmanagedStylusPoint objects. This means that UnmanagedStylusPoint's managed peer is not
			* reachable, so if it's unreffed here, it ends up getting gc'ed pretty quickly. DRTs: #TopXXScenarios5 and #TopXXScenarios6 */
			if (GetObjectType () != Type::STYLUSPOINT_COLLECTION)
				added->Weaken (deployment);
		}

		return true;
	}
	else {
		delete added;
		return false;
	}
}

bool
Collection::Insert (int index, Value *value)
{
	MoonError error;
	return InsertWithError (index, value, &error);
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
	
	RemovedFromCollection (value, true);
	
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
	MoonError error;
	return SetValueAtWithError (index, value, &error);
}

bool
Collection::SetValueAtWithError (int index, Value *value, MoonError *error)
{
	Value *added, *removed;
	
	// Check that the value can be added to our collection
	if (!CanAdd (value)) {
		MoonError::FillIn (error, MoonError::ARGUMENT, "");
		return false;
	}
	
	// check array bounds
	if (index < 0 || (guint) index >= array->len) {
		MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, "");
		return false;
	}
	
	removed = (Value *) array->pdata[index];
	added = new Value (*value);
	
	if (AddedToCollection (added, error)) {
		array->pdata[index] = added;
	
		RemovedFromCollection (removed, true);
	
		EmitChanged (CollectionChangedActionReplace, added, removed, index);

		Deployment *deployment = GetDeployment ();
		if (addManagedRef && added->HoldManagedRef (deployment) && !deployment->IsShuttingDown ()) {
			/* The managed StylusPointCollection contains StylusPoint objects, while the native StylusPointCollection
			* contains UnmanagedStylusPoint objects. This means that UnmanagedStylusPoint's managed peer is not
			* reachable, so if it's unreffed here, it ends up getting gc'ed pretty quickly. DRTs: #TopXXScenarios5 and #TopXXScenarios6 */
			if (GetObjectType () != Type::STYLUSPOINT_COLLECTION)
				added->Weaken (deployment);
		}

		delete removed;

		return true;
	} else {
		delete added;
		return false;
	}
}

void
Collection::EmitChanged (CollectionChangedAction action, Value *new_value, Value *old_value, int index)
{
#if EVENT_ARG_REUSE
	if (!HasHandlers (Collection::ChangedEvent))
		return;

	if (!changedEventArgs)
		changedEventArgs = MoonUnmanagedFactory::CreateCollectionChangedEventArgs ();

	changedEventArgs->SetChangedAction (action);
	changedEventArgs->SetNewItem (new_value);
	changedEventArgs->SetOldItem (old_value);
	changedEventArgs->SetIndex (index);

	changedEventArgs->ref ();

	Emit (Collection::ChangedEvent, changedEventArgs);

	changedEventArgs->SetNewItem (NULL);
	changedEventArgs->SetOldItem (NULL);
#else
	if (HasHandlers (Collection::ChangedEvent))
		Emit (Collection::ChangedEvent, new CollectionChangedEventArgs (action, new_value, old_value, index));
#endif
}

void
Collection::EmitItemChanged (DependencyObject *object, DependencyProperty *property, Value *newValue, Value *oldValue)
{
#if EVENT_ARG_REUSE
	if (!HasHandlers (Collection::ItemChangedEvent))
		return;

	if (!itemChangedEventArgs)
		itemChangedEventArgs = new CollectionItemChangedEventArgs ();

	itemChangedEventArgs->SetCollectionItem (object);
	itemChangedEventArgs->SetProperty (property);
	itemChangedEventArgs->SetOldValue (oldValue);
	itemChangedEventArgs->SetNewValue (newValue);

	itemChangedEventArgs->ref ();
	Emit (Collection::ItemChangedEvent, itemChangedEventArgs);

	itemChangedEventArgs->SetCollectionItem (NULL);
	itemChangedEventArgs->SetProperty (NULL);
	itemChangedEventArgs->SetOldValue (NULL);
	itemChangedEventArgs->SetNewValue (NULL);
#else
	if (HasHandlers (Collection::ItemChangedEvent))
		Emit (Collection::ItemChangedEvent, new CollectionItemChangedEventArgs (object, property, oldValue, newValue));
#endif
}

bool
Collection::CanAdd (Value *value)
{
	return value->Is (GetDeployment (), GetElementType ());
}


//
// DependencyObjectCollection
//

DependencyObjectCollection::DependencyObjectCollection ()
	: Collection (Type::DEPENDENCY_OBJECT_COLLECTION)
{
	is_secondary_parent = false;
	sets_parent = true;
}

DependencyObjectCollection::DependencyObjectCollection (bool sets_parent)
	: Collection (Type::DEPENDENCY_OBJECT_COLLECTION)
{
	is_secondary_parent = false;
	this->sets_parent = sets_parent;
}

DependencyObjectCollection::DependencyObjectCollection (Type::Kind object_type)
	: Collection (object_type)
{
	is_secondary_parent = false;
	sets_parent = true;
}

DependencyObjectCollection::DependencyObjectCollection (Type::Kind object_type, bool sets_parent)
	: Collection (object_type)
{
	is_secondary_parent = false;
	this->sets_parent = sets_parent;
}

DependencyObjectCollection::~DependencyObjectCollection ()
{
}


void
DependencyObjectCollection::OnMentorChanged (DependencyObject *old_mentor, DependencyObject *new_mentor)
{
	Collection::OnMentorChanged (old_mentor, new_mentor);

	DependencyObject *obj;
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		obj = value->AsDependencyObject ();
		if (obj)
			obj->SetMentor (new_mentor);
	}
}

bool
DependencyObjectCollection::AddedToCollection (Value *value, MoonError *error)
{
	DependencyObject *obj = value->AsDependencyObject ();
	
	if (sets_parent) {
		DependencyObject *existing_parent = obj->GetParent ();
		obj->AddParent (this, error);
		if (!error->number && !existing_parent && GetIsSecondaryParent ())
			obj->AddParent (this, error);
		if (error->number)
			return false;
	} else {
		obj->SetMentor (GetMentor ());
	}

	obj->AddPropertyChangeListener (this);
	
	// Only set the IsAttached state when the object has successfully been
	// added to the collection.
	bool rv = Collection::AddedToCollection (value, error);
	obj->SetIsAttached (rv && IsAttached ());
	
	if (!rv) {
		if (sets_parent) {
			/* If we set the parent, but the object wasn't added to the collection, make sure we clear the parent */
			obj->RemoveParent (this, error);
			obj->SetMentor (GetMentor ());
		} else {
			obj->SetMentor (NULL);
		}
	}
	
	return rv;
}

void
DependencyObjectCollection::RemovedFromCollection (Value *value, bool is_value_safe)
{
	if (is_value_safe) {
		DependencyObject *obj = value->AsDependencyObject ();

		if (obj) {
			obj->RemovePropertyChangeListener (this);
			if (GetIsSecondaryParent ())
				obj->RemoveSecondaryParent (this);

			if (sets_parent && obj->GetParent () == this)
				obj->RemoveParent (this, NULL);
			obj->SetIsAttached (false);
		}
	}
	
	Collection::RemovedFromCollection (value, is_value_safe);
}

void
DependencyObjectCollection::OnIsAttachedChanged (bool attached)
{
	Collection::OnIsAttachedChanged (attached);

	DependencyObject *obj;
	Value *value;
	
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		obj = value->AsDependencyObject ();
		if (obj)
			obj->SetIsAttached (attached);
	}
}

void
DependencyObjectCollection::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	EmitItemChanged (obj, subobj_args->GetProperty(), subobj_args->GetNewValue(), subobj_args->GetOldValue());
}

void
DependencyObjectCollection::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	DependencyObject *obj;
	Value *value;
	
	Types *types = GetDeployment()->GetTypes ();
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		obj = value->AsDependencyObject (types);
		if (obj)
			obj->UnregisterAllNamesRootedAt (from_ns);
	}
	
	Collection::UnregisterAllNamesRootedAt (from_ns);
}

void
DependencyObjectCollection::RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error)
{
	DependencyObject *obj;
	Value *value;
	
	Types *types = GetDeployment()->GetTypes ();
	for (guint i = 0; i < array->len; i++) {
		if (error->number)
			break;

		value = (Value *) array->pdata[i];
		obj = value->AsDependencyObject (types);
		if (obj)
			obj->RegisterAllNamesRootedAt (to_ns, error);
	}
	
	Collection::RegisterAllNamesRootedAt (to_ns, error);
}

//
// InlineCollection
//

InlineCollection::InlineCollection ()
	: DependencyObjectCollection (Type::INLINE_COLLECTION)
{
	for_hyperlink = false;
}

InlineCollection::~InlineCollection ()
{
}

bool
InlineCollection::Equals (InlineCollection *inlines)
{
	Inline *run0, *run1;
	
	if (inlines->array->len != array->len)
		return false;

	Types *types = GetDeployment()->GetTypes ();
	for (guint i = 0; i < array->len; i++) {
		run1 = ((Value *) inlines->array->pdata[i])->AsInline (types);
		run0 = ((Value *) array->pdata[i])->AsInline (types);
		
		if (!run0->Equals (run1))
			return false;
	}
	
	return true;
}

bool
InlineCollection::ValueIsSupportedInHyperlink (Deployment *depl, Value* value)
{
	if (value->Is (depl, Type::INLINEUICONTAINER) ||
	    value->Is (depl, Type::LINEBREAK) ||
	    value->Is (depl, Type::HYPERLINK)) {
		return false;
	}

	if (value->Is (depl, Type::SPAN)) {
		// recurse into Span.Inlines
		Span *span = value->AsSpan(depl->GetTypes());
		int count = span->GetInlines()->GetCount();
		for (int i = 0; i < count; i ++) {
			Value *v = span->GetInlines()->GetValueAt(i);
			if (!ValueIsSupportedInHyperlink (depl, v))
				return false;
		}
	}

	return true;
}

bool
InlineCollection::AddedToCollection (Value *value, MoonError *error)
{
	if (!for_hyperlink) {
		return DependencyObjectCollection::AddedToCollection (value, error);
	}
	else {
		Deployment *depl = GetDeployment ();

		if (!ValueIsSupportedInHyperlink (depl, value)) {
			// FIXME we need to figure out the actual message here..
			MoonError::FillIn (error, MoonError::ARGUMENT, "value not permissible in Hyperlink");
			return false;
		}

		return DependencyObjectCollection::AddedToCollection (value, error);
	}
}

//
// UIElementCollection
//

UIElementCollection::UIElementCollection ()
	: DependencyObjectCollection (Type::UIELEMENT_COLLECTION)
{
	z_sorted = g_ptr_array_new ();
}

UIElementCollection::UIElementCollection (bool sets_parent)
	: DependencyObjectCollection (Type::UIELEMENT_COLLECTION, sets_parent)
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
	int zi1 = Canvas::GetZIndex (*((UIElement **) ui1));
	int zi2 = Canvas::GetZIndex (*((UIElement **) ui2));

	if (zi1 == zi2) {
		double z1 = Canvas::GetZ (*((UIElement **) ui1));
		double z2 = Canvas::GetZ (*((UIElement **) ui2));

		if (isnan (z1) || isnan (z2))
			return 0;

		return z1 > z2 ? 1 : z1 < z2 ? -1 : 0;
	}

	return zi1 - zi2;
}

void
UIElementCollection::ResortByZIndex ()
{
	g_ptr_array_set_size (z_sorted, array->len);
	
	if (array->len == 0)
		return;
	
	Types *types = GetDeployment()->GetTypes ();
	for (guint i = 0; i < array->len; i++) 
		z_sorted->pdata[i] = ((Value *) array->pdata[i])->AsUIElement (types);
	
	if (array->len > 1)
		g_ptr_array_sort (z_sorted, UIElementZIndexComparer);
}

bool
UIElementCollection::Clear ()
{
	g_ptr_array_set_size (z_sorted, 0);
	return DependencyObjectCollection::Clear ();
}

//
// HitTestCollection
//

HitTestCollection::HitTestCollection ()
{
}

//
// DoubleCollection
//

DoubleCollection::DoubleCollection ()
	: Collection (Type::DOUBLE_COLLECTION)
{
}

DoubleCollection::~DoubleCollection ()
{
}

DoubleCollection *
DoubleCollection::FromStr (const char *s)
{
	GArray *values = double_garray_from_str (s, 0);

	if (values->len == 0) {
		g_array_free (values, true);
		return NULL;
	}

	DoubleCollection *doubles = MoonUnmanagedFactory::CreateDoubleCollection ();
	for (guint i = 0; i < values->len; i ++)
		doubles->Add (Value (g_array_index (values, double, i)));
	g_array_free (values, true);

	return doubles;
}

//
// Point Collection
//

PointCollection::PointCollection ()
	: Collection (Type::POINT_COLLECTION)
{
}

PointCollection::~PointCollection ()
{
}

PointCollection *
PointCollection::FromStr (const char *s)
{
	int i, j, n = 0;
	GArray *values = double_garray_from_str (s, 0);

	n = values->len / 2;
	if (n == 0 || n % 1 == 1) {
		g_array_free (values, true);
		return NULL;
	}

	PointCollection *points = MoonUnmanagedFactory::CreatePointCollection();
	for (i = 0, j = 0; j < n; j++) {
		double x = g_array_index (values, double, i++);
		double y = g_array_index (values, double, i++);

		points->Add (Point (x, y));
	}

	g_array_free (values, true);
	return points;
}

//
// Item Collection
//

ItemCollection::ItemCollection ()
	: Collection (Type::ITEM_COLLECTION)
{
}

ItemCollection::~ItemCollection ()
{
}

bool
ItemCollection::AddedToCollection (Value *value, MoonError *error)
{
	if (value->IsDependencyObject (GetDeployment ())) {
		DependencyObject *obj = value->AsDependencyObject ();
		obj->AddParent (this, error);
		if (error->number)
			return false;

		obj->AddPropertyChangeListener (this);
	
		// Only set the IsAttached state when the object has successfully been
		// added to the collection.
		bool rv = Collection::AddedToCollection (value, error);
		obj->SetIsAttached (rv && IsAttached ());
	
		if (!rv) {
			/* If we set the parent, but the object wasn't added to the collection, make sure we clear the parent */
			obj->RemoveParent (this, error);
		}
		return rv;
	} else {
		return Collection::AddedToCollection (value, error);
	}
}

void
ItemCollection::RemovedFromCollection (Value *value, bool is_value_safe)
{
	if (is_value_safe && value->IsDependencyObject (GetDeployment ())) {
		DependencyObject *obj = value->AsDependencyObject ();

		if (obj) {
			obj->RemovePropertyChangeListener (this);
			obj->RemoveParent (this, NULL);
			obj->SetMentor (NULL);
			obj->SetIsAttached (false);
		}
	}
	
	Collection::RemovedFromCollection (value, is_value_safe);
}

void
ItemCollection::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	DependencyObject *obj;
	Value *value;
	
	Types *types = GetDeployment()->GetTypes ();
	for (guint i = 0; i < array->len; i++) {
		value = (Value *) array->pdata[i];
		if (value->IsDependencyObject (GetDeployment ())) {
			obj = value->AsDependencyObject (types);
			if (obj)
				obj->UnregisterAllNamesRootedAt (from_ns);
		}
	}
	
	Collection::UnregisterAllNamesRootedAt (from_ns);
}

void
ItemCollection::RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error)
{
	DependencyObject *obj;
	Value *value;
	
	Types *types = GetDeployment()->GetTypes ();
	for (guint i = 0; i < array->len; i++) {
		if (error->number)
			break;

		value = (Value *) array->pdata[i];
		if (value->IsDependencyObject (GetDeployment ())) {
			obj = value->AsDependencyObject (types);
			if (obj)
				obj->RegisterAllNamesRootedAt (to_ns, error);
		}
	}
	
	Collection::RegisterAllNamesRootedAt (to_ns, error);
}

//
// Trigger Collection
//

TriggerCollection::TriggerCollection ()
	: DependencyObjectCollection (Type::TRIGGER_COLLECTION)
{
}

TriggerCollection::~TriggerCollection ()
{
}

//
// TriggerAction Collection
//

TriggerActionCollection::TriggerActionCollection ()
	: DependencyObjectCollection (Type::TRIGGERACTION_COLLECTION)
{
}

TriggerActionCollection::~TriggerActionCollection ()
{
}

//
// MultiScaleSubImage Collection
//
MultiScaleSubImageCollection::MultiScaleSubImageCollection ()
	: DependencyObjectCollection (Type::MULTISCALESUBIMAGE_COLLECTION)
{
	z_sorted = g_ptr_array_new ();
}

MultiScaleSubImageCollection::~MultiScaleSubImageCollection ()
{
	g_ptr_array_free (z_sorted, true);
}

static int
MultiScaleSubImageZIndexComparer (gconstpointer msisi1, gconstpointer msisi2)
{
	int z1 = (*((MultiScaleSubImage**)msisi1))->GetZIndex ();
	int z2 = (*((MultiScaleSubImage**)msisi2))->GetZIndex ();
	
	return z1 - z2;
}

void
MultiScaleSubImageCollection::ResortByZIndex ()
{
	g_ptr_array_set_size (z_sorted, array->len);
	
	if (array->len == 0)
		return;
	
	Types *types = GetDeployment()->GetTypes ();
	for (guint i = 0; i < array->len; i++)
		z_sorted->pdata[i] = ((Value *) array->pdata[i])->AsMultiScaleSubImage (types);
	
	if (array->len > 1)
		g_ptr_array_sort (z_sorted, MultiScaleSubImageZIndexComparer);
}

bool
MultiScaleSubImageCollection::Clear ()
{
	g_ptr_array_set_size (z_sorted, 0);
	return DependencyObjectCollection::Clear ();
}

//
// ResourceDictionaryCollection
//

ResourceDictionaryCollection::ResourceDictionaryCollection ()
	: DependencyObjectCollection (Type::RESOURCE_DICTIONARY_COLLECTION)
{
}

ResourceDictionaryCollection::~ResourceDictionaryCollection ()
{
}

static bool
WalkSubtreeLookingForCycle (ResourceDictionary* subtree_root,
			    ResourceDictionary* first_ancestor,
			    MoonError *error)
{
	const char *source = subtree_root->GetInternalSource();

	DependencyObject *p = first_ancestor;
	while (p) {
		if (p->Is (Type::RESOURCE_DICTIONARY)) {
			bool cycle_found = false;
			ResourceDictionary* rd = (ResourceDictionary*)p;
			const char *rd_source = rd->GetInternalSource();

			if (rd == subtree_root)
				cycle_found = true;
			else if (source && rd_source && !strcmp (rd_source, source))
				cycle_found = true;

			if (cycle_found) {
				MoonError::FillIn (error,
						   MoonError::INVALID_OPERATION, /* FIXME: verify exception type */
						   "cycle found in resource dictionaries");
				return false;
			}
		}
		p = p->GetParent();
	}

	ResourceDictionaryCollection *children = subtree_root->GetMergedDictionaries();
	for (int i = 0; i < children->GetCount(); i ++) {
		if (!WalkSubtreeLookingForCycle (children->GetValueAt(i)->AsResourceDictionary(), first_ancestor, error))
			return false;
	}

	return true;
}

bool
ResourceDictionaryCollection::AddedToCollection (Value *value, MoonError *error)
{
	if (!DependencyObjectCollection::AddedToCollection (value, error))
		return false;

	DependencyObject *parent = GetParent();
	if (!parent)
		return TRUE;

	ResourceDictionary *parent_rd = (ResourceDictionary*)parent;
	ResourceDictionary *rd = value->AsResourceDictionary();

	return WalkSubtreeLookingForCycle (rd, parent_rd, error);
}

//
// AudioCaptureDeviceCollection
//

AudioCaptureDeviceCollection::AudioCaptureDeviceCollection ()
	: DependencyObjectCollection (Type::AUDIOCAPTUREDEVICE_COLLECTION)
{
}

//
// VideoCaptureDeviceCollection
//

VideoCaptureDeviceCollection::VideoCaptureDeviceCollection ()
	: DependencyObjectCollection (Type::VIDEOCAPTUREDEVICE_COLLECTION)
{
}

//
// CollectionIterator
//

CollectionIterator::CollectionIterator (Collection *c)
{
	generation = c->Generation ();
	collection = c;
	collection->ref ();
	index = -1;
}

CollectionIterator::~CollectionIterator ()
{
	collection->unref ();
}

bool
CollectionIterator::Next (MoonError *err)
{
	if (generation != collection->Generation ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "The underlying collection has mutated");
		return false;
	}
	
	index++;
	
	return index < collection->GetCount ();
}

bool
CollectionIterator::Reset ()
{
	if (generation != collection->Generation ())
		return false;
	
	index = -1;
	
	return true;
}

Value *
CollectionIterator::GetCurrent (MoonError *err)
{
	if (generation != collection->Generation ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "The underlying collection has mutated");
		return NULL;
	}
	
	if (index < 0 || index >= collection->GetCount ()) {
		MoonError::FillIn (err, MoonError::INVALID_OPERATION, "Index out of bounds");
		return NULL;
	}
	
	return collection->GetValueAt (index);
}

void
CollectionIterator::Destroy (CollectionIterator *iterator)
{
	delete iterator;
}


//
// VisualTreeWalker
//

VisualTreeWalker::VisualTreeWalker (UIElement *obj, VisualTreeWalkerDirection dir, bool ref_content, Types *cached)
{
	index = 0;
	collection = NULL;
	content = obj->GetSubtreeObject ();
	direction = dir;
	this->ref_content = ref_content;
	types = (cached == NULL) ? obj->GetDeployment ()->GetTypes () : cached;

	if (content != NULL) {
		if (types->IsSubclassOf (content->GetObjectType (), Type::COLLECTION)) {
			collection = (Collection *)content;

			if (!types->IsSubclassOf (content->GetObjectType (), Type::UIELEMENT_COLLECTION))
				direction = Logical;
		}

		if (ref_content)
			content->ref ();
	}
}

UIElement *
VisualTreeWalker::Step ()
{
	UIElement *result = NULL;

	if (collection) {
		UIElementCollection *uiecollection = NULL;
		int count = GetCount ();

		if (count < 0 || index >= count)
			return NULL;
		
		if (count == 1 && index == 0) {
			index ++;
			return collection->GetValueAt (0)->AsUIElement(types);
		}

		if (direction == ZForward || direction == ZReverse) {
			uiecollection = (UIElementCollection *)collection;
			
			if ((int)uiecollection->z_sorted->len != count) {
#if SANITY
				printf ("VisualTreeWalker: unexpectedly got an unsorted UIElementCollection\n");
#endif
				uiecollection->ResortByZIndex ();
			}
		}

		switch (direction) {
		case ZForward:
			result = (UIElement*)uiecollection->z_sorted->pdata[index];
			break;
		case ZReverse:
			result = (UIElement *)uiecollection->z_sorted->pdata[count - (index + 1)];
			break;
		case Logical: {
			Value *v = collection->GetValueAt (index);
			result = v == NULL ? NULL : v->AsUIElement (types);
			break;
		}
		case LogicalReverse: {
			Value *v = collection->GetValueAt (count - (index + 1));
			result = v == NULL ? NULL : v->AsUIElement (types);
			break;
		}
		}
		
		index++;
	} else {
		if (index == 0) {
			index++;
			result = (UIElement *)content;
		} else { 
			result = NULL;
		}
	}

	return result;
}

int
VisualTreeWalker::GetCount ()
{
	if (!content)
		return 0;

	if (!collection)
		return 1;

	return collection->GetCount ();
}

VisualTreeWalker::~VisualTreeWalker()
{
	if (content && ref_content)
		content->unref ();
}


class UnsafeUIElementNode : public List::Node {
public:
	UIElement *uielement;
	
	UnsafeUIElementNode (UIElement *el) { uielement = el; }
};

DeepTreeWalker::DeepTreeWalker (UIElement *top, VisualTreeWalkerDirection direction, Types *types)
{
	walk_list = new List ();
	walk_list->Append (new UnsafeUIElementNode (top));
	last = NULL;
	this->types = types ? types : top->GetDeployment ()->GetTypes ();
	this->direction = direction;
}

UIElement *
DeepTreeWalker::Step ()
{
	if (last) {
		VisualTreeWalker walker (last, direction, types);
		//VisualTreeWalker walker (last, ZForward, types);
		UnsafeUIElementNode *prepend = (UnsafeUIElementNode *) walk_list->First ();
		while (UIElement *child = walker.Step ())
			walk_list->InsertBefore (new UnsafeUIElementNode (child), prepend);
	}

	UnsafeUIElementNode *next = (UnsafeUIElementNode*)walk_list->First ();
	
	if (!next) {
		last = NULL;
		return NULL;
	}

	UIElement *current = next->uielement;
	walk_list->Unlink (next);
	delete next;
	last = current;

	return current;
}

void
DeepTreeWalker::SkipBranch ()
{
	last = NULL;
}

DeepTreeWalker::~DeepTreeWalker ()
{
	delete walk_list;
}

//
// Manual C-Bindings
//
Collection *
collection_new (Type::Kind kind)
{
	Type *t = Type::Find (Deployment::GetCurrent (), kind);
	
	if (!t->IsSubclassOf (Type::COLLECTION)) {
		g_warning ("create_collection passed non-collection type");
		return NULL;
	}
	
	return (Collection *) t->CreateInstance();
}

};
