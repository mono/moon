/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-downloader.h: Plugin downloader
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __PLUGIN_DOWNLOADER_H__
#define __PLUGIN_DOWNLOADER_H__

#include "moonlight.h"
#include "plugin.h"
#include "plugin-class.h"
#include "network.h"

/*
 * BrowserHttpHandler
 */

class BrowserHttpHandler : public HttpHandler {
private:
	PluginInstance *instance;

protected:
	virtual ~BrowserHttpHandler ();

public:
	BrowserHttpHandler (PluginInstance *instance);

	virtual HttpRequest *CreateRequest (HttpRequest::Options options);
	PluginInstance *GetInstance () { return instance; }
};

/*
 * BrowserHttpRequest
 */

class BrowserHttpRequest : public HttpRequest {
private:
	BrowserHttpHandler *browser_handler;

protected:
	virtual ~BrowserHttpRequest ();

	PluginInstance *GetInstance () { return browser_handler->GetInstance (); }
	BrowserHttpHandler *GetHandler () { return (BrowserHttpHandler *) HttpRequest::GetHandler (); }

public:
	BrowserHttpRequest (BrowserHttpHandler *handler, HttpRequest::Options options);
};

/*
 * BrowserHttpResponse
 */

class BrowserHttpResponse : public HttpResponse {
public:
	BrowserHttpResponse (HttpRequest *request);
};

#endif // __PLUGIN_DOWNLOADER_H__
