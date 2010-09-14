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
#include "deployment.h"
#include "error.h"

namespace Moonlight {

//
// LocalPropertyValueProvider
//

LocalPropertyValueProvider::LocalPropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence, GHRFunc dispose_value)
	: PropertyValueProvider (obj, precedence, ProviderFlags_ProvidesLocalValue)
{
	local_values = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) value_delete_value);
	this->dispose_value = dispose_value;
}

LocalPropertyValueProvider::~LocalPropertyValueProvider ()
{
	g_hash_table_foreach_remove (local_values, dispose_value, obj);
}

Value *
LocalPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return (Value *) g_hash_table_lookup (local_values, property);
}

void
LocalPropertyValueProvider::ForeachValue (GHFunc func, gpointer data)
{
	g_hash_table_foreach (local_values, func, data);
}

void
LocalPropertyValueProvider::ClearValue (DependencyProperty *property)
{
	g_hash_table_remove (local_values, property);
}

void
LocalPropertyValueProvider::SetValue (DependencyProperty *property, Value *new_value)
{
	g_hash_table_insert (local_values, property, new_value);
}

//
// StylePropertyValueProvider
//

StylePropertyValueProvider::StylePropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence, GHRFunc dispose_value)
	: PropertyValueProvider (obj, precedence, ProviderFlags_RecomputesOnClear)
{
	style = NULL;
	style_hash = g_hash_table_new_full (g_direct_hash, g_direct_equal,
					    (GDestroyNotify)NULL,
					    (GDestroyNotify)value_delete_value);
	this->dispose_value = dispose_value;
}

StylePropertyValueProvider::~StylePropertyValueProvider ()
{
	if (this->style && !Deployment::GetCurrent()->IsShuttingDown())
		this->style->RemoveHandler (EventObject::DestroyedEvent, EventObject::ClearWeakRef, &this->style);

	g_hash_table_foreach_remove (style_hash, dispose_value, obj);
	g_hash_table_destroy (style_hash);
}

Value*
StylePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return (Value *) g_hash_table_lookup (style_hash, property);
}

void
StylePropertyValueProvider::RecomputePropertyValue (DependencyProperty *prop, ProviderFlags reason, MoonError *error)
{
	// we only recompute on clear
	if ((reason & ProviderFlags_RecomputesOnClear) == 0)
		return;

	Value *old_value = NULL;
	Value *new_value = NULL;
	DependencyProperty *property = NULL;

	DeepStyleWalker walker (style);
	while (Setter *setter = walker.Step ()) {
		property = setter->GetValue (Setter::PropertyProperty)->AsDependencyProperty ();
		if (property != prop)
			continue;

		new_value = setter->GetValue (Setter::ConvertedValueProperty);
		if (new_value != NULL)
			new_value = new Value (*new_value);

		old_value = (Value *) g_hash_table_lookup (style_hash, property);

		g_hash_table_insert (style_hash, property, new_value);

		obj->ProviderValueChanged (precedence, property, old_value, new_value, true, true, true, error);
		if (error->number)
			return;
	}
}

void
StylePropertyValueProvider::UpdateStyle (Style *style, MoonError *error)
{
	Value *oldValue;
	Value *newValue;

	DeepStyleWalker oldWalker (this->style);
	DeepStyleWalker newWalker (style);

	Setter *oldSetter = oldWalker.Step ();
	Setter *newSetter = newWalker.Step ();

	while (oldSetter || newSetter) {

		DependencyProperty *oldProp = NULL;
		DependencyProperty *newProp = NULL;

		if (oldSetter)
			oldProp = oldSetter->GetValue (Setter::PropertyProperty)->AsDependencyProperty ();
		if (newSetter)
			newProp = newSetter->GetValue (Setter::PropertyProperty)->AsDependencyProperty ();
		
		if (oldProp != NULL && (oldProp < newProp || newProp == NULL)) {
			// this is a property in the old style which is not in the new style
			oldValue = oldSetter->GetValue (Setter::ConvertedValueProperty);
			newValue = NULL;

			g_hash_table_remove (style_hash, oldProp);
			obj->ProviderValueChanged (precedence, oldProp, oldValue, newValue, true, true, false, error);

			oldSetter = oldWalker.Step ();
		} else if (oldProp == newProp) {
			// This is a property which is in both styles
			oldValue = oldSetter->GetValue (Setter::ConvertedValueProperty);
			newValue = newSetter->GetValue (Setter::ConvertedValueProperty);
			if (newValue != NULL)
				newValue = new Value (*newValue);

			g_hash_table_insert (style_hash, oldProp, newValue);
			obj->ProviderValueChanged (precedence, oldProp, oldValue, newValue, true, true, false, error);

			oldSetter = oldWalker.Step ();
			newSetter = newWalker.Step ();
		} else {
			// This is a property which is only in the new style
			oldValue = NULL;
			newValue = newSetter->GetValue (Setter::ConvertedValueProperty);
			if (newValue != NULL)
				newValue = new Value (*newValue);

			g_hash_table_insert (style_hash, newProp, newValue);
			obj->ProviderValueChanged (precedence, newProp, oldValue, newValue, true, true, false, error);

			newSetter = newWalker.Step ();
		}
	}

	if (this->style)
		this->style->RemoveHandler (EventObject::DestroyedEvent, EventObject::ClearWeakRef, &this->style);
	this->style = style;
	if (this->style)
		this->style->AddHandler (EventObject::DestroyedEvent, EventObject::ClearWeakRef, &this->style);
}

void
StylePropertyValueProvider::ForeachValue (GHFunc func, gpointer data)
{
	g_hash_table_foreach (style_hash, func, data);
}

//
// InheritedPropertyValueProvider
//
InheritedPropertyValueProvider::InheritedPropertyValueProvider (DependencyObject *obj, PropertyPrecedence _precedence)
  : PropertyValueProvider (obj, precedence)
{
	propertyToSupplyingAncestor = g_hash_table_new (g_direct_hash, g_direct_equal);
}

InheritedPropertyValueProvider::~InheritedPropertyValueProvider ()
{
	g_hash_table_destroy (propertyToSupplyingAncestor);
}


InheritedPropertyValueProvider::Inheritable
InheritedPropertyValueProvider::InheritablePropertyFromPropertyId (int propertyId)
{
	if (propertyId == Control::ForegroundProperty ||
	    propertyId == TextBlock::ForegroundProperty ||
	    propertyId == TextElement::ForegroundProperty)
		return InheritedPropertyValueProvider::Foreground;

	else if (propertyId == Control::FontFamilyProperty ||
		 propertyId == TextBlock::FontFamilyProperty ||
		 propertyId == TextElement::FontFamilyProperty)
		return InheritedPropertyValueProvider::FontFamily;

	else if (propertyId == Control::FontStretchProperty ||
		 propertyId == TextBlock::FontStretchProperty ||
		 propertyId == TextElement::FontStretchProperty)
		return InheritedPropertyValueProvider::FontStretch;

	else if (propertyId == Control::FontStyleProperty ||
		 propertyId == TextBlock::FontStyleProperty ||
		 propertyId == TextElement::FontStyleProperty)
		return InheritedPropertyValueProvider::FontStyle;

	else if (propertyId == Control::FontWeightProperty ||
		 propertyId == TextBlock::FontWeightProperty ||
		 propertyId == TextElement::FontWeightProperty)
		return InheritedPropertyValueProvider::FontWeight;

	else if (propertyId == Control::FontSizeProperty ||
		 propertyId == TextBlock::FontSizeProperty ||
		 propertyId == TextElement::FontSizeProperty)
		return InheritedPropertyValueProvider::FontSize;

	else if (propertyId == FrameworkElement::LanguageProperty ||
		 propertyId == TextElement::LanguageProperty)
		return InheritedPropertyValueProvider::Language;

	else if (propertyId == FrameworkElement::FlowDirectionProperty ||
		 propertyId == Run::FlowDirectionProperty)
		return InheritedPropertyValueProvider::FlowDirection;

	else if (propertyId == UIElement::UseLayoutRoundingProperty)
		return InheritedPropertyValueProvider::UseLayoutRounding;

	else if (propertyId == TextElement::TextDecorationsProperty)
		return InheritedPropertyValueProvider::TextDecorations;

	else
		return InheritedPropertyValueProvider::InheritableNone;
}

int
InheritedPropertyValueProvider::InheritablePropertyToPropertyId (Types *types, Inheritable property, Type::Kind objectType)
{
	switch (property) {
	case InheritedPropertyValueProvider::Foreground:
		if (types->IsSubclassOf (objectType, Type::CONTROL))
			return Control::ForegroundProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::ForegroundProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::ForegroundProperty;
		break;
	case InheritedPropertyValueProvider::FontFamily:
		if (types->IsSubclassOf (objectType, Type::CONTROL))
			return Control::FontFamilyProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::FontFamilyProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::FontFamilyProperty;
		break;
	case InheritedPropertyValueProvider::FontStretch:
		if (types->IsSubclassOf (objectType, Type::CONTROL))
			return Control::FontStretchProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::FontStretchProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::FontStretchProperty;
		break;
	case InheritedPropertyValueProvider::FontStyle:
		if (types->IsSubclassOf (objectType, Type::CONTROL))
			return Control::FontStyleProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::FontStyleProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::FontStyleProperty;
		break;
	case InheritedPropertyValueProvider::FontWeight:
		if (types->IsSubclassOf (objectType, Type::CONTROL))
			return Control::FontWeightProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::FontWeightProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::FontWeightProperty;
		break;
	case InheritedPropertyValueProvider::FontSize:
		if (types->IsSubclassOf (objectType, Type::CONTROL))
			return Control::FontSizeProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::FontSizeProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::FontSizeProperty;
		break;
	case InheritedPropertyValueProvider::Language:
		if (types->IsSubclassOf (objectType, Type::FRAMEWORKELEMENT))
			return FrameworkElement::LanguageProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::LanguageProperty;
		break;
	case InheritedPropertyValueProvider::FlowDirection:
		if (types->IsSubclassOf (objectType, Type::FRAMEWORKELEMENT))
			return FrameworkElement::FlowDirectionProperty;
		else if (types->IsSubclassOf (objectType, Type::RUN))
			return Run::FlowDirectionProperty;
		break;
	case InheritedPropertyValueProvider::UseLayoutRounding:
		if (types->IsSubclassOf (objectType, Type::UIELEMENT))
			return UIElement::UseLayoutRoundingProperty;
		break;
	case InheritedPropertyValueProvider::TextDecorations:
		if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::TextDecorationsProperty;
	default:
		break;
	}

	return -1;
}

Value*
InheritedPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	int propertyId = property->GetId ();

	if (!IsPropertyInherited (propertyId))
		return NULL;

	Inheritable inheritable = InheritablePropertyFromPropertyId (propertyId);

	DependencyObject *ancestor = (DependencyObject*)g_hash_table_lookup (propertyToSupplyingAncestor, GINT_TO_POINTER(inheritable));
	if (ancestor == NULL)
		return NULL; // don't walk up the tree

	Types *types =  obj->GetDeployment()->GetTypes();

	int ancestorPropertyId = InheritablePropertyToPropertyId (types, inheritable, ancestor->GetObjectType());

	if (ancestorPropertyId == -1) {
		g_warning ("property %s should be inherited, but mapping from descendent to ancestor property doesn't exist", property->GetName());
		return NULL;
	}

	Value *v = ancestor->GetValue (ancestorPropertyId);
	if (v)
		return v;

	g_warning ("inherited value for property %s should not be NULL", property->GetName());
	return NULL;
}


bool
InheritedPropertyValueProvider::IsPropertyInherited (int propertyId)
{
	Inheritable inheritable = InheritablePropertyFromPropertyId (propertyId);
	return inheritable != InheritedPropertyValueProvider::InheritableNone;
}

bool
InheritedPropertyValueProvider::PropAdd (Types *types, DependencyObject *rootParent, DependencyObject *element, Inheritable property)
{
	// we always assign a provider source on @element (the child),
	// and it'll either be the rootParent's source (if it's value
	// is inherited) or rootParent itself (if the value is local).

	int rootPropertyId = InheritablePropertyToPropertyId (types, property, rootParent->GetObjectType());

	if (rootPropertyId == -1) {
		// the property doesn't exist on rootParent at all, so
		// we must be inheriting.
		DependencyObject *source = rootParent->GetInheritedValueSource (property);
		Value *value = NULL;

		if (source) {
			DependencyProperty *sourceProperty =
				types->GetProperty (InheritablePropertyToPropertyId (types, property, source->GetObjectType()));
			value = source->GetValue (sourceProperty);

			return element->PropagateInheritedValue (property, source, NULL, value);
		}
		else
			return false;
	}
	else {
		DependencyProperty *rootProperty = types->GetProperty (rootPropertyId);
		int root_prec = rootParent->GetPropertyValueProvider (rootProperty);
		if (root_prec == -1)
			return true;

		DependencyObject *source;
		DependencyProperty *sourceProperty = NULL;

		if (root_prec == PropertyPrecedence_Inherited) {
			source = rootParent->GetInheritedValueSource (property);
			if (source)
				sourceProperty = types->GetProperty (InheritablePropertyToPropertyId (types, property, source->GetObjectType()));
		}
		else if (root_prec < PropertyPrecedence_Inherited) {
			source = rootParent;
			sourceProperty = rootProperty;
		}
		/* FIXME what happens with lower precedences? */

		Value *sourceValue = source && sourceProperty ? source->GetValue (sourceProperty) : NULL;
		return element->PropagateInheritedValue (property, source, NULL, sourceValue);
	}
}

bool
InheritedPropertyValueProvider::PropRemove (Types *types, DependencyObject *element, Inheritable property)
{
	int propertyId = InheritablePropertyToPropertyId (types, property, element->GetObjectType());

	element->SetInheritedValueSource (property, NULL);

	if (propertyId == -1) {
		// the property doesn't exist on this element, so it's
		// necessarily inherited.  clear the value source and
		// continue on our way.
		return true;
	}
	else {
		element->SetInheritedValueSource (property, NULL);

		DependencyProperty *dp = types->GetProperty (propertyId);
		int prec = element->GetPropertyValueProvider (dp);

		if (prec == -1)
			return true;

		if (prec < PropertyPrecedence_Inherited)
			return false;

		return true;
	}
}

void
InheritedPropertyValueProvider::WalkSubtree (Types *types, DependencyObject *rootParent, DependencyObject *element, guint32 seen, bool adding)
{
	if (seen == InheritedPropertyValueProvider::InheritableAll)
		return;

	Type::Kind elementType = element->GetObjectType();

	if (types->IsSubclassOf (elementType, Type::TEXTELEMENT)
	    || types->IsSubclassOf (elementType, Type::TEXTBLOCK)) {
		int childPropId = -1;
		if (types->IsSubclassOf (elementType, Type::TEXTBLOCK))
			childPropId = TextBlock::InlinesProperty;
		else if (types->IsSubclassOf (elementType, Type::PARAGRAPH))
			childPropId = Paragraph::InlinesProperty;
		else if (types->IsSubclassOf (elementType, Type::SPAN))
			childPropId = Span::InlinesProperty;
		else if (types->IsSubclassOf (elementType, Type::SECTION))
			childPropId = Section::BlocksProperty;
			
		if (childPropId != -1) {
			Value *v = element->GetValueNoAutoCreate (childPropId);
			DependencyObjectCollection *col = v ? v->AsDependencyObjectCollection () : NULL;

			if (col) {
				int count = col->GetCount ();

				for (int i = 0; i < count; i++) {
					DependencyObject *obj = col->GetValueAt (i)->AsDependencyObject ();
					WalkTree (types, rootParent, obj, seen, adding);
				}
			}
		}
	}

	if (types->IsSubclassOf (elementType, Type::UIELEMENT)) {
		VisualTreeWalker walker ((UIElement*)element, Logical, true, types);

		while (UIElement *child = walker.Step ())
			WalkTree (types, rootParent, child, seen, adding);
	}

}


void
InheritedPropertyValueProvider::WalkTree (Types *types, DependencyObject *rootParent, DependencyObject *element, guint32 seen, bool adding)
{
	if (seen == InheritedPropertyValueProvider::InheritableAll)
		return;

#define HAS_SEEN(p)  ((seen & (InheritedPropertyValueProvider::p))!=0)
#define SEEN(p)      (seen|=(InheritedPropertyValueProvider::p))

	if (adding) {
		if (!HAS_SEEN (Foreground) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::Foreground)) SEEN (Foreground);
		if (!HAS_SEEN (FontFamily) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::FontFamily)) SEEN (FontFamily);
		if (!HAS_SEEN (FontStretch) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::FontStretch)) SEEN (FontStretch);
		if (!HAS_SEEN (FontStyle) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::FontStyle)) SEEN (FontStyle);
		if (!HAS_SEEN (FontWeight) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::FontWeight)) SEEN (FontWeight);
		if (!HAS_SEEN (FontSize) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::FontSize)) SEEN (FontSize);

		if (!HAS_SEEN (Language) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::Language)) SEEN (Language);
		if (!HAS_SEEN (FlowDirection) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::FlowDirection)) SEEN (FlowDirection);

		if (!HAS_SEEN (UseLayoutRounding) && !PropAdd (types, rootParent, element, InheritedPropertyValueProvider::UseLayoutRounding)) SEEN (UseLayoutRounding);
	}
	else {
		if (!HAS_SEEN (Foreground) && !PropRemove (types, element, InheritedPropertyValueProvider::Foreground)) SEEN (Foreground);
		if (!HAS_SEEN (FontFamily) && !PropRemove (types, element, InheritedPropertyValueProvider::FontFamily)) SEEN (FontFamily);
		if (!HAS_SEEN (FontStretch) && !PropRemove (types, element, InheritedPropertyValueProvider::FontStretch)) SEEN (FontStretch);
		if (!HAS_SEEN (FontStyle) && !PropRemove (types, element, InheritedPropertyValueProvider::FontStyle)) SEEN (FontStyle);
		if (!HAS_SEEN (FontWeight) && !PropRemove (types, element, InheritedPropertyValueProvider::FontWeight)) SEEN (FontWeight);
		if (!HAS_SEEN (FontSize) && !PropRemove (types, element, InheritedPropertyValueProvider::FontSize)) SEEN (FontSize);

		if (!HAS_SEEN (Language) && !PropRemove (types, element, InheritedPropertyValueProvider::Language)) SEEN (Language);
		if (!HAS_SEEN (FlowDirection) && !PropRemove (types, element, InheritedPropertyValueProvider::FlowDirection)) SEEN (FlowDirection);

		if (!HAS_SEEN (UseLayoutRounding) && !PropRemove (types, element, InheritedPropertyValueProvider::UseLayoutRounding)) SEEN (UseLayoutRounding);
	}

	WalkSubtree (types, rootParent, element, seen, adding);

#undef HAS_SEEN
#undef SEEN
}

void
InheritedPropertyValueProvider::PropagateInheritedPropertiesOnAddingToTree (DependencyObject *subtree)
{
	Types *types = subtree->GetDeployment ()->GetTypes ();
	WalkTree (types, obj, subtree, InheritedPropertyValueProvider::InheritableNone, true);
}

void
InheritedPropertyValueProvider::PropagateInheritedProperty (DependencyProperty *property, DependencyObject *source, DependencyObject *subtree)
{
	Types *types = source->GetDeployment ()->GetTypes ();
	Inheritable inheritable = InheritablePropertyFromPropertyId(property->GetId());
	WalkSubtree (types, source, subtree, (InheritedPropertyValueProvider::InheritableAll & ~inheritable), true);
}

void
InheritedPropertyValueProvider::ClearInheritedPropertiesOnRemovingFromTree (DependencyObject *subtree)
{
	Types *types = subtree->GetDeployment ()->GetTypes ();
	WalkTree (types, obj, subtree, InheritedPropertyValueProvider::InheritableNone, false);
}
									    
DependencyObject*
InheritedPropertyValueProvider::GetPropertySource (DependencyProperty *property)
{
	return GetPropertySource (InheritablePropertyFromPropertyId(property->GetId()));
}

DependencyObject*
InheritedPropertyValueProvider::GetPropertySource (Inheritable inheritableProperty)
{
	return (DependencyObject*)g_hash_table_lookup (propertyToSupplyingAncestor,
						       GINT_TO_POINTER (inheritableProperty));
}

void
InheritedPropertyValueProvider::SetPropertySource (Inheritable inheritableProperty, DependencyObject *source)
{
	g_hash_table_insert (propertyToSupplyingAncestor,
			     GINT_TO_POINTER (inheritableProperty),
			     source);
}

//
// AutoPropertyValueProvider
//

AutoCreatePropertyValueProvider::AutoCreatePropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence, GHRFunc dispose_value)
	: PropertyValueProvider (obj, precedence, ProviderFlags_ProvidesLocalValue)
{
	auto_values = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) value_delete_value);
	this->dispose_value = dispose_value;
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
	
	// return previously set auto value next
	if ((value = (Value *) g_hash_table_lookup (auto_values, property)))
		return value;
	
	if (!(value = property->GetDefaultValue (obj->GetObjectType (), obj)))
		return NULL;

	Deployment *deployment = obj->GetDeployment();
#if SANITY
	if (!value->Is(deployment, property->GetPropertyType()))
		g_warning ("autocreated value for property '%s' (type=%s) is of incompatible type %s\n",
			   property->GetName(),
			   Type::Find (deployment, property->GetPropertyType ())->GetName(),
			   Type::Find (deployment, value->GetKind())->GetName());
#endif

	if (value->Is (deployment, Type::EVENTOBJECT)) {
		EventObject *eo = value->AsEventObject ();
		if (eo && eo->hadManagedPeer && obj->addStrongRef) {
			obj->addStrongRef (obj, eo, property->GetName());
			if (value->GetNeedUnref ()) {
				value->SetNeedUnref (false);
				eo->unref ();
			}
		}
	}

	g_hash_table_insert (auto_values, property, value);
	
	MoonError error;
	obj->ProviderValueChanged (precedence, property, NULL, value, false, true, false, &error);
	
	return value;
}

void
AutoCreatePropertyValueProvider::RecomputePropertyValue (DependencyProperty *prop, ProviderFlags reason, MoonError *error)
{
	// we only recompute on clear
	if ((reason & ProviderFlags_RecomputesOnClear) == 0)
		return;

	ClearValue (prop);
}

void
AutoCreatePropertyValueProvider::ForeachValue (GHFunc func, gpointer data)
{
	g_hash_table_foreach (auto_values, func, data);
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
AutoCreators::default_autocreator (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj)
{
	Deployment *deployment = forObj ? forObj->GetDeployment() : Deployment::GetCurrent();

	Type *type = Type::Find (deployment, property->GetPropertyType ());
	if (!type)
		return NULL;

	return Value::CreateUnrefPtr (type->CreateInstance ());
}

#define XAML_FONT_SIZE    14.666666984558105
#define XAP_FONT_SIZE     11.0

Value *
AutoCreators::CreateDefaultFontSize (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj)
{
	Deployment *deployment = forObj ? forObj->GetDeployment() : Deployment::GetCurrent();
	
	if (deployment && deployment->IsLoadedFromXap ())
		return new Value (XAP_FONT_SIZE);
	
	return new Value (XAML_FONT_SIZE);
}

Value *
AutoCreators::CreateBlackBrush (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj)
{
	SolidColorBrush *brush = new SolidColorBrush ("black");
	brush->Freeze ();
	return Value::CreateUnrefPtr (brush);
}

Value *
AutoCreators::ControlTypeCreator (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj)
{
	ManagedTypeInfo info (Type::CONTROL, "System.Windows.Controls.Control");
	return new Value (&info);
}

InheritedDataContextValueProvider::InheritedDataContextValueProvider (DependencyObject *obj, PropertyPrecedence precedence)
	: PropertyValueProvider (obj, precedence)
{
	source = NULL;
}

InheritedDataContextValueProvider::~InheritedDataContextValueProvider ()
{
	DetachListener ();
	source = NULL;
}

Value *
InheritedDataContextValueProvider::GetPropertyValue (DependencyProperty *property)
{
	if (source == NULL || property->GetId () != FrameworkElement::DataContextProperty)
		return NULL;
	return source->GetValue (property);
}

void
InheritedDataContextValueProvider::AttachListener ()
{
	if (source != NULL) {
		DependencyProperty *prop = obj->GetDeployment ()->GetTypes ()->GetProperty (FrameworkElement::DataContextProperty);
		source->AddPropertyChangeHandler (prop, source_data_context_changed, this);
		source->AddHandler (EventObject::DestroyedEvent, source_destroyed, this);
	}
}
void
InheritedDataContextValueProvider::DetachListener ()
{
	if (source != NULL) {
		Deployment *depl = obj->GetDeployment ();
		if (depl->IsShuttingDown ())
			return;

		DependencyProperty *prop = depl->GetTypes ()->GetProperty (FrameworkElement::DataContextProperty);
		source->RemovePropertyChangeHandler (prop, source_data_context_changed);
		source->RemoveHandler (EventObject::DestroyedEvent, source_destroyed, this);
	}
}

void
InheritedDataContextValueProvider::SetDataSource (FrameworkElement *source)
{
	if (this->source == source)
		return;

	Value *old_value = this->source ? this->source->GetValue (FrameworkElement::DataContextProperty) : NULL;
	Value *new_value = source ? source->GetValue (FrameworkElement::DataContextProperty) : NULL;

	DetachListener ();
	this->source = source;
	AttachListener ();

	// This is kinda hacky - silverlight doesn't notify the listeners that the datacontext changed in this situation.
	// It waits for the element to be loaded and fires the event when that happens.
	if (old_value != new_value) {
		MoonError error;
		DependencyProperty *prop = obj->GetDeployment ()->GetTypes ()->GetProperty (FrameworkElement::DataContextProperty);
		obj->ProviderValueChanged (precedence, prop, old_value, new_value, false, false, false, &error);
	}
}

void
InheritedDataContextValueProvider::EmitChanged ()
{
	// This method exists solely so that we can notify any listeners that the inherited datacontext has changed
	// once an element is added to the live tree. This is required so that bindings refresh at the correct time.
	if (source != NULL) {
		DependencyProperty *prop = obj->GetDeployment ()->GetTypes ()->GetProperty (FrameworkElement::DataContextProperty);
		MoonError error;
		obj->ProviderValueChanged (precedence, prop, NULL, source->GetValue (prop), true, false, false, &error);
	}
}

void
InheritedDataContextValueProvider::source_data_context_changed (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error, gpointer closure)
{
	InheritedDataContextValueProvider *p = (InheritedDataContextValueProvider *) closure;
	p->obj->ProviderValueChanged (p->precedence, args->GetProperty (), args->GetOldValue (), args->GetNewValue (), true, false, false, error);
}

void
InheritedDataContextValueProvider::source_destroyed (EventObject *sender, EventArgs *args, gpointer closure)
{
	InheritedDataContextValueProvider *p = (InheritedDataContextValueProvider *) closure;
	p->SetDataSource (NULL);
}

InheritedIsEnabledValueProvider::InheritedIsEnabledValueProvider (DependencyObject *obj, PropertyPrecedence precedence)
	: PropertyValueProvider (obj, precedence, ProviderFlags_RecomputesOnLowerPriorityChange)
{
	source = NULL;
	current_value = *obj->GetValue (Control::IsEnabledProperty, PropertyPrecedence_LocalValue);
}

InheritedIsEnabledValueProvider::~InheritedIsEnabledValueProvider ()
{
	DetachListener (source);
	source = NULL;
}

Value *
InheritedIsEnabledValueProvider::GetPropertyValue (DependencyProperty *property)
{
	if (property->GetId () == Control::IsEnabledProperty)
		return &current_value;
	return NULL;
}

void
InheritedIsEnabledValueProvider::AttachListener (DependencyObject *source)
{
	if (source != NULL) {
		DependencyProperty *prop = obj->GetDeployment ()->GetTypes ()->GetProperty (Control::IsEnabledProperty);
		source->AddPropertyChangeHandler (prop, is_enabled_changed, this);
		source->AddHandler (EventObject::DestroyedEvent, source_destroyed, this);
	}
}
void
InheritedIsEnabledValueProvider::DetachListener (DependencyObject *source)
{
	if (source != NULL) {
		DependencyProperty *prop = obj->GetDeployment ()->GetTypes ()->GetProperty (Control::IsEnabledProperty);
		source->RemovePropertyChangeHandler (prop, is_enabled_changed);
		source->RemoveHandler (EventObject::DestroyedEvent, source_destroyed, this);
	}
}

void
InheritedIsEnabledValueProvider::SetDataSource (DependencyObject *source)
{
	if (source) {
		Types *types = source->GetDeployment ()->GetTypes ();
		while (source) {
			if (types->IsSubclassOf (source->GetObjectType (), Type::CONTROL))
				break;
			else if (types->IsSubclassOf (source->GetObjectType (), Type::FRAMEWORKELEMENT))
				source = ((FrameworkElement *) source)->GetLogicalParent ();
			else
				source = NULL;
		}
	}

	if (this->source != source) {
		DetachListener (this->source);
		this->source = source;
		AttachListener (this->source);
	}


	if (!source || obj->IsAttached ())
		LocalValueChanged (NULL);
}

bool
InheritedIsEnabledValueProvider::LocalValueChanged (DependencyProperty *property)
{
	if (property && property->GetId () != Control::IsEnabledProperty)
		return false;

	Value *local_enabled = obj->GetValue (Control::IsEnabledProperty, PropertyPrecedence_LocalValue);
	Value *parent_enabled = source && ((Control *) obj)->GetVisualParent () ? source->GetValue (Control::IsEnabledProperty) : NULL;
	Value new_value (local_enabled->AsBool () && (!parent_enabled || parent_enabled->AsBool ()));
	if (new_value != current_value) {
		Value old_value = current_value;
		current_value = new_value;

		MoonError error;
		obj->ProviderValueChanged (precedence, obj->GetDeployment ()->GetTypes ()->GetProperty (Control::IsEnabledProperty), new Value (old_value), new Value (new_value), true, false, false, &error);
		return true;
	}
	return false;
}

void
InheritedIsEnabledValueProvider::is_enabled_changed (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error, gpointer closure)
{
	InheritedIsEnabledValueProvider *p = (InheritedIsEnabledValueProvider *) closure;
	p->LocalValueChanged (NULL);
}

void
InheritedIsEnabledValueProvider::source_destroyed (EventObject *sender, EventArgs *args, gpointer closure)
{
	InheritedIsEnabledValueProvider *p = (InheritedIsEnabledValueProvider *) closure;
	p->SetDataSource (NULL);
}

};
