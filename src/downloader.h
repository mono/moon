/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * downloader.h: Downloader class.
 *
 * Contact:
 *   Moonlight List (moonligt-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __DOWNLOADER_H__
#define __DOWNLOADER_H__

#include <glib.h>

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "dependencyobject.h"
#include "internal-downloader.h"
#include "http-streaming.h"

class FileDownloader;
class Downloader;

typedef void     (*downloader_write_func) (void *buf, gint32 offset, gint32 n, gpointer cb_data);
typedef void     (*downloader_notify_size_func) (gint64 size, gpointer cb_data);
typedef void     (*downloader_request_position_func) (gint64 *pos, gpointer cb_data);

typedef gpointer (*downloader_create_state_func) (Downloader *dl);
typedef void     (*downloader_destroy_state_func) (gpointer state);
typedef void     (*downloader_open_func) (const char *verb, const char *uri, bool streaming, gpointer state);
typedef void     (*downloader_send_func) (gpointer state);
typedef void     (*downloader_abort_func) (gpointer state);
typedef void     (*downloader_header_func) (gpointer state, const char *header, const char *value);
typedef void     (*downloader_body_func) (gpointer state, void *body, guint32 length);
typedef gpointer (*downloader_create_webrequest_func) (const char *method, const char *uri, gpointer context);

class Downloader : public DependencyObject {
	static downloader_create_state_func create_state;
	static downloader_destroy_state_func destroy_state;
	static downloader_open_func open_func;
	static downloader_send_func send_func;
	static downloader_abort_func abort_func;
	static downloader_header_func header_func;
	static downloader_body_func body_func;
	static downloader_create_webrequest_func request_func;

	// Set by the consumer
	downloader_notify_size_func notify_size;
	downloader_write_func write;
	downloader_request_position_func  request_position;
	gpointer consumer_closure;
	
	// Set by the supplier.
	gpointer downloader_state;
	
	gpointer context;
	HttpStreamingFeatures streaming_features;
	
	gint64 file_size;
	gint64 total;
	
	char *filename;
	
	char *failed_msg;
	bool send_queued;
	bool started;
	bool aborted;
	InternalDownloader *internal_dl;

 protected:
	virtual ~Downloader ();
	
	void SetStatusText (const char *text);
	void SetStatus (int status);
	
 public:
	// Properties
	static DependencyProperty *DownloadProgressProperty;
	static DependencyProperty *ResponseTextProperty;
	static DependencyProperty *StatusProperty;
	static DependencyProperty *StatusTextProperty;
	static DependencyProperty *UriProperty;
	
	// Events you can AddHandler to
	const static int CompletedEvent;
	const static int DownloadProgressChangedEvent;
	const static int DownloadFailedEvent;
	
	Downloader ();
	
	virtual Type::Kind GetObjectType () { return Type::DOWNLOADER; };	
	
	void Abort ();
	char *GetResponseText (const char *Partname, guint64 *size);
	void Open (const char *verb, const char *uri);
	void SendInternal ();
	void Send ();
	void SendNow ();
	
	// the following is stuff not exposed by C#/js, but is useful
	// when writing unmanaged code for downloader implementations
	// or data sinks.
	
	void InternalAbort ();
	void InternalWrite (void *buf, gint32 offset, gint32 n);
	void InternalOpen (const char *verb, const char *uri, bool streaming);
	void InternalSetHeader (const char *header, const char *value);
	void InternalSetBody (void *body, guint32 length);

	void Write (void *buf, gint32 offset, gint32 n);
	void NotifyFinished ();
	void NotifyFailed (const char *msg);
	void NotifySize (gint64 size);
	char *GetDownloadedFilename (const char *partname);
	void SetFilename (const char *fname);
	
	InternalDownloader *GetInternalDownloader () { return internal_dl; }
	
	// This is called by the consumer of the downloaded data (the
	// Image class for instance)
	void SetWriteFunc (downloader_write_func write,
			   downloader_notify_size_func notify_size,
			   gpointer closure);
	
	// This is called by the supplier of the downloaded data (the
	// managed framework, the browser plugin, the demo test)
	static void SetFunctions (downloader_create_state_func create_state,
				  downloader_destroy_state_func destroy_state,
				  downloader_open_func open,
				  downloader_send_func send,
				  downloader_abort_func abort,
				  downloader_header_func header,
				  downloader_body_func body,
			          downloader_create_webrequest_func request,
				  bool only_if_not_set);
	
	void RequestPosition (gint64 *pos);
	void SetRequestPositionFunc (downloader_request_position_func request_position);
	
	bool Started ();
	bool Completed ();
	bool IsAborted () { return aborted; }
	
	void     SetContext (gpointer context) { this->context = context;}
	gpointer GetContext () { return context; }
	gpointer GetDownloaderState () { return downloader_state; }
	void     SetHttpStreamingFeatures (HttpStreamingFeatures features) { streaming_features = features; }
	HttpStreamingFeatures GetHttpStreamingFeatures () { return streaming_features; }
	downloader_create_webrequest_func GetRequestFunc () {return request_func; }

	//
	// Property Accessors
	//
	void SetDownloadProgress (double progress);
	double GetDownloadProgress ();
	
	const char *GetStatusText ();
	int GetStatus ();
	
	void SetUri (const char *uri);
	const char *GetUri ();

	// FIXME: This is exposed for text right now and should be cleaned up.
	FileDownloader *getFileDownloader () { return (FileDownloader *) internal_dl; }
};

class DownloaderResponse;

typedef uint32_t (* DownloaderResponseStartedHandler) (DownloaderResponse *response, gpointer context);
typedef uint32_t (* DownloaderResponseDataAvailableHandler) (DownloaderResponse *response, gpointer context, char *buffer, uint32_t length);
typedef uint32_t (* DownloaderResponseFinishedHandler) (DownloaderResponse *response, gpointer context, bool success, gpointer data);


class DownloaderResponse {
 protected:
	DownloaderResponseStartedHandler started;
	DownloaderResponseDataAvailableHandler available;
	DownloaderResponseFinishedHandler finished;
	gpointer context;

	bool aborted;

 public:
	DownloaderResponse ()
	{
		aborted = false;
	}

	DownloaderResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context)
	{
		this->aborted = false;
		this->started = started;
		this->available = available;
		this->finished = finished;
		this->context = context;
	}

	virtual ~DownloaderResponse ()
	{
	}

	virtual void Abort () = 0;
	virtual const bool IsAborted () { return this->aborted; }
};

class DownloaderRequest {
 protected:
	char *uri;
	char *method;

	bool aborted;

 public:
	DownloaderRequest (const char *method, const char *uri)
	{
		this->method = g_strdup (method);
		this->uri = g_strdup (uri);
	}

	virtual ~DownloaderRequest ()
	{
		g_free (method);
		g_free (uri);
	}

	virtual void Abort () = 0;
	virtual bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context) = 0;
	virtual const bool IsAborted () { return this->aborted; }
	virtual void SetHttpHeader (const char *name, const char *value) = 0;
	virtual void SetBody (void *body, int size) = 0;
};


Downloader *downloader_new (void);

double downloader_get_download_progress (Downloader *dl);

const char *downloader_get_status_text (Downloader *dl);
int downloader_get_status (Downloader *dl);

void downloader_set_uri (Downloader *dl, const char *uri);
const char *downloader_get_uri (Downloader *dl);

Surface *downloader_get_surface    (Downloader *dl);


void  downloader_abort	       (Downloader *dl);
char *downloader_get_downloaded_file (Downloader *dl);
char *downloader_get_response_text   (Downloader *dl, const char *PartName, guint64 *size);
char *downloader_get_response_file   (Downloader *dl, const char *PartName);
void  downloader_open		(Downloader *dl, const char *verb, const char *uri);
void  downloader_send		(Downloader *dl);

//
// Used to push data to the consumer
//
void downloader_write		(Downloader *dl, void *buf, gint32 offset, gint32 n);
void downloader_completed       (Downloader *dl, const char *filename);

void downloader_notify_size     (Downloader *dl, gint64 size);
void downloader_notify_finished (Downloader *dl, const char *filename);
void downloader_notify_error    (Downloader *dl, const char *msg);
void downloader_request_position (Downloader *dl, gint64 *pos);


void downloader_set_functions (downloader_create_state_func create_state,
			       downloader_destroy_state_func destroy_state,
			       downloader_open_func open,
			       downloader_send_func send,
			       downloader_abort_func abort,
			       downloader_header_func header,
			       downloader_body_func body,
			       downloader_create_webrequest_func request);

void downloader_init (void);

void *downloader_create_webrequest (Downloader *dl, const char *method, const char *uri);

G_END_DECLS

#endif
