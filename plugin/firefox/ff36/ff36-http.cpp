/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff36-http.cpp: Firefox 3.6.x bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#define NO_NSPR_10_SUPPORT

#include "plugin.h"

#include "ff36-bridge.h"

#define CONCAT(x,y) x##y
#define GECKO_SYM(x) CONCAT(FF36,x)
#include "../browser-http.inc"

DownloaderRequest*
FF36BrowserBridge::CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache)
{
	return new FF36DownloaderRequest (method, uri, disable_cache);
}