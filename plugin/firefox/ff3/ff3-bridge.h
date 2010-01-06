/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff3-bridge.h: Firefox 3.x bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef FF3_BRIDGE
#define FF3_BRIDGE

#include "browser-bridge.h"

class FF3BrowserBridge : public BrowserBridge {
 public:
	FF3BrowserBridge ();

	virtual const char *HtmlElementGetText (NPP npp, const char *element_id);
	virtual gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb, gpointer context);
	virtual void     HtmlObjectDetachEvent (NPP npp, const char *name, gpointer listener_ptr);

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache);
};

#endif // FF3_BRIDGE
