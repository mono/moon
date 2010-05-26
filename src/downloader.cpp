/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * downloader.cpp: Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "downloader.h"
#include "deployment.h"
#include "utils.h"
#include "debug.h"
#include "uri.h"

//
// Downloader
//

Downloader::Downloader ()
	: DependencyObject (Type::DOWNLOADER)
{
	LOG_DOWNLOADER ("Downloader::Downloader ()\n");

	request = GetDeployment ()->CreateHttpRequest (HttpRequest::DisableAsyncSend);
	request->AddHandler (HttpRequest::StoppedEvent, StoppedCallback, this);
	request->AddHandler (HttpRequest::ProgressChangedEvent, ProgressChangedCallback, this);
	
	send_queued = false;
	started = false;
	aborted = false;
	completed = false;
	
	filename = NULL;
	failed_msg = NULL;
	unzipdir = NULL;
	unzipped = false;
}

Downloader::~Downloader ()
{
	LOG_DOWNLOADER ("Downloader::~Downloader ()\n");
	
	if (request != NULL) {
		request->RemoveAllHandlers (this);
		request->unref ();
		request = NULL;
	}
	
	g_free (failed_msg);

	CleanupUnzipDir ();

	if (filename)
		g_free (filename);
}

void
Downloader::CleanupUnzipDir ()
{
	if (!unzipdir)
		return;
	
	RemoveDir (unzipdir);
	g_free (unzipdir);
	unzipped = false;
	unzipdir = NULL;
}

bool
Downloader::DownloadedFileIsZipped ()
{
	unzFile zipfile;
	
	if (!filename)
		return false;
	
	if (!(zipfile = unzOpen (filename)))
		return false;
	
	unzClose (zipfile);
	
	return true;
}

void
Downloader::Abort ()
{
	LOG_DOWNLOADER ("Downloader::Abort ()\n");
	
	SetCurrentDeployment ();
	
	if (!aborted && !failed_msg) {
		if (request != NULL) {
			request->RemoveAllHandlers (this);
			request->Abort ();
		}
		SetDownloadProgress (0.0);
		send_queued = false;
		aborted = true;
	}
}

char *
Downloader::GetDownloadedFilename (const char *partname)
{
	LOG_DOWNLOADER ("Downloader::GetDownloadedFilename (%s)\n", filename);
	
	char *dirname, *path, *part;
	unzFile zipfile;
	struct stat st;
	int rv, fd;
	
	if (!filename)
		return NULL;
	
	if (!partname || !partname[0])
		return g_strdup (filename);
	
	if (!DownloadedFileIsZipped ())
		return NULL;
	
	if (!unzipdir && !(unzipdir = CreateTempDir (filename)))
		return NULL;
	
	part = g_ascii_strdown (partname, -1);
	path = g_build_filename (unzipdir, part, NULL);
	if ((rv = g_stat (path, &st)) == -1 && errno == ENOENT) {
		if (strchr (part, '/') != NULL) {
			// create the directory path
			dirname = g_path_get_dirname (path);
			rv = g_mkdir_with_parents (dirname, 0700);
			g_free (dirname);
			
			if (rv == -1 && errno != EEXIST)
				goto exception1;
		}
		
		// open the zip archive...
		if (!(zipfile = unzOpen (filename)))
			goto exception1;
		
		// locate the file we want to extract... (2 = case-insensitive)
		if (unzLocateFile (zipfile, partname, 2) != UNZ_OK)
			goto exception2;
		
		// open the requested part within the zip file
		if (unzOpenCurrentFile (zipfile) != UNZ_OK)
			goto exception2;
		
		// open the output file
		if ((fd = g_open (path, O_CREAT | O_WRONLY | O_TRUNC, 0600)) == -1)
			goto exception3;
		
		// extract the file from the zip archive... (closes the fd on success and fail)
		if (!ExtractFile (zipfile, fd))
			goto exception3;
		
		unzCloseCurrentFile (zipfile);
		unzClose (zipfile);
	} else if (rv == -1) {
		// irrecoverable error
		goto exception0;
	}
	
	g_free (part);
	
	return path;
	
exception3:
	
	unzCloseCurrentFile (zipfile);
	
exception2:
	
	unzClose (zipfile);
	
exception1:
	
	g_free (part);
	
exception0:
	
	g_free (path);
	
	return NULL;
}

const char *
Downloader::GetUnzippedPath ()
{
	char filename[256], *p;
	unz_file_info info;
	const char *name;
	GString *path;
	unzFile zip;
	size_t len;
	int fd;
	
	if (!this->filename)
		return NULL;
	
	if (!DownloadedFileIsZipped ())
		return this->filename;
	
	if (!unzipdir && !(unzipdir = CreateTempDir (this->filename)))
		return NULL;
	
	if (unzipped)
		return unzipdir;
	
	// open the zip archive...
	if (!(zip = unzOpen (this->filename)))
		return NULL;
	
	path = g_string_new (unzipdir);
	g_string_append_c (path, G_DIR_SEPARATOR);
	len = path->len;
	
	unzipped = true;
	
	// extract all the parts
	do {
		if (unzOpenCurrentFile (zip) != UNZ_OK)
			break;
		
		unzGetCurrentFileInfo (zip, &info, filename, sizeof (filename),
				       NULL, 0, NULL, 0);
		
		// convert filename to lowercase
		for (p = filename; *p; p++) {
			if (*p >= 'A' && *p <= 'Z')
				*p += 0x20;
		}
		
		if ((name = strrchr (filename, '/'))) {
			// make sure the full directory path exists, if not create it
			g_string_append_len (path, filename, name - filename);
			g_mkdir_with_parents (path->str, 0700);
			g_string_append (path, name);
		} else {
			g_string_append (path, filename);
		}
		
		if ((fd = g_open (path->str, O_WRONLY | O_CREAT | O_EXCL, 0600)) != -1) {
			if (!ExtractFile (zip, fd))
				unzipped = false;
		} else if (errno != EEXIST) {
			unzipped = false;
		}
		
		g_string_truncate (path, len);
		unzCloseCurrentFile (zip);
	} while (unzGoToNextFile (zip) == UNZ_OK);
	
	g_string_free (path, true);
	unzClose (zip);
	
	return unzipdir;
}

char *
Downloader::GetResponseText (const char *partname, gint64 *size)
{
	LOG_DOWNLOADER ("Downloader::GetResponseText (%s, %p)\n", partname, size);

	TextStream *stream;
	char buffer[4096];
	GByteArray *buf;
	struct stat st;
	ssize_t nread;
	char *data;
	char *path;
	
	if (!(path = GetDownloadedFilename (partname)))
		return NULL;
	
	if (g_stat (path, &st) == -1) {
		g_free (path);
		return NULL;
	}
	
	if (st.st_size > 0) {
		stream = new TextStream ();
		
		if (!stream->OpenFile (path, true)) {
			delete stream;
			g_free (path);
			return NULL;
		}
		
		g_free (path);
		
		buf = g_byte_array_new ();
		while ((nread = stream->Read (buffer, sizeof (buffer))) > 0)
			g_byte_array_append (buf, (const guint8 *) buffer, nread);
		
		*size = buf->len;
		
		g_byte_array_append (buf, (const guint8 *) "", 1);
		data = (char *) buf->data;
		
		g_byte_array_free (buf, false);
		delete stream;
	} else {
		data = g_strdup ("");
		*size = 0;
	}
	
	return data;
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
		if (!Uri::SameScheme (target, source))
			retval = false;
		//X-Domain: requires policy file
		// FIXME only managed is implemented
		if (!Uri::SameDomain (target, source))
			retval = false;
		break;
	case MsiPolicy:
	case MediaPolicy: //Media, images, ASX
		//Allowed schemes: http, https, file
		if (!target->IsScheme ("http") && !target->IsScheme ("https") && !target->IsScheme ("file"))
			retval = false;
		//X-Scheme: no
		if (!Uri::SameScheme (target, source))
			retval = false;
		//X-Domain: Allowed
		break;
	case XamlPolicy:
		//Allowed schemes: http, https, file
		if (!target->IsScheme ("http") && !target->IsScheme ("https") && !target->IsScheme ("file"))
			retval = false;
		//X-Scheme: no
		if (!Uri::SameScheme (target, source))
			retval =false;
		//X-domain: allowed if not HTTPS to HTTPS
		if (!Uri::SameDomain (target, source) && target->IsScheme ("https") && source->IsScheme ("https"))
			retval = false;
		break;
	case FontPolicy:
		//Allowed schemes: http, https, file
		if (!target->IsScheme ("http") && !target->IsScheme ("https") && !target->IsScheme ("file"))
			retval = false;
		//X-Scheme: no
		if (!Uri::SameScheme (target, source))
			retval = false;
		//X-domain: no
		if (!Uri::SameDomain (target, source))
			retval = false;
		break;
	case StreamingPolicy: //Streaming media
		//Allowed schemes: http
		if (!target->IsScheme ("http"))
			retval = false;
		//X-scheme: Not from https
		if (source->IsScheme ("https") && !Uri::SameScheme (source, target))
			retval = false;
		//X-domain: allowed if not HTTPS to HTTPS
		if (!Uri::SameDomain (target, source) && target->IsScheme ("https") && source->IsScheme ("https"))
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
	CleanupUnzipDir ();
	unzipped = false;
	send_queued = false;
	started = false;
	aborted = false;
	completed = false;

	g_free (failed_msg);
	g_free (filename);
	failed_msg = NULL;
	filename = NULL;
}

void
Downloader::Open (const char *verb, const char *uri, DownloaderAccessPolicy policy)
{
	LOG_DOWNLOADER ("Downloader::Open (%s, %s)\n", verb, uri);
	
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

	LOG_DOWNLOADER ("Downloader::Open (%s, %p)\n", verb, uri);
	
	OpenInitialize ();
	
	if (!(source_location = GetDeployment ()->GetXapLocation ()))
		source_location = GetDeployment ()->GetSurface ()->GetSourceLocation ();
	
	request->Open (verb, url, policy);
	if (failed_msg == NULL)
		SetUri (uri);

	delete src_uri;
}

void
Downloader::SendInternal ()
{
	LOG_DOWNLOADER ("Downloader::SendInternal ()\n");
	
	g_return_if_fail (request != NULL);

	if (!send_queued)
		return;
	
	send_queued = false;
	
	if (completed) {
		// Consumer is re-sending a request which finished successfully.
		NotifyFinished ();
		return;
	}
	
	if (failed_msg != NULL) {
		if (HasHandlers (DownloadFailedEvent)) {
			// Consumer is re-sending a request which failed.
			Emit (DownloadFailedEvent, new ErrorEventArgs (DownloadError,
								       MoonError (MoonError::EXCEPTION, 4001, failed_msg)));
		}
		return;
	}
	
	started = true;
	aborted = false;
	
	g_return_if_fail (request != NULL);

	request->Send ();
}

void
Downloader::SendAsync (EventObject *user_data)
{
	Downloader *downloader = (Downloader *) user_data;
	
	downloader->SendInternal ();
}

void
Downloader::Send ()
{
	LOG_DOWNLOADER ("Downloader::Send ()\n");

	if (send_queued)
		return;
	
	send_queued = true;
	SetStatusText ("");
	SetStatus (0);
	
	AddTickCall (SendAsync);
}

void
Downloader::ProgressChangedHandler (HttpRequest *request, HttpRequestProgressChangedEventArgs *args)
{
	SetDownloadProgress (args->GetProgress ());
	Emit (DownloadProgressChangedEvent);
}

void
Downloader::SetFilename (const char *fname)
{
	LOG_DOWNLOADER ("Downloader::SetFilename (%s)\n", fname);
	
	g_free (filename);

	filename = g_strdup (fname);
}

void
Downloader::NotifyFinished ()
{
	if (aborted)
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
	
	// SetStatus (400);
	// For some reason the status is 0, not updated on errors?
	
	if (HasHandlers (DownloadFailedEvent))
		Emit (DownloadFailedEvent, new ErrorEventArgs (DownloadError,
							       MoonError (MoonError::EXCEPTION, 4001, msg)));
	
	failed_msg = g_strdup (msg);
}

void
Downloader::StoppedHandler (HttpRequest *sender, HttpRequestStoppedEventArgs *args)
{
	SetFilename (sender->GetFilename ());
	if (args->IsSuccess ()) {
		NotifyFinished ();
	} else {
		NotifyFailed (args->GetErrorMessage ());
	}
}
