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
#include <nsIOutputStream.h>
#include <nsIStreamListener.h>
#include <nsEmbedString.h>
#include <nsIChannel.h>
#include <nsIRequest.h>
#include <nsIRequestObserver.h>
#include <nsIHttpChannel.h>
#include <nsIHttpHeaderVisitor.h>
#include <nsEmbedString.h>
#include <nsIUploadChannel.h>

// unfrozen apis
#include <necko/nsNetError.h>
#include <xpcom/nsIStorageStream.h>

typedef void (* HttpHeaderHandler) (const char *name, const char *value);

class BrowserHttpResponse : public nsIHttpHeaderVisitor {
private:
	nsCOMPtr<nsIChannel> channel;
	HttpHeaderHandler handler;
protected:
	NS_DECL_NSIHTTPHEADERVISITOR
public:
	NS_DECL_ISUPPORTS

	BrowserHttpResponse (nsCOMPtr<nsIChannel> channel) : handler (NULL)
	{
		this->channel = channel;
	}

	virtual ~BrowserHttpResponse ()
	{
	}

	void VisitHeaders (HttpHeaderHandler handler);

	virtual void *Read (int *length) = 0;
};

class SyncBrowserHttpResponse : public BrowserHttpResponse {
private:
	nsCOMPtr<nsIInputStream> response_stream;
public:
	SyncBrowserHttpResponse (nsCOMPtr<nsIChannel> channel, nsCOMPtr<nsIInputStream> response) : BrowserHttpResponse (channel)
	{
		this->response_stream = response;
	}

	virtual ~SyncBrowserHttpResponse ()
	{
		if (response_stream)
			response_stream->Close ();
	}

	virtual void *Read (int *length);
};

class AsyncBrowserHttpResponse;

typedef void (* AsyncResponseAvailableHandler) (BrowserHttpResponse *response, gpointer context);

class AsyncBrowserHttpResponse : public BrowserHttpResponse, public nsIStreamListener {
private:
	AsyncResponseAvailableHandler handler;
	gpointer context;
	char *buffer;
	int size;
protected:
	NS_DECL_NSIREQUESTOBSERVER
	NS_DECL_NSISTREAMLISTENER
public:
	NS_DECL_ISUPPORTS

	AsyncBrowserHttpResponse (nsCOMPtr<nsIChannel> channel, AsyncResponseAvailableHandler handler, gpointer context)
		: BrowserHttpResponse (channel), buffer (NULL), size (0)
	{
		this->handler = handler;
		this->context = context;
	}

	virtual ~AsyncBrowserHttpResponse ()
	{
	}

	virtual void *Read (int *size);
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
		g_free ((gpointer) uri);
		g_free ((gpointer) method);
	}

	void Abort ();
	BrowserHttpResponse *GetResponse ();
	void GetAsyncResponse (AsyncResponseAvailableHandler handler, gpointer context);
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (const char *body, int size);
};

G_BEGIN_DECLS;

BrowserHttpRequest *browser_http_request_new (const char *method, const char *uri);
void browser_http_request_destroy (BrowserHttpRequest *request);
void browser_http_request_abort (BrowserHttpRequest *request);
void browser_http_request_set_header (BrowserHttpRequest *request, const char *name, const char *value);
void browser_http_request_set_body (BrowserHttpRequest *request, const char *body, int size);
BrowserHttpResponse *browser_http_request_get_response (BrowserHttpRequest *request);
void browser_http_request_get_async_response (BrowserHttpRequest *request, AsyncResponseAvailableHandler handler, gpointer context);
void *browser_http_response_read (BrowserHttpResponse *response, int *size);
void browser_http_response_visit_headers (BrowserHttpResponse *response, HttpHeaderHandler handler);
void browser_http_response_destroy (BrowserHttpResponse *response);

void browser_http_test ();

G_END_DECLS;
