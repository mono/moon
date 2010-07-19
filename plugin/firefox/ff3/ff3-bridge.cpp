/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff3-bridge.cpp: Firefox 3.x bridge
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
#include "plugin.h"

#include "ff3-bridge.h"

namespace Moonlight {

FF3BrowserBridge::FF3BrowserBridge ()
{
}

BrowserBridge* CreateBrowserBridge ()
{
	return new FF3BrowserBridge ();
}

};
