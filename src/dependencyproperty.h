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
	DependencyProperty (Type::Kind owner_type, const char *name, Value *default_value, Type::Kind property_type, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback);

	/* @GenerateCBinding:Type=DependencyProperty,GenerateManaged=true */
	char *GetName() { return name; }
	Type::Kind GetOwnerType() { return owner_type; }
	/* @GenerateCBinding:Type=DependencyProperty,GenerateManaged=true */
	Type::Kind GetPropertyType() { return property_type; }
	
	/* @GenerateCBinding:Type=DependencyProperty,GenerateManaged=true */
	bool IsNullable () { return is_nullable; }
	bool IsReadOnly () { return is_readonly; }
	bool IsAttached () { return is_attached; }
	bool AlwaysChange () { return always_change; }
	NativePropertyChangedHandler *GetChangedCallback () { return changed_callback; }
	
	Value *GetDefaultValue () { return default_value; }

	AnimationStorage *AttachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	void DetachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	AnimationStorage *GetAnimationStorageFor (DependencyObject *obj);
	
	/* @GenerateCBinding:Type=DependencyProperty */
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value);
	static DependencyProperty *Register (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype);
	static DependencyProperty *RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change = false, NativePropertyChangedHandler *changed_callback = NULL);
	static DependencyProperty *RegisterFull (Surface *surface, Type *type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback);
	// 2.0 only, registers properties per surface.
	/* @GenerateCBinding:Type=DependencyProperty,GenerateManaged=true */
	static DependencyProperty *RegisterFull (Surface *surface, Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change, NativePropertyChangedHandler *changed_callback);

	/* @GenerateCBinding:Type=DependencyProperty,GenerateManaged=true */
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name);
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name, bool inherits);
	static DependencyProperty *GetDependencyProperty (Type *type, const char *name, bool inherits);
	static DependencyProperty *GetDependencyProperty (Surface *surface, Type::Kind type, const char *name, bool inherits);
	
	static void Shutdown () {}

private:
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
	NativePropertyChangedHandler *changed_callback;
};

G_BEGIN_DECLS

DependencyProperty *resolve_property_path (DependencyObject **o, const char *path);

/* @GenerateManaged */
DependencyProperty *dependency_property_register_managed_property (Surface *surface, const char *name, Type::Kind property_type, Type::Kind owner_type, bool attached, NativePropertyChangedHandler *callback);

G_END_DECLS

#endif /* __MOONLIGHT_DEPENDENCY_OBJECT_H__ */
