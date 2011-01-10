/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * network-curl.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifdef HAVE_CURL

#ifndef __MOON_NETWORK_CURL__
#define __MOON_NETWORK_CURL__


#include <glib.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <pthread.h>

#include "network.h"
#include "list.h"
#include "timemanager.h"
#include "ptr.h"

namespace Moonlight {

class ResponseClosure;
class CurlDownloaderRequest;
class CurlDownloaderResponse;
class CallData;
class CurlHttpHandler;

typedef void ( * CallHandler ) ( CallData * object ) ;

class Closure : public EventObject {
public:
	/* @SkipFactories */
	Closure (CurlHttpHandler *value)
		: bridge (value)
	{
	}

	virtual ~Closure () {}

	CurlHttpHandler* bridge;
};

class CurlHttpHandler : public HttpHandler {
 public:
	CURL* sharecurl;
	CURL* multicurl;
	DOPtr<Closure> closure;
	bool quit;
	bool shutting_down;
	int fds [2]; // file descriptors to select on in addition to curl's file descriptors
	pthread_t worker_thread;
	pthread_mutex_t worker_mutex;
	pthread_cond_t worker_cond;

	// available handles pool
	List pool; // multi-threaded access, needs worker_mutex locked
	List handles; // multi-threaded access, needs worker_mutex locked
	List calls; // multi-threaded access, needs worker_mutex locked
	List handle_actions; // multi-threaded access, needs worker_mutex locked

	/* @SkipFactories */
	CurlHttpHandler ();
	virtual ~CurlHttpHandler ();

	virtual HttpRequest* CreateRequest (HttpRequest::Options options);
	virtual void Dispose ();

	CURL* RequestHandle ();
	void ReleaseHandle (CURL* handle);
	void OpenHandle (CurlDownloaderRequest* res, CURL* handle);
	void CloseHandle (CurlDownloaderRequest* res, CURL* handle);

	void GetData ();
	void AddCallback (CallData *data);
	void AddCallback (CallHandler func, CurlDownloaderResponse *res, char *buffer, size_t size, const char* name, const char* val);
	bool IsDataThread ();
	bool IsShuttingDown () { return shutting_down; }
	static bool EmitCallback (gpointer http_handler);
	void Emit ();
	void WakeUp ();
	void ExecuteHandleActions ();
};

class CallData : public List::Node {
public:
	CallData (CurlHttpHandler *bridge, CallHandler func, CurlDownloaderResponse *res, char *buffer, size_t size, const char* name, const char* val);
	CallData (CurlHttpHandler *bridge, CallHandler func, CurlDownloaderRequest *req);
	virtual ~CallData ();

	CurlHttpHandler *bridge;
	CallHandler func;
	CurlDownloaderResponse *res;
	CurlDownloaderRequest *req;
	char *buffer;
	size_t size;
	const char *name;
	const char *val;
};

class CurlDownloaderRequest : public HttpRequest {
 private:
	curl_slist *headers;
	CurlDownloaderResponse *response;
	CurlHttpHandler *bridge;
	curl_httppost *post;
	curl_httppost *postlast;
	void *body;
	CURL* curl;
	bool body_set;

	enum State {
		NONE = 0,
		OPENED = 1,
		CLOSED = 3,
		ABORTED = 4,
	};
	State state;

 public:
	bool aborting;

	/* @SkipFactories */
	CurlDownloaderRequest (CurlHttpHandler *bridge, HttpRequest::Options options);
	virtual ~CurlDownloaderRequest ();
	virtual void AbortImpl ();
	const bool IsAborted () {
		if (state != ABORTED && bridge->IsShuttingDown ())
			state = ABORTED;
		return state == ABORTED;
	}
	virtual void OpenImpl () { /* Nothing to do */ }
	virtual void SendImpl ();
	virtual void SetHeaderImpl (const char *name, const char *value, bool disable_folding);
	virtual void SetBodyImpl (void *ptr, guint32 size);

	bool isPost () { return strstr (GetVerb (), "POST"); }
	void Close ();
	CURL* GetHandle () { return curl; }

	void Started ();
	void Succeeded ();

	void Write (gint64 offset, void *buffer, gint32 length);
	void NotifyFinalUri (const char *value);
};

class CurlDownloaderResponse : public HttpResponse {
 private:
	CurlHttpHandler *bridge;
	CurlDownloaderRequest *request;

	long status;
	char* statusText;
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
	bool aborted;
	bool reported_start;

 public:
	/* @SkipFactories */
	CurlDownloaderResponse (CurlHttpHandler *bridge,
	    CurlDownloaderRequest *request);

	virtual ~CurlDownloaderResponse ();

	void Abort ();
	int GetResponseStatus ();
	const char * GetResponseStatusText ();

	bool IsAborted () {
		aborted = aborted || bridge->IsShuttingDown ();
		return aborted;
	}

	void Open ();
	void HeaderReceived (void *ptr, size_t size);
	size_t DataReceived (void *ptr, size_t size);

	void Started ();
	void Available (char* buffer, size_t size);
	void Finished ();
	void Visitor (const char *name, const char *val);
	void Close ();
	CURL* GetHandle () { return request ? request->GetHandle () : NULL; }
	void ClearRequest ();
};

class ResponseClosure : public EventObject {
public:
	/* @SkipFactories */
	ResponseClosure (CurlDownloaderResponse *value)
		: res (value)
	{
		res->ref ();
	}

	virtual ~ResponseClosure ()
	{
		res->unref ();
	}
	CurlDownloaderResponse *res;
};

};
#endif /* __MOON_NETWORK_CURL__ */

#endif /* HAVE_CURL */
