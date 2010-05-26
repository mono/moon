/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * network.cpp
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
#include <sys/types.h>
#include <unistd.h>

#include "network.h"
#include "debug.h"
#include "deployment.h"
#include "utils.h"
#include "uri.h"

/*
 * HttpRequest
 */

HttpRequest::HttpRequest (Type::Kind type, HttpHandler *handler, HttpRequest::Options options)
	: EventObject (type)
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::HttpRequest (%s, %i) id: %i %s\n", handler->GetTypeName (), options, GetId (), GetTypeName ());
	this->handler = handler;
	this->handler->ref ();
	this->response = NULL;
	this->options = options;
	is_aborted = false;
	is_completed = false;
	verb = NULL;
	uri = NULL;
	original_uri = NULL;
	final_uri = NULL;
	tmpfile = NULL;
	tmpfile_fd = -1;
	notified_size = -1;
	written_size = 0;
	access_policy = (DownloaderAccessPolicy) -1;
}

HttpRequest::~HttpRequest ()
{
	GetDeployment ()->UnregisterHttpRequest (this);
}

void
HttpRequest::Dispose ()
{
	HttpResponse *response;

	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Dispose () id: %i\n", GetId ());

	if (handler != NULL) {
		handler->unref ();
		handler = NULL;
	}

	response = this->response;
	this->response = NULL;
	if (response != NULL) {
		response->unref ();
		response = NULL;
	}

	g_free (verb);
	verb = NULL;

	delete original_uri;
	original_uri = NULL;

	g_free (uri);
	uri = NULL;

	g_free (final_uri);
	final_uri = NULL;

	g_free (tmpfile);
	tmpfile = NULL;

	if (tmpfile_fd != -1) {
		close (tmpfile_fd);
		tmpfile_fd = -1;
	}

	EventObject::Dispose ();
}

void
HttpRequest::Open (const char *verb, const char *uri, DownloaderAccessPolicy policy)
{
	VERIFY_MAIN_THREAD;

	Uri *url = new Uri ();
	if (url->Parse (uri))
		Open (verb, url, policy);
		
	delete url;
}

void
HttpRequest::Open (const char *verb, Uri *uri, DownloaderAccessPolicy policy)
{
	const char *source_location;
	Uri *src_uri = NULL;

	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Open (%s, %p = %s, %i)\n", verb, uri, uri == NULL ? NULL : uri->ToString (), policy);

	g_free (this->verb);
	delete original_uri;
	g_free (this->uri);

	this->verb = g_strdup (verb);
	this->original_uri = new Uri (*uri);
	this->uri = uri->ToString ();

	access_policy = policy;

	if (!(source_location = GetDeployment ()->GetXapLocation ()))
		source_location = GetDeployment ()->GetSurface ()->GetSourceLocation ();

	// FIXME: ONLY VALIDATE IF USED FROM THE PLUGIN
	if (!Downloader::ValidateDownloadPolicy (source_location, uri, policy)) {
		LOG_DOWNLOADER ("HttpRequest::Open (): aborting due to security policy violation\n");
		Failed ("Security Policy Violation");
		Abort ();
		return;
	}

	/* Make the uri we request to the derived http request an absolute uri */
	if (!uri->isAbsolute && source_location) {
		src_uri = new Uri ();
		if (!src_uri->Parse (source_location, true)) {
			Failed ("Could not parse source location");
			delete src_uri;
			return;
		}

		src_uri->Combine (uri);
		g_free (this->uri);
		this->uri = src_uri->ToString ();
		delete src_uri;
	}

	OpenImpl ();
}

void
HttpRequest::Send ()
{
	LOG_DOWNLOADER ("HttpRequest::Send () async send disabled: %i\n", options & DisableAsyncSend);

	if (options & DisableAsyncSend) {
		SendAsync ();
	} else {
		AddTickCall (SendAsyncCallback);
	}
}

void
HttpRequest::SendAsync ()
{
	char *templ;

	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::SendAsync () is_aborted: %i is_completed: %i\n", is_aborted, is_completed);

	if (is_aborted || is_completed)
		return;

	/* create tmp file */
	if ((options & DisableFileStorage) == 0) {
		const char *dir = handler->GetDownloadDir ();
		if (dir == NULL) {
			Failed ("Could not create temporary download directory");
			return;
		}
	
		templ = g_build_filename (dir, "XXXXXX", NULL);
		tmpfile_fd = g_mkstemp (templ);
		if (tmpfile_fd == -1) {
			char *msg = g_strdup_printf ("Could not create temporary download file %s for url %s\n", templ, GetUri ());
			Failed (msg);
			g_free (msg);
			g_free (templ);
			return;
		}
		tmpfile = templ;
		LOG_DOWNLOADER ("HttpRequest::Send () uri %s is being saved to %s\n", GetUri (), tmpfile);
	} else {
		LOG_DOWNLOADER ("HttpRequest::Send () uri %s is not being saved to disk\n", GetUri ());
	}

#if DEBUG
	GetDeployment ()->AddSource (GetOriginalUri (), tmpfile == NULL ? "Not stored on disk" : tmpfile);
#endif

	SendImpl ();
}

void
HttpRequest::SendAsyncCallback (EventObject *obj)
{
	((HttpRequest *) obj)->SendAsync ();
}

void
HttpRequest::Abort ()
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Abort () is_completed: %i\n", is_completed);

	if (is_completed)
		return;

	is_aborted = true;
	AbortImpl ();

	Failed ("aborted");

	Dispose ();
}

void
HttpRequest::SetBody (void *body, gint32 length)
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::SetBody (%p, %i)\n", body, length);
	SetBodyImpl (body, length);
}

void
HttpRequest::NotifySize (gint64 size)
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::NotifySize (%" G_GINT64_FORMAT ")\n", size);

	this->notified_size = size;
}

void
HttpRequest::Write (gint64 offset, void *buffer, gint32 length)
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Write (%" G_GINT64_FORMAT ", %p, %i) HasHandlers: %i\n", offset, buffer, length, HasHandlers (WriteEvent));

	written_size += length;

	/* write to tmp file */
	if (tmpfile_fd != -1) {
		if (offset != -1 && lseek (tmpfile_fd, offset, SEEK_SET) == -1) {
			printf ("Moonlight: error while seeking to %" G_GINT64_FORMAT " in temporary file '%s': %s\n", offset, tmpfile, strerror (errno));
		} else if (write (tmpfile_fd, buffer, length) != length) {
			printf ("Moonlight: error while writing to temporary file '%s': %s\n", tmpfile, strerror (errno));
		}
	}

	if (HasHandlers (WriteEvent))
		Emit (WriteEvent, new HttpRequestWriteEventArgs (buffer, offset, length));

	if (notified_size > 0 && HasHandlers (ProgressChangedEvent))
		Emit (ProgressChangedEvent, new HttpRequestProgressChangedEventArgs (((double) offset + (double) length) / (double) notified_size));
}

void
HttpRequest::Started (HttpResponse *response)
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Started ()\n");

	g_warn_if_fail (response != NULL);
	g_warn_if_fail (this->response == NULL);

	if (this->response != NULL)
		this->response->unref ();
	this->response = response;
	if (this->response != NULL)
		this->response->ref ();

	if (HasHandlers (StartedEvent))
		Emit (StartedEvent);
}

void
HttpRequest::Failed (const char *msg)
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Failed (%s) HasHandlers: %i\n", msg, HasHandlers (StoppedEvent));

	if (is_completed)
		return;

	is_completed = true;

	if (HasHandlers (StoppedEvent))
		Emit (StoppedEvent, new HttpRequestStoppedEventArgs (msg));
}

// Reference:	URL Access Restrictions in Silverlight 2
//		http://msdn.microsoft.com/en-us/library/cc189008(VS.95).aspx
bool
HttpRequest::CheckRedirectionPolicy (const char *url)
{
	if (!url)
		return false;

	// the original URI
	Uri *source = this->original_uri;
	if (Uri::IsNullOrEmpty (source))
		return false;

	// if the (original) source is relative then the (final) 'url' will be the absolute version of the uri
	// or if the source scheme is "file" then no server is present for redirecting the url somewhere else
	if (!source->IsAbsolute () || source->IsScheme ("file"))
		return true;

	char *strsrc = source->ToString ();
	// if the original URI and the end URI are identical then there was no redirection involved
	bool retval = (g_ascii_strcasecmp (strsrc, url) == 0);
	g_free (strsrc);
	if (retval)
		return true;

	// the destination URI
	Uri *dest = new Uri ();
	if (dest->Parse (url)) {
		// there was a redirection, but is it allowed ?
		switch (access_policy) {
		case DownloaderPolicy:
			// Redirection allowed for 'same domain' and 'same scheme'
			// note: if 'dest' is relative then it's the same scheme and site
			if (!dest->IsAbsolute () || (Uri::SameDomain (source, dest) && Uri::SameScheme (source, dest)))
				retval = true;
			break;
		case MediaPolicy:
			// Redirection allowed for: 'same scheme' and 'same or different sites'
			// note: if 'dest' is relative then it's the same scheme and site
			if (!dest->IsAbsolute () || Uri::SameScheme (source, dest))
				retval = true;
			break;
		case XamlPolicy:
		case FontPolicy:
		case MsiPolicy:
		case StreamingPolicy:
			// Redirection NOT allowed
			break;
		default:
			// no policy (e.g. downloading codec EULA and binary) is allowed
			retval = true;
			break;
		}
	}

	delete dest;
	
	return retval;
}

void
HttpRequest::NotifyFinalUri (const char *value)
{
	g_free (final_uri);
	final_uri = g_strdup (value);

	// check if (a) it's a redirection and (b) if it is allowed for the current downloader policy

	if (!CheckRedirectionPolicy (final_uri)) {
		LOG_DOWNLOADER ("HttpRequest::NotifyFinalUri ('%s'): Security Policy Validation failed\n", value);
		Failed ("Security Policy Violation");
		Abort ();
	}
}

void
HttpRequest::Succeeded ()
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Succeeded (%s) HasHandlers: %i\n", uri, HasHandlers (StoppedEvent));

	is_completed = true;

	NotifySize (written_size);

	if (HasHandlers (StoppedEvent))
		Emit (StoppedEvent, new HttpRequestStoppedEventArgs (NULL));
}

HttpResponse *
HttpRequest::GetResponse ()
{
	VERIFY_MAIN_THREAD;
	return response;
}

void
HttpRequest::SetHeaderFormatted (const char *header, char *value, bool disable_folding)
{
	SetHeader (header, value, disable_folding);
	g_free (value);
}

void
HttpRequest::SetHeader (const char *header, const char *value, bool disable_folding)
{
	LOG_DOWNLOADER ("HttpRequest::SetHeader (%s, %s, %i)\n", header, value, disable_folding);
	SetHeaderImpl (header, value, disable_folding);
}


/*
 * HttpResponse
 */

HttpResponse::HttpResponse (Type::Kind type, HttpRequest *request)
	: EventObject (type)
{
	/* We don't actually store the HttpRequest, it'd introduce a circular ref */
	headers = NULL;
	response_status = -1;
	response_status_text = NULL;
}

void
HttpResponse::Dispose ()
{
	delete headers;
	headers = NULL;

	g_free (response_status_text);
	response_status_text = NULL;

	EventObject::Dispose ();
}

List *
HttpResponse::GetHeaders ()
{
	return headers;
}

void
HttpResponse::AppendHeader (const char *header, const char *value)
{
	LOG_DOWNLOADER ("HttpResponse::AppendHeader ('%s', '%s')\n", header, value);
	if (headers == NULL)
		headers = new List ();
	headers->Append (new HttpHeader (header, value));
}

void
HttpResponse::ParseHeaders (const char *input)
{
	const char *ptr;
	const char *start = input;

	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpResponse::ParseHeaders:\n%s\n\n", input);

	if (start == NULL || start [0] == 0)
		return;

	/*
	 * Parse status line
	 */

	/* Find http version */
	ptr = start;
	while (*ptr != 0 && *ptr != '\n' && *ptr != '\r') {
		if (*ptr == ' ') {
			LOG_DOWNLOADER ("ParseHeaders: header version: %.*s\n", (int) (ptr - start), start);
			break;
		}
		ptr++;
	};
	/* Skip extra spaces */
	while (*ptr == ' ')
		ptr++;
	if (*ptr == '\n' || *ptr == '\r') {
		LOG_DOWNLOADER ("ParseHeaders: invalid status line\n");
		return;
	}

	/* Find status code */
	start = ptr;
	while (*ptr != 0 && *ptr != '\n' && *ptr != '\r') {
		if (*ptr == ' ') {
			response_status = atoi (start);
			LOG_DOWNLOADER ("ParseHeaders: status code: %i\n", response_status);
			break;
		}
		ptr++;
	}
	/* Skip extra spaces */
	while (*ptr == ' ')
		ptr++;
	if (*ptr == '\n' || *ptr == '\r') {
		LOG_DOWNLOADER ("ParseHeaders: invalid status line\n");
		return;
	}
	/* The rest is status text */
	start = ptr;
	while (*ptr != 0) {
		if (*ptr == '\n' || *ptr == '\r') {
			response_status_text = g_strndup (start, ptr - start);
			LOG_DOWNLOADER ("ParseHeaders: status text: %s\n", response_status_text);
			break;
		}
		ptr++;
	}

	start = ptr;


	/*
	 * Parse header lines
	 */
	char *header = NULL;
	char *value = NULL;

	while (*ptr != 0) {
		if (header == NULL && *ptr == ':') {
			header = g_strndup (start, ptr - start);
			while (*(ptr + 1) == ' ')
				ptr++;
			start = ptr + 1;
		} else if (*ptr == '\n' || *ptr == '\r') {
			if (header != NULL) {
				value = g_strndup (start, ptr - start);
				AppendHeader (header, value);
			}
			start = ptr + 1;
			g_free (header);
			header = NULL;
			g_free (value);
			value = NULL;
		}
		ptr++;
	}

	if (value != NULL && header != NULL)
		AppendHeader (header, value);
	g_free (header);
	g_free (value);
}

void
HttpResponse::VisitHeaders (HttpHeaderVisitor visitor, void *context)
{
	HttpHeader *header;

	VERIFY_MAIN_THREAD;

	if (headers == NULL)
		return;

	header = (HttpHeader *) headers->First ();
	while (header != NULL) {
		visitor (context, header->GetHeader (), header->GetValue ());
		header = (HttpHeader *) header->next;
	}
}

void
HttpResponse::SetStatus (gint32 status, const char *status_text)
{
	LOG_DOWNLOADER ("HttpResponse::SetStatus (%i, %s)\n", status, status_text);
	response_status = status;
	g_free (response_status_text);
	response_status_text = g_strdup (status_text);
}

/*
 * HttpHandler
 */

HttpHandler::HttpHandler (Type::Kind type)
	: EventObject (type)
{
	download_dir = NULL;
}


const char *
HttpHandler::GetDownloadDir ()
{
	if (download_dir == NULL) {
		char *buf = g_build_filename (g_get_tmp_dir (), "moonlight-downloads.XXXXXX", NULL);
		// create a root temp directory for all files
		download_dir = MakeTempDir (buf);
		if (download_dir == NULL) {
			g_free (buf);
			printf ("Moonlight: Could not create temporary download directory.\n");
		} else {
			GetDeployment ()->TrackPath (download_dir);
			LOG_DOWNLOADER ("HttpHandler::GetDownloadDir (): Created temporary download directory: %s\n", download_dir);
		}
	}

	return download_dir;
}