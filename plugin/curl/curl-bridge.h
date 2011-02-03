/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * curl-bridge.h: curl bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */


#ifndef __CURL_BRIDGE__
#define __CURL_BRIDGE__

class CallData;
class CurlBrowserBridge;

#include "moonbuild.h"
#include "browser-bridge.h"
#include "list.h"
#include "ptr.h"
#include "timemanager.h"

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <pthread.h>


typedef void ( * CallHandler ) ( CallData * object ) ;

class Closure : public EventObject {
public:
	Closure (CurlBrowserBridge *value)
		: bridge (value)
	{
	}

	virtual ~Closure () {}

	CurlBrowserBridge* bridge;
};

class CurlBrowserBridge : public BrowserBridge {
 public:
	CURL* sharecurl;
	CURL* multicurl;
	DOPtr<Closure> closure;
	int running;
	bool quit;

	// available handles pool
	Queue* pool;
	Queue* handles;
	GList *calls;

	CurlBrowserBridge ();
	~CurlBrowserBridge ();

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache);
	virtual void Shutdown ();

	CURL* RequestHandle ();
	void ReleaseHandle (CURL* handle);
	void OpenHandle (DownloaderRequest* res, CURL* handle);
	void CloseHandle (DownloaderRequest* res, CURL* handle);

	void GetData ();
	void AddCallback (CallHandler func, DownloaderResponse *res, char *buffer, size_t size, const char* name, const char* val);
	bool IsDataThread ();
};


class CallData {
public:
	CallData (CurlBrowserBridge *bridge, CallHandler func, DownloaderResponse *res, char *buffer, size_t size, const char* name, const char* val)
		: bridge(bridge), func(func), res (res), buffer(buffer), size(size), name(name), val(val)
	{}

	CallData (CurlBrowserBridge *bridge, CallHandler func, DownloaderRequest *req) :
		bridge(bridge), func(func), req(req), buffer(NULL), size(0), name(NULL), val(NULL) {}

	~CallData ();

	CurlBrowserBridge *bridge;
	CallHandler func;
	DownloaderResponse *res;
	DownloaderRequest *req;
	char *buffer;
	size_t size;
	const char *name;
	const char *val;
};

#endif // CURL_BRIDGE
