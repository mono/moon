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
#include "enums.h"

Binding::Binding ()
{
 	property_path = g_strdup ("");
 	binding_mode = BindingModeOneWay;
 	is_sealed = false;
}

char *
Binding::GetPropertyPath ()
{
	return this->property_path;	
}

void
Binding::SetPropertyPath (char *path)
{
	if (this->property_path)
		g_free (this->property_path);
	this->property_path = g_strdup (path);	
}

BindingMode
Binding::GetBindingMode ()
{
	return this->binding_mode;	
}

void
Binding::SetBindingMode (BindingMode mode)
{
	this->binding_mode = mode;
}

bool
Binding::GetIsSealed ()
{
	return this->is_sealed;	
}

void
Binding::SetIsSealed (bool sealed)
{
	this->is_sealed = sealed;
}

Binding::~Binding ()
{
	if (this->property_path) {
		g_free (this->property_path);
		this->property_path = NULL;
	}
}

BindingExpressionBase::BindingExpressionBase ()
{
	this->binding = NULL;
	this->got_value = false;
	this->gv_callback = NULL;
	this->source = NULL;
	this->stored_value = NULL;
	this->sv_callback = NULL;
	this->target = NULL;
	this->target_property = NULL;
}

BindingExpressionBase::~BindingExpressionBase ()
{
	if (this->binding) {
		this->binding->unref ();
		this->binding = NULL;	
	}
	if (stored_value) {
		this->stored_value->FreeValue ();
		this->stored_value = NULL;	
	}
}

void
BindingExpressionBase::SetBinding (Binding *binding)
{
	if (this->binding)
		this->binding->unref ();
	this->binding = binding;
}

void
BindingExpressionBase::SetSource (FrameworkElement *element)
{
	// FIXME: do we want to ref the element? or at least listen for the destroy signal?
	this->source = element;
}

void
BindingExpressionBase::SetTarget (FrameworkElement *element)
{
	// We have to listen to property changes on the target
	this->target = element;	
}

void
BindingExpressionBase::AttachListener (PropertyChangeHandler handler, gpointer user_data)
{
	if (source && binding && binding->GetPropertyPath () && handler) {
		DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->GetPropertyPath ());
		if (property)
			source->AddPropertyChangeHandler (property, handler, user_data);
	}
}

void
BindingExpressionBase::DetachListener (PropertyChangeHandler handler)
{
	if (source && binding && binding->GetPropertyPath () && handler) {
		DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->GetPropertyPath ());
		if (property)
			source->RemovePropertyChangeHandler (property, handler);
	}
}

Value *
BindingExpressionBase::GetValue ()
{
	if (!got_value || true) {
		got_value = true;
		
		if (stored_value) {
			stored_value->FreeValue ();
			stored_value = NULL;
		}
		
		if (gv_callback) {
				
			Value value = gv_callback ();
			if (value.GetKind () == Type::INVALID)
				stored_value = NULL;
			else
				stored_value = new Value (value);
			value.FreeValue ();
		}
		else if (source && binding && binding->GetPropertyPath ()) {
			DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->GetPropertyPath ());
			if (property)
				stored_value = source->GetValue (property);
		}
	}
	
	return stored_value;
}

void
BindingExpressionBase::RegisterManagedOverrides (GetValueCallback gv_callback, SetValueCallback sv_callback)
{
	this->gv_callback = gv_callback;
	this->sv_callback = sv_callback;
}

void
BindingExpressionBase::UpdateSource (Value *value)
{
	if (sv_callback)
		sv_callback (value);
	else if (source && binding && binding->GetPropertyPath ()) {
		DependencyProperty *property = DependencyProperty::GetDependencyProperty (source->GetType ()->GetKind (), binding->GetPropertyPath ());
		if (property)
			source->SetValue (property, value);
	}
}
