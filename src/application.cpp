/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * application.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "application.h"

#include <mono/metadata/appdomain.h>

GHashTable* Application::current_hash = NULL;

Application::Application ()
{
	apply_default_style_cb = NULL;
	apply_style_cb = NULL;

	SetValue (Application::ResourcesProperty, Value::CreateUnref (new ResourceDictionary ()));
}

Application::~Application ()
{
}

Application*
Application::GetCurrent ()
{
	if (!current_hash)
		current_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	MonoDomain *domain;
	if (!(domain = mono_domain_get ()))
		return NULL;

	return (Application*)g_hash_table_lookup (current_hash, domain);
}

void
Application::SetCurrent (Application *application)
{
	if (!current_hash)
		current_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	MonoDomain *domain;
	if (!(domain = mono_domain_get ()))
		return;

	return g_hash_table_insert (current_hash, domain, application);
}

void
Application::RegisterStyleCallbacks (ApplyDefaultStyleCallback apply_default_style_cb,
				     ApplyStyleCallback apply_style_cb)
{
	this->apply_default_style_cb = apply_default_style_cb;
	this->apply_style_cb = apply_style_cb;
}

void
Application::ApplyDefaultStyle (FrameworkElement *fwe, ManagedTypeInfo *key)
{
	if (apply_default_style_cb)
		apply_default_style_cb (fwe, key);
}

void
Application::ApplyStyle (FrameworkElement *fwe, Style *style)
{
	if (apply_style_cb)
		apply_style_cb (fwe, style);
}
