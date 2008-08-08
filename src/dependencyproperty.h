/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dependencypropery.cpp: 
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
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


typedef	void NativePropertyChangedHandler (DependencyProperty *dependency_property, DependencyObject *dependency_object, Value *old_value, Value *new_value);

//
// DependencyProperty
//
class DependencyProperty {
 public:
	DependencyProperty () {};
	~DependencyProperty ();
	DependencyProperty (Type::Kind owner_type, const char *name, Value *default_value, Type::Kind property_type, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback, bool is_custom);

	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetName() { return name; }
	Type::Kind GetOwnerType() { return owner_type; }
	/* @GenerateCBinding,GeneratePInvoke */
	Type::Kind GetPropertyType() { return property_type; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool IsNullable () { return is_nullable; }
	bool IsReadOnly () { return is_readonly; }
	bool IsAttached () { return is_attached; }
	bool AlwaysChange () { return always_change; }
	bool IsCustom () { return is_custom; }
	NativePropertyChangedHandler *GetChangedCallback () { return changed_callback; }
	
	Value *GetDefaultValue () { return default_value; }

	AnimationStorage *AttachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	void DetachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	AnimationStorage *GetAnimationStorageFor (DependencyObject *obj);
	
	/* @GenerateCBinding */
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value);
	static DependencyProperty *Register (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype);
	static DependencyProperty *RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool read_only, bool always_change = false, NativePropertyChangedHandler *changed_callback = NULL);
	static DependencyProperty *RegisterFull (Types *additional_types, Type *type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool read_only, bool always_change, NativePropertyChangedHandler *changed_callback, bool is_custom);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	static DependencyProperty *RegisterFull (Types *additional_types, Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool read_only, bool always_change, NativePropertyChangedHandler *changed_callback);
	
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	static DependencyProperty *RegisterManagedProperty (Types *additional_types, const char *name, Type::Kind property_type, Type::Kind owner_type, bool attached, NativePropertyChangedHandler *callback);
	
	/* @GenerateCBinding,GeneratePInvoke */
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name);
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name, bool inherits);
	static DependencyProperty *GetDependencyProperty (Type *type, const char *name, bool inherits);
	static DependencyProperty *GetDependencyProperty (Types *additional_types, Type::Kind type, const char *name, bool inherits);
	
	static void Shutdown () {}

private:
	GHashTable *storage_hash; // keys: objects, values: animation storage's

	bool is_readonly;
	bool is_nullable;
	bool is_attached;
	bool always_change; // determines if SetValue will do something if the current and new values are equal.
	bool is_custom; // If created using managed api
	
	char *hash_key;
	char *name;

	Value *default_value;

	Type::Kind owner_type;
	Type::Kind property_type;
	NativePropertyChangedHandler *changed_callback;
};

G_BEGIN_DECLS

DependencyProperty *resolve_property_path (DependencyObject **o, const char *path);
void dependency_property_g_init ();
G_END_DECLS

#endif /* __MOONLIGHT_DEPENDENCY_OBJECT_H__ */
