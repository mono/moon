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
#include "eventargs.h"
#include "deployment.h"

/*
 *	DependencyProperty
 */
DependencyProperty::DependencyProperty (Type::Kind owner_type, const char *name, Value *default_value, Type::Kind property_type, bool attached, bool readonly, bool always_change, PropertyChangeHandler changed_callback, ValueValidator *validator, AutoCreator* autocreator, bool is_custom)
{
	this->owner_type = owner_type;
	this->hash_key = NULL;
	this->name = g_strdup (name);
	this->default_value = default_value;
	this->property_type = property_type;
	this->is_nullable = false;
	this->is_attached = attached;
	this->is_readonly = readonly;
	this->always_change = always_change;
	this->changed_callback = changed_callback;
	this->validator = validator ? validator : Validators::default_validator;
	this->autocreator = autocreator;
	this->is_custom = is_custom;
}

DependencyProperty::~DependencyProperty ()
{
	g_free (name);
	if (default_value != NULL)
		delete default_value;
	g_free (hash_key);
}

void
DependencyProperty::Dispose ()
{
	/* 
	 * We want to clear out any refs the default_value might have, but we still
	 * need a default value, since we depend on not returning null for the
	 * default value in some places. So if the current default value is an
	 * EventObject, delete it (clears out the ref) and create a new one with
	 * the same type and null value.
	 */
	if (default_value != NULL) {
		Type::Kind k = default_value->GetKind ();
		if (Type::IsSubclassOf (Deployment::GetCurrent (), k, Type::EVENTOBJECT)) {
			delete default_value;
			default_value = new Value (k); /* null */
		}
	}
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
	return GetDependencyProperty (Type::Find (Deployment::GetCurrent (), type), name);
}

DependencyProperty *
DependencyProperty::GetDependencyProperty (Type *type, const char *name)
{
	return GetDependencyProperty (type, name, true);
}

DependencyProperty *
DependencyProperty::GetDependencyPropertyFull (Type::Kind type, const char *name, bool inherits)
{
	DependencyProperty *property;
	Type *t = Type::Find (Deployment::GetCurrent (), type);
	
	if (t == NULL)
		return NULL;
		
	property = GetDependencyProperty (t, name, inherits);
	
	if (property == NULL) {
		if (inherits)
			property = GetDependencyProperty (t, name, false);
		if (property == NULL && t->HasParent ())
			return GetDependencyPropertyFull (t->GetParentType (), name, inherits);
	}

	return property;
}

DependencyProperty *
DependencyProperty::GetDependencyPropertyFull (Type *type, const char *name, bool inherits)
{
	DependencyProperty *property;

	if (type == NULL)
		return NULL;
	
	property = GetDependencyProperty (type, name, inherits);
	
	if (property == NULL) {
		property = GetDependencyProperty (type, name, false);
		if (property == NULL && type->HasParent ())
			return GetDependencyPropertyFull (type->GetParentType (), name, inherits);
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
	
	if (!type->HasParent ())
		return NULL;

	return GetDependencyProperty (type->GetParentType (), name, inherits);
}

//
// Use this for values that can be null
//
int
DependencyProperty::Register (Types *types, Type::Kind type, const char *name, bool is_custom, Type::Kind vtype)
{
	return RegisterFull (types, type, name, is_custom, NULL, vtype, false, false, false, NULL, NULL, NULL, false);
}

//
// DependencyObject takes ownership of the Value * for default_value
//
int
DependencyProperty::Register (Types *types, Type::Kind type, const char *name, bool is_custom, Value *default_value)
{
	g_return_val_if_fail (default_value != NULL, NULL);

	return RegisterFull (types, type, name, is_custom, default_value, default_value->GetKind (), false, false, false, NULL, NULL, NULL, false);
}

//
// DependencyObject takes ownership of the Value * for default_value
// This overload can be used to set the type of the property to a different type
// than the default value (the default value can for instance be a SolidColorBrush
// while the property type can be a Brush).
//
int
DependencyProperty::Register (Types *types, Type::Kind type, const char *name, bool is_custom, Value *default_value, Type::Kind vtype)
{
	return RegisterFull (types, type, name, is_custom, default_value, vtype, false, false, false, NULL, NULL, NULL, false);
}

DependencyProperty *
DependencyProperty::RegisterCoreProperty (const char *name, Type::Kind property_type, Type::Kind owner_type, Value *default_value, bool attached, bool readonly, PropertyChangeHandler callback)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	int id;
	
	if (default_value && default_value->GetKind () == Type::INVALID)
		default_value = NULL;
	else
		default_value = new Value (*default_value);
	
	id = DependencyProperty::RegisterFull (types, owner_type, name, false, default_value, property_type, attached, readonly, false, callback, NULL, NULL, false);
	
	return types->GetProperty (id);
}

DependencyProperty *
DependencyProperty::RegisterCustomProperty (const char *name, Type::Kind property_type, Type::Kind owner_type, Value *default_value, bool attached, bool readonly, PropertyChangeHandler callback)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	int id;
	
	if (default_value && default_value->GetKind () == Type::INVALID)
		default_value = NULL;
	else
		default_value = new Value (*default_value);
	
	id = DependencyProperty::RegisterFull (types, owner_type, name, true, default_value, property_type, attached, readonly, false, callback, NULL, NULL, false);
	
	return types->GetProperty (id);
}

//
// Register the dependency property that belongs to @type with the name @name
// The default value is @default_value (if provided) and the type that can be
// stored in the dependency property is of type @vtype
//
int
DependencyProperty::RegisterFull (Types *types, Type::Kind type, const char *name, bool is_custom, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, PropertyChangeHandler changed_callback, ValueValidator *validator, AutoCreator* autocreator, bool is_nullable)
{
	DependencyProperty *property;
	
	g_return_val_if_fail (types != NULL, NULL);
	g_return_val_if_fail (type != Type::INVALID, NULL);
	g_return_val_if_fail (name != NULL, NULL);
	
	if (!is_custom && default_value && types->IsSubclassOf (default_value->GetKind (), Type::DEPENDENCY_OBJECT))
		default_value->AsDependencyObject ()->Freeze();
		
	property = new DependencyProperty (type, name, default_value, vtype, attached, readonly, always_change, changed_callback, validator, autocreator, is_custom);
	property->is_nullable = is_nullable;
	
	types->AddProperty (property);
	
	return property->GetId ();
}

bool
DependencyProperty::Validate (DependencyObject *instance, Value *value, MoonError *error)
{ 
	return validator (instance, this, value, error);
}

void
DependencyProperty::SetPropertyChangedCallback (PropertyChangeHandler changed_callback)
{
	this->changed_callback = changed_callback;
}

Type *
lookup_type (DependencyObject *lu, const char* name)
{
	Type *t = Type::Find (lu->GetDeployment (), name);

	if (t)
		return t;

	//
	// If we are dealing with a managed type and we don't have the full namespace
	// we just verify that the type name matches the lookup type.
	//

	const char *tname = strchr (name, ':');
	if (!tname || ! *(++tname))
		return NULL;

	const char *luname = lu->GetTypeName ();
	int lulen = strlen (luname);
	int tlen = strlen (tname);

	if (lulen < tlen || strcmp (luname + lulen - tlen, tname))
		return NULL;

	return lu->GetType ();
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
resolve_property_path (DependencyObject **o, PropertyPath *propertypath, GHashTable *promoted_values)
{
	g_return_val_if_fail (o != NULL, NULL);
	g_return_val_if_fail (propertypath != NULL, NULL);
	g_return_val_if_fail (propertypath->path != NULL || propertypath->property != NULL, NULL);
	
	if (propertypath->property)
		return propertypath->property;

	const char *path = propertypath->path;
	if (propertypath->expanded_path)
		path = propertypath->expanded_path;

	const char *inend = path + strlen (path);
	register const char *inptr = path;
	const char *start, *prop = path;
	bool expression_found = false;
	DependencyProperty *res = NULL;
	DependencyObject *lu = *o;
	Collection *collection;
	char *p, *name = NULL;
	Value *value = NULL;
	Type *type = NULL;
	int index;
	bool paren_open = false;
	bool tick_open = false;
	bool cloned = false;

	while (inptr < inend) {
		switch (*inptr++) {
		case '(':
			paren_open = true;
			break;
		case ')':
			paren_open = false;
			break;
		case '\'':
			// Ticks are only legal in expanded paths, so we should just fail here
			if (!propertypath->expanded_path) {
				g_warning ("The ' character is not legal in property paths.");
				break;
			}

			tick_open = !tick_open;
			break;
		case '.':
			if (tick_open)
				continue;

			// resolve the dependency property
			if (res) {
				DependencyObject *new_lu;

				// make sure that we are getting what we expect
				if (!(value = lu->GetValue (res)))
					goto error;

				if (!(new_lu = value->AsDependencyObject ()))
					goto error;

				if (!cloned && !g_hash_table_lookup (promoted_values, value) && !value->Is (lu->GetDeployment (), Type::UIELEMENT)) {
					// we need to clone the value here so that we deep copy any
					// DO subclasses (such as brushes, etc) that we're promoting
					// from a shared space (Styles, default values)
					Value *cloned_value = Value::Clone (value);
					lu->SetValue (res, cloned_value);
					new_lu = cloned_value->AsDependencyObject();
					delete cloned_value;

					cloned_value = lu->GetValue (res);
					g_hash_table_insert (promoted_values, cloned_value, cloned_value);
				}

				lu = new_lu;
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

			if (expression_found) {
				expression_found = false;
				if (!(value = lu->GetValue (res)))
					goto error;
			}
			
			if (value == NULL)
				goto error;
			
			if (!(collection = value->AsCollection ()))
				goto error;

			if (!(value = collection->GetValueAt (index)))
				goto error;
			
			if (!(lu = value->AsDependencyObject ()))
				goto error;
			
			break;
		
		default:
			bool explicit_type = false;
			expression_found = true;
			start = inptr - 1;

			while (inptr < inend && (*inptr != '.' || tick_open) && (!paren_open || *inptr != ')') && *inptr != '[') {
				if (*inptr == '\'') {
					tick_open = !tick_open;
					if (!tick_open) {
						inptr++;
						break;
					}
				}
				inptr++;
			}

			if (inptr == start)
				goto error;

			if (*inptr == '.') {
				// we found a type name, now we need to find the property name
				if ((inptr - start) == 11 && !g_ascii_strncasecmp (start, "TextElement", 11)) {
					// Some Beta versions of Blend had a bug where they would save the TextBlock
					// properties as TextElement instead. Since Silverlight 1.0 works around this
					// bug, we should too. Fixes http://silverlight.timovil.com and
					// http://election.msn.com/podium08.aspx.
					type = Type::Find (lu->GetDeployment (), "TextBlock");
					explicit_type = true;
				} else {
					const char *s = inptr;
					if (*(inptr -1) == '\'' && !tick_open) {
						s = inptr - 1;
					}
					name = g_strndup (start, s - start);
					type = lookup_type (lu, name);
					explicit_type = true;
					if (!type)
						type = lu->GetType ();
					g_free (name);
				}
				
				inptr++;
				start = inptr;
				while (inptr < inend && (!paren_open || *inptr != ')') && (*inptr != '.' || tick_open)) {
					if (*inptr == '\'') {
						tick_open = !tick_open;
						if (!tick_open) {
							inptr++;
							break;
						}
					}
					inptr++;
				}
				
				if (inptr == start)
					goto error;
			} else {
				type = lu->GetType ();
				explicit_type = false;
			}
			
			if ((*inptr != ')' && paren_open) || !type)
				goto error;

			name = g_strndup (start, inptr - start);
			if (!(res = DependencyProperty::GetDependencyProperty (type, name)) && lu)
				res = DependencyProperty::GetDependencyProperty (lu->GetType (), name);

			if (!res) {
				g_free (name);
				goto error;
			}

			if (!res->IsAttached () && !lu->Is (type->GetKind ())) {
				// We try to be gracefull here and do something smart...
				if (!(res = DependencyProperty::GetDependencyProperty (lu->GetType (), name))) {
					g_free (name);
					goto error;
				}
			}
			
			if (res->IsAttached () && explicit_type && !paren_open)
				goto error;
			
			g_free (name);
			break;
		}
	}
	
	*o = lu;
	return res;
	
 error:
	*o = NULL;	
	return NULL;
}
