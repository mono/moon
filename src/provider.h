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
	PropertyPrecedence_LocalValue,
	PropertyPrecedence_DynamicValue, // use this level for types that need to compute property values lazily

	PropertyPrecedence_LocalStyle,
	PropertyPrecedence_DefaultStyle,

	PropertyPrecedence_Inherited,
	PropertyPrecedence_InheritedDataContext,
	PropertyPrecedence_AutoCreate,

	PropertyPrecedence_Count,

	PropertyPrecedence_Highest = PropertyPrecedence_LocalValue,
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


private:
	GHRFunc dispose_value;
	GHashTable *style_hash;
	Style *style;
};

class InheritedPropertyValueProvider : public PropertyValueProvider {
public:
	InheritedPropertyValueProvider (DependencyObject *obj, PropertyPrecedence _precedence) : PropertyValueProvider (obj, precedence) { };
	virtual ~InheritedPropertyValueProvider () { };

	virtual Value *GetPropertyValue (DependencyProperty *property);

	static bool IsPropertyInherited (int propertyId);


	// this method is used when a property changes on object @obj,
	// and that notification needs to propagate down the tree
	static void PropagateInheritedProperty (DependencyObject *obj, DependencyProperty *property, Value *old_value, Value *new_value);

	// this method is used when you add a subtree into a
	// pre-existing tree.  it propagates all inheritable
	// properties throughout the tree
	static void PropagateInheritedPropertiesOnAddingToTree (UIElement *subtreeRoot);
private:

	// given a dependency property and a descendent, this maps the
	// property to whatever corresponds to that property on the
	// descendent.
	//
	// i.e. Control::ForegroundProperty + Type::TEXTBLOCK = TextBlock::ForegroundProperty
	//
	static DependencyProperty* MapPropertyToDescendant (Types *types,
							    DependencyProperty *property,
							    Type::Kind descendantKind);
};

typedef Value* AutoCreator  (Type::Kind kind, DependencyProperty *property);

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

#endif /* __MOON_PROVIDER_H__ */
