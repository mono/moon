/*
 * runtime.cpp: Core surface and canvas definitions.
 *
 * Author:
 *   Chris Toshok (toshok@novell.com)
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
	dl->write (buf, offset, n, dl->consumer_closure);
}

void
downloader_notify_size (Downloader *dl, int64_t size)
{
	dl->notify_size (size, dl->consumer_closure);
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
