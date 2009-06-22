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

LocalPropertyValueProvider::LocalPropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence)
	: PropertyValueProvider (obj, precedence)
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

StylePropertyValueProvider::StylePropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence)
	: PropertyValueProvider (obj, precedence)
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
			delete iter;
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
			delete iter;
			return;
		}
	}

	delete iter;
	
}

void
StylePropertyValueProvider::SealStyle (Style *style)
{
	style->Seal ();

	SetterBaseCollection *setters = style->GetSetters ();
	if (!setters)
		return;

	CollectionIterator *iter = setters->GetIterator ();
	Value *setterBase;
	int err;
	
	while (iter->Next () && (setterBase = iter->GetCurrent (&err))) {
		if (err) {
	 		// Something bad happened - what to do?
			delete iter;
			return;
	 	}

		if (!setterBase->Is (Type::SETTER))
			continue;
		
		Setter *setter = setterBase->AsSetter ();

		DependencyProperty *setter_property;
		Value *value;

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

		MoonError error;
		obj->ProviderValueChanged (precedence, setter_property, NULL, setter_value, true, &error);
	}

	delete iter;
}


static bool
IsPropertyInherited (int propertyId)
{
#define PROP_CTI(p) G_STMT_START { \
	if (propertyId == Control::p) return true; \
	if (propertyId == TextBlock::p) return true; \
	if (propertyId == Inline::p) return true; \
	} G_STMT_END
		
#define PROP_F(p) G_STMT_START { \
	if (propertyId == FrameworkElement::p) return true; \
	} G_STMT_END

#define PROP_U(p) G_STMT_START { \
	if (propertyId == UIElement::p) return true; \
        } G_STMT_END

#define PROP_I(p) G_STMT_START { \
	if (propertyId == Inline::p) return true; \
	} G_STMT_END

	PROP_CTI (ForegroundProperty);
	PROP_CTI (FontFamilyProperty);
	PROP_CTI (FontStretchProperty);
	PROP_CTI (FontStyleProperty);
	PROP_CTI (FontWeightProperty);
	PROP_CTI (FontSizeProperty);

	PROP_U (UseLayoutRoundingProperty);

	PROP_F (LanguageProperty);
	PROP_F (DataContextProperty);

	PROP_I (LanguageProperty);
	PROP_I (TextDecorationsProperty);
	PROP_I (FontFilenameProperty);
	PROP_I (FontGUIDProperty);

	return false;
}

//
// InheritedPropertyValueProvider
//
Value*
InheritedPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	int propertyId = property->GetId ();

	if (!IsPropertyInherited (propertyId))
		return NULL;

	int parentPropertyId = -1;

	Types *types =  Deployment::GetCurrent()->GetTypes();

#define INHERIT_CTI_CTI(p) \
	G_STMT_START {							\
	if (property->GetId () == Control::p ||				\
	    property->GetId () == TextBlock::p ||			\
	    property->GetId () == Inline::p) {				\
									\
		if (types->IsSubclassOf (parent->GetObjectType(), Type::CONTROL)) \
			parentPropertyId = Control::p;			\
		else if (types->IsSubclassOf (parent->GetObjectType(), Type::TEXTBLOCK)) \
			parentPropertyId = TextBlock::p;		\
	}								\
	} G_STMT_END


#define INHERIT_I_T(p) \
	G_STMT_START {							\
	if (property->GetId () == Inline::p) {				\
		parentPropertyId = TextBlock::p;			\
	}								\
	} G_STMT_END

#define INHERIT_F_F(p) \
	G_STMT_START {							\
	if (property->GetId () == FrameworkElement::p) {		\
		parentPropertyId = FrameworkElement::p;			\
	}								\
	} G_STMT_END

#define INHERIT_U_U(p) \
	G_STMT_START {							\
	if (property->GetId () == UIElement::p) {		\
		parentPropertyId = UIElement::p;			\
	}								\
	} G_STMT_END
	
	DependencyObject *parent = NULL;

	if (types->IsSubclassOf (obj->GetObjectType(), Type::FRAMEWORKELEMENT)) {
		// we loop up the visual tree
		parent = ((FrameworkElement*)obj)->GetVisualParent();
		if (parent) {
			while (parent) {
				INHERIT_CTI_CTI (ForegroundProperty);
				INHERIT_CTI_CTI (FontFamilyProperty);
				INHERIT_CTI_CTI (FontStretchProperty);
				INHERIT_CTI_CTI (FontStyleProperty);
				INHERIT_CTI_CTI (FontWeightProperty);
				INHERIT_CTI_CTI (FontSizeProperty);

				INHERIT_F_F (LanguageProperty);
				INHERIT_F_F (DataContextProperty);
				
				INHERIT_U_U (UseLayoutRoundingProperty);

				if (parentPropertyId != -1)
					return parent->GetValue (types->GetProperty (parentPropertyId));

				parent = ((FrameworkElement*)parent)->GetVisualParent();
			}
		}
	}
	else if (types->IsSubclassOf (obj->GetObjectType(), Type::INLINE)) {
		// skip collections
		DependencyObject *new_parent = obj->GetParent();
		while (new_parent && !types->IsSubclassOf (new_parent->GetObjectType(), Type::TEXTBLOCK))
			new_parent = new_parent->GetParent ();
		parent = new_parent;

		if (!parent)
			return NULL;

		INHERIT_I_T (ForegroundProperty);
		INHERIT_I_T (FontFamilyProperty);
		INHERIT_I_T (FontStretchProperty);
		INHERIT_I_T (FontStyleProperty);
		INHERIT_I_T (FontWeightProperty);
		INHERIT_I_T (FontSizeProperty);

		INHERIT_I_T (LanguageProperty);
		INHERIT_I_T (TextDecorationsProperty);
		INHERIT_I_T (FontFilenameProperty);
		INHERIT_I_T (FontGUIDProperty);
		
		if (parentPropertyId != -1) {
			return parent->GetValue (parentPropertyId);
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

AutoCreatePropertyValueProvider::AutoCreatePropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence)
	: PropertyValueProvider (obj, precedence)
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
	
	MoonError error;
	obj->ProviderValueChanged (precedence, property, NULL, value, false, &error);
	
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

