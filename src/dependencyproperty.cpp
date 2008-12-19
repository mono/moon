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
	this->hash_key = g_ascii_strdown (name, -1);
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

static void
free_property (gpointer v)
{
	delete (DependencyProperty*)v;
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

#if SL_2_0
DependencyProperty *
DependencyProperty::GetDependencyProperty (Types *additional_types, Type::Kind type, const char *name, bool inherits)
{
	DependencyProperty *property;
	
	property = GetDependencyProperty (type, name, inherits);
	
	if (property == NULL)
		property = GetDependencyProperty (additional_types->Find (type), name, inherits);

	return property;
}
#endif

DependencyProperty *
DependencyProperty::GetDependencyProperty (Type *type, const char *name, bool inherits)
{
	DependencyProperty *property = NULL;

	if (type == NULL)
		return NULL;

	if (type->properties != NULL) {
		char *key = g_ascii_strdown (name, -1);
		property = (DependencyProperty*) g_hash_table_lookup (type->properties, key);
		g_free (key);
	
		if (property != NULL)
			return property;
	}

	if (!inherits) {
		fprintf (stderr, "DependencyProperty::GetDependencyProperty (%s, %s, %i): Property not found.\n", type->name, name, inherits);
		return NULL;
	}
	
	if (type->GetParent () == Type::INVALID)
		return NULL;

	return GetDependencyProperty (Type::Find (type->GetParent ()), name, inherits);
}

//
// Use this for values that can be null
//
DependencyProperty *
DependencyProperty::Register (Type::Kind type, const char *name, Type::Kind vtype)
{
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, NULL, vtype, false, false, false, NULL);
}

//
// DependencyObject takes ownership of the Value * for default_value
//
DependencyProperty *
DependencyProperty::Register (Type::Kind type, const char *name, Value *default_value)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, default_value->GetKind (), false, false, false, NULL);
}

//
// DependencyObject takes ownership of the Value * for default_value
// This overload can be used to set the type of the property to a different type
// than the default value (the default value can for instance be a SolidColorBrush
// while the property type can be a Brush).
//
DependencyProperty *
DependencyProperty::Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, vtype, false, false, false, NULL);
}

DependencyProperty *
DependencyProperty::RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype)
{
	DependencyProperty *property;
	property = Register (type, name, vtype);
	property->is_nullable = true;
	return property;
}

DependencyProperty *
DependencyProperty::RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback, ValueValidator *validator)
{
	return RegisterFull (NULL, Type::Find (type), name, default_value, vtype, attached, readonly, always_change, changed_callback, validator, false);
}

DependencyProperty *
DependencyProperty::RegisterFull (Types *additional_types, Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback)
{
	return RegisterFull (additional_types, additional_types->Find (type), name, default_value, vtype, attached, readonly, always_change, changed_callback, NULL, true);
}

DependencyProperty *
DependencyProperty::RegisterManagedProperty (Types *additional_types, const char *name, Type::Kind property_type, Type::Kind owner_type, Value *default_value, bool attached, bool readonly, NativePropertyChangedHandler *callback)
{
	if (default_value && default_value->GetKind () == Type::INVALID)
		default_value = NULL;
	else
		default_value = new Value (*default_value);
	return DependencyProperty::RegisterFull (additional_types, owner_type, name, default_value, property_type, attached, readonly, false, callback);
}

//
// Register the dependency property that belongs to @type with the name @name
// The default value is @default_value (if provided) and the type that can be
// stored in the dependency property is of type @vtype
//
DependencyProperty *
DependencyProperty::RegisterFull (Types *additional_types, Type *type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback, ValueValidator *validator, bool is_custom)
{
	DependencyProperty *property;
	
	if (type == NULL)
		return NULL;
	
	property = new DependencyProperty (type->type, name, default_value, vtype, attached, readonly, always_change, changed_callback, validator, is_custom);
	
	if (is_custom) {
		// Managed code is allowed to register several properties with the same name
		// and they all get the callback called when the property value changes.
		// See comment in type.h.
		type->custom_properties = g_slist_prepend (type->custom_properties, property);
	} else {
		DependencyProperty *existing;
		if (type->properties == NULL)
			type->properties = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, free_property);

		if ((existing = (DependencyProperty *)g_hash_table_lookup (type->properties, property->hash_key)) != NULL) {
			g_warning ("DependencyProperty::RegisterFull (): Trying to register the property '%s' in the type '%s', and there already is a property registered on that type with the same name.",
				property->GetName (), type->name);
			delete property;
			return existing;
		} else {
			g_hash_table_insert (type->properties, property->hash_key, property);
		}
	}

	return property;
}

bool
DependencyProperty::Validate (Value *value, MoonError *error)
{ 
	return validator (value, error);
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
	g_return_val_if_fail (o != NULL, NULL);
	g_return_val_if_fail (path != NULL, NULL);
	
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
