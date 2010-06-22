/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * downloader.cpp: Downloader class.
 *
 * The downloader implements two modes of operation:
 *
 *    bare bones:  this is the interface expected by Javascript and C#
 *		 this is the default if the caller does not call
 *		 Downloader::SetWriteFunc
 * 
 *    progressive: this interface is used internally by the Image
 *		 class to do progressive loading.   If you want to
 *		 use this mode, you must call the SetWriteFunc routine
 *		 to install your callbacks before starting the download.
 * 
 * TODO:
 *    Need a mechanism to notify the managed client of errors during 
 *    download.
 *
 *    Need to provide the buffer we downloaded to GetResponseText(string PartName)
 *    so we can return the response text for the given part name.
 *
 *    The providers should store the files *somewhere* and should be able
 *    to respond to the "GetResponsetext" above on demand.   The current
 *    code in demo.cpp and ManagedDownloader are not complete in this regard as
 *    they only stream
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "downloader.h"
#include "file-downloader.h"
#include "mms-downloader.h"
#include "runtime.h"
#include "utils.h"
#include "error.h"
#include "debug.h"
#include "uri.h"
#include "deployment.h"

//
// Downloader
//

Downloader::Downloader ()
	: DependencyObject (Type::DOWNLOADER)
{
	LOG_DOWNLOADER ("Downloader::Downloader ()\n");

	downloader_state = Downloader::create_state (this);
	user_data = NULL;
	context = NULL;
	notify_size = NULL;
	writer = NULL;
	internal_dl = NULL;
	
	send_queued = false;
	started = false;
	aborted = false;
	completed = false;
	custom_header_support = false;
	disable_cache = false;
	file_size = -2;
	total = 0;
	
	filename = NULL;
	buffer = NULL;
	failed_msg = NULL;
}


Downloader::~Downloader ()
{
	LOG_DOWNLOADER ("Downloader::~Downloader ()\n");
	
	Downloader::destroy_state (downloader_state);
	
	g_free (filename);
	g_free (buffer);
	g_free (failed_msg);

	// NOTE:
	// mms code relies on the internal downloader to be alive while it has a ref on the downloader
	// update mms code if this assumption changes.
	if (internal_dl != NULL)
		internal_dl->unref ();
}

void
Downloader::InternalAbort ()
{
	LOG_DOWNLOADER ("Downloader::InternalAbort ()\n");
	if (!GetSurface ())
		return;

	abort_func (downloader_state);
}

void
Downloader::Abort ()
{
	LOG_DOWNLOADER ("Downloader::Abort ()\n");
	
	SetCurrentDeployment ();
	
	if (!aborted && !failed_msg) {
		InternalAbort ();
		SetDownloadProgress (0.0);
		send_queued = false;
		aborted = true;
	}
}

char *
Downloader::GetDownloadedFilename (const char *partname)
{
	LOG_DOWNLOADER ("Downloader::GetDownloadedFilename (%s)\n", filename);
	
	g_return_val_if_fail (internal_dl != NULL && internal_dl->Is (Type::FILEDOWNLOADER), NULL);
	
	// This is a horrible hack to work around mozilla bug #444160
	// Basically if a very small file is downloaded (<64KB in mozilla as of Jan5/09
	// it can be inserted into a shared cache map, and served up to us without ever
	// giving us the filename for a NP_ASFILE request.
	if (buffer != NULL) {
		FileDownloader *fdl = (FileDownloader *) internal_dl;
		char *tmpfile;
		int fd;
		
		tmpfile = g_build_filename (g_get_tmp_dir (), "mozilla-workaround-XXXXXX", NULL);
		if ((fd = g_mkstemp (tmpfile)) == -1) {
			g_free (tmpfile);
			return NULL;
		}
		
		if (write_all (fd, buffer, (size_t) total) == -1) {
			unlink (tmpfile);
			g_free (tmpfile);
			close (fd);
			return NULL;
		}
		
		close (fd);
		
		fdl->SetFilename (tmpfile);
		fdl->SetUnlink (true);
		g_free (tmpfile);
		g_free (buffer);
		buffer = NULL;
	}
	
	return internal_dl->GetDownloadedFilename (partname);
}

char *
Downloader::GetResponseText (const char *PartName, gint64 *size)
{
	LOG_DOWNLOADER ("Downloader::GetResponseText (%s, %p)\n", PartName, size);

	// This is a horrible hack to work around mozilla bug #444160
	// Basically if a very small file is downloaded (<64KB in mozilla as of Jan5/09
	// it can be inserted into a shared cache map, and served up to us without ever
	// giving us the filename for a NP_ASFILE request.
	if (PartName == NULL && buffer != NULL) {
		char *data;
		char b[4096];
		ssize_t nread;
		GByteArray *buf;

		TextStream *stream = new TextStream ();

		if (!stream->OpenBuffer (buffer, total)) {
			delete stream;
			return NULL;
		}

		buf = g_byte_array_new ();
		while ((nread = stream->Read (b, sizeof (b))) > 0)
			g_byte_array_append (buf, (const guint8 *) b, nread);

		*size = buf->len;

		g_byte_array_append (buf, (const guint8 *) "", 1);
		data = (char *) buf->data;

		g_byte_array_free (buf, false);
		delete stream;

		return data;
	}

	return internal_dl->GetResponseText (PartName, size);
}

void
Downloader::InternalOpen (const char *verb, const char *uri)
{
	LOG_DOWNLOADER ("Downloader::InternalOpen (%s, %s) requires custom header support: %i\n", verb, uri, custom_header_support);

	open_func (downloader_state, verb, uri, custom_header_support, disable_cache);
}

static bool
same_scheme (const Uri *uri1, const Uri *uri2)
{
	return uri1->GetScheme () && uri2->GetScheme () &&
		!strcmp (uri1->GetScheme (), uri2->GetScheme ());
}

static bool
same_domain (const Uri *uri1, const Uri *uri2)
{
	const char *host1 = uri1->GetHost ();
	const char *host2 = uri2->GetHost ();
	
	if (host1 && host2)
		return g_ascii_strcasecmp (host1, host2) == 0;
	
	if (!host1 && !host2)
		return true;
	
	return false;
}

// Reference:	URL Access Restrictions in Silverlight 2
//		http://msdn.microsoft.com/en-us/library/cc189008(VS.95).aspx
bool
Downloader::CheckRedirectionPolicy (const char *url)
{
	if (!url)
		return false;

	// the original URI
	Uri *source = GetUri ();
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
			if (!dest->IsAbsolute () || (same_domain (source, dest) && same_scheme (source, dest)))
				retval = true;
			break;
		case MediaPolicy:
			// Redirection allowed for: 'same scheme' and 'same or different sites'
			// note: if 'dest' is relative then it's the same scheme and site
			if (!dest->IsAbsolute () || same_scheme (source, dest))
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

// Reference:	URL Access Restrictions in Silverlight 2
//		http://msdn.microsoft.com/en-us/library/cc189008(VS.95).aspx
static bool
validate_policy (const char *location, const Uri *source, DownloaderAccessPolicy policy)
{
	if (!location || !source)
		return true;
	
	if (!source->IsAbsolute ()) {
		//relative uri, not checking policy
		return true;
	}

	Uri *target = new Uri ();
	if (!target->Parse (location)) {
		delete target;
		return false;
	}

	bool retval = true;
	switch (policy) {
	case DownloaderPolicy:
		//Allowed schemes: http, https
		if (!target->IsScheme ("http") && !target->IsScheme ("https"))
			retval = false;
		//X-Scheme: no
		if (!same_scheme (target, source))
			retval = false;
		//X-Domain: requires policy file
		// FIXME only managed is implemented
		if (!same_domain (target, source))
			retval = false;
		break;
	case MsiPolicy:
	case MediaPolicy: //Media, images, ASX
		//Allowed schemes: http, https, file
		if (!target->IsScheme ("http") && !target->IsScheme ("https") && !target->IsScheme ("file"))
			retval = false;
		//X-Scheme: no
		if (!same_scheme (target, source))
			retval = false;
		//X-Domain: Allowed
		break;
	case XamlPolicy:
		//Allowed schemes: http, https, file
		if (!target->IsScheme ("http") && !target->IsScheme ("https") && !target->IsScheme ("file"))
			retval = false;
		//X-Scheme: no
		if (!same_scheme (target, source))
			retval =false;
		//X-domain: allowed if not HTTPS to HTTPS
		if (!same_domain (target, source) && target->IsScheme ("https") && source->IsScheme ("https"))
			retval = false;
		break;
	case FontPolicy:
		//Allowed schemes: http, https, file
		if (!target->IsScheme ("http") && !target->IsScheme ("https") && !target->IsScheme ("file"))
			retval = false;
		//X-Scheme: no
		if (!same_scheme (target, source))
			retval = false;
		//X-domain: no
		if (!same_domain (target, source))
			retval = false;
		break;
	case StreamingPolicy: //Streaming media
		//Allowed schemes: http
		if (!target->IsScheme ("http"))
			retval = false;
		//X-scheme: Not from https
		if (source->IsScheme ("https") && !same_scheme (source, target))
			retval = false;
		//X-domain: allowed if not HTTPS to HTTPS
		if (!same_domain (target, source) && target->IsScheme ("https") && source->IsScheme ("https"))
			retval = false;
		break;
	default:
		break;
	}
	
	delete target;
	
	return retval;
}

void
Downloader::OpenInitialize ()
{
	send_queued = false;
	started = false;
	aborted = false;
	completed = false;
	file_size = -2;
	total = 0;

	g_free (failed_msg);
	g_free (filename);
	g_free (buffer);
	failed_msg = NULL;
	filename = NULL;
	buffer = NULL;
}

void
Downloader::Open (const char *verb, const char *uri, DownloaderAccessPolicy policy)
{
	LOG_DOWNLOADER ("Downloader::Open (%s, %s)\n", verb, uri);
	
	OpenInitialize ();
	
	Uri *url = new Uri ();
	if (url->Parse (uri))
		Open (verb, url, policy);
		
	delete url;
}

bool
Downloader::ValidateDownloadPolicy (const char *source_location, Uri *uri, DownloaderAccessPolicy policy)
{
	Uri *src_uri = NULL;
	bool valid;
	
	if (!uri->isAbsolute && source_location) {
		src_uri = new Uri ();
		if (!src_uri->Parse (source_location, true)) {
			delete src_uri;
			return false;
		}
		
		src_uri->Combine (uri);
		uri = src_uri;
	}
	
	valid = validate_policy (source_location, uri, policy);
	delete src_uri;
	
	return valid;
}

void
Downloader::Open (const char *verb, Uri *uri, DownloaderAccessPolicy policy)
{
	const char *source_location;
	Uri *src_uri = NULL;
	Uri *url = uri;
	char *str;
	
	LOG_DOWNLOADER ("Downloader::Open (%s, %p)\n", verb, uri);
	
	OpenInitialize ();
	
	access_policy = policy;
	
	if (!(source_location = GetDeployment ()->GetXapLocation ()))
		source_location = GetSurface ()->GetSourceLocation ();
	
	// FIXME: ONLY VALIDATE IF USED FROM THE PLUGIN
	if (!Downloader::ValidateDownloadPolicy (source_location, uri, policy)) {
		LOG_DOWNLOADER ("aborting due to security policy violation\n");
		failed_msg = g_strdup ("Security Policy Violation");
		Abort ();
		return;
	}
	
	if (!uri->isAbsolute && source_location) {
		src_uri = new Uri ();
		if (!src_uri->Parse (source_location, true)) {
			delete src_uri;
			return;
		}
		
		src_uri->Combine (uri);
		url = src_uri;
	}
	
	if (policy == StreamingPolicy) {
		internal_dl = (InternalDownloader *) new MmsDownloader (this);
	} else {
		internal_dl = (InternalDownloader *) new FileDownloader (this);
	}

	send_queued = false;
	
	SetUri (uri);
	
	int uriflags = 0;
	if (GetSurface () && GetSurface ()->GetRelaxedMediaMode ())
		uriflags |= UriShowFileScheme;
	str = url->ToString ((UriToStringFlags)uriflags);
	delete src_uri;
	
	internal_dl->Open (verb, str);
	g_free (str);
}

void
Downloader::InternalSetHeader (const char *header, const char *value)
{
	LOG_DOWNLOADER ("Downloader::InternalSetHeader (%s, %s)\n", header, value);
	
	header_func (downloader_state, header, value);
}

void
Downloader::InternalSetHeaderFormatted (const char *header, char *value)
{
	InternalSetHeader (header, (const char *) value);
	g_free (value);
}

void
Downloader::InternalSetBody (void *body, guint32 length)
{
	LOG_DOWNLOADER ("Downloader::InternalSetBody (%p, %u)\n", body, length);
	
	body_func (downloader_state, body, length);
}

void
Downloader::SendInternal ()
{
	LOG_DOWNLOADER ("Downloader::SendInternal ()\n");
	
	if (!GetSurface ()) {
		// The plugin is already checking for surface before calling Send, so
		// if we get here, it's either managed code doing something wrong or ourselves.
		g_warning ("Downloader::SendInternal (): No surface!\n");
	}

	if (!send_queued)
		return;
	
	send_queued = false;
	
	if (completed) {
		// Consumer is re-sending a request which finished successfully.
		NotifyFinished (NULL);
		return;
	}
	
	if (failed_msg != NULL) {
		// Consumer is re-sending a request which failed.
		Emit (DownloadFailedEvent, new ErrorEventArgs (DownloadError,
							       MoonError (MoonError::EXCEPTION, 1, failed_msg)));
		return;
	}
	
	started = true;
	aborted = false;
	
	send_func (downloader_state);
}

static void
send_async (EventObject *user_data)
{
	Downloader *downloader = (Downloader *) user_data;
	
	downloader->SendInternal ();
}

void
Downloader::Send ()
{
	LOG_DOWNLOADER ("Downloader::Send ()\n");
	
	if (!GetSurface ()) {
		// The plugin is already checking for surface before calling Send, so
		// if we get here, it's either managed code doing something wrong or ourselves.
		g_warning ("Downloader::Send (): No surface!\n");
	}

	if (send_queued)
		return;
	
	send_queued = true;
	SetStatusText ("");
	SetStatus (0);
	
	AddTickCall (send_async);
}

void
Downloader::SendNow ()
{
	LOG_DOWNLOADER ("Downloader::SendNow ()\n");
	
	send_queued = true;
	SetStatusText ("");
	SetStatus (0);
	
	SendInternal ();
}

//
// A zero write means that we are done
//
void
Downloader::Write (void *buf, gint32 offset, gint32 n)
{
	char* struri = NULL;
	LOG_DOWNLOADER ("Downloader::Write (%p, %i, %i). Uri: %s\n", buf, offset, n, (struri = GetUri ()->ToString ()));
	g_free (struri);
	
	SetCurrentDeployment ();
	
	if (aborted)
		return;
		
	if (!GetSurface ())
		return;
	
	internal_dl->Write (buf, offset, n);
}

void
Downloader::InternalWrite (void *buf, gint32 offset, gint32 n)
{
	LOG_DOWNLOADER ("Downloader::InternalWrite (%p, %i, %i)\n", buf, offset, n);
	
	double progress;

	// Update progress
	if (n > 0)
		total += n;

	if (file_size >= 0) {
		if ((progress = total / (double) file_size) > 1.0)
			progress = 1.0;
	} else 
		progress = 0.0;

	SetDownloadProgress (progress);
	
	Emit (DownloadProgressChangedEvent);

	if (writer)
		writer (buf, offset, n, user_data);
	
	// This is a horrible hack to work around mozilla bug #444160
	// See Downloader::GetResponseText for an explanation
	if (internal_dl->GetObjectType () == Type::FILEDOWNLOADER && n == total && total < 65536) {
		buffer = (char *) g_malloc ((size_t) total);
		memcpy (buffer, buf, (size_t) total);
	} 
}

void
Downloader::SetFilename (const char *fname)
{
	LOG_DOWNLOADER ("Downloader::SetFilename (%s)\n", fname);
	
	g_free (filename);
	g_free (buffer);
	buffer = NULL;
	
	filename = g_strdup (fname);
	
	internal_dl->SetFilename (filename);
}

void
Downloader::NotifyFinished (const char *final_uri)
{
	if (aborted)
		return;
	
	SetCurrentDeployment ();
	
	if (!GetSurface ())
		return;
	
	SetDownloadProgress (1.0);
	
	Emit (DownloadProgressChangedEvent);
	
	// HACK, we should provide the actual status text and code
	SetStatusText ("OK");
	SetStatus (200);
	
	completed = true;

	Emit (CompletedEvent, NULL);
}

void
Downloader::NotifyFailed (const char *msg)
{
	LOG_DOWNLOADER ("Downloader::NotifyFailed (%s)\n", msg);
	
	/* if we've already been notified of failure, no-op */
	if (failed_msg)
		return;
	
	SetCurrentDeployment ();
	
	if (!GetSurface ())
		return;
	
	// SetStatus (400);
	// For some reason the status is 0, not updated on errors?
	
	Emit (DownloadFailedEvent, new ErrorEventArgs (DownloadError,
						       MoonError (MoonError::EXCEPTION, 1, msg)));
	
	failed_msg = g_strdup (msg);
}

void
Downloader::NotifySize (gint64 size)
{
	LOG_DOWNLOADER ("Downloader::NotifySize (%" G_GINT64_FORMAT ")\n", size);
	
	file_size = size;
	
	if (aborted)
		return;
	
	SetCurrentDeployment ();
	
	if (!GetSurface ())
		return;
	
	if (notify_size)
		notify_size (size, user_data);
}

bool
Downloader::Started ()
{
	LOG_DOWNLOADER ("Downloader::Started (): %i\n", started);
	
	return started;
}

bool
Downloader::Completed ()
{
	LOG_DOWNLOADER ("Downloader::Completed (), filename: %s\n", filename);
	
	return completed;
}

void
Downloader::SetStreamFunctions (DownloaderWriteFunc writer,
				DownloaderNotifySizeFunc notify_size,
				gpointer user_data)
{
	LOG_DOWNLOADER ("Downloader::SetStreamFunctions\n");
	
	this->notify_size = notify_size;
	this->writer = writer;
	this->user_data = user_data;
}

void
Downloader::SetFunctions (DownloaderCreateStateFunc create_state,
			  DownloaderDestroyStateFunc destroy_state,
			  DownloaderOpenFunc open,
			  DownloaderSendFunc send,
			  DownloaderAbortFunc abort,
			  DownloaderHeaderFunc header,
			  DownloaderBodyFunc body,
			  DownloaderCreateWebRequestFunc request,
			  DownloaderSetResponseHeaderCallbackFunc response_header_callback,
			  DownloaderGetResponseFunc get_response)
{
	LOG_DOWNLOADER ("Downloader::SetFunctions\n");
	Downloader::create_state = create_state;
	Downloader::destroy_state = destroy_state;
	Downloader::open_func = open;
	Downloader::send_func = send;
	Downloader::abort_func = abort;
	Downloader::header_func = header;
	Downloader::body_func = body;
	Downloader::request_func = request;
	Downloader::set_response_header_callback_func = response_header_callback;
	Downloader::get_response_func = get_response;
}


/*
 * DownloaderRequest / DownloaderResponse
 */

DownloaderResponse::~DownloaderResponse ()
{
	if (request != NULL && request->GetDownloaderResponse () == this)
		request->SetDownloaderResponse (NULL);
	GetDeployment ()->UnregisterDownloader (this);
}

DownloaderResponse::DownloaderResponse ()
{
	aborted = false;
	started = NULL;
	available = NULL;
	finished = NULL;
	context = NULL;
	request = NULL;
	SetDeployment (Deployment::GetCurrent ());
	GetDeployment ()->RegisterDownloader (this);
}

DownloaderResponse::DownloaderResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context)
{
	this->aborted = false;
	this->started = started;
	this->available = available;
	this->finished = finished;
	this->context = context;
	this->request = NULL;
	SetDeployment (Deployment::GetCurrent ());
	GetDeployment ()->RegisterDownloader (this);
}

DownloaderRequest::DownloaderRequest (const char *method, const char *uri)
{
	this->method = g_strdup (method);
	this->uri = g_strdup (uri);
	this->response = NULL;
	this->aborted = false;
	SetDeployment (Deployment::GetCurrent ());
	GetDeployment ()->RegisterDownloader (this);
}

DownloaderRequest::~DownloaderRequest ()
{
	g_free (method);
	g_free (uri);
	if (response != NULL && response->GetDownloaderRequest () == this)
		response->SetDownloaderRequest (NULL);
	GetDeployment ()->UnregisterDownloader (this);
}

void *
Downloader::CreateWebRequest (const char *method, const char *uri)
{
	return GetRequestFunc () (method, uri, GetContext ());
}

void
Downloader::SetResponseHeaderCallback (DownloaderResponseHeaderCallback callback, gpointer context)
{
	if (set_response_header_callback_func != NULL)
		set_response_header_callback_func (downloader_state, callback, context);
}

DownloaderResponse *
Downloader::GetResponse ()
{
	if (get_response_func != NULL)
		return get_response_func (downloader_state);
	return NULL;
}

void
downloader_write (Downloader *dl, void *buf, gint32 offset, gint32 n)
{
	dl->Write (buf, offset, n);
}

void
downloader_notify_finished (Downloader *dl, const char *fname)
{
	dl->SetFilename (fname);
	dl->NotifyFinished (NULL);
}

void
downloader_notify_error (Downloader *dl, const char *msg)
{
	dl->NotifyFailed (msg);
}

void
downloader_notify_size (Downloader *dl, gint64 size)
{
	dl->NotifySize (size);
}


static gpointer
dummy_downloader_create_state (Downloader* dl)
{
	g_warning ("downloader_set_function has never been called.\n");
	return NULL;
}

static void
dummy_downloader_destroy_state (gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
}

static void
dummy_downloader_open (gpointer state, const char *verb, const char *uri, bool custom_header_support, bool disble_cache)
{
	g_warning ("downloader_set_function has never been called.\n");
}

static void
dummy_downloader_send (gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
}

static void
dummy_downloader_abort (gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
}

static void
dummy_downloader_header (gpointer state, const char *header, const char *value)
{
	g_warning ("downloader_set_function has never been called.\n");
}

static void
dummy_downloader_body (gpointer state, void *body, guint32 length)
{
	g_warning ("downloader_set_function has never been called.\n");
}

static gpointer
dummy_downloader_create_web_request (const char *method, const char *uri, gpointer context)
{
	g_warning ("downloader_set_function has never been called.\n");
	return NULL;
}

static void
dummy_downloader_set_response_header_callback (gpointer state, DownloaderResponseHeaderCallback callback, gpointer context)
{
	g_warning ("downloader_set_function has never been called.\n");
}

static DownloaderResponse *
dummy_downloader_get_response (gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
	return NULL;
}

DownloaderCreateStateFunc Downloader::create_state = dummy_downloader_create_state;
DownloaderDestroyStateFunc Downloader::destroy_state = dummy_downloader_destroy_state;
DownloaderOpenFunc Downloader::open_func = dummy_downloader_open;
DownloaderSendFunc Downloader::send_func = dummy_downloader_send;
DownloaderAbortFunc Downloader::abort_func = dummy_downloader_abort;
DownloaderHeaderFunc Downloader::header_func = dummy_downloader_header;
DownloaderBodyFunc Downloader::body_func = dummy_downloader_body;
DownloaderCreateWebRequestFunc Downloader::request_func = dummy_downloader_create_web_request;
DownloaderSetResponseHeaderCallbackFunc Downloader::set_response_header_callback_func = dummy_downloader_set_response_header_callback;
DownloaderGetResponseFunc Downloader::get_response_func = dummy_downloader_get_response;

void
downloader_init (void)
{
}
