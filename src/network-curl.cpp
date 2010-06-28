/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * network-curl.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>
#include <errno.h>

#include "network-curl.h"
#include "pipeline.h"
#include "debug.h"

#define ds(x) 1

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

class CurlDownloaderRequest;
class CurlDownloaderResponse;

static size_t header_received (void *ptr, size_t size, size_t nmemb, void *data);
static size_t data_received (void *ptr, size_t size, size_t nmemb, void *data);
static void _open (EventObject *sender);
static gboolean _abort (void *sender);

// These 4 methods are for asynchronous operation
static void _started (CallData *sender);
static void _visitor (CallData *sender);
static void _available (CallData *sender);
//static void _finished (CallData *sender);


// Debugging methods that dump all data sent through
// curl
#if !(ds(0))

struct data {
	char trace_ascii; /* 1 or 0 */
};

static struct data config;

static void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex)
{
	size_t i;
	size_t c;

	unsigned int width=0x10;

	if(nohex) /* without the hex output, we can fit more on screen */
		width = 0x40;

	fprintf(stream, "%s, %010.10ld bytes (0x%08.8lx)\n", text, (long)size, (long)size);

	for (i=0; i<size; i+= width) {

		fprintf(stream, "%04.4lx: ", (long)i);

		if(!nohex) {
			/* hex not disabled, show it */
			for (c = 0; c < width; c++)
				if (i+c < size)
					fprintf (stream, "%02x ", ptr[i+c]);
				else
					fputs ("   ", stream);
		}

		for (c = 0; (c < width) && (i+c < size); c++) {
			/* check for 0D0A; if found, skip past and start a new line of output */
			if (nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
				i += (c+2-width);
				break;
			}

			fprintf (stream, "%c", (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');

			/* check again for 0D0A, to avoid an extra \n if it's at width */
			if (nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
				i += (c+3-width);
				break;
			}
		}
		fputc ('\n', stream); /* newline */
	}
	fflush (stream);
}

static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
	struct data *config = (struct data *)userp;
	const char *text;
	(void)handle; /* prevent compiler warning */

	switch (type) {
		case CURLINFO_TEXT:
			fprintf(stderr, "== Info: %s", data);
		default: /* in case a new one is introduced to shock us */
		return 0;

		case CURLINFO_HEADER_OUT:
			text = "=> Send header";
//			dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
//			text = "";
		break;
		case CURLINFO_DATA_OUT:
			text = "=> Send data";
//			dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
//			text = "";
		break;
		case CURLINFO_SSL_DATA_OUT:
			text = "=> Send SSL data";
//			dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
//			text = "";
		break;
		case CURLINFO_HEADER_IN:
			text = "<= Recv header";
			dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
			text = "";
		break;
		case CURLINFO_DATA_IN:
			text = "<= Recv data";
		break;
		case CURLINFO_SSL_DATA_IN:
			text = "<= Recv SSL data";
		break;
	}

	fprintf (stderr, text);
	fprintf(stderr, "\n");
	fflush (stderr);
//	dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);

	return 0;
}

#endif

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

static size_t
header_received (void *ptr, size_t size, size_t nmemb, void *data)
{
	CurlDownloaderResponse* response = (CurlDownloaderResponse*) data;
	response->HeaderReceived (ptr, size*nmemb);
	return size*nmemb;
}

static size_t
data_received (void *ptr, size_t size, size_t nmemb, void *data)
{
	CurlDownloaderResponse* response = (CurlDownloaderResponse*) data;
	return response->DataReceived (ptr, size*nmemb);
}

static void
_open (EventObject *sender)
{
	((ResponseClosure*)sender)->res->Open ();
}

static gboolean
_abort (void *req)
{
	((CurlDownloaderRequest*)req)->Abort ();
	return FALSE;
}

// These 4 methods are for asynchronous operation

static void
_started (CallData *sender)
{
	CurlDownloaderResponse* res = (CurlDownloaderResponse*) ((CallData*)sender)->res;
	res->Started ();
}

static void
_visitor (CallData *sender)
{
	CallData* data = ((CallData*)sender);
	CurlDownloaderResponse* res = (CurlDownloaderResponse*) ((CallData*)sender)->res;
	res->Visitor (data->name, data->val);
}

static void
_available (CallData *sender)
{
	CallData* data = ((CallData*)sender);
	CurlDownloaderResponse* res = (CurlDownloaderResponse*) ((CallData*)sender)->res;
	res->Available (data->buffer, data->size);
}

#if 0
static void
_finished (CallData *sender)
{
	CurlDownloaderResponse* res = (CurlDownloaderResponse*) ((CallData*)sender)->res;
	res->Finished ();
}
#endif

CurlDownloaderRequest::CurlDownloaderRequest (CurlHttpHandler *handler, HttpRequest::Options options)
	: HttpRequest (Type::CURLDOWNLOADERREQUEST, handler, options), headers(NULL), response(NULL),
	  bridge(handler), post(NULL), postlast(NULL), body(NULL), state(NONE), aborting(FALSE)
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::CurlDownloaderRequest %p\n", this);

	VERIFY_MAIN_THREAD

	curl = bridge->RequestHandle ();

#if !(ds(0))
	config.trace_ascii = 1;
	curl_easy_setopt (curl, CURLOPT_DEBUGFUNCTION, my_trace);
	curl_easy_setopt (curl, CURLOPT_DEBUGDATA, &config);
	curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
#endif

	// only validate host/ip, without using local certificates
	// TODO: test with nss stores and activate these checks
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl,CURLOPT_CAINFO, NULL);
	curl_easy_setopt(curl,CURLOPT_CAPATH, NULL);

	curl_easy_setopt (curl, CURLOPT_USERAGENT, "NSPlayer/11.08.0005.0000");
	curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
}

void CurlDownloaderRequest::Started ()
{
	HttpRequest::Started (response);
}

void
CurlDownloaderRequest::Succeeded ()
{
	HttpRequest::Succeeded ();
}

void CurlDownloaderRequest::SetHeaderImpl (const char *name, const char *value, bool disable_folding)
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::SetHttpHeader %p - %s:%s\n", this, name, value);
	char *header = g_strdup_printf ("%s: %s", name, value);
	headers = curl_slist_append(headers, header);
}

void CurlDownloaderRequest::SetBodyImpl (void *ptr, guint32 size)
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::SetBody %p\n", this);

	body = (void *) g_malloc (size);
	memcpy(body, ptr, size);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, size);
}

void CurlDownloaderRequest::SendImpl ()
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Send %p\n", this);

	if (IsAborted ())
		return;

	VERIFY_MAIN_THREAD

	state = OPENED;

	if (isPost ())
		curl_easy_setopt(curl, CURLOPT_POST, 1);

	// we're ready to start the connection, set the headers
	response = new CurlDownloaderResponse (bridge, this);
	curl_easy_setopt (curl, CURLOPT_URL, GetUri ());
	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, data_received);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, response);
	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, header_received);
	curl_easy_setopt (curl, CURLOPT_WRITEHEADER, response);

	response->Open ();
}

CurlDownloaderRequest::~CurlDownloaderRequest ()
{
}

CurlDownloaderResponse::CurlDownloaderResponse (CurlHttpHandler *bridge,
	CurlDownloaderRequest *request)
	: HttpResponse (Type::CURLDOWNLOADERRESPONSE, request),
	  bridge(bridge), request(request),
	  status(0), statusText(NULL),
	  delay(2), state(STOPPED), aborted (false)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::CurlDownloaderResponse %p\n", this);

	closure = new ResponseClosure (this);
}

CurlDownloaderResponse::~CurlDownloaderResponse ()
{
	g_free (statusText);
}

void
CurlDownloaderResponse::Open ()
{
	VERIFY_MAIN_THREAD

	if (IsAborted ())
		return;

	if (delay) {
		delay--;
		GetDeployment ()->GetSurface()->GetTimeManager()->AddDispatcherCall (_open, closure);
		return;
	}
	bridge->OpenHandle (request, request->GetHandle ());
}

void
CurlDownloaderRequest::AbortImpl () {
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Abort request:%p response:%p\n", this, response);

	if (bridge->IsDataThread ()) {
		aborting = TRUE;
		bridge->CloseHandle (this, GetHandle ());
		g_idle_add (_abort, this);
	} else {
		if (state != OPENED)
			return;

		state = ABORTED;
		Close ();
	}
}

void
CurlDownloaderRequest::Close ()
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Close request:%p response:%p\n", this, response);

	VERIFY_MAIN_THREAD

	if (state != OPENED && state != ABORTED)
		return;

	if (state != ABORTED)
		state = CLOSED;

	if (response) {
		if (IsAborted ())
			response->Abort ();
		else
			response->Close ();
		response = NULL;
	}

	bridge->ReleaseHandle (curl);

	if (body)
		g_free (body);

	if (headers)
		curl_slist_free_all (headers);

}

void
CurlDownloaderRequest::Write (gint64 offset, void *buffer, gint32 length)
{
	HttpRequest::Write (offset, buffer, length);
}

void
CurlDownloaderResponse::Abort ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Abort request:%p response:%p\n", request, this);

	VERIFY_MAIN_THREAD

	if (IsAborted ()) {
		closure = NULL;
	}

	aborted = true;
	Close ();
}

void
CurlDownloaderResponse::Close ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Close request:%p response:%p\n", request, this);

	VERIFY_MAIN_THREAD

	curl_easy_setopt(request->GetHandle (), CURLOPT_HTTPHEADER, NULL);
	curl_easy_setopt(request->GetHandle (), CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(request->GetHandle (), CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(request->GetHandle (), CURLOPT_HEADERFUNCTION, NULL);

	bridge->CloseHandle (request, request->GetHandle ());

	if (closure) {
		GetDeployment ()->GetSurface()->GetTimeManager()->RemoveTickCall (_open, closure);
		closure = NULL;
	}
	state = DONE;

	Finished ();
}

int CurlDownloaderResponse::GetResponseStatus ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::GetResponseStatus %p\n", this);
	return status;
}

const char * CurlDownloaderResponse::GetResponseStatusText ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::GetResponseStatusText %p\n", this);
	return 0;
}

void
CurlDownloaderResponse::HeaderReceived (void *ptr, size_t size)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::HeaderReceived %p\n%s", this, (char *) ptr);

	if (IsAborted () || request->aborting)
		return;

	if (!ptr || size <= 2)
		return;

	if (state == STOPPED) {
		curl_easy_getinfo (request->GetHandle (), CURLINFO_RESPONSE_CODE, &status);
		statusText = g_strndup ((char*)ptr, size-2);
		if (status == 200) {
			state = STARTED;
			bridge->AddCallback (_started, this, NULL, NULL, NULL, NULL);
		} else if (status > 302) {
			request->AbortImpl ();
		}
		return;
	}

	if (size <= 2)
		return;

	gchar *name, *val;
	char **header = g_strsplit ((char*)ptr, ":", 2);

	if (!header[1])
		return;

	name = g_strdup (header[0]);
	val = g_strdup (header[1]);
	val = g_strstrip (val);

	bridge->AddCallback (_visitor, this, NULL, NULL, name, val);
}

size_t
CurlDownloaderResponse::DataReceived (void *ptr, size_t size)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::DataReceived %p\n", this);

	if (request->aborting)
		return -1;

	if (state == STOPPED || state == DONE)
		return size;
	state = DATA;

	if (IsAborted ())
		return -1;

	char *buffer = (char *) g_malloc (size);
	memcpy(buffer, ptr, size);

	bridge->AddCallback (_available, this, buffer, size, NULL, NULL);
	return aborted ? -1 : size;
}

void
CurlDownloaderResponse::Started ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Started %p\n", this);

	SetCurrentDeployment ();
	state = HEADER;
	SetStatus (status, statusText);
	request->Started ();
	if (state == FINISHED)
		Finished ();
}

void
CurlDownloaderResponse::Visitor (const char *name, const char *val)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Visitor %p name: %s val: %s\n", this, name, val);
	SetCurrentDeployment ();
	AppendHeader (name, val);
}

void
CurlDownloaderResponse::Available (char* buffer, size_t size)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Available %p\n", this);
	SetCurrentDeployment ();
	request->Write (-1, buffer, size);
}

void
CurlDownloaderResponse::Finished ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Finished %p\n", this);

	SetCurrentDeployment ();
	if (state == STARTED) {
		state = FINISHED;
		return;
	}
	if ((int) state > FINISHED && !IsAborted ())
		request->Succeeded ();
}

static pthread_t worker_thread;
static pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t worker_cond = PTHREAD_COND_INITIALIZER;
static gboolean Emit (void* data);

class CurlNode : public List::Node {
public:
	CURL* handle;
	CurlNode (CURL* handle) : Node (), handle(handle) {};
};

class HandleNode : public List::Node {
public:
	CurlDownloaderRequest* res;
	HandleNode (CurlDownloaderRequest* res) : Node (), res(res) {
		if (res)
			res->ref ();
	};
	virtual ~HandleNode ()
	{
		if (res)
			res->unref ();
	}
	CURL* GetHandle () { return res->GetHandle (); }
	void Close () { return res->Close (); }
};

static void*
getdata_callback (void* sender)
{
	((CurlHttpHandler*)sender)->GetData ();

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
	HttpRequest* handle = (HttpRequest*)data;
	return (rn->res == handle);
}

CurlHttpHandler::CurlHttpHandler () :
	HttpHandler (Type::CURLHTTPHANDLER),
	sharecurl(NULL),
	multicurl(NULL),
	running(0),
	quit(false),
	shutting_down(false),
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

HttpRequest *
CurlHttpHandler::CreateRequest (HttpRequest::Options options)
{
	if (!closure) {
		closure = new Closure (this);
		pthread_create (&worker_thread, NULL, getdata_callback, (void*)this);
	}

	return new CurlDownloaderRequest (this, options);
}

void
CurlHttpHandler::Dispose ()
{
	shutting_down = true;
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

	HttpHandler::Dispose ();
}

CurlHttpHandler::~CurlHttpHandler ()
{
	LOG_CURL("BRIDGE ~CurlHttpHandler\n");

	delete handles;
	handles = NULL;
	delete pool;
	pool = NULL;
}

CURL*
CurlHttpHandler::RequestHandle ()
{
	LOG_CURL ("BRIDGE CurlHttpHandler::RequestHandle pool is %s\n", pool->IsEmpty () ? "empty" : "not empty");

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
	LOG_CURL ("\t%p\n", handle);

	return handle;
}

void
CurlHttpHandler::ReleaseHandle (CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::ReleaseHandle handle:%p\n", handle);

	curl_easy_reset (handle);
	pool->Push (new CurlNode (handle));
}

void
CurlHttpHandler::OpenHandle (CurlDownloaderRequest* res, CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::OpenHandle res:%p handle:%p\n", res, handle);

	pthread_mutex_lock (&worker_mutex);
	if (!quit) {
		handles->Push (new HandleNode (res));
		curl_multi_add_handle (multicurl, handle);
		pthread_cond_signal (&worker_cond);
	}
	pthread_mutex_unlock (&worker_mutex);
}

void
CurlHttpHandler::CloseHandle (CurlDownloaderRequest* res, CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::CloseHandle res:%p handle:%p\n", res, handle);

	pthread_mutex_lock (&worker_mutex);
	if (!quit) {
		handles->Lock ();
		List* list = handles->LinkedList ();
		HandleNode* node = (HandleNode*)list->Find (find_handle, res);
		if (node) {
			curl_multi_remove_handle (multicurl, handle);
			list->Remove (node);
		}
		handles->Unlock ();
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
CurlHttpHandler::GetData ()
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
			g_idle_add (::Emit, tmp);
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
CurlHttpHandler::AddCallback (CallHandler func, HttpResponse *res, char *buffer, size_t size, const char* name, const char* val)
{
	VERIFY_CURL_THREAD

	CallData* data = new CallData (this, func, res, buffer, size, name, val);
	calls = g_list_append (calls, data);
}


bool
CurlHttpHandler::IsDataThread ()
{
	return pthread_equal (pthread_self (), worker_thread);
}

CallData::CallData (CurlHttpHandler *bridge, CallHandler func, CurlDownloaderRequest *req)
	: bridge(bridge), func(func), res(NULL), req(req), buffer(NULL), size(0), name(NULL), val(NULL)
{
	if (bridge)
		bridge->ref ();
	if (req)
		req->ref ();
}

CallData::CallData (CurlHttpHandler *bridge, CallHandler func, HttpResponse *res, char *buffer, size_t size, const char* name, const char* val)
	: bridge(bridge), func(func), res (res), req(NULL), buffer(buffer), size(size), name(name), val(val)
{
	if (bridge)
		bridge->ref ();
	if (res)
		res->ref ();
	req = NULL;
}

CallData::~CallData ()
{
	if (buffer)
		g_free (buffer);
	if (name)
		g_free ((gpointer) name);
	if (val)
		g_free ((gpointer) val);
	if (req)
		req->unref ();
	if (res)
		res->unref ();
	if (bridge)
		bridge->unref ();
}
