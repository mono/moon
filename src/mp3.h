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
	IMediaSource *stream;
	gint64 stream_start;
	guint32 frame_dur;
	guint32 frame_len;
	guint64 cur_pts;
	gint32 bit_rate;
	bool xing;
	
	MpegFrame *jmptab;
	guint32 avail;
	guint32 used;
	
	guint32 MpegFrameSearch (guint64 pts);
	void AddFrameIndex (gint64 offset, guint64 pts, guint32 dur, gint32 bit_rate);
	
	bool SkipFrame ();
	
public:
	Mp3FrameReader (IMediaSource *source, gint64 start, guint32 frame_len, guint32 frame_duration, bool xing);
	~Mp3FrameReader ();
	
	bool Seek (guint64 pts);
	
	MediaResult ReadFrame (MediaFrame *frame);
	
	gint64 EstimatePtsPosition (guint64 pts);
};

class Mp3Demuxer : public IMediaDemuxer {
private:
	Mp3FrameReader *reader;
	bool xing;

protected:
	virtual ~Mp3Demuxer ();

public:
	Mp3Demuxer (Media *media, IMediaSource *source);
	
	virtual MediaResult ReadHeader ();
	virtual MediaResult ReadFrame (MediaFrame *frame);
	virtual MediaResult Seek (guint64 pts);
	virtual gint64 EstimatePtsPosition (guint64 pts);
	virtual const char *GetName () { return "Mp3Demuxer"; }
};

class Mp3DemuxerInfo : public DemuxerInfo {
public:
	virtual bool Supports (IMediaSource *source);
	virtual IMediaDemuxer *Create (Media *media, IMediaSource *source); 
	virtual const char *GetName () { return "Mp3Demuxer"; }
};

class NullMp3Decoder : public IMediaDecoder {
protected:
	virtual ~NullMp3Decoder () {};

public:
	NullMp3Decoder (Media *media, IMediaStream *stream) : IMediaDecoder (media, stream) {}
	
	virtual MediaResult DecodeFrame (MediaFrame *frame);
	
	virtual MediaResult Open ()
	{
		return MEDIA_SUCCESS;
	}
};

class NullMp3DecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char *codec) { return !strcmp (codec, "mp3"); };
	
	virtual IMediaDecoder *Create (Media *media, IMediaStream *stream)
	{
		return new NullMp3Decoder (media, stream);
	}
};

bool mpeg_parse_header (MpegFrameHeader *mpeg, const guint8 *buffer);
guint32 mpeg_frame_length (MpegFrameHeader *mpeg, bool xing);

#endif
