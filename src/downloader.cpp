/*
 * runtime.cpp: Downloader class.
 *
 * The downloader implements two modes of operation:
 *
 *    bare bones:  this is the interface expected by Javascript and C#
 *                 this is the default if the caller does not call
 *                 Downloader::SetWriteFunc
 * 
 *    progressive: this interface is used internally by the Image
 *                 class to do progressive loading.   If you want to
 *                 use this mode, you must call the SetWriteFunc routine
 *                 to install your callbacks before starting the download.
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
 * Author:
 *   Chris Toshok (toshok@novell.com)
 *   Miguel de Icaza (miguel@novell.com).
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "downloader.h"
#include "zip/unzip.h"
#include "runtime.h"
#include "utils.h"
#include "error.h"

//
// Downloader
//

downloader_create_state_func Downloader::create_state = NULL;
downloader_destroy_state_func Downloader::destroy_state = NULL;
downloader_open_func Downloader::open_func = NULL;
downloader_send_func Downloader::send_func = NULL;
downloader_abort_func Downloader::abort_func = NULL;

DependencyProperty *Downloader::DownloadProgressProperty;
DependencyProperty *Downloader::ResponseTextProperty;
DependencyProperty *Downloader::StatusProperty;
DependencyProperty *Downloader::StatusTextProperty;
DependencyProperty *Downloader::UriProperty;

int Downloader::CompletedEvent = -1;
int Downloader::DownloadProgressChangedEvent = -1;
int Downloader::DownloadFailedEvent = -1;


Downloader::Downloader ()
{
	downloader_state = Downloader::create_state (this);
	consumer_closure = NULL;
	context = NULL;
	notify_size = NULL;
	deobfuscated = false;
	unlinkit = false;
	filename = NULL;
	unzipdir = NULL;
	failed_msg = NULL;
	send_queued = false;
	unzipped = false;
	started = false;
	aborted = false;
	this->write = NULL;
	file_size = -2;
	total = 0;
}

void
Downloader::CleanupUnzipDir ()
{
	if (!unzipdir)
		return;
	
	moon_rmdir (unzipdir);
	g_free (unzipdir);
	unzipped = false;
	unzipdir = NULL;
}

Downloader::~Downloader ()
{
	Downloader::destroy_state (downloader_state);
	
	if (filename) {
		if (unlinkit)
			unlink (filename);
		g_free (filename);
	}
	
	g_free (failed_msg);
	
	// Delete temporary files.
	CleanupUnzipDir ();
}

void
Downloader::Abort ()
{
	if (!aborted && !failed_msg) {
		abort_func (downloader_state);
		send_queued = false;
		aborted = true;
	}
}

char *
Downloader::GetResponseText (const char *PartName, uint64_t *size)
{
	TextStream *stream;
	char buffer[4096];
	GByteArray *buf;
	struct stat st;
	ssize_t nread;
	char *data;
	char *path;
	
	if (!(path = GetDownloadedFilePart (PartName)))
		return NULL;
	
	if (stat (path, &st) == -1) {
		g_free (path);
		return NULL;
	}
	
	if (st.st_size > 0) {
		stream = new TextStream ();
		
		if (!stream->Open (path, true)) {
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

const char *
Downloader::GetDownloadedFile ()
{
	return filename;
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

static char *
create_unzipdir (const char *filename)
{
	const char *name;
	char *path, *buf;
	
	// create an unzip directory in /tmp
	if (!(name = strrchr (filename, '/')))
		name = filename;
	else
		name++;
	
	buf = g_strdup_printf ("%s.XXXXXX", name);
	path = g_build_filename (g_get_tmp_dir (), buf, NULL);
	g_free (buf);
	
	if (!make_tmpdir (path)) {
		g_free (path);
		return NULL;
	}
	
	return path;
}

char *
Downloader::GetDownloadedFilePart (const char *PartName)
{
	char *dirname, *path;
	unzFile zipfile;
	struct stat st;
	int rv, fd;
	
	if (!filename)
		return NULL;
	
	if (!PartName || !PartName[0])
		return g_strdup (filename);
	
	if (!DownloadedFileIsZipped ())
		return NULL;
	
	if (!unzipdir && !(unzipdir = create_unzipdir (filename)))
		return NULL;
	
	path = g_build_filename (unzipdir, PartName, NULL);
	if ((rv = stat (path, &st)) == -1 && errno == ENOENT) {
		if (strchr (PartName, '/') != NULL) {
			// create the directory path
			dirname = g_path_get_dirname (path);
			rv = g_mkdir_with_parents (dirname, 0700);
			g_free (dirname);
			
			if (rv == -1 && errno != EEXIST) {
				g_free (path);
				return NULL;
			}
		}
		
		// open the zip archive...
		if (!(zipfile = unzOpen (filename))) {
			g_free (path);
			return NULL;
		}
		
		// locate the file we want to extract...
		if (unzLocateFile (zipfile, PartName, 0) != UNZ_OK) {
			unzClose (zipfile);
			g_free (path);
			return NULL;
		}
		
		// open the requested part within the zip file
		if (unzOpenCurrentFile (zipfile) != UNZ_OK) {
			unzClose (zipfile);
			g_free (path);
			return NULL;
		}
		
		// open the output file
		if ((fd = open (path, O_CREAT | O_WRONLY, 0644)) == -1) {
			unzCloseCurrentFile (zipfile);
			unzClose (zipfile);
			g_free (path);
			return NULL;
		}
		
		// extract the file from the zip archive... (closes the fd on success and fail)
		if (!ExtractFile (zipfile, fd)) {
			unzCloseCurrentFile (zipfile);
			unzClose (zipfile);
			g_free (path);
			return NULL;
		}
		
		unzCloseCurrentFile (zipfile);
		unzClose (zipfile);
	} else if (rv == -1) {
		// irrecoverable error
		g_free (path);
		return NULL;
	}
	
	return path;
}

const char *
Downloader::GetUnzippedPath ()
{
	char filename[256];
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
	
	if (!unzipdir && !(unzipdir = create_unzipdir (this->filename)))
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
		
		if ((name = strrchr (filename, '/'))) {
			// make sure the full directory path exists, if not create it
			g_string_append_len (path, filename, name - filename);
			g_mkdir_with_parents (path->str, 0700);
			g_string_append (path, name);
		} else {
			g_string_append (path, filename);
		}
		
		if ((fd = open (path->str, O_WRONLY | O_CREAT | O_EXCL, 0600)) != -1) {
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

bool
Downloader::IsDeobfuscated ()
{
	return deobfuscated;
}

void
Downloader::SetDeobfuscated (bool val)
{
	deobfuscated = val;
}

void
Downloader::SetDeobfuscatedFile (const char *path)
{
	if (!filename || !path)
		return;
	
	if (deobfuscated)
		unlink (filename);
	g_free (filename);
	
	filename = g_strdup (path);
	deobfuscated = true;
	unlinkit = true;
}

void
Downloader::Open (const char *verb, const char *uri)
{
	CleanupUnzipDir ();
	
	started = false;
	g_free (failed_msg);
	
	if (filename) {
		if (unlinkit)
			unlink (filename);
		g_free (filename);
	}
	
	deobfuscated = false;
	send_queued = false;
	unlinkit = false;
	
	failed_msg = NULL;
	filename = NULL;
	
	SetValue (Downloader::UriProperty, Value (uri));
	open_func (verb, uri, downloader_state);
}

void
Downloader::SendInternal ()
{
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
send_async (void *user_data)
{
	Downloader *downloader = (Downloader *) user_data;
	
	downloader->SendInternal ();
	downloader->unref ();
}

void
Downloader::Send ()
{
	Surface *surface;
	
	if (send_queued)
		return;
	
	if ((surface = GetSurface ())) {
		TimeManager *tm = surface->GetTimeManager ();
		
		tm->AddTickCall (send_async, this);
		send_queued = true;
		ref ();
	} else {
		// Not attached to a surface?
		send_queued = true;
		SendInternal ();
	}
}

//
// A zero write means that we are done
//
void
Downloader::Write (void *buf, int32_t offset, int32_t n)
{
	double progress;

	if (aborted)
		return;
	
	// Update progress
	if (n > 0)
		total += n;
	
	if (file_size >= 0)
		progress = total / (double) file_size;
	else 
		progress = 0;
	
	
	SetValue (Downloader::DownloadProgressProperty, Value (progress));
	Emit (DownloadProgressChangedEvent);
	
	if (write)
		write (buf, offset, n, consumer_closure);
}

void
Downloader::NotifyFinished (const char *fname)
{
	if (aborted)
		return;
		
	filename = g_strdup (fname);
	
	SetValue (Downloader::DownloadProgressProperty, Value (1.0));
	Emit (DownloadProgressChangedEvent);
	
	// HACK, we should provide the status code
	SetValue (Downloader::StatusProperty, Value (200));
	Emit (CompletedEvent, NULL);
}

void
Downloader::NotifyFailed (const char *msg)
{
	/* if we've already been notified of failure, no-op */
	if (failed_msg)
		return;
	
	// dl->SetValue (Downloader::StatusProperty, Value (400))
	// For some reason the status is 0, not updated on errors?
	
	Emit (DownloadFailedEvent, new ErrorEventArgs (DownloadError, 1, msg));
	
	// save the error in case someone else calls ::Send() on this
	// downloader for the same uri.
	failed_msg = g_strdup (msg);
}

void
Downloader::NotifySize (int64_t size)
{
	file_size = size;
	
	if (aborted)
		return;
	
	if (notify_size)
		notify_size (size, consumer_closure);
	
	SetValue (Downloader::DownloadProgressProperty, Value (0.0));
	
	Emit (DownloadProgressChangedEvent);
}

bool
Downloader::Started ()
{
	return started;
}

bool
Downloader::Completed ()
{
	return filename != NULL;
}

void
Downloader::SetWriteFunc (downloader_write_func write,
			  downloader_notify_size_func notify_size,
			  gpointer data)
{
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
			  bool only_if_not_set)
{
	if (only_if_not_set &&
	    (Downloader::create_state != NULL ||
	     Downloader::destroy_state != NULL ||
	     Downloader::open_func != NULL ||
	     Downloader::send_func != NULL ||
	     Downloader::abort_func != NULL))
	  return;

	Downloader::create_state = create_state;
	Downloader::destroy_state = destroy_state;
	Downloader::open_func = open;
	Downloader::send_func = send;
	Downloader::abort_func = abort;
}


void
downloader_abort (Downloader *dl)
{
	dl->Abort ();
}

//
// Returns the filename that holds that given file.
// 
// Return values:
//   A newly allocated string containing the filename.
//
char *
downloader_get_response_file (Downloader *dl, const char *PartName)
{
	return dl->GetDownloadedFilePart (PartName);
}


void *
downloader_get_response_text (Downloader *dl, const char *PartName, uint64_t *size)
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

Downloader *
downloader_new (void)
{
	return new Downloader ();
}

void
downloader_set_functions (downloader_create_state_func create_state,
			  downloader_destroy_state_func destroy_state,
			  downloader_open_func open,
			  downloader_send_func send,
			  downloader_abort_func abort)
{
	Downloader::SetFunctions (create_state, destroy_state,
				  open, send, abort, false);
}

void
downloader_write (Downloader *dl, void *buf, int32_t offset, int32_t n)
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
downloader_notify_size (Downloader *dl, int64_t size)
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
dummy_downloader_open (const char *verb, const char *uri, gpointer state)
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


void
downloader_init (void)
{
	Downloader::DownloadProgressProperty = DependencyObject::Register (Type::DOWNLOADER, "DownloadProgress", new Value (0.0));
	Downloader::ResponseTextProperty = DependencyObject::Register (Type::DOWNLOADER, "ResponseText", Type::STRING);
	Downloader::StatusProperty = DependencyObject::Register (Type::DOWNLOADER, "Status", new Value (0));
	Downloader::StatusTextProperty = DependencyObject::Register (Type::DOWNLOADER, "StatusText", Type::STRING);
	Downloader::UriProperty = DependencyObject::Register (Type::DOWNLOADER, "Uri", Type::STRING);
	
	/* lookup events */
	Type *t = Type::Find (Type::DOWNLOADER);
	Downloader::CompletedEvent               = t->LookupEvent ("Completed");
	Downloader::DownloadProgressChangedEvent = t->LookupEvent ("DownloadProgressChanged");
	Downloader::DownloadFailedEvent          = t->LookupEvent ("DownloadFailed");
	
	Downloader::SetFunctions (dummy_downloader_create_state,
				  dummy_downloader_destroy_state,
				  dummy_downloader_open,
				  dummy_downloader_send,
				  dummy_downloader_abort, true);
}
