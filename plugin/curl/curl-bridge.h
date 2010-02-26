/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * curl-bridge.h: curl bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __CURL_BRIDGE__
#define __CURL_BRIDGE__

#include "browser-bridge.h"

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


class CurlBrowserBridge : public BrowserBridge {
 public:
	CurlBrowserBridge ();
	~CurlBrowserBridge ();

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache);
};

#endif // FF3_BRIDGE
