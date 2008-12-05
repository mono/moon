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
	return property_path;
}

void
Binding::SetPropertyPath (const char *path)
{
	g_free (property_path);
	property_path = g_strdup (path);
}

BindingMode
Binding::GetBindingMode ()
{
	return binding_mode;
}

void
Binding::SetBindingMode (BindingMode mode)
{
	binding_mode = mode;
}

bool
Binding::GetIsSealed ()
{
	return is_sealed;
}

void
Binding::SetIsSealed (bool sealed)
{
	is_sealed = sealed;
}

Binding::~Binding ()
{
	g_free (property_path);
}

BindingExpressionBase::BindingExpressionBase ()
{
	binding = NULL;
	got_value = false;
	gv_callback = NULL;
	source = NULL;
	stored_value = NULL;
	sv_callback = NULL;
	target = NULL;
	target_property = NULL;
}

BindingExpressionBase::~BindingExpressionBase ()
{
	if (binding)
		binding->unref ();
	
	if (stored_value)
		stored_value->FreeValue ();
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
