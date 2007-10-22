/*
 * browser-http.h: Moonlight plugin routines for http requests/responses.
 *
 * Author:
 *   Jb Evain (jbevain@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "moonlight.h"
#include "runtime.h"

#include <nsCOMPtr.h>
#include <nsXPCOM.h>
#include <nsIURI.h>
#include <nsIServiceManager.h>
#include <nsIComponentManager.h>
#include <nsIIOService.h>
#include <nsStringAPI.h>
#include <nsIInputStream.h>
#include <nsEmbedString.h>
#include <nsIChannel.h>
#include <nsIHttpChannel.h>
#include <nsEmbedString.h>

class BrowserHttpResponse {
private:
	nsCOMPtr<nsIChannel> channel;

public:
	BrowserHttpResponse (nsCOMPtr<nsIChannel> channel)
	{
		this->channel = channel;
	}

	virtual ~BrowserHttpResponse ()
	{
	}
};

class SyncBrowserHttpResponse : public BrowserHttpResponse {
private:
	char *data;
	PRUint32 length;

	nsCOMPtr<nsIInputStream> response_stream;

public:
	SyncBrowserHttpResponse (nsCOMPtr<nsIChannel> channel, nsCOMPtr<nsIInputStream> response) : BrowserHttpResponse (channel), data (NULL)
	{
		this->response_stream = response;
	}

	virtual ~SyncBrowserHttpResponse ();

	char *Read (int *length);
};

class AsyncBrowserHttpResponse : public BrowserHttpResponse {
public:
	AsyncBrowserHttpResponse (nsCOMPtr<nsIChannel> channel) : BrowserHttpResponse (channel)
	{
	}
};

class BrowserHttpRequest {
private:
	const char *uri;
	const char *method;

	nsCOMPtr<nsIChannel> channel;

	void CreateChannel ();
public:

	BrowserHttpRequest (const char *method, const char *uri)
	{
		this->method = g_strdup (method);
		this->uri = g_strdup (uri);

		CreateChannel ();
	}

	~BrowserHttpRequest ()
	{
	}

	SyncBrowserHttpResponse *GetResponse ();
	AsyncBrowserHttpResponse *GetAsyncResponse ();
	void SetHttpHeader (const char *name, const char *value);
};

G_BEGIN_DECLS

BrowserHttpRequest *browser_http_request_new (const char *method, const char *uri);
void browser_http_request_destroy (BrowserHttpRequest *request);
void browser_http_request_set_header (BrowserHttpRequest *request, const char *name, const char *value);
SyncBrowserHttpResponse *browser_http_request_get_response (BrowserHttpRequest *request);

void browser_http_test ();

G_END_DECLS
