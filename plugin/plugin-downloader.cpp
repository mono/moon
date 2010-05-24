/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-downloader.cpp: Moonlight plugin download routines.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "plugin-downloader.h"
#include "browser-bridge.h"
#include "npstream-request.h"

#define d(x)

/*
 * BrowserHttpHandler
 */

BrowserHttpHandler::BrowserHttpHandler (PluginInstance *instance)
	: HttpHandler (Type::BROWSERHTTPHANDLER)
{
	this->instance = instance;
	this->instance->ref ();
}

BrowserHttpHandler::~BrowserHttpHandler ()
{
	instance->unref ();
}

HttpRequest *
BrowserHttpHandler::CreateRequest (HttpRequest::Options options)
{
	BrowserBridge *bridge;

	if ((options & HttpRequest::CustomHeaders) != 0) {
		bridge = instance->GetBridge ();
		if (bridge != NULL) {
			return bridge->CreateRequest (this, options);
		} else {
			/* If we need custom headers and don't have a bridge, we can't satisfy the request */
			return NULL;
		}
	}

	return new NPStreamRequest (this, options);
}

/*
 * BrowserHttpRequest
 */

BrowserHttpRequest::BrowserHttpRequest (BrowserHttpHandler *handler, HttpRequest::Options options)
	: HttpRequest (Type::BROWSERHTTPREQUEST, handler, options)
{
	browser_handler = handler;
	browser_handler->ref ();
}

BrowserHttpRequest::~BrowserHttpRequest ()
{
	browser_handler->unref ();
	browser_handler = NULL;
}

/*
 * BrowserHttpResponse
 */

BrowserHttpResponse::BrowserHttpResponse (HttpRequest *request)
	: HttpResponse (Type::BROWSERHTTPRESPONSE, request)
{
}
