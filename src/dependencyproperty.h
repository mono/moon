/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dependencypropery.cpp: 
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __DEPENDENCY_PROPERTY_H__
#define __DEPENDENCY_PROPERTY_H__

#include <glib.h>


#include "value.h"
#include "enums.h"
#include "list.h"

class PropertyChangedEventArgs;
class MoonError;

typedef	bool ValueValidator (DependencyObject *instance, DependencyProperty *property, Value *value, MoonError *error);
typedef Value* AutoCreator  (DependencyObject *instance, DependencyProperty *property);

/* @CBindingRequisite */
typedef void (* PropertyChangeHandler) (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error, gpointer closure);

//
// DependencyProperty
//
/* @IncludeInKinds */
/* @SkipValue */
class DependencyProperty {
 public:
	DependencyProperty (Type::Kind owner_type, const char *name, Value *default_value, Type::Kind property_type, bool attached, bool readonly, bool always_change, PropertyChangeHandler changed_callback, ValueValidator *validator, AutoCreator *autocreator, bool is_custom);
	void Dispose ();
	~DependencyProperty ();

	int GetId () { return id; }
	void SetId (int value) { id = value; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetName() { return name; }
	const char *GetHashKey ();
	Type::Kind GetOwnerType() { return owner_type; }
	/* @GenerateCBinding,GeneratePInvoke */
	Type::Kind GetPropertyType() { return property_type; }

	/* @GenerateCBinding,GeneratePInvoke */
	bool IsNullable () { return is_nullable; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetIsNullable (bool value) { is_nullable = value; }
	/* @GenerateCBinding,GeneratePInvoke */
	bool IsReadOnly () { return is_readonly; }
	/* @GenerateCBinding,GeneratePInvoke */
	bool IsAttached () { return is_attached; }
	bool IsAutoCreated () { return autocreator != NULL; }
	AutoCreator* GetAutoCreator () { return autocreator; }
	bool AlwaysChange () { return always_change; }
	bool IsCustom () { return is_custom; }
	PropertyChangeHandler GetChangedCallback () { return changed_callback; }

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	Value *GetDefaultValue () { return default_value; }

	bool Validate (DependencyObject *instance, Value *value, MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	void SetPropertyChangedCallback (PropertyChangeHandler changed_callback);
	
	static int Register (Types *types, Type::Kind type, const char *name, bool is_custom, Value *default_value);
	static int Register (Types *types, Type::Kind type, const char *name, bool is_custom, Type::Kind vtype);
	static int Register (Types *types, Type::Kind type, const char *name, bool is_custom, Value *default_value, Type::Kind vtype);
	static int RegisterFull (Types *types, Type::Kind type, const char *name, bool is_custom, Value *default_value, Type::Kind vtype, bool attached, bool read_only, bool always_change, PropertyChangeHandler changed_callback, ValueValidator *validator,  AutoCreator* autocreator, bool is_nullable);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	static DependencyProperty *RegisterCustomProperty (const char *name, Type::Kind property_type, Type::Kind owner_type, Value *defaultValue, bool attached, bool read_only, PropertyChangeHandler callback);
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	static DependencyProperty *RegisterCoreProperty (const char *name, Type::Kind property_type, Type::Kind owner_type, Value *defaultValue, bool attached, bool read_only, PropertyChangeHandler callback);
	
	/* @GenerateCBinding,GeneratePInvoke */
	MOON_API static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name);
	MOON_API static DependencyProperty *GetDependencyProperty (Type *type, const char *name);
	static DependencyProperty *GetDependencyProperty (Type *type, const char *name, bool inherits);
	
	/* @GenerateCBinding,GeneratePInvoke */
	static DependencyProperty *GetDependencyPropertyFull (Type::Kind type, const char *name, bool inherits);
	static DependencyProperty *GetDependencyPropertyFull (Type *type, const char *name, bool inherits);

private:
	int id;
	
	AutoCreator* autocreator; // invoked by AutoCreatePropertyValueProvider to create values
	bool is_readonly;
	bool is_nullable;
	bool is_attached;
	bool always_change; // determines if SetValue will do something if the current and new values are equal.
	bool is_custom; // If created using managed api
	
	char *hash_key;
	char *name;

	Value *default_value;
	ValueValidator *validator;

	Type::Kind owner_type;
	Type::Kind property_type;
	PropertyChangeHandler changed_callback;
};

G_BEGIN_DECLS

DependencyProperty *resolve_property_path (DependencyObject **o, PropertyPath *propertypath, GHashTable *promoted_values);

G_END_DECLS

#endif /* __DEPENDENCY_PROPERTY_H__ */
