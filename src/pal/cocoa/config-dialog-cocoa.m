/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * config-dialog-cocoa.cpp: right click dialog for cocoa
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <glib.h>

#include "config-dialog-cocoa.h"

#include "window-cocoa.h"
#include "openfile.h"
#include "pipeline.h"
#include "timemanager.h"
#include "debug-ui-cocoa.h"
#include "consent.h"

#define PLUGIN_OURNAME      "Novell Moonlight"

using namespace Moonlight;

MoonConfigDialogCocoa::MoonConfigDialogCocoa (MoonWindowCocoa *window, Surface *surface, Deployment *deployment)
  : window (window), surface (surface), deployment (deployment)
{
	g_assert_not_reached ();
}

MoonConfigDialogCocoa::~MoonConfigDialogCocoa ()
{
	g_assert_not_reached ();
}

void
MoonConfigDialogCocoa::Show ()
{
	g_assert_not_reached ();
}
