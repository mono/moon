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
#include <pthread.h>

class MmsDownloader;

#include "internal-downloader.h"
#include "clock.h"
#include "downloader.h"
#include "http-streaming.h"
#include "pipeline.h"
#include "pipeline-asf.h"

#define MMS_DATA		0x44
#define MMS_HEADER	      0x48
#define MMS_METADATA	    0x4D
#define MMS_STREAM_C	    0x43
#define MMS_END		 0x45
#define MMS_PAIR_P	      0x50

#define ASF_DEFAULT_PACKET_SIZE 2888

#define VIDEO_BITRATE_PERCENTAGE 75
#define AUDIO_BITRATE_PERCENTAGE 25

struct MmsHeader {
	char b:1;
	char frame:7;
	guint8 id;
	guint16 length;
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

	guint32 asf_packet_size;
	guint32 header_size;
	guint32 size;
	guint32 packets_received;
	
	guint64 requested_pts;
	pthread_mutex_t request_mutex;
	
	TimeSpan p_packet_times[3];
	gint32 p_packet_sizes[3];

	gint32 audio_streams[128];
	gint32 video_streams[128];
	gint32 marker_stream;
	gint32 best_audio_stream;
	gint32 best_audio_stream_rate;
	gint32 best_video_stream;
	gint32 best_video_stream_rate;

	guint8 p_packet_count;

	bool described;
	bool seekable;
	bool seeked;
	
	ASFParser *parser;
	MemoryQueueSource *source;

	void AddAudioStream (int index, int bitrate) { audio_streams [index] = bitrate; if (bitrate > best_audio_stream_rate) { best_audio_stream_rate = bitrate; best_audio_stream = index; } }
	void AddVideoStream (int index, int bitrate) { video_streams [index] = bitrate; if (bitrate > best_video_stream_rate) { best_video_stream_rate = bitrate; best_video_stream = index; } }

	int GetAudioStream ();
	int GetVideoStream ();

	void RestartAtPts (guint64 pts);
	bool ProcessPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);

	bool ProcessDataPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessHeaderPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessMetadataPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);
	bool ProcessPairPacket (MmsHeader *header, MmsPacket *packet, char *payload, guint32 *size);

 public:
	MmsDownloader (Downloader *dl);
	virtual ~MmsDownloader ();

	virtual void Open (const char *verb, const char *uri);
	virtual void Write (void *buf, gint32 offset, gint32 n);
	virtual char *GetDownloadedFilename (const char *partname);
	virtual char *GetResponseText (const char *partname, gint64 *size);

	ASFParser *GetASFParser () { return parser; }
	void SetSource (MemoryQueueSource *src);

	void SetRequestedPts (guint64 value);
	guint64 GetRequestedPts ();
};

#endif /* __MMS_DOWNLOADER_H__ */
