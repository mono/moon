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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include "font.h"
#include "uri.h"

//
// Downloader
//

DownloaderCreateStateFunc Downloader::create_state = NULL;
DownloaderDestroyStateFunc Downloader::destroy_state = NULL;
DownloaderOpenFunc Downloader::open_func = NULL;
DownloaderSendFunc Downloader::send_func = NULL;
DownloaderAbortFunc Downloader::abort_func = NULL;
DownloaderHeaderFunc Downloader::header_func = NULL;
DownloaderBodyFunc Downloader::body_func = NULL;
DownloaderCreateWebRequestFunc Downloader::request_func = NULL;

Downloader::Downloader ()
{
	LOG_DOWNLOADER ("Downloader::Downloader ()\n");
	
	SetObjectType (Type::DOWNLOADER);

	downloader_state = Downloader::create_state (this);
	user_data = NULL;
	context = NULL;
	streaming_features = HttpStreamingFeaturesNone;
	notify_size = NULL;
	writer = NULL;
	internal_dl = NULL;
	
	send_queued = false;
	started = false;
	aborted = false;
	completed = false;
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

	if (internal_dl != NULL)
		delete internal_dl;
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
Downloader::InternalOpen (const char *verb, const char *uri, bool streaming)
{
	LOG_DOWNLOADER ("Downloader::InternalOpen (%s, %s, %i)\n", verb, uri, streaming);

	open_func (verb, uri, streaming, downloader_state);
}

static bool
scheme_is (const Uri *uri, const char *scheme)
{
	return uri->GetScheme () && !strcmp (uri->GetScheme (), scheme);
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
	return (g_ascii_strcasecmp (uri1->GetHost (), uri2->GetHost ()) == 0);
}

static bool
check_redirection_policy (const Uri *uri, const char *final_uri, DownloaderAccessPolicy policy)
{
	if (!uri || !final_uri)
		return true;
	
	Uri *final = new Uri ();
	final->Parse (final_uri);
	char *struri = NULL;

	bool retval = true;
	switch (policy) {
	case DownloaderPolicy:
	case XamlPolicy:
	case StreamingPolicy: //Streaming media
		//Redirection allowed: same domain.
		struri = uri->ToString ();
		if (g_ascii_strcasecmp (struri, final_uri) == 0)
			break;
		if (!same_domain (uri, final))
			retval = false;
		break;
	case MediaPolicy:
		struri = uri->ToString ();
		if (g_ascii_strcasecmp (struri, final_uri) != 0)
			retval = false;
		break;
	default:
		break;
	}

	g_free (struri);
	
	delete final;
	
	return retval;
}

static bool
validate_policy (const char *location, const Uri *source, DownloaderAccessPolicy policy)
{
	if (!location || !source)
		return true;
	
	if (source->GetHost () == NULL) {
		//relative uri, not checking policy
		return true;
	}

	Uri *target = new Uri ();
	target->Parse (location);

	bool retval = true;
	switch (policy) {
	case DownloaderPolicy:
		//Allowed schemes: http, https
		if (!scheme_is (target, "http") && !scheme_is (target, "https"))
			retval = false;
		//X-Scheme: no
		if (!same_scheme (target, source))
			retval = false;
		//X-Domain: no
		if (!same_domain (target, source))
			retval = false;
		break;
	case MediaPolicy: //Media, images, ASX
		//Allowed schemes: http, https, file
		if (!scheme_is (target, "http") && !scheme_is (target, "https") && !scheme_is (target, "file"))
			retval = false;
		//X-Scheme: no
		if (!same_scheme (target, source))
			retval = false;
		//X-Domain: if not https
		if (scheme_is (source, "https") && !same_domain (target, source))
			retval = false;
		break;
	case XamlPolicy: //XAML files, font files
		//Allowed schemes: http, https, file
		if (!scheme_is (target, "http") && !scheme_is (target, "https") && !scheme_is (target, "file"))
			retval = false;
		//X-Scheme: no
		if (!same_scheme (target, source))
			retval =false;
		//X-domain: no
		if (!same_domain (target, source))
			retval = false;
		break;
	case StreamingPolicy: //Streaming media
		//Allowed schemes: http
		if (!scheme_is (target, "http"))
			retval = false;
		//X-scheme: Not from https
		if (scheme_is (source, "https") && !same_scheme (source, target))
			retval = false;
	default:
		break;
	}
	
	delete target;
	
	return retval;
}

void
Downloader::Open (const char *verb, const char *uri, DownloaderAccessPolicy policy)
{
	LOG_DOWNLOADER ("Downloader::Open (%s, %s)\n", verb, uri);
	
	send_queued = false;
	started = false;
	aborted = false;
	completed = false;
	file_size = -2;
	total = 0;
	access_policy = policy;

	g_free (failed_msg);
	g_free (filename);
	g_free (buffer);
	failed_msg = NULL;
	filename = NULL;
	buffer = NULL;
	
	Uri *url = new Uri ();
	if (!url->Parse (uri))
		return;
	
	if (!url->isAbsolute) {
		const char *source_location = NULL;
		source_location = GetDeployment ()->GetXapLocation ();
		if (source_location) {
			if (!url->Parse (source_location))
				return;
			url->Combine (uri);
		}
	}

	//FIXME: ONLY VALIDATE IF USED FROM THE PLUGIN
	char *location;
	(location = g_strdup(GetDeployment ()->GetXapLocation ())) || (location = g_strdup(GetSurface ()->GetSourceLocation ()));
	if (!validate_policy (location, url, policy)) {
		LOG_DOWNLOADER ("aborting due to security policy violation\n");
		failed_msg = g_strdup ("Security Policy Violation");
		Abort ();
		g_free (location);
		delete url;
		return;
	}
	g_free (location);

	if (strncmp (uri, "mms://", 6) == 0) {
		internal_dl = (InternalDownloader *) new MmsDownloader (this);
	} else {
		internal_dl = (InternalDownloader *) new FileDownloader (this);
	}

	send_queued = false;
	
	SetUri (url);
	char *struri = url->ToString ();
	internal_dl->Open (verb, struri);
	g_free (struri);
	delete url;
}

void
Downloader::InternalSetHeader (const char *header, const char *value)
{
	LOG_DOWNLOADER ("Downloader::InternalSetHeader (%s, %s)\n", header, value);
	
	header_func (downloader_state, header, value);
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
		Emit (DownloadFailedEvent, new ErrorEventArgs (DownloadError, 1, failed_msg));
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
	if (internal_dl->GetType () == InternalDownloader::FileDownloader && n == total && total < 65536) {
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
	
	((FileDownloader *)internal_dl)->SetFilename (filename);
}

void
Downloader::NotifyFinished (const char *final_uri)
{
	if (aborted)
		return;
	
	SetCurrentDeployment ();
	
	if (!GetSurface ())
		return;
	
	if (!check_redirection_policy (GetUri (), final_uri, access_policy)) {
		LOG_DOWNLOADER ("aborting due to security policy violation\n");
		failed_msg = g_strdup ("Security Policy Violation");
		Abort ();
		return;
	}

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
	
	Emit (DownloadFailedEvent, new ErrorEventArgs (DownloadError, 1, msg));
	
	// save the error in case someone else calls ::Send() on this
	// downloader for the same uri.
	failed_msg = g_strdup (msg);
}

void
Downloader::NotifySize (gint64 size)
{
	LOG_DOWNLOADER ("Downloader::NotifySize (%lld)\n", size);
	
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
			  bool only_if_not_set)
{
	LOG_DOWNLOADER ("Downloader::SetFunctions\n");
	
	if (only_if_not_set &&
	    (Downloader::create_state != NULL ||
	     Downloader::destroy_state != NULL ||
	     Downloader::open_func != NULL ||
	     Downloader::send_func != NULL ||
	     Downloader::abort_func != NULL ||
	     Downloader::header_func != NULL ||
	     Downloader::body_func != NULL ||
	     Downloader::request_func != NULL))
	  return;

	Downloader::create_state = create_state;
	Downloader::destroy_state = destroy_state;
	Downloader::open_func = open;
	Downloader::send_func = send;
	Downloader::abort_func = abort;
	Downloader::header_func = header;
	Downloader::body_func = body;
	Downloader::request_func = request;
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
dummy_downloader_open (const char *verb, const char *uri, bool open, gpointer state)
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


void
downloader_init (void)
{	
	Downloader::SetFunctions (dummy_downloader_create_state,
				  dummy_downloader_destroy_state,
				  dummy_downloader_open,
				  dummy_downloader_send,
				  dummy_downloader_abort,
				  dummy_downloader_header,
				  dummy_downloader_body,
				  dummy_downloader_create_web_request, true);
}
