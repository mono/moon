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
#include "runtime.h"
#include "downloader.h"

//
// Downloader
//

downloader_create_state_func Downloader::create_state;
downloader_destroy_state_func Downloader::destroy_state;
downloader_open_func Downloader::open;
downloader_send_func Downloader::send;
downloader_abort_func Downloader::abort;
downloader_get_response_text_func Downloader::get_response_text;

Downloader::Downloader ()
{
	downloader_state = Downloader::create_state (this);
	notify_size = NULL;
	this->write = NULL;
	event_notify = NULL;
	file_size = -2;
	total = 0;
}

Downloader::~Downloader ()
{
	Downloader::destroy_state (downloader_state);
}

void
downloader_abort (Downloader *dl)
{
	dl->abort (dl->downloader_state);
}

char *
downloader_get_response_text (Downloader *dl, char* PartName)
{
	return dl->get_response_text (PartName, dl->downloader_state);
}

void
downloader_open (Downloader *dl, char *verb, char *URI, bool Async)
{
	dl->open (verb, URI, Async, dl->downloader_state);
}

void
downloader_send (Downloader *dl)
{
	dl->send (dl->downloader_state);
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

Downloader*
downloader_new (void)
{
	return new Downloader ();
}

void downloader_set_functions (downloader_create_state_func create_state,
			       downloader_destroy_state_func destroy_state,
			       downloader_open_func open,
			       downloader_send_func send,
			       downloader_abort_func abort,
			       downloader_get_response_text_func get_response)
{
	Downloader::create_state = create_state;
	Downloader::destroy_state = destroy_state;
	Downloader::open = open;
	Downloader::send = send;
	Downloader::abort = abort;
	Downloader::get_response_text = get_response;
}

void
downloader_write (Downloader *dl, guchar *buf, gsize offset, gsize n)
{
	if (dl->write)
		dl->write (buf, offset, n, dl->consumer_closure);

	// Update progress
	dl->total += n;
	double p;
	if (dl->file_size >= 0)
		p = dl->total / (double) dl->file_size;
	else 
		p = 0;

	dl->SetValue (Downloader::DownloadProgressProperty, Value (p));

	if (dl->event_notify != NULL){
		dl->event_notify (Downloader::NOTIFY_PROGRESS_CHANGED, dl->event_closure);

		if (dl->total == dl->file_size)
			dl->event_notify (Downloader::NOTIFY_COMPLETED, dl->event_closure);
	}
}

void
downloader_notify_size (Downloader *dl, int64_t size)
{
	dl->file_size = size;

	if (dl->notify_size)
		dl->notify_size (size, dl->consumer_closure);

	dl->SetValue (Downloader::DownloadProgressProperty, Value (0.0));

	if (dl->event_notify != NULL)
		dl->event_notify (Downloader::NOTIFY_PROGRESS_CHANGED, dl->event_closure);
}

void  
downloader_want_events (Downloader *dl, downloader_event_notify event_notify, gpointer closure)
{
	dl->event_notify = event_notify;
	dl->event_closure = closure;
}

DependencyProperty *Downloader::DownloadProgressProperty;
DependencyProperty *Downloader::ResponseTextProperty;
DependencyProperty *Downloader::StatusProperty;
DependencyProperty *Downloader::StatusTextProperty;
DependencyProperty *Downloader::UriProperty;

void
downloader_init (void)
{
	Downloader::DownloadProgressProperty = DependencyObject::Register (Value::DOWNLOADER, "DownloadProgress", Value::DOUBLE);
	Downloader::ResponseTextProperty = DependencyObject::Register (Value::DOWNLOADER, "ResponseText", Value::STRING);
	Downloader::StatusProperty = DependencyObject::Register (Value::DOWNLOADER, "Status", Value::INT32);
	Downloader::StatusTextProperty = DependencyObject::Register (Value::DOWNLOADER, "StatusText", Value::STRING);
	Downloader::UriProperty = DependencyObject::Register (Value::DOWNLOADER, "Uri", Value::STRING);
}
