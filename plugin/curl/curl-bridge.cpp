/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * curl-bridge.cpp: Curl bridge
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

#include "curl-bridge.h"

CurlBrowserBridge::CurlBrowserBridge ()
{
	curl_global_init(CURL_GLOBAL_ALL);
	sharecurl = curl_share_init();
	curl_share_setopt (sharecurl, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
}

CurlBrowserBridge::~CurlBrowserBridge ()
{
	curl_share_cleanup(sharecurl);
	curl_global_cleanup ();
}

BrowserBridge* CreateBrowserBridge ()
{
	return new CurlBrowserBridge ();
}
