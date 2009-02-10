/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dependencypropery.h: 
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "dependencyproperty.h"
#include "animation.h"
#include "runtime.h"
#include "validators.h"

/*
 *	DependencyProperty
 */
DependencyProperty::DependencyProperty (Type::Kind owner_type, const char *name, Value *default_value, Type::Kind property_type, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback, ValueValidator *validator, bool is_custom)
{
	this->owner_type = owner_type;
	this->hash_key = NULL;
	this->name = g_strdup (name);
	this->default_value = default_value;
	this->property_type = property_type;
	this->is_nullable = false;
	this->is_attached = attached;
	this->is_readonly = readonly;
	this->storage_hash = NULL; // Create it on first usage request
	this->always_change = always_change;
	this->changed_callback = changed_callback;
	this->validator = validator ? validator : Validators::default_validator;
	this->is_custom = is_custom;
}

AnimationStorage*
DependencyProperty::GetAnimationStorageFor (DependencyObject *obj)
{
	if (! storage_hash)
		return NULL;

	return (AnimationStorage *) g_hash_table_lookup (storage_hash, obj);
}

AnimationStorage*
DependencyProperty::AttachAnimationStorage (DependencyObject *obj, AnimationStorage *storage)
{
	// Create hash on first access to save some mem
	if (! storage_hash)
		storage_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	AnimationStorage *attached_storage = (AnimationStorage *) g_hash_table_lookup (storage_hash, obj);
	if (attached_storage)
		attached_storage->DetachTarget ();

	g_hash_table_insert (storage_hash, obj, storage);
	return attached_storage;
}

void
DependencyProperty::DetachAnimationStorage (DependencyObject *obj, AnimationStorage *storage)
{
	if (! storage_hash)
		return;

	if (g_hash_table_lookup (storage_hash, obj) == storage)
		g_hash_table_remove (storage_hash, obj);
}

static void
detach_target_func (DependencyObject *obj, AnimationStorage *storage, gpointer unused)
{
	storage->DetachTarget ();
	if (storage->IsFloating ()) {
		delete storage;
	}
}

DependencyProperty::~DependencyProperty ()
{
	g_free (name);
	if (default_value != NULL)
		delete default_value;

	if (storage_hash) {
		g_hash_table_foreach (storage_hash, (GHFunc) detach_target_func, NULL);
		g_hash_table_destroy (storage_hash);
		storage_hash = NULL;
	}
	g_free (hash_key);
}

const char *
DependencyProperty::GetHashKey ()
{
	if (hash_key == NULL) 
		hash_key = g_ascii_strdown (name, -1);
		
	return hash_key;
}

DependencyProperty *
DependencyProperty::GetDependencyProperty (Type::Kind type, const char *name)
{
	return GetDependencyProperty (type, name, true);
}

DependencyProperty *
DependencyProperty::GetDependencyProperty (Type::Kind type, const char *name, bool inherits)
{
	return GetDependencyProperty (Type::Find (type), name, inherits);
}

DependencyProperty *
DependencyProperty::GetDependencyPropertyFull (Type::Kind type, const char *name, bool inherits)
{
	DependencyProperty *property;
	
	property = GetDependencyProperty (type, name, inherits);
	
	if (property == NULL) {
		Type *t = Type::Find (type);
		if (t == NULL)
			return NULL;

		property = GetDependencyProperty (t, name, false);
		if (property == NULL && t->GetParent () != Type::INVALID)
			return GetDependencyPropertyFull (t->GetParent (), name, inherits);
	}

	return property;
}

DependencyProperty *
DependencyProperty::GetDependencyProperty (Type *type, const char *name, bool inherits)
{
	DependencyProperty *property = NULL;

	if (type == NULL)
		return NULL;

	property = type->LookupProperty (name);
			
	if (property)
		return property;

	if (!inherits)
		return NULL;
	
	if (type->GetParent () == Type::INVALID)
		return NULL;

	return GetDependencyProperty (Type::Find (type->GetParent ()), name, inherits);
}

//
// Use this for values that can be null
//
DependencyProperty *
DependencyProperty::Register (Types *types, Type::Kind type, const char *name, Type::Kind vtype)
{
	return RegisterFull (types, type, name, NULL, vtype, false, false, false, NULL, NULL, false, false);
}

//
// DependencyObject takes ownership of the Value * for default_value
//
DependencyProperty *
DependencyProperty::Register (Types *types, Type::Kind type, const char *name, Value *default_value)
{
	g_return_val_if_fail (default_value != NULL, NULL);

	return RegisterFull (types, type, name, default_value, default_value->GetKind (), false, false, false, NULL, NULL, false, false);
}

//
// DependencyObject takes ownership of the Value * for default_value
// This overload can be used to set the type of the property to a different type
// than the default value (the default value can for instance be a SolidColorBrush
// while the property type can be a Brush).
//
DependencyProperty *
DependencyProperty::Register (Types *types, Type::Kind type, const char *name, Value *default_value, Type::Kind vtype)
{
	return RegisterFull (types, type, name, default_value, vtype, false, false, false, NULL, NULL, false, false);
}

DependencyProperty *
DependencyProperty::RegisterManagedProperty (const char *name, Type::Kind property_type, Type::Kind owner_type, Value *default_value, bool attached, bool readonly, NativePropertyChangedHandler *callback)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	if (default_value && default_value->GetKind () == Type::INVALID)
		default_value = NULL;
	else
		default_value = new Value (*default_value);
	return DependencyProperty::RegisterFull (types, owner_type, name, default_value, property_type, attached, readonly, false, callback, NULL, true, false);
}

//
// Register the dependency property that belongs to @type with the name @name
// The default value is @default_value (if provided) and the type that can be
// stored in the dependency property is of type @vtype
//
DependencyProperty *
DependencyProperty::RegisterFull (Types *types, Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback, ValueValidator *validator, bool is_custom, bool is_nullable)
{
	DependencyProperty *property;
	
	g_return_val_if_fail (types != NULL, NULL);
	g_return_val_if_fail (type != Type::INVALID, NULL);
	g_return_val_if_fail (name != NULL, NULL);
	
	if (default_value && types->IsSubclassOf (default_value->GetKind (), Type::DEPENDENCY_OBJECT))
		default_value->AsDependencyObject ()->Freeze();
		
	property = new DependencyProperty (type, name, default_value, vtype, attached, readonly, always_change, changed_callback, validator, is_custom);
	property->is_nullable = is_nullable;
	property->validator = validator ? validator : Validators::default_validator;
	
	types->AddProperty (property);
	
	return property;
}

bool
DependencyProperty::Validate (DependencyObject *instance, Value *value, MoonError *error)
{ 
	return validator (instance, this, value, error);
}

void
DependencyProperty::SetPropertyChangedCallback (NativePropertyChangedHandler *changed_callback)
{
	this->changed_callback = changed_callback;
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
resolve_property_path (DependencyObject **o, PropertyPath *propertypath)
{
	g_return_val_if_fail (o != NULL, NULL);
	g_return_val_if_fail (propertypath != NULL, NULL);
	g_return_val_if_fail (propertypath->path != NULL, NULL);
	
	const char *path = propertypath->path;
	const char *inend = path + strlen (path);
	register const char *inptr = path;
	const char *start, *prop = path;
	bool expression_found = false;
	DependencyProperty *res = NULL;
	DependencyObject *lu = *o;
	Collection *collection;
	char *p, *name = NULL;
	Value *value;
	Type *type;
	int index;
	
	while (inptr < inend) {
		switch (*inptr++) {
		case '(':
			expression_found = true;
			
			start = inptr;
			while (inptr < inend && *inptr != '.' && *inptr != ')')
				inptr++;
			
			if (inptr == start)
				goto error;
			
			if (*inptr == '.') {
				// we found a type name, now we need to find the property name
				if ((inptr - start) == 11 && !g_ascii_strncasecmp (start, "TextElement", 11)) {
					// Some Beta versions of Blend had a bug where they would save the TextBlock
					// properties as TextElement instead. Since Silverlight 1.0 works around this
					// bug, we should too. Fixes http://silverlight.timovil.com and
					// http://election.msn.com/podium08.aspx.
					type = Type::Find ("TextBlock");
				} else {
					name = g_strndup (start, inptr - start);
					type = Type::Find (name);
					g_free (name);
				}
				
				inptr++;
				start = inptr;
				while (inptr < inend && *inptr != ')')
					inptr++;
				
				if (inptr == start)
					goto error;
			} else {
				type = Type::Find (lu->GetObjectType ());
			}
			
			if (*inptr != ')' || !type)
				goto error;
			
			name = g_strndup (start, inptr - start);
			if (!(res = DependencyProperty::GetDependencyProperty (type->GetKind (), name))) {
				g_free (name);
				goto error;
			}
			
			if (!res->IsAttached () && !lu->Is (type->GetKind ())) {
				// We try to be gracefull here and do something smart...
				if (!(res = DependencyProperty::GetDependencyProperty (lu->GetObjectType (), name))) {
					g_free (name);
					goto error;
				}
			}
			
			g_free (name);
			inptr++;
			break;
		case '.':
			// resolve the dependency property
			if (res) {
				// make sure that we are getting what we expect
				if (!(value = lu->GetValue (res)))
					goto error;
				
				if (!(lu = value->AsDependencyObject ()))
					goto error;
			}
			
			expression_found = false;
			prop = inptr;
			break;
		case '[':
			// Need to be a little more loving
			if (*inptr == '\0')
				break;
			
			index = strtol (inptr, &p, 10);
			if (*p != ']' || *(p + 1) != '.')
				break;
			
			inptr = p + 2;
			prop = inptr;
			
			if (!(value = lu->GetValue (res)))
				goto error;
			
			if (!(collection = value->AsCollection ()))
				goto error;
			
			if (!(value = collection->GetValueAt (index)))
				goto error;
			
			if (!(lu = value->AsDependencyObject ()))
				goto error;
			
			break;
		}
	}
	
	if (!expression_found)
		res = DependencyProperty::GetDependencyProperty (lu->GetObjectType (), prop);
	
	*o = lu;
	
	return res;
	
 error:
	
	*o = NULL;
	
	return NULL;
}
