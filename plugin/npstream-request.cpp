/*
 * npstream-downloader.cpp: NPStream Browser downloader
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "plugin-downloader.h"
#include "browser-bridge.h"
#include "npstream-request.h"

namespace Moonlight {

/*
 * NPStreamRequest
 */

NPStreamRequest::NPStreamRequest (BrowserHttpHandler *handler, HttpRequest::Options options)
	: BrowserHttpRequest (handler, options)
{
	stream = NULL;
	pending_unref = false;
}

void
NPStreamRequest::NewStream (NPStream *stream)
{
	BrowserHttpResponse *response;

	this->stream = stream;
	if (IsAborted ()) {
		AbortImpl ();
	} else {
		response = new BrowserHttpResponse (this);
		response->ParseHeaders (stream->headers);
		Started (response);
		response->unref ();

		NotifyFinalUri (stream->url);
		NotifySize (stream->end);
	}
}

void
NPStreamRequest::DestroyStream ()
{
	if (stream != NULL)
		stream = NULL;

	if (pending_unref) {
		/*  Unref the ref we took when we called NPN_GetURLNotify in SendImpl */
		pending_unref = false;
		unref ();
	}
}

void
NPStreamRequest::OpenImpl ()
{
}

void
NPStreamRequest::SendImpl ()
{
	PluginInstance *instance = GetInstance ();
	NPError err;

	g_return_if_fail (instance != NULL);

	err = MOON_NPN_GetURLNotify (instance->GetInstance (), GetUri ()->GetHttpRequestString (), NULL, this);

	if (err == NPERR_NO_ERROR) {
		/* This is a ref to ensure that we don't get deleted while the browser have a pointer to us */
		/* We unref when DestroyStream is called */
		pending_unref = true;
		this->ref ();
		return;
	}

	const char *msg;

	switch (err) {
	case NPERR_GENERIC_ERROR:
		msg = "generic error";
		break;
	case NPERR_OUT_OF_MEMORY_ERROR:
		msg = "out of memory";
		break;
	case NPERR_INVALID_URL:
		msg = "invalid url requested";
		break;
	case NPERR_FILE_NOT_FOUND:
		msg = "file not found";
		break;
	default:
		msg = "unknown error";
		break;
	}

	Failed (msg);
}

void
NPStreamRequest::AbortImpl ()
{
	VERIFY_MAIN_THREAD;

	if (stream != NULL) {
		MOON_NPN_DestroyStream (GetInstance ()->GetInstance (), stream, NPRES_USER_BREAK);

		if (stream != NULL) {
			/* Calling NPN_DestroyStream should end up calling our DestroyStream method, but only ff 3.7+ does it.
			   so if DestroyStream wasn't called, call it manually */
			 DestroyStream ();
		}
	}
}

void
NPStreamRequest::SetBodyImpl (void *body, guint32 length)
{
	printf ("Moonlight: NPStreamRequest does not support SetBody\n");
}

void
NPStreamRequest::SetHeaderImpl (const char *name, const char *value, bool disable_folding)
{
	printf ("Moonlight: NPStreamRequest does not support SetHttpHeader\n");
}

void
NPStreamRequest::UrlNotify (const char *url, NPReason reason)
{
	switch (reason) {
	case NPRES_DONE:
		Succeeded ();
		break;
	case NPRES_USER_BREAK:
		Failed ("user break");
		break;
	case NPRES_NETWORK_ERR:
		Failed ("network error");
		break;
	default:
		Failed ("unknown error");
		break;
	}

	if (pending_unref) {
		/* The browser is supposed to call DestroyStream before UrlNotify.
		 * That doesn't always happen (if the stream was never created, due to a 404 for instance). */
		pending_unref = false;
		this->unref ();
	}
}

void
NPStreamRequest::Write (gint32 offset, gint32 len, void *buffer)
{
	BrowserHttpRequest::Write (offset, buffer, len);
}

};