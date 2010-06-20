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

#define d(x) 1
#define ds(x) 1

// FIX: this is temporary
#define SKIP_PEER 1
#define SKIP_HOSTNAME 1

class CurlDownloaderRequest;
class CurlDownloaderResponse;


// Debugging methods that dump all data send through
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
			dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
			text = "";
		break;
		case CURLINFO_DATA_OUT:
			text = "=> Send data";
			dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
			text = "";
		break;
		case CURLINFO_SSL_DATA_OUT:
			text = "=> Send SSL data";
			dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
			text = "";
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
//	d(printf ("CALLBACK header_received thread:%i\n", (int)pthread_self()));
	CurlDownloaderResponse* response = (CurlDownloaderResponse*) data;
	response->HeaderReceived (ptr, size*nmemb);
	return size*nmemb;
}

static size_t
data_received (void *ptr, size_t size, size_t nmemb, void *data)
{
//	d(printf ("CALLBACK data_received thread:%i\n", (int)pthread_self()));
	CurlDownloaderResponse* response = (CurlDownloaderResponse*) data;
	return response->DataReceived (ptr, size*nmemb);
}

static void
open_callback (EventObject *sender)
{
//	d(printf ("CALLBACK open_callback %p closure:%p\n", ((ResponseClosure*)sender)->res, sender));
	((ResponseClosure*)sender)->res->Open();
}

static void
getdata_callback (EventObject *sender)
{
//	d(printf ("CALLBACK getdata_callback %p closure:%p\n", ((ResponseClosure*)sender)->res, sender));
	((ResponseClosure*)sender)->res->GetData ();
}


// These 4 methods are for asynchronous operation, not used right now

static void
_started (EventObject *sender)
{
	CurlDownloaderResponse *res = ((ResponseClosure*)sender)->res;
	res->Started ();
}

static void
_available (EventObject *sender)
{
	DataClosure *dl = (DataClosure*)sender;
	dl->res->Available (dl);
}

static void
_finished (EventObject *sender)
{
	CurlDownloaderResponse *res = ((ResponseClosure*)sender)->res;
	res->Finished ();
}

static void
_visitor (EventObject *sender)
{
	DataClosure *dl = (DataClosure*)sender;
	dl->res->Visitor (dl->name, dl->val);
}

DownloaderRequest*
CurlBrowserBridge::CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache)
{
	return new CurlDownloaderRequest (this, method, uri, disable_cache);
}


CurlDownloaderRequest::CurlDownloaderRequest (CurlBrowserBridge *bridge, const char *method, const char *uri, bool disable_cache)
	: DownloaderRequest (method, uri), headers(NULL), response(NULL), bridge(bridge), body(0)
{
	d(printf ("BRIDGE CurlDownloaderRequest::CurlDownloaderRequest %p %s\n", this, uri));
	multicurl = curl_multi_init ();
	curl = curl_easy_init ();
	curl_easy_setopt (curl, CURLOPT_SHARE, bridge->sharecurl);


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
	curl_multi_add_handle (multicurl, curl);
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

	body = (void *) MOON_NPN_MemAlloc (size);
	memcpy(body, ptr, size);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDS, body);
	curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, size);
}

bool CurlDownloaderRequest::GetResponse (DownloaderResponseStartedHandler started,
	DownloaderResponseDataAvailableHandler available,
	DownloaderResponseFinishedHandler finished, gpointer context)
{
	d(printf ("BRIDGE CurlDownloaderRequest::GetResponse %p\n", this));

	this->started = true;

	// we're ready to start the connection, set the headers
	response = new CurlDownloaderResponse (bridge, this, started, available, finished, context);

	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, data_received);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, response);
	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, header_received);
	curl_easy_setopt (curl, CURLOPT_WRITEHEADER, response);

	if (isPost ())
		curl_easy_setopt(curl, CURLOPT_POST, 1);

	response->Open ();
	return TRUE;
}

CurlDownloaderRequest::~CurlDownloaderRequest ()
{
//	d(printf ("DESTROYED REQUEST %p\n", this));
}

CurlDownloaderResponse::CurlDownloaderResponse (CurlBrowserBridge *bridge,
	CurlDownloaderRequest *request,
	DownloaderResponseStartedHandler started,
	DownloaderResponseDataAvailableHandler available,
	DownloaderResponseFinishedHandler finished, gpointer ctx)
	: DownloaderResponse(started, available, finished, ctx),
	  bridge(bridge), request(request), visitor(NULL), vcontext(NULL),
	  delay(1), headers(NULL), bodies(NULL), state(STOPPED)
{
	d(printf ("BRIDGE CurlDownloaderResponse::CurlDownloaderResponse %p\n", this));

	curl = request->curl;
	multicurl = request->multicurl;
	closure = new ResponseClosure (this);
}

CurlDownloaderResponse::~CurlDownloaderResponse ()
{
	if (!aborted)
		Abort ();
	headers = NULL;
	bodies = NULL;

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NULL);

	curl_multi_remove_handle (multicurl, curl);

	curl_easy_setopt (curl, CURLOPT_SHARE, NULL);
	curl_easy_cleanup (curl);
	curl_multi_cleanup (multicurl);
}

void
CurlDownloaderResponse::Open ()
{
	if (IsAborted ())
		return;

	if (delay) {
		delay--;
		bridge->plugin->GetSurface()->GetTimeManager()->AddDispatcherCall (open_callback, closure);
		return;
	}
	GetData ();
}

void
CurlDownloaderRequest::Abort () {
	d(printf ("BRIDGE CurlDownloaderRequest::Abort %p\n", this));

	aborted = true;
	Close ();
}

void
CurlDownloaderRequest::Close ()
{
	if (body)
		MOON_NPN_MemFree (body);

	if (response) {
		response->Abort ();
		response = NULL;
	}
	if (headers)
		curl_slist_free_all (headers);
}

void
CurlDownloaderResponse::Abort ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::Abort %p\n", this));
	if (IsAborted ()) {
		closure = NULL;
		if (headers)
			g_list_free (headers);
		if (bodies)
			g_list_free (bodies);
		headers = NULL;
		bodies = NULL;
	}

	aborted = true;
	Close ();
}

void
CurlDownloaderResponse::Close ()
{
	if (closure) {
		bridge->plugin->GetSurface()->GetTimeManager()->RemoveTickCall (open_callback, closure);
		bridge->plugin->GetSurface()->GetTimeManager()->RemoveTickCall (getdata_callback, closure);
		bridge->plugin->GetSurface()->GetTimeManager()->RemoveTickCall (_finished, closure);
		closure = NULL;
	}

	if (headers) {
		GList *t;
		for (t = headers; t; t = t->next)
			bridge->plugin->GetSurface()->GetTimeManager()->RemoveTickCall (_available, (EventObject*)t->data);
		g_list_free (headers);
		headers = NULL;
	}

	if (bodies) {
		GList *t;
		for (t = bodies; t; t = t->next)
			bridge->plugin->GetSurface()->GetTimeManager()->RemoveTickCall (_available, (EventObject*)t->data);
		g_list_free (bodies);
		bodies = NULL;
	}
	state = DONE;
}

void CurlDownloaderResponse::SetHeaderVisitor (DownloaderResponseHeaderCallback callback, gpointer ctx)
{
	d(printf ("BRIDGE CurlDownloaderResponse::SetHeaderVisitor %p callback:%p vcontext:%p\n", this, callback, ctx));
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
	if (IsAborted ())
		return;

	if (!ptr || size <= 2)
		return;

	d(printf ("BRIDGE CurlDownloaderResponse::HeaderReceived %p\n", this));
	if (state == STOPPED) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
		statusText = g_strndup ((char*)ptr, size-2);
		if (status == 200) {
			state = HEADER;
			if (started)
				started (this, context);
		} else if (status > 300) {
			request->Close ();
		}
		return;
	}

	if (!visitor)
		return;
	if (size <= 2)
		return;

	gchar *name, *val;
	char **header = g_strsplit ((char*)ptr, ":", 2);

	if (!header[1])
		return;

	name = g_strdup (header[0]);
	val = g_strdup (header[1]);
	val = g_strstrip (val);

	visitor (vcontext, name, val);
}

size_t
CurlDownloaderResponse::DataReceived (void *ptr, size_t size)
{
	d(printf ("BRIDGE CurlDownloaderResponse::DataReceived thread:%i\n", (int)pthread_self()));

	state = DATA;

	if (!available || IsAborted ())
		return -1;

	char *buffer = (char *) MOON_NPN_MemAlloc (size);
	memcpy(buffer, ptr, size);

	available (this, context, buffer, size);
	MOON_NPN_MemFree (buffer);
	return aborted ? -1 : size;
}

void
CurlDownloaderResponse::GetData ()
{
	if (IsAborted () || state == DONE)
		return;

	int running;
	if (state < DATA) {
		// keep pumping until all the headers have arrived and we start getting data
		while (state < DATA) {
			while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform (multicurl, &running));
			Emit ();
		}
	} else {
		while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform (multicurl, &running));
		Emit ();
	}

	if (running && !IsAborted ())
		bridge->plugin->GetSurface()->GetTimeManager()->AddDispatcherCall (getdata_callback, closure);
	else {
		request->Close ();
		Finished ();
	}
}

void
CurlDownloaderResponse::Started ()
{
	d(printf ("BRIDGE CurlDownloaderResponse::Started %p\n", this));
	started (this, context);
}

void
CurlDownloaderResponse::Available (DataClosure *dl)
{
	d(printf ("BRIDGE CurlDownloaderResponse::Available %p\n", this));
	available (this, context, dl->buffer, dl->size);
}

void
CurlDownloaderResponse::Finished ()
{
	finished (this, context, true, NULL, NULL);
}

void
CurlDownloaderResponse::Visitor (const char *name, const char *val)
{
	d(printf ("BRIDGE CurlDownloaderResponse::Visitor %p visitor:%p vcontext:%p\n", this, visitor, vcontext));
	if (visitor)
		visitor (vcontext, name, val);
}

void
CurlDownloaderResponse::AddCallback (TickData *data, bool delayed)
{
	bodies = g_list_append (bodies, data);
}

static bool
emit (GList *list, Surface *surface, bool delay)
{
	TickData *data = NULL;
	GList *t;
	for (t = list; t; t = t->next) {
		data = (TickData*)t->data;
		surface->GetTimeManager()->AddDispatcherCall (data->func, data->GetClosure ());
	}
	return delay;
}

void
CurlDownloaderResponse::Emit ()
{
	if (IsAborted ())
		return;

	if (!headers && !bodies)
		return;

	bool delayed = FALSE;

	if (bodies) {
		delayed = emit (bodies, bridge->plugin->GetSurface(), delayed);
		g_list_free (bodies);
	}
}

DataClosure::~DataClosure ()
{
	if (buffer)
		MOON_NPN_MemFree (buffer);
	if (name)
		g_free ((gpointer) name);
	if (val)
		g_free ((gpointer) val);
}

TickData::~TickData ()
{
}