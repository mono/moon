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
typedef void     (*downloader_open_func)(const char *verb, const char *uri, gpointer state);
typedef void     (*downloader_send_func)(gpointer state);
typedef void     (*downloader_abort_func)(gpointer state);

class Downloader : public DependencyObject {
 public:
	Downloader ();
	virtual ~Downloader ();

	virtual Type::Kind GetObjectType () { return Type::DOWNLOADER; };	

	void Abort ();
	void* GetResponseText (char* Partname, uint64_t *size);
	void Open (const char *verb, const char *URI);
	void Send ();

	static DependencyProperty *DownloadProgressProperty;
	static DependencyProperty *ResponseTextProperty;
	static DependencyProperty *StatusProperty;
	static DependencyProperty *StatusTextProperty;
	static DependencyProperty *UriProperty;

	// Events you can AddHandler to
	static int CompletedEvent;
	static int DownloadProgressChangedEvent;
	static int DownloadFailedEvent;



	// the following is stuff not exposed by C#/js, but is useful
	// when writing unmanaged code for downloader implementations
	// or data sinks.

	char* GetResponseFile (char *PartName);
	void Write (guchar *buf, gsize offset, gsize n);
	void NotifyFinished (const char *fname);
	void NotifyFailed (const char *msg);
	void NotifySize (int64_t size);

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

	bool Started ();
	bool Completed ();


 private:
	GHashTable *part_hash;

	int64_t file_size;
	int64_t total;

	char *filename;
	char *failed_msg;
	bool  started;
	
	// Set by the consumer
	downloader_write_func       write;
	downloader_notify_size_func notify_size;
	gpointer consumer_closure;

	// Set by the supplier.
	gpointer downloader_state;

	static downloader_create_state_func create_state;
	static downloader_destroy_state_func destroy_state;
	static downloader_open_func open_func;
	static downloader_send_func send_func;
	static downloader_abort_func abort_func;

	char * ll_downloader_get_response_file (char *PartName);
};

Downloader* downloader_new (void);

void downloader_set_functions (downloader_create_state_func create_state,
			       downloader_destroy_state_func destroy_state,
			       downloader_open_func open,
			       downloader_send_func send,
			       downloader_abort_func abort);

void  downloader_abort             (Downloader *dl);
void *downloader_get_response_text (Downloader *dl, char *PartName, uint64_t *size);
char *downloader_get_response_file (Downloader *dl, char *PartName);
void  downloader_open              (Downloader *dl, const char *verb, const char *URI);
void  downloader_send              (Downloader *dl);

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
