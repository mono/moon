/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mms-downloader.h: MMS Downloader class.
 *
 * Contact:
 *   Moonlight List (moonlist-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MMS_DOWNLOADER_H__
#define __MMS_DOWNLOADER_H__

#include <glib.h>

class MmsDownloader;
class ContentDescriptionList;

#include "internal-downloader.h"
#include "downloader.h"
#include "http-streaming.h"
#include "pipeline.h"
#include "pipeline-asf.h"
#include "mutex.h"

#define MMS_DATA      0x44
#define MMS_HEADER	  0x48
#define MMS_METADATA  0x4D
#define MMS_STREAM_C  0x43
#define MMS_END       0x45
#define MMS_PAIR_P    0x50

struct MmsHeader {
	char b:1;
	char frame:7;
	guint8 id;
	guint16 length;
};

struct MmsHeaderReason {
	char b:1;
	char frame:7;
	guint8 id;
	guint16 length;
	guint32 reason;
};

struct MmsDataPacket {
	guint32 id;
	guint8 incarnation;
	guint8 flags;
	guint16 size;
};

struct MmsPacket {
	union {
		guint32 reason;
		MmsDataPacket data;
	} packet;
};

typedef struct MmsHeader MmsHeader;
typedef struct MmsPacket MmsPacket;

class MmsDownloader : public InternalDownloader {
 private:
	char *uri;
	char *buffer;
	char *client_id;
	char *playlist_gen_id;
	bool failure_reported;

	guint32 size;
	
	guint64 requested_pts;
	Mutex request_mutex;
	
	TimeSpan p_packet_times[3];
	gint32 p_packet_sizes[3];
	guint8 p_packet_count;
	guint64 max_bitrate;

	bool is_playing;
	bool stream_switched;
	
	MmsSource *source;
	ContentDescriptionList *content_descriptions;

	static void PlayCallback (EventObject *sender);
	void Play (); // sends a play request. if SetRequestedPts has been called, start from that pts
	void SelectStreams ();  // sends a select stream request

	bool ProcessPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);

	bool ProcessDataPacket         (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessHeaderPacket       (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessMetadataPacket     (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessPairPacket         (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessStreamSwitchPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessEndPacket          (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);

	static void ProcessResponseHeaderCallback (gpointer context, const char *header, const char *value);
	void ProcessResponseHeader (const char *header, const char *value);

 protected:
	virtual ~MmsDownloader ();

 public:
	MmsDownloader (Downloader *dl);

	virtual void Open (const char *verb, const char *uri);
	virtual void Write (void *buf, gint32 offset, gint32 n);
	virtual char *GetDownloadedFilename (const char *partname);
	virtual char *GetResponseText (const char *partname, gint64 *size);
	virtual void SetFilename (const char *fname) { /* we don't need this */ }

	void SetSource (MmsSource *src); // main thread only
	MmsPlaylistEntry *GetCurrentEntryReffed (); // main thread only

	void SetRequestedPts (guint64 value); // thread safe
	guint64 GetRequestedPts (); // thread safe

	const char *GetUri () { return uri; }
	const char *GetClientId () { return client_id; }
	const char *GetPlaylistGenId () { return playlist_gen_id; }
	guint64 GetMaxBitrate () { return max_bitrate; }
};

class ContentDescriptionList {
public:
	List list;
	bool Parse (const char *input, gint32 length);
};

class ContentDescription : public List::Node {
public:
	enum ValueType {
		VT_BOOL   = 11,
		VT_BSTR   =  8,
		VT_CY     =  6,
		VT_ERROR  = 10,
		VT_I1     = 16,
		VT_I2     =  2,
		VT_I4     =  3,
		VT_INT    = 22,
		VT_LPWSTR = 31,
		VT_UI1    = 17,
		VT_UI2    = 18,
		VT_UI4    = 19,
		VT_UINT   = 23,
	};

public:
	virtual ~ContentDescription ();
	char *name;
	ValueType value_type;
	void *value;
	int value_length;
};

class MmsSecondDownloader : public EventObject {
private:
	Downloader *dl;
	MmsDownloader *mms;
	guint kill_timeout;

	EVENTHANDLER (MmsSecondDownloader, DownloadFailed, EventObject, EventArgs);
	EVENTHANDLER (MmsSecondDownloader, Completed, EventObject, EventArgs);
	static void data_write (void *data, gint32 offset, gint32 n, void *closure);
	void CreateDownloader ();
	static gboolean KillTimeoutCallback (gpointer context);
	void KillTimeoutHandler ();

protected:
	virtual ~MmsSecondDownloader () {}

public:
	MmsSecondDownloader (MmsDownloader *mms);

	virtual void Dispose ();
	void SendStreamSwitch ();
	// TODO: void SendLog ();
	void SetKillTimeout (guint seconds); // the second downloader will kill itself after this amount of time
};

#endif /* __MMS_DOWNLOADER_H__ */
