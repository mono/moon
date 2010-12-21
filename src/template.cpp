/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * template.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>
#include "template.h"
#include "namescope.h"
#include "deployment.h"

namespace Moonlight {

FrameworkTemplate::FrameworkTemplate ()
{
	SetObjectType (Type::FRAMEWORKTEMPLATE);

	holdManagedRef = false;
	xaml_buffer = NULL;
	parse_template = NULL;
	parse_template_data = NULL;
}

void
FrameworkTemplate::Dispose ()
{
	ClearXamlBuffer ();
	DependencyObject::Dispose ();
}

void
FrameworkTemplate::ClearXamlBuffer ()
{
	if (xaml_buffer) {
		g_free (xaml_buffer);
		xaml_buffer = NULL;
	}
	if (holdManagedRef && clearManagedRef && !GetDeployment ()->IsShuttingDown ()) {
		// No need to strengthen the Value* because we're deleting it next
		clearManagedRef (this, this->parse_template_data, "XamlContext");
	}
	parse_template = NULL;
	delete parse_template_data;
	parse_template_data = NULL;
	GetDeployment ()->RemoveHandler (Deployment::ShuttingDownEvent, ShuttingDownEventCallback, this);
}

void
FrameworkTemplate::SetXamlBuffer (parse_template_func parse_template, Value *parse_template_data, const char *xaml_buffer, bool holdManagedRef)
{
	this->holdManagedRef = holdManagedRef;
	this->xaml_buffer = g_strdup (xaml_buffer);
	this->parse_template = parse_template;
	this->parse_template_data = new Value (*parse_template_data);
	if (holdManagedRef && addManagedRef && !GetDeployment ()->IsShuttingDown ()) {
		addManagedRef (this, this->parse_template_data, "XamlContext");
		this->parse_template_data->Weaken ();
	} else {
		// If we can't add a managed ref to the xaml context we may end up with a circular ref
		// and so we should destroy this at shutdown rather than leaking it.
		GetDeployment ()->AddHandler (Deployment::ShuttingDownEvent, ShuttingDownEventCallback, this);
	}
}

DependencyObject*
FrameworkTemplate::GetVisualTreeWithError (FrameworkElement *templateBindingSource, MoonError *error)
{
	if (xaml_buffer) {
		DependencyObject *result = parse_template (parse_template_data, GetResourceBase (), GetDeployment ()->GetSurface (), templateBindingSource, xaml_buffer, error);

		if (result)
			NameScope::GetNameScope (result)->Lock ();
		return result;
	}

	return NULL;
}

void
FrameworkTemplate::ShuttingDownEventHandler (Deployment *sender, EventArgs *args)
{
	// The simple act of clearing the xaml buffer may end up with our destruction,
	// so make sure we don't get destructed during cleanup, since we'll crash. Note
	// that we're detaching from the ShuttingDownEvent in ClearXamlBuffer, so we're
	// losing the ref our caller has.
	ref ();
	ClearXamlBuffer ();
	unref ();
}

ControlTemplate::ControlTemplate ()
{
	SetObjectType (Type::CONTROLTEMPLATE);
}

DataTemplate::DataTemplate ()
{
	SetObjectType (Type::DATATEMPLATE);
}

DependencyObject *
DataTemplate::GetVisualTreeWithError (FrameworkElement *templateBindingSource, MoonError *error)
{
	// DataTemplate ignores the source paramater and always uses null
	return FrameworkTemplate::GetVisualTreeWithError (NULL, error);
}

};
