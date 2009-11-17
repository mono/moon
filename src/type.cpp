/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
Type::Type (Deployment *deployment, Type::Kind type, Type::Kind parent, bool is_value_type, bool is_interface,
	    const char *name, 
	    int event_count, int total_event_count, const char **events, 
	    int interface_count, const Type::Kind *interfaces, bool ctor_visible,
	    create_inst_func *create_inst, const char *content_property)
{
	this->type = type;
	this->parent = parent;
	this->is_value_type = is_value_type;
	this->is_interface = is_interface;
	this->name = name;
	this->event_count = event_count;
	this->total_event_count = total_event_count;
	this->events = events;
	this->ctor_visible = ctor_visible;
	this->create_inst = create_inst;
	this->content_property = content_property;
	this->properties = NULL;
	this->interface_count = interface_count;
	if (this->interface_count) {
		this->interfaces = new Type::Kind[interface_count];
		memcpy (this->interfaces, interfaces, interface_count * sizeof (Type::Kind));
	}
	else {
		this->interfaces = NULL;
	}
	this->deployment = deployment;
}
		
Type::~Type ()
{
	if (properties) {
		g_hash_table_destroy (properties);
		properties = NULL;
	}

	delete [] interfaces;
}

int
Type::LookupEvent (const char *event_name)
{
	Type *parent_type = Type::Find (deployment, parent);
	int result;

	if (events != NULL) {
		for (int i = 0; events [i] != NULL; i++) {
			if (!g_ascii_strcasecmp (events [i], event_name))
				return i + (parent_type == NULL ? 0 : parent_type->total_event_count);
		}
	}
	
	if (parent == Type::INVALID || parent_type == NULL) {
#if SANITY
		printf ("Event lookup of event '%s' in type '%s' failed.\n", event_name, name);
#endif
		return -1;
	}

	result = parent_type->LookupEvent (event_name);

#if SANITY
	if (result == -1)
		printf ("Event lookup of event '%s' in (more exactly) type '%s' failed.\n", event_name, name);
#endif

	return result;
}

bool
Type::IsSubclassOf (Deployment *deployment, Type::Kind type, Type::Kind super)
{
	return deployment->GetTypes ()->IsSubclassOf (type, super);
}

bool
Type::IsSubclassOf (Type::Kind super)
{
	return deployment->GetTypes ()->IsSubclassOf (type, super);
}

#if SANITY || DEBUG
bool
Types::IsSubclassOrSuperclassOf (Type::Kind unknown, Type::Kind known)
{
	return IsSubclassOf(unknown, known) || IsSubclassOf (known, unknown);
}

bool
Types::IsSubclassOrSuperclassOf (Types *arg, Type::Kind unknown, Type::Kind known)
{
	Types *types = arg == NULL ? Deployment::GetCurrent ()->GetTypes () : arg;
	return types->IsSubclassOf(unknown, known) || types->IsSubclassOf (known, unknown);
}
#endif

bool
Types::IsSubclassOf (Type::Kind type, Type::Kind super)
{
	Type *t;
	Type::Kind parent;
	
	if (type == Type::INVALID)
		return false;
	
	if (type == super)
		return true;
	
	t = Find (type);
	
	g_return_val_if_fail (t != NULL, false);
	
	do {
		parent = t->parent;
		
		if (parent == super)
			return true;
			
		if (parent == Type::INVALID)
			return false;
		
		t = Find (parent);
		
		if (t == NULL)
			return false;		
	} while (true);
	
	return false;
}

bool
Type::IsAssignableFrom (Type::Kind type)
{
	return deployment->GetTypes ()->IsAssignableFrom (GetKind (), type);
}
bool
Type::IsAssignableFrom (Deployment *deployment, Type::Kind destination, Type::Kind type)
{
	return deployment->GetTypes ()->IsAssignableFrom (destination, type);
}

bool
Types::IsAssignableFrom (Type::Kind destination, Type::Kind type)
{
	if (destination == type)
		return true;

	if (IsSubclassOf (type, destination))
		return true;

	// more expensive..  interface checks
	Type *destination_type = Find (destination);
	if (!destination_type->IsInterface())
		return false;

	Type *type_type = Find (type);
	while (type_type && type_type->GetKind() != Type::INVALID) {
		for (int i = 0; i < type_type->GetInterfaceCount(); i ++) {
			// should this be IsAssignableFrom instead of ==?  ugh
			if (type_type->GetInterface(i) == destination)
				return true;
		}
		type_type = Find (type_type->parent);
	}

	return false;
}

Type *
Type::Find (Deployment *deployment, const char *name)
{
	return deployment->GetTypes ()->Find (name);
}

Type *
Type::Find (Deployment *deployment, const char *name, bool ignore_case)
{
	return deployment->GetTypes ()->Find (name, ignore_case);
}

Type *
Type::Find (Deployment *deployment, Type::Kind type)
{
	if (type < Type::INVALID || type == Type::LASTTYPE)
		return NULL;
		
	return deployment->GetTypes ()->Find (type);
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

	parent_type = Find (deployment, parent);

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
	
	return NULL;
}

void
Type::AddProperty (DependencyProperty *property)
{
	DependencyProperty *existing = NULL;

	g_return_if_fail (property != NULL);

	if (properties == NULL) {
		properties = g_hash_table_new (g_str_hash, g_str_equal);
	} else {
		existing = (DependencyProperty *) g_hash_table_lookup (properties, property->GetHashKey ());
	}

	if (existing == NULL || existing->IsCustom ()) {
		// Allow overwriting of custom properties
		g_hash_table_insert (properties, (gpointer) property->GetHashKey (), property);
	} else {
		g_warning ("Type::AddProperty (): Trying to register the property '%s' (of type %s) in the owner type '%s', and there already is a property registered on that type with the same name.",
			   property->GetName (), Type::Find (deployment, property->GetPropertyType ())->GetName(), GetName());	
	}
}

Type *
Type::GetParentType ()
{
	if (parent == Type::INVALID)
		return NULL;
	
	return deployment->GetTypes ()->Find (parent);
}

static void
property_add (gpointer key, gpointer value, gpointer user_data)
{
	g_hash_table_insert ((GHashTable *) user_data, key, value);
}

GHashTable *
Type::CopyProperties (bool inherited)
{
	GHashTable *props = g_hash_table_new (g_str_hash, g_str_equal);
	Type *type = this;
	
	do {
		if (type->properties)
			g_hash_table_foreach (type->properties, property_add, props);
		
		if (!inherited || !type->HasParent ())
			break;
		
		type = type->GetParentType ();
	} while (type);
	
	return props;
}

bool
type_get_value_type (Type::Kind type)
{
	Type *t = Type::Find (Deployment::GetCurrent (), type);
	
	if (t == NULL)
		return false;
	
	return t->IsValueType ();
}

bool
type_is_dependency_object (Type::Kind type)
{
	return Type::IsSubclassOf (Deployment::GetCurrent (), type, Type::DEPENDENCY_OBJECT);
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
	Type *t = Type::Find (Deployment::GetCurrent (), kind);
	
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
	types.SetCount ((int) Type::LASTTYPE + 1);
	RegisterNativeTypes ();
}

void
Types::Initialize ()
{
	RegisterNativeProperties ();
}

void
Types::DeleteProperties ()
{
	/* this can't be done in the destructor, since deleting the properties might end up accessing the types */
	for (int i = 0; i < properties.GetCount (); i++)
		delete (DependencyProperty *) properties [i];
	properties.SetCount (0);	
}

void
Types::Dispose ()
{
	for (int i = 0; i < properties.GetCount (); i++)
		((DependencyProperty *) properties [i])->Dispose ();
}

Types::~Types ()
{
	for (int i = 0; i < types.GetCount (); i++)
		delete (Type *) types [i];
}

void
Types::AddProperty (DependencyProperty *property)
{
	Type *type;
	
	g_return_if_fail (property != NULL);
	
	type = Find (property->GetOwnerType ());
	
	g_return_if_fail (type != NULL);
	
	property->SetId (properties.Add (property));
	type->AddProperty (property);
}

DependencyProperty *
Types::GetProperty (int id)
{
	g_return_val_if_fail (properties.GetCount () > id, NULL);
	return (DependencyProperty *) properties [id];
}

Type *
Types::Find (const char *name)
{
	return Types::Find (name, true);
}

Type *
Types::Find (const char *name, bool ignore_case)
{
	Type *t;
	
	for (int i = 1; i < types.GetCount (); i++) { // 0 = INVALID, shouldn't compare against that
		if (i == Type::LASTTYPE)
			continue;
	
		t = (Type *) types [i];
		if ((ignore_case && !g_ascii_strcasecmp (t->GetName (), name)) || !strcmp (t->GetName (), name))
			return t;
	}

	return NULL;
}

Type::Kind
Types::RegisterType (const char *name, void *gc_handle, Type::Kind parent, bool is_interface, bool ctor_visible, Type::Kind* interfaces, int interface_count)
{
	Type *type = new Type (Deployment::GetCurrent (), Type::INVALID, parent, false, is_interface, g_strdup (name), 0, Find (parent)->GetEventCount (), NULL, interface_count, interfaces, ctor_visible, NULL, NULL);
	
	// printf ("Types::RegisterType (%s, %p, %i (%s)). this: %p, size: %i, count: %i\n", name, gc_handle, parent, Type::Find (this, parent) ? Type::Find (this, parent)->name : NULL, this, size, count);
	
	type->SetKind ((Type::Kind) types.Add (type));
	
	return type->GetKind ();
}

