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

Value*
LocalPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return (Value*)g_hash_table_lookup (obj->GetCurrentValues(), property);
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
		return setter->GetValue (Setter::ValueProperty);
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

	// XXX replace this with a hash lookup.  create the hash table when the style is sealed?

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

		// the hash holds a ref
		setter->ref ();
		g_hash_table_insert (style_hash, property, setter);
	}
}

Value*
InheritedPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	DependencyObject *parent = NULL;

	if (obj->Is (Type::UIELEMENT)) {
		UIElement *ui = (UIElement*)obj;
		if (ui->GetVisualParent() != NULL)
			parent = ui->GetVisualParent();
	}

	if (!parent)
		parent = obj->GetLogicalParent();

	if (!parent)
		return NULL;

	Type::Kind parent_kind = parent->GetObjectType();

	DependencyProperty *parent_property = NULL;

#define INHERIT1(p) \
	G_STMT_START { \
	if (property == Control::p || \
	    property == TextBlock::p || \
	    property == Inline::p) { \
		switch (parent_kind) { \
		case Type::CONTROL:   parent_property = Control::p; break; \
		case Type::TEXTBLOCK: parent_property = TextBlock::p; break; \
		case Type::INLINE:    parent_property = Inline::p; break; \
		default:              parent_property = NULL; break; \
		} \
	} \
	} G_STMT_END

#define INHERIT2(p) \
	G_STMT_START { \
	if (property == Inline::p) { \
		switch (parent_kind) { \
		case Type::TEXTBLOCK: parent_property = TextBlock::p; break; \
		default:              parent_property = NULL; break; \
		} \
	} \
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

	return NULL;
}

Value*
DefaultValuePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return property->GetDefaultValue ();
}
