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

Application* Application::_current = NULL;

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
	return _current;
}

void
Application::SetCurrent (Application *application)
{
	_current = application;
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
