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


#include <glib.h>
#include "timemanager.h"
#include "ptr.h"

class ResponseClosure;
class TickData;
class DataClosure;
class CurlDownloaderResponse;

class CurlDownloaderRequest : public DownloaderRequest {
 protected:
	bool aborted;
	bool started;
	struct curl_slist *headers;
	CurlDownloaderResponse *response;
	CurlBrowserBridge *bridge;
	struct curl_httppost *post;
	struct curl_httppost *postlast;
	void *body;
 public:
	CURL* curl;
	CURL* multicurl;

	CurlDownloaderRequest (CurlBrowserBridge *bridge, const char *method, const char *uri, bool disable_cache);
	~CurlDownloaderRequest ();
	void Abort ();
	const bool IsAborted () { return this->aborted; }
	bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context);
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (void *ptr, int size);

	bool isPost () { return strstr (method, "POST"); }
	void Close ();
};

class CurlDownloaderResponse : public DownloaderResponse {
 protected:
	CurlBrowserBridge *bridge;
	CurlDownloaderRequest *request;
	DownloaderResponseHeaderCallback visitor;
	gpointer vcontext;

	long status;
	const char* statusText;
	int delay;
	CURL* curl;
	CURL* multicurl;
	DOPtr<ResponseClosure> closure;
	GList *headers;
	GList *bodies;
	GList *callCache;

	enum State {
		STOPPED = 0,
		HEADER = 1,
		DATA = 2,
		DONE = 3,
	};
	State state;

	const bool IsAborted () {
		aborted = aborted || bridge->plugin->IsShuttingDown ();
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
	void GetData ();
	void HeaderReceived (void *ptr, size_t size);
	size_t DataReceived (void *ptr, size_t size);

	void Started ();
	void Available (DataClosure *dl);
	void Finished ();
	void Visitor (const char *name, const char *val);
	void AddCallback (TickData *data, bool delayed = FALSE);
	void Emit ();
	void Close ();
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

class DataClosure : public EventObject {
private :
public:
	DataClosure (CurlDownloaderResponse *res,
	    gpointer context, char *buffer, size_t size,
	    const char *name, const char *val) :
		res (res), context(context),
		buffer(buffer), size(size),
		name(name), val(val)
	{
	}

	virtual ~DataClosure ();

	CurlDownloaderResponse *res;
	gpointer context;
	char *buffer;
	size_t size;
	const char *name;
	const char *val;
};

class TickData {
public:
	enum CallType {
		HEADER = 0,
		BODY = 1,
	};

	TickData (CallType type, TickCallHandler func, CurlDownloaderResponse *res, gpointer context, char *buffer, size_t size) :
		type(type), func(func), res(res),
		context(context), buffer(buffer), size(size), name(NULL), val(NULL)

	{
	}

	TickData (CallType type, TickCallHandler func, CurlDownloaderResponse *res, const char *name, const char *val) :
		type(type), func(func), res(res),
		context(NULL), buffer(NULL), size(0), name(name), val(val)
	{
	}

	~TickData ();

	EventObject* GetClosure ()
	{
		return new DataClosure (res, context, buffer, size, name, val);
	}

	CallType type;
	TickCallHandler func;
	CurlDownloaderResponse *res;
	gpointer context;
	char *buffer;
	size_t size;
	const char *name;
	const char *val;
	bool delay;
};

#endif
