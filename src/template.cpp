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

FrameworkTemplate::FrameworkTemplate ()
{
	SetObjectType (Type::FRAMEWORKTEMPLATE);

	xaml_buffer = NULL;
	xaml_context = NULL;
}

FrameworkTemplate::~FrameworkTemplate ()
{
	if (xaml_buffer) {
		g_free (xaml_buffer);
		xaml_buffer = NULL;
	}
	delete xaml_context;
	xaml_context = NULL;
}

void
FrameworkTemplate::SetXamlBuffer (XamlContext *xaml_context, const char *xaml_buffer)
{
//	printf ("%p setting xaml buffer to %s\n", this, xaml_buffer);
	this->xaml_buffer = g_strdup (xaml_buffer);
	this->xaml_context = xaml_context;
}

DependencyObject*
FrameworkTemplate::GetVisualTree (FrameworkElement *templateBindingSource)
{
	if (xaml_buffer) {
		XamlLoader *loader = new XamlLoader (GetResourceBase(), NULL, xaml_buffer, GetSurface(), xaml_context);
		Type::Kind dummy;

		loader->SetExpandingTemplate (true);
		loader->SetTemplateOwner (templateBindingSource);
		loader->SetImportDefaultXmlns (true);

		xaml_context->SetTemplateBindingSource (templateBindingSource);

		DependencyObject *result = loader->CreateDependencyObjectFromString (xaml_buffer, true, &dummy);

		delete loader;

		if (result)
			NameScope::GetNameScope (result)->Lock ();
		return result;
	}

	return NULL;
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
DataTemplate::GetVisualTree (FrameworkElement *templateBindingSource)
{
	// DataTemplate ignores the source paramater and always uses null
	return FrameworkTemplate::GetVisualTree (NULL);
}
