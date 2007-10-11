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
#include "clock.h"

#if OBJECT_TRACKING
// Define the ID of the object you want to track
// Object creation, destruction and all ref/unrefs
// are logged to the console, with a stacktrace.
#define OBJECT_TRACK_ID (-1)

int Base::objects_created = 0;
int Base::objects_destroyed = 0;
GList* Base::objects_alive = NULL;

void
Base::Track (const char* done, const char* typname)
{
#if OBJECT_TRACK_ID
	if (id == OBJECT_TRACK_ID) {
		printf ("%s tracked object of type '%s': %i, current refcount: %i\n", done, typname, id, refcount);
		PrintStackTrace ();
	}
#endif
}

char* 
Base::GetStackTrace (const char* prefix)
{
	return get_stack_trace_prefix (prefix);
}

void
Base::PrintStackTrace ()
{
	print_stack_trace ();
}
#endif

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

static GStaticRecMutex delayed_unref_mutex = G_STATIC_REC_MUTEX_INIT;
static bool drain_tick_call_added = false;
static GSList *pending_unrefs = NULL;

static void
drain_unrefs_tick_call (gpointer data)
{
	g_static_rec_mutex_lock (&delayed_unref_mutex);
	drain_unrefs ();
	drain_tick_call_added = false;
	g_static_rec_mutex_unlock (&delayed_unref_mutex);
}

void
base_unref_delayed (Base *base)
{
	if (!base)
		return;
		
	g_static_rec_mutex_lock (&delayed_unref_mutex);
	pending_unrefs = g_slist_prepend (pending_unrefs, base);

#if OBJECT_TRACKING
	base->Track ("DelayedUnref", base->GetTypeName ());
#endif

	if (!drain_tick_call_added) {
		time_manager_add_tick_call (drain_unrefs_tick_call, NULL);
		drain_tick_call_added = true;
	}
	g_static_rec_mutex_unlock (&delayed_unref_mutex);
}

void
drain_unrefs ()
{
	g_static_rec_mutex_lock (&delayed_unref_mutex);
	g_slist_foreach (pending_unrefs, (GFunc)base_unref, NULL);
	g_slist_free (pending_unrefs);
	pending_unrefs = NULL;
	g_static_rec_mutex_unlock (&delayed_unref_mutex);
}

GHashTable *DependencyObject::properties = NULL;

typedef struct {
	DependencyObject *dob;
	DependencyProperty *prop;
} Attacher;

//
// Attaches the dependency object to the container on the given property.
//
void
DependencyObject::Attach (DependencyProperty *property, DependencyObject *container)
{
	Attacher *att = new Attacher ();
	att->dob = container;
	att->prop = property;
	attached_list = g_slist_append (attached_list, att);
}

void
DependencyObject::Detach (DependencyProperty *property, DependencyObject *container)
{
	for (GSList *l = attached_list; l; l = l->next) {
		Attacher *att = (Attacher*)l->data;

		if (att->dob == container && (property == NULL || att->prop == property)) {
			attached_list = g_slist_remove_link (attached_list, l);
			delete att;
		}
	}
}

static void
detach_depobj_values (gpointer  key,
		      gpointer  value,
		      gpointer  user_data)
{
	DependencyObject *this_obj = (DependencyObject*)user_data;
	//DependencyProperty *prop = (DependencyProperty*)key;
	Value *v = (Value*)value;

	if (v != NULL && v->GetKind() >= Type::DEPENDENCY_OBJECT && v->AsDependencyObject() != NULL) {
		//printf ("detaching from property %s\n", prop->name);
		v->AsDependencyObject()->Detach (NULL, this_obj);
	}
}

void
DependencyObject::DetachAll ()
{
	g_hash_table_foreach (current_values, detach_depobj_values, this);
}

static bool attachers_notified;

void
DependencyObject::NotifyAttachersOfPropertyChange (DependencyProperty *subproperty)
{
	attachers_notified = true;
	for (GSList *l = attached_list; l != NULL; l = l->next){
		Attacher *att = (Attacher*)l->data;

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

	Value *current_value = (Value*)g_hash_table_lookup (current_values, property);

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

		g_hash_table_insert (current_values, property, store);

		if (value) {
			if (value->GetKind () >= Type::DEPENDENCY_OBJECT){
				DependencyObject *dob = value->AsDependencyObject();
				
				if (dob != NULL)
					dob->Attach (property, this);
			}
		}

		attachers_notified = false;

		OnPropertyChanged (property);

		if (!attachers_notified)
			g_warning ("setting property %s::%s on object of type %s didn't result in attachers being notified\n",
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
	found = g_hash_table_lookup_extended (current_values, property, NULL, &value);

	if (found)
		return (Value*)value;

	return property->default_value;
}

Value *
DependencyObject::GetValueNoDefault (DependencyProperty *property)
{
	Value *value = NULL;

	value = (Value *) g_hash_table_lookup (current_values, property);

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
DependencyObject::ClearValue (DependencyProperty *property)
{
	Value *current_value = GetValueNoDefault (property);

	if (current_value == NULL) {
		/* the property has the default value, nothing to do */
		return;
	}

	// much of this logic is c&p from SetValue.

	if (current_value->GetKind () >= Type::DEPENDENCY_OBJECT){
		DependencyObject *dob = current_value->AsDependencyObject();

		if (dob != NULL)
			dob->Detach (property, this);
	}

	g_hash_table_remove (current_values, property);

	attachers_notified = false;

	OnPropertyChanged (property);

	if (!attachers_notified)
		g_warning ("setting property %s::%s on object of type %s didn't result in attachers being notified\n",
			   Type::Find(property->type)->name, property->name, Type::Find(GetObjectType())->name);

	if (property->is_attached_property)
		NotifyParentOfPropertyChange (property, true);
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
	current_values = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, free_value);
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

void free_attacher (gpointer data, gpointer user_data)
{
	Attacher* att = (Attacher*) data;
	delete att;
}

DependencyObject::~DependencyObject ()
{
	if (attached_list != NULL) {
		g_slist_foreach (attached_list, free_attacher, NULL);
		g_slist_free (attached_list);
	}

	DetachAll ();
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

	if (scope && !scope->GetMerged())
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
	if (obj == NULL)
		return NULL;
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

static gboolean
strcase_equal (gconstpointer  v1,
	       gconstpointer  v2)
{
	return !g_strcasecmp ((char*)v1, (char*)v2);
}

static guint
strcase_hash (gconstpointer v)
{
	char *case_v = g_strdup ((char*)v);

	for (char *p = case_v; *p != 0; p ++)
		*p = g_ascii_tolower (*p);

	guint rv = g_str_hash (case_v);
	g_free (case_v);
	return rv;
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
		table = g_hash_table_new_full (strcase_hash, strcase_equal,
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

DependencyObject*
DependencyObject::GetParent ()
{
	DependencyObject *res = parent;
	while (res && Type::Find (res->GetObjectType ())->IsSubclassOf (Type::COLLECTION))
		res = res->GetParent ();
	return res;
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

			if (!t) {
				g_free (propn);
				return NULL;
			}
	
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

	NotifyAttachersOfPropertyChange (property);
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
}

static void
deleter (gpointer data)
{
	delete (EventClosure *) data;
}

EventObject::~EventObject ()
{
	if (events) {
		for (int i = 0; i < GetType()->GetEventCount(); i ++)
			g_slist_foreach (events[i], (GFunc)deleter, NULL);
		g_free (events);
	}
}

void
EventObject::AddHandler (char *event_name, EventHandler handler, gpointer data)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("adding handler to event '%s', which has not been registered\n", event_name);
		return;
	}

	AddHandler (id, handler, data);
}

void
EventObject::AddHandler (int event_id, EventHandler handler, gpointer data)
{ 
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("adding handler to event with id %d, which has not been registered\n", event_id);
		return;
	}

	if (events == NULL) {
		events = (GSList**)g_new0 (GSList*, GetType()->GetEventCount());
	}

	EventClosure *closure = new EventClosure ();
	closure->func = handler;
	closure->data = data;

	events[event_id] = g_slist_append (events[event_id], closure);
}

void
EventObject::RemoveHandler (char *event_name, EventHandler handler, gpointer data)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("removing handler for event '%s', which has not been registered\n", event_name);
		return;
	}

	RemoveHandler (id, handler, data);
}

void
EventObject::RemoveHandler (int event_id, EventHandler handler, gpointer data)
{
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return;
	}

	if (events == NULL)
		return;

	GSList *l;
	for (l = events[event_id]; l; l = l->next) {
		EventClosure *closure = (EventClosure*)l->data;
		if (closure->func == handler && closure->data == data)
			break;
	}

	if (l == NULL) /* we didn't find it */
		return;

	delete (EventClosure*)l->data;
	events[event_id] = g_slist_delete_link (events[event_id], l);
}

void
EventObject::Emit (char *event_name, gpointer calldata)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("trying to emit event '%s', which has not been registered\n", event_name);
		return;
	}

	Emit (id, calldata);
}

void
EventObject::Emit (int event_id, gpointer calldata)
{
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("trying to emit event with id %d, which has not been registered\n", event_id);
		return;
	}

	if (events == NULL || events[event_id] == NULL)
		return;
	
	GSList *copy = g_slist_copy (events[event_id]);

	for (GSList *l = copy; l; l = l->next) {
		EventClosure *closure = (EventClosure*)l->data;
		closure->func (this, calldata, closure->data);
	}

	g_slist_free (copy);
}

void
event_object_add_event_handler (EventObject *o, char *event, EventHandler handler, gpointer closure)
{
	o->AddHandler (event, handler, closure);
}

void
event_object_remove_event_handler (EventObject *o, char *event, EventHandler handler, gpointer closure)
{
	o->RemoveHandler (event, handler, closure);
}
