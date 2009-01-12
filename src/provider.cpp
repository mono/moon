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

	Value *style = obj->GetValueSkippingPrecedence (FrameworkElement::StyleProperty, PropertyPrecedence_Style);

	if (!style)
		return NULL;

	Style *s = style->AsStyle ();
	return s ? s->GetPropertyValue (property) : NULL;
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
