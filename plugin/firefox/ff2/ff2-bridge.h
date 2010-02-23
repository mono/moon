/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff2-bridge.h: Firefox 2.x bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef FF2_BRIDGE
#define FF2_BRIDGE

#include "browser-bridge.h"
#include "plugin-class.h"

class FF2BrowserBridge : public BrowserBridge {
 public:
	FF2BrowserBridge ();

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache);
};

struct FF2DomEventType : MoonlightObjectType {
	FF2DomEventType ();
};

extern FF2DomEventType *FF2DomEventClass;

#endif // FF2_BRIDGE

