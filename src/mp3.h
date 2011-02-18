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

namespace Moonlight {

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

/*
 * Mp3FrameReader
 */
class Mp3FrameReader {
	Mp3Demuxer *demuxer;
	AudioStream *stream;
	guint64 cur_pts;
	gint64 stream_start; /* The offset of the first frame */
	guint32 frame_dur; /* The duration (in pts) of a frame */
	double frame_len; /* The length (in bytes) of a frame */
	double nframes; /* The total number of frames */
	gint32 bit_rate;
	/* If we seek forward, we'll be skipping an unknown number of frames (vbr), and since we have to calculate
	 * cur_pts by keeping a running total, it follows that after a forward seek we can't guarantee that the cur_pts
	 * is correct (it's basically just an educated guess) */
	bool is_cur_pts_guaranteed;
	
	MpegFrame *jmptab;
	guint32 avail;
	guint32 used;
	
	guint32 MpegFrameSearch (guint64 pts);
	void AddFrameIndex (gint64 offset, guint64 pts, guint32 dur, gint32 bit_rate);
	
	static MediaResult ReadFrameCallback (MediaClosure *closure);
	
public:
	Mp3FrameReader (Mp3Demuxer *demuxer, IMediaSource *source, AudioStream *stream, gint64 stream_start, double frame_len, guint32 frame_duration, double nframes);
	~Mp3FrameReader ();
	
	void Seek (guint64 pts);
	
	void ReadFrame ();
	
	/* Reads forward to the next mpeg header, returns false if not enough data */
	/* If true is returned, the stream is positioned at a mpeg header */
	static bool FindMpegHeader (MpegFrameHeader *mpeg, MpegVBRHeader *vbr, MemoryBuffer *source);
};

/*
 * Mp3Demuxer
 */
class Mp3Demuxer : public IMediaDemuxer {
private:
	Mp3FrameReader *reader;
	MemoryBuffer *current_source;
	MediaReadClosure *read_closure;
	bool waiting_for_read;
	guint64 next_read_position;
	gint64 current_position;

	void OpenDemuxer (MemoryBuffer *open_source);
	static MediaResult OpenDemuxerCallback (MediaClosure *closure);

protected:
	virtual ~Mp3Demuxer ();

	virtual void GetFrameAsyncInternal (IMediaStream *stream);
	virtual void OpenDemuxerAsyncInternal ();
	virtual void SeekAsyncInternal (guint64 timeToSeek);
	virtual void SwitchMediaStreamAsyncInternal (IMediaStream *stream) {}; // An mp3 file has only 1 stream, so this doesn't make any sense

public:
	/* @SkipFactories */
	Mp3Demuxer (Media *media, IMediaSource *source);
	virtual void Dispose ();
	virtual const char *GetName () { return "Mp3Demuxer"; }	

	Mp3FrameReader *GetReader () { return reader; }
	MemoryBuffer *GetCurrentSource () { return current_source; }
	void SetCurrentSource (MemoryBuffer *value);

	/* This method returns false if there is no more data to request */
	bool RequestMoreData (MediaCallback *callback, guint32 count = 4096);
	void SetWaitingForRead (bool value) { waiting_for_read = value; }
	void CancelPendingReads ();
	void SetNextReadPosition (gint64 value) { next_read_position = value; }
	gint64 GetNextReadPosition () { return next_read_position; }
	gint64 GetCurrentPosition () { return current_position; }
};

/*
 * Mp3DemuxerInfo
 */
class Mp3DemuxerInfo : public DemuxerInfo {
public:
	virtual MediaResult Supports (MemoryBuffer *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source, MemoryBuffer *initial_buffer);
	virtual const char *GetName () { return "Mp3Demuxer"; }
};

bool mpeg_parse_header (MpegFrameHeader *mpeg, const guint8 *buffer);
double mpeg_frame_length (MpegFrameHeader *mpeg);
};
#endif
