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

FrameworkTemplate::FrameworkTemplate ()
{
	SetObjectType (Type::FRAMEWORKTEMPLATE);

	xaml_buffer = NULL;
	xaml_context = NULL;
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
	delete xaml_context;
	xaml_context = NULL;
	GetDeployment ()->RemoveHandler (Deployment::ShuttingDownEvent, ShuttingDownEventCallback, this);
}

void
FrameworkTemplate::SetXamlBuffer (XamlContext *xaml_context, const char *xaml_buffer)
{
//	printf ("%p setting xaml buffer to %s\n", this, xaml_buffer);
	this->xaml_buffer = g_strdup (xaml_buffer);
	this->xaml_context = xaml_context;
	GetDeployment ()->AddHandler (Deployment::ShuttingDownEvent, ShuttingDownEventCallback, this);
}

DependencyObject*
FrameworkTemplate::GetVisualTreeWithError (FrameworkElement *templateBindingSource, MoonError *error)
{
	if (xaml_buffer) {
		XamlLoader *loader = new XamlLoader (GetResourceBase(), NULL, xaml_buffer, GetDeployment ()->GetSurface(), xaml_context);
		Type::Kind dummy;

		loader->SetExpandingTemplate (true);
		loader->SetTemplateOwner (templateBindingSource);
		loader->SetImportDefaultXmlns (true);

		xaml_context->SetTemplateBindingSource (templateBindingSource);

		DependencyObject *result = loader->CreateDependencyObjectFromString (xaml_buffer, true, &dummy);

		if (error && loader->error_args && loader->error_args->GetErrorCode () != -1)
			MoonError::FillIn (error, loader->error_args);
		delete loader;

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
