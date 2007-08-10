/*
 * dependencyobject.h: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "namescope.h"
#include "list.h"
#include "collection.h"
#include "dependencyobject.h"

void 
Base::ref ()
{
	if (refcount & BASE_FLOATS)
		refcount = 1;
	else
		refcount++;

#if DEBUG_REFCNT
	printf ("refcount++ %p (%s), refcnt = %d\n", this,
		Type::Find (((DependencyObject *) this)->GetObjectType ())->name, refcount);
#endif
}

void
Base::unref ()
{
	if (refcount == BASE_FLOATS || refcount == 1) {
#if DEBUG_REFCNT
		printf ("destroying object %p (%s)\n", this,
			Type::Find (((DependencyObject *) this)->GetObjectType ())->name);
#endif
		delete this;
	} else {
		refcount--;
#if DEBUG_REFCNT
		printf ("refcount-- %p (%s), refcnt = %d\n", this,
			Type::Find (((DependencyObject *) this)->GetObjectType ())->name,
			refcount);
#endif
	}
}

void 
base_ref (Base *base)
{
	if (base != NULL)
		base->ref ();
}
 
void
base_unref (Base *base)
{
	if (base != NULL)
		base->unref ();
}

GHashTable *DependencyObject::properties = NULL;

typedef struct {
	DependencyObject *dob;
	DependencyProperty *prop;
} Attachee;

//
// Attaches the dependency object to the container on the given property.
//
void
DependencyObject::Attach (DependencyProperty *property, DependencyObject *container)
{
	Attachee *att = new Attachee ();
	att->dob = container;
	att->prop = property;
	attached_list = g_slist_append (attached_list, att);
}

void
DependencyObject::Detach (DependencyProperty *property, DependencyObject *container)
{
	for (GSList *l = attached_list; l; l = l->next) {
		Attachee *att = (Attachee*)l->data;

		if (att->dob == container && att->prop == property) {
			attached_list = g_slist_remove_link (attached_list, l);
			delete att;
			break;
		}
	}
}

static bool attachees_notified;

void
DependencyObject::NotifyAttacheesOfPropertyChange (DependencyProperty *subproperty)
{
	attachees_notified = true;
	for (GSList *l = attached_list; l != NULL; l = l->next){
		Attachee *att = (Attachee*)l->data;

		att->dob->OnSubPropertyChanged (att->prop, subproperty);
	}
}

void
DependencyObject::SetValue (DependencyProperty *property, Value *value)
{
	g_return_if_fail (property != NULL);

	if (value != NULL){
		if (!Type::Find (value->GetKind ())->IsSubclassOf (property->value_type)) {
			g_warning ("DependencyObject::SetValue, value cannot be assigned to the property %s::%s (property has type '%s', value has type '%s')\n", GetTypeName (), property->name, Type::Find (property->value_type)->name, Type::Find (value->GetKind ())->name);
			return;
		}
	} else {
		if (!(property->value_type >= Type::DEPENDENCY_OBJECT) && !property->IsNullable ()) {
			g_warning ("Can not set a non-nullable scalar type to NULL");
			return;
		}
	}

	Value *current_value = (Value*)g_hash_table_lookup (current_values, property->name);

	if (current_value != NULL && current_value->GetKind () >= Type::DEPENDENCY_OBJECT) {
		DependencyObject *current_as_dep = current_value->AsDependencyObject ();
		if (current_as_dep)
			current_as_dep->SetParent (NULL);
	}
	if (value != NULL && value->GetKind () >= Type::DEPENDENCY_OBJECT) {
		DependencyObject *new_as_dep = value->AsDependencyObject ();
		
		new_as_dep->SetParent (this);
	}

	if ((current_value == NULL && value != NULL) ||
	    (current_value != NULL && value == NULL) ||
	    (current_value != NULL && value != NULL && *current_value != *value)) {

		if (current_value != NULL && current_value->GetKind () >= Type::DEPENDENCY_OBJECT){
			DependencyObject *dob = current_value->AsDependencyObject();

			if (dob != NULL)
				dob->Detach (property, this);
		}

		Value *store;
		if (value == NULL) {
			store = NULL;
		} else {
			store = new Value (*value);
		}

		g_hash_table_insert (current_values, property->name, store);

		if (value) {
			if (value->GetKind () >= Type::DEPENDENCY_OBJECT){
				DependencyObject *dob = value->AsDependencyObject();
				
				if (dob != NULL)
					dob->Attach (property, this);
			}
		}

		attachees_notified = false;

		OnPropertyChanged (property);

		if (!attachees_notified)
			g_warning ("setting property %s::%s on object of type %s didn't result in attachees being notified\n",
				   Type::Find(property->type)->name, property->name, Type::Find(GetObjectType())->name);

		if (property->is_attached_property)
			NotifyParentOfPropertyChange (property, true);
	}
}

void
DependencyObject::SetValue (DependencyProperty *property, Value value)
{
	SetValue (property, &value);
}

Value *
DependencyObject::GetValue (DependencyProperty *property)
{
	void *value = NULL;

	bool found;
	found = g_hash_table_lookup_extended (current_values, property->name, NULL, &value);

	if (found)
		return (Value*)value;

	return property->default_value;
}

Value *
DependencyObject::GetValueNoDefault (DependencyProperty *property)
{
	Value *value = NULL;

	value = (Value *) g_hash_table_lookup (current_values, property->name);

	if (value != NULL)
		return value;

	return NULL;
}

Value *
DependencyObject::GetValue (const char *name)
{
	DependencyProperty *property;
	property = GetDependencyProperty (name);

	if (property == NULL) {
		g_warning ("This object (of type '%s') doesn't have a property called '%s'\n", GetType ()->name, name);
		return NULL;
	}

	return GetValue (property);
}

void 
DependencyObject::SetValue (const char *name, Value value)
{
	SetValue (name, &value);
}

void
DependencyObject::SetValue (const char *name, Value *value)
{
	DependencyProperty *property;
	property = GetDependencyProperty (name);

	if (property == NULL) {
		g_warning ("This object (of type '%s') doesn't have a property called '%s'\n", GetType ()->name, name);
		return;
	}

	SetValue (property, value);
}

static void
free_value (void *v)
{
	delete (Value*)v;
}

DependencyObject::DependencyObject ()
{
	current_values = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, free_value);
	this->attached_list = NULL;
	this->parent = NULL;
}

static void
dump (gpointer key, gpointer value, gpointer data)
{
	printf ("%s\n", (char*)key);
}

Type::Kind
DependencyObject::GetObjectType ()
{
	g_warning ("%p This class is missing an override of GetObjectType ()", this);
	g_hash_table_foreach (current_values, dump, NULL);
	return Type::DEPENDENCY_OBJECT; 
}

DependencyObject::~DependencyObject ()
{
	g_hash_table_destroy (current_values);
}

DependencyProperty *
DependencyObject::GetDependencyProperty (const char *name)
{
	return DependencyObject::GetDependencyProperty (GetObjectType (), name);
}

DependencyProperty *
DependencyObject::GetDependencyProperty (Type::Kind type, const char *name)
{
	return GetDependencyProperty (type, name, true);
}

DependencyProperty *
DependencyObject::GetDependencyProperty (Type::Kind type, const char *name, bool inherits)
{
	GHashTable *table;
	DependencyProperty *property = NULL;

	if (properties == NULL)
		return NULL;

	table = (GHashTable*) g_hash_table_lookup (properties, &type);

	if (table)
		property = (DependencyProperty*) g_hash_table_lookup (table, name);

	if (property != NULL)
		return property;

	if (!inherits)
		return NULL;	

	// Look in the parent type
	Type *current_type;
	current_type = Type::Find (type);
	
	if (current_type == NULL)
		return NULL;
	
	if (current_type->parent == Type::INVALID)
		return NULL;

	return GetDependencyProperty (current_type->parent, name);
}

bool
DependencyObject::HasProperty (const char *name, bool inherits)
{
	return GetDependencyProperty (GetObjectType (), name, inherits) != NULL;
}

DependencyObject*
DependencyObject::FindName (const char *name)
{
	NameScope *scope = NameScope::GetNameScope (this);
	DependencyObject *rv = NULL;

	if (scope)
		rv = scope->FindName (name);

	if (rv)
		return rv;
	else if (parent)
		return parent->FindName (name);
	else
		return NULL;
}

NameScope*
DependencyObject::FindNameScope ()
{
	NameScope *scope = NameScope::GetNameScope (this);

	if (scope)
		return scope;

	if (parent)
		return parent->FindNameScope ();

	return NULL;
}
		 
DependencyObject *
dependency_object_find_name (DependencyObject *obj, const char *name, Type::Kind *element_kind)
{
	//printf ("Looking up in %p the string %p\n", obj, name);
	//printf ("        String: %s\n", name);
	DependencyObject *ret = obj->FindName (name);

	if (ret == NULL)
		return NULL;

	*element_kind = ret->GetObjectType ();

	return ret;
}

//
//  A helper debugging routine for C#
//
const char *
dependency_object_get_name (DependencyObject *obj)
{
	return obj->GetName ();
}

Type::Kind
dependency_object_get_object_type (DependencyObject *obj)
{
	return obj->GetObjectType ();
}

const char *
dependency_object_get_type_name (DependencyObject *obj)
{
	return obj->GetTypeName ();
}

void
dependency_object_add_event_handler (DependencyObject *o, char *event, EventHandler handler, gpointer closure)
{
	o->AddHandler (event, handler, closure);
}

void
dependency_object_remove_event_handler (DependencyObject *o, char *event, EventHandler handler, gpointer closure)
{
	o->RemoveHandler (event, handler, closure);
}


//
// Use this for values that can be null
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Type::Kind vtype)
{
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, NULL, vtype, false);
}

//
// DependencyObject takes ownership of the Value * for default_value
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Value *default_value)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, default_value->GetKind (), false);
}

static void
free_property_hash (gpointer v)
{
	g_hash_table_destroy ((GHashTable*)v);
}

static void
free_property (gpointer v)
{
	delete (DependencyProperty*)v;
}

//
// DependencyObject takes ownership of the Value * for default_value
// This overload can be used to set the type of the property to a different type
// than the default value (the default value can for instance be a SolidColorBrush
// while the property type can be a Brush).
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, vtype, false);
}

DependencyProperty *
DependencyObject::RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype)
{
	DependencyProperty *property;
	property = Register (type, name, vtype);
	property->is_nullable = true;
	return property;
}

//
// Register the dependency property that belongs to @type with the name @name
// The default value is @default_value (if provided) and the type that can be
// stored in the dependency property is of type @vtype
//
DependencyProperty *
DependencyObject::RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached)
{
	GHashTable *table;

	DependencyProperty *property = new DependencyProperty (type, name, default_value, vtype, attached);
	
	/* first add the property to the global 2 level property hash */
	if (properties == NULL)
		properties = g_hash_table_new_full (g_int_hash, g_int_equal,
						    NULL, free_property_hash);

	table = (GHashTable*) g_hash_table_lookup (properties, &property->type);

	if (table == NULL) {
		table = g_hash_table_new_full (g_str_hash, g_str_equal,
					       NULL, free_property);
		g_hash_table_insert (properties, &property->type, table);
	}

	g_hash_table_insert (table, property->name, property);

	return property;
}

void
DependencyObject::Shutdown ()
{
	g_hash_table_destroy (DependencyObject::properties);
	DependencyObject::properties = NULL;
}

DependencyObject *
DependencyObject::GetParent ()
{
	return parent;
}

void
DependencyObject::SetParent (DependencyObject *parent)
{
#if DEBUG
	// Check for circular families
	DependencyObject *current = parent;
	do while (current != NULL) {
		g_assert (current != this);
		current = current->GetParent ();
	} 
#endif
	this->parent = parent;
}

void
DependencyObject::NotifyParentOfPropertyChange (DependencyProperty *property, bool only_exact_type)
{
	DependencyObject *current = GetParent ();
	while (current != NULL) {
		if (!only_exact_type || property->type == current->GetObjectType ()) {	

			// Only handle up to the first one that catches the attached change
			if (only_exact_type && current->OnChildPropertyChanged (property, this))
				return;
		}
		current = current->GetParent ();
	}
}

Value *
dependency_object_get_value (DependencyObject *object, DependencyProperty *prop)
{
	if (object == NULL)
		return NULL;

	return object->GetValue (prop);
}

Value *
dependency_object_get_value_no_default (DependencyObject *object, DependencyProperty *prop)
{
	if (object == NULL)
		return NULL;
	
	return object->GetValueNoDefault (prop);
}

void
dependency_object_set_value (DependencyObject *object, DependencyProperty *prop, Value *val)
{
	if (object == NULL)
		return;

	object->SetValue (prop, val);
}

/*
 *	DependencyProperty
 */
DependencyProperty::DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached)
{
	this->type = type;
	this->name = g_strdup (name);
	this->default_value = default_value;
	this->value_type = value_type;
	this->is_attached_property = attached;
}

DependencyProperty::~DependencyProperty ()
{
	g_free (name);
	if (default_value != NULL)
		delete default_value;
}

DependencyProperty *dependency_property_lookup (Type::Kind type, char *name)
{
	return DependencyObject::GetDependencyProperty (type, name);
}

char*
dependency_property_get_name (DependencyProperty *property)
{
	return property->name;
}

Type::Kind
dependency_property_get_value_type (DependencyProperty *property)
{
	return property->value_type;
}

bool
dependency_property_is_nullable (DependencyProperty *property)
{
	return property->IsNullable ();
}


//
// Everything inside of a ( ) resolves to a DependencyProperty, if there is a
// '.' after the property, we get the object, and continue resolving from there
// if there is a [n] after the property, we convert the property to a collection
// and grab the nth item.
//
// Dependency properties can be specified as (PropertyName) of the current object
// or they can be specified as (DependencyObject.PropertyName).
//
// Returns NULL on any error
//
DependencyProperty *
resolve_property_path (DependencyObject **o, const char *path)
{
	g_assert (o);
	g_assert (path);

	int c;
	int len = strlen (path);
	char *typen = NULL;
	char *propn = NULL;
	bool expression_found = false;
	DependencyProperty *res = NULL;
	DependencyObject *lu = *o;
	const char *prop = path;
	
	for (int i = 0; i < len; i++) {
		switch (path [i]) {
		case '(':
		{
			expression_found = true;

			typen = NULL;
			propn = NULL;
			int estart = i + 1;
			for (c = estart; c < len; c++) {
				if (path [c] == '.') {
					typen = strndup (path + estart, c - estart);
					estart = c + 1;
					continue;
				}
				if (path [c] == ')') {
					propn = strndup (path + estart, c - estart);
					break;
				}
			}

			i = c;
			
			Type *t = NULL;
			if (typen) {
				t = Type::Find (typen);
				g_free (typen);
			} else
				t = Type::Find (lu->GetObjectType ());

			res = DependencyObject::GetDependencyProperty (t->type, propn);
			g_free (propn);
			break;
		}
		case '.':
			lu = lu->GetValue (res)->AsDependencyObject ();
			expression_found = false;
			prop = path + (i + 1);
			// we can ignore this, since we pull the lookup object when we finish a ( ) block
			break;
		case '[':
		{
			int indexer = 0;

			// Need to be a little more loving
			g_assert (path [i + 1]);

			char *p;

			indexer = strtol (path + i + 1, &p, 10);
			i = p - path;

			g_assert (path [i] == ']');
			g_assert (path [i + 1] == '.');

			Collection *col = lu->GetValue (res)->AsCollection ();
			List::Node *n = col->list->Index (indexer);
			lu = n ? ((Collection::Node *) n)->obj : NULL;
			i += 1;
			break;
		}
		}
	}

	if (!expression_found)
		res = DependencyObject::GetDependencyProperty (lu->GetObjectType (), prop);

	*o = lu;
	return res;
}

DependencyProperty* DependencyObject::NameProperty;

void
DependencyObject::OnPropertyChanged (DependencyProperty *property)
{
	if (NameProperty == property) {
		NameScope *scope = FindNameScope ();
		Value *v = GetValue (NameProperty);
		if (scope && v)
			scope->RegisterName (v->AsString (), this);
	}

	NotifyAttacheesOfPropertyChange (property);
}

void
dependencyobject_init(void)
{
	DependencyObject::NameProperty = DependencyObject::Register (Type::DEPENDENCY_OBJECT, "Name", Type::STRING);
}


// event handlers for c++
typedef struct {
	EventHandler func;
	gpointer data;
} EventClosure;

EventObject::EventObject ()
{
	events = NULL;
	event_name_hash = NULL;
}

static void
deleter (gpointer data)
{
	delete (EventClosure *) data;
}

static void
event_list_delete (gpointer l)
{
	g_list_foreach ((GList*)l, (GFunc)deleter, NULL);
}

EventObject::~EventObject ()
{
	if (event_name_hash) {
		g_hash_table_foreach (event_name_hash, (GHFunc)g_free, NULL);
		g_hash_table_destroy (event_name_hash);
	}
	if (events) {
		g_ptr_array_foreach (events, (GFunc)event_list_delete, NULL);
	}
}

void
EventObject::AddHandler (char *event_name, EventHandler handler, gpointer data)
{
	gpointer key, value;

	//printf ("**** Removing handler %s pointing to %p with data %p\n");
	if (!event_name_hash ||
	    !g_hash_table_lookup_extended (event_name_hash, event_name,
					   &key, &value)) {
		g_warning ("adding handler to event '%s', which has not been registered\n", event_name);
		return;
	}

	int id = GPOINTER_TO_INT (value);

	AddHandler (id, handler, data);
}

void
EventObject::AddHandler (int event_id, EventHandler handler, gpointer data)
{ 
	if (!events) {
		g_warning ("adding handler to event with id %d, which has not been registered\n", event_id);
		return;
	}

	GList *event_list = (GList*)g_ptr_array_index (events, event_id);

	EventClosure *closure = new EventClosure ();
	closure->func = handler;
	closure->data = data;

	event_list = g_list_append (event_list, closure);

	g_ptr_array_index (events, event_id) = event_list;
}

void
EventObject::RemoveHandler (char *event_name, EventHandler handler, gpointer data)
{
	gpointer key, value;

	//printf ("**** Removing handler %s pointing to %p with data %p\n");
	if (!event_name_hash ||
	    !g_hash_table_lookup_extended (event_name_hash, event_name,
					   &key, &value)) {
		g_warning ("removing handler for event '%s', which has not been registered\n", event_name);
		return;
	}

	int id = GPOINTER_TO_INT (value);

	RemoveHandler (id, handler, data);
}

void
EventObject::RemoveHandler (int event_id, EventHandler handler, gpointer data)
{
	if (!events) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return;
	}

	GList *event_list = (GList*)g_ptr_array_index (events, event_id);

	GList *l;
	for (l = event_list; l; l = l->next) {
		EventClosure *closure = (EventClosure*)l->data;
		if (closure->func == handler && closure->data == data)
			break;
	}

	if (l == NULL) /* we didn't find it */
		return;

	delete (EventClosure*)l->data;
	event_list = g_list_delete_link (event_list, l);

	g_ptr_array_index (events, event_id) = event_list;
}

int
EventObject::RegisterEvent (const char *event_name)
{
	if (events == NULL) {
		event_name_hash = g_hash_table_new (g_str_hash, g_str_equal);
		events = g_ptr_array_new ();
	}

	int id = events->len;

	g_hash_table_insert (event_name_hash, g_strdup (event_name), GINT_TO_POINTER (id));

	g_ptr_array_add (events, NULL);

	return id;
}

void
EventObject::Emit (char *event_name, gpointer calldata)
{
	if (events == NULL) {
		g_warning ("trying to emit event '%s', which has not been registered\n", event_name);
		return;
	}

	gpointer key, value;
	if (!g_hash_table_lookup_extended (event_name_hash,
					   event_name,
					   &key,
					   &value))
		return;

	int id = GPOINTER_TO_INT (value);

	Emit (id, calldata);
}

void
EventObject::Emit (int event_id, gpointer calldata)
{
	if (events == NULL) {
		g_warning ("trying to emit event with id %d, which has not been registered\n", event_id);
		return;
	}

	GList *event_list = (GList*)g_ptr_array_index (events, event_id);

	if (event_list == NULL)
		return;
	
	GList *copy = g_list_copy (event_list);

	for (GList *l = copy; l; l = l->next) {
		EventClosure *closure = (EventClosure*)l->data;
		closure->func (this, calldata, closure->data);
	}

	g_list_free (copy);
}
