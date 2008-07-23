/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include <pthread.h>

#include "dependencyproperty.h"
#include "animation.h"

/*
 *	DependencyProperty
 */
DependencyProperty::DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached, bool readonly, bool always_change)
{
	this->type = type;
	this->hash_key = g_ascii_strdown (name, -1);
	this->name = g_strdup (name);
	this->default_value = default_value;
	this->value_type = value_type;
	this->is_nullable = false;
	this->is_attached_property = attached;
	this->is_readonly = readonly;
	this->storage_hash = NULL; // Create it on first usage request
	this->always_change = always_change;
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
	
			res = DependencyObject::GetDependencyProperty (t->GetKind (), propn);
			if (!res) {
				g_warning ("Can't find '%s' property in '%s'", propn, typen);
				g_free (propn);
				g_free (typen);
				*o = NULL;
				return NULL;
			}

			if (! res->is_attached_property && ! lu->Is (t->GetKind ())) {
				// We try to be gracefull here and do something smart...
				res = DependencyObject::GetDependencyProperty (lu->GetObjectType (), propn);

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
			List::Node *n = col->list->Index (indexer);
			if (n)
				lu = ((Collection::Node *) n)->obj;
			else {
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
		res = DependencyObject::GetDependencyProperty (lu->GetObjectType (), prop);

	*o = lu;
	return res;
}
