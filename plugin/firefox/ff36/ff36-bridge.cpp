/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff36-bridge.cpp: Firefox 3.6.x bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "plugin.h"

#include "ff36-bridge.h"

FF36BrowserBridge::FF36BrowserBridge ()
{
}

BrowserBridge* CreateBrowserBridge ()
{
	return new FF36BrowserBridge ();
}
