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

namespace Moonlight {

class FF2BrowserBridge : public BrowserBridge {
 public:
	FF2BrowserBridge ();

	virtual BrowserHttpRequest* CreateRequest (BrowserHttpHandler *handler, HttpRequest::Options options);
};

};
#endif // FF2_BRIDGE
