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
#include "text.h"
#include "style.h"

Value*
AnimationPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	// look up the Applier/AnimationStorage for this object/property and return the value
	return NULL;
}

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
	return (Value *) g_hash_table_lookup (obj->GetCurrentValues (), property);
}

StylePropertyValueProvider::StylePropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
	style_hash = g_hash_table_new_full (g_direct_hash, g_direct_equal,
					    (GDestroyNotify)NULL,
					    (GDestroyNotify)event_object_unref);
}

StylePropertyValueProvider::~StylePropertyValueProvider ()
{
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

static DependencyObject*
get_parent (DependencyObject *obj)
{
	DependencyObject *parent = NULL;

	if (obj->Is (Type::UIELEMENT)) {
		UIElement *ui = (UIElement*)obj;
		if (ui->GetVisualParent() != NULL)
			parent = ui->GetVisualParent();
	}

	if (!parent)
		parent = obj->GetLogicalParent();

	return parent;
}

Value*
InheritedPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	DependencyObject *parent = get_parent (obj);

	if (!parent)
		return NULL;

	DependencyProperty *parent_property = NULL;

#define INHERIT1(p) \
	G_STMT_START {									\
	if (property == Control::p ||							\
	    property == TextBlock::p ||							\
	    property == Inline::p) {							\
											\
		do {									\
			if (parent->Is (Type::CONTROL))					\
				parent_property = Control::p;				\
			else if (parent->Is (Type::TEXTBLOCK))				\
				parent_property = TextBlock::p;				\
			else if (parent->Is (Type::INLINE))				\
				parent_property = Inline::p;				\
											\
			if (!parent_property)						\
				parent = get_parent (parent);				\
		} while (parent && !parent_property);					\
	}										\
	} G_STMT_END

#define INHERIT2(p) \
	G_STMT_START {									\
	if (property == Inline::p) {							\
											\
		do {									\
			if (parent->Is (Type::TEXTBLOCK))				\
				parent_property = TextBlock::p;				\
											\
			if (!parent_property)						\
				parent = get_parent (parent);				\
		} while (parent && !parent_property);					\
	}										\
	} G_STMT_END

#define INHERIT3(p) \
	G_STMT_START {									\
	if (property == FrameworkElement::p) {						\
		do {									\
			if (parent->Is (Type::FRAMEWORKELEMENT)) {			\
				Value *contextV = parent->ReadLocalValue (FrameworkElement::p); \
				if (contextV)						\
					return contextV;				\
			}								\
			parent = get_parent (parent);					\
		} while (parent);							\
	}										\
	} G_STMT_END


	INHERIT1 (ForegroundProperty);
	INHERIT1 (FontFamilyProperty);
	INHERIT1 (FontStretchProperty);
	INHERIT1 (FontStyleProperty);
	INHERIT1 (FontWeightProperty);
	INHERIT1 (FontSizeProperty);

	INHERIT2 (TextDecorationsProperty);
	INHERIT2 (FontFilenameProperty);

	if (parent_property)
		return parent->GetValue (parent_property);

	INHERIT3 (DataContextProperty);

	return NULL;
}

Value *
DefaultValuePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return property->GetDefaultValue ();
}
