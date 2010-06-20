/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * curl-http.cpp: Curl bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */


// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#include "plugin.h"

#include "curl-bridge.h"
#include "curl-http.h"
#include "config.h"
#include "pipeline.h"

#define d(x)
#define ds(x) 1

// FIX: this is temporary
#define SKIP_PEER 1
#define SKIP_HOSTNAME 1

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
static void _finished (CallData *sender);


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

static void
_finished (CallData *sender)
{
	CurlDownloaderResponse* res = (CurlDownloaderResponse*) ((CallData*)sender)->res;
	res->Finished ();
}

CurlDownloaderRequest::CurlDownloaderRequest (CurlBrowserBridge *bridge, const char *method, const char *uri, bool disable_cache)
	: DownloaderRequest (method, uri), headers(NULL), response(NULL),
	  bridge(bridge), post(NULL), postlast(NULL), body(NULL), state(NONE), aborting(FALSE)
{
	d(printf ("BRIDGE CurlDownloaderRequest::CurlDownloaderRequest %p %s\n", this, uri));

	VERIFY_MAIN_THREAD

	curl = bridge->RequestHandle ();

#if !(ds(0))
	config.trace_ascii = 1;
	curl_easy_setopt (curl, CURLOPT_DEBUGFUNCTION, my_trace);
	curl_easy_setopt (curl, CURLOPT_DEBUGDATA, &config);
	curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
#endif
#ifdef SKIP_PEER
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
	curl_easy_setopt (curl, CURLOPT_USERAGENT, "NSPlayer/11.08.0005.0000");
	curl_easy_setopt (curl, CURLOPT_URL, uri);
	curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
}


void CurlDownloaderRequest::SetHttpHeader (const char *name, const char *value)
{
	d(printf ("BRIDGE CurlDownloaderRequest::SetHttpHeader %p - %s:%s\n", this, name, value));
	char *header = g_strdup_printf ("%s: %s", name, value);
	headers = curl_slist_append(headers, header);
}

void CurlDownloaderRequest::SetBody (void *ptr, int size)
{
	d(printf ("BRIDGE CurlDownloaderRequest::SetBody %p\n", this));

	body = (void *) g_malloc (size);
	memcpy(body, ptr, size);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, size);
}

bool CurlDownloaderRequest::GetResponse (DownloaderResponseStartedHandler started,
	DownloaderResponseDataAvailableHandler available,
	DownloaderResponseFinishedHandler finished, gpointer context)
{
	d(printf ("BRIDGE CurlDownloaderRequest::GetResponse %p\n", this));

	if (IsAborted ())
		return FALSE;

	VERIFY_MAIN_THREAD

	state = OPENED;

	if (isPost ())
		curl_easy_setopt(curl, CURLOPT_POST, 1);

	// we're ready to start the connection, set the headers
	response = new CurlDownloaderResponse (bridge, this, started, available, finished, context);
	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, data_received);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, response);
	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, header_received);
	curl_easy_setopt (curl, CURLOPT_WRITEHEADER, response);

	response->Open ();
	return TRUE;
}

CurlDownloaderRequest::~CurlDownloaderRequest ()
{
}

CurlDownloaderResponse::CurlDownloaderResponse (CurlBrowserBridge *bridge,
	CurlDownloaderRequest *request,
	DownloaderResponseStartedHandler started,
	DownloaderResponseDataAvailableHandler available,
	DownloaderResponseFinishedHandler finished, gpointer ctx)
	: DownloaderResponse(started, available, finished, ctx),
	  bridge(bridge), request(request), visitor(NULL), vcontext(NULL),
	  delay(2), state(STOPPED)
{
	d(printf ("BRIDGE CurlDownloaderResponse::CurlDownloaderResponse %p\n", this));

	closure = new ResponseClosure (this);
}

CurlDownloaderResponse::~CurlDownloaderResponse ()
{
}

void
CurlDownloaderResponse::Open ()
{
	VERIFY_MAIN_THREAD

	if (IsAborted ())
		return;

	if (delay) {
		delay--;
		bridge->GetSurface()->GetTimeManager()->AddDispatcherCall (_open, closure);
		return;
	}
	bridge->OpenHandle (request, request->GetHandle ());
}

void
CurlDownloaderRequest::Abort () {
	d(printf ("BRIDGE CurlDownloaderRequest::Abort request:%p response:%p\n", this, response));

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
	d(printf ("BRIDGE CurlDownloaderRequest::Close request:%p response:%p\n", this, response));

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
CurlDownloaderResponse::Abort ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::Abort request:%p response:%p\n", request, this));

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
	d(printf ("BRIDGE CurlDownloaderResponse::Close request:%p response:%p\n", request, this));

	VERIFY_MAIN_THREAD

	curl_easy_setopt(request->GetHandle (), CURLOPT_HTTPHEADER, NULL);
	curl_easy_setopt(request->GetHandle (), CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(request->GetHandle (), CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(request->GetHandle (), CURLOPT_HEADERFUNCTION, NULL);

	bridge->CloseHandle (request, request->GetHandle ());

	if (closure) {
		bridge->GetSurface()->GetTimeManager()->RemoveTickCall (_open, closure);
		closure = NULL;
	}
	state = DONE;

	Finished ();
}

void CurlDownloaderResponse::SetHeaderVisitor (DownloaderResponseHeaderCallback callback, gpointer ctx)
{
	d(printf ("BRIDGE CurlDownloaderResponse::SetHeaderVisitor %p callback:%p vcontext:%p\n", this, callback, ctx));

	VERIFY_MAIN_THREAD

	this->visitor = callback;
	this->vcontext = ctx;
}

int CurlDownloaderResponse::GetResponseStatus ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::GetResponseStatus %p\n", this));
	return status;
}

const char * CurlDownloaderResponse::GetResponseStatusText ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::GetResponseStatusText %p\n", this));
	return 0;
}

void CurlDownloaderResponse::ref ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::ref %p %p\n", this, closure.get()));
}

void CurlDownloaderResponse::unref ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::unref %p %p\n", this, closure.get()));
}

void
CurlDownloaderResponse::HeaderReceived (void *ptr, size_t size)
{
	d(printf ("BRIDGE CurlDownloaderResponse::HeaderReceived %p\n", this));
	d(printf ("%s", ptr));

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
			request->Abort ();
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
	d(printf ("BRIDGE CurlDownloaderResponse::DataReceived %p\n", this));

	if (request->aborting)
		return -1;

	if (state == STOPPED || state == DONE)
		return size;
	state = DATA;

	if (!available || IsAborted ())
		return -1;

	char *buffer = (char *) g_malloc (size);
	memcpy(buffer, ptr, size);

	bridge->AddCallback (_available, this, buffer, size, NULL, NULL);
	return aborted ? -1 : size;
}

void
CurlDownloaderResponse::Started ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::Started %p\n", this));

	state = HEADER;
	if (started)
		started (this, context);
	if (state == FINISHED)
		Finished ();
}

void
CurlDownloaderResponse::Visitor (const char *name, const char *val)
{
	d(printf ("BRIDGE CurlDownloaderResponse::Visitor %p visitor:%p vcontext:%p\n", this, visitor, vcontext));
	if (visitor)
		visitor (vcontext, name, val);
}

void
CurlDownloaderResponse::Available (char* buffer, size_t size)
{
	d(printf ("BRIDGE CurlDownloaderResponse::Available %p\n", this));
	if (available)
		available (this, context, buffer, size);
}

void
CurlDownloaderResponse::Finished ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::Finished %p\n", this));

	if (state == STARTED) {
		state = FINISHED;
		return;
	}
	if (finished && (int)state > FINISHED && !IsAborted ())
		finished (this, context, true, NULL, NULL);
}
