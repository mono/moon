/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mp3.h: Mp3 for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_MP3_H_
#define __MOON_MP3_H_

#include <glib.h>

#include "pipeline.h"


/* validate that this is an MPEG audio stream by checking that
 * the 32bit header matches the pattern:
 *
 * 1111 1111 111* **** **** **** **** **** = 0xff 0xe0
 *
 * Use a mask of 0xffe6 (because bits 12 and 13 can both be 0 if it is
 * MPEG 2.5). Compare the second byte > 0xe0 because one of the other
 * masked bits has to be set (the Layer bits cannot both be 0).
 */
#define is_mpeg_header(buffer) (buffer[0] == 0xff && ((buffer[1] & 0xe6) > 0xe0) && (buffer[1] & 0x18) != 0x08)

struct MpegFrameHeader {
	guint8 version:2;
	guint8 layer:2;
	
	guint8 copyright:1;
	guint8 original:1;
	guint8 padded:1;
	guint8 prot:1;
	
	guint8 channels:6;
	guint8 intensity:1;
	guint8 ms:1;
	
	gint32 bit_rate;
	gint32 sample_rate;
};

enum MpegVBRHeaderType {
	MpegNoVBRHeader,
	MpegXingHeader,
	MpegVBRIHeader
};

struct MpegVBRHeader {
	MpegVBRHeaderType type;
	guint32 nframes;
};

struct MpegFrame {
	gint64 offset;
	guint64 pts;
	guint32 dur;
	
	// this is needed in case this frame did not specify it's own
	// bit rate which is possible for MPEG-1 Layer 3 audio.
	gint32 bit_rate;
};

class Mp3FrameReader {
	IMediaSource *source;
	AudioStream *stream;
	gint64 stream_start;
	guint32 frame_dur;
	guint32 frame_len;
	guint64 cur_pts;
	gint32 bit_rate;
	bool xing;
	bool sync_lost;
	
	MpegFrame *jmptab;
	guint32 avail;
	guint32 used;
	
	guint32 MpegFrameSearch (guint64 pts);
	void AddFrameIndex (gint64 offset, guint64 pts, guint32 dur, gint32 bit_rate);
	
	MediaResult SkipFrame ();
	
public:
	Mp3FrameReader (IMediaSource *source, AudioStream *stream, gint64 start, guint32 frame_len, guint32 frame_duration, bool xing);
	~Mp3FrameReader ();
	
	MediaResult Seek (guint64 pts);
	
	MediaResult TryReadFrame (MediaFrame **frame);
	
	// FindMpegHeader
	//   Might change the current position of the source
	static MediaResult FindMpegHeader (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, IMediaSource *source, gint64 start, gint64 *result);
};

class Mp3Demuxer : public IMediaDemuxer {
private:
	Mp3FrameReader *reader;
	bool xing;
	
	MediaResult ReadHeader ();
	
	static MediaResult GetFrameCallback (MediaClosure *closure);
	
protected:
	virtual ~Mp3Demuxer ();

	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 timeToSeek);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream) {}; // An mp3 file has only 1 stream, so this doesn't make any sense
	
public:
	Mp3Demuxer (Media *media, IMediaSource *source);
	
	virtual const char *GetName () { return "Mp3Demuxer"; }	
};

class Mp3DemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "Mp3Demuxer"; }
};

bool mpeg_parse_header (MpegFrameHeader *mpeg, const guint8 *buffer);
double mpeg_frame_length (MpegFrameHeader *mpeg, bool xing);

#endif
