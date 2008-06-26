/*
 * runtime.cpp: Downloader class.
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

#define d(x)

//
// Downloader
//

downloader_create_state_func Downloader::create_state = NULL;
downloader_destroy_state_func Downloader::destroy_state = NULL;
downloader_open_func Downloader::open_func = NULL;
downloader_send_func Downloader::send_func = NULL;
downloader_abort_func Downloader::abort_func = NULL;
downloader_header_func Downloader::header_func = NULL;
downloader_body_func Downloader::body_func = NULL;

DependencyProperty *Downloader::DownloadProgressProperty;
DependencyProperty *Downloader::ResponseTextProperty;
DependencyProperty *Downloader::StatusProperty;
DependencyProperty *Downloader::StatusTextProperty;
DependencyProperty *Downloader::UriProperty;

Downloader::Downloader ()
{
	d (printf ("Downloader::Downloader ()\n"));
	
	downloader_state = Downloader::create_state (this);
	consumer_closure = NULL;
	context = NULL;
	streaming_features = HttpStreamingFeaturesNone;
	notify_size = NULL;
	this->write = NULL;
	request_position = NULL;
	internal_dl = NULL;
	
	send_queued = false;
	started = false;
	aborted = false;
	file_size = -2;
	total = 0;
	
	filename = NULL;
	failed_msg = NULL;
}


Downloader::~Downloader ()
{
	d (printf ("Downloader::~Downloader ()\n"));
	
	Downloader::destroy_state (downloader_state);
	
	g_free (filename);
	g_free (failed_msg);

	if (internal_dl != NULL)
		delete internal_dl;
}

void
Downloader::InternalAbort ()
{
	d (printf ("Downloader::InternalAbort ()\n"));
	if (!GetSurface ())
		return;

	abort_func (downloader_state);
}

void
Downloader::Abort ()
{
	d (printf ("Downloader::Abort ()\n"));
	
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
	d (printf ("Downloader::GetDownloadedFilename (%s)\n", filename));
	
	return internal_dl->GetDownloadedFilename (partname);
}

char *
Downloader::GetResponseText (const char *PartName, guint64 *size)
{
	d (printf ("Downloader::GetResponseText (%s, %p)\n", PartName, size));
	
	return internal_dl->GetResponseText (PartName, size);
}

void
Downloader::InternalOpen (const char *verb, const char *uri, bool streaming)
{
	d (printf ("Downloader::InternalOpen (%s, %s, %i)\n", verb, uri, streaming));
	
	open_func (verb, uri, streaming, downloader_state);
}

void
Downloader::Open (const char *verb, const char *uri)
{
	d (printf ("Downloader::Open (%s, %s)\n", verb, uri));
	
	send_queued = false;
	started = false;
	aborted = false;
	file_size = -2;
	total = 0;

	g_free (failed_msg);
	failed_msg = NULL;
	filename = NULL;

	if (strncmp (uri, "mms://", 6) == 0) {
		internal_dl = (InternalDownloader *) new MmsDownloader (this);
	} else {
		internal_dl = (InternalDownloader *) new FileDownloader (this);
	}

	send_queued = false;

	SetUri (uri);

	internal_dl->Open (verb, uri);
}

void
Downloader::InternalSetHeader (const char *header, const char *value)
{
	d (printf ("Downloader::InternalSetHeader (%s, %s)\n", header, value));
	
	header_func (downloader_state, header, value);
}

void
Downloader::InternalSetBody (void *body, guint32 length)
{
	d (printf ("Downloader::InternalSetBody (%p, %u)\n", body, length));
	
	body_func (downloader_state, body, length);
}

void
Downloader::SendInternal ()
{
	d (printf ("Downloader::SendInternal ()\n"));
	
	if (!GetSurface ()) {
		// The plugin is already checking for surface before calling Send, so
		// if we get here, it's either managed code doing something wrong or ourselves.
		g_warning ("Downloader::SendInternal (): No surface!\n");
	}

	if (!send_queued)
		return;
	
	send_queued = false;
	
	if (filename != NULL) {
		// Consumer is re-sending a request which finished successfully.
		NotifyFinished (filename);
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
	downloader->unref ();
}

void
Downloader::Send ()
{
	d (printf ("Downloader::Send ()\n"));
	
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
	d (printf ("Downloader::SendNow ()\n"));
	
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
	d (printf ("Downloader::Write (%p, %i, %i). Uri: %s\n", buf, offset, n, GetUri ()));
	
	if (aborted)
		return;
		
	if (!GetSurface ())
		return;
	
	internal_dl->Write (buf, offset, n);
}

void
Downloader::InternalWrite (void *buf, gint32 offset, gint32 n)
{
	d (printf ("Downloader::InternalWrite (%p, %i, %i)\n", buf, offset, n));
	
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

	if (write)
		write (buf, offset, n, consumer_closure);
}

void
Downloader::RequestPosition (gint64 *pos)
{
	d (printf ("Downloader::RequestPosition (%p)\n", pos));
	
	if (aborted)
		return;
	
	if (request_position)
		request_position (pos, consumer_closure);
}


void
Downloader::NotifyFinished (const char *fname)
{
	d (printf ("Downloader::NotifyFinished (%s)\n", fname));
	
	if (aborted)
		return;
	
	if (!GetSurface ())
		return;
	
	filename = g_strdup (fname);
	((FileDownloader *)internal_dl)->setFilename (filename);
	
	SetDownloadProgress (1.0);
	
	Emit (DownloadProgressChangedEvent);
	
	// HACK, we should provide the actual status text and code
	SetStatusText ("OK");
	SetStatus (200);
	
	Emit (CompletedEvent, NULL);
}

void
Downloader::NotifyFailed (const char *msg)
{
	d (printf ("Downloader::NotifyFailed (%s)\n", msg));
	
	/* if we've already been notified of failure, no-op */
	if (failed_msg)
		return;
	
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
	d (printf ("Downloader::NotifySize (%lld)\n", size));
	
	file_size = size;
	
	if (aborted)
		return;
	
	if (!GetSurface ())
		return;
	
	if (notify_size)
		notify_size (size, consumer_closure);
}

bool
Downloader::Started ()
{
	d (printf ("Downloader::Started (): %i\n", started));
	
	return started;
}

bool
Downloader::Completed ()
{
	d (printf ("Downloader::Completed (), filename: %s\n", filename));
	
	return filename != NULL;
}

void
Downloader::SetWriteFunc (downloader_write_func write,
			  downloader_notify_size_func notify_size,
			  gpointer data)
{
	d (printf ("Downloader::SetWriteFunc\n"));
	
	this->write = write;
	this->notify_size = notify_size;
	this->consumer_closure = data;
}

void
Downloader::SetFunctions (downloader_create_state_func create_state,
			  downloader_destroy_state_func destroy_state,
			  downloader_open_func open,
			  downloader_send_func send,
			  downloader_abort_func abort,
			  downloader_header_func header,
			  downloader_body_func body,
			  bool only_if_not_set)
{
	d (printf ("Downloader::SetFunctions\n"));
	
	if (only_if_not_set &&
	    (Downloader::create_state != NULL ||
	     Downloader::destroy_state != NULL ||
	     Downloader::open_func != NULL ||
	     Downloader::send_func != NULL ||
	     Downloader::abort_func != NULL ||
	     Downloader::header_func != NULL ||
	     Downloader::body_func != NULL))
	  return;

	Downloader::create_state = create_state;
	Downloader::destroy_state = destroy_state;
	Downloader::open_func = open;
	Downloader::send_func = send;
	Downloader::abort_func = abort;
	Downloader::header_func = header;
	Downloader::body_func = body;
}

void
Downloader::SetRequestPositionFunc (downloader_request_position_func request_position)
{
	d (printf ("Downloader::SetRequestPositionFunc\n"));
	
	this->request_position = request_position;
}

void
Downloader::SetDownloadProgress (double progress)
{
	d (printf ("Downloader::SetDownloadProgress\n"));
	
	SetValue (Downloader::DownloadProgressProperty, Value (progress));
}

double
Downloader::GetDownloadProgress ()
{
	d (printf ("Downloader::GetDownloadProgress\n"));
	
	return GetValue (Downloader::DownloadProgressProperty)->AsDouble ();
}

void
Downloader::SetStatusText (const char *text)
{
	d (printf ("Downloader::SetStatusText\n"));
	
	SetValue (Downloader::StatusTextProperty, Value (text));
}

const char *
Downloader::GetStatusText ()
{
	d (printf ("Downloader::GetStatusText\n"));
	
	Value *value = GetValue (Downloader::StatusTextProperty);
	
	return value ? value->AsString () : NULL;
}

void
Downloader::SetStatus (int status)
{
	d (printf ("Downloader::SetStatus\n"));
	
	SetValue (Downloader::StatusProperty, Value (status));
}

int
Downloader::GetStatus ()
{
	d (printf ("Downloader::GetStatus\n"));
	
	return GetValue (Downloader::StatusProperty)->AsInt32 ();
}

void
Downloader::SetUri (const char *uri)
{
	d (printf ("Downloader::SetUri (%s)\n", uri));
	
	SetValue (Downloader::UriProperty, Value (uri));

}

const char *
Downloader::GetUri ()
{
	d (printf ("Downloader::GetUri ()\n"));
	
	Value *value = GetValue (Downloader::UriProperty);
	
	return value ? value->AsString () : NULL;
}

Downloader *
downloader_new (void)
{
	return new Downloader ();
}

double
downloader_get_download_progress (Downloader *dl)
{
	return dl->GetDownloadProgress ();
}

const char *
downloader_get_status_text (Downloader *dl)
{
	return dl->GetStatusText ();
}

int
downloader_get_status (Downloader *dl)
{
	return dl->GetStatus ();
}

void
downloader_set_uri (Downloader *dl, const char *uri)
{
	dl->SetUri (uri);
}

const char *
downloader_get_uri (Downloader *dl)
{
	return dl->GetUri ();
}

void
downloader_abort (Downloader *dl)
{
	dl->Abort ();
}

char *
downloader_get_response_text (Downloader *dl, const char *PartName, guint64 *size)
{
	return dl->GetResponseText (PartName, size);
}

void
downloader_open (Downloader *dl, const char *verb, const char *uri)
{
	dl->Open (verb, uri);
}

void
downloader_send (Downloader *dl)
{
	if (!dl->Completed () && dl->Started ())
		downloader_abort (dl);
	
	dl->Send ();
}

void
downloader_set_functions (downloader_create_state_func create_state,
			  downloader_destroy_state_func destroy_state,
			  downloader_open_func open,
			  downloader_send_func send,
			  downloader_abort_func abort, 
			  downloader_header_func header,
			  downloader_body_func body)
{
	Downloader::SetFunctions (create_state, destroy_state,
				  open, send, abort, header, body, false);
}

void
downloader_request_position (Downloader *dl, gint64 *pos)
{
	dl->RequestPosition (pos);
}

void
downloader_write (Downloader *dl, void *buf, gint32 offset, gint32 n)
{
	dl->Write (buf, offset, n);
}

void
downloader_notify_finished (Downloader *dl, const char *fname)
{
	dl->NotifyFinished (fname);
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


void
downloader_init (void)
{
	Downloader::DownloadProgressProperty = DependencyObject::Register (Type::DOWNLOADER, "DownloadProgress", new Value (0.0));
	Downloader::ResponseTextProperty = DependencyObject::Register (Type::DOWNLOADER, "ResponseText", Type::STRING);
	Downloader::StatusProperty = DependencyObject::Register (Type::DOWNLOADER, "Status", new Value (0));
	Downloader::StatusTextProperty = DependencyObject::Register (Type::DOWNLOADER, "StatusText", new Value (""));
	Downloader::UriProperty = DependencyObject::Register (Type::DOWNLOADER, "Uri", Type::STRING);
		
	Downloader::SetFunctions (dummy_downloader_create_state,
				  dummy_downloader_destroy_state,
				  dummy_downloader_open,
				  dummy_downloader_send,
				  dummy_downloader_abort,
				  dummy_downloader_header,
				  dummy_downloader_body, true);
}
