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
#include <unistd.h>
#include <fcntl.h>

#include "network-curl.h"
#include "pipeline.h"
#include "debug.h"

namespace Moonlight {

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
static void _abort (EventObject *sender);

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

bool
CurlHttpHandler::EmitCallback (gpointer http_handler)
{
	CurlHttpHandler *handler;

	STARTCALLTIMER (emit_call, "Emit - Call");

	handler = (CurlHttpHandler *) http_handler;
	handler->Emit ();
	handler->unref ();

	ENDCALLTIMER (emit_call, "Emit - Call");

	return false;
}

void
CurlHttpHandler::Emit ()
{
	CallData *node;
	CallData *next;
	List tmp;

	/* Clone the list and invoke the calls with the mutex unlocked */
	pthread_mutex_lock (&worker_mutex);
	node = (CallData *) calls.First ();
	while (node != NULL) {
		next = (CallData *) node->next;
		calls.Unlink (node);
		tmp.Append (node);
		node = next;
	}
	pthread_mutex_unlock (&worker_mutex);

	/* Invoke the calls */
	node = (CallData *) tmp.First ();
	while (node != NULL && !IsShuttingDown ()) {
		node->func (node);
		node = (CallData *) node->next;
	}

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
_abort (EventObject *req)
{
	((CurlDownloaderRequest*)req)->Abort ();
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

static inline const char *
get_state_name (int state)
{
	switch (state) {
	case 0: return "STOPPED";
	case 1: return "STARTED";
	case 2: return "FINISHED";
	case 3: return "HEADER";
	case 4: return "DATA";
	case 5: return "DONE";
	default: return "UNKNOWN";
	}
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
	body_set = false;

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
	body_set = true;
}

void CurlDownloaderRequest::SendImpl ()
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Send %p\n", this);

	if (IsAborted ())
		return;

	VERIFY_MAIN_THREAD

	state = OPENED;

	if (isPost ()) {
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		if (!body_set)
			curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, 0);
	}

	if (response)
		response->unref ();

	// we're ready to start the connection, set the headers
	response = new CurlDownloaderResponse (bridge, this);
	curl_easy_setopt (curl, CURLOPT_URL, GetUri ()->GetHttpRequestString ());
	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, data_received);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, response);
	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, header_received);
	curl_easy_setopt (curl, CURLOPT_WRITEHEADER, response);

	LOG_CURL ("BRIDGE CurlDownloaderRequest::Send %p handle: %p URL: %s\n", this, curl, GetUri ()->GetHttpRequestString ());
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Send %p handle: %p headers: %p\n", this, curl, headers);

	response->Open ();
}

CurlDownloaderRequest::~CurlDownloaderRequest ()
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::~CurlDownloaderRequest handle: %p\n", curl);
	if (response) {
		response->ClearRequest ();
		response->unref ();
		response = NULL;
	}

	// the body and headers variables can't be freed until curl_easy_reset is called on the curl handle
	// we ref ourselves until curl_easy_reset has been called, thereby making sure that we don't end up
	// freeing them too early.

	if (body) {
		g_free (body);
		body = NULL;
	}

	if (headers) {
		curl_slist_free_all (headers);
		headers = NULL;
	}
}

CurlDownloaderResponse::CurlDownloaderResponse (CurlHttpHandler *bridge,
	CurlDownloaderRequest *request)
	: HttpResponse (Type::CURLDOWNLOADERRESPONSE, request),
	  bridge(bridge), request(request),
	  status(0), statusText(NULL),
	  state(STOPPED), aborted (false), reported_start (false)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::CurlDownloaderResponse %p request: %p\n", this, request);

	closure = new ResponseClosure (this);
}

CurlDownloaderResponse::~CurlDownloaderResponse ()
{
	g_free (statusText);
}

void
CurlDownloaderResponse::ClearRequest ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::ClearRequest () request: %p\n", request);
	request = NULL;
}

void
CurlDownloaderResponse::Open ()
{
	VERIFY_MAIN_THREAD

	if (IsAborted ())
		return;

	if (request == NULL)
		return;

	LOG_CURL ("BRIDGE CurlDownloaderResponse::Open () request: %p handle: %p\n", request, request->GetHandle ());
	bridge->OpenHandle (request, request->GetHandle ());
}

static void
_close_handle (CallData *cd)
{
	cd->bridge->CloseHandle (cd->req, cd->req->GetHandle ());
	cd->req->AddTickCall (_abort);
}

void
CurlDownloaderRequest::AbortImpl () {
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Abort request:%p response:%p\n", this, response);

	if (bridge->IsDataThread ()) {
		aborting = TRUE;
		bridge->AddCallback (new CallData (bridge, _close_handle, this));
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
		response->ClearRequest ();
		response->unref ();
		response = NULL;
	}

	if (curl) {
		// we need to keep a ref to ourselves so that we're not destroyed until the handle has been released,
		// since we can't free the body and headers until then (and they're freed in the dtor).
		bridge->ReleaseHandle (this, curl);
		curl = NULL;
	} else {
#if SANITY
		printf ("CurlDownloaderRequest::Close () was called with no curl handle (state: %i)\n", state);
#endif
	}
}

void
CurlDownloaderRequest::Write (gint64 offset, void *buffer, gint32 length)
{
	HttpRequest::Write (offset, buffer, length);
}

void
CurlDownloaderRequest::NotifyFinalUri (const char *value)
{
	HttpRequest::NotifyFinalUri (value);
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

	bridge->CloseHandle (request, request->GetHandle ());

	if (closure) {
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
	char *final_url = NULL;

	LOG_CURL ("BRIDGE CurlDownloaderResponse::HeaderReceived %p\n%s", this, (char *) ptr);

	if (IsAborted () || request == NULL || request->aborting)
		return;

	if (!ptr || size <= 2)
		return;

	if (state == STOPPED) {
		curl_easy_getinfo (request->GetHandle (), CURLINFO_RESPONSE_CODE, &status);
		curl_easy_getinfo (request->GetHandle (), CURLINFO_EFFECTIVE_URL, &final_url);

		request->NotifyFinalUri (final_url);

		// The first line
		// Parse status line: "HTTP/1.X # <status>"
		// Split on the space character and select the third entry
		char *statusLine = g_strndup ((char *) ptr, size); /* Null terminate string */
		char **strs = g_strsplit (statusLine, " ", 3);
		if (strs [0] != NULL && strs [1] != NULL && strs [2] != NULL)
			statusText = g_strdup (strs [2]);
		g_free (statusLine);
		g_strfreev (strs);

		SetStatus (status, statusText);

		/* if we got 100, curl still needs to send the body of the POST
		 * so we don't have the "real" response yet
		 */
		if (status != 100)
			state = STARTED;

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

	bridge->AddCallback (_visitor, this, NULL, 0, name, val);
}

size_t
CurlDownloaderResponse::DataReceived (void *ptr, size_t size)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::DataReceived %p\n", this);

	if (request == NULL)
		return -1;

	if (request->aborting)
		return -1;

	if (state == STOPPED || state == DONE)
		return size;

	if (state == STARTED)
		bridge->AddCallback (_started, this, NULL, 0, NULL, NULL);

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
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Started %p state: %s\n", this, get_state_name (state));

	if (request == NULL)
		return;

	SetCurrentDeployment ();
	state = HEADER;
	request->Started ();
	reported_start = true;
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

	if (request == NULL)
		return;

	SetCurrentDeployment ();
	request->Write (-1, buffer, size);
}

void
CurlDownloaderResponse::Finished ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Finished %p state: %s IsAborted: %i\n", this, get_state_name (state), IsAborted ());

	if (request == NULL)
		return;

	SetCurrentDeployment ();

	if (!reported_start && !IsAborted ())
		request->Started ();

	if (state == STARTED) {
		state = FINISHED;
		return;
	}
	if ((int) state > FINISHED && !IsAborted ())
		request->Succeeded ();
}

class CurlNode : public List::Node {
public:
	enum Action {
		None,
		Add,
		Remove,
		Release,
	};
	CURL* handle;
	Action action;
	EventObject *keep_alive;
	CurlNode (CURL* handle) : Node (), handle(handle), action (None), keep_alive (NULL) {};
	CurlNode (CURL* handle, Action action) : handle (handle), action (action), keep_alive (NULL) {};
	CurlNode (CURL* handle, Action action, EventObject *keep_alive) : handle (handle), action (action), keep_alive (keep_alive)
	{
		if (keep_alive)
			keep_alive->ref ();
	}
	virtual ~CurlNode ()
	{
		if (keep_alive)
			keep_alive->unref_delayed ();
	}
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
			res->unref_delayed ();
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
	quit(false),
	shutting_down(false)
{
	// Create our pipe
	if (pipe (fds) != 0) {
		fds [0] = -1;
		fds [1] = -1;
		LOG_CURL ("BRIDGE CurlHttpHandler pipe (%s).\n", strerror (errno));
	} else {
		// Make the writer pipe non-blocking.
		fcntl (fds [1], F_SETFL, fcntl (fds [1], F_GETFL) | O_NONBLOCK);
	}

	curl_global_init(CURL_GLOBAL_ALL);
	sharecurl = curl_share_init();
	multicurl = curl_multi_init ();
	curl_share_setopt (sharecurl, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);

	pthread_mutexattr_t attribs;
	pthread_mutexattr_init (&attribs);
	pthread_mutexattr_settype (&attribs, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&worker_mutex, &attribs);
	pthread_mutexattr_destroy (&attribs);
	pthread_cond_init (&worker_cond, NULL);
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
CurlHttpHandler::WakeUp ()
{
	int result;

	LOG_CURL ("BRIDGE CurlHttpHandler::WakeUp ()\n");

	pthread_mutex_lock (&worker_mutex);
	pthread_cond_signal (&worker_cond);
	pthread_mutex_unlock (&worker_mutex);

	// Write until something has been written.
	do {
		result = write (fds [1], "c", 1);
	} while (result == 0);
}

void
CurlHttpHandler::Dispose ()
{
	LOG_CURL ("BRIDGE CurlHttpHandler::Dispose ()\n");

	shutting_down = true;
	if (closure) {
		pthread_mutex_lock (&worker_mutex);
		quit = true;
		calls.Clear (true);
		pthread_mutex_unlock (&worker_mutex);
		WakeUp ();

		pthread_join (worker_thread, NULL);
		closure = NULL;
	}

	ExecuteHandleActions ();

	curl_share_cleanup(sharecurl);
	CurlNode* node = (CurlNode *) pool.First ();
	// No need to lock worker_mutex here, since the curl thread isn't running anymore
	while (node != NULL) {
		curl_easy_cleanup (node->handle);
		node = (CurlNode *) node->next;
	}
	pool.Clear (true);

	curl_multi_cleanup(multicurl);
	curl_global_cleanup ();

	pthread_mutex_destroy (&worker_mutex),
	pthread_cond_destroy (&worker_cond);

	HttpHandler::Dispose ();
}

CurlHttpHandler::~CurlHttpHandler ()
{
	LOG_CURL("BRIDGE ~CurlHttpHandler\n");

	if (fds [0] != -1) {
		close (fds [0]);
		fds [0] = -1;
	}
	if (fds [1] != -1) {
		close (fds [1]);
		fds [1] = -1;
	}
}

CURL*
CurlHttpHandler::RequestHandle ()
{
	LOG_CURL ("BRIDGE CurlHttpHandler::RequestHandle pool is %s: ", pool.IsEmpty () ? "empty" : "not empty");

	CURL* handle = NULL;
	CurlNode *node;

	pthread_mutex_lock (&worker_mutex);
	node = (CurlNode *) pool.First ();
	if (node) {
		handle = node->handle;
		pool.Remove (node);
	}
	pthread_mutex_unlock (&worker_mutex);

	if (handle == NULL) {
		handle = curl_easy_init ();
		curl_easy_setopt (handle, CURLOPT_SHARE, sharecurl);
	}
	LOG_CURL ("\t%p\n", handle);

	return handle;
}

void
CurlHttpHandler::ReleaseHandle (CurlDownloaderRequest *res, CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::ReleaseHandle handle:%p\n", handle);

	pthread_mutex_lock (&worker_mutex);
	handle_actions.Append (new CurlNode (handle, CurlNode::Release, res));
	pthread_mutex_unlock (&worker_mutex);
}

void
CurlHttpHandler::ExecuteHandleActions ()
{
	CurlNode *node;

	// LOG_CURL ("BRIDGE CurlHttpHandler::InvokeHandleActions () IsDataThread: %i quit: %i\n", IsDataThread (), quit);

	/* Here we should either be quitting or in the curl thread */
#if SANITY
	g_assert (IsDataThread () || quit); /* #if SANITY */
#endif

	pthread_mutex_lock (&worker_mutex);
	node = (CurlNode *) handle_actions.First ();
	while (node != NULL) {
		switch (node->action) {
		case CurlNode::Release: {

#if SANITY
			/* Check that we don't have duplicate handles in the pool */
			CurlNode *n = (CurlNode *) pool.First ();
			while (n) {
				g_assert (n->handle != node->handle); /* #if SANITY */
				n = (CurlNode *) n->next;
			}
#endif
			LOG_CURL ("BRIDGE CurlHttpHandler::InvokeHandleActions () Release: %p\n", node->handle);
			curl_easy_reset (node->handle);
			pool.Append (new CurlNode (node->handle));
			break;
		}
		case CurlNode::Add:
			LOG_CURL ("BRIDGE CurlHttpHandler::InvokeHandleActions () Add: %p\n", node->handle);
			curl_multi_add_handle (multicurl, node->handle);
			break;
		case CurlNode::Remove:
			LOG_CURL ("BRIDGE CurlHttpHandler::InvokeHandleActions () Remove: %p\n", node->handle);
			curl_multi_remove_handle (multicurl, node->handle);
			break;
		case CurlNode::None:
			LOG_CURL ("BRIDGE CurlHttpHandler::InvokeHandleActions () None !?!: %p\n", node->handle);
			break;
		}
		node = (CurlNode *) node->next;
	}
	handle_actions.Clear (true);
	pthread_mutex_unlock (&worker_mutex);
}

void
CurlHttpHandler::OpenHandle (CurlDownloaderRequest* res, CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::OpenHandle res:%p handle:%p\n", res, handle);

	pthread_mutex_lock (&worker_mutex);
	if (!quit) {
		handles.Append (new HandleNode (res));
		handle_actions.Append (new CurlNode (handle, CurlNode::Add));
	}
	pthread_mutex_unlock (&worker_mutex);

	WakeUp ();
}

void
CurlHttpHandler::CloseHandle (CurlDownloaderRequest* res, CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::CloseHandle res:%p handle:%p\n", res, handle);

	pthread_mutex_lock (&worker_mutex);
	if (!quit) {
		HandleNode* node = (HandleNode*) handles.Find (find_handle, res);
		if (node) {
			handle_actions.Append (new CurlNode (handle, CurlNode::Remove, res));
			handles.Remove (node);
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
CurlHttpHandler::GetData ()
{
	VERIFY_CURL_THREAD

	fd_set r,w,x;
	int running;
	int available;
	int msgs;
	CURLMsg* msg;
	long timeout;
	struct timespec tv;
	CURLMcode res;

	Deployment::RegisterThread ();

	SetCurrentDeployment (true);

	do {
		/* Wait for work */
		pthread_mutex_lock (&worker_mutex);
		while (!quit && handles.IsEmpty ()) {
			pthread_cond_wait (&worker_cond, &worker_mutex);
		}
		pthread_mutex_unlock (&worker_mutex);
		if (quit) break;

		/* Check if handles needs work */
		ExecuteHandleActions ();

		/* Ask curl to do work */
		do {
			res = curl_multi_perform (multicurl, &running);
		} while (!quit && res == CURLM_CALL_MULTI_PERFORM);
		if (quit) break;

		/* Check if some handles are done */
		while ((msg = curl_multi_info_read (multicurl, &msgs))) {
			if (msg->msg == CURLMSG_DONE) {
				pthread_mutex_lock (&worker_mutex);
				HandleNode* node = (HandleNode*) handles.Find (find_easy_handle, msg->easy_handle);
				if (node) {
					calls.Append (new CallData (this, _close, node->res));
				}
				pthread_mutex_unlock (&worker_mutex);
			}
		}

		/* Emit callbacks */
		pthread_mutex_lock (&worker_mutex);
		if (!calls.IsEmpty ()) {
			this->ref ();
			Runtime::GetWindowingSystem ()->AddIdle (EmitCallback, this);
		}
		pthread_mutex_unlock (&worker_mutex);

		if (running == 0)
			continue;

		/* Wait for something to happen */
		FD_ZERO(&r);
		FD_ZERO(&w);
		FD_ZERO(&x);

		if (curl_multi_fdset (multicurl, &r, &w, &x, &available)) {
			fprintf(stderr, "Moonlight: Curl Error: curl_multi_fdset\n");
			goto done;
		}

		if (available == -1)
			continue;

		FD_SET (fds [0], &r);
		available++;

		if (curl_multi_timeout (multicurl, &timeout)) {
			fprintf(stderr, "Moonlight: Curl Error: curl_multi_timeout\n");
			goto done;
		}

		if (timeout <= 0)
			continue;

		tv.tv_sec = timeout / 1000;
		tv.tv_nsec = (timeout % 1000) * 1000 * 1000;

		LOG_CURL ("BRIDGE CurlHttpHandler::GetData (): Entering select...\n");
		if (pselect (available + 1, &r, &w, &x, &tv, NULL) < 0) {
			fprintf(stderr, "Moonlight: Curl Error: select (%i,,,,%li): %i: %s\n", available + 1, timeout, errno, strerror (errno));
		} else if (FD_ISSET (fds [0], &r)) {
			/* We need to read a byte from our pipe */
			guint32 tmp;
			read (fds [0], &tmp, 1);
		}
	} while (!quit);

 done:
	Deployment::SetCurrent (NULL);
	Deployment::UnregisterThread ();
}

void
CurlHttpHandler::AddCallback (CallData *data)
{
	VERIFY_CURL_THREAD

	pthread_mutex_lock (&worker_mutex);
	calls.Append (data);
	pthread_mutex_unlock (&worker_mutex);
}

void
CurlHttpHandler::AddCallback (CallHandler func, CurlDownloaderResponse *res, char *buffer, size_t size, const char* name, const char* val)
{
	AddCallback (new CallData (this, func, res, buffer, size, name, val));
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

CallData::CallData (CurlHttpHandler *bridge, CallHandler func, CurlDownloaderResponse *res, char *buffer, size_t size, const char* name, const char* val)
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

};
