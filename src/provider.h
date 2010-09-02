/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * provider.h: an api for PropertyValue providers (for property inheritance)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_PROVIDER_H__
#define __MOON_PROVIDER_H__

#include <glib.h>

#include "type.h"
#include "value.h"

namespace Moonlight {

class EventObject;
class EventArgs;
class PropertyChangedEventArgs;
class FrameworkElement;
class DependencyObject;
class DependencyProperty;
class UIElement;
class Surface;
class Style;
struct Value;
struct MoonError;

enum PropertyPrecedence {
	PropertyPrecedence_IsEnabled,
	PropertyPrecedence_LocalValue,
	PropertyPrecedence_DynamicValue, // use this level for types that need to compute property values lazily

	PropertyPrecedence_LocalStyle,
	PropertyPrecedence_DefaultStyle,

	PropertyPrecedence_Inherited,
	PropertyPrecedence_InheritedDataContext,
	PropertyPrecedence_AutoCreate,

	PropertyPrecedence_Count,

	PropertyPrecedence_Highest = PropertyPrecedence_IsEnabled,
	PropertyPrecedence_Lowest = PropertyPrecedence_AutoCreate,
};

class PropertyValueProvider {
public:
	PropertyValueProvider (DependencyObject *_obj, PropertyPrecedence _precedence) : obj(_obj), precedence(_precedence) { }
	virtual ~PropertyValueProvider () { }

	virtual Value *GetPropertyValue (DependencyProperty *property) = 0;

	virtual void RecomputePropertyValue (DependencyProperty *property, MoonError *error) { }

protected:
	DependencyObject *obj;
	PropertyPrecedence precedence;
};


class LocalPropertyValueProvider : public PropertyValueProvider {
public:
	LocalPropertyValueProvider (DependencyObject *obj, PropertyPrecedence _precedence, GHRFunc dispose_value);
	virtual ~LocalPropertyValueProvider ();

	virtual Value *GetPropertyValue (DependencyProperty *property);
 private:
	GHRFunc dispose_value;
};

class StylePropertyValueProvider : public PropertyValueProvider {
public:
	StylePropertyValueProvider (DependencyObject *obj, PropertyPrecedence _precedence, GHRFunc dispose_value);
	virtual ~StylePropertyValueProvider ();

	virtual Value *GetPropertyValue (DependencyProperty *property);

	virtual void RecomputePropertyValue (DependencyProperty *property, MoonError *error);

	void UpdateStyle (Style *Style, MoonError *error);

	GHashTable *style_hash;

private:
	GHRFunc dispose_value;
	Style *style;
};

class InheritedPropertyValueProvider : public PropertyValueProvider {
public:
	InheritedPropertyValueProvider (DependencyObject *obj, PropertyPrecedence _precedence);
	virtual ~InheritedPropertyValueProvider ();

	virtual Value *GetPropertyValue (DependencyProperty *property);

	DependencyObject* GetPropertySource (DependencyProperty *property);
	void SetPropertySource (DependencyProperty *property, DependencyObject *source);

	static bool IsPropertyInherited (int propertyId);


	// this method is used when a property changes on object @obj,
	// and that notification needs to propagate down the tree
	void PropagateInheritedProperty (DependencyProperty *property, DependencyObject *source);

	// this method is used when you add a subtree into a
	// pre-existing tree.  it propagates all inheritable
	// properties throughout the tree
	void PropagateInheritedPropertiesOnAddingToTree (DependencyObject *subtree);
private:
	enum Inheritable {
		Foreground = 1 << 0,
		FontFamily = 1 << 1,
		FontStretch = 1 << 2,
		FontStyle = 1 << 3,
		FontWeight = 1 << 4,
		FontSize = 1 << 5,
		Language = 1 << 6,
		FlowDirection = 1 << 7,
		UseLayoutRounding = 1 << 8,
		TextDecorations = 1 << 9,

		InheritableAll = 0x1ff,
		InheritableNone = 0
	};

	static Inheritable InheritablePropertyFromPropertyId (int propertyId);
	static int InheritablePropertyToPropertyId (Types *types, Inheritable property, Type::Kind objectType);

	bool PROP_ADD (Types *types, DependencyObject *rootParent, DependencyObject *element, Inheritable property);

	static int MapPropertyToAncestor (Types *types,
					  int propertyId,
					  Type::Kind ancestorKind);

	void walk_subtree (Types *types, DependencyObject *rootParent, DependencyObject *element, guint32 seen);
	void walk_tree (Types *types, DependencyObject *rootParent, DependencyObject *element, guint32 seen);

	GHashTable *propertyToSupplyingAncestor;
};

typedef Value* AutoCreator  (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj);

class AutoCreators {
public:
	static AutoCreator default_autocreator;
	static AutoCreator CreateDefaultFontSize;
	static AutoCreator CreateBlackBrush;
	static AutoCreator ControlTypeCreator;
};

class AutoCreatePropertyValueProvider : public PropertyValueProvider {
 public:
	GHashTable *auto_values;

	AutoCreatePropertyValueProvider (DependencyObject *obj, PropertyPrecedence _precedence, GHRFunc dispose_value);
	virtual ~AutoCreatePropertyValueProvider ();

	virtual Value *GetPropertyValue (DependencyProperty *property);
	
	Value *ReadLocalValue (DependencyProperty *property);
	void ClearValue (DependencyProperty *property);

 private:
	GHRFunc dispose_value;
};

class InheritedDataContextValueProvider : public PropertyValueProvider {
public:
	InheritedDataContextValueProvider (DependencyObject *obj, PropertyPrecedence precedence);
	virtual ~InheritedDataContextValueProvider ();

	void EmitChanged ();
	virtual Value *GetPropertyValue (DependencyProperty *property);
	void SetDataSource (FrameworkElement *source);
private:
	static void source_data_context_changed (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error, gpointer closure);
	static void source_destroyed (EventObject *sender, EventArgs *args, gpointer closure);
	void AttachListener ();
	void DetachListener ();
	FrameworkElement *source;
};

class InheritedIsEnabledValueProvider : public PropertyValueProvider {
public:
	InheritedIsEnabledValueProvider (DependencyObject *obj, PropertyPrecedence precedence);
	virtual ~InheritedIsEnabledValueProvider ();

	bool LocalValueChanged (DependencyProperty *property);
	virtual Value *GetPropertyValue (DependencyProperty *property);
	void SetDataSource (DependencyObject *source);
private:
	static void is_enabled_changed (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error, gpointer closure);
	static void source_destroyed (EventObject *sender, EventArgs *args, gpointer closure);
	void AttachListener (DependencyObject *source);
	void DetachListener (DependencyObject *source);
	DependencyObject *source;
	Value current_value;
};

};
#endif /* __MOON_PROVIDER_H__ */
