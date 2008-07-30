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

/*
 * Type implementation
 */

Type::~Type ()
{
	if (properties) {
		g_hash_table_destroy (properties);
		properties = NULL;
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
	// Types are ordered alphabetically according to kindname
	// so an optimization here would be to do a binary search.

	for (int i = 1; i < Type::LASTTYPE; i++) {
		if (!g_strcasecmp (type_infos [i].name, name))
			return &type_infos [i];
		
		if (!g_strcasecmp (type_infos [i].kindname, name))
			return &type_infos [i];
	}

	return NULL;
}

Type *
Type::Find (Type::Kind type)
{
	return Find (NULL, type);
}


Type *
Type::Find (Surface *surface, Type::Kind type)
{
	int index;
		
	if (type < Type::INVALID || type == Type::LASTTYPE)
		return NULL;
		
	if (type < Type::LASTTYPE)
		return &type_infos [type];

#if SL_2_0				
	if (surface == NULL) {
		fprintf (stderr, "Type::Find (%p, %i): No surface to look in.\n", surface, type);
		return NULL;
	}
	
	return surface->GetManagedType (type, false);
#else
	return NULL;
#endif
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
	Type *t = Type::Find (type);
	if (t == NULL)
		return false;
	return t->IsSubclassOf (Type::DEPENDENCY_OBJECT);
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

void
types_init ()
{
#if DEBUG
	for (int i = 0; i <= Type::LASTTYPE; i++) {
#if !SL_2_0
		if (type_infos [i].type != i && (type_infos [i].name == NULL || strstr (type_infos [i].name, "2.0 specific") == NULL)) {
			
#else
		if (type_infos [i].type != i) {
#endif
			fprintf (stderr, "Type verification: type #%i is stored with Kind %i, name %s\n", i, type_infos [i].type, type_infos [i].name);
		}
	}
#endif
}
