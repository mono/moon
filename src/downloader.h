/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * downloader.h: Downloader class.
 *
 * Author:
 *   Chris Toshok (toshok@novell.com)
 *   Miguel de Icaza (miguel@novell.com).
 *   Jeffrey Stedfast (fejj@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __DOWNLOADER_H__
#define __DOWNLOADER_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "dependencyobject.h"
#include "http-streaming.h"

class Downloader;

typedef void     (*downloader_write_func) (void *buf, int32_t offset, int32_t n, gpointer cb_data);
typedef void     (*downloader_notify_size_func) (int64_t size, gpointer cb_data);
typedef void     (*downloader_request_position_func) (int64_t *pos, gpointer cb_data);

typedef gpointer (*downloader_create_state_func) (Downloader *dl);
typedef void     (*downloader_destroy_state_func) (gpointer state);
typedef void     (*downloader_open_func) (const char *verb, const char *uri, gpointer state);
typedef void     (*downloader_send_func) (gpointer state);
typedef void     (*downloader_abort_func) (gpointer state);

class Downloader : public DependencyObject {
	static downloader_create_state_func create_state;
	static downloader_destroy_state_func destroy_state;
	static downloader_open_func open_func;
	static downloader_send_func send_func;
	static downloader_abort_func abort_func;
	
	// Set by the consumer
	downloader_notify_size_func notify_size;
	downloader_write_func write;
	downloader_request_position_func  request_position;
	gpointer consumer_closure;
	
	// Set by the supplier.
	gpointer downloader_state;
	
	gpointer context;
	HttpStreamingFeatures streaming_features;
	
	int64_t file_size;
	int64_t total;
	
	char *filename;
	char *unzipdir;
	
	char *failed_msg;
	bool deobfuscated;
	bool send_queued;
	bool unlinkit;
	bool unzipped;
	bool started;
	bool aborted;
	
	void CleanupUnzipDir ();
	
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
	char *GetResponseText (const char *Partname, uint64_t *size);
	void Open (const char *verb, const char *uri);
	void SendInternal ();
	void Send ();
	void SendNow ();
	
	// the following is stuff not exposed by C#/js, but is useful
	// when writing unmanaged code for downloader implementations
	// or data sinks.
	
	void Write (void *buf, int32_t offset, int32_t n);
	void NotifyFinished (const char *fname);
	void NotifyFailed (const char *msg);
	void NotifySize (int64_t size);
	
	char *GetDownloadedFilePart (const char *PartName);
	const char *GetDownloadedFile ();
	bool DownloadedFileIsZipped ();
	const char *GetUnzippedPath ();
	
	void RequestPosition (int64_t *pos);
	bool IsDeobfuscated ();
	void SetDeobfuscated (bool val);
	void SetDeobfuscatedFile (const char *filename);
	
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
				  bool only_if_not_set);
	
	void SetRequestPositionFunc (downloader_request_position_func request_position);
	
	bool Started ();
	bool Completed ();
	
	void     SetContext (gpointer context) { this->context = context;}
	gpointer GetContext () { return context; }
	gpointer GetDownloaderState () { return downloader_state; }
	void     SetHttpStreamingFeatures (HttpStreamingFeatures features) { streaming_features = features; }
	HttpStreamingFeatures GetHttpStreamingFeatures () { return streaming_features; }
	
	//
	// Property Accessors
	//
	void SetDownloadProgress (double progress);
	double GetDownloadProgress ();
	
	const char *GetStatusText ();
	int GetStatus ();
	
	void SetUri (const char *uri);
	const char *GetUri ();
};

Downloader *downloader_new (void);

void downloader_set_functions (downloader_create_state_func create_state,
			       downloader_destroy_state_func destroy_state,
			       downloader_open_func open,
			       downloader_send_func send,
			       downloader_abort_func abort);

Surface *downloader_get_surface    (Downloader *dl);

void  downloader_abort             (Downloader *dl);
void *downloader_get_response_text (Downloader *dl, const char *PartName, uint64_t *size);
char *downloader_get_response_file (Downloader *dl, const char *PartName);
void  downloader_open              (Downloader *dl, const char *verb, const char *uri);
void  downloader_send              (Downloader *dl);

//
// Used to push data to the consumer
//
void downloader_write           (Downloader *dl, void *buf, int32_t offset, int32_t n);
void downloader_completed       (Downloader *dl, const char *filename);

void downloader_notify_size     (Downloader *dl, int64_t size);
void downloader_notify_finished (Downloader *dl, const char *filename);
void downloader_notify_error    (Downloader *dl, const char *msg);
void downloader_request_position (Downloader *dl, int64_t *pos);

void downloader_init (void);

G_END_DECLS

#endif
