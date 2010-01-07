/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal.cpp
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include "pal.h"

void
MoonWindowingSystem::SetWindowlessCtor (MoonWindowlessCtor ctor)
{
	windowless_ctor = ctor;
}

MoonWindow *
MoonWindowingSystem::CreateWindowless (int width, int height, PluginInstance *forPlugin)
{
	// call into the plugin to create the windowless
	if (windowless_ctor) {
		MoonWindow *window = windowless_ctor (width, height, forPlugin);
		return window;
	}
	else {
		g_warning ("Windowless mode only works in the plugin");
		return NULL;
	}
}
