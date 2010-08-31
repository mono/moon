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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "network.h"
#include "debug.h"
#include "deployment.h"
#include "utils.h"
#include "uri.h"

namespace Moonlight {

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
	request_uri = NULL;
	original_uri = NULL;
	final_uri = NULL;
	tmpfile = NULL;
	tmpfile_fd = -1;
	notified_size = -1;
	written_size = 0;
	access_policy = (DownloaderAccessPolicy) -1;
	local_file = NULL;
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

	delete request_uri;
	request_uri = NULL;

	delete final_uri;
	final_uri = NULL;

	g_free (tmpfile);
	tmpfile = NULL;

	if (tmpfile_fd != -1) {
		close (tmpfile_fd);
		tmpfile_fd = -1;
	}

	g_free (local_file);
	local_file = NULL;

	EventObject::Dispose ();
}

void
HttpRequest::Open (const char *verb, const Uri *uri, DownloaderAccessPolicy policy)
{
	Open (verb, uri, NULL, policy);
}

void
HttpRequest::Open (const char *verb, const Uri *uri, const Uri *res_base, DownloaderAccessPolicy policy)
{
	Surface *surface;
	Application *application;
	const Uri *source_location;
	Uri *src_uri = NULL;
	Uri *resource_base = NULL;
	bool is_xap = false;

	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Open (%s, Uri: '%s', ResourceBase: %s, Policy: %i)\n",
		verb, uri ? uri->GetOriginalString () : NULL,
		resource_base ? (resource_base->GetOriginalString () ? resource_base->GetOriginalString () : "") : NULL,
		policy);

	g_free (this->verb);
	delete original_uri;
	delete request_uri;

	this->verb = g_strdup (verb);
	this->original_uri = Uri::Clone (uri);
	this->request_uri = Uri::Clone (uri);

	access_policy = policy;

	surface = GetDeployment ()->GetSurface ();
	application = GetDeployment ()->GetCurrentApplication ();

	/* Get source location */
	if (application->IsRunningOutOfBrowser ()) {
		source_location = surface->GetSourceLocation ();
		is_xap = true;
	} else {
		source_location = GetDeployment ()->GetXapLocation ();
		if (source_location == NULL) {
			source_location = surface->GetSourceLocation ();
		} else {
			is_xap = true;
		}
	}

	/* Validate source location */
	if (source_location == NULL) {
		Failed ("No source location for uri");
		Abort ();
		goto cleanup;
	}

	if (!source_location->IsAbsolute ()) {
		char *msg = g_strdup_printf ("Source location '%s' for uri '%s' is not an absolute uri.", source_location->GetOriginalString (), uri->GetOriginalString ());
		Failed (msg);
		g_free (msg);
		Abort ();
		goto cleanup;
	}

	// Check if the uri is valid
	if (uri->IsInvalidPath ()) {
		LOG_DOWNLOADER ("HttpRequest::Open (): aborting due to invalid uri (uri: %s)\n", uri->GetOriginalString ());
		Failed ("invalid path found in uri");
		Abort ();
		goto cleanup;
	}

	/* Strip off /<assembly name>;component/ from the resource base */
	/* www.getpivot.com runs into this. */
	if (res_base != NULL) {
		const char *rb = res_base->GetOriginalString ();
		if (rb != NULL && rb [0] == '/') {
			const char *res_path = strstr (rb, ";component/");
			if (res_path != NULL)
				resource_base = Uri::Create (res_path + 11 /* ";component/".Length */);
		}
	
		if (resource_base == NULL)
			resource_base = Uri::Clone (res_base);
	}

	/* Make the uri we request to the derived http request an absolute uri */
	if (!uri->IsAbsolute ()) {
		/* DRTs entering here: #0, #171, #173 */

		/* If resource_base != null, absolute path is: Uri.Combine (source_location, Uri.Combine (resource_base, uri)) */
		if (resource_base != NULL && is_xap) {
			Uri *src_base_uri;
			Uri *base_uri;
			Uri *absolute_dummy;
			Uri *absolute_base;

			/* Calculate Uri.Combine (resource_base, uri) => base_uri
			 * Note that the combining ordering is important here, 'uri' can't escape itself
			 * out of 'resource_base' using '..'. We also have to make 'resource_base' an
			 * absolute uri so that the managed uri class can do the combining (we use a dummy
			 * absolute uri to do this) */

			/* Create an absolute base uri of resource_base */

			absolute_dummy = Uri::Create ("http://www.mono-project.com/");
			if (resource_base->GetOriginalString () != NULL) {
				absolute_base = Uri::Create (absolute_dummy, resource_base);
			} else {
				absolute_base = Uri::Clone (absolute_dummy);
			}
			delete absolute_dummy;

			if (absolute_base == NULL) {
				Failed ("Could not create an absolute base uri of the resource base");
				goto cleanup;
			}
			LOG_DOWNLOADER ("HttpRequest::Open (): absolute base uri with dummy root: '%s'\n", absolute_base->ToString ());

			/* Combine the absolute base uri with the uri of the resource */
			base_uri = Uri::Create (absolute_base, uri);
			delete absolute_base;

			if (base_uri == NULL) {
				Failed ("Could not combine absolute resource base and uri");
				goto cleanup;
			}
			LOG_DOWNLOADER ("HttpRequest::Open (): absolute base with dummy root and uri: '%s'\n", base_uri->ToString ());

			/* Calculate Path.Combine (source_dir, base_uri) */
			const char *path = base_uri->GetPath ();
			if (path != NULL && path [0] == '/') {
				/* Skip over any '/' so that the base uri's path is not resolved against the root of the src uri */
				path++;
			}
			src_base_uri = Uri::Create (source_location, path);
			delete base_uri;

			if (src_base_uri == NULL) {
				Failed ("Could not combine source location and relative base+uri");
				goto cleanup;
			}
			LOG_DOWNLOADER ("HttpRequest::Open () final uri: '%s' (path: '%s')\n", src_base_uri->ToString (), path);

			src_uri = src_base_uri;
		} else if (is_xap) {
			Uri *src_base_uri;
			Uri *absolute_dummy;
			Uri *absolute_base;

			/* The GB_* drts run into this condition (GB18030_double1 for instance) */
			/* The uri is resolved against the directory where the xap/xaml file is,
			 * and it's not possible to escape out of it, so we need a dummy root */

			/* Create an absolute base uri of uri */
			absolute_dummy = Uri::Create ("http://www.mono-project.com/");
			if (uri->GetOriginalString () != NULL) {
				absolute_base = Uri::Create (absolute_dummy, uri);
			} else {
				absolute_base = Uri::Clone (absolute_dummy);
			}
			delete absolute_dummy;

			if (absolute_base == NULL) {
				Failed ("Could not create an absolute base uri of the uri");
				goto cleanup;
			}
			LOG_DOWNLOADER ("HttpRequest::Open (): absolute uri with dummy root: '%s'\n", absolute_base->ToString ());

			/* Calculate Uri.Combine (source_dir, absolute_base) */
			const char *path = absolute_base->GetPath ();
			if (path != NULL && path [0] == '/') {
				/* Skip over any '/' so that the base uri's path is not resolved against the root of the src uri */
				path++;
			}
			src_base_uri = Uri::Create (source_location, path);
			delete absolute_base;

			if (src_base_uri == NULL) {
				Failed ("Could not combine source location and relative uri");
				goto cleanup;
			}
			LOG_DOWNLOADER ("HttpRequest::Open () final uri: '%s'\n", src_base_uri->ToString ());

			src_uri = src_base_uri;
		} else {
			/* #45 enters here */
			src_uri = Uri::Create (source_location, uri);
			if (src_uri == NULL) {
				Failed ("Could not combine source location and uri");
				goto cleanup;
			}
		}

		delete request_uri;
		request_uri = Uri::Clone (src_uri);
		LOG_DOWNLOADER ("HttpRequest::Open (%s, %s, %i): is_xap: %i\n"
				"  absolutified to: '%s'\n"
				"  source location: '%s'\n"
				"  resource base:   '%s'\n",
			verb,
			uri ? uri->GetOriginalString () : NULL,
			policy,
			is_xap,
			request_uri ? request_uri->GetOriginalString () : NULL,
			source_location->GetOriginalString (),
			resource_base ?  (resource_base->GetOriginalString () ? resource_base->GetOriginalString () : "") : NULL);
	}

	// FIXME: ONLY VALIDATE IF USED FROM THE PLUGIN
	if (!Downloader::ValidateDownloadPolicy (source_location, request_uri, policy)) {
		LOG_DOWNLOADER ("HttpRequest::Open (): aborting due to security policy violation (request_uri: %s)\n", request_uri->GetOriginalString ());
		Failed ("Security Policy Violation");
		Abort ();
		goto cleanup;
	}

	if (request_uri->IsScheme ("file")) {
		local_file = g_strdup (request_uri->GetPath ());
		NotifyFinalUri (this->request_uri->ToString ());
	}

	if (local_file == NULL)
		OpenImpl ();

cleanup:
	delete src_uri;
	delete resource_base;

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

	if (local_file != NULL) {
		LOG_DOWNLOADER ("HttpRequest::Send (): we're serving the local file: '%s' without going through a network/browser bridge\n", local_file);
		tmpfile = g_strdup (local_file);
		tmpfile_fd = open (local_file, O_RDONLY);
		if (tmpfile_fd == -1) {
			char *msg = g_strdup_printf ("Failed to open '%s': %s\n", local_file, strerror (errno));
			Failed (msg);
			g_free (msg);
			return;
		}

		/* Get the size of the file */
		gint64 file_size;
		file_size = lseek (tmpfile_fd, 0, SEEK_END);
		if (file_size < 0) {
			char *msg = g_strdup_printf ("Failed to get size of file '%s': %s\n", local_file, strerror (errno));
			Failed (msg);
			g_free (msg);
			return;
		};

		if (lseek (tmpfile_fd, 0, SEEK_SET) != 0) {
			char *msg = g_strdup_printf ("Failed to seek to beginning of file '%s': %s\n", local_file, strerror (errno));
			Failed (msg);
			g_free (msg);
			return;
		}

		NotifySize (file_size);

		/* File is here and we can report start request */
		HttpResponse *response = new HttpResponse (this);
		response->SetStatus (200, "OK"); /* Not sure if this is expected or not */
		Started (response);

		/* TODO: Opt-in for write events unless we're serving a local file. */
		/* Serve the file if needed */
		if (HasHandlers (WriteEvent)) {
			void *buffer = g_malloc (4096);
			int n;
			while ((n = read (tmpfile_fd, buffer, 4096)) > 0) {
				Write (-1, buffer, n);
			}
			g_free (buffer);
			if (n == -1) {
				char *msg = g_strdup_printf ("Could not read from '%s': %s\n", local_file, strerror (errno));
				Failed (msg);
				g_free (msg);
				return;
			}
		}

		if (HasHandlers (ProgressChangedEvent)) {
			Emit (ProgressChangedEvent, new HttpRequestProgressChangedEventArgs (1.0));
		}

		/* We're done */
		Succeeded ();
	} else {
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
				char *msg = g_strdup_printf ("Could not create temporary download file %s for url %s\n", templ, GetUri ()->ToString ());
				Failed (msg);
				g_free (msg);
				g_free (templ);
				return;
			}
			tmpfile = templ;
			LOG_DOWNLOADER ("HttpRequest::Send () uri %s is being saved to %s\n", GetUri ()->ToString (), tmpfile);
		} else {
			LOG_DOWNLOADER ("HttpRequest::Send () uri %s is not being saved to disk\n", GetUri ()->ToString ());
		}
	}

#if DEBUG
	GetDeployment ()->AddSource (GetOriginalUri (), tmpfile == NULL ? "Not stored on disk" : tmpfile);
#endif

	if (local_file == NULL)
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
	LOG_DOWNLOADER ("HttpRequest::Abort () is_completed: %i is_aborted: %i original_uri: %s\n", is_completed, is_aborted, original_uri == NULL ? NULL : original_uri->ToString ());

	if (is_completed || is_aborted)
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
	gint64 reported_offset = offset;

	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Write (%" G_GINT64_FORMAT ", %p, %i) HasHandlers: %i\n", offset, buffer, length, HasHandlers (WriteEvent));

	written_size += length;

	/* write to tmp file */
	if (local_file == NULL && tmpfile_fd != -1) {
		if (offset != -1 && lseek (tmpfile_fd, offset, SEEK_SET) == -1) {
			printf ("Moonlight: error while seeking to %" G_GINT64_FORMAT " in temporary file '%s': %s\n", offset, tmpfile, strerror (errno));
		} else if (write (tmpfile_fd, buffer, length) != length) {
			printf ("Moonlight: error while writing to temporary file '%s': %s\n", tmpfile, strerror (errno));
		}
	}

	if (HasHandlers (WriteEvent)) {
		if (reported_offset == -1 && ((options & CustomHeaders) == 0)) {
			/* We check if custom headers is required, if so, our creator might have issued a byte range request,
			 * in which case written_size isn't related to offset */
			reported_offset = written_size - length;
		}
		Emit (WriteEvent, new HttpRequestWriteEventArgs (buffer, reported_offset, length));
	}

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
	LOG_DOWNLOADER ("HttpRequest::Failed (%s) HasHandlers: %i uri: %s\n", msg, HasHandlers (StoppedEvent), GetUri () != NULL ? GetUri ()->ToString () : NULL);

	if (is_completed)
		return;

	is_completed = true;

	if (HasHandlers (StoppedEvent))
		Emit (StoppedEvent, new HttpRequestStoppedEventArgs (msg));
}

// Reference:	URL Access Restrictions in Silverlight 2
//		http://msdn.microsoft.com/en-us/library/cc189008(VS.95).aspx
bool
HttpRequest::CheckRedirectionPolicy (const Uri *dest)
{
	bool retval = false;

	if (!dest)
		return false;

	// the original URI
	Uri *source = this->original_uri;
	if (Uri::IsNullOrEmpty (source))
		return false;

	// if the (original) source is relative then the (final) 'url' will be the absolute version of the uri
	// or if the source scheme is "file" then no server is present for redirecting the url somewhere else
	if (!source->IsAbsolute () || source->IsScheme ("file"))
		return true;

	// if the original URI and the end URI are identical then there was no redirection involved
	if (Uri::Equals (source, dest))
		return true;

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

	return retval;
}

void
HttpRequest::NotifyFinalUri (const char *value)
{
	Uri *uri = Uri::Create (value);
	NotifyFinalUri (uri);
	delete uri;
}

void
HttpRequest::NotifyFinalUri (const Uri *value)
{
	delete final_uri;
	final_uri = Uri::Clone (value);

	// check if (a) it's a redirection and (b) if it is allowed for the current downloader policy

	if (!CheckRedirectionPolicy (final_uri)) {
		LOG_DOWNLOADER ("HttpRequest::NotifyFinalUri ('%s'): Security Policy Validation failed\n", value ? value->ToString () : NULL);
		Failed ("Security Policy Violation");
		Abort ();
	}
}

void
HttpRequest::Succeeded ()
{
	VERIFY_MAIN_THREAD;
	LOG_DOWNLOADER ("HttpRequest::Succeeded (%s) HasHandlers: %i\n", request_uri ? request_uri->ToString () : NULL, HasHandlers (StoppedEvent));

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

HttpResponse::HttpResponse (HttpRequest *request)
	: EventObject (Type::HTTPRESPONSE)
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
	VERIFY_MAIN_THREAD;

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

bool
HttpResponse::ContainsHeader (const char *header, const char *value)
{
	HttpHeader *node;

	if (headers == NULL)
		return false;

	node = (HttpHeader *) headers->First ();
	while (node != NULL) {
		if (!strcmp (node->GetHeader (), header) && !strcmp (node->GetValue (), value))
			return true;
		node = (HttpHeader *) node->next;
	}

	return false;
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

};
