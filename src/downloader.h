/*
 * downloader.h: Downloader class.
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

#ifndef __DOWNLOADER_H__
#define __DOWNLOADER_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "dependencyobject.h"

class Downloader;

typedef void     (*downloader_write_func)(guchar *buf, gsize offset, gsize n, gpointer cb_data);
typedef void     (*downloader_notify_size_func)(int64_t size, gpointer cb_data);

typedef gpointer (*downloader_create_state_func) (Downloader* dl);
typedef void     (*downloader_destroy_state_func) (gpointer state);
typedef void     (*downloader_open_func)(char *verb, char *uri, bool async, gpointer state);
typedef void     (*downloader_send_func)(gpointer state);
typedef void     (*downloader_abort_func)(gpointer state);
typedef char*    (*downloader_get_response_text_func)(char *part, gpointer state);

//
// downloader_event_notify:
//    @kind:    indicates the kind of operation that we are being notified about
//    @cb_data: callback data, this is the closure value passed to SetWriteFunc.
//    @extra:   Depends on the value of kind:
//              kind == NOTIFY_COMPLETED, extra = filename that holds the full downloaded data
//              kind == NOTIFY_DOWNLOAD_FAILED, extra = error message to provide on ErrorArgs
//
typedef void     (*downloader_event_notify) (int kind, gpointer cb_data, gpointer extra);

class Downloader : public DependencyObject {
 public:
	Downloader ();
	virtual ~Downloader ();

	virtual Type::Kind GetObjectType () { return Type::DOWNLOADER; };	

	static DependencyProperty *DownloadProgressProperty;
	static DependencyProperty *ResponseTextProperty;
	static DependencyProperty *StatusProperty;
	static DependencyProperty *StatusTextProperty;
	static DependencyProperty *UriProperty;


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
				  downloader_get_response_text_func get_response_text);

	enum EventKind {
		NOTIFY_COMPLETED,
		NOTIFY_PROGRESS_CHANGED,
		NOTIFY_DOWNLOAD_FAILED
	};

	GSList *downloader_events;

	int64_t file_size;
	int64_t total;

	char *filename;
	bool  started;
	
	// Set by the consumer
	downloader_write_func       write;
	downloader_notify_size_func notify_size;
	gpointer consumer_closure;

	// Set by the supplier.
	gpointer downloader_state;

	static downloader_create_state_func create_state;
	static downloader_destroy_state_func destroy_state;
	static downloader_open_func open;
	static downloader_send_func send;
	static downloader_abort_func abort;
	static downloader_get_response_text_func get_response_text;

	bool Started ();
	bool Completed ();

};

Downloader* downloader_new (void);

void downloader_set_functions (downloader_create_state_func create_state,
			       downloader_destroy_state_func destroy_state,
			       downloader_open_func open,
			       downloader_send_func send,
			       downloader_abort_func abort,
			       downloader_get_response_text_func get_response);

void  downloader_abort             (Downloader *dl);
void *downloader_get_response_text (Downloader *dl, char *PartName, uint *size);
void  downloader_open              (Downloader *dl, char *verb, char *URI, bool Async);
void  downloader_send              (Downloader *dl);

void  downloader_want_events       (Downloader *dl, downloader_event_notify event_notify, gpointer closure);

//
// Used to push data to the consumer
//
void downloader_write           (Downloader *dl, guchar *buf, gsize offset, gsize n);
void downloader_completed       (Downloader *dl, const char *filename);

void downloader_notify_size     (Downloader *dl, int64_t size);
void downloader_notify_finished (Downloader *dl, const char *filename);
void downloader_notify_error    (Downloader *dl, const char *msg);

void downloader_init (void);

G_END_DECLS

#endif
