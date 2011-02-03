/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * curl-http.h: Downloader class.
 *
 * Contact:
 *   Moonlight List (moonligt-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __CURL_HTTP_H__
#define __CURL_HTTP_H__

class ResponseClosure;
class CurlDownloaderRequest;
class CurlDownloaderResponse;

#include <glib.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "moonbuild.h"
#include "timemanager.h"
#include "ptr.h"

class CurlDownloaderRequest : public DownloaderRequest {
 private:
	struct curl_slist *headers;
	CurlDownloaderResponse *response;
	CurlBrowserBridge *bridge;
	struct curl_httppost *post;
	struct curl_httppost *postlast;
	void *body;
	CURL* curl;

	enum State {
		NONE = 0,
		OPENED = 1,
		CLOSED = 3,
		ABORTED = 4,
	};
	State state;

 public:
	bool aborting;

	CurlDownloaderRequest (CurlBrowserBridge *bridge, const char *method, const char *uri, bool disable_cache);
	~CurlDownloaderRequest ();
	void Abort ();
	const bool IsAborted () {
		if (state != ABORTED && bridge->IsShuttingDown ())
			state = ABORTED;
		return state == ABORTED;
	}
	bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context);
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (void *ptr, int size);

	bool isPost () { return strstr (method, "POST"); }
	void Close ();
	CURL* GetHandle () { return curl; }
};

class CurlDownloaderResponse : public DownloaderResponse {
 private:
	CurlBrowserBridge *bridge;
	CurlDownloaderRequest *request;
	DownloaderResponseHeaderCallback visitor;
	gpointer vcontext;

	long status;
	const char* statusText;
	int delay;
	DOPtr<ResponseClosure> closure;

	enum State {
		STOPPED = 0,
		STARTED = 1,
		FINISHED = 2,
		HEADER = 3,
		DATA = 4,
		DONE = 5,
	};
	State state;

	const bool IsAborted () {
		aborted = aborted || bridge->IsShuttingDown ();
		return aborted;
	}

 public:
	CurlDownloaderResponse (CurlBrowserBridge *bridge,
	    CurlDownloaderRequest *request,
	    DownloaderResponseStartedHandler started,
	    DownloaderResponseDataAvailableHandler available,
	    DownloaderResponseFinishedHandler finished,
	    gpointer context);

	~CurlDownloaderResponse ();

	DownloaderRequest *GetDownloaderRequest () { return request; }
	void SetDownloaderRequest (DownloaderRequest *value) { request = (CurlDownloaderRequest*)value; }

	void Abort ();
	void SetHeaderVisitor (DownloaderResponseHeaderCallback visitor, gpointer context);
	int GetResponseStatus ();
	const char * GetResponseStatusText ();
	void ref ();
	void unref ();

	void Open ();
	void HeaderReceived (void *ptr, size_t size);
	size_t DataReceived (void *ptr, size_t size);

	void Started ();
	void Available (char* buffer, size_t size);
	void Finished ();
	void Visitor (const char *name, const char *val);
	void Close ();
	CURL* GetHandle () { return request->GetHandle (); }
};

class ResponseClosure : public EventObject {
public:
	ResponseClosure (CurlDownloaderResponse *value)
		: res (value)
	{
	}

	virtual ~ResponseClosure () {}

	CurlDownloaderResponse *res;
};

#endif
