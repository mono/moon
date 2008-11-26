/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * binding.cpp: data binding
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "binding.h"


BindingExpressionBase::BindingExpressionBase ()
{
	binding = NULL;
	source = NULL;
	target = NULL;
	target_property = NULL;
}

BindingExpressionBase::~BindingExpressionBase ()
{
	delete binding;
}

void
BindingExpressionBase::SetBinding (Binding *binding)
{
	delete this->binding;
	this->binding = binding;
}

void
BindingExpressionBase::SetSource (FrameworkElement *element)
{
	// FIXME: do we want to ref the element? or at least listen for the destroy signal?
	this->source = element;
}

void
BindingExpressionBase::AttachListener (PropertyChangeHandler handler, gpointer user_data)
{
	if (source && binding && binding->property_path && handler) {
		DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->property_path);
		if (property)
			source->AddPropertyChangeHandler (property, handler, user_data);
	}
}

void
BindingExpressionBase::DetachListener (PropertyChangeHandler handler)
{
	if (source && binding && binding->property_path && handler) {
		DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->property_path);
		if (property)
			source->RemovePropertyChangeHandler (property, handler);
	}
}

Value *
BindingExpressionBase::GetValue ()
{
	if (source && binding && binding->property_path) {
		DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->property_path);
		if (property)
			return source->GetValue (property);
	}
	
	return NULL;
}

void
BindingExpressionBase::UpdateSource (Value *value)
{
	if (source && binding && binding->property_path) {
		DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->property_path);
		if (property)
			source->SetValue (property, value);
	}
}
