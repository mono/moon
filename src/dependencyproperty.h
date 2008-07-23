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
 public:
	DependencyProperty () {};
	~DependencyProperty ();
	DependencyProperty (Type::Kind owner_type, const char *name, Value *default_value, Type::Kind property_type, bool attached, bool readonly, bool always_change);

	char *GetName() { return name; }
	Type::Kind GetOwnerType() { return owner_type; }
	Type::Kind GetPropertyType() { return property_type; }
	
	bool IsNullable () { return is_nullable; }
	bool IsReadOnly () { return is_readonly; }
	bool IsAttached () { return is_attached; }
	bool AlwaysChange () { return always_change; }

	Value *GetDefaultValue () { return default_value; }

	AnimationStorage *AttachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	void DetachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	AnimationStorage *GetAnimationStorageFor (DependencyObject *obj);
	
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value);
	static DependencyProperty *Register (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype);
	static DependencyProperty *RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change = false);

	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name);
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name, bool inherits);
	
	static void Shutdown ();

private:
	static GHashTable *properties;
	GHashTable *storage_hash; // keys: objects, values: animation storage's

	bool is_readonly;
	bool is_nullable;
	bool is_attached;
	bool always_change; // determines if SetValue will do something if the current and new values are equal.
	
	char *hash_key;
	char *name;

	Value *default_value;

	Type::Kind owner_type;
	Type::Kind property_type;
};

G_BEGIN_DECLS

DependencyProperty *dependency_property_lookup (Type::Kind type, char *name);
char *dependency_property_get_name (DependencyProperty* property);
bool  dependency_property_is_nullable (DependencyProperty* property);
Type::Kind dependency_property_get_property_type (DependencyProperty* property);
DependencyProperty *resolve_property_path (DependencyObject **o, const char *path);

G_END_DECLS

#endif /* __MOONLIGHT_DEPENDENCY_OBJECT_H__ */
