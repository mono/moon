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

#define MMS_DATA		0x44
#define MMS_HEADER	      0x48
#define MMS_METADATA	    0x4D
#define MMS_STREAM_C	    0x43
#define MMS_END		 0x45
#define MMS_PAIR_P	      0x50

#define ASF_DEFAULT_PACKET_SIZE 2888

struct MmsHeader {
	char b:1;
	char frame:7;
	uint8_t id;
	uint16_t length;
};

struct MmsDataPacket {
	uint32_t id;
	uint8_t incarnation;
	uint8_t flags;
	uint16_t size;
};

struct MmsPacket {
	union {
		uint32_t reason;
		struct MmsDataPacket data;
	} packet;
};

typedef struct MmsHeader MmsHeader;
typedef struct MmsPacket MmsPacket;

class Downloader;

typedef void     (*downloader_write_func) (void *buf, int32_t offset, int32_t n, gpointer cb_data);
typedef void     (*downloader_notify_size_func) (int64_t size, gpointer cb_data);
typedef void     (*downloader_request_position_func) (int64_t *pos, gpointer cb_data);

typedef gpointer (*downloader_create_state_func) (Downloader *dl);
typedef void     (*downloader_destroy_state_func) (gpointer state);
typedef void     (*downloader_open_func) (const char *verb, const char *uri, bool streaming, gpointer state);
typedef void     (*downloader_send_func) (gpointer state);
typedef void     (*downloader_abort_func) (gpointer state);
typedef void     (*downloader_header_func) (gpointer state, const char *header, const char *value);
typedef void     (*downloader_body_func) (gpointer state, void *body, uint32_t length);

class Downloader : public DependencyObject {
	static downloader_create_state_func create_state;
	static downloader_destroy_state_func destroy_state;
	static downloader_open_func open_func;
	static downloader_send_func send_func;
	static downloader_abort_func abort_func;
	static downloader_header_func header_func;
	static downloader_body_func body_func;
	
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
	bool mms;

	// mms stuff
	char *buffer;

	int64_t requested_position;

	uint32_t asf_packet_size;
	uint32_t header_size;
	uint32_t size;
	uint32_t packets_received;

	int32_t audio_streams[128];
	int32_t video_streams[128];
	int32_t best_audio_stream;
	int32_t best_audio_stream_rate;
	int32_t best_video_stream;
	int32_t best_video_stream_rate;

	uint8_t p_packet_count;

	bool described;
	bool seekable;

	void AddAudioStream (int index, int bitrate) { audio_streams [index] = bitrate; if (bitrate > best_audio_stream_rate) { best_audio_stream_rate = bitrate; best_audio_stream = index; } }
	void AddVideoStream (int index, int bitrate) { video_streams [index] = bitrate; if (bitrate > best_video_stream_rate) { best_video_stream_rate = bitrate; best_video_stream = index; } }

	bool ProcessPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *size);

	bool ProcessDataPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *size);
	bool ProcessHeaderPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *size);
	bool ProcessMetadataPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *size);
	bool ProcessPairPacket (MmsHeader *header, MmsPacket *packet, char *payload, uint32_t *size);


	void InternalWrite (void *buf, int32_t offset, int32_t n);
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
				  downloader_header_func header,
				  downloader_body_func body,
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

double downloader_get_download_progress (Downloader *dl);

const char *downloader_get_status_text (Downloader *dl);
int downloader_get_status (Downloader *dl);

void downloader_set_uri (Downloader *dl, const char *uri);
const char *downloader_get_uri (Downloader *dl);

Surface *downloader_get_surface    (Downloader *dl);


void  downloader_abort	       (Downloader *dl);
char *downloader_get_downloaded_file (Downloader *dl);
char *downloader_get_response_text   (Downloader *dl, const char *PartName, uint64_t *size);
char *downloader_get_response_file   (Downloader *dl, const char *PartName);
void  downloader_open		(Downloader *dl, const char *verb, const char *uri);
void  downloader_send		(Downloader *dl);

//
// Used to push data to the consumer
//
void downloader_write		(Downloader *dl, void *buf, int32_t offset, int32_t n);
void downloader_completed       (Downloader *dl, const char *filename);

void downloader_notify_size     (Downloader *dl, int64_t size);
void downloader_notify_finished (Downloader *dl, const char *filename);
void downloader_notify_error    (Downloader *dl, const char *msg);
void downloader_request_position (Downloader *dl, int64_t *pos);


void downloader_set_functions (downloader_create_state_func create_state,
			       downloader_destroy_state_func destroy_state,
			       downloader_open_func open,
			       downloader_send_func send,
			       downloader_abort_func abort,
			       downloader_header_func header,
			       downloader_body_func body);

void downloader_init (void);

G_END_DECLS

#endif
