/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dependencyobject.h: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOONLIGHT_DEPENDENCY_OBJECT_H__
#define __MOONLIGHT_DEPENDENCY_OBJECT_H__

#include <glib.h>

#include "value.h"
#include "enums.h"
#include "list.h"

//
// DependencyProperty
//
class DependencyProperty {
	GHashTable *storage_hash; // keys: objects, values: animation storage's
	bool is_readonly;
	
 public:
	DependencyProperty () {};
	~DependencyProperty ();
	DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached, bool readonly, bool always_change);

	char *hash_key;
	char *name;
	Value *default_value;
	Type::Kind type;
	bool is_attached_property;
	Type::Kind value_type;
	bool is_nullable;
	bool always_change; // determines if SetValue will do something if the current and new values are equal.

	bool IsNullable () { return is_nullable; }
	bool IsReadOnly () { return is_readonly; }

	AnimationStorage *AttachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	void DetachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	AnimationStorage *GetAnimationStorageFor (DependencyObject *obj);
};

G_BEGIN_DECLS

DependencyProperty *dependency_property_lookup (Type::Kind type, char *name);
char *dependency_property_get_name (DependencyProperty* property);
bool  dependency_property_is_nullable (DependencyProperty* property);
Type::Kind dependency_property_get_value_type (DependencyProperty* property);
DependencyProperty *resolve_property_path (DependencyObject **o, const char *path);

G_END_DECLS

#endif /* __MOONLIGHT_DEPENDENCY_OBJECT_H__ */
