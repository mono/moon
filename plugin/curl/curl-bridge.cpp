/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * curl-bridge.cpp: Curl bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "plugin.h"

#include "curl-bridge.h"
#include "curl-http.h"
#include <errno.h>

#define d(x)

static pthread_t worker_thread;
static pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t worker_cond = PTHREAD_COND_INITIALIZER;
static gboolean Emit (void* data);


#ifdef SANITY
#define VERIFY_CURL_THREAD 										\
	if (!pthread_equal (pthread_self (), worker_thread)) {						\
		printf ("%s: this method should only be called on the curl thread.\n", __func__);	\
	}
#else
#define VERIFY_CURL_THREAD
#endif

#if TIMERS
#define STARTCALLTIMER(id,str) \
	struct timeval id##_t_start; \
	gettimeofday(&id##_t_start, NULL); \
	printf ("timing of '%s' started at %" G_GINT64_FORMAT "\n", str, id##_t_start.tv_usec)

#define ENDCALLTIMER(id,str) \
	struct timeval id##_t_end; \
	gettimeofday(&id##_t_end, NULL); \
	printf ("timing of '%s' ended at %" G_GINT64_FORMAT " (%f seconds)\n", str, id##_t_end.tv_usec, (double)(id##_t_end.tv_usec - id##_t_start.tv_usec) / 10000000)
#else
#define STARTCALLTIMER(id,str)
#define ENDCALLTIMER(id,str)
#endif

class CurlNode : public List::Node {
public:
	CURL* handle;
	CurlNode (CURL* handle) : Node (), handle(handle) {};
};

class HandleNode : public List::Node {
public:
	DownloaderRequest* res;
	HandleNode (DownloaderRequest* res) : Node (), res(res) {};
	CURL* GetHandle () { return ((CurlDownloaderRequest*)res)->GetHandle (); }
	void Close () { return ((CurlDownloaderRequest*)res)->Close (); }
};

static void*
getdata_callback (void* sender)
{
	((CurlBrowserBridge*)sender)->GetData ();

	return NULL;
}

bool
find_easy_handle (List::Node *node, void *data)
{
	HandleNode* rn = (HandleNode*)node;
	CURL* handle = (CURL*)data;
	return (rn->GetHandle () == handle);
}

bool
find_handle (List::Node *node, void *data)
{
	HandleNode* rn = (HandleNode*)node;
	DownloaderRequest* handle = (DownloaderRequest*)data;
	return (rn->res == handle);
}

BrowserBridge* CreateBrowserBridge ()
{
	return new CurlBrowserBridge ();
}

CurlBrowserBridge::CurlBrowserBridge () :
	sharecurl(NULL),
	multicurl(NULL),
	running(0),
	quit(false),
	pool(NULL),
	handles(NULL),
	calls(NULL)
{
	pool = new Queue ();
	handles = new Queue ();

	curl_global_init(CURL_GLOBAL_ALL);
	sharecurl = curl_share_init();
	multicurl = curl_multi_init ();
	curl_share_setopt (sharecurl, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
}

void
CurlBrowserBridge::Shutdown ()
{
	BrowserBridge::Shutdown ();

	if (closure) {
		pthread_mutex_lock (&worker_mutex);
		quit = true;

		if (calls)
			g_list_free (calls);
		calls = NULL;

		pthread_cond_signal (&worker_cond);
		pthread_mutex_unlock (&worker_mutex);

		pthread_join (worker_thread, NULL);
		closure = NULL;
	}

	curl_share_cleanup(sharecurl);
	CurlNode* node = NULL;
	while ((node = (CurlNode*)pool->Pop ())) {
		curl_easy_cleanup (node->handle);
		delete node;
	}

	curl_multi_cleanup(multicurl);
	curl_global_cleanup ();
}

CurlBrowserBridge::~CurlBrowserBridge ()
{
	d(printf("BRIDGE ~CurlBrowserBridge\n"));

	delete handles;
	handles = NULL;
	delete pool;
	pool = NULL;
}

DownloaderRequest*
CurlBrowserBridge::CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache)
{
	if (!closure) {
		closure = new Closure (this);
		pthread_create (&worker_thread, NULL, getdata_callback, (void*)this);
	}
	CurlDownloaderRequest* req = new CurlDownloaderRequest (this, method, uri, disable_cache);

	d(printf ("BRIDGE CurlBrowserBridge::CreateDownloaderRequest %p\n", req));

	return req;
}

CURL*
CurlBrowserBridge::RequestHandle ()
{
	d(printf ("BRIDGE CurlBrowserBridge::RequestHandle pool is %s\n", pool->IsEmpty () ? "empty" : "not empty"));

	CURL* handle;
	if (!pool->IsEmpty ()) {
		CurlNode* node = (CurlNode*) pool->Pop ();
		handle = node->handle;
		delete node;
	}
	else {
		handle = curl_easy_init ();
		curl_easy_setopt (handle, CURLOPT_SHARE, sharecurl);
	}
	d(printf ("\t%p\n", handle));

	return handle;
}

void
CurlBrowserBridge::ReleaseHandle (CURL* handle)
{
	d(printf ("BRIDGE CurlBrowserBridge::ReleaseHandle handle:%p\n", handle));

	if (!handle)
		return;
	curl_easy_reset (handle);
	if (!pool)
		quit = true;
	else
		pool->Push (new CurlNode (handle));
}

void
CurlBrowserBridge::OpenHandle (DownloaderRequest* res, CURL* handle)
{
	d(printf ("BRIDGE CurlBrowserBridge::OpenHandle res:%p handle:%p\n", res, handle));

	pthread_mutex_lock (&worker_mutex);
	if (!quit) {
		handles->Push (new HandleNode (res));
		curl_multi_add_handle (multicurl, handle);
		pthread_cond_signal (&worker_cond);
	}
	pthread_mutex_unlock (&worker_mutex);
}

void
CurlBrowserBridge::CloseHandle (DownloaderRequest* res, CURL* handle)
{
	d(printf ("BRIDGE CurlBrowserBridge::CloseHandle res:%p handle:%p\n", res, handle));

	VERIFY_MAIN_THREAD

	pthread_mutex_lock (&worker_mutex);
	if (!quit) {
		if (!handles)
			quit = true;
		else {
			handles->Lock ();
			List* list = handles->LinkedList ();
			HandleNode* node = (HandleNode*)list->Find (find_handle, res);
			if (node) {
				curl_multi_remove_handle (multicurl, handle);
				list->Remove (node);
			}
			handles->Unlock ();
		}
	}
	pthread_mutex_unlock (&worker_mutex);
}

static void
_close (CallData *sender)
{
	VERIFY_MAIN_THREAD

	CurlDownloaderRequest* req = (CurlDownloaderRequest*) ((CallData*)sender)->req;
	req->Close ();
}

void
CurlBrowserBridge::GetData ()
{
	VERIFY_CURL_THREAD

	fd_set r,w,x;
	int num, available;
	long timeout;
	struct timespec tv;

	do {
		if (handles->IsEmpty ()) {
			pthread_mutex_lock (&worker_mutex);
			if (!quit) pthread_cond_wait (&worker_cond, &worker_mutex);
			pthread_mutex_unlock (&worker_mutex);
			if (quit) break;
		}

		pthread_mutex_lock (&worker_mutex);
		if (!quit) while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform (multicurl, &num));
		pthread_mutex_unlock (&worker_mutex);
		if (quit) break;

		if (num != running) {
			running = num;
			int msgs;
			CURLMsg* msg;
			while ((msg = curl_multi_info_read (multicurl, &msgs))) {
				if (msg->msg == CURLMSG_DONE) {

					handles->Lock ();
					List* list = handles->LinkedList ();
					HandleNode* node = (HandleNode*) list->Find (find_easy_handle, msg->easy_handle);
					handles->Unlock ();
					if (node) {
						CallData* data = new CallData (this, _close, node->res);
						calls = g_list_append (calls, data);
					}
				}
			}
		}


		if (calls) {
			GList* tmp = g_list_copy (calls);
			g_list_free (calls);
			calls = NULL;
			g_idle_add (Emit, tmp);
		}

		if (running > 0) {
			FD_ZERO(&r);
			FD_ZERO(&w);
			FD_ZERO(&x);

			if (curl_multi_fdset (multicurl, &r, &w, &x, &available)) {
				fprintf(stderr, "E: curl_multi_fdset\n");
				return;
			}

			if (curl_multi_timeout (multicurl, &timeout)) {
				fprintf(stderr, "E: curl_multi_timeout\n");
				return;
			}

			if (timeout > 0) {
				tv.tv_sec = timeout / 1000;
				tv.tv_nsec = (timeout % 1000) * 1000 * 1000;

				if (available == -1) {
					pthread_mutex_lock (&worker_mutex);
					if (!quit) pthread_cond_timedwait (&worker_cond, &worker_mutex, &tv);
					pthread_mutex_unlock (&worker_mutex);
				} else {
					if (pselect (available+1, &r, &w, &x, &tv, NULL) < 0) {
						fprintf(stderr, "E: select(%i,,,,%li): %i: %s\n", available+1, timeout, errno, strerror(errno));
					}
				}
			}
		} else {
			pthread_mutex_lock (&worker_mutex);
			if (!quit) pthread_cond_wait (&worker_cond, &worker_mutex);
			pthread_mutex_unlock (&worker_mutex);
		}
	} while (!quit);
}

void
CurlBrowserBridge::AddCallback (CallHandler func, DownloaderResponse *res, char *buffer, size_t size, const char* name, const char* val)
{
	VERIFY_CURL_THREAD

	CallData* data = new CallData (this, func, res, buffer, size, name, val);
	calls = g_list_append (calls, data);
}


bool
CurlBrowserBridge::IsDataThread ()
{
	return pthread_equal (pthread_self (), worker_thread);
}

static gboolean
Emit (void* data)
{
	STARTCALLTIMER (emit_call, "Emit - Call");

	CallData* call = NULL;
	GList* t;
	GList* list = (GList*)data;
	for (t = list; t; t = t->next) {
		call = (CallData*) t->data;
		if (!call->bridge->IsShuttingDown ())
			call->func (call);
		delete call;
	}
	g_list_free (list);

	ENDCALLTIMER (emit_call, "Emit - Call");

	return FALSE;
}

CallData::~CallData ()
{
	if (buffer)
		g_free (buffer);
	if (name)
		g_free ((gpointer) name);
	if (val)
		g_free ((gpointer) val);
}
