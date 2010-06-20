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

#include "timemanager.h"
#include "ptr.h"

class ResponseClosure;
class CurlDownloaderResponse;

class CurlDownloaderRequest : public DownloaderRequest {
 protected:
	bool aborted;
	bool started;
	struct curl_slist *headers;
	CurlDownloaderResponse *response;
	CurlBrowserBridge *bridge;

 public:
	CURL* curl;
	CURL* multicurl;

	CurlDownloaderRequest (CurlBrowserBridge *bridge, const char *method, const char *uri, bool disable_cache);
	~CurlDownloaderRequest ();
	void Abort ();
	const bool IsAborted () { return this->aborted; }
	bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context);
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (void *body, int size);
};

class CurlDownloaderResponse : public DownloaderResponse {
 protected:
	CurlBrowserBridge *bridge;
	DownloaderResponseHeaderCallback visitor;
	gpointer vcontext;
	bool isStarted;
	long status;
	const char* statusText;
	int delay;
	CURL* curl;
	CURL* multicurl;
	DOPtr<ResponseClosure> closure;

	const bool IsAborted () {
		aborted = aborted || bridge->plugin->IsShuttingDown ();
		return aborted;
	}

 public:

	CurlDownloaderResponse () {}
	CurlDownloaderResponse (CurlBrowserBridge *bridge,
	    CurlDownloaderRequest *request,
	    DownloaderResponseStartedHandler started,
	    DownloaderResponseDataAvailableHandler available,
	    DownloaderResponseFinishedHandler finished,
	    gpointer context);

	~CurlDownloaderResponse () {}

	DownloaderRequest *GetDownloaderRequest () { return request; }
	void SetDownloaderRequest (DownloaderRequest *value) { request = (CurlDownloaderRequest*)value; }

	void Abort ();
	void SetHeaderVisitor (DownloaderResponseHeaderCallback visitor, gpointer context);
	int GetResponseStatus ();
	const char * GetResponseStatusText ();
	void ref ();
	void unref ();

	void Open ();
	void GetData ();
	void HeaderReceived (void *ptr, size_t size);
	size_t DataReceived (void *ptr, size_t size);
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
