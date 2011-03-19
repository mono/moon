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
#include "textblock.h"
#include "popup.h"
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
	local_values = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) Value::DeleteValue);
	this->dispose_value = dispose_value;
}

LocalPropertyValueProvider::~LocalPropertyValueProvider ()
{
	g_hash_table_foreach_remove (local_values, dispose_value, obj);
	g_hash_table_destroy (local_values);
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
					    (GDestroyNotify)Value::DeleteValue);
	this->dispose_value = dispose_value;
}

StylePropertyValueProvider::~StylePropertyValueProvider ()
{
	if (this->style)
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

	Value old_value;
	Value *new_value = NULL;
	DependencyProperty *property = NULL;

	DeepStyleWalker walker (style);
	while (Setter *setter = walker.Step ()) {
		property = setter->GetValue (Setter::PropertyProperty)->AsDependencyProperty ();
		if (property != prop)
			continue;

		new_value = setter->GetValue (Setter::ConvertedValueProperty);
		if (new_value != NULL) {
			new_value = new Value (*new_value);
			new_value->Weaken (style->GetDeployment ());
		}

		old_value = *(Value *) g_hash_table_lookup (style_hash, property);

		g_hash_table_insert (style_hash, property, new_value);

		obj->ProviderValueChanged (precedence, property, &old_value, new_value, true, true, true, error);
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
	Deployment *deployment = style ? style->GetDeployment () : Deployment::GetCurrent ();

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
			if (newValue != NULL) {
				newValue = new Value (*newValue);
				newValue->Weaken (deployment);
			}

			g_hash_table_insert (style_hash, oldProp, newValue);
			obj->ProviderValueChanged (precedence, oldProp, oldValue, newValue, true, true, false, error);

			oldSetter = oldWalker.Step ();
			newSetter = newWalker.Step ();
		} else {
			// This is a property which is only in the new style
			oldValue = NULL;
			newValue = newSetter->GetValue (Setter::ConvertedValueProperty);
			if (newValue != NULL) {
				newValue = new Value (*newValue);
				newValue->Weaken (deployment);
			}

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
// ImplicitStylePropertyValueProvider
//

ImplicitStylePropertyValueProvider::ImplicitStylePropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence, GHRFunc dispose_value)
	: PropertyValueProvider (obj, precedence, ProviderFlags_RecomputesOnClear)
{
	styles = NULL;
	style_mask = StyleMaskNone;
	style_hash = g_hash_table_new_full (g_direct_hash, g_direct_equal,
					    (GDestroyNotify)NULL,
					    (GDestroyNotify)Value::DeleteValue);
	this->dispose_value = dispose_value;
}

ImplicitStylePropertyValueProvider::~ImplicitStylePropertyValueProvider ()
{
	if (styles) {
		for (int i = 0; styles[i]; i ++) {
			if (styles[i]) {
				styles[i]->RemoveHandler (Style::DetachedEvent, ImplicitStylePropertyValueProvider::style_detached, this);
				styles[i]->RemoveHandler (EventObject::DestroyedEvent, EventObject::ClearWeakRef, &styles[i]);
			}
		}
		g_free (styles);
	}

	g_hash_table_foreach_remove (style_hash, dispose_value, obj);
	g_hash_table_destroy (style_hash);
}

Value*
ImplicitStylePropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return (Value *) g_hash_table_lookup (style_hash, property);
}

void
ImplicitStylePropertyValueProvider::RecomputePropertyValue (DependencyProperty *prop, ProviderFlags reason, MoonError *error)
{
	// we only recompute on clear
	if ((reason & ProviderFlags_RecomputesOnClear) == 0)
		return;

	if (!styles)
		return;

	Value old_value;
	Value *new_value = NULL;
	DependencyProperty *property = NULL;

	DeepStyleWalker walker (styles);
	while (Setter *setter = walker.Step ()) {
		property = setter->GetValue (Setter::PropertyProperty)->AsDependencyProperty ();
		if (property != prop)
			continue;

		new_value = setter->GetValue (Setter::ConvertedValueProperty);
		if (new_value != NULL) {
			new_value = new Value (*new_value);
			new_value->Weaken (setter->GetDeployment ());
		}

		old_value = *(Value *) g_hash_table_lookup (style_hash, property);

		g_hash_table_insert (style_hash, property, new_value);

		obj->ProviderValueChanged (precedence, property, &old_value, new_value, true, true, true, error);
		if (error->number)
			return;
	}
}

void
ImplicitStylePropertyValueProvider::ApplyStyles (ImplicitStylePropertyValueProvider::StyleMask style_mask, Style **styles, MoonError *error)
{
#if 0 && SANITY
	printf ("::ApplyStyles (");
	if (style_mask == ImplicitStylePropertyValueProvider::StyleMaskAll) {
		printf ("All)\n");
	}
	else {
		if (style_mask & ImplicitStylePropertyValueProvider::StyleMaskGenericXaml)
			printf (" generic.xaml");
		if (style_mask & ImplicitStylePropertyValueProvider::StyleMaskApplicationResources)
			printf (" appresources");
		if (style_mask & ImplicitStylePropertyValueProvider::StyleMaskVisualTree)
			printf (" virusl-tree");
		printf (" )\n");
	}
#endif

	bool changed = this->styles == NULL || style_mask != this->style_mask; // if our styles array is null, it's a given that things have changed

	if (!changed) {
		for (int i = 0; i < StyleIndexCount; i ++) {
			if (styles[i] != this->styles[i]) {
				changed = true;
				break;
			}
		}
	}

	if (!changed) {
#if 0 && SANITY
		printf ("ImplicitStylePropertyValueProvider::ApplyStyles called but there was no change to the Styles\n");
#endif
		return;
	}

	Value *oldValue;
	Value *newValue;

	Types *types = obj->GetDeployment()->GetTypes();
	DeepStyleWalker oldWalker (this->styles, types);
	DeepStyleWalker newWalker (styles, types);

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
			if (newValue != NULL) {
				newValue = new Value (*newValue);
				newValue->Weaken (newSetter->GetDeployment ());
			}

			g_hash_table_insert (style_hash, oldProp, newValue);
			obj->ProviderValueChanged (precedence, oldProp, oldValue, newValue, true, true, false, error);

			oldSetter = oldWalker.Step ();
			newSetter = newWalker.Step ();
		} else {
			// This is a property which is only in the new style
			oldValue = NULL;
			newValue = newSetter->GetValue (Setter::ConvertedValueProperty);
			if (newValue != NULL) {
				newValue = new Value (*newValue);
				newValue->Weaken (newSetter->GetDeployment ());
			}

			g_hash_table_insert (style_hash, newProp, newValue);
			obj->ProviderValueChanged (precedence, newProp, oldValue, newValue, true, true, false, error);

			newSetter = newWalker.Step ();
		}
	}

	// FIXME: we need to not RemoveHandler/AddHandler styles that aren't changing
	if (this->styles) {
		for (int i = 0; i < StyleIndexCount; i ++) {
			if (this->styles [i])
				this->styles[i]->RemoveHandler (Style::DetachedEvent, ImplicitStylePropertyValueProvider::style_detached, this);
		}
		g_free (this->styles);
	}
	this->styles = styles;
	this->style_mask = style_mask;
	if (this->styles) {
		for (int i = 0; i < StyleIndexCount; i ++) {
			if (this->styles [i])
				this->styles[i]->AddHandler (Style::DetachedEvent, ImplicitStylePropertyValueProvider::style_detached, this);
		}
	}
}

void
ImplicitStylePropertyValueProvider::SetStyles (ImplicitStylePropertyValueProvider::StyleMask style_mask, Style **styles, MoonError *e)
{
#if 0 && SANITY
	printf ("::SetStyles (");
	if (style_mask == ImplicitStylePropertyValueProvider::StyleMaskAll) {
		printf ("All)\n");
	}
	else {
		if (style_mask & ImplicitStylePropertyValueProvider::StyleMaskGenericXaml)
			printf (" generic.xaml");
		if (style_mask & ImplicitStylePropertyValueProvider::StyleMaskApplicationResources)
			printf (" appresources");
		if (style_mask & ImplicitStylePropertyValueProvider::StyleMaskVisualTree)
			printf (" virusl-tree");
		printf (" )\n");
	}
#endif

	if (!styles)
		return;
		
	Style **new_styles = (Style**)g_new0 (Style*, StyleIndexCount);
	if (this->styles)
		memmove (new_styles, this->styles, StyleIndexCount * sizeof (Style*));

	if (style_mask & StyleMaskGenericXaml)
		new_styles[StyleIndexGenericXaml] = styles[StyleIndexGenericXaml];
	if (style_mask & StyleMaskApplicationResources)
		new_styles[StyleIndexApplicationResources] = styles[StyleIndexApplicationResources];
	if (style_mask & StyleMaskVisualTree)
		new_styles[StyleIndexVisualTree] = styles[StyleIndexVisualTree];

	ApplyStyles ((ImplicitStylePropertyValueProvider::StyleMask)(this->style_mask | style_mask),
		     new_styles,
		     e);
}

void
ImplicitStylePropertyValueProvider::ClearStyles (ImplicitStylePropertyValueProvider::StyleMask style_mask, MoonError *e)
{
#if 0 && SANITY
	printf ("::ClearStyles (");
	if (style_mask == ImplicitStylePropertyValueProvider::StyleMaskAll) {
		printf ("All)\n");
	}
	else {
		if (style_mask & StyleMaskGenericXaml)
			printf (" generic.xaml");
		if (style_mask & StyleMaskApplicationResources)
			printf (" appresources");
		if (style_mask & StyleMaskVisualTree)
			printf (" virusl-tree");
		printf (" )\n");
	}
#endif

	if (!styles)
		return;
		
	Style **new_styles = (Style**)g_new (Style*, StyleIndexCount);
	memmove (new_styles, styles, 3 * sizeof (Style*));

	if (style_mask & StyleMaskGenericXaml)
		new_styles[StyleIndexGenericXaml] = NULL;
	if (style_mask & StyleMaskApplicationResources)
		new_styles[StyleIndexApplicationResources] = NULL;
	if (style_mask & StyleMaskVisualTree)
		new_styles[StyleIndexVisualTree] = NULL;

	ApplyStyles ((ImplicitStylePropertyValueProvider::StyleMask)(this->style_mask & ~style_mask),
		     new_styles,
		     e);
}

void
ImplicitStylePropertyValueProvider::style_detached (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((ImplicitStylePropertyValueProvider*)closure)->StyleDetached ((Style*)sender);
}

void
ImplicitStylePropertyValueProvider::StyleDetached (Style *style)
{
	// printf ("calling ApplyDefaultStyle from ImplicitStylePropertyValueProvider::StyleDetached: \n");
	StyleMask mask = StyleMaskNone;

	if (styles[StyleIndexVisualTree] == style)
		mask = StyleMaskVisualTree;
	else if (styles[StyleIndexGenericXaml] == style)
		mask = StyleMaskGenericXaml;
	else if (styles[StyleIndexApplicationResources] == style)
		mask = StyleMaskApplicationResources;
		
	if (mask == StyleMaskNone)
		g_warning ("style detached that wasn't being used..");
	else {
		if (mask == StyleMaskVisualTree) {
			// in the event that the style was in the visual tree, we need to locate a possible replacement.
			((FrameworkElement*)obj)->SetImplicitStyles(mask);
		}
		else {
			// otherwise we just clear it.
			((FrameworkElement*)obj)->ClearImplicitStyles(mask);
		}
	}
}

void
ImplicitStylePropertyValueProvider::ForeachValue (GHFunc func, gpointer data)
{
	g_hash_table_foreach (style_hash, func, data);
}

//
// InheritedPropertyValueProvider
//

class InheritedContext {
public:
	InheritedContext (DependencyObject *foreground_source,
			  DependencyObject *font_family_source,
			  DependencyObject *font_stretch_source,
			  DependencyObject *font_style_source,
			  DependencyObject *font_weight_source,
			  DependencyObject *font_size_source,
			  DependencyObject *language_source,
			  DependencyObject *flow_direction_source,
			  DependencyObject *use_layout_rounding_source,
			  DependencyObject *text_decorations_source,
			  DependencyObject *font_resource_source)
	{
		this->foreground_source = foreground_source;
		this->font_family_source = font_family_source;
		this->font_stretch_source = font_stretch_source;
		this->font_style_source = font_style_source;
		this->font_weight_source = font_weight_source;
		this->font_size_source = font_size_source;
		this->language_source = language_source;
		this->flow_direction_source = flow_direction_source;
		this->use_layout_rounding_source = use_layout_rounding_source;
		this->text_decorations_source = text_decorations_source;
		this->font_resource_source = font_resource_source;
	}
	

	// initialize our sources with obj's local sources, falling
	// back to the parent context for those properties for which
	// obj doesn't have higher precedent values.
	InheritedContext (Types *types, DependencyObject *obj, InheritedContext *parent_context)
	{
		foreground_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::Foreground);
		if (!foreground_source && parent_context) foreground_source = parent_context->foreground_source;

		font_family_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::FontFamily);
		if (!font_family_source && parent_context) font_family_source = parent_context->font_family_source;

		font_stretch_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::FontStretch);
		if (!font_stretch_source && parent_context) font_stretch_source = parent_context->font_stretch_source;

		font_style_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::FontStyle);
		if (!font_style_source && parent_context) font_style_source = parent_context->font_style_source;

		font_weight_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::FontWeight);
		if (!font_weight_source && parent_context) font_weight_source = parent_context->font_weight_source;

		font_size_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::FontSize);
		if (!font_size_source && parent_context) font_size_source = parent_context->font_size_source;

		language_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::Language);
		if (!language_source && parent_context) language_source = parent_context->language_source;

		flow_direction_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::FlowDirection);
		if (!flow_direction_source && parent_context) flow_direction_source = parent_context->flow_direction_source;

		use_layout_rounding_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::UseLayoutRounding);
		if (!use_layout_rounding_source && parent_context) use_layout_rounding_source = parent_context->use_layout_rounding_source;

		text_decorations_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::TextDecorations);
		if (!text_decorations_source && parent_context) text_decorations_source = parent_context->text_decorations_source;

		font_resource_source = GetLocalSource (types, obj, InheritedPropertyValueProvider::FontResource);
		if (!font_resource_source && parent_context) font_resource_source = parent_context->font_resource_source;
	}

	~InheritedContext()
	{
	}

	// this method takes @this and @with_context, and returns the
	// original bitmask with bits cleared corresponding to
	// properties that are *not* inherited from the same source.
	//
	// we use this to determine which properties have higher
	// precedent values in @this than @with_context.
	InheritedPropertyValueProvider::Inheritable Compare (InheritedContext *with_context, InheritedPropertyValueProvider::Inheritable props)
	{
		guint32 rv = InheritedPropertyValueProvider::InheritableNone;
		if (props & InheritedPropertyValueProvider::Foreground && with_context->foreground_source == foreground_source)
			rv |= InheritedPropertyValueProvider::Foreground;
		if (props & InheritedPropertyValueProvider::FontFamily && with_context->font_family_source == font_family_source)
			rv |= InheritedPropertyValueProvider::FontFamily;
		if (props & InheritedPropertyValueProvider::FontStretch && with_context->font_stretch_source == font_stretch_source)
			rv |= InheritedPropertyValueProvider::FontStretch;
		if (props & InheritedPropertyValueProvider::FontStyle && with_context->font_style_source == font_style_source)
			rv |= InheritedPropertyValueProvider::FontStyle;
		if (props & InheritedPropertyValueProvider::FontWeight && with_context->font_weight_source == font_weight_source)
			rv |= InheritedPropertyValueProvider::FontWeight;
		if (props & InheritedPropertyValueProvider::FontSize && with_context->font_size_source == font_size_source)
			rv |= InheritedPropertyValueProvider::FontSize;
		if (props & InheritedPropertyValueProvider::Language && with_context->language_source == language_source)
			rv |= InheritedPropertyValueProvider::Language;
		if (props & InheritedPropertyValueProvider::FlowDirection && with_context->flow_direction_source == flow_direction_source)
			rv |= InheritedPropertyValueProvider::FlowDirection;
		if (props & InheritedPropertyValueProvider::UseLayoutRounding && with_context->use_layout_rounding_source == use_layout_rounding_source)
			rv |= InheritedPropertyValueProvider::UseLayoutRounding;
		if (props & InheritedPropertyValueProvider::TextDecorations && with_context->text_decorations_source == text_decorations_source)
			rv |= InheritedPropertyValueProvider::TextDecorations;
		if (props & InheritedPropertyValueProvider::FontResource && with_context->font_resource_source == font_resource_source)
			rv |= InheritedPropertyValueProvider::FontResource;

		return (InheritedPropertyValueProvider::Inheritable)rv;
	}

	// Returns obj if obj has a value of higher precedence than Inherited for the given property.
	// Returns NULL if obj inherits its value for the given property, or if obj doesn't *have* that property at all.
	DependencyObject* GetLocalSource (Types *types, DependencyObject *obj, InheritedPropertyValueProvider::Inheritable prop)
	{
		DependencyObject *source = NULL;
		DependencyProperty *dp = NULL;
		int propertyId = InheritedPropertyValueProvider::InheritablePropertyToPropertyId (types, prop, obj->GetObjectType());
		if (propertyId != -1)
			dp = types->GetProperty (propertyId);
		if (dp != NULL && obj->GetPropertyValueProvider (dp) < PropertyPrecedence_Inherited)
			source = obj;

		return source;
	}


	DependencyObject *foreground_source;
	DependencyObject *font_family_source;
	DependencyObject *font_stretch_source;
	DependencyObject *font_style_source;
	DependencyObject *font_weight_source;
	DependencyObject *font_size_source;
	DependencyObject *language_source;
	DependencyObject *flow_direction_source;
	DependencyObject *use_layout_rounding_source;
	DependencyObject *text_decorations_source;
	DependencyObject *font_resource_source;
};



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
InheritedPropertyValueProvider::InheritablePropertyFromPropertyId (DependencyObject *obj, int propertyId)
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

	else if (propertyId == FrameworkElement::FlowDirectionProperty 
		 && obj->GetObjectType() != Type::IMAGE
		 && obj->GetObjectType() != Type::MEDIAELEMENT)
		return InheritedPropertyValueProvider::FlowDirection;

	else if (propertyId == Run::FlowDirectionProperty)
		return InheritedPropertyValueProvider::FlowDirection;

	else if (propertyId == UIElement::UseLayoutRoundingProperty)
		return InheritedPropertyValueProvider::UseLayoutRounding;

	else if (propertyId == TextElement::TextDecorationsProperty || propertyId == TextBlock::TextDecorationsProperty)
		return InheritedPropertyValueProvider::TextDecorations;

	else if (propertyId == TextBlock::FontResourceProperty ||
		 propertyId == TextElement::FontResourceProperty)
		return InheritedPropertyValueProvider::FontResource;

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
		if (types->IsSubclassOf (objectType, Type::FRAMEWORKELEMENT)) {
			if (objectType == Type::IMAGE || objectType == Type::MEDIAELEMENT)
				return -1;
			return FrameworkElement::FlowDirectionProperty;
		}
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
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::TextDecorationsProperty;
	case InheritedPropertyValueProvider::FontResource:
		if (types->IsSubclassOf (objectType, Type::TEXTELEMENT))
			return TextElement::FontResourceProperty;
		else if (types->IsSubclassOf (objectType, Type::TEXTBLOCK))
			return TextBlock::FontResourceProperty;
	default:
		break;
	}

	return -1;
}

Value*
InheritedPropertyValueProvider::GetPropertyValue (DependencyProperty *property)
{
	int propertyId = property->GetId ();

	if (!IsPropertyInherited (obj, propertyId))
		return NULL;

	Inheritable inheritable = InheritablePropertyFromPropertyId (obj, propertyId);

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
InheritedPropertyValueProvider::IsPropertyInherited (DependencyObject *obj, int propertyId)
{
	Inheritable inheritable = InheritablePropertyFromPropertyId (obj, propertyId);
	return inheritable != InheritedPropertyValueProvider::InheritableNone;
}

void
InheritedPropertyValueProvider::WalkSubtree (Types *types, DependencyObject *rootParent, DependencyObject *element, InheritedContext *context, Inheritable props, bool adding)
{
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
					WalkTree (types, rootParent, obj, context, props, adding);
				}
			}
		}
	}

	if (types->IsSubclassOf (elementType, Type::POPUP)) {
		Popup *popup = (Popup*)element;
		UIElement *child = popup->GetChild();

		if (child)
			WalkTree (types, rootParent, child, context, props, adding);
	}

	if (types->IsSubclassOf (elementType, Type::UIELEMENT)) {
		VisualTreeWalker walker ((UIElement*)element, Logical, true, types);

		while (UIElement *child = walker.Step ())
			WalkTree (types, rootParent, child, context, props, adding);
	}

}

void
InheritedPropertyValueProvider::WalkTree (Types *types, DependencyObject *rootParent, DependencyObject *element, InheritedContext *context, Inheritable props, bool adding)
{
	if (props == InheritableNone)
		return;

	if (adding) {
		MaybePropagateInheritedValue (types, context->foreground_source, Foreground, props, element);
		MaybePropagateInheritedValue (types, context->font_family_source, FontFamily, props, element);
		MaybePropagateInheritedValue (types, context->font_stretch_source, FontStretch, props, element);
		MaybePropagateInheritedValue (types, context->font_style_source, FontStyle, props, element);
		MaybePropagateInheritedValue (types, context->font_weight_source, FontWeight, props, element);
		MaybePropagateInheritedValue (types, context->font_size_source, FontSize, props, element);
		MaybePropagateInheritedValue (types, context->language_source, Language, props, element);
		MaybePropagateInheritedValue (types, context->flow_direction_source, FlowDirection, props, element);
		MaybePropagateInheritedValue (types, context->use_layout_rounding_source, UseLayoutRounding, props, element);
		MaybePropagateInheritedValue (types, context->text_decorations_source, TextDecorations, props, element);
		MaybePropagateInheritedValue (types, context->font_resource_source, FontResource, props, element);

		InheritedContext element_context (types, element, context);

		props = element_context.Compare (context, props);
		if (props == InheritableNone)
			return;

		WalkSubtree (types, rootParent, element, &element_context, props, adding);
	}
	else {
		InheritedContext element_context (types, element, context);

		MaybeRemoveInheritedValue (types, context->foreground_source, Foreground, props, element);
		MaybeRemoveInheritedValue (types, context->font_family_source, FontFamily, props, element);
		MaybeRemoveInheritedValue (types, context->font_stretch_source, FontStretch, props, element);
		MaybeRemoveInheritedValue (types, context->font_style_source, FontStyle, props, element);
		MaybeRemoveInheritedValue (types, context->font_weight_source, FontWeight, props, element);
		MaybeRemoveInheritedValue (types, context->font_size_source, FontSize, props, element);
		MaybeRemoveInheritedValue (types, context->language_source, Language, props, element);
		MaybeRemoveInheritedValue (types, context->flow_direction_source, FlowDirection, props, element);
		MaybeRemoveInheritedValue (types, context->use_layout_rounding_source, UseLayoutRounding, props, element);
		MaybeRemoveInheritedValue (types, context->text_decorations_source, TextDecorations, props, element);
		MaybeRemoveInheritedValue (types, context->font_resource_source, FontResource, props, element);

		props = element_context.Compare (context, props);
		if (props == InheritableNone)
			return;

		WalkSubtree (types, rootParent, element, context, props, adding);
	}
}

void
InheritedPropertyValueProvider::MaybePropagateInheritedValue (Types *types, DependencyObject *source, Inheritable prop, Inheritable props, DependencyObject *element)
{
	if (!source) return;
	if ((props & prop) == 0) return;

	DependencyProperty *sourceProperty =
		types->GetProperty (InheritablePropertyToPropertyId (types, prop, source->GetObjectType()));
	Value *value = source->GetValue (sourceProperty);
	if (value)
		element->PropagateInheritedValue (prop, source, value);
}

void
InheritedPropertyValueProvider::MaybeRemoveInheritedValue (Types *types, DependencyObject *source, Inheritable prop, Inheritable props, DependencyObject *element)
{
	if (!source) return;
	if ((props & prop) == 0) return;

	if (source == element->GetInheritedValueSource (prop))
		element->PropagateInheritedValue (prop, NULL, NULL);
}

void
InheritedPropertyValueProvider::PropagateInheritedPropertiesOnAddingToTree (DependencyObject *subtree)
{
	Types *types = subtree->GetDeployment ()->GetTypes ();

	// initially populate our source_stack with a context
	// containing the the property sources from this->obj
	InheritedContext base_context (GetPropertySource (Foreground),
				       GetPropertySource (FontFamily),
				       GetPropertySource (FontStretch),
				       GetPropertySource (FontStyle),
				       GetPropertySource (FontWeight),
				       GetPropertySource (FontSize),
				       GetPropertySource (Language),
				       GetPropertySource (FlowDirection),
				       GetPropertySource (UseLayoutRounding),
				       GetPropertySource (TextDecorations),
				       GetPropertySource (FontResource));


	InheritedContext obj_context (types, obj, &base_context);
	WalkTree (types, obj, subtree, &obj_context, InheritableAll, true);
}

void
InheritedPropertyValueProvider::PropagateInheritedProperty (DependencyProperty *property, DependencyObject *source, DependencyObject *subtree)
{
	Inheritable inheritable = InheritablePropertyFromPropertyId(source, property->GetId());

	Types *types = source->GetDeployment ()->GetTypes ();

	// since we're only propagating one property to the subtree,
	// we don't care about anything other than @inheritable, and
	// we also don't need to create a base_context as in
	// PropagateInheritedPropertiesOnAddingToTree (since we know
	// the value is higher precedence, otherwise we wouldn't have
	// been called.)
	InheritedContext obj_context (types, obj, NULL);

	WalkSubtree (types, source, subtree, &obj_context, inheritable, true);
}

void
InheritedPropertyValueProvider::ClearInheritedPropertiesOnRemovingFromTree (DependencyObject *subtree)
{
	Types *types = subtree->GetDeployment ()->GetTypes ();

	// initially populate our source_stack with a context
	// containing the the property sources from this->obj
	InheritedContext base_context (GetPropertySource (Foreground),
				       GetPropertySource (FontFamily),
				       GetPropertySource (FontStretch),
				       GetPropertySource (FontStyle),
				       GetPropertySource (FontWeight),
				       GetPropertySource (FontSize),
				       GetPropertySource (Language),
				       GetPropertySource (FlowDirection),
				       GetPropertySource (UseLayoutRounding),
				       GetPropertySource (TextDecorations),
				       GetPropertySource (FontResource));


	InheritedContext obj_context (types, obj, &base_context);

	WalkTree (types, obj, subtree, &obj_context, InheritableAll, false);
}
									    
DependencyObject*
InheritedPropertyValueProvider::GetPropertySource (DependencyProperty *property)
{
	return GetPropertySource (InheritablePropertyFromPropertyId(obj, property->GetId()));
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
	if (source)
		g_hash_table_insert (propertyToSupplyingAncestor,
				     GINT_TO_POINTER (inheritableProperty),
				     source);
	else
		g_hash_table_remove (propertyToSupplyingAncestor,
				     GINT_TO_POINTER (inheritableProperty));
}

//
// DefaultValueProvider
//
DefaultValueProvider::DefaultValueProvider (DependencyObject *obj, PropertyPrecedence precedence)
	: PropertyValueProvider (obj, precedence)
{

}

DefaultValueProvider::~DefaultValueProvider ()
{

}

Value *
DefaultValueProvider::GetPropertyValue (DependencyProperty *property)
{
	return property->GetDefaultValue (obj->GetObjectType ());
}

//
// AutoPropertyValueProvider
//

AutoCreatePropertyValueProvider::AutoCreatePropertyValueProvider (DependencyObject *obj, PropertyPrecedence precedence, GHRFunc dispose_value)
	: PropertyValueProvider (obj, precedence, ProviderFlags_ProvidesLocalValue)
{
	auto_values = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) Value::DeleteValue);
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

	if (!(value = property->IsAutoCreated () ? property->GetAutoCreatedValue (obj->GetObjectType (), obj) : NULL))
		return NULL;

	Deployment *deployment = obj->GetDeployment();
#if SANITY
	if (!value->Is(deployment, property->GetPropertyType()))
		g_warning ("autocreated value for property '%s' (type=%s) is of incompatible type %s\n",
			   property->GetName(),
			   Type::Find (deployment, property->GetPropertyType ())->GetName(),
			   Type::Find (deployment, value->GetKind())->GetName());
#endif

	if (obj->addManagedRef && value->HoldManagedRef (deployment) && !deployment->IsShuttingDown ()) {
		obj->addManagedRef (obj, value->AsGCHandle (), property);
		value->Weaken (deployment);
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
		obj->ProviderValueChanged (precedence, obj->GetDeployment ()->GetTypes ()->GetProperty (Control::IsEnabledProperty), &old_value, &new_value, true, false, false, &error);
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
