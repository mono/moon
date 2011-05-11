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
	if (!MoonThread::IsThread (worker_thread)) {							\
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

static size_t header_received (void *ptr, size_t size, size_t nmemb, void *data);
static size_t data_received (void *ptr, size_t size, size_t nmemb, void *data);
static void _abort (EventObject *sender);
static void _close_handle (CallData *data);

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

	g_warning (text);
	fprintf(stderr, "\n");
	fflush (stderr);
//	dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);

	return 0;
}

#endif

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

/*
 * CurlDownloaderRequest
 */

CurlDownloaderRequest::CurlDownloaderRequest (CurlHttpHandler *handler, HttpRequest::Options options)
	: HttpRequest (Type::CURLDOWNLOADERREQUEST, handler, options), headers(NULL), response(NULL),
	  bridge(handler), body(NULL), state(NONE), aborting(FALSE)
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

void
CurlDownloaderRequest::Started ()
{
	HttpRequest::Started (response);
}

void
CurlDownloaderRequest::Succeeded ()
{
	HttpRequest::Succeeded ();
}

void
CurlDownloaderRequest::SetHeaderImpl (const char *name, const char *value, bool disable_folding)
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::SetHttpHeader %p - %s:%s\n", this, name, value);

	VERIFY_MAIN_THREAD;

	char *header = g_strdup_printf ("%s: %s", name, value);
	headers = curl_slist_append(headers, header);
}

void
CurlDownloaderRequest::SetBodyImpl (const void *ptr, guint32 size)
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::SetBody (%p, %u) %p\n", ptr, size, this);

	VERIFY_MAIN_THREAD;

	g_free (body);
	body = (void *) g_malloc (size);
	memcpy(body, ptr, size);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, size);
	body_set = true;
}

void
CurlDownloaderRequest::SendImpl ()
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Send %p\n", this);

	VERIFY_MAIN_THREAD;

	if (IsAborted ())
		return;

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

	if ((GetOptions () & HttpRequest::ForceHttp_1_0) == ForceHttp_1_0) {
		curl_easy_setopt (curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
	}

	LOG_CURL ("BRIDGE CurlDownloaderRequest::Send %p handle: %p URL: %s\n", this, curl, GetUri ()->GetHttpRequestString ());
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Send %p handle: %p headers: %p\n", this, curl, headers);

	response->Open ();
}

CurlDownloaderRequest::~CurlDownloaderRequest ()
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::~CurlDownloaderRequest handle: %p\n", curl);

	VERIFY_MAIN_THREAD;

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

void
CurlDownloaderRequest::AbortImpl ()
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Abort request:%p response:%p\n", this, response);

	// thread-safe

	if (bridge->IsDataThread ()) {
		aborting = TRUE;
		bridge->AddCallback (new CallData (bridge, _close_handle, this));
	} else {
		if (state != OPENED)
			return;

		state = ABORTED;
		Close (CURLE_OK);
	}
}

void
CurlDownloaderRequest::Close (CURLcode curl_code)
{
	LOG_CURL ("BRIDGE CurlDownloaderRequest::Close (%i = %s) request:%p response:%p\n", curl_code, curl_easy_strerror (curl_code), this, response);

	VERIFY_MAIN_THREAD;

	SetCurrentDeployment ();

	if (state != OPENED && state != ABORTED)
		return;

	if (state != ABORTED)
		state = CLOSED;

	if (response) {
		if (IsAborted ()) {
			response->Abort ();
		} else if (curl_code != CURLE_OK) {
			Failed (curl_easy_strerror (curl_code));
			response->Abort ();
		} else {
			response->Close ();
		}
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

/*
 * CurlDownloaderResponse
 */

CurlDownloaderResponse::CurlDownloaderResponse (CurlHttpHandler *bridge,
	CurlDownloaderRequest *request)
	: HttpResponse (Type::CURLDOWNLOADERRESPONSE, request),
	  bridge(bridge), request(request),
	  status(0), statusText(NULL),
	  state(STOPPED), aborted (false), reported_start (false)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::CurlDownloaderResponse %p request: %p\n", this, request);

	ref (); // we need to keep ourselves alive artifically for some time
	self_ref = true;
}

CurlDownloaderResponse::~CurlDownloaderResponse ()
{
	g_free (statusText);
}

void
CurlDownloaderResponse::ClearRequest ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::ClearRequest () request: %p\n", request);
	VERIFY_MAIN_THREAD;
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
CurlDownloaderResponse::Abort ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Abort request:%p response:%p\n", request, this);

	VERIFY_MAIN_THREAD;

	if (IsAborted () && self_ref) {
		self_ref = false;
		unref (); // no need to stay alive anymore
	}

	aborted = true;
	Close ();
}

void
CurlDownloaderResponse::Close ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Close request:%p response:%p\n", request, this);

	VERIFY_MAIN_THREAD;

	bridge->CloseHandle (request, request->GetHandle ());

	if (self_ref) {
		self_ref = false;
		unref (); // no need to stay alive anymore
	}
	state = DONE;

	Finished ();
}

void
CurlDownloaderResponse::NotifyFinalUriCallback (CallData *data)
{
	data->res->NotifyFinalUri ((char *) data->buffer);
}

void
CurlDownloaderResponse::NotifyFinalUri (char *uri)
{
	request->NotifyFinalUri (uri);
}

void
CurlDownloaderResponse::HeaderReceivedCallback (CallData *data)
{
	data->res->HeaderReceived (data->buffer, data->size);
}

void
CurlDownloaderResponse::HeaderReceived (void *ptr, size_t size)
{
	char *final_url = NULL;
	char *arr [2];

	LOG_CURL ("BRIDGE CurlDownloaderResponse::HeaderReceived (%p, %i): %.*s", this, (int) size, (int) size, (char *) ptr);

	SetCurrentDeployment ();

	// only touch thread-safe properties here
	if (aborted)
		return;

	if (bridge->IsDataThread ()) {
		// check for curl data we can only get on the curl thread
		if (state == 0 || state == 100) {
			curl_easy_getinfo (request->GetHandle (), CURLINFO_RESPONSE_CODE, &status);
			curl_easy_getinfo (request->GetHandle (), CURLINFO_EFFECTIVE_URL, &final_url);
			if (final_url != NULL)
				bridge->AddCallback (new CallData (bridge, NotifyFinalUriCallback, this, g_strdup (final_url), strlen (final_url)));
		}

		bridge->AddCallback (new CallData (bridge, HeaderReceivedCallback, this, g_memdup (ptr, size), size));
		return;
	}

	// we're on the main thread now
	if (IsAborted () || request == NULL || request->aborting)
		return;

	if (!ptr || size <= 2)
		return;

	if (state == STOPPED) {
		// The first line
		// Parse status line: "HTTP/1.X # <status>"
		// Split on the space character and select the third entry
		char *statusLine = g_strndup ((char *) ptr, size); /* Null terminate string */
		char **strs = g_strsplit (statusLine, " ", 3);
		if (strs [0] != NULL && strs [1] != NULL && strs [2] != NULL)
			statusText = g_strdup (strs [2]);
		g_free (statusLine);
		g_strfreev (strs);

		// strip off any ending \r\n
		// fixes #287 in ff4/chrome
		for (int i = 0; statusText [i] != 0; i++) {
			if (statusText [i] == '\n' || statusText [i] == '\r') {
				statusText [i] = 0;
				break;
			}
		}
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

	arr [0] = g_strndup ((char *) ptr, size); // null terminate
	arr [1] = strchr (arr [0], ':');
	if (arr [0] != NULL && arr [1] != NULL) {
		*arr [1] = 0;
		arr [1]++;
		arr [1] = g_strstrip (arr [1]);
		AppendHeader (arr [0], arr [1]);
	}
	g_free (arr [0]);
}

void
CurlDownloaderResponse::DataReceivedCallback (CallData *data)
{
	data->res->DataReceived (data->buffer, data->size);
}

size_t
CurlDownloaderResponse::DataReceived (void *ptr, size_t size)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::DataReceived %p\n", this);

	SetCurrentDeployment ();

	// only touch thread-safe properties here
	if (aborted)
		return -1;

	if (bridge->IsDataThread ()) {
		bridge->AddCallback (DataReceivedCallback, this, g_memdup (ptr, size), size);
		return size;
	}

	// we're on the main thread now

	if (request == NULL) {
		aborted = true;
		return -1;
	}

	if (request->aborting) {
		aborted = true;
		return -1;
	}

	if (state == STOPPED || state == DONE)
		return size;

	if (state == STARTED)
		Started ();

	state = DATA;

	if (IsAborted ()) {
		aborted = true;
		return -1;
	}

	Available (ptr, size);

	return size;
}

void
CurlDownloaderResponse::Started ()
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Started %p state: %s\n", this, get_state_name (state));

	VERIFY_MAIN_THREAD;

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
CurlDownloaderResponse::Available (void *buffer, size_t size)
{
	LOG_CURL ("BRIDGE CurlDownloaderResponse::Available %p\n", this);

	VERIFY_MAIN_THREAD;

	if (request == NULL)
		return;

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

/*
 * CurlNode
 */

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

/*
 * HandleNode
 */
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
	void Close () { return res->Close (CURLE_OK); }
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

/*
 * CurlHttpHandler
 */

CurlHttpHandler::CurlHttpHandler () :
	HttpHandler (Type::CURLHTTPHANDLER),
	sharecurl(NULL),
	multicurl(NULL),
	thread_started (false),
	quit(false),
	shutting_down(false),
	worker_mutex(true)
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
}

HttpRequest *
CurlHttpHandler::CreateRequest (HttpRequest::Options options)
{
	if (!thread_started) {
		thread_started = true;
		MoonThread::Start (&worker_thread, getdata_callback, this);
	}

	return new CurlDownloaderRequest (this, options);
}

void
CurlHttpHandler::WakeUp ()
{
	int result;

	LOG_CURL ("BRIDGE CurlHttpHandler::WakeUp ()\n");

	worker_mutex.Lock();
	worker_cond.Signal();
	worker_mutex.Unlock();

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
	if (thread_started) {
		worker_mutex.Lock();
		quit = true;
		calls.Clear (true);
		worker_mutex.Unlock();
		WakeUp ();

		worker_thread->Join ();
		thread_started = false;
	}

	ExecuteHandleActions ();

	if (sharecurl != NULL) {
		curl_share_cleanup (sharecurl);
		sharecurl = NULL;
	}

	CurlNode* node = (CurlNode *) pool.First ();
	// No need to lock worker_mutex here, since the curl thread isn't running anymore
	while (node != NULL) {
		curl_easy_cleanup (node->handle);
		node = (CurlNode *) node->next;
	}
	pool.Clear (true);

	if (multicurl != NULL) {
		curl_multi_cleanup (multicurl);
		multicurl = NULL;
	}

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

	curl_global_cleanup ();
}

CURL*
CurlHttpHandler::RequestHandle ()
{
	LOG_CURL ("BRIDGE CurlHttpHandler::RequestHandle pool is %s: ", pool.IsEmpty () ? "empty" : "not empty");

	VERIFY_MAIN_THREAD;

	CURL* handle = NULL;
	CurlNode *node;

	worker_mutex.Lock();
	node = (CurlNode *) pool.First ();
	if (node) {
		handle = node->handle;
		pool.Remove (node);
	}
	worker_mutex.Unlock();

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

	VERIFY_MAIN_THREAD;

	worker_mutex.Lock();
	handle_actions.Append (new CurlNode (handle, CurlNode::Release, res));
	worker_mutex.Unlock();
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

	worker_mutex.Lock();
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
	worker_mutex.Unlock();
}

void
CurlHttpHandler::OpenHandle (CurlDownloaderRequest* res, CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::OpenHandle res:%p handle:%p\n", res, handle);

	VERIFY_MAIN_THREAD;

	worker_mutex.Lock();
	if (!quit) {
		handles.Append (new HandleNode (res));
		handle_actions.Append (new CurlNode (handle, CurlNode::Add));
	}
	worker_mutex.Unlock();

	WakeUp ();
}

void
CurlHttpHandler::CloseHandle (CurlDownloaderRequest* res, CURL* handle)
{
	LOG_CURL ("BRIDGE CurlHttpHandler::CloseHandle res:%p handle:%p\n", res, handle);

	VERIFY_MAIN_THREAD;

	worker_mutex.Lock();
	if (!quit) {
		HandleNode* node = (HandleNode*) handles.Find (find_handle, res);
		if (node) {
			handle_actions.Append (new CurlNode (handle, CurlNode::Remove, res));
			handles.Remove (node);
		}
	}
	worker_mutex.Unlock();
}

static void
_close (CallData *sender)
{
	VERIFY_MAIN_THREAD

	CurlDownloaderRequest* req = (CurlDownloaderRequest*) ((CallData*)sender)->req;
	req->Close (sender->curl_code);
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
		worker_mutex.Lock();
		while (!quit && handles.IsEmpty ()) {
			worker_cond.Wait(worker_mutex);
		}
		worker_mutex.Unlock();
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
				worker_mutex.Lock();
				HandleNode* node = (HandleNode*) handles.Find (find_easy_handle, msg->easy_handle);
				if (node) {
					CallData *data = new CallData (this, _close, node->res);
					data->curl_code = msg->data.result;
					calls.Append (data);
				}
				worker_mutex.Unlock();
			}
		}

		/* Emit callbacks */
		worker_mutex.Lock();
		if (!calls.IsEmpty ()) {
			this->ref ();
			Runtime::GetWindowingSystem ()->AddIdle (EmitCallback, this);
		}
		worker_mutex.Unlock();

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
			// this is harmless
			//fprintf(stderr, "Moonlight: Curl Error: select (%i,,,,%li): %i: %s\n", available + 1, timeout, errno, strerror (errno));
		} else if (FD_ISSET (fds [0], &r)) {
			/* We need to read a byte from our pipe */
			char tmp[1];
			
			while (read (fds [0], tmp, 1) == -1 && errno == EINTR)
				;
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

	worker_mutex.Lock();
	calls.Append (data);
	worker_mutex.Unlock();
}

void
CurlHttpHandler::AddCallback (CallHandler func, CurlDownloaderResponse *res, void *buffer, size_t size)
{
	AddCallback (new CallData (this, func, res, buffer, size));
}

bool
CurlHttpHandler::IsDataThread ()
{
	return MoonThread::IsThread (worker_thread);
}

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

	SetCurrentDeployment ();

	/* Clone the list and invoke the calls with the mutex unlocked */
	worker_mutex.Lock();
	node = (CallData *) calls.First ();
	while (node != NULL) {
		next = (CallData *) node->next;
		calls.Unlink (node);
		tmp.Append (node);
		node = next;
	}
	worker_mutex.Unlock();

	/* Invoke the calls */
	node = (CallData *) tmp.First ();
	while (node != NULL && !IsShuttingDown ()) {
		node->func (node);
		node = (CallData *) node->next;
	}
}

/*
 * CallData
 */

CallData::CallData (CurlHttpHandler *bridge, CallHandler func, CurlDownloaderRequest *req)
	: bridge(bridge), func(func), res(NULL), req(req), buffer(NULL), size(0), curl_code (CURLE_OK)
{
	if (bridge)
		bridge->ref ();
	if (req)
		req->ref ();
}

CallData::CallData (CurlHttpHandler *bridge, CallHandler func, CurlDownloaderResponse *res, void *buffer, size_t size)
	: bridge(bridge), func(func), res (res), req(NULL), buffer(buffer), size(size), curl_code (CURLE_OK)
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
	if (req)
		req->unref ();
	if (res)
		res->unref ();
	if (bridge)
		bridge->unref ();
}

};
