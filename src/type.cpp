/*
 * type.cpp: Our type system
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>

#include "type.h"
#include "runtime.h"
#include "deployment.h"
#include "dependencyproperty.h"

/*
 * Type implementation
 */
Type::Type (Type::Kind type, Type::Kind parent, bool value_type, const char *name, 
		const char *kindname, int event_count, int total_event_count, const char **events, 
		create_inst_func *create_inst, const char *content_property)
{
	this->type = type;
	this->parent = parent;
	this->value_type = value_type;
	this->name = name;
	this->kindname = kindname;
	this->event_count = event_count;
	this->total_event_count = total_event_count;
	this->events = events;
	this->create_inst = create_inst;
	this->content_property = content_property;
	this->properties = NULL;
	this->custom_properties_hash = NULL;
	this->custom_properties = NULL;
}
		
Type::~Type ()
{
	if (properties) {
		g_hash_table_destroy (properties);
		properties = NULL;
	}

	if (custom_properties_hash != NULL)
		g_hash_table_destroy (custom_properties_hash);
		
	if (custom_properties != NULL) {
		GSList *current = custom_properties;
		while (current != NULL) {
			delete (DependencyProperty *) current->data;
			current = current->next;
		}
		g_slist_free (custom_properties);
	}
}

Type *
Type::Clone ()
{
	Type *result = new Type ();
	
	result->type = type;
	result->parent = parent;
	result->value_type = value_type;
	result->name = g_strdup (name);
	result->kindname = g_strdup (kindname);
	result->event_count = event_count;
	result->total_event_count = total_event_count;
	result->events = events;
	result->create_inst = create_inst;
	result->content_property = g_strdup (content_property);
	result->properties = NULL;
	result->custom_properties = NULL;
	
	return result;
}

const char *
Type::LookupEventName (int id)
{
	Type *parent_type = Type::Find (parent);
	int parent_event_count = (parent_type == NULL ? 0 : parent_type->total_event_count);
	int current_id;
	const char *result;
	
	if (id < 0)
		return "";
		
	if (events != NULL) {
		for (int i = 0; events [i] != NULL; i++) {
			current_id = i + parent_event_count;
			if (current_id == id)
				return events [i];
		}
	}
	
	if (parent == Type::INVALID || parent_type == NULL) {
		printf ("Event lookup of event id %i in type '%s' failed.\n", id, name);
		return NULL;
	}
	
	result = parent_type->LookupEventName (id);

	if (result == NULL)
		printf ("Event lookup of event %i in (more exactly) type '%s' failed.\n", id, name);

	return result;
}

int
Type::LookupEvent (const char *event_name)
{
	Type *parent_type = Type::Find (parent);
	int result;

	if (events != NULL) {
		for (int i = 0; events [i] != NULL; i++) {
			if (!g_strcasecmp (events [i], event_name))
				return i + (parent_type == NULL ? 0 : parent_type->total_event_count);
		}
	}
	
	if (parent == Type::INVALID || parent_type == NULL) {
		printf ("Event lookup of event '%s' in type '%s' failed.\n", event_name, name);
		return -1;
	}

	result = parent_type->LookupEvent (event_name);

	if (result == -1)
		printf ("Event lookup of event '%s' in (more exactly) type '%s' failed.\n", event_name, name);

	return result;
}

bool
Type::IsSubclassOf (Type::Kind type, Type::Kind super)
{
	Type *t = Find (type);
	if (t == NULL)
		return false;
	return t->IsSubclassOf (super);
}

bool 
Type::IsSubclassOf (Type::Kind super)
{
	Type *parent_type;

	if (type == super)
		return true;

	if (parent == super)
		return true;

	if (parent == Type::INVALID || type == Type::INVALID)
		return false;

	parent_type = Type::Find (parent);
	
	if (parent_type == NULL)
		return false;
	
	return parent_type->IsSubclassOf (super);
}

Type *
Type::Find (const char *name)
{
	return Deployment::GetCurrent ()->GetTypes ()->Find (name);
}

Type *
Type::Find (const char *name, bool ignore_case)
{
	return Deployment::GetCurrent ()->GetTypes ()->Find (name, ignore_case);
}

Type *
Type::Find (Type::Kind type)
{
	if (type < Type::INVALID || type == Type::LASTTYPE)
		return NULL;
	
	return Deployment::GetCurrent ()->GetTypes ()->Find (type);
}

DependencyObject *
Type::CreateInstance ()
{
	if (!create_inst) {
		g_warning ("Unable to create an instance of type: %s\n", name);
		return NULL;
	}
	
	return create_inst ();
}

const char *
Type::GetContentPropertyName ()
{
	Type *parent_type;

	if (type == INVALID)
		return NULL;

	if (content_property)
		return content_property;

	parent_type = Find (parent);

	if (parent_type == NULL)
		return NULL;

	return parent_type->GetContentPropertyName ();
}

DependencyProperty *
Type::LookupProperty (const char *name)
{
	DependencyProperty *property = NULL;
	
	g_return_val_if_fail (name != NULL, NULL);
	
	if (properties != NULL) {
		char *key = g_ascii_strdown (name, -1);
		property = (DependencyProperty*) g_hash_table_lookup (properties, key);
		g_free (key);
		
		if (property)
			return property;
	}
	
	if (custom_properties_hash != NULL) {
		property = (DependencyProperty *) g_hash_table_lookup (custom_properties_hash, name);
		
		if (property != NULL)
			return property;
	}
	
	return NULL;
}

static void
free_property (gpointer v)
{
	delete (DependencyProperty *) v;
}

void
Type::AddProperty (DependencyProperty *property)
{
	g_return_if_fail (property != NULL);
	
	if (property->IsCustom ()) {
		// Managed code is allowed to register several properties with the same name
		// and they all get the callback called when the property value changes.
		// See comment in type.h.
		custom_properties = g_slist_prepend (custom_properties, property);
		if (custom_properties_hash == NULL)
			custom_properties_hash = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (custom_properties_hash, (gpointer) property->GetName (), property);
	} else {
		DependencyProperty *existing;
		if (properties == NULL)
			properties = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, free_property);

		if ((existing = (DependencyProperty *) g_hash_table_lookup (properties, property->GetHashKey ())) != NULL) {
			g_warning ("Type::AddProperty (): Trying to register the property '%s' (of type %s) in the owner type '%s', and there already is a property registered on that type with the same name.",
				   property->GetName (), Type::Find (property->GetPropertyType ())->GetName(), GetName());
		} else {
			g_hash_table_insert (properties, (gpointer) property->GetHashKey (), property);
		}
	}
}

bool
type_get_value_type (Type::Kind type)
{
	Type *t = Type::Find (type);
	
	if (t == NULL)
		return false;
	
	return t->IsValueType ();
}

bool
type_is_dependency_object (Type::Kind type)
{
	return Type::IsSubclassOf (type, Type::DEPENDENCY_OBJECT);
}

DependencyObject *
type_create_instance (Type *type)
{
	if (!type) {
		g_warning ("Unable to create instance of type %p.", type);
		return NULL;
	}

	return type->CreateInstance ();
}

DependencyObject *
type_create_instance_from_kind (Type::Kind kind)
{
	Type *t = Type::Find (kind);
	
	if (t == NULL) {
		g_warning ("Unable to create instance of type %d. Type not found.", kind);
		return NULL;
	}
	
	return t->CreateInstance ();
}

/*
 * Types
 */

Types::Types ()
{
	//printf ("Types::Types (). this: %p\n", this);
	types = NULL;
	size = 0;
	count = 0;
	EnsureSize ((int) Type::LASTTYPE + 1); // expand immediately to the builtin types we know we'll put into the array
	RegisterStaticTypes ();
	count = 1 + (int) Type::LASTTYPE;
}

Types::~Types ()
{
	//printf ("Types::~Types (). this: %p\n", this);
	if (types != NULL) {
		for (int i = 0; i < count; i++) {
			if (types [i] != NULL)
				delete types [i];
		}
		g_free (types);
		types = NULL;
		size = 0;
		count = 0;
	}
}

void
Types::Initialize ()
{
	RegisterStaticDependencyProperties ();
}

void
Types::EnsureSize (int size)
{
	//printf ("Types::EnsureSize (%i). this: %p\n", size, this);
	
	Type **new_array;
	
	if (this->size > size)
		return;
	
	new_array = (Type **) g_malloc0 (size * sizeof (Type *));
	if (this->types != NULL) {
		for (int i = 0; i < count; i++)
			new_array [i] = this->types [i];
		g_free (this->types);
		this->types = NULL;
	}
	types = new_array;
	this->size = size;
}

Type *
Types::Find (Type::Kind type)
{
	if ((int) type + 1 > count)
		return NULL;
	
	return types [(int) type];
}

Type *
Types::Find (const char *name)
{
	return Types::Find (name, true);
}

Type *
Types::Find (const char *name, bool ignore_case)
{
	for (int i = 1; i < count; i++) { // 0 = INVALID, shouldn't compare against that
		if (i == Type::LASTTYPE)
			continue;
			
		if ((ignore_case && !g_ascii_strcasecmp (types [i]->GetName (), name)) || !strcmp (types [i]->GetName (), name))
			return types [i];
	}

	return NULL;
}

Type::Kind
Types::RegisterType (const char *name, void *gc_handle, Type::Kind parent)
{
	Type::Kind type_id = (Type::Kind) count;
	Type *type = new Type (type_id, parent, false, g_strdup (name), NULL, 0, Find (parent)->GetEventCount (), NULL, NULL, NULL);
	
	// printf ("Types::RegisterType (%s, %p, %i (%s)). this: %p, size: %i, count: %i\n", name, gc_handle, parent, Type::Find (this, parent) ? Type::Find (this, parent)->name : NULL, this, size, count);
	
	EnsureSize (type_id + 1);

	count++;
	
	types [type_id] = type;
	
	return type_id;
}
