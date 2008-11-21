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
	property = NULL;
	element = NULL;
	binding = NULL;
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
BindingExpressionBase::SetElement (FrameworkElement *element)
{
	// FIXME: do we want to ref the element? or at least listen for the destroy signal?
	this->element = element;
}

void
BindingExpressionBase::SetProperty (DependencyProperty *property)
{
	this->property = property;
}

static void
changed_cb (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer closure)
{
	FrameworkElement *listener = (FrameworkElement *) closure;
	BindingExpressionBase *expr = listener->GetBindingExpression (args->property);
	Binding *binding = expr->GetBinding ();
	
	// FIXME: args->property isn't the same property sued as a key
	// in FW::bindings... ugh, gotta figure this out.
	
	// Setting the value will unregister the binding, so grab a
	// ref before we set the new value.
	expr->ref ();
	
	// update the bound value on the listener
	listener->SetValue (expr->GetProperty (), args->new_value);
	
	// restore the binding
	if (binding->mode != BindingModeOneTime)
		listener->SetBindingExpression (args->property, expr);
	
	expr->unref ();
}

void
BindingExpressionBase::AttachListener (FrameworkElement *listener)
{
	if (element && property)
		element->AddPropertyChangeHandler (property, changed_cb, listener);
}

void
BindingExpressionBase::DetachListener ()
{
	if (element && property)
		element->RemovePropertyChangeHandler (property, changed_cb);
}

Value *
BindingExpressionBase::GetValue ()
{
	if (element && property)
		return element->GetValue (property);
	
	return NULL;
}

void
BindingExpressionBase::UpdateSource (Value *value)
{
	
}
