/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff2-bridge.cpp: Firefox 2.x bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "plugin.h"

#include "ff2-bridge.h"

FF2BrowserBridge::FF2BrowserBridge ()
{
}

BrowserBridge* CreateBrowserBridge ()
{
	return new FF2BrowserBridge ();
}
