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

G_BEGIN_DECLS

#include "internal-downloader.h"
#include "downloader.h"
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

class MmsDownloader : public InternalDownloader {
 private:
	char *uri;
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

 public:
	MmsDownloader (Downloader *dl);
	~MmsDownloader ();

	void Open (const char *verb, const char *uri);
	void Write (void *buf, int32_t offset, int32_t n);
};

G_END_DECLS

#endif
