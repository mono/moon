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
#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "downloader.h"
#include "zip/unzip.h"

//
// Downloader
//

downloader_create_state_func Downloader::create_state = NULL;
downloader_destroy_state_func Downloader::destroy_state = NULL;
downloader_open_func Downloader::open_func = NULL;
downloader_send_func Downloader::send_func = NULL;
downloader_abort_func Downloader::abort_func = NULL;

int Downloader::CompletedEvent = -1;
int Downloader::DownloadProgressChangedEvent = -1;
int Downloader::DownloadFailedEvent = -1;

Downloader::Downloader ()
{
	surface = NULL;
	downloader_state = Downloader::create_state (this);
	notify_size = NULL;
	filename = NULL;
	failed_msg = NULL;
	started = false;
	aborted = false;
	this->write = NULL;
	file_size = -2;
	total = 0;
	part_hash = NULL;
	context = NULL;
	consumer_closure = NULL;
}

static void 
delete_file (gpointer key, gpointer value, gpointer user_data)
{
	unlink ((char *) value);
}

Downloader::~Downloader ()
{
	Downloader::destroy_state (downloader_state);

	g_free (filename);
	g_free (failed_msg);

	// Delete temporary files.
	if (part_hash != NULL){
		g_hash_table_foreach (part_hash, delete_file, NULL);
		g_hash_table_destroy (part_hash);
	}
}

void
Downloader::Abort ()
{
	if (!aborted && !failed_msg) {
		abort_func (downloader_state);
		aborted = true;
	}
}

void *
Downloader::GetResponseText (const char *PartName, uint64_t *size)
{
	FILE *f = NULL;
	struct stat buf;
	long n = 0;
	void *data = NULL;

	char *fname = GetResponseFile (PartName);
	if (fname == NULL)
		return NULL;

	if (stat (fname, &buf) == -1)
		goto leave_error;
	
	// 
	// Must use g_malloc here, because the C# code will call
	// g_free on that
	//
	data = g_try_malloc (buf.st_size);
	if (data == NULL)
		goto leave_error;

	f = fopen (fname, "r");
	if (f == NULL)
		goto leave_error;

	n = fread (data, 1, buf.st_size, f);
	*size = n;
	
	fclose (f);
	return data;

 leave_error:
	g_free (fname);
	if (data != NULL)
		g_free (data);
	if (f != NULL)
		fclose (f);
	return NULL;
}

char *
Downloader::ll_downloader_get_response_file (const char *PartName)
{
	char buffer[32 * 1024];
	char *tmpname = NULL;
	FILE *fp;
	int fd;
	
	if (filename == NULL)
		return NULL;

	// Null or empty, get the original file.
	if (PartName == NULL || *PartName == 0)
		return g_strdup (filename);

	//
	// Open zip file
	//
	unzFile zipfile = unzOpen (filename);
	if (zipfile == NULL)
		return NULL;

	if (unzLocateFile (zipfile, PartName, 0) != UNZ_OK)
		goto leave;

	if (unzOpenCurrentFile (zipfile) != UNZ_OK)
		goto leave;

	// 
	// Create the file where the content is extracted
	//
	tmpname = g_build_filename (g_get_tmp_dir (), "MoonlightDownloaderStream.XXXXXX", NULL);
	if ((fd = g_mkstemp (tmpname)) == -1) {
		g_free (tmpname);
		tmpname = NULL;
		goto leave1;
	}
	
	if (!(fp = fdopen (fd, "w"))) {
		unlink (tmpname);
		g_free (tmpname);
		tmpname = NULL;
		close (fd);
		goto leave1;
	}
	
	int n;
	do {
		n = unzReadCurrentFile (zipfile, buffer, sizeof (buffer));
		if (n < 0) {
			unlink (tmpname);
			g_free (tmpname);
			tmpname = NULL;
			goto leave2;
		}
		
		if (n != 0 && fwrite (buffer, n, 1, fp) != 1) {
			unlink (tmpname);
			g_free (tmpname);
			tmpname = NULL;
			goto leave2;
		}
	} while (n > 0);
	
leave2:
	fclose (fp);
	
leave1:
	unzCloseCurrentFile (zipfile);
	
leave:
	unzClose (zipfile);
	
	return tmpname;
}

char *
Downloader::GetResponseFile (const char *PartName)
{
	if (part_hash != NULL){
		char *fname = (char*) g_hash_table_lookup (part_hash, PartName);
		if (fname != NULL)
			return g_strdup (fname);
	}

	char *part = ll_downloader_get_response_file (PartName);
	if (part != NULL && PartName != NULL && *PartName != 0){
		if (part_hash == NULL)
			part_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
		g_hash_table_insert (part_hash, g_strdup (PartName), g_strdup (part));
	}
	return part;
}

void
Downloader::Open (const char *verb, const char *uri)
{
	started = false;
	g_free (failed_msg);
	g_free (filename);
	failed_msg = NULL;
	filename = NULL;
	
	SetValue (Downloader::UriProperty, Value (uri));
	open_func (verb, uri, downloader_state);
}

void
Downloader::Send ()
{
	if (filename != NULL) {
		// Consumer is re-sending a request which finished successfully.
		NotifyFinished (filename);
		return;
	}
	
	if (failed_msg != NULL) {
		// Consumer is re-sending a request which failed.
		Emit (DownloadFailedEvent, failed_msg);
		return;
	}
	
	started = true;
	aborted = false;
	
	send_func (downloader_state);
}

//
// A zero write means that we are done
//
void
Downloader::Write (void *buf, int32_t offset, int32_t n)
{
	double progress;
	
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
	filename = g_strdup (fname);
	
	SetValue (Downloader::DownloadProgressProperty, Value (1.0));
	Emit (DownloadProgressChangedEvent);
	
	// HACK, we should provide the status code
	SetValue (Downloader::StatusProperty, Value (200));
	Emit (CompletedEvent, (gpointer) fname);
}

void
Downloader::NotifyFailed (const char *msg)
{
	/* if we've already been notified of failure, no-op */
	if (failed_msg)
		return;
	
	// dl->SetValue (Downloader::StatusProperty, Value (400))
	// For some reason the status is 0, not updated on errors?
	
	Emit (DownloadFailedEvent, (gpointer) msg);
	
	// save the error in case someone else calls ::Send() on this
	// downloader for the same uri.
	failed_msg = g_strdup (msg);
}

void
Downloader::NotifySize (int64_t size)
{
	file_size = size;
	
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
	return dl->GetResponseFile (PartName);
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

void 
dummy_downloader_write (void *buf, int32_t offset, int32_t n, gpointer cb_data)
{
	g_warning ("downloader_set_function has never been called.\n");
}

void
dummy_downloader_notify_size (int64_t size, gpointer cb_data)
{
	g_warning ("downloader_set_function has never been called.\n");
}

gpointer
dummy_downloader_create_state (Downloader* dl)
{
	g_warning ("downloader_set_function has never been called.\n");
	return NULL;
}

void
dummy_downloader_destroy_state (gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
}

void
dummy_downloader_open (const char *verb, const char *uri, gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
}

void
dummy_downloader_send (gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
}

void
dummy_downloader_abort (gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
}

char *
dummy_downloader_get_response_text (char *fname, char *part, gpointer state)
{
	g_warning ("downloader_set_function has never been called.\n");
	return NULL;
}



DependencyProperty *Downloader::DownloadProgressProperty;
DependencyProperty *Downloader::ResponseTextProperty;
DependencyProperty *Downloader::StatusProperty;
DependencyProperty *Downloader::StatusTextProperty;
DependencyProperty *Downloader::UriProperty;

void
downloader_init (void)
{
	Downloader::DownloadProgressProperty = DependencyObject::Register (Type::DOWNLOADER, "DownloadProgress", new Value (0.0));
	Downloader::ResponseTextProperty = DependencyObject::Register (Type::DOWNLOADER, "ResponseText", Type::STRING);
	Downloader::StatusProperty = DependencyObject::Register (Type::DOWNLOADER, "Status", Type::INT32);
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
