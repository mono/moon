/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff36-bridge.h: Firefox 3.6.x bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef FF3_BRIDGE
#define FF3_BRIDGE

#include "browser-bridge.h"

class FF36BrowserBridge : public BrowserBridge {
 public:
	FF36BrowserBridge ();

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache);
};

#endif // FF3_BRIDGE
