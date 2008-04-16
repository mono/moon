/*
 * type.cpp: Our type system
 *
 * Author:
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
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

int
Type::LookupEvent (const char *event_name)
{
	Type *parent_type = Type::Find (parent);
	int result;

	if (events != NULL) {
		for (int i = 0; events [i] != NULL; i++) {
			if (strcase_equal (events [i], event_name))
				return i + (parent_type == NULL ? 0 : parent_type->total_event_count);
		}
	}

	if (parent_type == NULL) {
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
		if (strcmp (type_infos [i].name, name) == 0)
			return &type_infos [i];
		
		if (strcmp (type_infos [i].kindname, name) == 0)
			return &type_infos [i];
	}

	return NULL;
}

Type *
Type::Find (Type::Kind type)
{
	if (type < Type::INVALID || type >= Type::LASTTYPE)
		return NULL;

	return &type_infos [type];
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
		if (type_infos [i].type != i)
			fprintf (stderr, "Type verification: type #%i is stored with Kind %i, name %s\n", i, type_infos [i].type, type_infos [i].name);
	}
#endif
}
