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

#define d(x)
#define ds(x) 1

class CurlDownloaderRequest;
class CurlDownloaderResponse;

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
		width = 0x100;

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
		break;
		case CURLINFO_DATA_OUT:
			text = "=> Send data";
		break;
		case CURLINFO_SSL_DATA_OUT:
			text = "=> Send SSL data";
		break;
		case CURLINFO_HEADER_IN:
			text = "<= Recv header";
		break;
		case CURLINFO_DATA_IN:
			text = "<= Recv data";
		break;
		case CURLINFO_SSL_DATA_IN:
			text = "<= Recv SSL data";
		break;
	}

	dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
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
open_callback (EventObject *sender)
{
	((ResponseClosure*)sender)->res->Open();
}

static void
getdata_callback (EventObject *sender)
{
	d(printf ("SENDER:%p %p\n", sender, ((ResponseClosure*)sender)->res));
	((ResponseClosure*)sender)->res->GetData ();
}

DownloaderRequest*
CurlBrowserBridge::CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache)
{
	d(printf ("CurlBrowserBridge::CreateDownloaderRequest %p\n", this));
	return new CurlDownloaderRequest (this, method, uri, disable_cache);
}


CurlDownloaderRequest::CurlDownloaderRequest (CurlBrowserBridge *bridge, const char *method, const char *uri, bool disable_cache)
	: DownloaderRequest (method, uri), headers(NULL), response(NULL), bridge(bridge)
{
	d(printf ("BRIDGE CurlDownloaderRequest::CurlDownloaderRequest %p\n", this));
	multicurl = curl_multi_init();
	curl = curl_easy_init();

#if !(ds(0))
	config.trace_ascii = 1;
	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
	curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "NSPlayer/11.08.0005.0000");
	curl_easy_setopt(curl, CURLOPT_URL, uri);
	curl_multi_add_handle (multicurl, curl);
}


void CurlDownloaderRequest::SetHttpHeader (const char *name, const char *value)
{
	d(printf ("BRIDGE CurlDownloaderRequest::SetHttpHeader %p - %s:%s\n", this, name, value));
	char *header = g_strdup_printf ("%s: %s", name, value);
	headers = curl_slist_append(headers, header);
}

void CurlDownloaderRequest::SetBody (void *body, int size)
{
	d(printf ("BRIDGE CurlDownloaderRequest::SetBody %p %s\n", this, (char*)body));
	headers = curl_slist_append(headers, (char*)body);
}

bool CurlDownloaderRequest::GetResponse (DownloaderResponseStartedHandler started,
	DownloaderResponseDataAvailableHandler available,
	DownloaderResponseFinishedHandler finished, gpointer context)
{
	d(printf ("BRIDGE CurlDownloaderRequest::GetResponse %p, %p %p %p\n", this, started, available, finished));

	this->started = true;
	// we're ready to start the connection, set the headers
	response = new CurlDownloaderResponse (bridge, this, started, available, finished, context);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_received);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_received);
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, response);

	response->Open ();
	return TRUE;
}

CurlDownloaderRequest::~CurlDownloaderRequest ()
{
	d(printf ("DESTROYED REQUEST %p\n", this));
}

CurlDownloaderResponse::CurlDownloaderResponse (CurlBrowserBridge *bridge,
	CurlDownloaderRequest *request,
	DownloaderResponseStartedHandler started,
	DownloaderResponseDataAvailableHandler available,
	DownloaderResponseFinishedHandler finished, gpointer ctx)
	: DownloaderResponse(started, available, finished, ctx),
	  bridge(bridge), visitor(NULL), vcontext(NULL),
	  delay(2)
{
	d(printf ("BRIDGE CurlDownloaderResponse::CurlDownloaderResponse %p, %p %p %p\n", bridge, started, available, finished));

	curl = request->curl;
	multicurl = request->multicurl;
	closure = new ResponseClosure (this);
}


void
CurlDownloaderResponse::Open ()
{
	d(printf ("BRIDGE CurlDownloaderRequest::Open %p\n", this));

	if (IsAborted ())
		return;

	delay--;
	if (delay) {
		bridge->plugin->GetSurface()->GetTimeManager()->AddDispatcherCall (open_callback, closure);
		return;
	}
	GetData ();
}

void CurlDownloaderRequest::Abort ()
{
	d(printf ("CurlDownloaderRequest::Abort %p\n", this));

	aborted = true;
	curl_multi_remove_handle (multicurl, curl);
	curl_easy_cleanup (curl);
	curl_multi_cleanup (multicurl);
	if (response) {
		response->Abort ();
		response = NULL;
	}
}

void CurlDownloaderResponse::Abort ()
{
	d(printf ("CurlDownloaderResponse::Abort %p\n", this));
	if (IsAborted ()) {
		closure = NULL;
	}

	aborted = true;
	if (closure) {
		bridge->plugin->GetSurface()->GetTimeManager()->RemoveTickCall (open_callback, closure);
		bridge->plugin->GetSurface()->GetTimeManager()->RemoveTickCall (getdata_callback, closure);
		closure = NULL;
	}
}

void CurlDownloaderResponse::SetHeaderVisitor (DownloaderResponseHeaderCallback callback, gpointer context)
{
	d(printf ("BRIDGE CurlDownloaderResponse::SetHeaderVisitor %p\n", callback));
	this->visitor = callback;
	this->vcontext = context;
}

int CurlDownloaderResponse::GetResponseStatus ()
{
	return status;
}

const char * CurlDownloaderResponse::GetResponseStatusText ()
{
	return 0;
}

void CurlDownloaderResponse::ref ()
{
	d(printf ("CurlDownloaderResponse::ref %p %p\n", this, closure.get()));
}

void CurlDownloaderResponse::unref ()
{
	d(printf ("CurlDownloaderResponse::unref %p %p\n", this, closure.get()));
}

void
CurlDownloaderResponse::HeaderReceived (void *ptr, size_t size)
{
	if (IsAborted ())
		return;

	d(printf ("BRIDGE CurlDownloaderResponse::HeaderReceived %s %p\n", (isStarted ? "true" : "false"), started));
	if (!isStarted) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
		if (status == 200) {
			isStarted = TRUE;
			if (started)
				started (this, context);
			return;
		}
	}

	if (!visitor)
		return;

	const char *name, *val;
	char **header = g_strsplit ((char*)ptr, ":", 2);
	if (!header[0] || !header[1])
		return;

	name = g_strdup (header[0]);
	val = g_strndup (header[1], g_utf8_strlen(header[1], -1) - 2);
	visitor (vcontext, name, val);

	g_free ((gpointer) name);
	g_free ((gpointer) val);
}

size_t
CurlDownloaderResponse::DataReceived (void *ptr, size_t size)
{
	if (!available || IsAborted ())
		return -1;

	char *buffer = (char *) MOON_NPN_MemAlloc (size);
	memcpy(buffer, ptr, size);
	available (this, this->context, buffer, size);
	MOON_NPN_MemFree (buffer);
	return aborted ? -1 : size;
}

void
CurlDownloaderResponse::GetData ()
{
	if (IsAborted ())
		return;

	d(printf ("BRIDGE:%p\n", bridge));
	int running;
	while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multicurl, &running));
	bridge->plugin->GetSurface()->GetTimeManager()->AddDispatcherCall (getdata_callback, closure);
}
