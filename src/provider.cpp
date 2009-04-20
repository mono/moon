/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * provider.cpp: an api for PropertyValue providers (for property inheritance)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "cbinding.h"
#include "runtime.h"
#include "provider.h"
#include "control.h"
#include "frameworkelement.h"
#include "textblock.h"
#include "style.h"

//
// LocalPropertyValueProvider
//

LocalPropertyValueProvider::LocalPropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
	// XXX maybe move the "DependencyObject::current_values" hash table here?
}

LocalPropertyValueProvider::~LocalPropertyValueProvider ()
{
}

Value *
LocalPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return (Value *) g_hash_table_lookup (obj->GetLocalValues (), property);
}


//
// StylePropertyValueProvider
//

StylePropertyValueProvider::StylePropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
	style_hash = g_hash_table_new_full (g_direct_hash, g_direct_equal,
					    (GDestroyNotify)NULL,
					    (GDestroyNotify)event_object_unref);
}

void
StylePropertyValueProvider::unlink_converted_value (gpointer key, gpointer value, gpointer data)
{
	StylePropertyValueProvider *provider = (StylePropertyValueProvider*)data;
	Setter *s = (Setter*)value;

	Value *v = s->GetValue(Setter::ConvertedValueProperty);
	if (v->Is(Type::DEPENDENCY_OBJECT)) {
		DependencyObject *dob = v->AsDependencyObject();
		if (dob->GetParent() == provider->obj)
			dob->SetParent(NULL, NULL);
	}
}

StylePropertyValueProvider::~StylePropertyValueProvider ()
{
	g_hash_table_foreach (style_hash, StylePropertyValueProvider::unlink_converted_value, this);
	g_hash_table_destroy (style_hash);
}

Value*
StylePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	Setter *setter = (Setter*)g_hash_table_lookup (style_hash, property);

	if (!setter)
		return NULL;
	else
		return setter->GetValue (Setter::ConvertedValueProperty);
}

void
StylePropertyValueProvider::RecomputePropertyValue (DependencyProperty *prop)
{
	Style *style = ((FrameworkElement*)obj)->GetStyle();
	if (!style)
		return;

	DependencyProperty *property = NULL;
	Value *value = NULL;
	SetterBaseCollection *setters = style->GetSetters ();
	if (!setters)
		return;

	CollectionIterator *iter = setters->GetIterator ();
	Value *setterBase;
	int err;
	
	while (iter->Next () && (setterBase = iter->GetCurrent (&err))) {
		if (err) {
	 		// Something bad happened - what to do?
			return;
	 	}

		if (!setterBase->Is (Type::SETTER))
			continue;
		
		Setter *setter = setterBase->AsSetter ();
		if (!(value = setter->GetValue (Setter::PropertyProperty)))
			continue;

		if (!(property = value->AsDependencyProperty ()))
			continue;

		if (prop == property) {
			// the hash holds a ref
			setter->ref ();
			g_hash_table_insert (style_hash, property, setter);
			return;
		}
	}
	
}

void
StylePropertyValueProvider::SealStyle (Style *style)
{
	style->Seal();

	SetterBaseCollection *setters = style->GetSetters ();
	if (!setters)
		return;

	CollectionIterator *iter = setters->GetIterator ();
	Value *setterBase;
	int err;
	
	while (iter->Next () && (setterBase = iter->GetCurrent (&err))) {
		if (err) {
	 		// Something bad happened - what to do?
			return;
	 	}

		if (!setterBase->Is (Type::SETTER))
			continue;
		
		Setter *setter = setterBase->AsSetter ();
		Value *value;

		DependencyProperty *setter_property;
		if (!(value = setter->GetValue (Setter::PropertyProperty)))
			continue;

		if (!(setter_property = value->AsDependencyProperty ()))
			continue;

		Value *setter_value;
		if (!(setter_value = setter->GetValue (Setter::ConvertedValueProperty)))
			continue;

		// the hash holds a ref
		setter->ref ();
		g_hash_table_insert (style_hash, setter_property, setter);

		// let the DO know the property might have changed
 		obj->ProviderValueChanged (PropertyPrecedence_Style, setter_property, NULL, setter_value, true, NULL);
	}
}


//
// InheritedPropertyValueProvider
//
Value*
InheritedPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	int parentProperty = -1;

	bool inheritableProperty = false;

#define INHERIT1(p) \
	G_STMT_START {							\
	if (property->GetId () == Control::p ||				\
	    property->GetId () == TextBlock::p ||			\
	    property->GetId () == Inline::p) {				\
									\
		inheritableProperty = true;				\
									\
		if (parent->Is (Type::CONTROL))				\
			parentProperty = Control::p;			\
		else if (parent->Is (Type::TEXTBLOCK))			\
			parentProperty = TextBlock::p;			\
		else if (parent->Is (Type::INLINE))			\
			parentProperty = Inline::p;			\
	}								\
	} G_STMT_END


#define INHERIT2(p) \
	G_STMT_START {							\
	if (property->GetId () == Inline::p) {				\
									\
		inheritableProperty = true;				\
									\
		if (parent->Is (Type::TEXTBLOCK))			\
			parentProperty = TextBlock::p;			\
	}								\
	} G_STMT_END

#define INHERIT3(p) \
	G_STMT_START {							\
	if (property->GetId () == FrameworkElement::p) {		\
									\
		inheritableProperty = true;				\
									\
		if (parent->Is (Type::FRAMEWORKELEMENT))		\
			parentProperty = FrameworkElement::p;		\
	}								\
	} G_STMT_END
	
	DependencyObject *parent = NULL;
	
	if (obj->Is(Type::FRAMEWORKELEMENT)) {
		// we loop up the visual tree
		parent = ((FrameworkElement*)obj)->GetVisualParent();
		if (parent) {
			while (parent) {
				INHERIT1 (ForegroundProperty);
				INHERIT1 (FontFamilyProperty);
				INHERIT1 (FontStretchProperty);
				INHERIT1 (FontStyleProperty);
				INHERIT1 (FontWeightProperty);
				INHERIT1 (FontSizeProperty);

				INHERIT3 (LanguageProperty);

				INHERIT3 (DataContextProperty);

				if (!inheritableProperty)
					return NULL;

				if (parentProperty != -1)
					return parent->GetValue (parentProperty);

				if (parent->Is(Type::FRAMEWORKELEMENT))
					parent = ((FrameworkElement*)parent)->GetVisualParent();
				else
					parent = NULL;
			}
		}
	}
	else if (obj->Is(Type::INLINE)) {
		// skip collections
		DependencyObject *new_parent = obj->GetParent();
		while (new_parent && new_parent->Is(Type::COLLECTION))
			new_parent = new_parent->GetParent ();
		parent = new_parent;

		if (!parent)
			return NULL;

		INHERIT1 (ForegroundProperty);
		INHERIT1 (FontFamilyProperty);
		INHERIT1 (FontStretchProperty);
		INHERIT1 (FontStyleProperty);
		INHERIT1 (FontWeightProperty);
		INHERIT1 (FontSizeProperty);

		INHERIT2 (LanguageProperty);
		INHERIT2 (TextDecorationsProperty);
		INHERIT2 (FontFilenameProperty);
		INHERIT2 (FontGUIDProperty);
		
		if (!inheritableProperty)
			return NULL;

		if (parentProperty != -1) {
			return parent->GetValue (parentProperty);
		}
	}
	  
	return NULL;
}


//
// DefaultPropertyValueProvider
//

Value *
DefaultValuePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return property->GetDefaultValue ();
}


//
// AutoPropertyValueProvider
//

static gboolean
dispose_value (gpointer key, gpointer value, gpointer data)
{
	DependencyObject *obj = (DependencyObject *) data;
	Value *v = (Value *) value;
	
	if (!value)
		return true;
	
	// detach from the existing value
	if (v->Is (Type::DEPENDENCY_OBJECT)) {
		DependencyObject *dob = v->AsDependencyObject ();
		
		if (dob != NULL) {
			if (obj == dob->GetParent ()) {
				// unset its logical parent
				dob->SetParent (NULL, NULL);
			}
			
			// unregister from the existing value
			dob->RemovePropertyChangeListener (obj, NULL);
		}
	}
	
	delete v;
	
	return true;
}

AutoCreatePropertyValueProvider::AutoCreatePropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
	auto_values = g_hash_table_new (g_direct_hash, g_direct_equal);
}

AutoCreatePropertyValueProvider::~AutoCreatePropertyValueProvider ()
{
	g_hash_table_foreach_remove (auto_values, dispose_value, obj);
	g_hash_table_destroy (auto_values);
}

Value *
AutoCreatePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	Value *value;
	
	if (!property->IsAutoCreated ())
		return NULL;
	
	// return previously set auto value next
	if ((value = (Value *) g_hash_table_lookup (auto_values, property)))
		return value;
	
	value = (property->GetAutoCreator()) (obj, property);

#if SANITY
	if (!value->Is(property->GetPropertyType()))
		g_warning ("autocreated value for property '%s' (type=%s) is of incompatible type %s\n",
			   property->GetName(),
			   Type::Find (property->GetPropertyType ())->GetName(),
			   Type::Find (value->GetKind())->GetName());
#endif

	g_hash_table_insert (auto_values, property, value);
	
	obj->ProviderValueChanged (PropertyPrecedence_AutoCreate, property, NULL, value, true, NULL);
	
	return value;
}

Value *
AutoCreatePropertyValueProvider::ReadLocalValue (DependencyProperty *property)
{
	return (Value *) g_hash_table_lookup (auto_values, property);
}

void
AutoCreatePropertyValueProvider::ClearValue (DependencyProperty *property)
{
	g_hash_table_remove (auto_values, property);
}

Value* 
AutoCreators::default_autocreator (DependencyObject *instance, DependencyProperty *property)
{
	Type *type = Type::Find (property->GetPropertyType ());
	if (!type)
		return NULL;

	return Value::CreateUnrefPtr (type->CreateInstance ());
}

#define XAML_FONT_SIZE    14.666666984558105
#define XAP_FONT_SIZE     11.0

Value *
AutoCreators::CreateDefaultFontSize (DependencyObject *obj, DependencyProperty *property)
{
	Deployment *deployment;
	
	if ((deployment = Deployment::GetCurrent ()) && deployment->IsLoadedFromXap ())
		return new Value (XAP_FONT_SIZE);
	
	return new Value (XAML_FONT_SIZE);
}

