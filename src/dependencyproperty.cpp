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

/*
 *	DependencyProperty
 */
DependencyProperty::DependencyProperty (Type::Kind owner_type, const char *name, Value *default_value, Type::Kind property_type, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback, bool is_custom)
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
detach_target_func (DependencyObject *obj, AnimationStorage *storage)
{
	storage->DetachTarget ();
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
DependencyProperty::RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback)
{
	return RegisterFull (NULL, Type::Find (type), name, default_value, vtype, attached, readonly, always_change, changed_callback, false);
}

#if SL_2_0
DependencyProperty *
DependencyProperty::RegisterFull (Types *additional_types, Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback)
{
	return RegisterFull (additional_types, additional_types->Find (type), name, default_value, vtype, attached, readonly, always_change, changed_callback, true);
}

DependencyProperty *
DependencyProperty::RegisterManagedProperty (Types *additional_types, const char *name, Type::Kind property_type, Type::Kind owner_type, bool attached, NativePropertyChangedHandler *callback)
{
	return DependencyProperty::RegisterFull (additional_types, owner_type, name, NULL, property_type, attached, false, false, callback);
}
#endif

//
// Register the dependency property that belongs to @type with the name @name
// The default value is @default_value (if provided) and the type that can be
// stored in the dependency property is of type @vtype
//
DependencyProperty *
DependencyProperty::RegisterFull (Types *additional_types, Type *type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback, bool is_custom)
{
	DependencyProperty *property;
	
	if (type == NULL)
		return NULL;
	
	property = new DependencyProperty (type->type, name, default_value, vtype, attached, readonly, always_change, changed_callback, is_custom);
	
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
			/*
			We'll end up here every time we call runtime_init more than once.
			Proper fix is to make DPs not initialize more than once, or make them really static (like our Types).
			g_warning ("DependencyProperty::RegisterFull (): Trying to register the property '%s' in the type '%s', and there already is a property registered on that type with the same name.",
				property->GetName (), type->name);
				*/
			delete property;
			return existing;
		} else {
			g_hash_table_insert (type->properties, property->hash_key, property);
		}
	}

	return property;
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
					// Some Beta versions of Blend had a bug where they would save the TextBlock
					// properties as TextElement instead. Since Silverlight 1.0 works around this
					// bug, we should too. Fixes http://silverlight.timovil.com and
					// http://election.msn.com/podium08.aspx.
					if ((c - estart) == 11 && !g_ascii_strncasecmp (path + estart, "TextElement", 11))
						typen = g_strdup ("TextBlock");
					else
						typen = g_strndup (path + estart, c - estart);
					estart = c + 1;
					continue;
				}
				if (path [c] == ')') {
					propn = g_strndup (path + estart, c - estart);
					break;
				}
			}

			i = c;
			
			Type *t = NULL;
			if (typen) {
				t = Type::Find (typen);
			} else
				t = Type::Find (lu->GetObjectType ());

			if (!t || !propn || (strlen (propn) == 0)) {
				g_free (propn);
				g_free (typen);
				*o = NULL;
				return NULL;
			}
	
			res = DependencyProperty::GetDependencyProperty (t->GetKind (), propn);
			if (!res) {
				g_warning ("Can't find '%s' property in '%s'", propn, typen);
				g_free (propn);
				g_free (typen);
				*o = NULL;
				return NULL;
			}

			if (! res->IsAttached() && ! lu->Is (t->GetKind ())) {
				// We try to be gracefull here and do something smart...
				res = DependencyProperty::GetDependencyProperty (lu->GetObjectType (), propn);

				if (! res) {
					g_warning ("Got '%s' but expected a type of '%s'!", typen, lu->GetTypeName ());
					g_free (propn);
					g_free (typen);
					*o = NULL;
					return NULL;
				} else {
					g_warning ("Got '%s' but expected a type of '%s' ! "
						   "Did you mean '%s.%s' ? Using that.",
						   typen, lu->GetTypeName (), lu->GetTypeName (), propn);
				}
			}

			g_free (propn);
			g_free (typen);
			break;
		}
		case '.':
			// do not process unless we processed a '(' earlier
			if (!res)
				break;
			lu = lu->GetValue (res)->AsDependencyObject ();
			expression_found = false;
			prop = path + (i + 1);
			// we can ignore this, since we pull the lookup object when we finish a ( ) block
			break;
		case '[':
		{
			int indexer = 0;

			// Need to be a little more loving
			if (path [i + 1] == 0)
				break;

			char *p;

			indexer = strtol (path + i + 1, &p, 10);
			i = p - path;

			if (path [i] != ']'
			    || path [i + 1] != '.')
				break;
			
			Collection *col = lu->GetValue (res)->AsCollection ();
			Value *n = col->GetValueAt (indexer);
			
			if (n) {
				lu = n->AsDependencyObject ();
			} else {
				g_warning ("%s collection doesn't have element %d!", lu->GetTypeName (), indexer);
				*o = NULL;
				return NULL;
			}

			i += 1;
			break;
		}
		}
	}

	if (!expression_found)
		res = DependencyProperty::GetDependencyProperty (lu->GetObjectType (), prop);

	*o = lu;
	return res;
}
