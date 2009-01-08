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

#include "runtime.h"
#include "provider.h"
#include "control.h"
#include "frameworkelement.h"
#include "text.h"
#include "style.h"

PropertyValueProvider::PropertyValueProvider (DependencyObject *obj)
{
	this->obj = obj;
}

PropertyValueProvider::~PropertyValueProvider ()
{
}

AnimationPropertyValueProvider::AnimationPropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
}

AnimationPropertyValueProvider::~AnimationPropertyValueProvider ()
{
}

Value*
AnimationPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	// look up the Applier for this object/property and return the value
	return NULL;
}

LocalPropertyValueProvider::LocalPropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
	// XXX move the "DependencyObject::current_values" hash table here
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
	// attach an handler for obj->StyleProperty changing
}

StylePropertyValueProvider::~StylePropertyValueProvider ()
{
}

Value*
StylePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	if (!obj->Is(Type::FRAMEWORKELEMENT))
		return NULL;

#if notyet
	// XXX doesn't exist yet
	FrameworkElement *el = (FrameworkElement*)obj;

	return el->GetStyle()->GetValueForProperty (property);
#endif
	return NULL;
}

InheritedPropertyValueProvider::InheritedPropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
}

InheritedPropertyValueProvider::~InheritedPropertyValueProvider ()
{
}

Value*
InheritedPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	if (!obj->Is(Type::UIELEMENT))
		return NULL;

	UIElement *ui = (UIElement*)obj;

	if (ui->GetVisualParent() == NULL)
		return NULL;

	Type::Kind parent_kind = ui->GetVisualParent()->GetObjectType();

	DependencyProperty *parent_property = NULL;

#define INHERIT(p) \
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


	INHERIT (ForegroundProperty);
	INHERIT (FontFamilyProperty);
	INHERIT (FontStyleProperty);
	INHERIT (FontWeightProperty);
	INHERIT (FontSizeProperty);

	if (parent_property)
		return ui->GetVisualParent()->GetValue (parent_property);

	return NULL;
}

NaturalMediaSizePropertyValueProvider::NaturalMediaSizePropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
}

NaturalMediaSizePropertyValueProvider::~NaturalMediaSizePropertyValueProvider ()
{
}

Value*
NaturalMediaSizePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return NULL;
}

DefaultValuePropertyValueProvider::DefaultValuePropertyValueProvider (DependencyObject *obj)
	: PropertyValueProvider (obj)
{
}

DefaultValuePropertyValueProvider::~DefaultValuePropertyValueProvider ()
{
}

Value*
DefaultValuePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return property->GetDefaultValue ();
}
