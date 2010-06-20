/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * browser-bridge.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef BROWSER_BRIDGE
#define BROWSER_BRIDGE

#include "plugin-class.h"
#include "plugin-downloader.h"

#define DOWNLOADER_OK 0
#define DOWNLOADER_ERR -1

G_BEGIN_DECLS

BrowserBridge *CreateBrowserBridge ();

G_END_DECLS

class BrowserBridge {
	Surface *surface;
	bool shutting_down;

 public:
	BrowserBridge () : surface(NULL), shutting_down(0) {}
	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache) = 0;
	virtual void Shutdown () { shutting_down = true; }
	bool IsShuttingDown () { return shutting_down; }
	void SetSurface (Surface *value) { surface = value; }
	Surface *GetSurface () { return surface; }
};

#endif /* BROWSER_BRIDGE */
